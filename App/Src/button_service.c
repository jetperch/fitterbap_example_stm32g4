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

#include "button_service.h"
#include "app_comms.h"
#include "fitterbap/log.h"
#include "main.h"
#include "stm32g4xx_ll_gpio.h"
#include <stdbool.h>

static const char TOPIC[] = "button/0";
static uint32_t button_value_ = 0;


static const char META[] =
    "{"
        "\"dtype\": \"bool\","
        "\"brief\": \"Button 0.\","
        "\"default\": 0,"
        "\"flags\": [\"ro\"]"
    "}";


void button_service_initialize() {
    app_meta(TOPIC, META);
}

void button_service_poll() {
    uint32_t now = LL_GPIO_IsInputPinSet(B1_GPIO_Port, B1_Pin);
    if (now != button_value_) {
        button_value_ = now;
        FBP_LOGI("on_button(%s)", button_value_ ? "on" : "off");
        app_publish(TOPIC, &fbp_union_u8_r(now), NULL, NULL);
    }
}

