#include "spi_sd.h"
#include "bsp_spi.h"

static __spi_sd_obj *this_obj = NULL;
extern void HAL_Delay(u32 Delay);
#define spi_sd_delay_ms(x)		HAL_Delay(x)

#define MAX_RETRY_TIME			0x20
#define HOST_SUPPORT_HIGH_CAP	1		//if host supports high capacity HCS=1, else HCS = 0 
#define SD_BLOCK_SIZE			512


static u8 sd_speed_set(u8 speed)
{
	u8 res  = 0;
	if((speed == SD_CLK_SPEED_HIGH)||(speed == SD_CLK_SPEED_LOW))
		res = this_obj->hd_io->io_ctr(SD_CLK_SPEED_SET, (void *)&speed);	
	return res;
}
u8 sd_detect(void)
{
	u8 status = SD_ON_LINE;
	return status;
}

static u8 sd_wait_ready(void)  
{  
    u32 t=0;  
	u8 dat = 0;
    do{  
		this_obj->hd_io->read(&dat, 1);
        if(dat == 0xff)return 0;
		sd_printf("sd wait:0x%x t:%d\n",dat, t);
    }while(++t < MAX_RETRY_TIME);
	sd_printf("[SD ERR]:SD_ERR_TIME_OUT\n");
    return 0xff;  
}  
  
static void sd_dis_select(void)  
{  
	u8 dat = 0;
    this_obj->hd_io->cs_str(1); 
    this_obj->hd_io->read(&dat, 1);
}  

static u8 sd_select(void)  
{  
	u8 res = 0;
    this_obj->hd_io->cs_str(0);   
    res = sd_wait_ready();
	if(res == 0xff)
		sd_dis_select();  
    return res; 
}  

static u8 sd_cmd_r1(u8 Cmd, u32 Arg, u8 Crc)
{
	u8 retry = 0xf;
	u8 cmd_fream[6];
	u8 r1 = 0xff;
    sd_dis_select();
	r1 = sd_select();
	if(r1) return r1;
	
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

	if(retry == 0){
		sd_printf("[ERR]: SD_SPI_R1 \n");
		return 0xff;
	}
	return r1;
}

static u8 sd_cmd(u8 Rx, u8 Cmd, u32 Arg, u8 Crc, u8 *r_buf)
{
	u8 r1 = 0xff;
	u8 len = 0;
	switch(Rx){
		case SD_SPI_R1:
		     len = SD_SPI_R1_LEN-1;
		     break;
		case SD_SPI_R2:
			 len = SD_SPI_R2_LEN-1;
			break;
		case SD_SPI_R3:
			len = SD_SPI_R3_LEN-1;
			break;
		case SD_SPI_R7:
			len = SD_SPI_R7_LEN-1;
			break;
		default:
			break;
	}
	 r1 = sd_cmd_r1(Cmd, Arg, Crc);
	 r_buf[0] = r1;
	 if(len){
		this_obj->hd_io->read(&(r_buf[1]), len);		 
	 }
	sd_printf("R%d: ",Rx);
	sd_puthex((const char *)r_buf, len+1);
	return r1;
}


//根据SPI时钟频率作相应修改
#define INIT_RETRY_MAX    400000
#define READ_BLOCK_MAX    200000 //200ms
#define WAIT_READY_MAX    500000 //500ms

SD_Error sd_check_res_token(u8 token)
{
	u32 retry = WAIT_READY_MAX; 
	// Check if response is got or a timeout is happen
	while ((this_obj->hd_io->w_r_byte(0xFF) != token) && --retry);
	if (retry){
		return SD_NO_ERR; // Right response got
	}else{
		return SD_ERR_CHECK_TOKEN; // After time out
	}
}





