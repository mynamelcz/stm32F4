#include "sdio_sd.h"

#include "stm32f4xx_ll.h"

#if 1

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
  SDIO_ReadFIFO(SDIO);
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

SD_Error SD_SelectDeselect(uint64_t addr)
{
  SD_Error errorstatus = SD_OK;
  /*!< Send CMD7 SDIO_SEL_DESEL_CARD */
  sdio_send_cmd(SD_CMD_SEL_DESEL_CARD, (u32)addr, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp1Error(SD_CMD_SEL_DESEL_CARD);
  return(errorstatus);
}


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
    }
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




SD_Error SD_ReadBlock(u8 *readbuff, uint64_t ReadAddr, u16 BlockSize)
{
  SD_Error errorstatus = SD_OK;
#if defined (SD_POLLING_MODE) 
  u32 count = 0, *tempbuff = (u32 *)readbuff;
#endif

  TransferError = SD_OK;
  TransferISEnd = 0;
  StopCondition = 0;

  SDIO->DCTRL = 0x0;

#if defined (SD_DMA_MODE)
  __SDIO_ENABLE_IT(SDIO,SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);
  __SDIO_DMA_ENABLE(SDIO);
  SD_LowLevel_DMA_RxConfig((u32 *)readbuff, BlockSize);
#endif

  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    ReadAddr /= 512;
  }


  
  /* Set Block Size for Card */ 
  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)BlockSize, SDIO_RESPONSE_SHORT);
  
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

  if (SD_OK != errorstatus){
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  sdio_data_cfg(BlockSize, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_DATABLOCK_SIZE_512B);
  
  /*!< Send CMD17 READ_SINGLE_BLOCK */
  sdio_send_cmd(SD_CMD_READ_SINGLE_BLOCK, (u32)ReadAddr, SDIO_RESPONSE_SHORT);
 
  errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);
  if (errorstatus != SD_OK){
	ERR_printf(errorstatus);
    return(errorstatus);
  }

#if defined (SD_POLLING_MODE)  
  /*!< In case of single block transfer, no need of stop transfer at all.*/
  /*!< Polling mode */
  while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (sdio_get_state(SDIO_FLAG_RXFIFOHF) != RESET)
    {
      for (count = 0; count < 8; count++)
      {
        *(tempbuff + count) = sdio_read_data();
      }
      tempbuff += 8;
    }
  }

  if (sdio_get_state(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_DCRCFAIL) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_RXOVERR) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_RXOVERR);
    errorstatus = SD_RX_OVERRUN;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_STBITERR) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }
  count = SD_DATATIMEOUT;
  while ((sdio_get_state(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
  {
    *tempbuff = sdio_read_data();
    tempbuff++;
    count--;
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

#endif

  return(errorstatus);
}


SD_Error SD_ReadMultiBlocks(u8 *readbuff, uint64_t ReadAddr, u16 BlockSize, u32 NumberOfBlocks)
{
  SD_Error errorstatus = SD_OK;
  TransferError = SD_OK;
  TransferISEnd = 0;
  StopCondition = 1;
	
  SDIO->DCTRL = 0x0;



	
	
  __SDIO_ENABLE_IT(SDIO,SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR); 
  __SDIO_DMA_ENABLE(SDIO);
  SD_LowLevel_DMA_RxConfig((u32 *)readbuff, (NumberOfBlocks * BlockSize));
  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    ReadAddr /= 512;
  }
  /*!< Set Block Size for Card */
  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)BlockSize, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus){
	ERR_printf(errorstatus);
    return(errorstatus);
  }
    
  sdio_data_cfg(NumberOfBlocks * BlockSize, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_DATABLOCK_SIZE_512B);
  
  /*!< Send CMD18 READ_MULT_BLOCK with argument data address */
  sdio_send_cmd(SD_CMD_READ_MULT_BLOCK, (u32)ReadAddr, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);
  if (errorstatus != SD_OK){
	ERR_printf(errorstatus);
    return(errorstatus);
  }
  
  sd_printf("TransferEnd: %d    line:%d\n",TransferISEnd, __LINE__);

  
  return(errorstatus);
}



/**
  * @brief  Aborts an ongoing data transfer.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_StopTransfer(void)
{
  SD_Error errorstatus = SD_OK;
  /*!< Send CMD12 STOP_TRANSMISSION  */
  sdio_send_cmd(SD_CMD_STOP_TRANSMISSION, (u32)0, SDIO_RESPONSE_SHORT);    
  errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
  return(errorstatus);
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished. 
  *         This function should be called after SDIO_ReadMultiBlocks() function
  *         to insure that all data sent by the card are already transferred by 
  *         the DMA controller.
  * @param  None.
  * @retval SD_Error: SD Card Error code.
  */
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
	sd_printf("timeout:%d  line:%d\n",timeout, __LINE__);
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

/**
  * @brief  Returns the current card's status.
  * @param  pcardstatus: pointer to the buffer that will contain the SD card 
  *         status (Card Status register).
  * @retval SD_Error: SD Card Error code.
  */
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
/**
  * @brief  Returns the current card's state.
  * @param  None
  * @retval SDCardState: SD Card Error or SD Card Current State.
  */
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

SD_Error sdio_read_blocks(u8 *readbuff, uint64_t ReadAddr, u16 BlockSize, u32 NumberOfBlocks)
{
	uint32_t timeout;
	SD_Error Status;
	timeout = SD_DATATIMEOUT;
	if(NumberOfBlocks==0)return SD_OK;
	if(NumberOfBlocks == 1){
		Status = SD_ReadBlock(readbuff, ReadAddr, BlockSize);
	}else{
		Status = SD_ReadMultiBlocks(readbuff, ReadAddr, BlockSize, NumberOfBlocks);
	}
	
	

	if(Status == SD_OK){
		Status = SD_WaitReadOperation();
		if(Status == SD_OK){
			while((SD_GetStatus() != SD_TRANSFER_OK)&&(--timeout));
			if(timeout){
				return SD_OK;
			}
		}		
	}
	ERR_printf(Status);
	return Status;
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
  
  	
   sd_printf("SDIO_FLAG_RXACT:%x  line:%d\n",(SDIO->STA &(1<<13)), __LINE__);
   while(1);	
  
    sdio_read_blocks(test_buf, 0, 512, 2);
    sd_puthex((const char *)test_buf, 1024);


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
	sd_printf("TransferEnd: %d\n",TransferISEnd);
	sd_printf("^^^^^^SDIO_IT_DATAEND\n");
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












__sdio_sd_obj sdio_sd_obj = {

	.init 	= SD_Init,
	.read 	= NULL,
	.write 	= NULL,
	.io_ctr = NULL,
	.status = NULL,
};




  
 
#if 0




/** @defgroup STM324x7I_EVAL_SDIO_SD_Private_Functions
  * @{
  */  

/**
  * @brief  DeInitializes the SDIO interface.
  * @param  None
  * @retval None
  */
void SD_DeInit(void)
{ 
  SD_LowLevel_DeInit();
}





/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for data 
  *         transfer).
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_Init(void)
{
  volatile SD_Error errorstatus = SD_OK;
  
  /* SDIO Peripheral Low Level Init */
   SD_LowLevel_Init();
  
  //SDIO_DeInit();

  errorstatus = SD_PowerON();

  if (errorstatus != SD_OK)
  {
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return(errorstatus);
  }

  errorstatus = SD_InitializeCards();

  if (errorstatus != SD_OK)
  {
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return(errorstatus);
  }

  /*!< Configure the SDIO peripheral */
  /*!< SDIO_CK = SDIOCLK / (SDIO_TRANSFER_CLK_DIV + 2) */
  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
//  SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV;
//  SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
//  SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
//  SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
//  SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
//  SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
//  SDIO_Init(&SDIO_InitStructure);
  
    sdio_set_high_speed(SDIO_BUS_WIDE_1B);

  /*----------------- Read CSD/CID MSD registers ------------------*/
  errorstatus = SD_GetCardInfo(&SDCardInfo);

  if (errorstatus == SD_OK)
  {
    /*----------------- Select Card --------------------------------*/
    errorstatus = SD_SelectDeselect((u32) (SDCardInfo.RCA << 16));
  }

  if (errorstatus == SD_OK)
  {
    errorstatus = SD_EnableWideBusOperation(SDIO_BUS_WIDE_4B);
  }  

  return(errorstatus);
}

