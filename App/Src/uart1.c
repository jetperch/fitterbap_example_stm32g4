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

#include "uart1.h"
#include "isr.h"
#include "main.h"
#include "fitterbap/assert.h"
#include "fitterbap/cdef.h"
#include "fitterbap/ec.h"
#include "fitterbap/time.h"
#include "fitterbap/collections/ring_buffer_u8.h"

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_gpio.h"


#define UART1_TASK_STACK (512)
#define UART1_TASK_PRIORITY ((osPriority_t) osPriorityNormal)
#define UART1_SERVICE_TIME_MAX_MS (500)
#define UART1_BAUDRATE (3000000)
#define UART1_RX_BUFFER_SIZE  (256)
#define UART1_TX_BUFFER_SIZE  ((270 + 16) * 2)


struct uart1_s {
    uart1_recv_fn recv_fn;
    void * recv_user_data;
    TaskHandle_t task;
    uint8_t rx_buffer[UART1_RX_BUFFER_SIZE];
    uint32_t rx_offset;
    uint8_t tx_buffer[UART1_TX_BUFFER_SIZE];
    uint32_t tx_dma_sz;
    struct fbp_rbu8_s tx_rbu8_;
    struct fbp_evm_s * evm;
    struct fbp_evm_api_s evm_api;
    fbp_os_mutex_t mutex;
};

enum events_e {
    EV_RECV = (1 << 0),
    EV_SEND = (1 << 1),
    EV_SEND_DONE = (1 << 2),
    EV_APP = (1 << 3),
};

static struct uart1_s self_;


static inline void lock() {
    fbp_os_mutex_lock(self_.mutex);
}

static inline void unlock() {
    fbp_os_mutex_unlock(self_.mutex);
}


static void gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = UART1_TX_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(UART1_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART1_RX_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;    // disable to save power
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(UART1_RX_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void) {
    LL_USART_InitTypeDef USART_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    gpio_init();

    /* USART1_RX DMA Init */
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_USART1_RX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_CIRCULAR);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t) &USART1->RDR);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t) self_.rx_buffer);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, FBP_ARRAY_SIZE(self_.rx_buffer));
    LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);

    /* USART1_TX DMA Init */
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMAMUX_REQ_USART1_TX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, 0); // for now
    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t) &USART1->TDR);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);

    /* USER CODE END USART1_Init 1 */
    USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
    USART_InitStruct.BaudRate = UART1_BAUDRATE;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART1, &USART_InitStruct);
    LL_USART_SetTXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_SetRXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_DisableFIFO(USART1);
    LL_USART_ConfigAsyncMode(USART1);
    LL_USART_EnableDMAReq_RX(USART1);
    LL_USART_EnableDMAReq_TX(USART1);
    LL_USART_SetRxTimeout(USART1, 1);
    LL_USART_EnableIT_IDLE(USART1);
    LL_USART_EnableIT_RTO(USART1);

    /* USART1 interrupt Init */
    NVIC_SetPriority(USART1_IRQn, ISR_UART1);

    /* RX DMA interrupt init */
    NVIC_SetPriority(DMA1_Channel1_IRQn, ISR_UART1_DMA_RX);

    /* TX DMA interrupt init */
    NVIC_SetPriority(DMA1_Channel2_IRQn, ISR_UART1_DMA_TX);
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_USART_Enable(USART1);

    /* Polling USART1 initialisation */
    while((!(LL_USART_IsActiveFlag_TEACK(USART1))) || (!(LL_USART_IsActiveFlag_REACK(USART1))))
    {
    }
}

static void tx_start(uint32_t min_size) {
    uint8_t * tail = fbp_rbu8_tail(&self_.tx_rbu8_);
    uint8_t * head = fbp_rbu8_head(&self_.tx_rbu8_);
    if (tail == head) {
        return; // empty, return
    }
    if (tail > head) {
        self_.tx_dma_sz = self_.tx_rbu8_.buf_size - self_.tx_rbu8_.tail;
    } else {
        uint32_t sz = head - tail;
        if (sz < min_size) {
            return;
        }
        self_.tx_dma_sz = sz;
    }
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t) tail);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, self_.tx_dma_sz);
    WRITE_REG(DMA1->IFCR, DMA_IFCR_CTEIF2 | DMA_IFCR_CHTIF2 | DMA_IFCR_CTCIF2);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
}

static void rx_process(void) {
    uint32_t pos = FBP_ARRAY_SIZE(self_.rx_buffer) - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_1);
    while (pos != self_.rx_offset) {
        if (pos > self_.rx_offset) {
            // data received, normal incrementing mode
            if (self_.recv_fn) {
                self_.recv_fn(self_.recv_user_data, &self_.rx_buffer[self_.rx_offset], pos - self_.rx_offset);
            }
            self_.rx_offset = pos;
        } else {
            // data received, but wrapped, process to end
            if (self_.recv_fn) {
                self_.recv_fn(self_.recv_user_data, &self_.rx_buffer[self_.rx_offset],
                              FBP_ARRAY_SIZE(self_.rx_buffer) - self_.rx_offset);
            }
            self_.rx_offset = 0;
        }
        if (self_.rx_offset >= FBP_ARRAY_SIZE(self_.rx_buffer)) {
            self_.rx_offset -= FBP_ARRAY_SIZE(self_.rx_buffer);
        }
    }
}