static SD_Error sd_get_csd_reg(SD_CSD_T *sd_csd)
{
	SD_Error res = SD_NO_ERR;
	SD_Type sd_type = this_obj->sd_inf.type;
	u8 r1;
	u8 CSD_Tab[16];
	sd_cmd(SD_SPI_R1, SD_CMD_SEND_CSD, 0x00, 0xff, &r1);
	if(r1 != 0){
		sd_dis_select();
		return SD_ERR_GET_CSD;
	}
	res = sd_check_res_token(SD_START_DATA_SINGLE_BLOCK_READ);
	if(res != SD_NO_ERR){
		return res;
	}
	this_obj->hd_io->read(CSD_Tab, 16);
	sd_dis_select();
	sd_printf("CSD: ");
	sd_puthex((const char *)CSD_Tab, 16);
	
	/*!< Byte 0 */	
	sd_csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
	sd_csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;		// ????
	sd_csd->Reserved1 = CSD_Tab[0] & 0x03;					// ????
	

	/*!< Byte 1 */
	sd_csd->TAAC = CSD_Tab[1];

	/*!< Byte 2 */
	sd_csd->NSAC = CSD_Tab[2];

	/*!< Byte 3 */
	sd_csd->MaxBusClkFrec = CSD_Tab[3];

	/*!< Byte 4 */
	sd_csd->CardComdClasses = CSD_Tab[4] << 4;

	/*!< Byte 5 */
	sd_csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
	sd_csd->RdBlockLen = CSD_Tab[5] & 0x0F;
	
	/*!< Byte 6 */
	sd_csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
	sd_csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
	sd_csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
	sd_csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
	sd_csd->Reserved2 = 0; /*!< Reserved */

	if (sd_type == SD_TYPE_SDHC) // or SDXC CSD v2.0
	{
		/*!< Byte 7 */
        sd_csd->DeviceSize = (CSD_Tab[7] & 0x3F) << 16;

        /*!< Byte 8 */
        sd_csd->DeviceSize |= (CSD_Tab[8] << 8);

        /*!< Byte 9 */
        sd_csd->DeviceSize |= (CSD_Tab[9]);
	}
	else if (sd_type == SD_TYPE_V1_X || sd_type == SD_TYPE_SDSC) //CSD v1.0
	{
		sd_csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;
		/*!< Byte 7 */
		sd_csd->DeviceSize |= (CSD_Tab[7]) << 2;

		/*!< Byte 8 */
		sd_csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;

		sd_csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
		sd_csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);

		/*!< Byte 9 */
		sd_csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
		sd_csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
		sd_csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;

		/*!< Byte 10 */
		sd_csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
	}
	
	sd_csd->EraseGrSize = (CSD_Tab[10] & 0x40) >> 6;
	sd_csd->EraseGrMul = (CSD_Tab[10] & 0x3F) << 1;

	/*!< Byte 11 */
	sd_csd->EraseGrMul |= (CSD_Tab[11] & 0x80) >> 7;
	sd_csd->WrProtectGrSize = (CSD_Tab[11] & 0x7F);

	/*!< Byte 12 */
	sd_csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
	sd_csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
	sd_csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
	sd_csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;

	/*!< Byte 13 */
	sd_csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xC0) >> 6;
	sd_csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
	sd_csd->Reserved3 = 0;
	sd_csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);

	/*!< Byte 14 */
	sd_csd->FileFormatGroup = (CSD_Tab[14] & 0x80) >> 7;
	sd_csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
	sd_csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
	sd_csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
	sd_csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
	sd_csd->ECC = (CSD_Tab[14] & 0x03);

	/*!< Byte 15 */
	sd_csd->CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
	sd_csd->Reserved4 = 1;
	
//	sd_printf("CSDStruct:\t0x%x \n",sd_csd->CSDStruct);
//	sd_printf("SysSpecVersion:\t0x%x \n",sd_csd->SysSpecVersion);
//	sd_printf("Reserved1\t0x%x \n",sd_csd->Reserved1);

//	sd_printf("TAAC:\t\t0x%x \n",sd_csd->TAAC);
//	sd_printf("NSAC:\t\t0x%x \n",sd_csd->NSAC);
//	sd_printf("MaxBusClkFrec\t0x%x \n",sd_csd->MaxBusClkFrec);	//0x32:25M 0x5A:50M 

//	sd_printf("CardComdClasses\t0x%x \n",sd_csd->CardComdClasses);	
//	sd_printf("RdBlockLen\t0x%x \n",sd_csd->RdBlockLen);	

//	sd_printf("PartBlockRead\t0x%x \n",sd_csd->PartBlockRead);
//	sd_printf("WrBlockMisalign\t0x%x \n",sd_csd->WrBlockMisalign);
//	sd_printf("RdBlockMisalign\t0x%x \n",sd_csd->RdBlockMisalign);
//	sd_printf("DSRImpl  \t0x%x \n",sd_csd->DSRImpl);
//	sd_printf("Reserved2\t0x%x \n",sd_csd->Reserved2);

//	sd_printf("DeviceSize\t0x%x \n",sd_csd->DeviceSize);
//	sd_printf("MaxRdCurrentVDDMin\t0x%x \n",sd_csd->MaxRdCurrentVDDMin);
//	sd_printf("MaxWrCurrentVDDMax\t0x%x \n",sd_csd->MaxWrCurrentVDDMax);
//	sd_printf("DeviceSizeMul\t0x%x \n",sd_csd->DeviceSizeMul);


	return res;

}


