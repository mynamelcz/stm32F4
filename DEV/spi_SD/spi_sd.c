#include "spi_sd.h"
#include "bsp_spi.h"

static __spi_sd_obj *this_obj = NULL;




u8 sd_detect(void)
{
	u8 status = SD_ON_LINE;

	return status;
}


static SD_Error sd_wait_ready(void)  
{  
	SD_Error res = SD_NO_ERR;
    u32 t=0;  
	u8 dat = 0;
    do{  
		this_obj->hd_io->read(&dat, 1);
        if(dat == 0xff)return res;
        t++; 
		sd_printf("sd wait:0x%x t:%d\n",dat, t);
    }while(t<0X0F);
    res = SD_ERR_TIME_OUT;	
	sd_printf("[SD ERR]:SD_ERR_TIME_OUT\n");
    return res;  
}  
  

static void sd_dis_select(void)  
{  
	u8 dat = 0;
    this_obj->hd_io->cs_str(1); 
    this_obj->hd_io->read(&dat, 1);
}  
  
static SD_Error sd_select(void)  
{  
	SD_Error res = SD_NO_ERR;
    this_obj->hd_io->cs_str(0);   
    res = sd_wait_ready();
	if(res != SD_NO_ERR)
		sd_dis_select();  
    return res; 
}  

static u8 sd_cmd(u8 Rx, u8 Cmd, u32 Arg, u8 Crc, u8 *r_buf)
{
	u8 retry;
	u8 cmd_fream[6];
	
	SD_Error res = SD_NO_ERR;
	sd_dis_select();
	res = sd_select();
	if(res != SD_NO_ERR){
		sd_printf("[SD ERR] line: %d\n",__LINE__);
		return 0xff;
	}

	cmd_fream[0] = (Cmd | 0x40); 
	cmd_fream[1] = (u8)(Arg >> 24); 
	cmd_fream[2] = (u8)(Arg >> 16); 
	cmd_fream[3] = (u8)(Arg >> 8);
	cmd_fream[4] = (u8)(Arg); 
	cmd_fream[5] = (Crc | 0x01); 
    
	sd_printf("SD CMD:");
	sd_puthex((const char *)cmd_fream,6);
	this_obj->hd_io->write(cmd_fream, 6);
	
	switch(Rx){
		case SD_SPI_R1:
			retry = 10; 
			do
			{
				this_obj->hd_io->read(r_buf, 1);
			}while ((*r_buf & SPI_R1_ALWAYS_0) && --retry); 
			if(retry == 0){
				return 0xff;
			}
			return *r_buf;
		case SD_SPI_R2:
			break;
		case SD_SPI_R3:
			break;
		case SD_SPI_R7:
			 this_obj->hd_io->read(r_buf, SD_SPI_R7_LEN);
			break;
		default:
			break;
	}
	return res;
}

static u8 sd_cmd_R1(u8 Cmd, u32 Arg, u8 Crc)
{
	u8 r1 = 0xff;
	sd_cmd(SD_SPI_R1, Cmd, Arg, Crc, &r1);
	sd_printf("sd_cmd_R1: 0x%x\n",r1);
	return r1;
}


static u8 sd_cmd_R7(u8 Cmd, u32 Arg, u8 Crc)
{
	u8 r1 = 0xff;
	sd_cmd(SD_SPI_R7, Cmd, Arg, Crc, &r1);
	sd_printf("sd_cmd_R7: 0x%x\n",r1);
	return r1;
}

static u8 sd_speed_set(u8 speed)
{
	u8 res  = 0;
	if((speed == SD_CLK_SPEED_HIGH)||(speed == SD_CLK_SPEED_LOW))
		res = this_obj->hd_io->io_ctr(SD_CLK_SPEED_SET, (void *)&speed);	
	return res;
}
static void sd_write_buf(const u8 *buf, u32 addr, u32 len)
{
	ASSERT(buf);

}

static void sd_read_buf(u8 *buf, u32 addr, u32 len)
{
	ASSERT(buf);

}

static SD_Error sd_init(__spi_ctr_obj  *hd_io)
{
	sd_printf("FUN:%s\n",__func__);
	ASSERT(hd_io);
	ASSERT(hd_io->read);
	ASSERT(hd_io->write);
	u8 retry = 0;
	u8 r1 = 0;
	u8 sd_tmp_buf[0x10] = {0};
	SD_Error res = SD_NO_ERR;
	
    this_obj = &spi_sd_obj;
	this_obj->hd_io = hd_io;
	this_obj->sd_inf.type = SD_TYPE_UNKNOW;	
	
	
	sd_speed_set(SD_CLK_SPEED_HIGH);			// 0:  clk low			
	this_obj->hd_io->read(sd_tmp_buf, 10);		// 1:  >74  clk
	retry = 10;
	do{											// 2:  idle mode
		r1 = sd_cmd_R1(SD_CMD_GO_IDLE_STATE, 0x00, 0x95);
	}while((r1 != SPI_R1_IDLE_STATE) && --retry);
	
	
	
	while(1);
	if(retry == 0){
		res = SD_ERR_TIME_OUT;
		return res;
	}
	
	
	
	
	
	
}


__spi_sd_obj spi_sd_obj = {

	.init = sd_init,

};
 
