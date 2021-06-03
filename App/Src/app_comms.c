/*
 * Copyright 2020-2021 Jetperch LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "app_comms.h"
#include "fitterbap/comm/stack.h"
#include "fitterbap/assert.h"
#include "fitterbap/cstr.h"
#include "fitterbap/ec.h"
#include "fitterbap/log.h"
#include "fitterbap/time.h"
#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "uart4.h"
#include "uart5.h"
#include "cmsis_os.h"
#include "main.h"
#include "stm32g4xx_ll_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#define TIMEOUT_DEFAULT_MS (1000)

#define PUBSUB_TASK_STACK (256)
#define PUBSUB_TASK_PRIORITY ((osPriority_t) osPriorityNormal)
#define PUBSUB_SERVICE_TIME_MAX_MS (500)
#define DATA_DYNAMIC_BUFFER_SIZE (512)

static fbp_os_mutex_t pubsub_mutex_;
struct fbp_pubsub_s * pubsub = NULL;
static TaskHandle_t pubsub_task_;

enum pubsub_events_e {
    PUBSUB_EV_RECV = (1 << 0),
};

struct stack_fn_s {
    void (*initialize)();
    void (*evm_api)(struct fbp_evm_api_s * api);
    void (*recv_register)(uart1_recv_fn recv_fn, void * recv_user_data);
    int32_t (*send)(uint8_t const *buffer, uint32_t buffer_size);
    uint32_t (*send_available)();
    void (*mutex)(fbp_os_mutex_t * mutex);
};

// function pointer table to uart instances
static const struct stack_fn_s stack_fn[5] = {
    {uart1_initialize, uart1_evm_api, uart1_recv_register, uart1_send, uart1_send_available, uart1_mutex},
    {uart2_initialize, uart2_evm_api, uart2_recv_register, uart2_send, uart2_send_available, uart2_mutex},
    {uart3_initialize, uart3_evm_api, uart3_recv_register, uart3_send, uart3_send_available, uart3_mutex},
    {uart4_initialize, uart4_evm_api, uart4_recv_register, uart4_send, uart4_send_available, uart4_mutex},
    {uart5_initialize, uart5_evm_api, uart5_recv_register, uart5_send, uart5_send_available, uart5_mutex},
};

struct fbp_stack_s * stacks[5] = {NULL, NULL, NULL, NULL, NULL};

int64_t fbp_time_utc() {
    return 0;  // todo implement using time sync
}

static char app_prefix() {
    // Note that this value is also available as "_/topic/prefix"
    static uint8_t ch = 0;
    if (~ch) {
        ch = 'a'
             + (LL_GPIO_IsInputPinSet(ID0_GPIO_Port, ID0_Pin) ? 1 : 0)
             + (LL_GPIO_IsInputPinSet(ID1_GPIO_Port, ID1_Pin) ? 2 : 0)
             + (LL_GPIO_IsInputPinSet(ID2_GPIO_Port, ID2_Pin) ? 4 : 0);
    }
    return ch;
}

static inline void topic_extend(char * topic_target, const char * topic_local) {
    topic_target[0] = app_prefix();
    topic_target[1] = '/';
    fbp_cstr_copy(&topic_target[2], topic_local, FBP_PUBSUB_TOPIC_LENGTH_MAX - 2);
}

#define TOPIC_EXTEND()                                                      \
    char topic_ex[FBP_PUBSUB_TOPIC_LENGTH_MAX];                             \
    topic_extend(topic_ex, topic)

int32_t app_subscribe(const char * topic, uint8_t flags, fbp_pubsub_subscribe_fn cbk_fn, void * cbk_user_data) {
    TOPIC_EXTEND();
    return fbp_pubsub_subscribe(pubsub, topic_ex, flags, cbk_fn, cbk_user_data);
}

int32_t app_unsubscribe(const char * topic, fbp_pubsub_subscribe_fn cbk_fn, void * cbk_user_data) {
    TOPIC_EXTEND();
    return fbp_pubsub_unsubscribe(pubsub, topic_ex, cbk_fn, cbk_user_data);
}

int32_t app_publish(const char * topic, const struct fbp_union_s * value,
                    fbp_pubsub_subscribe_fn src_fn, void * src_user_data) {
    TOPIC_EXTEND();
    return fbp_pubsub_publish(pubsub, topic_ex, value, src_fn, src_user_data);
}

int32_t app_meta(const char * topic, const char * meta_json) {
    TOPIC_EXTEND();
    return fbp_pubsub_meta(pubsub, topic_ex, meta_json);
}

int32_t app_query(const char * topic, struct fbp_union_s * value) {
    TOPIC_EXTEND();
    return fbp_pubsub_query(pubsub, topic_ex, value);
}

static void pubsub_task(void *argument) {
    (void) argument;
    uint32_t notify;
    while (1) {
        notify = 0;
        if (pdTRUE == xTaskNotifyWait(0, 0xffffffff, &notify, PUBSUB_SERVICE_TIME_MAX_MS)) {
            fbp_pubsub_process(pubsub);
        }
        // todo watchdog pet, regardless of data send/receive
    }
}

static void on_publish(void * user_data) {
    (void) user_data;
    if (pubsub_task_) {
        xTaskNotify(pubsub_task_, PUBSUB_EV_RECV, eSetBits);
    }
}

void app_pubsub_initialize() {
    if (pubsub) {
        return;
    }
    pubsub_mutex_ = xSemaphoreCreateMutex();
    FBP_ASSERT_ALLOC(pubsub_mutex_);

    char pubsub_topic_prefix[2];
    pubsub_topic_prefix[0] = app_prefix();
    pubsub_topic_prefix[1] = 0;
    pubsub = fbp_pubsub_initialize(pubsub_topic_prefix, DATA_DYNAMIC_BUFFER_SIZE);
    FBP_ASSERT_ALLOC(pubsub);

    if (pdTRUE != xTaskCreate(
            pubsub_task,            /* pvTaskCode */
            "pubsub",               /* pcName */
            PUBSUB_TASK_STACK,      /* usStackDepth in 32-bit words */
            NULL,                   /* pvParameters */
            PUBSUB_TASK_PRIORITY,   /* uxPriority */
            &pubsub_task_)) {
        FBP_FATAL("pubsub task");
    }

    pubsub_mutex_ = fbp_os_mutex_alloc();
    if (!pubsub_mutex_) {
        FBP_FATAL("mutex");
    }
    fbp_pubsub_register_mutex(pubsub, pubsub_mutex_);
    fbp_pubsub_register_on_publish(pubsub, on_publish, NULL);
}