/**
  * @brief  Gets the cuurent sd card data transfer status.
  * @param  None
  * @retval SDTransferState: Data Transfer state.
  *   This value can be: 
  *        - SD_TRANSFER_OK: No data transfer is acting
  *        - SD_TRANSFER_BUSY: Data transfer is acting
  */
SDTransferState SD_GetStatus(void)
{
  SDCardState cardstate =  SD_CARD_TRANSFER;

  cardstate = SD_GetState();
  
  if (cardstate == SD_CARD_TRANSFER)
  {
    return(SD_TRANSFER_OK);
  }
  else if(cardstate == SD_CARD_ERROR)
  {
    return (SD_TRANSFER_ERROR);
  }
  else
  {
    return(SD_TRANSFER_BUSY);
  }
}

/**
  * @brief  Returns the current card's state.
  * @param  None
  * @retval SDCardState: SD Card Error or SD Card Current State.
  */
SDCardState SD_GetState(void)
{
  u32 resp1 = 0;
  
  if(SD_Detect()== SD_PRESENT)
  {
    if (SD_SendStatus(&resp1) != SD_OK)
    {
      return SD_CARD_ERROR;
    }
    else
    {
      return (SDCardState)((resp1 >> 9) & 0x0F);
    }
  }
  else
  {
    return SD_CARD_ERROR;
  }
}

/**
 * @brief  Detect if SD card is correctly plugged in the memory slot.
 * @param  None
 * @retval Return if SD is detected or not
 */
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

/**
  * @brief  Enquires cards about their operating voltage and configures 
  *   clock controls.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_PowerON(void)
{
  volatile SD_Error errorstatus = SD_OK;
  u32 response = 0, count = 0, validvoltage = 0;
  u32 SDType = SD_STD_CAPACITY;

  /*!< Power ON Sequence -----------------------------------------------------*/
  /*!< Configure the SDIO peripheral */
  /*!< SDIO_CK = SDIOCLK / (SDIO_INIT_CLK_DIV + 2) */
  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
  /*!< SDIO_CK for initialization should not exceed 400 KHz */  
//  SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;
//  SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
//  SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
//  SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
//  SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
//  SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
//  SDIO_Init(&SDIO_InitStructure);
    sdio_set_low_speed(SDIO_BUS_WIDE_1B);
  /*!< Set Power State to ON */
  SDIO_SetPowerState(SDIO_PowerState_ON);

  /*!< Enable SDIO Clock */
  SDIO_ClockCmd(ENABLE);

  /*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
  /*!< No CMD response required */
//  SDIO_CmdInitStructure.SDIO_Argument = 0x0;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

	sdio_send_cmd(SD_CMD_GO_IDLE_STATE, 0x0, SDIO_RESPONSE_NO);


  errorstatus = CmdError();

  if (errorstatus != SD_OK)
  {
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return(errorstatus);
  }

  /*!< CMD8: SEND_IF_COND ----------------------------------------------------*/
  /*!< Send CMD8 to verify SD card interface operating condition */
  /*!< Argument: - [31:12]: Reserved (shall be set to '0')
               - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
               - [7:0]: Check Pattern (recommended 0xAA) */
  /*!< CMD Response: R7 */
//  SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SDIO_SEND_IF_COND, SD_CHECK_PATTERN, SDIO_RESPONSE_SHORT);
  
  errorstatus = CmdResp7Error();

  if (errorstatus == SD_OK)
  {
    CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; /*!< SD Card 2.0 */
    SDType = SD_HIGH_CAPACITY;
  }
  else
  {
    /*!< CMD55 */
//    SDIO_CmdInitStructure.SDIO_Argument = 0x00;
//    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
//    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//    SDIO_SendCommand(&SDIO_CmdInitStructure);
	  
	sdio_send_cmd(SD_CMD_APP_CMD, 0x00, SDIO_RESPONSE_SHORT);
	  
    errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
  }
  /*!< CMD55 */
//  SDIO_CmdInitStructure.SDIO_Argument = 0x00;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);
  
  sdio_send_cmd(SD_CMD_APP_CMD, 0x00, SDIO_RESPONSE_SHORT);
  
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

  /*!< If errorstatus is Command TimeOut, it is a MMC card */
  /*!< If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch)
     or SD card 1.x */
  if (errorstatus == SD_OK)
  {
    /*!< SD CARD */
    /*!< Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
    while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
    {

      /*!< SEND CMD55 APP_CMD with RCA as 0 */
//      SDIO_CmdInitStructure.SDIO_Argument = 0x00;
//      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
//      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//      SDIO_SendCommand(&SDIO_CmdInitStructure);

	 sdio_send_cmd(SD_CMD_APP_CMD, 0x00, SDIO_RESPONSE_SHORT);	
		
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
//      SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType;
//      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
//      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//      SDIO_SendCommand(&SDIO_CmdInitStructure);
	  
	  
	  sdio_send_cmd(SD_CMD_SD_APP_OP_COND, SD_VOLTAGE_WINDOW_SD | SDType, SDIO_RESPONSE_SHORT);

      errorstatus = CmdResp3Error();
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }

      response = SDIO_GetResponse(SDIO, SDIO_RESP1);
      validvoltage = (((response >> 31) == 1) ? 1 : 0);
      count++;
    }
    if (count >= SD_MAX_VOLT_TRIAL)
    {
      errorstatus = SD_INVALID_VOLTRANGE;
      return(errorstatus);
    }

    if (response &= SD_HIGH_CAPACITY)
    {
      CardType = SDIO_HIGH_CAPACITY_SD_CARD;
    }

  }/*!< else MMC Card */

  return(errorstatus);
}

