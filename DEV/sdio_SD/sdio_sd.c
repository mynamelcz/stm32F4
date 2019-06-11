#include "sdio_sd.h"

#include "stm32f4xx_ll.h"



#define NULL 0
#define SDIO_STATIC_FLAG               ((u32)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((u32)0x00010000)

/** 
  * @brief  Mask for errors Card Status R1 (OCR Register) 
  */
#define SD_OCR_ADDR_OUT_OF_RANGE        ((u32)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((u32)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((u32)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((u32)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((u32)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((u32)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((u32)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((u32)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((u32)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((u32)0x00200000)
#define SD_OCR_CC_ERROR                 ((u32)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((u32)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((u32)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((u32)0x00020000)
#define SD_OCR_CID_CSD_OVER_WRIETE      ((u32)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((u32)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((u32)0x00004000)
#define SD_OCR_ERASE_RESET              ((u32)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((u32)0x00000008)
#define SD_OCR_ERRORBITS                ((u32)0xFDFFE008)

/** 
  * @brief  Masks for R6 Response 
  */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((u32)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((u32)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((u32)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((u32)0x80100000)
#define SD_HIGH_CAPACITY                ((u32)0x40000000)
#define SD_STD_CAPACITY                 ((u32)0x00000000)
#define SD_CHECK_PATTERN                ((u32)0x000001AA)

#define SD_MAX_VOLT_TRIAL               ((u32)0x0000FFFF)
#define SD_ALLZERO                      ((u32)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((u32)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((u32)0x00010000)
#define SD_CARD_LOCKED                  ((u32)0x02000000)

#define SD_DATATIMEOUT                  ((u32)0xFFFFFF)
#define SD_0TO7BITS                     ((u32)0x000000FF)
#define SD_8TO15BITS                    ((u32)0x0000FF00)
#define SD_16TO23BITS                   ((u32)0x00FF0000)
#define SD_24TO31BITS                   ((u32)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((u32)0x01FFFFFF)

#define SD_HALFFIFO                     ((u32)0x00000008)
#define SD_HALFFIFOBYTES                ((u32)0x00000020)

/** 
  * @brief  Command Class Supported 
  */
#define SD_CCCC_LOCK_UNLOCK             ((u32)0x00000080)
#define SD_CCCC_WRITE_PROT              ((u32)0x00000040)
#define SD_CCCC_ERASE                   ((u32)0x00000020)

/** 
  * @brief  Following commands are SD Card Specific commands.
  *         SDIO_APP_CMD should be sent before sending these commands. 
  */
#define SDIO_SEND_IF_COND               ((u32)0x00000008)

/**
  * @}
  */ 

/** @defgroup STM324x7I_EVAL_SDIO_SD_Private_Macros
  * @{
  */
/**
  * @}
  */ 

/** @defgroup STM324x7I_EVAL_SDIO_SD_Private_Variables
  * @{
  */

static u32 CardType =  SDIO_STD_CAPACITY_SD_CARD_V1_1;
static u32 CSD_Tab[4], CID_Tab[4], RCA = 0;
static u8 SDSTATUS_Tab[16];
volatile u32 StopCondition = 0;
volatile SD_Error TransferError = SD_OK;
volatile u32 DMAEndOfTransfer = 0;

volatile u32 TransferISEnd = 0;

SD_CardInfo SDCardInfo;













extern __weak void HAL_Delay(uint32_t Delay);


SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;
/**
  * @}
  */ 


/** @defgroup STM324x7I_EVAL_SDIO_SD_Private_Function_Prototypes
  * @{
  */
static SD_Error CmdError(void);
static SD_Error CmdResp1Error(u8 cmd);
static SD_Error CmdResp7Error(void);
static SD_Error CmdResp3Error(void);
static SD_Error CmdResp2Error(void);
static SD_Error CmdResp6Error(u8 cmd, u16 *prca);
static SD_Error SDEnWideBus(FunctionalState NewState);
static SD_Error IsCardProgramming(u8 *pstatus);
static SD_Error FindSCR(u16 rca, u32 *pscr);
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes);
  
void SD_LowLevel_DeInit(void)
{
	;
}
  
void SD_LowLevel_Init(void)
{
	;
}	
  


//============================================================//

#define SDIO_POWER_STATE_OFF                 ((uint32_t)0x00000000)
#define SDIO_POWER_STATE_ON                  ((uint32_t)0x00000003)

u8 SDIO_GetITStatus(u32 flag)
{
	if(__SDIO_GET_IT(SDIO, flag)){
		return SET;
	}
	return RESET;
	
}
void SDIO_ClearITPendingBit(u32 sdio_IT)
{
	__SDIO_CLEAR_IT(SDIO, sdio_IT);
}

u8 SDIO_GetFlagStatus(uint32_t SDIO_FLAG)
{ 
  if (__SDIO_GET_FLAG(SDIO, SDIO_FLAG)){
    return SET;
  }
  return RESET;
}
void SDIO_ClearFlag(u32 SDIO_FLAG)
{
	__SDIO_CLEAR_FLAG(SDIO, SDIO_FLAG);
}


u32 SDIO_Get_Response(u32 SDIO_RESP)
{
	return SDIO_GetResponse(SDIO, SDIO_RESP);
}

uint8_t SDIO_Get_CommandResponse(void)
{
  return SDIO_GetCommandResponse(SDIO);
}

void SDIO_ITConfig(uint32_t SDIO_IT, FunctionalState NewState)
{
  if (NewState != DISABLE) {
    __SDIO_ENABLE_IT(SDIO, SDIO_IT);
  }
  else{
    __SDIO_DISABLE_IT(SDIO, SDIO_IT);
  } 
}

u32 SDIO_ReadData(void)
{ 
  return SDIO_ReadFIFO(SDIO);
}


extern void sdio_dma_send(u32 *src_addr, u32 bufferSize);
extern void sdio_dma_receive(u32 *src_addr, u32 bufferSize);


void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize)
{
	sdio_dma_receive(BufferDST, BufferSize);
}
void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize)
{
	sdio_dma_send(BufferSRC, BufferSize);
}