static void parent_phy_send(void * user_data, uint8_t const * buffer, uint32_t buffer_size) {
    struct stack_fn_s * stack = (struct stack_fn_s *) user_data;
    stack->send(buffer, buffer_size);
}

static uint32_t parent_phy_send_available(void * user_data) {
    struct stack_fn_s * stack = (struct stack_fn_s *) user_data;
    return stack->send_available();
}

static void on_uart_recv_fn(void *user_data, uint8_t *buffer, uint32_t buffer_size) {
    struct fbp_stack_s * stack = (struct fbp_stack_s *) user_data;
    fbp_dl_ll_recv(stack->dl, buffer, buffer_size);
}

int32_t parent_link_initialize(struct fbp_pubsub_s * pubsub) {
    char subtopic[] = "c0/";
    struct fbp_evm_api_s evm_api;
    fbp_os_mutex_t mutex;
    char topic[FBP_PUBSUB_TOPIC_LENGTH_MAX];
    struct fbp_dl_config_s dl_config = {
            .tx_window_size = 16,
            .rx_window_size = 16,
            .tx_timeout = 15 * FBP_TIME_MILLISECOND,
            .tx_link_size = 64,
    };

    for (int uart_offset = 0; uart_offset < 5; ++uart_offset) {
        const struct stack_fn_s * fn = &stack_fn[uart_offset];
        fn->initialize();
        fn->evm_api(&evm_api);
        fn->mutex(&mutex);

        struct fbp_dl_ll_s ll = {
                .user_data = (void *) fn,
                .send = parent_phy_send,
                .send_available = parent_phy_send_available,
        };

        subtopic[1] = '0' + uart_offset + 1;
        topic_extend(topic, subtopic);

        enum fbp_port0_mode_e mode = (uart_offset & 1) ? FBP_PORT0_MODE_CLIENT : FBP_PORT0_MODE_SERVER;
        stacks[uart_offset] = fbp_stack_initialize(&dl_config, mode, topic,
                                                   &evm_api, &ll, pubsub);
        if (!stacks[uart_offset]) {
            FBP_FATAL("host_link_stack");
        }
        fbp_stack_mutex_set(stacks[uart_offset], mutex);
        fn->recv_register(on_uart_recv_fn, stacks[uart_offset]);
    }

    return 0;
}

void app_comms_initialize() {
    parent_link_initialize(pubsub);
}
