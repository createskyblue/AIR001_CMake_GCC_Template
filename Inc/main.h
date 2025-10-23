/**
  ******************************************************************************
  * @file    main.h
  * @author  MCU Application Team
  * @brief   Header for main.c file.
  *          This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) AirM2M.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "air001xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define DALI_RX_Pin GPIO_PIN_0
#define DALI_RX_GPIO_Port GPIOA
#define DALI_RX_EXTI_IRQn EXTI0_1_IRQn
#define DALI_TX_Pin GPIO_PIN_1
#define DALI_TX_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_1
#define LED_GPIO_Port GPIOB
#define RX_IRQ_Pin GPIO_PIN_2
#define RX_IRQ_GPIO_Port GPIOA

typedef enum
{
  MODBUS_HOLDING_REG_IS_MILLIS_0 = 0,
  MODBUS_HOLDING_REG_IS_MILLIS_1,
  MODBUS_HOLDING_REG_IS_CONTROLER,
  MODBUS_HOLDING_REG_TX_MSG_BOX_0,
  MODBUS_HOLDING_REG_TX_MSG_BOX_1,
  MODBUS_HOLDING_REG_TX_EN,
  MODBUS_HOLDING_REG_RX_LEN,
  MODBUS_HOLDING_REG_RX_MSG_BOX_0,
  MODBUS_HOLDING_REG_RX_MSG_BOX_1,
  MODBUS_HOLDING_REG_DEMO_PHONE_0,
  MODBUS_HOLDING_REG_DEMO_PHONE_1,
  MODBUS_HOLDING_REG_DEMO_PHONE_2,
};

/* Exported variables prototypes ---------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