void sdio_set_low_speed(u32 bus_wide)
{
  SDIO_InitStructure.ClockDiv = SDIO_INIT_CLK_DIV;
  SDIO_InitStructure.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  SDIO_InitStructure.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  SDIO_InitStructure.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  SDIO_InitStructure.BusWide = bus_wide;
  SDIO_InitStructure.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE;
  SDIO_Init(SDIO, SDIO_InitStructure);
}
void sdio_set_high_speed(u32 bus_wide)
{
  SDIO_InitStructure.ClockDiv = SDIO_TRANSFER_CLK_DIV;
  SDIO_InitStructure.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  SDIO_InitStructure.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  SDIO_InitStructure.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  SDIO_InitStructure.BusWide = bus_wide;
  SDIO_InitStructure.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE;
  SDIO_Init(SDIO, SDIO_InitStructure);
}
void sdio_send_cmd(u32 cmd, u32 arg, u32 res)
{
  SDIO_CmdInitStructure.Argument = arg;
  SDIO_CmdInitStructure.CmdIndex = cmd;
  SDIO_CmdInitStructure.Response = res;
  SDIO_CmdInitStructure.WaitForInterrupt = SDIO_WAIT_NO;
  SDIO_CmdInitStructure.CPSM = SDIO_CPSM_ENABLE;
  SDIO_SendCommand(SDIO, &SDIO_CmdInitStructure);
}

void sdio_data_cfg(u32 len,u32 dir, u32 block_sz)
{
  SDIO_DataInitStructure.DataLength = len;
  SDIO_DataInitStructure.TransferDir = dir;
  SDIO_DataInitStructure.DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.DataBlockSize = block_sz;
  SDIO_DataInitStructure.TransferMode = SDIO_TRANSFER_MODE_BLOCK;
  SDIO_DataInitStructure.DPSM = SDIO_DPSM_ENABLE;
  SDIO_ConfigData(SDIO, &SDIO_DataInitStructure);
}




static SD_Error FindSCR(u16 rca, u32 *pscr)
{
  u32 index = 0;
  SD_Error errorstatus = SD_OK;
  u32 tempscr[2] = {0, 0};

  /*!< Set Block Size To 8 Bytes */
  /*!< Send CMD55 APP_CMD with argument as card's RCA */
  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)8, SDIO_RESPONSE_SHORT);
 
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (errorstatus != SD_OK){
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  /*!< Send CMD55 APP_CMD with argument as card's RCA */
  sdio_send_cmd(SD_CMD_APP_CMD, (u32)RCA << 16, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
  if (errorstatus != SD_OK){
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  sdio_data_cfg(8, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_DATABLOCK_SIZE_8B);

  /*!< Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
  sdio_send_cmd(SD_CMD_SD_APP_SEND_SCR, (u32)0x0, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);
  if (errorstatus != SD_OK){
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  while (!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET){
      *(tempscr + index) = SDIO_ReadData();
		sd_printf("SCR[%d]:0x%x\n",index, tempscr[index]);
		index++;
		*(tempscr + index) = SDIO_ReadData();
    }
  }
  
  if(((SDIO->STA & SDIO_FLAG_RXACT)))
  {
	 sd_printf("SDIO_ReadData:0x%x  ,LINE:%d \n", SDIO_ReadData(), __LINE__); 
  }  
  
  
  
  if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET){
    SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET){
    SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET){
    SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
    errorstatus = SD_RX_OVERRUN;
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET){
    SDIO_ClearFlag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  *(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);

  *(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);

  return(errorstatus);
}













SD_Error SD_StopTransfer(void)
{
  SD_Error errorstatus = SD_OK;
  /*!< Send CMD12 STOP_TRANSMISSION  */
  sdio_send_cmd(SD_CMD_STOP_TRANSMISSION, (u32)0, SDIO_RESPONSE_SHORT);    
  errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
  return(errorstatus);
}

SD_Error SD_WaitReadOperation(void)
{
  SD_Error errorstatus = SD_OK;
  uint32_t timeout;

  timeout = SD_DATATIMEOUT;
  
  while ((DMAEndOfTransfer == 0x00) && (TransferISEnd == 0) && (TransferError == SD_OK) && (timeout > 0))
  {
	  sd_printf("timeout:%d  line:%d\n",timeout, __LINE__);
      timeout--;
  }




  DMAEndOfTransfer = 0x00;
  timeout = SD_DATATIMEOUT;
  while(((SDIO->STA & SDIO_FLAG_RXACT)) && (timeout > 0))
  {
    sd_printf("SDIO_ReadData:0x%x  ,LINE:%d \n", SDIO_ReadData(), __LINE__);
    timeout--;  
  }
  
  
  
  if (StopCondition == 1){
    errorstatus = SD_StopTransfer();
    StopCondition = 0;
  }
  if ((timeout == 0) && (errorstatus == SD_OK)){
    errorstatus = SD_DATA_TIMEOUT;
  } 
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  if (TransferError != SD_OK){
	ERR_printf(TransferError);
    return(TransferError);
  }else{
    return(errorstatus);  
  }
}


SD_Error SD_SendStatus(u32 *pcardstatus)
{
  SD_Error errorstatus = SD_OK;

  if (pcardstatus == NULL){
    errorstatus = SD_INVALID_PARAMETER;
	ERR_printf(TransferError);
    return(errorstatus);
  }
  sdio_send_cmd(SD_CMD_SEND_STATUS, (u32)RCA << 16, SDIO_RESPONSE_SHORT);    
  
  errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);
  if (errorstatus != SD_OK){
	ERR_printf(TransferError);
    return(errorstatus);
  }
  *pcardstatus = SDIO_GetResponse(SDIO, SDIO_RESP1);
  return(errorstatus);
}

u8 SD_Detect(void)
{
  volatile u8 status = SD_PRESENT;

  /*!< Check GPIO to detect SD */
//  if (GPIO_ReadInputDataBit(SD_DETECT_GPIO_PORT, SD_DETECT_PIN) != Bit_RESET)
//  {
//    status = SD_NOT_PRESENT;
//  }
  return status;
}

SDCardState SD_GetState(void)
{
  uint32_t resp1 = 0;
  
  if(SD_Detect()== SD_PRESENT){
    if (SD_SendStatus(&resp1) != SD_OK){
	  ERR_printf(TransferError);
      return SD_CARD_ERROR;
    }
    else{
      return (SDCardState)((resp1 >> 9) & 0x0F);
    }
  }else{
	ERR_printf(TransferError);
    return SD_CARD_ERROR;
  }
}

SDTransferState SD_GetStatus(void)
{
  SDCardState cardstate =  SD_CARD_TRANSFER;

  cardstate = SD_GetState();
  if (cardstate == SD_CARD_TRANSFER){
    return(SD_TRANSFER_OK);
  }else if(cardstate == SD_CARD_ERROR){
    return (SD_TRANSFER_ERROR);
  }else{
    return(SD_TRANSFER_BUSY);
  }
}

SD_Error sd_read_blocks(u8 *readbuff, u32 start_blk, u16 block_num)
{
  SD_Error errorstatus = SD_OK;
  u32 BlockSize = SD_BLOCK_SIZE;
  u32 block_addr = start_blk;
	
  TransferError = SD_OK;
  TransferISEnd = 0;

  /* Initialize data control register */
  SDIO->DCTRL = 0x0;

  if (CardType != SDIO_HIGH_CAPACITY_SD_CARD){
    block_addr *= 512;
  }

 // __SDIO_ENABLE_IT(SDIO,SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);

  /* Set Block Size for Card */ 
  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)BlockSize, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus){
	ERR_printf(errorstatus);
    return(errorstatus);
  }	
  /* Configure the SD DPSM (Data Path State Machine) */
  sdio_data_cfg(BlockSize * block_num, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_DATABLOCK_SIZE_512B);

  
  if(block_num>1){
	StopCondition = 1;
	sdio_send_cmd(SD_CMD_READ_MULT_BLOCK, (u32)block_addr, SDIO_RESPONSE_SHORT); 
	errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);
  }else{
	StopCondition = 0;
	sdio_send_cmd(SD_CMD_READ_SINGLE_BLOCK, (u32)block_addr, SDIO_RESPONSE_SHORT);
	errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);
  } 
  if (errorstatus != SD_OK){
	ERR_printf(errorstatus);
    return(errorstatus);
  }	
  
