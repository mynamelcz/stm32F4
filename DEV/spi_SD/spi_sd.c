#include "spi_sd.h"
#include "bsp_spi.h"

static __spi_sd_obj *this_obj = NULL;
extern void HAL_Delay(u32 Delay);
#define spi_sd_delay_ms(x)		HAL_Delay(x)



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
	u8 res = SD_NO_ERR;

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
    
	sd_printf("SD CMD%d: ",Cmd);
	sd_puthex((const char *)cmd_fream,6);
	
	this_obj->hd_io->write(cmd_fream, 6);
	switch(Rx){
		case SD_SPI_R1:
			retry = 100; 
			do
			{
				this_obj->hd_io->read(r_buf, SD_SPI_R1_LEN);
			}while ((*r_buf & SPI_R1_ALWAYS_0) && --retry); 
			sd_dis_select();
			if(retry == 0){
				sd_printf("[ERR]: SD_SPI_R1 \n");
				return 0xff;
			}
			sd_printf("sd_cmd_R1: 0x%x\n", r_buf[0]);
			res = r_buf[0];
		case SD_SPI_R2:
			break;
		case SD_SPI_R3:
			 this_obj->hd_io->read(r_buf, SD_SPI_R3_LEN);
			 sd_dis_select();
		
		     sd_printf("sd_cmd_R3:");
			 sd_puthex((const char *)r_buf, SD_SPI_R3_LEN);
             res = r_buf[0];		
			break;
		case SD_SPI_R7:
			 this_obj->hd_io->read(r_buf, SD_SPI_R7_LEN);
		     sd_dis_select();
		     sd_printf("sd_cmd_R7:");
			 sd_puthex((const char *)r_buf, SD_SPI_R7_LEN);	
             res = r_buf[0];		
			break;
		default:
			break;
	}
	return res;
}


static u8 sd_cmd_r1(u8 Cmd, u32 Arg, u8 Crc)
{
	u8 retry = 0xf;
	u8 cmd_fream[6];
	u8 r1 = 0xff;

	this_obj->hd_io->cs_str(0);  
    do{  
		this_obj->hd_io->read(&r1, 1);
        if(r1 == 0xff)break;
    }while((--retry));
	if(retry == 0)
		return 0xff;
	
	
	cmd_fream[0] = (Cmd | 0x40); 
	cmd_fream[1] = (u8)(Arg >> 24); 
	cmd_fream[2] = (u8)(Arg >> 16); 
	cmd_fream[3] = (u8)(Arg >> 8);
	cmd_fream[4] = (u8)(Arg); 
	cmd_fream[5] = (Crc | 0x01); 
    
	sd_printf("SD CMD%d: ",Cmd);
	sd_puthex((const char *)cmd_fream,6);
	
	retry = 100; 
	this_obj->hd_io->write(cmd_fream, 6);
	do
	{	
		this_obj->hd_io->read(&r1, SD_SPI_R1_LEN);
	}while ((r1 & SPI_R1_ALWAYS_0) && --retry); 
	sd_dis_select();
	if(retry == 0){
		sd_printf("[ERR]: SD_SPI_R1 \n");
		return 0xff;
	}
	sd_printf("R1: 0x%x\n", r1);
    this_obj->hd_io->cs_str(1);  
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
	u8 r7_buf[SD_SPI_R7_LEN] = {0};
	u8 sd_tmp_buf[0x10] = {0};
	SD_Error res = SD_NO_ERR;
	
    this_obj = &spi_sd_obj;
	this_obj->hd_io = hd_io;
	this_obj->sd_inf.type = SD_TYPE_UNKNOW;	

	/**0: Reset card  >74  clk **/
	sd_speed_set(SD_CLK_SPEED_LOW);
    this_obj->hd_io->cs_str(1);	
	this_obj->hd_io->read(sd_tmp_buf, 10);
	/**1: CMD0--> idle mode **/
	retry = 10;
	do{
		r1 = sd_cmd_r1(SD_CMD_GO_IDLE_STATE, 0x00, 0x95);	
	}while((r1 != SPI_R1_IDLE_STATE)&&(--retry));
	if(retry == 0){
		sd_printf("[ERR]: SD_CMD_GO_IDLE_STATE \n");
		return SD_ERR_RES_FAILURE;
	}
	
	 this_obj->hd_io->read(r7_buf, 4);
	
while(1)
{
	r1 = sd_cmd_r1(SD_CMD_IF_COND, 0x01AA, 0x87);

	 this_obj->hd_io->read(r7_buf, 4);
     sd_printf("R7:");
	 sd_puthex((const char *)r7_buf,4);	
     spi_sd_delay_ms(500);
}

	
//	sd_cmd(SD_SPI_R7, SD_CMD_IF_COND, 0x01AA, 0x87, r7_buf);

	

	


	
	
	
	
	
	
}
//static SD_Error sd_init(__spi_ctr_obj  *hd_io)
//{
//	sd_printf("FUN:%s\n",__func__);
//	ASSERT(hd_io);
//	ASSERT(hd_io->read);
//	ASSERT(hd_io->write);
//	u8 retry = 0;
//	u8 r1 = 0;
//	u8 sd_tmp_buf[0x10] = {0};
//	SD_Error res = SD_NO_ERR;
//	
//    this_obj = &spi_sd_obj;
//	this_obj->hd_io = hd_io;
//	this_obj->sd_inf.type = SD_TYPE_UNKNOW;	

