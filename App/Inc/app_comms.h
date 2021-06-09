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

#ifndef FBP_EXAMPLE_STM32G4_APP_COMMS_H__
#define FBP_EXAMPLE_STM32G4_APP_COMMS_H__

#include <stdint.h>
#include "fitterbap/pubsub.h"


#ifdef __cplusplus
extern "C" {
#endif

// The singleton pubsub instance for this microcontroller.
extern struct fbp_pubsub_s * pubsub;

/// Wrappers for fbp_pubsub_*, with without pubsub topic_prefix
int32_t app_subscribe(const char * topic, uint8_t flags, fbp_pubsub_subscribe_fn cbk_fn, void * cbk_user_data);
int32_t app_unsubscribe(const char * topic, fbp_pubsub_subscribe_fn cbk_fn, void * cbk_user_data);
int32_t app_publish(const char * topic, const struct fbp_union_s * value,
                    fbp_pubsub_subscribe_fn src_fn, void * src_user_data);
int32_t app_meta(const char * topic, const char * meta_json);
int32_t app_query(const char * topic, struct fbp_union_s * value);


/**
 * @brief Construct and initialize the pubsub instance.
 *
 * @return 0 or error code.
 */
void app_pubsub_initialize();

/**
 * @brief Construct and initialize the comlinks.
 *
 * @return 0 or error code.
 */
void app_comms_initialize();


#ifdef __cplusplus
}
#endif

#endif  /* FBP_EXAMPLE_STM32G4_APP_COMMS_H__ */
