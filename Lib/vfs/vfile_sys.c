#include "vfile_sys.h"
#include "diskio.h"
#include "ff.h"
#include "app_cfg.h"


typedef FATFS   fs_t;
typedef FIL     fil_t;

fs_t  flash_fs;
fil_t new_fil;

void fs_test(void)
{
    u32 res = 0;
    res = f_mount(&flash_fs, "0:", 1);
	
}