static SD_Error sd_get_cid_reg(SD_CID_T *sd_cid)
{
	SD_Error res = SD_NO_ERR;
	SD_Type sd_type = this_obj->sd_inf.type;
	u8 r1;
	u8 CID_Tab[16];
	sd_cmd(SD_SPI_R1, SD_CMD_SEND_CID, 0x00, 0xff, &r1);
	if(r1 != 0){
		sd_dis_select();
		return SD_ERR_GET_CSD;
	}
	res = sd_check_res_token(SD_START_DATA_SINGLE_BLOCK_READ);
	if(res != SD_NO_ERR){
		return res;
	}
	this_obj->hd_io->read(CID_Tab, 16);
	sd_dis_select();
	sd_printf("CID: ");
	sd_puthex((const char *)CID_Tab, 16);
	
	
	/*!< Byte 0 */
	sd_cid->ManufacturerID = CID_Tab[0];

	/*!< Byte 1 */
	sd_cid->OEM_AppliID = CID_Tab[1] << 8;

	/*!< Byte 2 */
	sd_cid->OEM_AppliID |= CID_Tab[2];

	/*!< Byte 3 */
	sd_cid->ProdName1 = CID_Tab[3] << 24;

	/*!< Byte 4 */
	sd_cid->ProdName1 |= CID_Tab[4] << 16;

	/*!< Byte 5 */
	sd_cid->ProdName1 |= CID_Tab[5] << 8;

	/*!< Byte 6 */
	sd_cid->ProdName1 |= CID_Tab[6];

	/*!< Byte 7 */
	sd_cid->ProdName2 = CID_Tab[7];

	/*!< Byte 8 */
	sd_cid->ProdRev = CID_Tab[8];

	/*!< Byte 9 */
	sd_cid->ProdSN = CID_Tab[9] << 24;

	/*!< Byte 10 */
	sd_cid->ProdSN |= CID_Tab[10] << 16;

	/*!< Byte 11 */
	sd_cid->ProdSN |= CID_Tab[11] << 8;

	/*!< Byte 12 */
	sd_cid->ProdSN |= CID_Tab[12];

	/*!< Byte 13 */
	sd_cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
	sd_cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;

	/*!< Byte 14 */
	sd_cid->ManufactDate |= CID_Tab[14];

	/*!< Byte 15 */
	sd_cid->CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
	sd_cid->Reserved2 = 1;

//	sd_printf("ManufacturerID\t0x%x \n",sd_cid->ManufacturerID);
//	sd_printf("OEM_AppliID\t0x%x \n",sd_cid->OEM_AppliID);
//	sd_printf("ProdName1\t0x%x \n",sd_cid->ProdName1);
//	sd_printf("ProdName2\t0x%x \n",sd_cid->ProdName2);
//	sd_printf("ProdRev    \t0x%x \n",sd_cid->ProdRev);
//	sd_printf("ProdSN    \t0x%x \n",sd_cid->ProdSN);
//	sd_printf("Reserved1\t0x%x \n",sd_cid->Reserved1);
//	sd_printf("CID_CRC    \t0x%x \n",sd_cid->CID_CRC);
//	sd_printf("Reserved2\t0x%x \n",sd_cid->Reserved2);

	/*!< Return the reponse */
	return res;	
}	






SD_Error sd_getcard_inf(__sd_inf_t *sd_inf)
{
	SD_Error res = SD_NO_ERR;
	SD_Type sd_type = sd_inf->type;
	res = sd_get_csd_reg(&(this_obj->sd_inf.CSD));
	res = sd_get_cid_reg(&(this_obj->sd_inf.CID));

	if (sd_type == SD_TYPE_SDHC)
	{
		sd_inf->capacity = ((u64)sd_inf->CSD.DeviceSize + 1) * 512 * 1024;
        sd_inf->block_sz = 512;
	}
	else if (sd_type == SD_TYPE_SDSC || sd_type == SD_TYPE_V1_X)
	{
		sd_inf->capacity = (sd_inf->CSD.DeviceSize + 1);
		sd_inf->capacity *= (1 << (sd_inf->CSD.DeviceSizeMul + 2));
		sd_inf->block_sz = 1 << (sd_inf->CSD.RdBlockLen);
		sd_inf->capacity *= sd_inf->block_sz;
	}
	sd_printf("DeviceSize\t0x%x \n",sd_inf->CSD.DeviceSize);
	sd_printf("block_sz\t0x%x \n",sd_inf->block_sz);
	sd_printf("capacity\t%lld\n",sd_inf->capacity);
	sd_printf("Reserved2\t0x%x \n",sd_inf->type);


	
	
	return res;
}
	
	