/**
  * @brief  Turns the SDIO output signals off.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_PowerOFF(void)
{
  SD_Error errorstatus = SD_OK;

  /*!< Set Power State to OFF */
  SDIO_SetPowerState(SDIO_PowerState_OFF);

  return(errorstatus);
}

/**
  * @brief  Intialises all cards or single card as the case may be Card(s) come 
  *         into standby state.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_InitializeCards(void)
{
  SD_Error errorstatus = SD_OK;
  u16 rca = 0x01;

  if (SDIO_GetPowerState(SDIO) == SDIO_PowerState_OFF)
  {
    errorstatus = SD_REQUEST_NOT_APPLICABLE;
    return(errorstatus);
  }

  if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
  {
    /*!< Send CMD2 ALL_SEND_CID */
//    SDIO_CmdInitStructure.SDIO_Argument = 0x0;
//    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
//    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
//    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//    SDIO_SendCommand(&SDIO_CmdInitStructure);

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
  }
  if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) ||  (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||  (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType)
      ||  (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
    /*!< Send CMD3 SET_REL_ADDR with argument 0 */
    /*!< SD Card publishes its RCA. */
//    SDIO_CmdInitStructure.SDIO_Argument = 0x00;
//    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;
//    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//    SDIO_SendCommand(&SDIO_CmdInitStructure);

	sdio_send_cmd(SD_CMD_SET_REL_ADDR, 0, SDIO_RESPONSE_SHORT);  
	  
    errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
  }

  if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
  {
    RCA = rca;

    /*!< Send CMD9 SEND_CSD with argument as card's RCA */
//    SDIO_CmdInitStructure.SDIO_Argument = (u32)(rca << 16);
//    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
//    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
//    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//    SDIO_SendCommand(&SDIO_CmdInitStructure);

	sdio_send_cmd(SD_CMD_SEND_CSD, (u32)(rca << 16), SDIO_RESPONSE_LONG); 	  
	  
    errorstatus = CmdResp2Error();

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }

    CSD_Tab[0] = SDIO_GetResponse(SDIO, SDIO_RESP1);
    CSD_Tab[1] = SDIO_GetResponse(SDIO, SDIO_RESP2);
    CSD_Tab[2] = SDIO_GetResponse(SDIO, SDIO_RESP3);
    CSD_Tab[3] = SDIO_GetResponse(SDIO, SDIO_RESP4);
  }

  errorstatus = SD_OK; /*!< All cards get intialized */

  return(errorstatus);
}

/**
  * @brief  Returns information about specific card.
  * @param  cardinfo: pointer to a SD_CardInfo structure that contains all SD card 
  *         information.
  * @retval SD_Error: SD Card Error code.
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
  
  return(errorstatus);
}

/**
  * @brief  Enables wide bus opeartion for the requeseted card if supported by 
  *         card.
  * @param  WideMode: Specifies the SD card wide bus mode. 
  *   This parameter can be one of the following values:
  *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
  *     @arg SDIO_BusWide_4b: 4-bit data transfer
  *     @arg SDIO_BusWide_1b: 1-bit data transfer
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_GetCardStatus(SD_CardStatus *cardstatus)
{
  SD_Error errorstatus = SD_OK;
  u8 tmp = 0;

  errorstatus = SD_SendSDStatus((u32 *)SDSTATUS_Tab);

  if (errorstatus  != SD_OK)
  {
    return(errorstatus);
  }

  /*!< Byte 0 */
  tmp = (u8)((SDSTATUS_Tab[0] & 0xC0) >> 6);
  cardstatus->DAT_BUS_WIDTH = tmp;

  /*!< Byte 0 */
  tmp = (u8)((SDSTATUS_Tab[0] & 0x20) >> 5);
  cardstatus->SECURED_MODE = tmp;

  /*!< Byte 2 */
  tmp = (u8)((SDSTATUS_Tab[2] & 0xFF));
  cardstatus->SD_CARD_TYPE = tmp << 8;

  /*!< Byte 3 */
  tmp = (u8)((SDSTATUS_Tab[3] & 0xFF));
  cardstatus->SD_CARD_TYPE |= tmp;

  /*!< Byte 4 */
  tmp = (u8)(SDSTATUS_Tab[4] & 0xFF);
  cardstatus->SIZE_OF_PROTECTED_AREA = tmp << 24;

  /*!< Byte 5 */
  tmp = (u8)(SDSTATUS_Tab[5] & 0xFF);
  cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 16;

  /*!< Byte 6 */
  tmp = (u8)(SDSTATUS_Tab[6] & 0xFF);
  cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 8;

  /*!< Byte 7 */
  tmp = (u8)(SDSTATUS_Tab[7] & 0xFF);
  cardstatus->SIZE_OF_PROTECTED_AREA |= tmp;

  /*!< Byte 8 */
  tmp = (u8)((SDSTATUS_Tab[8] & 0xFF));
  cardstatus->SPEED_CLASS = tmp;

  /*!< Byte 9 */
  tmp = (u8)((SDSTATUS_Tab[9] & 0xFF));
  cardstatus->PERFORMANCE_MOVE = tmp;

  /*!< Byte 10 */
  tmp = (u8)((SDSTATUS_Tab[10] & 0xF0) >> 4);
  cardstatus->AU_SIZE = tmp;

  /*!< Byte 11 */
  tmp = (u8)(SDSTATUS_Tab[11] & 0xFF);
  cardstatus->ERASE_SIZE = tmp << 8;

  /*!< Byte 12 */
  tmp = (u8)(SDSTATUS_Tab[12] & 0xFF);
  cardstatus->ERASE_SIZE |= tmp;

  /*!< Byte 13 */
  tmp = (u8)((SDSTATUS_Tab[13] & 0xFC) >> 2);
  cardstatus->ERASE_TIMEOUT = tmp;

  /*!< Byte 13 */
  tmp = (u8)((SDSTATUS_Tab[13] & 0x3));
  cardstatus->ERASE_OFFSET = tmp;
 
  return(errorstatus);
}

/**
  * @brief  Enables wide bus opeartion for the requeseted card if supported by 
  *         card.
  * @param  WideMode: Specifies the SD card wide bus mode. 
  *   This parameter can be one of the following values:
  *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
  *     @arg SDIO_BusWide_4b: 4-bit data transfer
  *     @arg SDIO_BusWide_1b: 1-bit data transfer
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_EnableWideBusOperation(u32 WideMode)
{
  SD_Error errorstatus = SD_OK;

  /*!< MMC Card doesn't support this feature */
  if (SDIO_MULTIMEDIA_CARD == CardType)
  {
    errorstatus = SD_UNSUPPORTED_FEATURE;
    return(errorstatus);
  }
  else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
    if (SDIO_BUS_WIDE_8B == WideMode)
    {
      errorstatus = SD_UNSUPPORTED_FEATURE;
      return(errorstatus);
    }
    else if (SDIO_BUS_WIDE_4B == WideMode)
    {
      errorstatus = SDEnWideBus(ENABLE);

      if (SD_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
//        SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
//        SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
//        SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
//        SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
//        SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
//        SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
//        SDIO_Init(&SDIO_InitStructure);
		  
		  sdio_set_high_speed(SDIO_BUS_WIDE_4B);  
		  
      }
    }
    else
    {
      errorstatus = SDEnWideBus(DISABLE);

      if (SD_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
//        SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
//        SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
//        SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
//        SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
//        SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
//        SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
//        SDIO_Init(&SDIO_InitStructure);
		  sdio_set_high_speed(SDIO_BUS_WIDE_1B);  
      }
    }
  }

  return(errorstatus);
}

