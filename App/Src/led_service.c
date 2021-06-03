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

#include "led_service.h"
#include "led.h"
#include "app_comms.h"
#include "fitterbap/ec.h"
#include "fitterbap/log.h"
#include <stdbool.h>

static const char TOPIC[] = "led/0";

static const char META[] =
    "{"
        "\"dtype\": \"bool\","
        "\"brief\": \"Control LED 0.\","
        "\"default\": 0"
    "}";

static uint8_t on_led(void * user_data, const char * topic, const struct fbp_union_s * value) {
    (void) user_data;
    (void) topic;
    bool rv = false;
    if (0 == fbp_union_to_bool(value, &rv)) {
        FBP_LOGI("on_led(%s)", rv ? "on" : "off");
        if (rv) {
            led0_on();
        } else {
            led0_off();
        }
        return 0;
    } else {
        FBP_LOGW("on_led: invalid value");
    }
    return FBP_ERROR_PARAMETER_INVALID;
}

void led_service_initialize() {
    app_meta(TOPIC, META);
    app_subscribe(TOPIC, 0, on_led, NULL);
    app_publish(TOPIC, &fbp_union_u8_r(0), NULL, NULL);
}
