#ifndef __SDIO_SD_H
#define __SDIO_SD_H

#if 0

#include "includes.h"

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/



typedef enum
{
/** 
  * @brief  SDIO specific error defines  
  */   
  SD_CMD_CRC_FAIL                    = (1), /*!< Command response received (but CRC check failed) */
  SD_DATA_CRC_FAIL                   = (2), /*!< Data bock sent/received (CRC check Failed) */
  SD_CMD_RSP_TIMEOUT                 = (3), /*!< Command response timeout */
  SD_DATA_TIMEOUT                    = (4), /*!< Data time out */
  SD_TX_UNDERRUN                     = (5), /*!< Transmit FIFO under-run */
  SD_RX_OVERRUN                      = (6), /*!< Receive FIFO over-run */
  SD_START_BIT_ERR                   = (7), /*!< Start bit not detected on all data signals in widE bus mode */
  SD_CMD_OUT_OF_RANGE                = (8), /*!< CMD's argument was out of range.*/
  SD_ADDR_MISALIGNED                 = (9), /*!< Misaligned address */
  SD_BLOCK_LEN_ERR                   = (10), /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
  SD_ERASE_SEQ_ERR                   = (11), /*!< An error in the sequence of erase command occurs.*/
  SD_BAD_ERASE_PARAM                 = (12), /*!< An Invalid selection for erase groups */
  SD_WRITE_PROT_VIOLATION            = (13), /*!< Attempt to program a write protect block */
  SD_LOCK_UNLOCK_FAILED              = (14), /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
  SD_COM_CRC_FAILED                  = (15), /*!< CRC check of the previous command failed */
  SD_ILLEGAL_CMD                     = (16), /*!< Command is not legal for the card state */
  SD_CARD_ECC_FAILED                 = (17), /*!< Card internal ECC was applied but failed to correct the data */
  SD_CC_ERROR                        = (18), /*!< Internal card controller error */
  SD_GENERAL_UNKNOWN_ERROR           = (19), /*!< General or Unknown error */
  SD_STREAM_READ_UNDERRUN            = (20), /*!< The card could not sustain data transfer in stream read operation. */
  SD_STREAM_WRITE_OVERRUN            = (21), /*!< The card could not sustain data programming in stream mode */
  SD_CID_CSD_OVERWRITE               = (22), /*!< CID/CSD overwrite error */
  SD_WP_ERASE_SKIP                   = (23), /*!< only partial address space was erased */
  SD_CARD_ECC_DISABLED               = (24), /*!< Command has been executed without using internal ECC */
  SD_ERASE_RESET                     = (25), /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
  SD_AKE_SEQ_ERROR                   = (26), /*!< Error in sequence of authentication. */
  SD_INVALID_VOLTRANGE               = (27),
  SD_ADDR_OUT_OF_RANGE               = (28),
  SD_SWITCH_ERROR                    = (29),
  SD_SDIO_DISABLED                   = (30),
  SD_SDIO_FUNCTION_BUSY              = (31),
  SD_SDIO_FUNCTION_FAILED            = (32),
  SD_SDIO_UNKNOWN_FUNCTION           = (33),

/** 
  * @brief  Standard error defines   
  */ 
  SD_INTERNAL_ERROR, 
  SD_NOT_CONFIGURED,
  SD_REQUEST_PENDING, 
  SD_REQUEST_NOT_APPLICABLE, 
  SD_INVALID_PARAMETER,  
  SD_UNSUPPORTED_FEATURE,  
  SD_UNSUPPORTED_HW,  
  SD_ERROR,  
  SD_OK = 0 
} SD_Error;

/** 
  * @brief  SDIO Transfer state  
  */   
typedef enum
{
  SD_TRANSFER_OK  = 0,
  SD_TRANSFER_BUSY = 1,
  SD_TRANSFER_ERROR
} SDTransferState;

/** 
  * @brief  SD Card States 
  */   