/**
  * @brief  Selects od Deselects the corresponding card.
  * @param  addr: Address of the Card to be selected.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_SelectDeselect(uint64_t addr)
{
  SD_Error errorstatus = SD_OK;

  /*!< Send CMD7 SDIO_SEL_DESEL_CARD */
//  SDIO_CmdInitStructure.SDIO_Argument =  (u32)addr;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

	sdio_send_cmd(SD_CMD_SEL_DESEL_CARD, (u32)addr, SDIO_RESPONSE_SHORT);
  errorstatus = CmdResp1Error(SD_CMD_SEL_DESEL_CARD);

  return(errorstatus);
}

/**
  * @brief  Allows to read one block from a specified address in a card. The Data
  *         transfer can be managed by DMA mode or Polling mode. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.            
  * @param  readbuff: pointer to the buffer that will contain the received data
  * @param  ReadAddr: Address from where data are to be read.  
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_ReadBlock(u8 *readbuff, uint64_t ReadAddr, u16 BlockSize)
{
  SD_Error errorstatus = SD_OK;
#if defined (SD_POLLING_MODE) 
  u32 count = 0, *tempbuff = (u32 *)readbuff;
#endif

  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 0;

  SDIO->DCTRL = 0x0;

#if defined (SD_DMA_MODE)
  __SDIO_ENABLE_IT(SDIO,SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR);
  __SDIO_ENABLE(SDIO);
  SD_LowLevel_DMA_RxConfig((u32 *)readbuff, BlockSize);
#endif

  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    ReadAddr /= 512;
  }

  /* Set Block Size for Card */ 
//  SDIO_CmdInitStructure.SDIO_Argument = (u32) BlockSize;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)BlockSize, SDIO_RESPONSE_SHORT);
  
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }

//  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
//  SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
//  SDIO_DataInitStructure.SDIO_DataBlockSize = (u32) 9 << 4;
//  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
//  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
//  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
//  SDIO_DataConfig(&SDIO_DataInitStructure);

  sdio_data_cfg(BlockSize, SDIO_TRANSFER_DIR_TO_SDIO);
  
  /*!< Send CMD17 READ_SINGLE_BLOCK */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32)ReadAddr;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_READ_SINGLE_BLOCK, (u32)ReadAddr, SDIO_RESPONSE_SHORT);
  
  errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

#if defined (SD_POLLING_MODE)  
  /*!< In case of single block transfer, no need of stop transfer at all.*/
  /*!< Polling mode */
  while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (sdio_get_state(SDIO_FLAG_RXFIFOHF) != RESET)
    {
      for (count = 0; count < 8; count++)
      {
        *(tempbuff + count) = sdio_read_data();
      }
      tempbuff += 8;
    }
  }

  if (sdio_get_state(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_DCRCFAIL) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_RXOVERR) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_RXOVERR);
    errorstatus = SD_RX_OVERRUN;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_STBITERR) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }
  count = SD_DATATIMEOUT;
  while ((sdio_get_state(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
  {
    *tempbuff = sdio_read_data();
    tempbuff++;
    count--;
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

#endif

  return(errorstatus);
}

/**
  * @brief  Allows to read blocks from a specified address  in a card.  The Data
  *         transfer can be managed by DMA mode or Polling mode. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.   
  * @param  readbuff: pointer to the buffer that will contain the received data.
  * @param  ReadAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_ReadMultiBlocks(u8 *readbuff, uint64_t ReadAddr, u16 BlockSize, u32 NumberOfBlocks)
{
  SD_Error errorstatus = SD_OK;
  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 1;
	
  SDIO->DCTRL = 0x0;

  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  SD_LowLevel_DMA_RxConfig((u32 *)readbuff, (NumberOfBlocks * BlockSize));
  SDIO_DMACmd(ENABLE);

  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    ReadAddr /= 512;
  }

  /*!< Set Block Size for Card */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32) BlockSize;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)BlockSize, SDIO_RESPONSE_SHORT);
 
  
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
    
//  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
//  SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
//  SDIO_DataInitStructure.SDIO_DataBlockSize = (u32) 9 << 4;
//  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
//  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
//  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
//  SDIO_DataConfig(&SDIO_DataInitStructure);

  sdio_data_cfg(NumberOfBlocks * BlockSize, SDIO_TRANSFER_DIR_TO_SDIO);
  
  /*!< Send CMD18 READ_MULT_BLOCK with argument data address */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32)ReadAddr;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_READ_MULT_BLOCK, (u32)ReadAddr, SDIO_RESPONSE_SHORT);
 
  
  errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  return(errorstatus);
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished. 
  *         This function should be called after SDIO_ReadMultiBlocks() function
  *         to insure that all data sent by the card are already transferred by 
  *         the DMA controller.
  * @param  None.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_WaitReadOperation(void)
{
  SD_Error errorstatus = SD_OK;
  u32 timeout;

  timeout = SD_DATATIMEOUT;
  
  while ((DMAEndOfTransfer == 0x00) && (TransferEnd == 0) && (TransferError == SD_OK) && (timeout > 0))
  {
    timeout--;
  }
  
  DMAEndOfTransfer = 0x00;

  timeout = SD_DATATIMEOUT;
  
  while(((SDIO->STA & SDIO_FLAG_RXACT)) && (timeout > 0))
  {
    timeout--;  
  }

  if (StopCondition == 1)
  {
    errorstatus = SD_StopTransfer();
    StopCondition = 0;
  }
  
  if ((timeout == 0) && (errorstatus == SD_OK))
  {
    errorstatus = SD_DATA_TIMEOUT;
  }
  
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

  if (TransferError != SD_OK)
  {
    return(TransferError);
  }
  else
  {
    return(errorstatus);  
  }
}

/**
  * @brief  Allows to write one block starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.      
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  WriteAddr: Address from where data are to be read.   
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_WriteBlock(u8 *writebuff, uint64_t WriteAddr, u16 BlockSize)
{
  SD_Error errorstatus = SD_OK;

#if defined (SD_POLLING_MODE)
  u32 bytestransferred = 0, count = 0, restwords = 0;
  u32 *tempbuff = (u32 *)writebuff;
#endif

  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 0;

  SDIO->DCTRL = 0x0;

#if defined (SD_DMA_MODE)
  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  SD_LowLevel_DMA_TxConfig((u32 *)writebuff, BlockSize);
  SDIO_DMACmd(ENABLE);
#endif

  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    WriteAddr /= 512;
  }

  /* Set Block Size for Card */ 
