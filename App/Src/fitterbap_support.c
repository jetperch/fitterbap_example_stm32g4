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

#include "fitterbap/assert.h"
#include "fitterbap/log.h"
#include "fitterbap/time.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32g4xx.h"

void fbp_fatal(char const * file, int line, char const * msg) {
    (void) file;
    (void) line;
    (void) msg;
    taskDISABLE_INTERRUPTS();
    while (1) {
        // stall forever: watchdog should cause reset
    }
}

static void * hal_alloc(fbp_size_t size_bytes) {
    void * p = pvPortMalloc((size_t) size_bytes);
    if (!p) {
        size_t sz = xPortGetFreeHeapSize();
        FBP_LOGE("alloc(%d) but only %d remain", (int) size_bytes, (int) sz);
        FBP_FATAL("alloc");
    }
    return p;
}

static void hal_free(void * ptr) {
    FBP_FATAL("free");
    vPortFree(ptr);
}

void fitterbap_support_initialize() {
    fbp_allocator_set(hal_alloc, hal_free);
}

FBP_API struct fbp_time_counter_s fbp_time_counter() {
    struct fbp_time_counter_s counter;
    counter.value = xTaskGetTickCount();
    counter.frequency = 1000;
    return counter;
}

