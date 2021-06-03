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

#ifndef APP_STM32G4_UART3_H__
#define APP_STM32G4_UART3_H__

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "fitterbap/event_manager.h"
#include "fitterbap/os/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The function called when the UART receives data.
 *
 * @param user_data The arbitrary data.
 * @param buffer The received data.
 * @param buffer_size The size of buffer in bytes.
 */
typedef void (*uart3_recv_fn)(void *user_data, uint8_t *buffer, uint32_t buffer_size);

/**
 * @brief Initialize the UART and thread.
 */
void uart3_initialize();

/**
 * @brief Populate the event manager API.
 *
 * @param self The UART instance.
 * @param api[out] The API instance populated with the event manager instance
 *      and callbacks running on the UART thread.
 *
 * All events for processing on this thread MUST be posted to this
 * event manager.
 */
void uart3_evm_api(struct fbp_evm_api_s * api);

/**
 * @brief Set the function called when the UART receives data.
 *
 * @param recv_fn The function to call with received data.
 * @param recv_user_data The arbitrary data for recv_fn.
 */
void uart3_recv_register(uart3_recv_fn recv_fn, void * recv_user_data);

/**
 * @brief Transmit data out the UART.
 *
 * @param buffer The data to transmit.
 * @param buffer_size The size of buffer in bytes.
 * @return 0 or FBP_ERROR_NOT_ENOUGH_MEMORY.
 */
int32_t uart3_send(uint8_t const *buffer, uint32_t buffer_size);

/**
 * @brief Get the amount of space available in the transmit buffer.
 *
 * @return The available transmit buffer, in bytes.
 *
 * Calling this function and respecting buffer_size can
 * ensure that the subsequent call to uart3_send succeeds.
 */
uint32_t uart3_send_available();

/**
 * @brief Get the mutex for accessing this UART thread.
 *
 * @param mutex[out] The mutex.
 */
void uart3_mutex(fbp_os_mutex_t * mutex);

#ifdef __cplusplus
}
#endif

#endif  /* APP_STM32G4_UART3_H__ */