//  SDIO_CmdInitStructure.SDIO_Argument = (u32) BlockSize;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)BlockSize, SDIO_RESPONSE_SHORT);  
  
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }

  /*!< Send CMD24 WRITE_SINGLE_BLOCK */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32)WriteAddr;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_WRITE_SINGLE_BLOCK, (u32)WriteAddr, SDIO_RESPONSE_SHORT);  
  
  
  errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

//  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
//  SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
//  SDIO_DataInitStructure.SDIO_DataBlockSize = (u32) 9 << 4;
//  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
//  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
//  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
//  SDIO_DataConfig(&SDIO_DataInitStructure);

  sdio_data_cfg(BlockSize, SDIO_TRANSFER_DIR_TO_CARD);
  
  
  /*!< In case of single data block transfer no need of stop command at all */
#if defined (SD_POLLING_MODE) 
  while (!(SDIO->STA & (SDIO_FLAG_DBCKEND | SDIO_FLAG_TXUNDERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR)))
  {
    if (sdio_get_state(SDIO_FLAG_TXFIFOHE) != RESET)
    {
      if ((512 - bytestransferred) < 32)
      {
        restwords = ((512 - bytestransferred) % 4 == 0) ? ((512 - bytestransferred) / 4) : (( 512 -  bytestransferred) / 4 + 1);
        for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
        {
          SDIO_WriteData(*tempbuff);
        }
      }
      else
      {
        for (count = 0; count < 8; count++)
        {
          SDIO_WriteData(*(tempbuff + count));
        }
        tempbuff += 8;
        bytestransferred += 32;
      }
    }
  }
  if (sdio_get_state(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_DCRCFAIL) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_TXUNDERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);
    errorstatus = SD_TX_UNDERRUN;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_STBITERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }
  
#endif
  
  return(errorstatus);
}

/**
  * @brief  Allows to write blocks starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode only. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.     
  * @param  WriteAddr: Address from where data are to be read.
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_WriteMultiBlocks(u8 *writebuff, uint64_t WriteAddr, u16 BlockSize, u32 NumberOfBlocks)
{
  SD_Error errorstatus = SD_OK;

  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 1;
  SDIO->DCTRL = 0x0;

  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_TXUNDERR | SDIO_IT_STBITERR, ENABLE);
  SD_LowLevel_DMA_TxConfig((u32 *)writebuff, (NumberOfBlocks * BlockSize));
  SDIO_DMACmd(ENABLE);

  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    WriteAddr /= 512;
  }

  /* Set Block Size for Card */ 
//  SDIO_CmdInitStructure.SDIO_Argument = (u32) BlockSize;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)BlockSize, SDIO_RESPONSE_SHORT);   
  
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
  
  /*!< To improve performance */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32) (RCA << 16);
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_APP_CMD, (u32) (RCA << 16), SDIO_RESPONSE_SHORT);   
  
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  /*!< To improve performance */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32)NumberOfBlocks;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SET_BLOCK_COUNT, (u32) NumberOfBlocks, SDIO_RESPONSE_SHORT);    
  
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }


  /*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32)WriteAddr;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_WRITE_MULT_BLOCK, (u32)WriteAddr, SDIO_RESPONSE_SHORT);   
  
  errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);

  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }

//  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
//  SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
//  SDIO_DataInitStructure.SDIO_DataBlockSize = (u32) 9 << 4;
//  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
//  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
//  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
//  SDIO_DataConfig(&SDIO_DataInitStructure);

  sdio_data_cfg(NumberOfBlocks * BlockSize, SDIO_TRANSFER_DIR_TO_CARD);
  
  return(errorstatus);
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished. 
  *         This function should be called after SDIO_WriteBlock() and
  *         SDIO_WriteMultiBlocks() function to insure that all data sent by the 
  *         card are already transferred by the DMA controller.        
  * @param  None.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_WaitWriteOperation(void)
{
  SD_Error errorstatus = SD_OK;
  u32 timeout;

  timeout = SD_DATATIMEOUT;
  
  while ((DMAEndOfTransfer == 0x00) && (TransferEnd == 0) && (TransferError == SD_OK) && (timeout > 0))
  {
    timeout--;
  }
  
  DMAEndOfTransfer = 0x00;

  timeout = SD_DATATIMEOUT;
  
  while(((SDIO->STA & SDIO_FLAG_TXACT)) && (timeout > 0))
  {
    timeout--;  
  }

  if (StopCondition == 1)
  {
    errorstatus = SD_StopTransfer();
    StopCondition = 0;
  }
  
  if ((timeout == 0) && (errorstatus == SD_OK))
  {
    errorstatus = SD_DATA_TIMEOUT;
  }
  
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  
  if (TransferError != SD_OK)
  {
    return(TransferError);
  }
  else
  {
    return(errorstatus);
  }
}

/**
  * @brief  Gets the cuurent data transfer state.
  * @param  None
  * @retval SDTransferState: Data Transfer state.
  *   This value can be: 
  *        - SD_TRANSFER_OK: No data transfer is acting
  *        - SD_TRANSFER_BUSY: Data transfer is acting
  */
SDTransferState SD_GetTransferState(void)
{
  if (SDIO->STA & (SDIO_FLAG_TXACT | SDIO_FLAG_RXACT))
  {
    return(SD_TRANSFER_BUSY);
  }
  else
  {
    return(SD_TRANSFER_OK);
  }
}

/**
  * @brief  Aborts an ongoing data transfer.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_StopTransfer(void)
{
  SD_Error errorstatus = SD_OK;

  /*!< Send CMD12 STOP_TRANSMISSION  */
//  SDIO_CmdInitStructure.SDIO_Argument = 0x0;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_STOP_TRANSMISSION, (u32)0, SDIO_RESPONSE_SHORT);    
	
	
  errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);

  return(errorstatus);
}

/**
  * @brief  Allows to erase memory area specified for the given card.
  * @param  startaddr: the start address.
  * @param  endaddr: the end address.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_Erase(uint64_t startaddr, uint64_t endaddr)
{
  SD_Error errorstatus = SD_OK;
  u32 delay = 0;
  volatile u32 maxdelay = 0;
  u8 cardstate = 0;

  /*!< Check if the card coomnd class supports erase command */
  if (((CSD_Tab[1] >> 20) & SD_CCCC_ERASE) == 0)
  {
    errorstatus = SD_REQUEST_NOT_APPLICABLE;
    return(errorstatus);
  }

  maxdelay = 120000 / ((SDIO->CLKCR & 0xFF) + 2);

  if (SDIO_GetResponse(SDIO, SDIO_RESP1) & SD_CARD_LOCKED)
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }

  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    startaddr /= 512;
    endaddr /= 512;
  }
  
  /*!< According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
  if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
    /*!< Send CMD32 SD_ERASE_GRP_START with argument as addr  */
