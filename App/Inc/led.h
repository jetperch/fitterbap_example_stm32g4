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

#ifndef FBP_EXAMPLE_STM32G4_LED_H__
#define FBP_EXAMPLE_STM32G4_LED_H__

#include "main.h"
#include "stm32g4xx_ll_gpio.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void led0_toggle() {
    LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

static inline void led0_on() {
    LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
}

static inline void led0_off() {
    LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
}


#ifdef __cplusplus
}
#endif

#endif  /* FBP_EXAMPLE_STM32G4_LED_H__ */
