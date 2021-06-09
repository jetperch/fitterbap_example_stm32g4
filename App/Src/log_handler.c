/*
 * Copyright 2021 Jetperch LLC
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

#include "log_handler.h"
#include "fitterbap/comm/log_port.h"
#include "fitterbap/cdef.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#define LOG_TASK_STACK (256)
#define LOG_TASK_PRIORITY ((osPriority_t) osPriorityBelowNormal)
#define LOG_SERVICE_TIME_MAX_MS (500)
#define LOG_MSG_BUFFERS_MAX (10)

static fbp_os_mutex_t mutex_;
static TaskHandle_t task_;
struct fbp_port_api_s * log_handler_api;

enum events_e {
    EV_PUBLISH = (1 << 0),
};

static void on_publish(void * user_data) {
    (void) user_data;
    if (task_) {
        xTaskNotify(task_, EV_PUBLISH, eSetBits);
    }
}

static void log_task(void *argument) {
    struct fbp_port_api_s * api = (struct fbp_port_api_s *) argument;
    uint32_t notify;
    uint32_t wait;
    int32_t rc = 0;
    while (1) {
        notify = 0;
        wait = rc ? 1 : LOG_SERVICE_TIME_MAX_MS;
        xTaskNotifyWait(0, 0xffffffff, &notify, wait);
        rc = fbp_logp_process(api);
        // todo watchdog pet
    }
}

struct fbp_port_api_s * log_handler_factory(const char * topic) {
    mutex_ = xSemaphoreCreateMutex();
    FBP_ASSERT_ALLOC(mutex_);

    struct fbp_logp_config_s config = {
            .msg_buffers_max = LOG_MSG_BUFFERS_MAX,
            .on_publish_fn = on_publish,
            .on_publish_user_data = NULL,
            .mutex = mutex_,
    };
    log_handler_api = fbp_logp_factory(&config);
    log_handler_api->topic_prefix = topic;
    log_handler_api->port_id = 2;

    if (pdTRUE != xTaskCreate(
            log_task,            /* pvTaskCode */
            "log",               /* pcName */
            LOG_TASK_STACK,      /* usStackDepth in 32-bit words */
            log_handler_api,     /* pvParameters */
            LOG_TASK_PRIORITY,   /* uxPriority */
            &task_)) {
        FBP_FATAL("log task");
    }
    return log_handler_api;
}