//    SDIO_CmdInitStructure.SDIO_Argument =(u32)startaddr;
//    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_ERASE_GRP_START;
//    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//    SDIO_SendCommand(&SDIO_CmdInitStructure);

	sdio_send_cmd(SD_CMD_SD_ERASE_GRP_START, (u32)startaddr, SDIO_RESPONSE_SHORT);  
	  
    errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_START);
    if (errorstatus != SD_OK)
    {
      return(errorstatus);
    }

    /*!< Send CMD33 SD_ERASE_GRP_END with argument as addr  */
//    SDIO_CmdInitStructure.SDIO_Argument = (u32)endaddr;
//    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_ERASE_GRP_END;
//    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//    SDIO_SendCommand(&SDIO_CmdInitStructure);

	sdio_send_cmd(SD_CMD_SD_ERASE_GRP_END, (u32)endaddr, SDIO_RESPONSE_SHORT);  	
	
    errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_END);
    if (errorstatus != SD_OK)
    {
      return(errorstatus);
    }
  }

  /*!< Send CMD38 ERASE */
//  SDIO_CmdInitStructure.SDIO_Argument = 0;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ERASE;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_ERASE, (u32)0, SDIO_RESPONSE_SHORT);  	  
  
  errorstatus = CmdResp1Error(SD_CMD_ERASE);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  for (delay = 0; delay < maxdelay; delay++)
  {}

  /*!< Wait till the card is in programming state */
  errorstatus = IsCardProgramming(&cardstate);
  delay = SD_DATATIMEOUT;
  while ((delay > 0) && (errorstatus == SD_OK) && ((SD_CARD_PROGRAMMING == cardstate) || (SD_CARD_RECEIVING == cardstate)))
  {
    errorstatus = IsCardProgramming(&cardstate);
    delay--;
  }

  return(errorstatus);
}

/**
  * @brief  Returns the current card's status.
  * @param  pcardstatus: pointer to the buffer that will contain the SD card 
  *         status (Card Status register).
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_SendStatus(u32 *pcardstatus)
{
  SD_Error errorstatus = SD_OK;

  if (pcardstatus == NULL)
  {
    errorstatus = SD_INVALID_PARAMETER;
    return(errorstatus);
  }

//  SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SEND_STATUS, (u32)RCA << 16, SDIO_RESPONSE_SHORT);    
  
  errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  *pcardstatus = SDIO_GetResponse(SDIO, SDIO_RESP1);

  return(errorstatus);
}

/**
  * @brief  Returns the current SD card's status.
  * @param  psdstatus: pointer to the buffer that will contain the SD card status 
  *         (SD Status register).
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_SendSDStatus(u32 *psdstatus)
{
  SD_Error errorstatus = SD_OK;
  u32 count = 0;

  if (SDIO_GetResponse(SDIO, SDIO_RESP1) & SD_CARD_LOCKED)
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }

  /*!< Set block size for card if it is not equal to current block size for card. */
//  SDIO_CmdInitStructure.SDIO_Argument = 64;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)64, SDIO_RESPONSE_SHORT);    
  
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  /*!< CMD55 */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);
  
  sdio_send_cmd(SD_CMD_APP_CMD, (u32)RCA << 16, SDIO_RESPONSE_SHORT); 
  
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

//  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
//  SDIO_DataInitStructure.SDIO_DataLength = 64;
//  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_64b;
//  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
//  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
//  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
//  SDIO_DataConfig(&SDIO_DataInitStructure);

  sdio_data_cfg(64, SDIO_TRANSFER_DIR_TO_SDIO);
  
  /*!< Send ACMD13 SD_APP_STAUS  with argument as card's RCA.*/