//	/**0: Reset card  >74  clk **/
//	sd_speed_set(SD_CLK_SPEED_LOW);
//	
//    this_obj->hd_io->cs_str(1);	
//	this_obj->hd_io->read(sd_tmp_buf, 10);
//	/**1: CMD0--> idle mode **/

//	
//	retry = 10;
//	do{			
//		sd_cmd(SD_SPI_R1, SD_CMD_GO_IDLE_STATE, 0x00, 0x95, &r1);
//	}while((r1 != SPI_R1_IDLE_STATE) && --retry);	

//	if(retry == 0){
//		sd_printf("[SD ERR]: line:%d\n",__LINE__);
//		res = SD_ERR_TIME_OUT;
//		return res;
//	}
//	/**2: CMD8   **/
//	u8 r7_buf[SD_SPI_R7_LEN] = {0};
//	sd_cmd(SD_SPI_R7, SD_CMD_IF_COND, 0x1AA, 0x87, r7_buf);//  check 2.7v~3.6v
//	if(r7_buf[0] & SPI_R1_ILL_CMD){
//	/**3: 不支持CMD8的卡   **/
//		
//		
//	}else if(r7_buf[0] == SPI_R1_IDLE_STATE){
//	/**4: 支持CMD8的卡    **/
//		if(((r7_buf[3]&0xf)==0x01) && (r7_buf[4] == 0xAA)){//check pattern and VOL

//			
//		}else{
//    /**3: ACMD58   **/
//			u8 r3_buf[SD_SPI_R3_LEN] = {0};
//			sd_cmd(SD_SPI_R3, SD_CMD_READ_OCR, 0x00, 0xff, r3_buf);
//			if(r1 != SPI_R1_IDLE_STATE){
//				sd_printf("[ERR]: line:%d \n",__LINE__);
//				return r1;
//			}
//			while(1){
//				sd_cmd(SD_SPI_R1, SD_CMD_APP_CMD, 0x00, 0xff, &r1);
//			sd_cmd(SD_SPI_R1, SD_ACMD_SEND_OP_COND, 0, 0xff, &r1);
//				spi_sd_delay_ms(500);
//			}
//		    
//			
//		
//		}
//	
//	
//	}
//	
//	while(1);

//	
//	
//	
//	
//	
//	
//}


__spi_sd_obj spi_sd_obj = {

	.init = sd_init,

};
 