//   SD_LowLevel_DMA_RxConfig((u32 *)readbuff, (BlockSize * block_num)/4);  
//  __SDIO_DMA_ENABLE(SDIO); 

  u32 *p_buf = (u32 *)readbuff;
  u8 i = 0;
  u16 n = 0;

  while(!(SDIO->STA & SDIO_FLAG_DATAEND)){
	if(SDIO->STA & SDIO_FLAG_RXFIFOHF){
		for(i = 0; i<8; i++){
			*(p_buf + i) = SDIO_ReadData();
			n++;
		}
		p_buf+=8;
	}
  }
  sd_printf("N:%d\n",n);
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
//  errorstatus = SD_WaitReadOperation();
//  if(errorstatus !=SD_OK){
//	 ERR_printf(errorstatus);
//  }
  return(errorstatus);
}

SD_Error sd_write_blocks(u8 *writebuff, u32 start_blk, u16 block_num)
{
  SD_Error errorstatus = SD_OK;
  u32 BlockSize = SD_BLOCK_SIZE;
  u32 block_addr = start_blk;	
	
  TransferError = SD_OK;
  TransferISEnd = 0;
	
  SDIO->DCTRL = 0x0;
  if (CardType != SDIO_HIGH_CAPACITY_SD_CARD){
    block_addr *= 512;
  }

 // __SDIO_ENABLE_IT(SDIO,SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);  

  /* Set Block Size for Card */ 
  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)BlockSize, SDIO_RESPONSE_SHORT);  
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus){
	ERR_printf(TransferError);
    return(errorstatus);
  }
  if(block_num > 1){
      sdio_send_cmd(SD_CMD_WRITE_MULT_BLOCK, (u32)block_addr, SDIO_RESPONSE_SHORT);   
      errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);
  }else{
	  sdio_send_cmd(SD_CMD_WRITE_SINGLE_BLOCK, (u32)block_addr, SDIO_RESPONSE_SHORT);  
	  errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);  
  }
  if (errorstatus != SD_OK){
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  
//  __SDIO_DMA_ENABLE(SDIO);
//  SD_LowLevel_DMA_TxConfig((u32 *)writebuff, (BlockSize*block_num)/ 4);
  sdio_data_cfg(BlockSize * block_num, SDIO_TRANSFER_DIR_TO_CARD, SDIO_DATABLOCK_SIZE_512B);
  
  
  
  u32 *p_buf = (u32 *)block_addr;
  u8 i = 0;
  u16 n = 0;

  while(!(SDIO->STA & (SDIO_FLAG_TXUNDERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DATAEND | SDIO_FLAG_STBITERR))){
	if(SDIO->STA & SDIO_FLAG_TXFIFOHE){
		for(i = 0; i<8; i++){
			SDIO_WriteFIFO(SDIO, p_buf+i);
			n++;
		}
		p_buf+=8;

		sd_printf("N:%d\n",n);
	}
  }
  sd_printf("N:%d\n",n);
  
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);
  
  
  
//  errorstatus = SD_WaitWriteOperation();
//  if(errorstatus !=SD_OK){
//	 ERR_printf(errorstatus);
//  }
  return(errorstatus);
}