//  SDIO_CmdInitStructure.SDIO_Argument = 0;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_STAUS;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);
  
  sdio_send_cmd(SD_CMD_SD_APP_STAUSE, (u32)0, SDIO_RESPONSE_SHORT); 
  errorstatus = CmdResp1Error(SD_CMD_SD_APP_STAUSE);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (sdio_get_state(SDIO_FLAG_RXFIFOHF) != RESET)
    {
      for (count = 0; count < 8; count++)
      {
        *(psdstatus + count) = sdio_read_data();
      }
      psdstatus += 8;
    }
  }

  if (sdio_get_state(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_DCRCFAIL) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_RXOVERR) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_RXOVERR);
    errorstatus = SD_RX_OVERRUN;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_STBITERR) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }

  count = SD_DATATIMEOUT;
  while ((sdio_get_state(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
  {
    *psdstatus = sdio_read_data();
    psdstatus++;
    count--;
  }
  /*!< Clear all the static status flags*/
  sdio_clear_flag(SDIO_STATIC_FLAGS);

  return(errorstatus);
}

/**
  * @brief  Allows to process all the interrupts that are high.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_ProcessIRQSrc(void)
{ 
  if (sdio_get_IT(SDIO_IT_DATAEND) != RESET)
  {
    TransferError = SD_OK;
    sdio_clearIT_bit(SDIO_IT_DATAEND);
    TransferEnd = 1;
  }  
  else if (sdio_get_IT(SDIO_IT_DCRCFAIL) != RESET)
  {
    sdio_clearIT_bit(SDIO_IT_DCRCFAIL);
    TransferError = SD_DATA_CRC_FAIL;
  }
  else if (sdio_get_IT(SDIO_IT_DTIMEOUT) != RESET)
  {
    sdio_clearIT_bit(SDIO_IT_DTIMEOUT);
    TransferError = SD_DATA_TIMEOUT;
  }
  else if (sdio_get_IT(SDIO_IT_RXOVERR) != RESET)
  {
    sdio_clearIT_bit(SDIO_IT_RXOVERR);
    TransferError = SD_RX_OVERRUN;
  }
  else if (sdio_get_IT(SDIO_IT_TXUNDERR) != RESET)
  {
    sdio_clearIT_bit(SDIO_IT_TXUNDERR);
    TransferError = SD_TX_UNDERRUN;
  }
  else if (sdio_get_IT(SDIO_IT_STBITERR) != RESET)
  {
    sdio_clearIT_bit(SDIO_IT_STBITERR);
    TransferError = SD_START_BIT_ERR;
  }

  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
                SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
                SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
  return(TransferError);
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished. 
  * @param  None.
  * @retval None.
  */
void SD_ProcessDMAIRQ(void)
{
//  if(DMA2->LISR & SD_SDIO_DMA_FLAG_TCIF)
//  {
//    DMAEndOfTransfer = 0x01;
//    DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF|SD_SDIO_DMA_FLAG_FEIF);
//  }
}

/**
  * @brief  Checks for error conditions for CMD0.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdError(void)
{
  SD_Error errorstatus = SD_OK;
  u32 timeout;

  timeout = SDIO_CMD0TIMEOUT; /*!< 10000 */

  while ((timeout > 0) && (sdio_get_state(SDIO_FLAG_CMDSENT) == RESET))
  {
    timeout--;
  }

  if (timeout == 0)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R7 response.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp7Error(void)
{
  SD_Error errorstatus = SD_OK;
  u32 status;
  u32 timeout = SDIO_CMD0TIMEOUT;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) && (timeout > 0))
  {
    timeout--;
    status = SDIO->STA;
  }

  if ((timeout == 0) || (status & SDIO_FLAG_CTIMEOUT))
  {
    /*!< Card is not V2.0 complient or card does not support the set voltage range */
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }

  if (status & SDIO_FLAG_CMDREND)
  {
    /*!< Card is SD V2.0 compliant */
    errorstatus = SD_OK;
    sdio_clear_flag(SDIO_FLAG_CMDREND);
    return(errorstatus);
  }
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R1 response.
  * @param  cmd: The sent command index.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp1Error(u8 cmd)
{
  SD_Error errorstatus = SD_OK;
  u32 status;
  u32 response_r1;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    sdio_clear_flag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  /*!< Check response received is of desired command */
  if (SDIO_GetCommandResponse(SDIO) != cmd)
  {
    errorstatus = SD_ILLEGAL_CMD;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

  /*!< We have received response, retrieve it for analysis  */
  response_r1 = SDIO_GetResponse(SDIO, SDIO_RESP1);

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

  if (response_r1 & SD_OCR_CID_CSD_OVERWRIETE)
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
  u32 status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
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
  u32 status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    sdio_clear_flag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R6 (RCA) response.
  * @param  cmd: The sent command index.
  * @param  prca: pointer to the variable that will contain the SD card relative 
  *         address RCA. 
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp6Error(u8 cmd, u16 *prca)
{
  SD_Error errorstatus = SD_OK;
  u32 status;
  u32 response_r1;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    sdio_clear_flag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  /*!< Check response received is of desired command */
  if (SDIO_GetCommandResponse(SDIO) != cmd)
  {
    errorstatus = SD_ILLEGAL_CMD;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

  /*!< We have received response, retrieve it.  */
  response_r1 = SDIO_GetResponse(SDIO, SDIO_RESP1);

  if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
  {
    *prca = (u16) (response_r1 >> 16);
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

/**
  * @brief  Enables or disables the SDIO wide bus mode.
  * @param  NewState: new state of the SDIO wide bus mode.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error SDEnWideBus(FunctionalState NewState)
{
  SD_Error errorstatus = SD_OK;

  u32 scr[2] = {0, 0};

  if (SDIO_GetResponse(SDIO, SDIO_RESP1) & SD_CARD_LOCKED)
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }

  /*!< Get SCR Register */
  errorstatus = FindSCR(RCA, scr);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  /*!< If wide bus operation to be enabled */
  if (NewState == ENABLE)
  {
    /*!< If requested card supports wide bus operation */
    if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)
    {
      /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
//      SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
//      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
//      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//      SDIO_SendCommand(&SDIO_CmdInitStructure);

	  sdio_send_cmd(SD_CMD_APP_CMD, (u32) RCA << 16, SDIO_RESPONSE_SHORT);
		
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }

      /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
//      SDIO_CmdInitStructure.SDIO_Argument = 0x2;
//      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
//      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//      SDIO_SendCommand(&SDIO_CmdInitStructure);

	  sdio_send_cmd(SD_CMD_APP_SD_SET_BUSWIDTH, (u32) 0x2, SDIO_RESPONSE_SHORT);
	  
	  
      errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      return(errorstatus);
    }
    else
    {
      errorstatus = SD_REQUEST_NOT_APPLICABLE;
      return(errorstatus);
    }
  }   /*!< If wide bus operation to be disabled */
  else
  {
    /*!< If requested card supports 1 bit mode operation */
    if ((scr[1] & SD_SINGLE_BUS_SUPPORT) != SD_ALLZERO)
    {
      /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
//      SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
//      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
//      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//      SDIO_SendCommand(&SDIO_CmdInitStructure);

	  sdio_send_cmd(SD_CMD_APP_CMD, (u32)RCA << 16, SDIO_RESPONSE_SHORT);

		
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }

      /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
//      SDIO_CmdInitStructure.SDIO_Argument = 0x00;
//      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
//      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//      SDIO_SendCommand(&SDIO_CmdInitStructure);

	  sdio_send_cmd(SD_CMD_APP_SD_SET_BUSWIDTH, (u32)0x00, SDIO_RESPONSE_SHORT);
	  
	  
      errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }

      return(errorstatus);
    }
    else
    {
      errorstatus = SD_REQUEST_NOT_APPLICABLE;
      return(errorstatus);
    }
  }
}

/**
  * @brief  Checks if the SD card is in programming state.
  * @param  pstatus: pointer to the variable that will contain the SD card state.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error IsCardProgramming(u8 *pstatus)
{
  SD_Error errorstatus = SD_OK;
  volatile u32 respR1 = 0, status = 0;

//  SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SEND_STATUS, (u32)RCA << 16, SDIO_RESPONSE_SHORT);
	
	
  status = SDIO->STA;
  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    sdio_clear_flag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  status = (u32)SDIO_GetCommandResponse(SDIO);

  /*!< Check response received is of desired command */
  if (status != SD_CMD_SEND_STATUS)
  {
    errorstatus = SD_ILLEGAL_CMD;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);


  /*!< We have received response, retrieve it for analysis  */
  respR1 = SDIO_GetResponse(SDIO, SDIO_RESP1);

  /*!< Find out card status */
  *pstatus = (u8) ((respR1 >> 9) & 0x0000000F);

  if ((respR1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
  {
    return(errorstatus);
  }

  if (respR1 & SD_OCR_ADDR_OUT_OF_RANGE)
  {
    return(SD_ADDR_OUT_OF_RANGE);
  }

  if (respR1 & SD_OCR_ADDR_MISALIGNED)
  {
    return(SD_ADDR_MISALIGNED);
  }

  if (respR1 & SD_OCR_BLOCK_LEN_ERR)
  {
    return(SD_BLOCK_LEN_ERR);
  }

  if (respR1 & SD_OCR_ERASE_SEQ_ERR)
  {
    return(SD_ERASE_SEQ_ERR);
  }

  if (respR1 & SD_OCR_BAD_ERASE_PARAM)
  {
    return(SD_BAD_ERASE_PARAM);
  }

  if (respR1 & SD_OCR_WRITE_PROT_VIOLATION)
  {
    return(SD_WRITE_PROT_VIOLATION);
  }

  if (respR1 & SD_OCR_LOCK_UNLOCK_FAILED)
  {
    return(SD_LOCK_UNLOCK_FAILED);
  }

  if (respR1 & SD_OCR_COM_CRC_FAILED)
  {
    return(SD_COM_CRC_FAILED);
  }

  if (respR1 & SD_OCR_ILLEGAL_CMD)
  {
    return(SD_ILLEGAL_CMD);
  }

  if (respR1 & SD_OCR_CARD_ECC_FAILED)
  {
    return(SD_CARD_ECC_FAILED);
  }

  if (respR1 & SD_OCR_CC_ERROR)
  {
    return(SD_CC_ERROR);
  }

  if (respR1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
  {
    return(SD_GENERAL_UNKNOWN_ERROR);
  }

  if (respR1 & SD_OCR_STREAM_READ_UNDERRUN)
  {
    return(SD_STREAM_READ_UNDERRUN);
  }

  if (respR1 & SD_OCR_STREAM_WRITE_OVERRUN)
  {
    return(SD_STREAM_WRITE_OVERRUN);
  }

  if (respR1 & SD_OCR_CID_CSD_OVERWRIETE)
  {
    return(SD_CID_CSD_OVERWRITE);
  }

  if (respR1 & SD_OCR_WP_ERASE_SKIP)
  {
    return(SD_WP_ERASE_SKIP);
  }

  if (respR1 & SD_OCR_CARD_ECC_DISABLED)
  {
    return(SD_CARD_ECC_DISABLED);
  }

  if (respR1 & SD_OCR_ERASE_RESET)
  {
    return(SD_ERASE_RESET);
  }

  if (respR1 & SD_OCR_AKE_SEQ_ERROR)
  {
    return(SD_AKE_SEQ_ERROR);
  }

  return(errorstatus);
}

/**
  * @brief  Find the SD card SCR register value.
  * @param  rca: selected card address.
  * @param  pscr: pointer to the buffer that will contain the SCR value.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error FindSCR(u16 rca, u32 *pscr)
{
  u32 index = 0;
  SD_Error errorstatus = SD_OK;
  u32 tempscr[2] = {0, 0};

  /*!< Set Block Size To 8 Bytes */
  /*!< Send CMD55 APP_CMD with argument as card's RCA */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32)8;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)8, SDIO_RESPONSE_SHORT);
 
  
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  /*!< Send CMD55 APP_CMD with argument as card's RCA */
//  SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_APP_CMD, (u32)RCA << 16, SDIO_RESPONSE_SHORT);
  
  
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
//  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
//  SDIO_DataInitStructure.SDIO_DataLength = 8;
//  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b;
//  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
//  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
//  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
//  SDIO_DataConfig(&SDIO_DataInitStructure);

    sdio_data_cfg(8, SDIO_TRANSFER_DIR_TO_SDIO);
  /*!< Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
//  SDIO_CmdInitStructure.SDIO_Argument = 0x0;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

  sdio_send_cmd(SD_CMD_SD_APP_SEND_SCR, (u32)0x0, SDIO_RESPONSE_SHORT);
  
  errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  while (!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (sdio_get_state(SDIO_FLAG_RXDAVL) != RESET)
    {
      *(tempscr + index) = sdio_read_data();
      index++;
    }
  }

  if (sdio_get_state(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_DCRCFAIL) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_RXOVERR) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_RXOVERR);
    errorstatus = SD_RX_OVERRUN;
    return(errorstatus);
  }
  else if (sdio_get_state(SDIO_FLAG_STBITERR) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

  *(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);

  *(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);

  return(errorstatus);
}

/**
  * @brief  Converts the number of bytes in power of two and returns the power.
  * @param  NumberOfBytes: number of bytes.
  * @retval None
  */
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes)
{
  u8 count = 0;

  while (NumberOfBytes != 1)
  {
    NumberOfBytes >>= 1;
    count++;
  }
  return(count);
}

