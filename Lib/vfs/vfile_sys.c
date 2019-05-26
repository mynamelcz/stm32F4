#include "vfile_sys.h"
#include "diskio.h"
#include "ff.h"
#include "typedf.h"



u32 vfs_get_dev_inf(const TCHAR* path);
u32 vfs_get_file_inf(const TCHAR* path);
u32 scan_files (char* path);

typedef FATFS   fs_t;
typedef FIL     fil_t;

//u8 work[FF_MAX_SS]; 
fs_t  flash_fs;
fil_t new_fil;

void fs_test(void)
{
	
    u32 res = 0;
	u32 w_len = 0;
	u32 r_len = 0;
	u8 r_buf[32];
    res = f_mount(&flash_fs, "0:", 1);
	if(res != FR_OK){
		vfs_printf("VFS ERR	| f_mount err: %d\n",res);
//		if(res == FR_NO_FILESYSTEM){
//			res = f_mkfs("0:", FM_ANY, 0, work, FF_MAX_SS);
//			if(res != FR_OK){
//				vfs_printf("VFS ERR	| f_mkfs err: %d\n",res);
//				while(1);
//			}
//		}
	}

	res = scan_files ("/");
	if(res != FR_OK){
		vfs_printf("VFS ERR	| scan_files err: %d\n",res);
	}
	vfs_get_dev_inf("0:");
	
	
//	res = f_open(&new_fil,"0:test1.txt",FA_OPEN_EXISTING | FA_WRITE);
//	if(res != FR_OK){
//		vfs_printf("VFS ERR	| f_open err: %d\n",res);
//	}
//	res = f_write (&new_fil,"ÄãºÃ", 4, &w_len);
//	if(res != FR_OK){
//		vfs_printf("VFS ERR	| f_write err: %d\n",res);
//	}
//	
//	res = f_close (&new_fil);
//	if(res != FR_OK){
//		vfs_printf("VFS ERR	| f_close err: %d\n",res);
//	}		

    vfs_get_file_inf("0:test1.txt");
//	res = f_open(&new_fil,"0:testlongfilename.txt",FA_OPEN_EXISTING | FA_READ);
//	if(res != FR_OK){
//		vfs_printf("VFS ERR	| f_open err: %d\n",res);
//	}	
	

//	vfs_printf("w_len: %d\n",w_len);	

//	res = f_read (&new_fil, r_buf, 6, &r_len);
//	if(res != FR_OK){
//		vfs_printf("VFS ERR	| f_write err: %d\n",res);
//	}
//	vfs_puthex((const char *)r_buf, r_len);	
//	r_buf[r_len] = 0;
//	vfs_printf("f_read: %s\n",r_buf);

//	

	
	
}
u32 scan_files (char* path)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;
    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen((const char *)path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }
    return res;
}

/*******  get dev inf*******/
u32 vfs_get_dev_inf(const TCHAR* path)
{
	u32 res = FR_OK;
	fs_t  *fs;
	u32 free_clst = 0;
	u32 total_clst = 0;
	u32 tatal_sz_kb = 0;
	u32 free_sz_kb = 0;
	res =  f_getfree (path, &free_clst, &fs);
	if(res != FR_OK){
		vfs_printf("VFS ERR	| f_getfree err: %d\n",res);	
		return res;
	}
	total_clst = (fs->n_fatent-2) * fs->csize;
	
	free_sz_kb  = (free_clst  * fs->csize)<<2;
	tatal_sz_kb = (total_clst * fs->csize)<<2;
	

	
	vfs_printf("dev fs type: %d\n", fs->fs_type);
	vfs_printf("dev tatal cluster num: %d\n", total_clst);
	vfs_printf("dev free  cluster num: %d\n", free_clst);
	vfs_printf("dev tatal size kb: %d\n", tatal_sz_kb);
	vfs_printf("dev free  size kb: %d\n", free_sz_kb);
	
	return res;
}

u32 vfs_get_file_inf(const TCHAR* path)
{
	u32 res = FR_OK;
    FILINFO f_inf;
	time_t *t_tmp;
	date_t *d_tmp;
	res = f_stat(path, &f_inf);
	if(res != FR_OK){
		vfs_printf("VFS ERR	| f_stat err: %d\n",res);	
		return res;
	}
	

    vfs_printf("Attributes: %c%c%c%c%c\n",
              (f_inf.fattrib & AM_DIR) ? 'D' : '-',
              (f_inf.fattrib & AM_RDO) ? 'R' : '-',
              (f_inf.fattrib & AM_HID) ? 'H' : '-',
              (f_inf.fattrib & AM_SYS) ? 'S' : '-',
              (f_inf.fattrib & AM_ARC) ? 'A' : '-');
	
	
	t_tmp  = (time_t *)(&f_inf.ftime);
	d_tmp  = (date_t *)(&f_inf.fdate);
	vfs_printf("%d-%02d-%02d, %02d:%02d:%02d\n",
	            d_tmp->year+1980,d_tmp->mount,d_tmp->day,
	            t_tmp->hour,t_tmp->min,t_tmp->sec);
	
	vfs_printf("f_inf.fdate: 0x%x\n", f_inf.fdate);
	vfs_printf("f_inf.ftime: 0x%x\n", f_inf.ftime);
	vfs_printf("f_inf.altname: %s\n", f_inf.altname);
	vfs_printf("f_inf.fname:   %s\n", f_inf.fname);
	vfs_printf("f_inf.fattrib: %d\n", f_inf.fattrib);
	vfs_printf("f_inf.fsize:   %d\n", f_inf.fsize);	
    return res;

}










