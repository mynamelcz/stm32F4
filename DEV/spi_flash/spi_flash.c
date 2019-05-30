#include "spi_flash.h"
#include "bsp_spi.h"
#include "diskio.h"
#include "stm32f4xx_ll.h"
#include "stm32f4xx_hal.h"

#define DUMMY_DATA	0xFF

static void spi_cs_gpio_init(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SPI_CS_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(SPI_CS_PORT, &GPIO_InitStruct);	
}




/******	 读取 ID	******/
u16 flash_read_id(void)
{
	u8 cmd_buf[4]={FLASH_READ_DEVICE_ID, DUMMY_DATA, DUMMY_DATA, DUMMY_DATA};
	u16 dev_id = 0;	  
	spi_cs_ctr(0);	
	spi_write_buf(cmd_buf, 4);
	spi_read_buf((u8 *)(&dev_id),2);
	spi_cs_ctr(1);	
 	spi_printf("FLSH ID: 0x%x\n",dev_id);
	return dev_id;
}  
u32 flash_read_jedec(void)
{
	u32 dev_id = 0;
	u8 cmd_buf[1]={FLASH_READ_JEDEC_ID};
	u8 buf[3];	  
	spi_cs_ctr(0);	 
	spi_write_buf(cmd_buf, 1);
	spi_read_buf(buf,3);
	spi_cs_ctr(1);
    dev_id = (buf[0]<<16) + (buf[1]<<8) + buf[2];	
 	spi_printf("FLSH JEDEC: 0x%x\n",dev_id);
	return dev_id;
}
void flash_read_unique_id(u8 *id_buf)
{
	u8 cmd_buf[5]={FLASH_READ_UNIQUE_ID,DUMMY_DATA, DUMMY_DATA,DUMMY_DATA,DUMMY_DATA};
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 5);
	spi_read_buf(id_buf,8);	
	spi_cs_ctr(1);
}


/******	 写使能 	******/
void flash_write_enable(void)
{
	u8 cmd_buf[1]={FLASH_WIRT_ENABLE};
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 1);
	spi_cs_ctr(1);
}
/******	 写禁止 	******/
void flash_write_disable(void)
{
	u8 cmd_buf[1]={FLASH_WIRT_DISABLE};
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 1);
	spi_cs_ctr(1);
}

/******	 读状态REG 	******/
u8 flash_read_status_register(void)
{
	u8 status = 0;
	u8 cmd_buf[1]={FLASH_READ_STATUS1};
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 1);
	spi_read_buf(&status,1);
	spi_cs_ctr(1);	
	return status;
}
//void flash_wirte_status_register(void)
//{
//FLASH_WRITE_STATUS
//}
/******	 忙碌等待 	******/
void flash_wait_write_end(void)
{
	while(flash_read_status_register()&STATUS1_REG_BUSY_BIT);
}

/******	 掉电模式 	******/
void flash_power_down(void)
{
	u8 cmd_buf[1]={FLASH_POWER_DOWN};
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 1);
	spi_cs_ctr(1);
	spi_delay_us(3);	
}
/******	 唤醒  	******/
void flash_power_up(void)
{
	u8 cmd_buf[1]={FLASH_POWER_ON};
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 1);
	spi_cs_ctr(1);
	spi_delay_us(3);
}

/******	 扇区 擦除 	******/
/*1.地址要 4k 对齐		*/
/*如果没有对齐，则地址落在哪个扇区哪个扇区就被擦除*/
void flash_sector_erase(u32 sector_addr)
{
	if(sector_addr& (FLASH_SECTOR_SIZE-1)){		
		// not align by FLASH_SECTOR_SIZE
		spi_printf("FLASH WARNING: flash_sector_erase addr not align\n");
	}
	u8 cmd_buf[4];
    cmd_buf[0]=  FLASH_SECTOR_ERASE;
	cmd_buf[1] = (sector_addr>>16) & 0xff;
	cmd_buf[2] = (sector_addr>> 8) & 0xff;
	cmd_buf[3] = (sector_addr>> 0) & 0xff;
	flash_write_enable();
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 4);
	spi_cs_ctr(1);
	flash_wait_write_end();
}

