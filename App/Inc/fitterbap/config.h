/*
 * Copyright 2014-2021 Jetperch LLC
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

/**
 * @file
 *
 * @brief FBP configuration.
 */

#ifndef FBP_CONFIG_H_
#define FBP_CONFIG_H_

/**
 * @ingroup fbp
 * @defgroup fbp_config Configuration
 *
 * @brief FBP configuration.
 *
 * @{
 */


#include "FreeRTOS.h"
#include "semphr.h"


/* Set global log level */
#define FBP_LOG_GLOBAL_LEVEL FBP_LOG_LEVEL_ALL

/* Set the initial comm log port level for forwarding/receiving log messages. */
#define FBP_LOGP_LEVEL FBP_LOG_LEVEL_INFO

/* Override the log format */
struct fbp_logh_s;
int32_t fbp_logh_publish(struct fbp_logh_s * self, uint8_t level, const char * filename, uint32_t line, const char * format, ...);
#define FBP_LOG_PRINTF(level, format, ...) \
    fbp_logh_publish(NULL, level, __FILENAME__, __LINE__, format, __VA_ARGS__)

#define FBP_FRAMER_CRC32 fbp_crc32
#define FBP_CRC_CRC32 1

#define FBP_PLATFORM_STDLIB 1
#define FBP_CSTR_FLOAT_ENABLE 0
#define fbp_os_mutex_t SemaphoreHandle_t

/** @} */

#endif /* FBP_CONFIG_H_ */