static SD_Error sd_idle_mode(void)
{
	u8 tmp_buf[10];
	u8 retry = MAX_RETRY_TIME;
	u8 r1 = 0xff;
	//0: Reset card  >74  clk **/
	sd_speed_set(SD_CLK_SPEED_LOW);
    this_obj->hd_io->cs_str(1);	
	this_obj->hd_io->read(tmp_buf, 10);
	do{// CMD0--> idle mode **/
		r1 = sd_cmd(SD_SPI_R1, SD_CMD_GO_IDLE_STATE, 0x00, 0x95, &r1);
	}while((r1 != SPI_R1_IDLE_STATE)&&(--retry));
	if(retry == 0){
		return SD_ERR_GO_IDLE;
	}
	return SD_NO_ERR;
}


/*============	读写函数	=============*/
/*
*	1. addr shall aligned to block boundary
	   a. SDHC clock is 512-Byte
	   b. addr  在SDHC中以(block) 512-byte为单位, 
		    	在SDSC中以byte为单位。
*/
static u8 sd_read_single_blk(u8* buf, u32 addr, u16 blk_sz)
{
	u8 res = 0xff;
	u16 crc = 0;
	sd_cmd(SD_SPI_R1, SD_CMD_READ_SINGLE_BLOCK, addr, 0xFF, &res);
	if(res != 0){
		return res;
	}
	res = sd_check_res_token(SD_START_DATA_SINGLE_BLOCK_READ);
	if(res == SD_NO_ERR){
		this_obj->hd_io->read(buf,blk_sz);
	    this_obj->hd_io->read((u8 *)(&crc),sizeof(crc));
	}
	return res;
	
}
static void sd_read_multi_blks(u8* buf, u32 addr, u16 blk_bum)
{
//	u8 res = 0xff;
//	u16 crc = 0;
//	sd_cmd(SD_SPI_R1, SD_CMD_READ_MULTIPLE_BLOCK, addr, 0xFF, &res);
//	if(res != 0){
//		return res;
//	}
//	res = sd_check_res_token(SD_START_DATA_SINGLE_BLOCK_READ);
//	if(res == SD_NO_ERR){
//		this_obj->hd_io->read(buf,blk_sz);
//	    this_obj->hd_io->read((u8 *)(&crc),sizeof(crc));
//	}
//	return res;

}
static void sd_write_single_blk(const u8 *buf, u32 addr, u32 len)
{
	ASSERT(buf);
	u8 r1 = 0xff;
	sd_cmd(SD_SPI_R1, SD_CMD_WRITE_SINGLE_BLOCK, 0, 0xFF, &r1);
}

static void sd_write_multi_blks(u8 *buf, u32 addr, u32 len)
{
	ASSERT(buf);

}
/*============	end	=============*/


static SD_Error identification_v1_card(void)
{
	u8 retry, r1;
	u8 rx_buf[5] = {0};

//	r1 = sd_cmd(SD_SPI_R3, SD_CMD_READ_OCR, 0, 0XFF, rx_buf);  // CMD58:  Read OCR  to get supported voltage **/ 	

    retry = MAX_RETRY_TIME;		 
	do{															// ACMD41: 1.send host capacity support information  2.Active teh card
		r1 = sd_cmd(SD_SPI_R1, SD_CMD_APP_CMD, 0, 0xFF, rx_buf);
#if(HOST_SUPPORT_HIGH_CAP)
		r1 = sd_cmd(SD_SPI_R1, SD_ACMD_SEND_OP_COND, 0x40000000, 0xFF, rx_buf);			// HCS = 1 
#else			
		r1 = sd_cmd(SD_SPI_R1, SD_ACMD_SEND_OP_COND, 0x00000000, 0xFF, rx_buf);			// HCS = 0  
#endif			
	}while(r1 && --retry);		 
	if(r1 != 0){
		sd_printf("[ERR]: line：%d \n",__LINE__);
		this_obj->sd_inf.type = SD_TYPE_UNKNOW;	
        return SD_ERR_CHECK_V1;		
	}
	sd_printf(">>>> SD TYPE V1.x ==<<<<<<\n");
	this_obj->sd_inf.type = SD_TYPE_V1_X;
	return SD_NO_ERR;
}