/**
* @brief  Switch mode High-Speed 
* @note   This function must be used after "Transfer State"
* @note   This operation should be followed by the configuration 
*         of PLL to have SDIOCK clock between 67 and 75 MHz
* @param  None
* @retval SD_Error: SD Card Error code.
*/
SD_Error SD_HighSpeed (void)
{
  SD_Error errorstatus = SD_OK;
  u32 scr[2] = {0, 0};
  u32 SD_SPEC = 0 ;
  u8 hs[64] = {0} ;
  u32  count = 0, *tempbuff = (u32 *)hs;
  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 0;

  SDIO->DCTRL = 0x0;

  /*!< Get SCR Register */
  errorstatus = FindSCR(RCA, scr);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  /* Test the Version supported by the card*/ 
  SD_SPEC = (scr[1]  & 0x01000000)||(scr[1]  & 0x02000000);

  if (SD_SPEC != SD_ALLZERO)
  {
    /* Set Block Size for Card */
//    SDIO_CmdInitStructure.SDIO_Argument = (u32)64;
//    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
//    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//    SDIO_SendCommand(&SDIO_CmdInitStructure);
	  
    sdio_send_cmd(SD_CMD_SET_BLOCKLEN, (u32)64, SDIO_RESPONSE_SHORT);
  
	  
    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
    if (errorstatus != SD_OK)
    {
      return(errorstatus);
    }
//    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
//    SDIO_DataInitStructure.SDIO_DataLength = 64;
//    SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_64b ;
//    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
//    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
//    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
//    SDIO_DataConfig(&SDIO_DataInitStructure);

	sdio_data_cfg(64, SDIO_TRANSFER_DIR_TO_SDIO);
	
    /*!< Send CMD6 switch mode */
//    SDIO_CmdInitStructure.SDIO_Argument = 0x80FFFF01;
//    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SWITCH;
//    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//    SDIO_SendCommand(&SDIO_CmdInitStructure); 

    sdio_send_cmd(SD_CMD_HS_SWITCH, 0x80FFFF01, SDIO_RESPONSE_SHORT);		
	
    errorstatus = CmdResp1Error(SD_CMD_HS_SWITCH);

    if (errorstatus != SD_OK)
    {
      return(errorstatus);
    }
    while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
    {
      if (sdio_get_state(SDIO_FLAG_RXFIFOHF) != RESET)
      {
        for (count = 0; count < 8; count++)
        {
          *(tempbuff + count) = sdio_read_data();
        }
        tempbuff += 8;
      }
    }
    
    if (sdio_get_state(SDIO_FLAG_DTIMEOUT) != RESET)
    {
      sdio_clear_flag(SDIO_FLAG_DTIMEOUT);
      errorstatus = SD_DATA_TIMEOUT;
      return(errorstatus);
    }
    else if (sdio_get_state(SDIO_FLAG_DCRCFAIL) != RESET)
    {
      sdio_clear_flag(SDIO_FLAG_DCRCFAIL);
      errorstatus = SD_DATA_CRC_FAIL;
      return(errorstatus);
    }
    else if (sdio_get_state(SDIO_FLAG_RXOVERR) != RESET)
    {
      sdio_clear_flag(SDIO_FLAG_RXOVERR);
      errorstatus = SD_RX_OVERRUN;
      return(errorstatus);
    }
    else if (sdio_get_state(SDIO_FLAG_STBITERR) != RESET)
    {
      sdio_clear_flag(SDIO_FLAG_STBITERR);
      errorstatus = SD_START_BIT_ERR;
      return(errorstatus);
    }
    count = SD_DATATIMEOUT;
    while ((sdio_get_state(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
    {
      *tempbuff = sdio_read_data();
      tempbuff++;
      count--;
    }

    /*!< Clear all the static flags */
    sdio_clear_flag(SDIO_STATIC_FLAGS);

    /* Test if the switch mode HS is ok */
    if ((hs[13]& 0x2)==0x2)
    {
      errorstatus=SD_OK;
    }
    else
    {
      errorstatus=SD_UNSUPPORTED_FEATURE ;
    }  
  }
  return(errorstatus);
}


#endif


#endif
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */  

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
