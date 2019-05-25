#include "vfile_sys.h"
#include "diskio.h"
#include "ff.h"







typedef FATFS   fs_t;
typedef FIL     fil_t;

//u8 work[FF_MAX_SS]; 
fs_t  flash_fs;
fil_t new_fil;

void fs_test(void)
{
	
    u32 res = 0;
	u32 w_len = 0;
    res = f_mount(&flash_fs, "0:", 1);
	if(res != FR_OK){
		vfs_printf("VFS ERR	| f_mount err: %d\n",res);
		if(res == FR_NO_FILESYSTEM){
//			res = f_mkfs("0:", FM_ANY, 0, work, FF_MAX_SS);
//			if(res != FR_OK){
//				vfs_printf("VFS ERR	| f_mkfs err: %d\n",res);
//				while(1);
//			}
		}
	}
//	res = f_open(&new_fil,"0:test.txt",FA_CREATE_ALWAYS);
//	if(res != FR_OK){
//		vfs_printf("VFS ERR	| f_open err: %d\n",res);
//	}	
//	
//	res = f_write (&new_fil,"123456", 6, &w_len);
//	if(res != FR_OK){
//		vfs_printf("VFS ERR	| f_open err: %d\n",res);
//	}
//	vfs_printf("w_len: %d\n",w_len);	
//	
//	res = f_close (&new_fil);
//	if(res != FR_OK){
//		vfs_printf("VFS ERR	| f_close err: %d\n",res);
//	}	
	
	
}