SD_Error SD_WaitWriteOperation(void)
{
  SD_Error errorstatus = SD_OK;
  u32 timeout;

  timeout = SD_DATATIMEOUT;
  
  while ((DMAEndOfTransfer == 0x00) && (TransferISEnd == 0) && (TransferError == SD_OK) && (timeout > 0))
  {
    timeout--;
  }
  
    __SDIO_DMA_DISABLE(SDIO);
  
  DMAEndOfTransfer = 0x00;
  timeout = SD_DATATIMEOUT;
  while(((SDIO->STA & SDIO_FLAG_TXACT)) && (timeout > 0))
  {
	  
	  sd_printf("SDIO->STA :0x%x LINE:%d\n",SDIO->STA, __LINE__);
//	  SDIO_WriteFIFO(SDIO, &timeout);
      timeout--;  
  }
  if (StopCondition == 1){
    errorstatus = SD_StopTransfer();
    StopCondition = 0;
  }
  if ((timeout == 0) && (errorstatus == SD_OK)){
    errorstatus = SD_DATA_TIMEOUT;
  }
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);
  
  if (TransferError != SD_OK){
	ERR_printf(errorstatus);
    return(TransferError);
  }
  else{
    return(errorstatus);
  }
}


SDTransferState SD_GetTransferState(void)
{
  if (SDIO->STA & (SDIO_FLAG_TXACT | SDIO_FLAG_RXACT))
  {
	  sd_printf("SD_TRANSFER_BUSY\n");
    return(SD_TRANSFER_BUSY);
  }
  else
  {
    return(SD_TRANSFER_OK);
  }
}





u8 test_buf[1024];
SD_Error SD_Init(void)
{
  __IO SD_Error errorstatus = SD_OK;
	
  errorstatus = SD_PowerON();
  if (errorstatus != SD_OK)
  {
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  errorstatus = SD_InitializeCards();
  
  if (errorstatus != SD_OK)
  {
    ERR_printf(errorstatus);
    return(errorstatus);
  }
  
  sdio_set_high_speed(SDIO_BUS_WIDE_1B);
  /*----------------- Read CSD/CID MSD registers ------------------*/
  errorstatus = SD_GetCardInfo(&SDCardInfo);
  
  
  if (errorstatus == SD_OK){
    /*----------------- Select Card --------------------------------*/
    errorstatus = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
  }
  if (errorstatus == SD_OK){
     errorstatus = SD_EnableWideBusOperation(SDIO_BUS_WIDE_4B);
  }else{
	 ERR_printf(errorstatus);
  }
/////////////////////////////////////////////////////////////
//	SD_ReadBlock(test_buf, 0, 512);
  
  u16 i=0;
  for(;i<1024;i++){
	test_buf[i] = 0xEE;
  }
    
 //   while(SD_GetTransferState());
    //sd_write_blocks(test_buf, 0, 2);
    printf("==============================================\n");
 //       sd_write_blocks(test_buf, 1, 1);
//    sd_printf("test_buf addr = %d\n",(u32)test_buf);
//    if((u32)test_buf&0x3){
//		 i= (u32)test_buf & 0x3;
//		sd_printf("test_buf i = %d\n",(u32)i);
//	}
  
    sd_read_blocks(test_buf, 0, 1);
    sd_puthex((const char *)test_buf, 512*2);

//	sd_write_blocks(test_buf, 1, 1);
     
//   SD_WaitWriteOperation();
//      sdio_read_blocks(test_buf, 512, 512, 1);
//    sd_puthex((const char *)test_buf, 512);
/////////////////////////////////////////////////////////////  
  
  
  
  return(errorstatus);
}  
  
  
  
SD_Error SD_ProcessIRQSrc(void)
{ 
  if (SDIO_GetITStatus(SDIO_IT_DATAEND) != RESET)
  {
    TransferError = SD_OK;
    SDIO_ClearITPendingBit(SDIO_IT_DATAEND);
    TransferISEnd = 1;
  }  
  else if (SDIO_GetITStatus(SDIO_IT_DCRCFAIL) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_DCRCFAIL);
    TransferError = SD_DATA_CRC_FAIL;
	ERR_printf(TransferError);
  }
  else if (SDIO_GetITStatus(SDIO_IT_DTIMEOUT) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_DTIMEOUT);
    TransferError = SD_DATA_TIMEOUT;
	ERR_printf(TransferError);
  }
  else if (SDIO_GetITStatus(SDIO_IT_RXOVERR) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_RXOVERR);
    TransferError = SD_RX_OVERRUN;
	ERR_printf(TransferError);
  }
  else if (SDIO_GetITStatus(SDIO_IT_TXUNDERR) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_TXUNDERR);
    TransferError = SD_TX_UNDERRUN;
	ERR_printf(TransferError);
  }
  else if (SDIO_GetITStatus(SDIO_IT_STBITERR) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_STBITERR);
    TransferError = SD_START_BIT_ERR;
	ERR_printf(TransferError);
  }

  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
                SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
                SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
  return(TransferError);
}



void SDIO_IRQHandler(void)
{
	SD_ProcessIRQSrc();
}





/////////////////////////////////////////////////////////////////////////////




