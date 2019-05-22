#include "my_print.h"

static int uart_putchar(char c)
{
    putchar(c);               /// 需要自己添加
    return 0;
}
int my_puts(char *str)
{
    while(*str)
        uart_putchar(*str++);
    return 0;
}
int my_puthex8(char c_arg)
{
    char *hex = "0123456789ABCDEF";
    uart_putchar(hex[(c_arg>>4)&0x0f]);
    uart_putchar(hex[(c_arg>>0)&0x0f]);
    return 0;
}
int my_put_hex(int i_arg)
{
    if(i_arg <= 0xff)
    {
        my_puthex8(i_arg);
    }
    else if(i_arg <= 0xffff)
    {
        my_puthex8((i_arg>> 8)&0xff);
        my_puthex8((i_arg>> 0)&0xff);
    }
    else
    {
        my_puthex8((i_arg>>24)&0xff);
        my_puthex8((i_arg>>16)&0xff);
        my_puthex8((i_arg>> 8)&0xff);
        my_puthex8((i_arg>> 0)&0xff);
    }
    return 0;
}
int my_printhex(const char *buf, unsigned int len)
{
    u8 i = 0;
    while(len--)
    {
       if(i++ == 16)
       {
           i = 1;
           uart_putchar('\n');
       }
       my_puthex8(*buf++);
       uart_putchar(' ');
    }
	uart_putchar('\n');
	return len;
}




//static void my_printbits8(u8 data)
//{
//    u8 i;
//    for(i = 0; i < 8; i++)
//    {
//        if(i==4)
//            uart_putchar(' ');
//        if(data&(0x80>>i))
//            uart_putchar('1');
//        else
//            uart_putchar('0');
//    }
//}
//void my_printbits(int i_arg)
//{
//    if(i_arg <= 0xff)
//    {
//        my_printbits8(i_arg&0xff);
//    }
//    else if(i_arg <= 0xffff)
//    {
//        my_printbits8((i_arg>> 8)&0xff);
//        uart_putchar(' ');
//        my_printbits8((i_arg>> 0)&0xff);
//    }
//    else
//    {
//        my_printbits8((i_arg>>24)&0xff);
//        uart_putchar(' ');
//        my_printbits8((i_arg>>16)&0xff);
//        uart_putchar(' ');
//        my_printbits8((i_arg>> 8)&0xff);
//        uart_putchar(' ');
//        my_printbits8((i_arg>> 0)&0xff);
//    }
//    return 0;
//}

//int my_printint(int i_arg)
//{
//    char buf[20];
//    char n = 0;
//    do{
//        buf[n++] = i_arg%10 + '0';
//    }while(i_arg /= 10);
//    while(n)
//    {
//        uart_putchar(buf[--n]);
//    }
//}
//typedef char *va_list;
//#define va_start(ap, p)	    (ap = (char *) (&(p)+1))
//#define va_arg(ap, type)	((type *) (ap += sizeof(type)))[-1]
//#define va_end(ap)

//int my_printf(const char *format, ...)
//{
//    char c;
//    va_list p_arg = NULL;
//    va_start(p_arg, format);

//    while((c = *format++) != '\0')
//    {
//       switch(c)
//       {
//           char  c_arg;
//           char *ptr_arg;
//           int   i_arg;
//           case '%':
//               c = *format++;
//               switch(c)
//               {
//                   case 'C':
//                   case 'c':
//                       c_arg = va_arg(p_arg, char);
//                       uart_putchar(c_arg);
//                       break;
//                   case 'S':
//                   case 's':
//                       ptr_arg = va_arg(p_arg, char *);
//                       my_puts(ptr_arg);
//                       break;
//                   case 'X':
//                   case 'x':
//                       i_arg = va_arg(p_arg, int);
//                       my_put_hex(i_arg);
//                       break;
//                   case 'D':
//                   case 'd':
//                       i_arg = va_arg(p_arg, int);
//                       my_printint(i_arg);
//                       break;
//                   case 'B':
//                   case 'b':
//                        i_arg = va_arg(p_arg, int);
//                        my_printbits(i_arg);
//                       break;
//                   default:
//                        break;
//               }
//               break;
//           default:
//               uart_putchar(c);
//               break;
//       }
//    }
//    va_end(p_arg);
//}














