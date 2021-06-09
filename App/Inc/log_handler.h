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

#ifndef FBP_EXAMPLE_STM32G4_LOG_HANDLER_H__
#define FBP_EXAMPLE_STM32G4_LOG_HANDLER_H__

#include <stdint.h>
#include "fitterbap/comm/log_port.h"


#ifdef __cplusplus
extern "C" {
#endif

extern struct fbp_port_api_s * log_handler_api;

/**
 * @brief Create the (normally singleton) log handler instance.
 *
 * @param topic The log topic prefix, such as "a/log/"
 * @return The log handler instance.
 *
 * The log handler will not forward messages until connected
 * to the transport and fbp_port_api_s->transport is set.
 */
struct fbp_port_api_s * log_handler_factory(const char * topic);

#ifdef __cplusplus
}
#endif

#endif  /* FBP_EXAMPLE_STM32G4_LOG_HANDLER_H__ */
