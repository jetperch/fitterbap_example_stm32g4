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

#ifndef FBP_EXAMPLE_STM32G4_ISR_H__
#define FBP_EXAMPLE_STM32G4_ISR_H__

#include "fitterbap/cdef.h"
#include "FreeRTOSConfig.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Configuring interrupt priority levels on the Cortex-M with FreeRTOS
 * can be confusing.  Read these:
 *
 * https://www.freertos.org/RTOS-Cortex-M3-M4.html
 * https://community.arm.com/developer/ip-products/system/b/embedded-blog/posts/cutting-through-the-confusion-with-arm-cortex-m-interrupt-priorities
 *
 * The STM32G4 has 4 bits (16 priority levels), and we only use preempt
 * priorities 5 (most urgent) through 15 (least urgent).  We do not
 * use any subpriority bits.
 */

// NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0)

// FBP_STATIC_ASSERT(4 == __NVIC_PRIO_BITS, isr_priority_bits);
FBP_STATIC_ASSERT(5 == configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, isr_most_urgent_priority);
FBP_STATIC_ASSERT(15 == configLIBRARY_LOWEST_INTERRUPT_PRIORITY, isr_least_urgent_priority);

#define ISR_UART1_DMA_RX    (10)
#define ISR_UART1_DMA_TX    (11)
#define ISR_UART1           (15)  // rx timeout

#define ISR_UART2_DMA_RX    (10)
#define ISR_UART2_DMA_TX    (11)
#define ISR_UART2           (15)  // rx timeout

#define ISR_UART3_DMA_RX    (10)
#define ISR_UART3_DMA_TX    (11)
#define ISR_UART3           (15)  // rx timeout

#define ISR_UART4_DMA_RX    (10)
#define ISR_UART4_DMA_TX    (11)
#define ISR_UART4           (15)  // rx timeout

#define ISR_UART5_DMA_RX    (10)
#define ISR_UART5_DMA_TX    (11)
#define ISR_UART5           (15)  // rx timeout


#ifdef __cplusplus
}
#endif

#endif  /* FBP_EXAMPLE_STM32G4_ISR_H__ */
