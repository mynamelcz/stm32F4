#ifndef __SPI_DRIVE_H
#define __SPI_DRIVE_H
#include "includes.h"

typedef struct _ARM_DRIVER_SPI {
  u32   (*GetVersion)      (void);                             
  u32   (*Initialize)      (void(*CallBack)(void  *drv_hd)); 
  u32   (*Uninitialize)    (void);  
  void  (*CS)			   (u8 en);
  u32   (*Send)            (const void *data, u32 num);  
  u32   (*Receive)         (      void *data, u32 num);   
  u32   (*Transfer)        (const void *data_out,void *data_in, u32 num);                  
  u32   (*GetDataCount)    (void);                            
  u32   (*Control)         (u32 control, void* arg);  
  u32   (*GetStatus)       (void);  
} const SPI_DrvTypDef;







#endif