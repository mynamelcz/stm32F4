#ifndef __LED_H
#define __LED_H


#define LED1_PORT	GPIOA
#define LED1_PIN	LL_GPIO_PIN_7

#define LED2_PORT	GPIOA
#define LED2_PIN	LL_GPIO_PIN_6

#define LED1_ON()	LL_GPIO_ResetOutputPin(LED1_PORT, LED1_PIN)
#define LED1_OFF()	LL_GPIO_SetOutputPin(LED1_PORT, LED1_PIN)
#define LED1_REV()	LL_GPIO_TogglePin(LED1_PORT, LED1_PIN)

#define LED2_ON()	LL_GPIO_ResetOutputPin(LED2_PORT, LED2_PIN)
#define LED2_OFF()	LL_GPIO_SetOutputPin(LED2_PORT, LED2_PIN)
#define LED2_REV()	LL_GPIO_TogglePin(LED2_PORT, LED2_PIN)

void LED_GPIO_Init(void);

#endif



