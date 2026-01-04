#include "main.h"
#include "dma.h"
#include "usart.h"
#include "usart_dma_idle.h"
#include "gpio.h"

extern DMA_HandleTypeDef hdma_usart1_rx;
USART_DMA_Context USART1_DMA_Context;
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
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
    USART_Rx_DMA_Init(&USART1_DMA_Context, &huart1, &hdma_usart1_rx);
    
    while (1)
    {
        // LED闪烁(1Hz)
        static uint32_t led_timer = 0;
        if (HAL_GetTick() - led_timer >= 250) {
            led_timer = HAL_GetTick();
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
        }
        // USART1 loopback - echo received data back
        uint8_t rx_data;
        if (USART_read(&USART1_DMA_Context, &rx_data)) {
            // Echo the received data back through USART1
            USART_write(&USART1_DMA_Context, &rx_data, 1);
        }
    }
}

void Error_Handler(void)
{
    while (1)
    {
    }
}
