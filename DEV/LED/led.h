#ifndef __LED_H
#define __LED_H
#include "stm32f4xx_hal.h"

#define LED1_PORT	GPIOA
#define LED1_PIN	GPIO_PIN_6

#define LED2_PORT	GPIOA
#define LED2_PIN	GPIO_PIN_7

#define LED1_ON()	HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET)
#define LED1_OFF()	HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET)
#define LED1_REV()	HAL_GPIO_TogglePin(LED1_PORT, LED1_PIN)

#define LED2_ON()	HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_RESET)
#define LED2_OFF()	HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_SET)
#define LED2_REV()	HAL_GPIO_TogglePin(LED2_PORT, LED2_PIN)

void LED_GPIO_Init(void);

#endif