typedef enum
{
  SD_CARD_READY                  = ((u32)0x00000001),
  SD_CARD_IDENTIFICATION         = ((u32)0x00000002),
  SD_CARD_STANDBY                = ((u32)0x00000003),
  SD_CARD_TRANSFER               = ((u32)0x00000004),
  SD_CARD_SENDING                = ((u32)0x00000005),
  SD_CARD_RECEIVING              = ((u32)0x00000006),
  SD_CARD_PROGRAMMING            = ((u32)0x00000007),
  SD_CARD_DISCONNECTED           = ((u32)0x00000008),
  SD_CARD_ERROR                  = ((u32)0x000000FF)
}SDCardState;


/** 
  * @brief  Card Specific Data: CSD Register   
  */ 
typedef struct
{
  volatile u8  CSDStruct;            /*!< CSD structure */
  volatile u8  SysSpecVersion;       /*!< System specification version */
  volatile u8  Reserved1;            /*!< Reserved */
  volatile u8  TAAC;                 /*!< Data read access-time 1 */
  volatile u8  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
  volatile u8  MaxBusClkFrec;        /*!< Max. bus clock frequency */
  volatile u16 CardComdClasses;      /*!< Card command classes */
  volatile u8  RdBlockLen;           /*!< Max. read data block length */
  volatile u8  PartBlockRead;        /*!< Partial blocks for read allowed */
  volatile u8  WrBlockMisalign;      /*!< Write block misalignment */
  volatile u8  RdBlockMisalign;      /*!< Read block misalignment */
  volatile u8  DSRImpl;              /*!< DSR implemented */
  volatile u8  Reserved2;            /*!< Reserved */
  volatile u32 DeviceSize;           /*!< Device Size */
  volatile u8  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
  volatile u8  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
  volatile u8  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
  volatile u8  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
  volatile u8  DeviceSizeMul;        /*!< Device size multiplier */
  volatile u8  EraseGrSize;          /*!< Erase group size */
  volatile u8  EraseGrMul;           /*!< Erase group size multiplier */
  volatile u8  WrProtectGrSize;      /*!< Write protect group size */
  volatile u8  WrProtectGrEnable;    /*!< Write protect group enable */
  volatile u8  ManDeflECC;           /*!< Manufacturer default ECC */
  volatile u8  WrSpeedFact;          /*!< Write speed factor */
  volatile u8  MaxWrBlockLen;        /*!< Max. write data block length */
  volatile u8  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
  volatile u8  Reserved3;            /*!< Reserded */
  volatile u8  ContentProtectAppli;  /*!< Content protection application */
  volatile u8  FileFormatGrouop;     /*!< File format group */
  volatile u8  CopyFlag;             /*!< Copy flag (OTP) */
  volatile u8  PermWrProtect;        /*!< Permanent write protection */
  volatile u8  TempWrProtect;        /*!< Temporary write protection */
  volatile u8  FileFormat;           /*!< File Format */
  volatile u8  ECC;                  /*!< ECC code */
  volatile u8  CSD_CRC;              /*!< CSD CRC */
  volatile u8  Reserved4;            /*!< always 1*/
} SD_CSD;

/** 
  * @brief  Card Identification Data: CID Register   
  */
typedef struct
{
  volatile u8  ManufacturerID;       /*!< ManufacturerID */
  volatile u16 OEM_AppliID;          /*!< OEM/Application ID */
  volatile u32 ProdName1;            /*!< Product Name part1 */
  volatile u8  ProdName2;            /*!< Product Name part2*/
  volatile u8  ProdRev;              /*!< Product Revision */
  volatile u32 ProdSN;               /*!< Product Serial Number */
  volatile u8  Reserved1;            /*!< Reserved1 */
  volatile u16 ManufactDate;         /*!< Manufacturing Date */
  volatile u8  CID_CRC;              /*!< CID CRC */
  volatile u8  Reserved2;            /*!< always 1 */
} SD_CID;

/** 
  * @brief SD Card Status 
  */
