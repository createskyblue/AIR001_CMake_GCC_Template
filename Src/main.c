#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "app_drv_serial_rx.h"
#include "app_drv_fifo.h"

extern DMA_HandleTypeDef hdma_usart1_rx;
USART_DMA_Context USART1_DMA_Context;

// DMA发送状态标志
volatile uint8_t usart1_tx_busy = 0;
int __io_putchar(int ch)
{
  // 等待上一次DMA发送完成
  while (usart1_tx_busy != 0) {
    __NOP();
  }
  
  usart1_tx_busy = 1;
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  usart1_tx_busy = 0;
  return ch;
}

int _write(int file, char *ptr, int len)
{
  int i;
  for (i = 0; i < len; i++)
  {
    __io_putchar(*ptr++);
  }
  return len;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    usart1_tx_busy = 0;
  }
}

// 定义 FIFO 缓冲区大小
#define RX_FIFO_SIZE 256

// 用户自定义的 USART1 和 USART2 的 FIFO 缓冲区
static uint8_t usart1_rx_fifo_buffer[RX_FIFO_SIZE];

// FIFO 实例
static app_drv_fifo_t usart1_rx_fifo;

// 通用的批量队列写入函数（所有串口共用）
uint32_t USART_Queue_Write(void* user_queue, uint8_t* data, uint16_t length)
{
    uint16_t written = length;
    app_drv_fifo_result_t result = app_drv_fifo_write((app_drv_fifo_t*)user_queue, data, &written);
    if (result == APP_DRV_FIFO_RESULT_SUCCESS) {
        return written;
    }
    return 0;
}

// 通用的队列可用空间查询函数（所有串口共用）
uint32_t USART_Queue_Available(void* user_queue)
{
    return (uint32_t)(RX_FIFO_SIZE - app_drv_fifo_length((app_drv_fifo_t*)user_queue));
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/* 配置时钟源HSE/HSI/LSE/LSI */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                    /* 开启HSI */
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                    /* 不分频 */
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                           /* 配置HSI输出时钟为16MHz */
	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                                   /* 关闭HSE */
	RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                                               /* HSE工作频率范围16M~32M */
	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                                                   /* 关闭LSI */
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                                                   /* 关闭LSE */
	RCC_OscInitStruct.LSEDriver = RCC_ECSCR_LSE_DRIVER_1;                                       /* LSE默认驱动能力 */
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                                                /* 开启PLL */
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;                                        /* PLL的时钟源，频率必须要求12MHz及以上 */
	/* 初始化RCC振荡器 */
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/*初始化CPU,AHB,APB总线时钟*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; /* RCC系统时钟类型 */
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;                                      /* 配置PLL为系统时钟 */
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                             /* APH时钟不分频 */
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                              /* APB时钟不分频 */
	/* 初始化RCC系统时钟 */
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}
}


int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();

    // 初始化用户自定义的 FIFO 队列
    app_drv_fifo_init(&usart1_rx_fifo, usart1_rx_fifo_buffer, RX_FIFO_SIZE);
    
    // 初始化 USART DMA IDLE 接收
    USART_Rx_DMA_Init(&USART1_DMA_Context, &huart1, &hdma_usart1_rx);

    // 注册用户自定义队列指针和操作函数
    USART_RegisterQueueOps(&USART1_DMA_Context, &usart1_rx_fifo, USART_Queue_Write, USART_Queue_Available);
    
    while (1)
    {
        // LED闪烁(1Hz)
        static uint32_t led_timer = 0;
        if (HAL_GetTick() - led_timer >= 250) {
            led_timer = HAL_GetTick();
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
        }
        // 检查 USART1 FIFO 中是否有数据
        uint16_t usart1_len = app_drv_fifo_length(&usart1_rx_fifo);
        if (usart1_len > 0 && usart1_tx_busy == 0) {
            static uint8_t temp_buf[16];
            uint16_t read_len = (usart1_len > sizeof(temp_buf)) ? sizeof(temp_buf) : usart1_len;
            uint16_t actual_read = read_len;
            app_drv_fifo_result_t result = app_drv_fifo_read(&usart1_rx_fifo, temp_buf, &actual_read);
            if (result == APP_DRV_FIFO_RESULT_SUCCESS && actual_read > 0) {
                // 将接收到的数据回显到 USART1
                usart1_tx_busy = 1;
                HAL_UART_Transmit_DMA(&huart1, temp_buf, actual_read);
            }
        }
    }
}

void Error_Handler(void)
{
    while (1)
    {
    }
}