/******	 BLK64 擦除 	******/		
/*1.地址要 64k 对齐		*/
void flash_block64_erase(u32 block64_addr)
{
	if(block64_addr& (FLASH_BLOCK_SIZE-1)){		
		// not align by FLASH_SECTOR_SIZE
		spi_printf("FLASH WARNING: flash_block64_erase addr not align\n");
	}
	u8 cmd_buf[4];
    cmd_buf[0]=  FLASH_BLOCK64_ERASE;
	cmd_buf[1] = (block64_addr>>16) & 0xff;
	cmd_buf[2] = (block64_addr>> 8) & 0xff;
	cmd_buf[3] = (block64_addr>> 0) & 0xff;
	flash_write_enable();
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 4);
	spi_cs_ctr(1);
	flash_wait_write_end();
}

/******	 整片 擦除 	******/		
void flash_chip_erase(void)
{
	u8 cmd_buf[1];
    cmd_buf[0]=  FLASH_CHIP_ERASE;
	flash_write_enable();
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 1);
	spi_cs_ctr(1);	
	flash_wait_write_end();
}


/******	 页 写入 	******/
/*1.应确保写入的地方已经被擦除*/
/*2.应确保写入的地方在同一个页*/
void flash_page_program(const u8 *buf, u32 addr, u32 len)
{
	if((len + addr&(FLASH_PAGE_SIZE-1)) > FLASH_PAGE_SIZE){		
		spi_printf("FLASH WARNING: write too long or not in one page\n");
	}
	u8 cmd_buf[4];
    cmd_buf[0]=  FLASH_PAGE_PROGRAM;
	cmd_buf[1] = (addr>>16) & 0xff;
	cmd_buf[2] = (addr>> 8) & 0xff;
	cmd_buf[3] = (addr>> 0) & 0xff;	
	flash_write_enable();
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 4);
	spi_write_buf(buf, len);
	spi_cs_ctr(1);	
	flash_wait_write_end();
}

/******	 写flash 	******/
void flash_write_buf(const u8 *buf, u32 addr, u32 len)
{
    u16 w_len = 0;
	u16 page_offset = addr&(FLASH_PAGE_SIZE - 1);
    u16 page_last_len = FLASH_PAGE_SIZE - page_offset; 

    u16 first_page_len = len<page_last_len?len:page_last_len;
    u16 last_page_len = (len - first_page_len)&(FLASH_PAGE_SIZE-1); 
    u32 whole_page_num = (len -  first_page_len - last_page_len)/FLASH_PAGE_SIZE;
//  spi_printf("page_offset = %d\n",page_offset);
//	spi_printf("page_last_len = %d\n",page_last_len);
//	spi_printf("first_page_len = %d\n",first_page_len);
//	spi_printf("last_page_len = %d\n",last_page_len);
//	spi_printf("whole_page_num = %d\n",whole_page_num);
    flash_page_program(buf, addr, first_page_len);
    w_len += first_page_len;
    while(whole_page_num){
        flash_page_program(buf+w_len, addr+w_len, FLASH_PAGE_SIZE);
        w_len += FLASH_PAGE_SIZE;
		whole_page_num--;
    }
    flash_page_program(buf+w_len, addr+w_len, last_page_len);
}
/******	 读flash 	******/	
void flash_read_buf(u8 *buf, u32 addr, u32 len)
{
	u8 cmd_buf[4];
    cmd_buf[0]=  FLASH_READ_DATA;
	cmd_buf[1] = (addr>>16) & 0xff;
	cmd_buf[2] = (addr>> 8) & 0xff;
	cmd_buf[3] = (addr>> 0) & 0xff;
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 4);
	spi_read_buf(buf,len);
	spi_cs_ctr(1);	
}
void flash_fast_read_buf(u8 *buf, u32 addr, u32 len)
{
	u8 cmd_buf[5];
    cmd_buf[0]=  FLASH_FAST_READ_DATA;
	cmd_buf[1] = (addr>>16) & 0xff;
	cmd_buf[2] = (addr>> 8) & 0xff;
	cmd_buf[3] = (addr>> 0) & 0xff;
	cmd_buf[4] = DUMMY_DATA;
	spi_cs_ctr(0);
	spi_write_buf(cmd_buf, 5);
	spi_read_buf(buf,len);
	spi_cs_ctr(1);	
}
void flash_erase_sectors(u32 start_sec, u32 sct_num)
{
	spi_printf("start time = %d\n",HAL_GetTick());
	while(sct_num--){		
		spi_printf("erase_sec = %d\n",start_sec);
        flash_sector_erase(start_sec<<FLASH_SCT_POWER);
        start_sec++;

    }
	spi_printf("end time = %d\n",HAL_GetTick());
}