typedef struct
{
  volatile u8 DAT_BUS_WIDTH;
  volatile u8 SECURED_MODE;
  volatile u16 SD_CARD_TYPE;
  volatile u32 SIZE_OF_PROTECTED_AREA;
  volatile u8 SPEED_CLASS;
  volatile u8 PERFORMANCE_MOVE;
  volatile u8 AU_SIZE;
  volatile u16 ERASE_SIZE;
  volatile u8 ERASE_TIMEOUT;
  volatile u8 ERASE_OFFSET;
} SD_CardStatus;


/** 
  * @brief SD Card information 
  */
typedef struct
{
  SD_CSD SD_csd;
  SD_CID SD_cid;
  u64 CardCapacity;  /*!< Card Capacity */
  u32 CardBlockSize; /*!< Card Block Size */
  u16 RCA;
  u8 CardType;
} SD_CardInfo;

/**
  * @}
  */
  
/** @defgroup STM324x7I_EVAL_SDIO_SD_Exported_Constants
  * @{
  */ 

/** 
  * @brief SDIO Commands  Index 
  */
#define SD_CMD_GO_IDLE_STATE                       ((u8)0)
#define SD_CMD_SEND_OP_COND                        ((u8)1)
#define SD_CMD_ALL_SEND_CID                        ((u8)2)
#define SD_CMD_SET_REL_ADDR                        ((u8)3) /*!< SDIO_SEND_REL_ADDR for SD Card */
#define SD_CMD_SET_DSR                             ((u8)4)
#define SD_CMD_SDIO_SEN_OP_COND                    ((u8)5)
#define SD_CMD_HS_SWITCH                           ((u8)6)
#define SD_CMD_SEL_DESEL_CARD                      ((u8)7)
#define SD_CMD_HS_SEND_EXT_CSD                     ((u8)8)
#define SD_CMD_SEND_CSD                            ((u8)9)
#define SD_CMD_SEND_CID                            ((u8)10)
#define SD_CMD_READ_DAT_UNTIL_STOP                 ((u8)11) /*!< SD Card doesn't support it */
#define SD_CMD_STOP_TRANSMISSION                   ((u8)12)
#define SD_CMD_SEND_STATUS                         ((u8)13)
#define SD_CMD_HS_BUSTEST_READ                     ((u8)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((u8)15)
#define SD_CMD_SET_BLOCKLEN                        ((u8)16)
#define SD_CMD_READ_SINGLE_BLOCK                   ((u8)17)
#define SD_CMD_READ_MULT_BLOCK                     ((u8)18)
#define SD_CMD_HS_BUSTEST_WRITE                    ((u8)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP                ((u8)20) /*!< SD Card doesn't support it */
#define SD_CMD_SET_BLOCK_COUNT                     ((u8)23) /*!< SD Card doesn't support it */
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((u8)24)
#define SD_CMD_WRITE_MULT_BLOCK                    ((u8)25)
#define SD_CMD_PROG_CID                            ((u8)26) /*!< reserved for manufacturers */
#define SD_CMD_PROG_CSD                            ((u8)27)
#define SD_CMD_SET_WRITE_PROT                      ((u8)28)
#define SD_CMD_CLR_WRITE_PROT                      ((u8)29)
#define SD_CMD_SEND_WRITE_PROT                     ((u8)30)
#define SD_CMD_SD_ERASE_GRP_START                  ((u8)32) /*!< To set the address of the first write
                                                                  block to be erased. (For SD card only) */
#define SD_CMD_SD_ERASE_GRP_END                    ((u8)33) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For SD card only) */
#define SD_CMD_ERASE_GRP_START                     ((u8)35) /*!< To set the address of the first write block to be erased.
                                                                  (For MMC card only spec 3.31) */

#define SD_CMD_ERASE_GRP_END                       ((u8)36) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For MMC card only spec 3.31) */

#define SD_CMD_ERASE                               ((u8)38)
#define SD_CMD_FAST_IO                             ((u8)39) /*!< SD Card doesn't support it */
#define SD_CMD_GO_IRQ_STATE                        ((u8)40) /*!< SD Card doesn't support it */
#define SD_CMD_LOCK_UNLOCK                         ((u8)42)
#define SD_CMD_APP_CMD                             ((u8)55)
#define SD_CMD_GEN_CMD                             ((u8)56)
#define SD_CMD_NO_CMD                              ((u8)64)

