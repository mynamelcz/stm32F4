#include "bsp_init.h"

#include "bsp_spi.h"
#include "bsp_uart.h"
#include "bsp_sys.h"

#include "led.h"
#include "spi_flash.h"
#include "spi_sd.h"




static void gpio_clk_enable(void);

/** spi flash fun **/
static void flash_cs_gpio_init(void);
static void flash_cs_ctr(u8 en);
static void flash_bsp_init(void);
/** spi  sd  fun **/
static void spi_sd_cs_gpio_init(void);
static void spi_sd_cs_ctr(u8 en);
static void spi_sd_bsp_init(void);

void bsp_init(void)
{
	gpio_clk_enable();
	LED_GPIO_Init();
    uart1_init(); 
	
	flash_bsp_init();
	spi_sd_bsp_init();
}


/** ============  SPI FLASH  =========== **/
static void flash_bsp_init(void)
{
	flash_cs_gpio_init();
    spi1_obj.init(flash_cs_ctr);
	spi_flash_obj.init(&spi1_obj);
}

static void flash_cs_gpio_init(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = FLASH_CS_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(FLASH_CS_PORT, &GPIO_InitStruct);	
}
static void flash_cs_ctr(u8 en)
{
	en?LL_GPIO_SetOutputPin(FLASH_CS_PORT, FLASH_CS_PIN):\
	   LL_GPIO_ResetOutputPin(FLASH_CS_PORT, FLASH_CS_PIN);
}

/** ============  SPI SD   =========== **/
static void spi_sd_bsp_init(void)
{
	spi_sd_cs_gpio_init();
    spi2_obj.init(spi_sd_cs_ctr);
	spi_sd_obj.init(&spi2_obj);
}

static void spi_sd_cs_gpio_init(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SPI_SD_CS_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	LL_GPIO_Init(SPI_SD_CS_PORT, &GPIO_InitStruct);	
}
static void spi_sd_cs_ctr(u8 en)
{
	en?LL_GPIO_SetOutputPin(SPI_SD_CS_PORT, SPI_SD_CS_PIN):\
	   LL_GPIO_ResetOutputPin(SPI_SD_CS_PORT, SPI_SD_CS_PIN);
}



static void gpio_clk_enable(void)
{
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
}







