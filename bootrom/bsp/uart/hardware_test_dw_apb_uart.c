/*************************************************************************
    > File Name: hardware_test_dw_apb_uart.c
    > Author: yangl
    > Mail: yangl@innosilicon.com.cn 
    > Created Time: 2022-02-16-09:13:30
    > Func: 
 ************************************************************************/
#include "hardware_test_dw_apb_uart.h"

void delay(int time)
{
	while(time--);
}

char UartRegTest(unsigned long BaseAddr)
{
	unsigned int i = 0;
	
	for(i=0; i<255; i++)
	{
		word32(BaseAddr + UARTSCR) = i;
		if(word32(BaseAddr + UARTSCR) != i)
		{
			return (-1);
		}
	}
	
	return 0;
}

void InitUart(unsigned long BaseAddr)
{
	word32(0x060a0100) = 0;
	word32(0x060a0104) = 0;
	word32(0x060a0108) = 0;
	word32(0x060a010c) = 0;
	word32(0x060a0110) = 0;
	word32(0x060a0114) = 0;
	word32(0x060a0118) = 0;

	//需要将dw_apb_uart模块配置到可用状态
	//波特率：115200
	//数据位：8
	//停止位：1
	//无奇偶校验，无中断，无DMA，无FIFO，无流控
	
	unsigned int Baud_Rate_Divisor=0;
	unsigned int TempUse = 0;

	Baud_Rate_Divisor = UART_CLK/BAUD_RATE/16; 
	if(Baud_Rate_Divisor > MAXDIVISOR)
	{
		//Err
	}

	//1.Write "1" to LCR[7](DLAB) bit to enable reading and writing of the Dibisior Latch register.
	TempUse = word32(BaseAddr + UARTLCR);	//Read
	TempUse = TempUse | (1<<7);					//Modify
	word32(BaseAddr + UARTLCR) = TempUse;		//Write back
	
	//2.Write tp DLL,DLH to set up divisor for required baud rate.
	word32(BaseAddr + UARTDLL) = (Baud_Rate_Divisor & 0xff);
	word32(BaseAddr + UARTDLH) = ((Baud_Rate_Divisor>>8) & 0xff);
	
	//3.Write "0" to LCR[7](DLAB) bit.
	TempUse = word32(BaseAddr + UARTLCR);
	TempUse = TempUse & (~(0x1<<7));
	word32(BaseAddr + UARTLCR) = TempUse;

	/*	once the DLL is set, at least 
		8 clock cycles of the slowest DW_apb_uart clock should be 
		allowed to pass before transmitting or receiving data	*/
	for (TempUse = 0; TempUse < 500; TempUse++);
	
	//4.Write to LCR to setup transfer characteristics such as:
	//data length/number of stop bits/parity bits/and so on
	TempUse = word32(BaseAddr + UARTLCR);
	
	TempUse = TempUse & (~(0x3<<0));	//Clear bit[1:0]
	TempUse = TempUse | (0x3<<0); 		//Data Length Select:8 data

	TempUse = TempUse & (~(0x1<<2));	//Clear bit[2]
	TempUse = TempUse | (0x0<<2);		//Number of stop bits:1 stop bit

	TempUse = TempUse & (~(0x1<<3));	//Clear bit[3]
	TempUse = TempUse | (0x0<<3);		////Parity Enable:disable parity

	word32(BaseAddr + UARTLCR) = TempUse;  
	word32(BaseAddr + UARTTHR) = 'a';
}

void sendchar(unsigned long BaseAddr, char ch)
{
	//操作dw_apb_uart模块完成发送一个字符的功能
	
	unsigned int temp ;
	temp = word32(BaseAddr + UARTLSR) & (0x1<<6); //Transmitter Empty bit
	while(temp == 0)
	{
		delay(10);
		temp = word32(BaseAddr + UARTLSR) & (0x1<<6);
	}
	
	// Replace line feed with '\r'
	if (ch == '\n')
	{
		word32(BaseAddr + UARTTHR) = '\r';
		temp = word32(BaseAddr + UARTLSR) & (0x1<<6);
		while(temp == 0)
		{
			delay(10);
			temp = word32(BaseAddr + UARTLSR) & (0x1<<6);
		}
	}
	word32(BaseAddr + UARTTHR) = ch;
}

void SendUartString(unsigned long BaseAddr, const char * buf)
{
	int i = 0;

	while ( buf[i] != '\0' )
	{
		sendchar(BaseAddr, buf[i]);
		i++;
	}
}

int getUartReceiveBuf(unsigned long BaseAddr, unsigned char *ch)
{
	//操作dw_apb_uart模块接收一个字符
	unsigned int temp = 0;
	
	temp = word32(BaseAddr + UARTLSR);	//Data Ready Bit
	if (temp & 0x01) 
	{
		*ch = word32(BaseAddr + UARTRBR) & 0xFF;
		return 0;
	} else 
		return -1;
}

int getUartReceiveBufLenTimeout(unsigned long BaseAddr, unsigned char *buf, int len)
{
	int i = 0, k = 0;
	unsigned int temp = 0;
			
	while( i < len )
	{
		temp = word32(BaseAddr + UARTLSR);
		if (temp & 0x01)
		{
			buf[i++] = word32(BaseAddr + UARTRBR) & 0xFF;
			k = 0;
		}
		if ( k++ > 50000 )
			break;
	}
	return i;
}

void putc(char c)
{
	sendchar(STD_DEV_BASE_ADDR, c);
}

void puts(const char *str)
{
	SendUartString(STD_DEV_BASE_ADDR, str);
}


void print_hex_64(u64 val)
{
	char prt_buf[32];
	hex2str((unsigned char *)(&val), (unsigned char *)prt_buf, 8);
	puts("0X");
	puts(prt_buf);
}

