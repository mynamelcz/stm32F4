#include "led.h"
#include "stm32f4xx_hal.h"





void LED_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE(); 

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_SET);

  /*Configure GPIO pin : PA6 */
  GPIO_InitStruct.Pin = LED1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED1_PORT, &GPIO_InitStruct);

  /*Configure GPIO pin : PA7 */
  GPIO_InitStruct.Pin = LED2_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_PORT, &GPIO_InitStruct);

}


