#include "spi_drive.h"
#include "soft_spi.h"


Soft_SPI_hd soft_spi1_hd;

static u8 soft_spi1_miso(void)
{

}
static void soft_spi1_mosi(u8 en)
{

}
static void soft_spi1_clk(u8 en)
{

}
 


static u32 soft_spi1_init(void(*CallBack)(void *drv_hd))
{
	soft_spi1_hd.type = CPOL0_CPHA0;
	soft_spi1_hd.CLK = soft_spi1_clk;
	soft_spi1_hd.MISO= soft_spi1_miso;
	soft_spi1_hd.MOSI= soft_spi1_mosi;	
	
	
	
	
}

static void soft_spi1_cs(u8 en)
{

}
static u32 soft_spi1_read(void *buf, u32 len)
{

}
static u32 soft_spi1_write(const void *buf, u32 len)
{

}










SPI_DrvTypDef SOFT_SPI1_DRV = {
	
	.Initialize 	= soft_spi1_init,
	.Uninitialize 	= NULL,
	.CS				= soft_spi1_cs,
	.Send			= soft_spi1_write,
	.Receive		= soft_spi1_read,
	
};

















