#ifndef __SOFT_SPI_H
#define __SOFT_SPI_H
#include "includes.h"

//typedef struct _ARM_DRIVER_SPI {
//  u32   (*GetVersion)      (void);                             
//  u32   (*Initialize)      (void(*Tx_CB)(void),void(*Rx_CB)(void),void(*ERR_CB)(void)); 
//  u32   (*Uninitialize)    (void);  
//  u32   (*Send)            (const void *data, u32 num);  
//  u32   (*Receive)         (      void *data, u32 num);   
//  u32   (*Transfer)        (const void *data_out,void *data_in, u32 num);                  
//  u32   (*GetDataCount)    (void);                            
//  u32   (*Control)         (u32 control, void* arg);  
//  u32   (*GetStatus)       (void);  

//  void  (*Tx_CB)  (struct _ARM_DRIVER_SPI *hsd);
//  void  (*Rx_CB)  (struct _ARM_DRIVER_SPI *hsd);
//  void  (*ERR_CB) (struct _ARM_DRIVER_SPI *hsd);
//} const SPI_DrvTypDef;


typedef enum __SPI_TYPE{
	CPOL0_CPHA0,		// CLK  LOW,  1 age
	CPOL0_CPHA1,		// CLK  LOW,  2 age
	CPOL1_CPHA0,
	CPOL1_CPHA1,
}SPI_TYPE_E;

typedef struct _soft_spi_io {
	SPI_TYPE_E type;
	u8  (*MISO)(void);
	void (*MOSI)(u8 en);
	void (*CLK)(u8 en);
}Soft_SPI_hd;

u8 spi_tx_rx_byte(Soft_SPI_hd *hd, u8 s_dat);
void spi_send_buf(Soft_SPI_hd *hd, const u8 *buf, u32 len);
void spi_read_buf(Soft_SPI_hd *hd, u8 *buf, u32 len);


#endif