SD_Error SD_EnableWideBusOperation(u32 WideMode)
{
  SD_Error errorstatus = SD_OK;

  /*!< MMC Card doesn't support this feature */
  if (SDIO_MULTIMEDIA_CARD == CardType){
    errorstatus = SD_UNSUPPORTED_FEATURE;
	ERR_printf(errorstatus);
    return(errorstatus);
  }else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) ||
			(SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || 
			(SDIO_HIGH_CAPACITY_SD_CARD 	== CardType)){
				
    if (SDIO_BUS_WIDE_8B == WideMode){
      errorstatus = SD_UNSUPPORTED_FEATURE;
	  ERR_printf(errorstatus);
      return(errorstatus);
    }else if (SDIO_BUS_WIDE_4B == WideMode){
      errorstatus = SDEnWideBus(ENABLE);
      if (SD_OK == errorstatus){
        /*!< Configure the SDIO peripheral */
		  sdio_set_high_speed(SDIO_BUS_WIDE_4B); 
		  sd_printf("SDIO BUS Width 4_Bit\n");
      }
    }else{
      errorstatus = SDEnWideBus(DISABLE);
      if (SD_OK == errorstatus){
        /*!< Configure the SDIO peripheral */
		  sdio_set_high_speed(SDIO_BUS_WIDE_1B);
		  sd_printf("SDIO BUS Width 1_Bit\n");
      }
    }
  }
  return(errorstatus);
}
/*
* 设置SD卡总线宽度
*/
static SD_Error SDEnWideBus(FunctionalState NewState)
{
  SD_Error errorstatus = SD_OK;
  u32 scr[2] = {0, 0};
  if (SDIO_GetResponse(SDIO, SDIO_RESP1) & SD_CARD_LOCKED){
    errorstatus = SD_LOCK_UNLOCK_FAILED;
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  /*!< Get SCR Register */
  errorstatus = FindSCR(RCA, scr);

  if (errorstatus != SD_OK){
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  /*!< If wide bus operation to be enabled */
  if (NewState == ENABLE)
  {
    /*!< If requested card supports wide bus operation */
    if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)
    {
      /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
	  sdio_send_cmd(SD_CMD_APP_CMD, (u32) RCA << 16, SDIO_RESPONSE_SHORT);
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
		
      if (errorstatus != SD_OK){
		ERR_printf(errorstatus);
        return(errorstatus);
      }

      /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
	  sdio_send_cmd(SD_CMD_APP_SD_SET_BUSWIDTH, (u32) 0x2, SDIO_RESPONSE_SHORT);
	  
      errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
      if (errorstatus != SD_OK){
		ERR_printf(errorstatus);  
        return(errorstatus);
      }
      return(errorstatus);
    }
    else{
      errorstatus = SD_REQUEST_NOT_APPLICABLE;
	  ERR_printf(errorstatus);
      return(errorstatus);
    }
  }   /*!< If wide bus operation to be disabled */
  else
  {
    /*!< If requested card supports 1 bit mode operation */
    if ((scr[1] & SD_SINGLE_BUS_SUPPORT) != SD_ALLZERO)
    {
      /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
	  sdio_send_cmd(SD_CMD_APP_CMD, (u32)RCA << 16, SDIO_RESPONSE_SHORT);
		
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
      if (errorstatus != SD_OK){
		ERR_printf(errorstatus);
        return(errorstatus);
      }

      /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
	  sdio_send_cmd(SD_CMD_APP_SD_SET_BUSWIDTH, (u32)0x00, SDIO_RESPONSE_SHORT);
	  
      errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
      if (errorstatus != SD_OK){
		ERR_printf(errorstatus);
        return(errorstatus);
      }  
      return(errorstatus);
    }else{
      errorstatus = SD_REQUEST_NOT_APPLICABLE;
	  ERR_printf(errorstatus);
      return(errorstatus);
    }
  }
}

/**
  * @brief  Checks for error conditions for CMD0.
  */
static SD_Error CmdError(void)
{
  SD_Error errorstatus = SD_OK;
  uint32_t timeout;

  timeout = SDIO_CMD0TIMEOUT; /*!< 10000 */

  while ((timeout > 0) && (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) == RESET))
  {
    timeout--;
  }

  if (timeout == 0)
  {
	ERR_printf(SD_CMD_RSP_TIMEOUT);
    errorstatus = SD_CMD_RSP_TIMEOUT;
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R7 response.
  */
static SD_Error CmdResp7Error(void)
{
  SD_Error errorstatus = SD_OK;
  uint32_t status;
  uint32_t timeout = SDIO_CMD0TIMEOUT;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) && (timeout > 0))
  {
    timeout--;
    status = SDIO->STA;
  }
  if ((timeout == 0) || (status & SDIO_FLAG_CTIMEOUT)){
    /*!< Card is not V2.0 complient or card does not support the set voltage range */
    errorstatus = SD_CMD_RSP_TIMEOUT;
	ERR_printf(errorstatus);
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  if (status & SDIO_FLAG_CMDREND){
    /*!< Card is SD V2.0 compliant */
    errorstatus = SD_OK;
    SDIO_ClearFlag(SDIO_FLAG_CMDREND);
    return(errorstatus);
  }
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R1 response.
  */
static SD_Error CmdResp1Error(uint8_t cmd)
{
  SD_Error errorstatus = SD_OK;
  uint32_t status;
  uint32_t response_r1;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
	ERR_printf(errorstatus);
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  /*!< Check response received is of desired command */
  if (SDIO_Get_CommandResponse() != cmd)
  {
    errorstatus = SD_ILLEGAL_CMD;
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  /*!< We have received response, retrieve it for analysis  */
  response_r1 = SDIO_Get_Response(SDIO_RESP1);

  if ((response_r1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
  {
    return(errorstatus);
  }

  if (response_r1 & SD_OCR_ADDR_OUT_OF_RANGE)
  {
    return(SD_ADDR_OUT_OF_RANGE);
  }

  if (response_r1 & SD_OCR_ADDR_MISALIGNED)
  {
    return(SD_ADDR_MISALIGNED);
  }

  if (response_r1 & SD_OCR_BLOCK_LEN_ERR)
  {
    return(SD_BLOCK_LEN_ERR);
  }

  if (response_r1 & SD_OCR_ERASE_SEQ_ERR)
  {
    return(SD_ERASE_SEQ_ERR);
  }

  if (response_r1 & SD_OCR_BAD_ERASE_PARAM)
  {
    return(SD_BAD_ERASE_PARAM);
  }

  if (response_r1 & SD_OCR_WRITE_PROT_VIOLATION)
  {
    return(SD_WRITE_PROT_VIOLATION);
  }

  if (response_r1 & SD_OCR_LOCK_UNLOCK_FAILED)
  {
    return(SD_LOCK_UNLOCK_FAILED);
  }

  if (response_r1 & SD_OCR_COM_CRC_FAILED)
  {
    return(SD_COM_CRC_FAILED);
  }

  if (response_r1 & SD_OCR_ILLEGAL_CMD)
  {
    return(SD_ILLEGAL_CMD);
  }

  if (response_r1 & SD_OCR_CARD_ECC_FAILED)
  {
    return(SD_CARD_ECC_FAILED);
  }

  if (response_r1 & SD_OCR_CC_ERROR)
  {
    return(SD_CC_ERROR);
  }

  if (response_r1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
  {
    return(SD_GENERAL_UNKNOWN_ERROR);
  }

  if (response_r1 & SD_OCR_STREAM_READ_UNDERRUN)
  {
    return(SD_STREAM_READ_UNDERRUN);
  }

  if (response_r1 & SD_OCR_STREAM_WRITE_OVERRUN)
  {
    return(SD_STREAM_WRITE_OVERRUN);
  }

  if (response_r1 & SD_OCR_CID_CSD_OVER_WRIETE)
  {
    return(SD_CID_CSD_OVERWRITE);
  }

  if (response_r1 & SD_OCR_WP_ERASE_SKIP)
  {
    return(SD_WP_ERASE_SKIP);
  }

  if (response_r1 & SD_OCR_CARD_ECC_DISABLED)
  {
    return(SD_CARD_ECC_DISABLED);
  }

  if (response_r1 & SD_OCR_ERASE_RESET)
  {
    return(SD_ERASE_RESET);
  }

  if (response_r1 & SD_OCR_AKE_SEQ_ERROR)
  {
    return(SD_AKE_SEQ_ERROR);
  }
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R3 (OCR) response.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp3Error(void)
{
  SD_Error errorstatus = SD_OK;
  uint32_t status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R2 (CID or CSD) response.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp2Error(void)
{
  SD_Error errorstatus = SD_OK;
  uint32_t status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R6 (RCA) response.
  * @param  cmd: The sent command index.
  * @param  prca: pointer to the variable that will contain the SD card relative 
  *         address RCA. 
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca)
{
  SD_Error errorstatus = SD_OK;
  uint32_t status;
  uint32_t response_r1;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT){
    errorstatus = SD_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
	ERR_printf(errorstatus);
	  return(errorstatus);
  }else if (status & SDIO_FLAG_CCRCFAIL){
    errorstatus = SD_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  /*!< Check response received is of desired command */
  if (SDIO_Get_CommandResponse() != cmd){
    errorstatus = SD_ILLEGAL_CMD;
	ERR_printf(errorstatus);
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  /*!< We have received response, retrieve it.  */
  response_r1 = SDIO_Get_Response(SDIO_RESP1);

  if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
  {
    *prca = (uint16_t) (response_r1 >> 16);
    return(errorstatus);
  }

  if (response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR)
  {
    return(SD_GENERAL_UNKNOWN_ERROR);
  }

  if (response_r1 & SD_R6_ILLEGAL_CMD)
  {
    return(SD_ILLEGAL_CMD);
  }

  if (response_r1 & SD_R6_COM_CRC_FAILED)
  {
    return(SD_COM_CRC_FAILED);
  }

  return(errorstatus);
}





//============================================================//

/*
* 1. 启动SDIO
* 2. 鉴别卡的类型
*/
SD_Error SD_PowerON(void)
{
  volatile SD_Error errorstatus = SD_OK;
  u32 response = 0, count = 0, validvoltage = 0;
  u32 SDType = SD_STD_CAPACITY;

  sdio_set_low_speed(SDIO_BUS_WIDE_1B);

  SDIO_PowerState_ON(SDIO);
  /*!< Enable SDIO Clock */
   __SDIO_ENABLE(SDIO);
  /*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
  sdio_send_cmd(SD_CMD_GO_IDLE_STATE, 0x0, SDIO_RESPONSE_NO);
  errorstatus = CmdError();
  if (errorstatus != SD_OK)
  {
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  /*!< CMD8: SEND_IF_COND ----------------------------------------------------*/
  /*!< Send CMD8 to verify SD card interface operating condition */
  /*!< CMD Response: R7 */
  sdio_send_cmd(SDIO_SEND_IF_COND, SD_CHECK_PATTERN, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp7Error();
  if (errorstatus == SD_OK){
    CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; /*!< SD Card 2.0 */
    SDType = SD_HIGH_CAPACITY;
  }
  else{
    /*!< CMD55 */
	sdio_send_cmd(SD_CMD_APP_CMD, 0x00, SDIO_RESPONSE_SHORT);
    errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
  }
  /*!< CMD55 */
  sdio_send_cmd(SD_CMD_APP_CMD, 0x00, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
  /*!< If errorstatus is Command TimeOut, it is a MMC card */
  /*!< If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch)
     or SD card 1.x */
  if (errorstatus == SD_OK){
    /*!< SD CARD */
    /*!< Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
    while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
    {
      /*!< SEND CMD55 APP_CMD with RCA as 0 */
	  sdio_send_cmd(SD_CMD_APP_CMD, 0x00, SDIO_RESPONSE_SHORT);	
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
      if (errorstatus != SD_OK){
		ERR_printf(errorstatus);
        return(errorstatus);
      }
	  sdio_send_cmd(SD_CMD_SD_APP_OP_COND, SD_VOLTAGE_WINDOW_SD | SDType, SDIO_RESPONSE_SHORT);
      errorstatus = CmdResp3Error();
      if (errorstatus != SD_OK){
        return(errorstatus);
      }
      response = SDIO_Get_Response(SDIO_RESP1);
      validvoltage = (((response >> 31) == 1) ? 1 : 0);
      count++;
    }
    if (count >= SD_MAX_VOLT_TRIAL){
      errorstatus = SD_INVALID_VOLTRANGE;
	  ERR_printf(errorstatus);
      return(errorstatus);
    }
    if (response &= SD_HIGH_CAPACITY){
      CardType = SDIO_HIGH_CAPACITY_SD_CARD;
	  sd_printf(">>>>>  SDHC	CARD<<<<<\n");
    }else{
	  sd_printf(">>>>>  SDSC	CARD<<<<<\n");
	}

  }else{/*!< else MMC Card */
	sd_printf(">>>>>  MMC CARD<<<<<\n");
    ERR_printf(errorstatus);
  }
  return(errorstatus);
}


/*
* 1.读取 CSD  CID 
*/
SD_Error SD_InitializeCards(void)
{
  SD_Error errorstatus = SD_OK;
  u16 rca = 0x01;

  if (SDIO_GetPowerState(SDIO) == SDIO_POWER_STATE_OFF)
  {
    errorstatus = SD_REQUEST_NOT_APPLICABLE;
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
  {
    /*!< Send CMD2 ALL_SEND_CID */
	sdio_send_cmd(SD_CMD_ALL_SEND_CID, 0, SDIO_RESPONSE_LONG);   
    errorstatus = CmdResp2Error();
    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
    CID_Tab[0] = SDIO_GetResponse(SDIO, SDIO_RESP1);
    CID_Tab[1] = SDIO_GetResponse(SDIO, SDIO_RESP2);
    CID_Tab[2] = SDIO_GetResponse(SDIO, SDIO_RESP3);
    CID_Tab[3] = SDIO_GetResponse(SDIO, SDIO_RESP4);
	
	sd_printf("CID TAB:	");
	sd_puthex((const char *)(CID_Tab), 16);
  }
  if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) ||  (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||  (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType)
      ||  (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
    /*!< Send CMD3 SET_REL_ADDR with argument 0 */
    /*!< SD Card publishes its RCA. */
	sdio_send_cmd(SD_CMD_SET_REL_ADDR, 0, SDIO_RESPONSE_SHORT);    
    errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);
    if (SD_OK != errorstatus){
	  ERR_printf(errorstatus);	
      return(errorstatus);
    }
	sd_printf("RCA: %x\n",rca);
  }

  if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
  {
    RCA = rca;
    /*!< Send CMD9 SEND_CSD with argument as card's RCA */
	sdio_send_cmd(SD_CMD_SEND_CSD, (u32)(rca << 16), SDIO_RESPONSE_LONG); 	    
    errorstatus = CmdResp2Error();
    if (SD_OK != errorstatus)
    {
	  ERR_printf(errorstatus);
      return(errorstatus);
    }

    CSD_Tab[0] = SDIO_GetResponse(SDIO, SDIO_RESP1);
    CSD_Tab[1] = SDIO_GetResponse(SDIO, SDIO_RESP2);
    CSD_Tab[2] = SDIO_GetResponse(SDIO, SDIO_RESP3);
    CSD_Tab[3] = SDIO_GetResponse(SDIO, SDIO_RESP4);
	sd_printf("CSD TAB:	");
	sd_puthex((const char *)(CSD_Tab), 16);
  }
  errorstatus = SD_OK; /*!< All cards get intialized */
  return(errorstatus);
}


SD_Error SD_SelectDeselect(uint64_t addr)
{
  SD_Error errorstatus = SD_OK;
  /*!< Send CMD7 SDIO_SEL_DESEL_CARD */
  sdio_send_cmd(SD_CMD_SEL_DESEL_CARD, (u32)addr, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp1Error(SD_CMD_SEL_DESEL_CARD);
  return(errorstatus);
}



/*
* 1.解析CSD 计算卡容量
*/
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
  SD_Error errorstatus = SD_OK;
  u8 tmp = 0;
  cardinfo->CardType = (u8)CardType;
  cardinfo->RCA = (u16)RCA;
  /*!< Byte 0 */
  tmp = (u8)((CSD_Tab[0] & 0xFF000000) >> 24);
  cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
  cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
  cardinfo->SD_csd.Reserved1 = tmp & 0x03;
  /*!< Byte 1 */
  tmp = (u8)((CSD_Tab[0] & 0x00FF0000) >> 16);
  cardinfo->SD_csd.TAAC = tmp;
  /*!< Byte 2 */
  tmp = (u8)((CSD_Tab[0] & 0x0000FF00) >> 8);
  cardinfo->SD_csd.NSAC = tmp;
  /*!< Byte 3 */
  tmp = (u8)(CSD_Tab[0] & 0x000000FF);
  cardinfo->SD_csd.MaxBusClkFrec = tmp;
  /*!< Byte 4 */
  tmp = (u8)((CSD_Tab[1] & 0xFF000000) >> 24);
  cardinfo->SD_csd.CardComdClasses = tmp << 4;
  /*!< Byte 5 */
  tmp = (u8)((CSD_Tab[1] & 0x00FF0000) >> 16);
  
  cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
  cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;


  /*!< Byte 6 */
  tmp = (u8)((CSD_Tab[1] & 0x0000FF00) >> 8);
  cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
  cardinfo->SD_csd.Reserved2 = 0; /*!< Reserved */

  if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0))
  {
    cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;

    /*!< Byte 7 */
    tmp = (u8)(CSD_Tab[1] & 0x000000FF);
    cardinfo->SD_csd.DeviceSize |= (tmp) << 2;

    /*!< Byte 8 */
    tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24);
    cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;

    cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
    cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);

    /*!< Byte 9 */
    tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
    cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
    cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
    /*!< Byte 10 */
    tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);
    cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
    
    cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ;
    cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
    cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
    cardinfo->CardCapacity *= cardinfo->CardBlockSize;
  }
  else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    /*!< Byte 7 */
    tmp = (u8)(CSD_Tab[1] & 0x000000FF);
    cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;
    /*!< Byte 8 */
    tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24);
    cardinfo->SD_csd.DeviceSize |= (tmp << 8);
    /*!< Byte 9 */
    tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.DeviceSize |= (tmp);
    /*!< Byte 10 */
    tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);
    
    cardinfo->CardCapacity = ((uint64_t)cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024;
    cardinfo->CardBlockSize = 512;    
  }


  cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;

  /*!< Byte 11 */
  tmp = (u8)(CSD_Tab[2] & 0x000000FF);
  cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
  cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);

  /*!< Byte 12 */
  tmp = (u8)((CSD_Tab[3] & 0xFF000000) >> 24);
  cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
  cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
  cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;

  /*!< Byte 13 */
  tmp = (u8)((CSD_Tab[3] & 0x00FF0000) >> 16);
  cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
  cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.Reserved3 = 0;
  cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);

  /*!< Byte 14 */
  tmp = (u8)((CSD_Tab[3] & 0x0000FF00) >> 8);
  cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
  cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
  cardinfo->SD_csd.ECC = (tmp & 0x03);

  /*!< Byte 15 */
  tmp = (u8)(CSD_Tab[3] & 0x000000FF);
  cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
  cardinfo->SD_csd.Reserved4 = 1;

  /*!< Byte 0 */
  tmp = (u8)((CID_Tab[0] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ManufacturerID = tmp;

  /*!< Byte 1 */
  tmp = (u8)((CID_Tab[0] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.OEM_AppliID = tmp << 8;

  /*!< Byte 2 */
  tmp = (u8)((CID_Tab[0] & 0x000000FF00) >> 8);
  cardinfo->SD_cid.OEM_AppliID |= tmp;

  /*!< Byte 3 */
  tmp = (u8)(CID_Tab[0] & 0x000000FF);
  cardinfo->SD_cid.ProdName1 = tmp << 24;

  /*!< Byte 4 */
  tmp = (u8)((CID_Tab[1] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ProdName1 |= tmp << 16;

  /*!< Byte 5 */
  tmp = (u8)((CID_Tab[1] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.ProdName1 |= tmp << 8;

  /*!< Byte 6 */
  tmp = (u8)((CID_Tab[1] & 0x0000FF00) >> 8);
  cardinfo->SD_cid.ProdName1 |= tmp;

  /*!< Byte 7 */
  tmp = (u8)(CID_Tab[1] & 0x000000FF);
  cardinfo->SD_cid.ProdName2 = tmp;

  /*!< Byte 8 */
  tmp = (u8)((CID_Tab[2] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ProdRev = tmp;

  /*!< Byte 9 */
  tmp = (u8)((CID_Tab[2] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.ProdSN = tmp << 24;

  /*!< Byte 10 */
  tmp = (u8)((CID_Tab[2] & 0x0000FF00) >> 8);
  cardinfo->SD_cid.ProdSN |= tmp << 16;

  /*!< Byte 11 */
  tmp = (u8)(CID_Tab[2] & 0x000000FF);
  cardinfo->SD_cid.ProdSN |= tmp << 8;

  /*!< Byte 12 */
  tmp = (u8)((CID_Tab[3] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ProdSN |= tmp;

  /*!< Byte 13 */
  tmp = (u8)((CID_Tab[3] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
  cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;

  /*!< Byte 14 */
  tmp = (u8)((CID_Tab[3] & 0x0000FF00) >> 8);
  cardinfo->SD_cid.ManufactDate |= tmp;

  /*!< Byte 15 */
  tmp = (u8)(CID_Tab[3] & 0x000000FF);
  cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
  cardinfo->SD_cid.Reserved2 = 1;
  
  
//  sd_printf("PartBlockRead  :\t%d\n",cardinfo->SD_csd.PartBlockRead);
//  sd_printf("WrBlockMisalign:\t%d\n",cardinfo->SD_csd.WrBlockMisalign);
//  sd_printf("RdBlockMisalign:\t%d\n",cardinfo->SD_csd.RdBlockMisalign);
//  sd_printf("CardComdClasses:\t%d\n",cardinfo->SD_csd.CardComdClasses);
  sd_printf("RdBlockLen     :\t%d\n",cardinfo->SD_csd.RdBlockLen);
  sd_printf("CardBlockSize  :\t%d\n",cardinfo->CardBlockSize);
  sd_printf("CardCapacity   :\t%lld\n",cardinfo->CardCapacity);

  sd_printf("ManufacturerID :\t0x%x\n",cardinfo->SD_cid.ManufacturerID);		//制造商ID
//  sd_printf("OEM_AppliID    :\t0x%x\n",cardinfo->SD_cid.OEM_AppliID);			//应用ID
//  sd_printf("ProdName1      :\t0x%x\n",cardinfo->SD_cid.ProdName1);			//产品name
//  sd_printf("ProdName2      :\t0x%x\n",cardinfo->SD_cid.ProdName2);
//  sd_printf("ProdRev        :\t0x%x\n",cardinfo->SD_cid.ProdRev);				//产品版本
//  sd_printf("ProdSN         :\t0x%x\n",cardinfo->SD_cid.ProdSN);  			//产品序列号
//  sd_printf("ManufactDate   :\t0x%x\n",cardinfo->SD_cid.ManufactDate);  		//制造日期
  
  sd_printf("Date: %d - %d \n",(((cardinfo->SD_cid.ManufactDate>>4)&0xff)+2000),
								 (cardinfo->SD_cid.ManufactDate)&0xf);
  return(errorstatus);
}





__sdio_sd_obj sdio_sd_obj = {

	.init 	= SD_Init,
	.read 	= NULL,
	.write 	= NULL,
	.io_ctr = NULL,
	.status = NULL,
};




  
 



