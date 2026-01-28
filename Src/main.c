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
    app_drv_fifo_t* fifo = (app_drv_fifo_t*)user_queue;
    // 从 FIFO 结构体中读取 size，避免硬编码
    return (uint32_t)(fifo->size - app_drv_fifo_length(fifo));
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

    // ========================================
    // ARM M0 汇编示例代码
    // ========================================
    printf("\r\n======== ARM M0 Assembly Examples ========\r\n");

    // 示例1: 基本的数据传送 (MOV)
    printf("\r\n--- Example 1: Data Transfer (MOV) ---\r\n");
    uint32_t val1, val2;
    val1 = 10;
    val2 = 0;
    __asm volatile (
        "mov %0, %1       \n" // 将 val1 的值移动到 val2
        : "=l" (val2)      // 输出操作数，l=低寄存器(r0-r7)
        : "l" (val1)       // 输入操作数
    );
    printf("MOV: val1=%lu, val2=%lu\r\n", val1, val2);

    // 示例2: 加法运算 (ADD)
    printf("\r\n--- Example 2: Addition (ADD) ---\r\n");
    uint32_t a = 15, b = 27, sum = 0;
    __asm volatile (
        "add %0, %1, %2   \n" // 加法: sum = a + b
        : "=l" (sum)       // l=低寄存器(r0-r7)
        : "l" (a), "l" (b)
    );
    printf("ADD: %lu + %lu = %lu\r\n", a, b, sum);

    // 示例3: 减法运算 (SUB)
    printf("\r\n--- Example 3: Subtraction (SUB) ---\r\n");
    uint32_t x = 100, y = 37, diff = 0;
    __asm volatile (
        "sub %0, %1, %2   \n" // 减法: diff = x - y
        : "=l" (diff)
        : "l" (x), "l" (y)
    );
    printf("SUB: %lu - %lu = %lu\r\n", x, y, diff);

    // 示例4: 乘法运算 (MUL)
    printf("\r\n--- Example 4: Multiplication (MUL) ---\r\n");
    uint32_t m1 = 12, m2 = 8, product = 0;
    __asm volatile (
        "mul %0, %1, %2   \n" // 乘法: product = m1 * m2
        : "=l" (product)
        : "l" (m1), "l" (m2)
    );
    printf("MUL: %lu * %lu = %lu\r\n", m1, m2, product);

    // 示例5: 左移位操作 (LSL)
    printf("\r\n--- Example 5: Left Shift (LSL) ---\r\n");
    uint32_t shift_val = 5, shifted = 0;
    __asm volatile (
        "lsl %0, %1, #2   \n" // 左移2位: shifted = shift_val << 2
        : "=l" (shifted)
        : "l" (shift_val)
    );
    printf("LSL: %lu << 2 = %lu\r\n", shift_val, shifted);

    // 示例6: 右移位操作 (LSR)
    printf("\r\n--- Example 6: Right Shift (LSR) ---\r\n");
    uint32_t rshift_val = 32, rshifted = 0;
    __asm volatile (
        "lsr %0, %1, #3   \n" // 右移3位: rshifted = rshift_val >> 3
        : "=l" (rshifted)
        : "l" (rshift_val)
    );
    printf("LSR: %lu >> 3 = %lu\r\n", rshift_val, rshifted);

    // 示例7: 位与操作 (AND)
    printf("\r\n--- Example 7: Bitwise AND (AND) ---\r\n");
    uint32_t and1 = 0xFF, and2 = 0x0F, and_result = 0;
    __asm volatile (
        "mov %0, %1       \n" // 先移动到目标寄存器
        "and %0, %2       \n" // 位与: and_result = and1 & and2 (双操作数形式)
        : "=&l" (and_result)  // & 表示早期clobber，避免与输入共享
        : "l" (and1), "l" (and2)
    );
    printf("AND: 0x%02lX & 0x%02lX = 0x%02lX\r\n", and1, and2, and_result);

    // 示例8: 位或操作 (ORR)
    printf("\r\n--- Example 8: Bitwise OR (ORR) ---\r\n");
    uint32_t or1 = 0xF0, or2 = 0x0F, or_result = 0;
    __asm volatile (
        "mov %0, %1       \n" // 先移动到目标寄存器
        "orr %0, %2       \n" // 位或: or_result = or1 | or2 (双操作数形式)
        : "=&l" (or_result)
        : "l" (or1), "l" (or2)
    );
    printf("ORR: 0x%02lX | 0x%02lX = 0x%02lX\r\n", or1, or2, or_result);

    // 示例9: 位异或操作 (EOR)
    printf("\r\n--- Example 9: Bitwise XOR (EOR) ---\r\n");
    uint32_t eor1 = 0xAA, eor2 = 0x55, eor_result = 0;
    __asm volatile (
        "mov %0, %1       \n" // 先移动到目标寄存器
        "eor %0, %2       \n" // 位异或: eor_result = eor1 ^ eor2 (双操作数形式)
        : "=&l" (eor_result)
        : "l" (eor1), "l" (eor2)
    );
    printf("EOR: 0x%02lX ^ 0x%02lX = 0x%02lX\r\n", eor1, eor2, eor_result);

    // 示例10: 位取反操作 (MVN)
    printf("\r\n--- Example 10: Bitwise NOT (MVN) ---\r\n");
    uint32_t mvn_in = 0x55, mvn_out = 0;
    __asm volatile (
        "mvn %0, %1       \n" // 位取反: mvn_out = ~mvn_in
        : "=l" (mvn_out)
        : "l" (mvn_in)
    );
    printf("MVN: ~0x%02lX = 0x%02lX\r\n", mvn_in, (uint32_t)(mvn_out & 0xFF));

    // 示例11: 比较和条件分支 (CMP + BGE)
    printf("\r\n--- Example 11: Compare and Branch ---\r\n");
    int32_t cmp_a = 50, cmp_b = 30, max_val = 0;
    __asm volatile (
        "cmp %1, %2       \n" // 比较 cmp_a 和 cmp_b
        "bge 1f           \n" // 如果 cmp_a >= cmp_b，跳转到标签1
        "mov %0, %2       \n" // 否则 max_val = cmp_b
        "b   2f           \n" // 跳转到标签2
        "1:               \n" // 标签1
        "mov %0, %1       \n" // max_val = cmp_a
        "2:               \n" // 标签2
        : "=l" (max_val)
        : "l" (cmp_a), "l" (cmp_b)
    );
    printf("CMP: max(%ld, %ld) = %ld\r\n", cmp_a, cmp_b, max_val);

    // 示例12: 自增操作
    printf("\r\n--- Example 12: Increment (ADD) ---\r\n");
    uint32_t counter = 0;
    __asm volatile (
        "mov %0, #0       \n" // counter = 0
        "add %0, %0, #1   \n" // counter++
        "add %0, %0, #1   \n" // counter++
        : "=l" (counter)
    );
    printf("Counter after 2 increments: %lu\r\n", counter);

    // 示例13: 使用寄存器加载立即数 (LDR)
    printf("\r\n--- Example 13: Load Immediate (LDR) ---\r\n");
    uint32_t ldr_val = 0;
    __asm volatile (
        "ldr %0, =12345   \n" // 加载大立即数到寄存器
        : "=l" (ldr_val)
    );
    printf("LDR: loaded value = %lu\r\n", ldr_val);

    // 示例14: 内存加载和存储 (LDR/STR)
    printf("\r\n--- Example 14: Load/Store Memory ---\r\n");
    uint32_t mem_src = 0xDEADBEEF, mem_dest = 0;
    __asm volatile (
        "ldr %0, %1       \n" // 从内存加载: mem_dest = mem_src
        : "=l" (mem_dest)
        : "m" (mem_src)
    );
    printf("LDR/STR: loaded 0x%08lX from memory\r\n", mem_dest);

    printf("\r\n======== End of Assembly Examples ========\r\n\r\n");

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