u32 flash_get_sizeKB(void)
{
	spi_printf("FUN:%s\n",__func__);
    u32 flash_size = 0;
    const u32 flash_id[] = {0xEF4015,   //2M    W25Q16
							0xEF4016,   //4M    W25Q32
							0xEF4017,   //8M    W25Q64
							0xEF4018,   //16M   W25Q128
                            };
	for(u8 i=0; i< sizeof(flash_id)/sizeof(flash_id[0]); i++){
        if(flash_read_jedec() == flash_id[i]){
            flash_size = (1<<(i+1))<<10;
            return flash_size;
        }    
	}
	return 0;
}
bool flash_io_control(u8 cmd, void *buff)
{
    u8 res = false;
    switch(cmd){
/* Generic command (Used by FatFs) */
        case CTRL_SYNC:			//make sure write end
			res = true;			//default ok;  因为写入时已经有等待写入完成，所以这里默认ok
            break;
        case GET_SECTOR_COUNT:
            *((u32 *)buff) = (flash_get_sizeKB()<<10)/FLASH_SECTOR_SIZE - FLASH_REV_SEC_NUM;
            res = true;
            break;
        case GET_SECTOR_SIZE:
            *((u32 *)buff) = FLASH_SECTOR_SIZE;
            res = true;
            break;
        case GET_BLOCK_SIZE:
            *((u32 *)buff) = FLASH_BLOCK_SIZE;
            res = true;
            break;
        case CTRL_TRIM:
            break;
/* Generic command (Not used by FatFs) */
        case CTRL_POWER:
            break;
        case CTRL_LOCK:
            break;
        case CTRL_EJECT:
            break;
        case CTRL_FORMAT:
            break;
		default:
			break;

    }
    return res;
} 
void spi_flash_init(void)
{ 
	spi_printf("FUN:%s\n",__func__);
	spi1_init();
    spi_cs_gpio_init();
	spi_cs_ctr(0);
	flash_power_up();
}  
void spi_flash_test(void)
{
//	u8 unique_id[8];
//    u32 flash_size_kb = 0;
//	u32 flash_sec_cnt = 0;
	
	flash_read_id();
	flash_read_jedec();
//  flash_read_unique_id(unique_id);
//	
//	u8 i = 0;
//	for(i=0;i<8;i++){
//		spi_printf("unique_id[%d]: 0x%x\n",i,unique_id[i]);
//	}
//	flash_size_kb = flash_get_sizeKB();
//	spi_printf("flash_size_kb = 0x%x\n",flash_size_kb);
//	flash_sec_cnt = (flash_get_sizeKB()<<10)/FLASH_SECTOR_SIZE;
//	spi_printf("flash_sec_cnt = %d\n",flash_sec_cnt);
//	spi_printf("HAL_GetTick = %d\n",HAL_GetTick());
//	flash_chip_erase();
//	spi_printf("HAL_GetTick = %d\n",HAL_GetTick());	
	
	
//	flash_page_program(unique_id, 0, 8);
//	status_reg = flash_read_status_register();
//	spi_printf("status_reg: 0x%x\n",status_reg);
//	HAL_Delay(500);
//	flash_wait_write_end();
//	flash_write_buf(unique_id, 0, 8);
//	flash_fast_read_buf(read_buf, 0, 8);	
//	for(i=0;i<8;i++){
//		spi_printf("read_buf[%d]: 0x%x\n",i,read_buf[i]);
//	}
//	spi_puthex((const char *)read_buf, 8);
//	
//	flash_erase_sectors(0,10);
//	flash_sector_erase(FLASH_SECTOR_SIZE * 1-1);
//	flash_read_buf(read_buf, 0, 8);	
//	for(i=0;i<8;i++){
//		spi_printf("read_buf[%d]: 0x%x\n",i,read_buf[i]);
//	}
}













