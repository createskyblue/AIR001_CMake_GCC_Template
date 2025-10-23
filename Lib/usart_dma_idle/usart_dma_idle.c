#include "usart_dma_idle.h"
#include "usart.h"
void USART_Rx_DMA_Init(USART_Rx_DMA_Context* ctx, UART_HandleTypeDef* huart, DMA_HandleTypeDef* hdma)
{
    // Initialize context
    ctx->huart = huart;
    ctx->hdma = hdma;
    ctx->last_count = 0;
    
    // Create message queue
    createQueue(&ctx->queue, USART_DMA_BUFFER_SIZE+1, ctx->queue_buffer);
    
    // Configure USART idle interrupt
    __HAL_UART_CLEAR_IDLEFLAG(ctx->huart);
    __HAL_UART_ENABLE_IT(ctx->huart, UART_IT_IDLE);
    
    // Start UART DMA reception in circular mode
    HAL_UART_Receive_DMA(ctx->huart, ctx->dma_buffer, USART_DMA_BUFFER_SIZE);
}

void USART_Rx_DMA_IRQHandler_Process(USART_Rx_DMA_Context* ctx)
{
    // Process USART idle interrupt
    if (RESET != __HAL_UART_GET_FLAG(ctx->huart, UART_FLAG_IDLE))
    {
        // Get current buffer index and calculate received data length
        uint32_t thisCount = USART_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(ctx->hdma);

        __HAL_UART_CLEAR_IDLEFLAG(ctx->huart);

        while (ctx->last_count != thisCount) {
            if (!enqueue(&ctx->queue, ctx->dma_buffer[ctx->last_count])) {
                break; // Queue full
            }
            ctx->last_count = (ctx->last_count + 1) % USART_DMA_BUFFER_SIZE;
        }
    }
}

// Generic user interface functions
uint8_t USART_read(USART_Rx_DMA_Context* ctx, uint8_t* data)
{
    return dequeue(&ctx->queue, data);
}

HAL_StatusTypeDef USART_write(USART_Rx_DMA_Context* ctx, uint8_t* data, uint16_t size)
{
    return HAL_UART_Transmit(ctx->huart, data, size, 0xFFFF);
}

uint8_t USART_flush(USART_Rx_DMA_Context* ctx)
{
    uint8_t buf;
    while (USART_read(ctx, &buf));
    return 0;
}
