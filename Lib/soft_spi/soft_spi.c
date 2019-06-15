#include "soft_spi.h"



static volatile u32 delay_cnt_g = 100;

static void soft_spi_delay(u32 delay)
{
	u32 tmp = delay;
	while(delay--);
	delay_cnt_g = tmp;
}

static u8 spi_tx_rx_byte(Soft_SPI_hd *hd, u8 s_dat)
{
	u8 i = 0;
	u8 r_dat = 0;
	if(hd->type == CPOL0_CPHA0){
		hd->CLK(0);
		soft_spi_delay(delay_cnt_g);
		for(;i<8;i++){		
			if(s_dat & (0x80 >> i)){
				hd->MOSI(1);
			}else{
				hd->MOSI(0);
			}
			soft_spi_delay(delay_cnt_g);
			hd->CLK(1);
			soft_spi_delay(delay_cnt_g);
			
			r_dat <<= 1;
			if(hd->MISO()) r_dat++;
			
			hd->CLK(0);	
			soft_spi_delay(delay_cnt_g);
		}
	}
	if(hd->type == CPOL0_CPHA1){
		hd->CLK(0);
		soft_spi_delay(delay_cnt_g);
		for(;i<8;i++){	
			hd->CLK(1);
			soft_spi_delay(delay_cnt_g);
			if(s_dat & (0x80 >> i)){
				hd->MOSI(1);
			}else{
				hd->MOSI(0);
			}
			soft_spi_delay(delay_cnt_g);	
			hd->CLK(0);				
			r_dat <<= 1;
			if(hd->MISO()) r_dat++;
			soft_spi_delay(delay_cnt_g);
		}
	}
	if(hd->type == CPOL1_CPHA0){
		hd->CLK(1);
		soft_spi_delay(delay_cnt_g);
		for(;i<8;i++){		
			if(s_dat & (0x80 >> i)){
				hd->MOSI(1);
			}else{
				hd->MOSI(0);
			}
			soft_spi_delay(delay_cnt_g);	
			hd->CLK(0);
			soft_spi_delay(delay_cnt_g);	
			r_dat <<= 1;
			if(hd->MISO()) r_dat++;
			hd->CLK(1);	
			soft_spi_delay(delay_cnt_g);
		}
	}
	if(hd->type == CPOL1_CPHA1){
		hd->CLK(1);
		soft_spi_delay(delay_cnt_g);
		for(;i<8;i++){	
			hd->CLK(0);	
			soft_spi_delay(delay_cnt_g);
			if(s_dat & (0x80 >> i)){
				hd->MOSI(1);
			}else{
				hd->MOSI(0);
			}
			soft_spi_delay(delay_cnt_g);
			hd->CLK(1);				
			r_dat <<= 1;
			if(hd->MISO()) r_dat++;	
			soft_spi_delay(delay_cnt_g);
		}	
	}
	return r_dat;
}

static void spi_send_buf(Soft_SPI_hd *hd, const u8 *buf, u32 len)
{
	ASSERT(buf);
	const u8 *ptr = buf;
	if(len == 0) return;
	while(len--){
		spi_tx_rx_byte(hd,*ptr++);
	}
}
static void spi_read_buf(Soft_SPI_hd *hd, u8 *buf, u32 len)
{
	ASSERT(buf);
	u8 *ptr = buf;
	if(len == 0) return;
	while(len--){
		*ptr++ = spi_tx_rx_byte(hd, 0xFF);
	}
}


Soft_SPI_IO_Type soft_spi_io = {
	 .w_r_byte	=   spi_tx_rx_byte,
	 .write		=   spi_send_buf,
	 .read		=	spi_read_buf,
};