static SD_Error identification_v2v3(void)
{
	u8 retry, r1;
	u8 rx_buf[5] = {0};

    retry = MAX_RETRY_TIME;		 
	do{// ACMD41: 1.send host capacity support information 2.Active teh card
		r1 = sd_cmd(SD_SPI_R1, SD_CMD_APP_CMD, 0, 0xFF, rx_buf);
#if(HOST_SUPPORT_HIGH_CAP)
		r1 = sd_cmd(SD_SPI_R1, SD_ACMD_SEND_OP_COND, 0x40000000, 0xFF, rx_buf);			// HCS = 1 
#else			
		r1 = sd_cmd(SD_SPI_R1, SD_ACMD_SEND_OP_COND, 0x00000000, 0xFF, rx_buf);			// HCS = 0  
#endif			
	}while(r1 && --retry);
	if(r1 != 0){
		sd_printf("[ERR]: line：%d \n",__LINE__);
		return  SD_ERR_CHECK_V2V3;			
	}	 
	r1 = sd_cmd(SD_SPI_R3, SD_CMD_READ_OCR, 0, 0XFF, rx_buf);	// CMD58:  GET  CCS		 
	if(rx_buf[1] & 0x40){
		sd_printf(">>>> SDHC or SDXC card ==<<<<<<\n");
		this_obj->sd_inf.type = SD_TYPE_SDHC;
		return SD_NO_ERR;			
	}else{
		sd_printf(">>>> SDSC  card ==<<<<<<\n");
	    this_obj->sd_inf.type = SD_TYPE_SDSC;
		return SD_NO_ERR;	
	} 		
}
static SD_Error sd_identification(void)
{
	u8 r1;
	u8 rx_buf[5] = {0};
	SD_Error res = SD_NO_ERR;
    res = sd_idle_mode();
	if(res != SD_NO_ERR){
		sd_printf("[ERR]: line：%d \n",__LINE__);
		return res;		
	}
	r1 = sd_cmd(SD_SPI_R7, SD_CMD_IF_COND, 0x01AA, 0x87, rx_buf);	//CMD8: check 2.7v~3.6v
	if(r1&SPI_R1_ILL_CMD){
		sd_printf(">>>> SD does not surport CMD8 , likely V1.x ==<<<<<<\n");
		res = identification_v1_card();
	 }else if((rx_buf[3] == 0x01)&&(rx_buf[4]==0xAA)){
		sd_printf(">>>> SD surport CMD8 ,check 2.7v~3.6v OK !! ==<<<<<<\n"); 
		res = identification_v2v3();
	 }else{
		 sd_printf(">>>> SD surport CMD8 ,check 2.7v~3.6v ERR !! ==<<<<<<\n");
		 res = SD_ERR_CHECK_VOL;
	 }
	 return res;
}



static SD_Error sd_init(__spi_ctr_obj  *hd_io)
{
	sd_printf("FUN:%s\n",__func__);
	ASSERT(hd_io);
	ASSERT(hd_io->read);
	ASSERT(hd_io->write);
    SD_Error res;
	u8 r1;
    this_obj = &spi_sd_obj;
	this_obj->hd_io = hd_io;
	this_obj->sd_inf.type = SD_TYPE_UNKNOW;	
   
	
    res = sd_identification();
	
	if(res != SD_NO_ERR){
		return res;
	}
	// CMD59 关闭 CRC 校验
	sd_cmd(SD_SPI_R1, SD_CMD_CRC_ON_OFF, 0x00, 0xFF, &r1);
	sd_cmd(SD_SPI_R1, SD_CMD_CRC_ON_OFF, 0x00, 0xFF, &r1);
	sd_cmd(SD_SPI_R1, SD_CMD_CRC_ON_OFF, 0x00, 0xFF, &r1);
    // CMD16 设置 BLOCK 为 512 Bytes (对 SDHC/SDHX 无效)
	sd_cmd(SD_SPI_R1, SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, &r1);
	if(r1){
		return SD_ERR_SET_BLOCK_SIZE;
	}	
	sd_dis_select();
	sd_speed_set(SD_CLK_SPEED_HIGH);
	
	sd_printf(">>>>> get  sd  inform <<<<<\n");
	
    sd_getcard_inf(&this_obj->sd_inf);
	
	u8 sd_buf[512] = {0};
	
	sd_read_single_blk(sd_buf, 0, 512);
	
	sd_puthex((const char *)sd_buf,512);
	
	
	while(1);
	
	
	
	return SD_NO_ERR;
}



__spi_sd_obj spi_sd_obj = {

	.init = sd_init,

};
 
