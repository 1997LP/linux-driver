/*************************************************************************
    > File Name: hardware.c
    > Author: yangl
    > Mail: yangl@innosilicon.com.cn 
    > Created Time: 2022-02-16-09:13:30
    > Func: 
 ************************************************************************/
#ifndef __UART_INIT__
#define __UART_INIT__

#ifndef __ASSEMBLY__ 

#include <stdarg.h>
#include "types.h"
#include "printf.h"
#include "board.h"
#include "common.h"
#include "sys.h"

/* 以下为用户需要修改的配置 */
#define  STD_DEV_BASE_ADDR	UART0_BASE	//Puts默认定位到标准设备(也要先初始化然后才能调用puts函数)
#define  UART_CLK			24000000 	//dw_apb_uart模块使用Clock_Mode=2(双时钟,pclk+sclk),以sclk为时钟基准
#define  BAUD_RATE    		115200

/*
 * dw_apb_uart模块具有IP层面可裁剪性，因此将本文档对应的具体IP配置列出如下：
 * 具体参数含义请阅读 SYNOPSYS <DesignWare DW_apb_uart Databook 4.02a>,Chapter3 Parameter Descriptions.
 * Parameters:
 *		ADDITIONAL_FEATURES = false(0x0)
 *		UART_16550_COMPATIBLE = false(0)
 *		THRE_MODE_USER = Disabled(0x0)
 *		FIFO_MODE = 64
 *		MEM_SELECT_USER = Internal(1)
 *		CLOCK_MODE = Enabled(2)
 *		FRACTIONAL_BAUD_DIVISOR_EN = Disabled(0)
 */
#define  MAXDIVISOR 		0xFFFF
/* UART Basic Feature Related Registers */
#define UARTRBR			(0x00)		//Receive Buffer Register
#define UARTDLL			(0x00)		//Divisor Latch(Low)
#define UARTTHR			(0x00)		//Transmit Holding Register
#define UARTDLH			(0x04)		//Divisor Latch(High)
#define UARTLCR			(0x0C)		//Line Control Register
#define UARTLSR			(0x14)		//line Status Register
#define UARTSCR			(0x1C)		//Scratchpad Register
#define UARTUSR			(0x7C)		//UART Status Register
#define UARTDLF			(0xC0)		//Divisor Latch Fraction Register
#define UARTUCV			(0xF8)		//Uart Component Version
#define UARTCTR			(0xFC)		//Component Type Register

char UartRegTest(unsigned long BaseAddr);
void InitUart(unsigned long BaseAddr);
void sendchar(unsigned long BaseAddr, char ch);
void SendUartString(unsigned long BaseAddr, const char * buf);
int getUartReceiveBuf(unsigned long BaseAddr, unsigned char *ch);
int getUartReceiveBufLenTimeout(unsigned long BaseAddr, unsigned char *buf, int len);
void putc(char c);
void puts(const char *str);
void print_hex_64(u64 val);


#else
.macro	uart0_init
	
.endm	

.macro	uart0_sendchar value
	
.endm	

#endif


#endif
