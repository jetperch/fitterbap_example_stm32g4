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
#include "fitterbap/logh.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create the singleton log handler instance.
 *
 * @return The log handler instance.
 *
 * Thread wrapper around "fitterbap/logh.h".  The thread calls
 * fbp_logh_process().  Connect it to backend log message
 * handlers using fbp_logh_dispatch_register().
 *
 * Call fbp_logh_publish() to publish messages.
 */
struct fbp_logh_s * log_handler_factory();

#ifdef __cplusplus
}
#endif

#endif  /* FBP_EXAMPLE_STM32G4_LOG_HANDLER_H__ */
