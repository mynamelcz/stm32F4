#include "led.h"
#include "stm32f4xx_ll.h"

void LED_GPIO_Init(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
		

	LL_GPIO_SetOutputPin(LED1_PORT, LED1_PIN);
	
	GPIO_InitStruct.Pin = LED1_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(LED1_PORT, &GPIO_InitStruct);	
	
	LL_GPIO_SetOutputPin(LED2_PORT, LED2_PIN);
	GPIO_InitStruct.Pin = LED2_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(LED2_PORT, &GPIO_InitStruct);	
}


