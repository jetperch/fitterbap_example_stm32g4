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
#include "fitterbap/logh.h"
#include "fitterbap/cdef.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#define LOG_TASK_STACK (256)
#define LOG_TASK_PRIORITY ((osPriority_t) osPriorityBelowNormal)
#define LOG_SERVICE_TIME_MAX_MS (500)
#define LOG_MSG_BUFFERS_MAX (10)

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
    struct fbp_logh_s * logh = (struct fbp_logh_s *) argument;
    uint32_t notify;
    uint32_t wait;
    int32_t rc = 0;
    fbp_logh_publish_register(logh, on_publish, logh);
    while (1) {
        notify = 0;
        wait = rc ? 1 : LOG_SERVICE_TIME_MAX_MS;
        xTaskNotifyWait(0, 0xffffffff, &notify, wait);
        rc = fbp_logh_process(logh);
        // todo watchdog pet
    }
}

struct fbp_logh_s * log_handler_factory(const char * topic) {
    struct fbp_logh_s * logh = fbp_logh_initialize(topic[0], LOG_MSG_BUFFERS_MAX, NULL);

    if (pdTRUE != xTaskCreate(
            log_task,            /* pvTaskCode */
            "log",               /* pcName */
            LOG_TASK_STACK,      /* usStackDepth in 32-bit words */
            logh,                /* pvParameters */
            LOG_TASK_PRIORITY,   /* uxPriority */
            &task_)) {
        FBP_FATAL("log task");
    }
    return logh;
}