// RX DMA interrupt
void DMA1_Channel1_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Check half-transfer complete interrupt */
    if (LL_DMA_IsEnabledIT_HT(DMA1, LL_DMA_CHANNEL_1) && LL_DMA_IsActiveFlag_HT1(DMA1)) {
        LL_DMA_ClearFlag_HT1(DMA1);             /* Clear half-transfer complete flag */
        xTaskNotifyFromISR(self_.task, EV_RECV, eSetBits, &xHigherPriorityTaskWoken);
    }

    /* Check transfer-complete interrupt */
    if (LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_1) && LL_DMA_IsActiveFlag_TC1(DMA1)) {
        LL_DMA_ClearFlag_TC1(DMA1);             /* Clear transfer complete flag */
        xTaskNotifyFromISR(self_.task, EV_RECV, eSetBits, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void USART1_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* Check for IDLE line RX interrupt */
    if (LL_USART_IsEnabledIT_IDLE(USART1) && LL_USART_IsActiveFlag_IDLE(USART1)) {
        LL_USART_ClearFlag_IDLE(USART1);      /* Clear IDLE line flag */
        xTaskNotifyFromISR(self_.task, EV_RECV, eSetBits, &xHigherPriorityTaskWoken);
    }
    /* Check for IDLE line RX interrupt */
    if (LL_USART_IsEnabledIT_RTO(USART1) && LL_USART_IsActiveFlag_RTO(USART1)) {
        LL_USART_ClearFlag_RTO(USART1);      /* Clear IDLE line flag */
        xTaskNotifyFromISR(self_.task, EV_RECV, eSetBits, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

// TX DMA interrupt
void DMA1_Channel2_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_2) && LL_DMA_IsActiveFlag_TC2(DMA1)) {
        LL_DMA_ClearFlag_TC2(DMA1);             /* Clear transfer complete flag */
        xTaskNotifyFromISR(self_.task, EV_SEND_DONE, eSetBits, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void uart1_task(void *argument) {
    (void) argument;
    uint32_t notify;
    uint32_t duration_ms;
    int64_t now;
    int64_t duration;

    MX_USART1_UART_Init();

    while (1) {
        notify = 0;
        now = self_.evm_api.timestamp(self_.evm_api.evm);
        duration = fbp_evm_interval_next(self_.evm, now);
        duration_ms = FBP_TIME_TO_COUNTER(duration, 1000);
        if (duration_ms > UART1_SERVICE_TIME_MAX_MS) {
            duration_ms = UART1_SERVICE_TIME_MAX_MS;
        } else if (duration_ms <= 0) {
            duration_ms = 0;
        }
        if (pdTRUE == xTaskNotifyWait(0, 0xffffffff, &notify, duration_ms)) {
            lock();
            if (notify & EV_SEND_DONE) {
                fbp_rbu8_discard(&self_.tx_rbu8_, self_.tx_dma_sz);
                self_.tx_dma_sz = 0;
            }

            if (self_.tx_dma_sz == 0) {
                tx_start(UART1_TX_BUFFER_SIZE / 4);
            }

            if (notify & EV_RECV) {
                rx_process();
            }

            unlock();
        }

        now = self_.evm_api.timestamp(self_.evm_api.evm);
        fbp_evm_process(self_.evm, now);

        if (self_.tx_dma_sz == 0) {
            lock();
            tx_start(0);
            unlock();
        }

        // todo watchdog pet, regardless of data send/receive
    }
}

static void on_schedule(void * user_data, int64_t next_time) {
    (void) user_data;
    (void) next_time;
    if (self_.task) {
        xTaskNotify(self_.task, EV_APP, eSetBits);
    }
}

void uart1_initialize() {
    fbp_memset(&self_, 0, sizeof(self_));
    fbp_rbu8_init(&self_.tx_rbu8_, self_.tx_buffer, sizeof(self_.tx_buffer));

    self_.mutex = fbp_os_mutex_alloc();
    self_.evm = fbp_evm_allocate();
    FBP_ASSERT(0 == fbp_evm_api_get(self_.evm, &self_.evm_api));
    fbp_evm_register_mutex(self_.evm, self_.mutex);
    fbp_evm_register_schedule_callback(self_.evm, on_schedule, NULL);

    if (pdTRUE != xTaskCreate(
            uart1_task,             /* pvTaskCode */
            "uart1",                /* pcName */
            UART1_TASK_STACK,       /* usStackDepth in 32-bit words */
            NULL,                   /* pvParameters */
            UART1_TASK_PRIORITY,    /* uxPriority */
            &self_.task)) {
        FBP_FATAL("uart1 task");
    }
}

void uart1_evm_api(struct fbp_evm_api_s * api) {
    *api = self_.evm_api;
}

void uart1_recv_register(uart1_recv_fn recv_fn, void * recv_user_data) {
    lock();
    self_.recv_fn = NULL;
    self_.recv_user_data = recv_user_data;
    self_.recv_fn = recv_fn;
    unlock();
}

int32_t uart1_send(uint8_t const *buffer, uint32_t buffer_size) {
    int32_t rv = 0;
    lock();
    if (fbp_rbu8_add(&self_.tx_rbu8_, buffer, buffer_size)) {
        xTaskNotify(self_.task, EV_SEND, eSetBits);
    } else {
        rv = FBP_ERROR_NOT_ENOUGH_MEMORY;
    }
    unlock();
    return rv;
}

uint32_t uart1_send_available() {
    lock();
    uint32_t sz = fbp_rbu8_empty_size(&self_.tx_rbu8_);
    unlock();
    return sz;
}

void uart1_mutex(fbp_os_mutex_t * mutex) {
    *mutex = self_.mutex;
}