/** 
  * @brief Following commands are SD Card Specific commands.
  *        SDIO_APP_CMD should be sent before sending these commands. 
  */
#define SD_CMD_APP_SD_SET_BUSWIDTH                 ((u8)6)  /*!< For SD Card only */
#define SD_CMD_SD_APP_STAUS                        ((u8)13) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS        ((u8)22) /*!< For SD Card only */
#define SD_CMD_SD_APP_OP_COND                      ((u8)41) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT          ((u8)42) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_SCR                     ((u8)51) /*!< For SD Card only */
#define SD_CMD_SDIO_RW_DIRECT                      ((u8)52) /*!< For SD I/O Card only */
#define SD_CMD_SDIO_RW_EXTENDED                    ((u8)53) /*!< For SD I/O Card only */

/** 
  * @brief Following commands are SD Card Specific security commands.
  *        SDIO_APP_CMD should be sent before sending these commands. 
  */
#define SD_CMD_SD_APP_GET_MKB                      ((u8)43) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_MID                      ((u8)44) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CER_RN1                  ((u8)45) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_CER_RN2                  ((u8)46) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CER_RES2                 ((u8)47) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_CER_RES1                 ((u8)48) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((u8)18) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((u8)25) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_ERASE                 ((u8)38) /*!< For SD Card only */
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA           ((u8)49) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MKB             ((u8)48) /*!< For SD Card only */
  
/* Uncomment the following line to select the SDIO Data transfer mode */  
#if !defined (SD_DMA_MODE) && !defined (SD_POLLING_MODE)
#define SD_DMA_MODE                                ((u32)0x00000000)
/*#define SD_POLLING_MODE                            ((u32)0x00000002)*/
#endif

/**
  * @brief  SD detection on its memory slot
  */
#define SD_PRESENT                                 ((u8)0x01)
#define SD_NOT_PRESENT                             ((u8)0x00)

/** 
  * @brief Supported SD Memory Cards 
  */
#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((u32)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((u32)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD                 ((u32)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((u32)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((u32)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((u32)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((u32)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((u32)0x00000007)

/**
  * @}
  */ 
  
/** @defgroup STM324x7I_EVAL_SDIO_SD_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM324x7I_EVAL_SDIO_SD_Exported_Functions
  * @{
  */ 
void SD_DeInit(void);
SD_Error SD_Init(void);
SDTransferState SD_GetStatus(void);
SDCardState SD_GetState(void);
u8 SD_Detect(void);
SD_Error SD_PowerON(void);
SD_Error SD_PowerOFF(void);
SD_Error SD_InitializeCards(void);
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo);
SD_Error SD_GetCardStatus(SD_CardStatus *cardstatus);
SD_Error SD_EnableWideBusOperation(u32 WideMode);
SD_Error SD_SelectDeselect(u64 addr);
SD_Error SD_ReadBlock(u8 *readbuff, u64 ReadAddr, u16 BlockSize);
SD_Error SD_ReadMultiBlocks(u8 *readbuff, u64 ReadAddr, u16 BlockSize, u32 NumberOfBlocks);
SD_Error SD_WriteBlock(u8 *writebuff, u64 WriteAddr, u16 BlockSize);
SD_Error SD_WriteMultiBlocks(u8 *writebuff, u64 WriteAddr, u16 BlockSize, u32 NumberOfBlocks);
SDTransferState SD_GetTransferState(void);
SD_Error SD_StopTransfer(void);
SD_Error SD_Erase(u64 startaddr, u64 endaddr);
SD_Error SD_SendStatus(u32 *pcardstatus);
SD_Error SD_SendSDStatus(u32 *psdstatus);
SD_Error SD_ProcessIRQSrc(void);
void SD_ProcessDMAIRQ(void);
SD_Error SD_WaitReadOperation(void);
SD_Error SD_WaitWriteOperation(void);
SD_Error SD_HighSpeed(void);
#ifdef __cplusplus
}
#endif


#endif

#endif /* __STM324x7I_EVAL_SDIO_SD_H */




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
