#ifndef _TYPEDF_H
#define _TYPEDF_H

#define true	1
#define false	0


typedef char			bool;			

/* exact-width signed integer types */
typedef signed           char s8;
typedef signed short     int s16;
typedef signed           int s32;


/* exact-width unsigned integer types */
typedef unsigned          char u8;
typedef unsigned short    int u16;
typedef unsigned          int u32;

typedef struct{
	u32 sec:5;
	u32 min:6;
	u32 hour:5;
	u32 day:5;
	u32 mount:4;
	u32	year:7;
}__time_tag;
typedef struct{
	u16 sec:5;
	u16 min:6;
	u16 hour:5;	
}time_t;
typedef struct{
	u32 day:5;
	u32 mount:4;
	u32	year:7;	
}date_t;
#endif



