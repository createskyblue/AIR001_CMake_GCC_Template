#ifndef USART_DMA_IDLE_H_
#define USART_DMA_IDLE_H_

#include "air001xx_hal.h"
#include "Queue.h"

#define USART_DMA_BUFFER_SIZE  (32)
#define USART_FIFO_BUFFER_SIZE  (256)

// USART DMA context structure
typedef struct {
    UART_HandleTypeDef* huart;
    DMA_HandleTypeDef* hdma;
    QUEUE queue;
    uint8_t dma_buffer[USART_DMA_BUFFER_SIZE];
    uint8_t queue_buffer[USART_FIFO_BUFFER_SIZE];
    uint32_t last_count;
} USART_DMA_Context;

void USART_Rx_DMA_Init(USART_DMA_Context* ctx, UART_HandleTypeDef* huart, DMA_HandleTypeDef* hdma);
void USART_Rx_DMA_IRQHandler_Process(USART_DMA_Context* ctx);

uint8_t USART_read(USART_DMA_Context* ctx, uint8_t* data);
HAL_StatusTypeDef USART_write(USART_DMA_Context* ctx, uint8_t* data, uint16_t size);
uint8_t USART_flush(USART_DMA_Context* ctx);
#endif /* USART_DMA_IDLE_H_ */