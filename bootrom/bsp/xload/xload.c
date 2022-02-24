#include "config.h"
#include "macro.h"
#include "linkage.h"
#include <stdarg.h>
#include "types.h"
#include "printf.h"
#include <stddef.h>
#include "sys.h"
#include "common.h"
#include "xload.h"
#include "hardware_test_dw_apb_uart.h"
#include "board.h"

#define CMDLINE_BUF_LEN 64
#define CMD_BUF_LEN 32


void memDisp(void *memAddr, int memLen)
{	
	int i;

	printf("show mem addr: %016llx, len(in 8 bytes): %d\n", memAddr, memLen);
	printf("0x%016llx: ", (u64)(memAddr));
	for ( i = 0 ; i < memLen; i++){
		printf("0x%016llx ", *((u64 *)(memAddr + i * 8)));
	}
	putc('\n');
}

void hexdump(void *memAddr, int memLen)
{
	u32 line = memLen/16, i;

	printf("hexdump mem addr: %016llx, len: %d\n", memAddr, memLen);
	for(i = 0; i < line + 1; i++){
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				char8(memAddr + 0 + 16*i),
				char8(memAddr + 1 + 16*i), char8(memAddr + 2 + 16*i),
				char8(memAddr + 3 + 16*i), char8(memAddr + 4 + 16*i),
				char8(memAddr + 5 + 16*i), char8(memAddr + 6 + 16*i),
				char8(memAddr + 7 + 16*i), char8(memAddr + 8 + 16*i),
				char8(memAddr + 9 + 16*i), char8(memAddr + 10 + 16*i),
				char8(memAddr + 11 + 16*i), char8(memAddr + 12 + 16*i),
				char8(memAddr + 13 + 16*i), char8(memAddr + 14 + 16*i),
				char8(memAddr + 15 + 16*i));
	}
}

static void Trim(char* command)
{
	int i = 0;
	char szBuf[128];
	
	//if (!command)
		//return;
	
	i = strlen(command) - 1;
	while (i >= 0)
	{
		if (command[i] != ' ' && command[i] != '\t' && command[i] != '\n' && command[i] != '\r' && command[i] != '\0')
		{
			command[i+1] = '\0';
			break;
		}
		i--;
	}
	i = 0;
	while((command[i] == ' ') || (command[i] == '\n') || (command[i] == '\r') || (command[i] == '\t')) i++;

	strcpy(szBuf, (command + i));
	strcpy(command, szBuf);
}

static int getCommandSubStr(char * command, int num, char * subStr)
{
	
	int i, j, temp, len;

	//SendUart0String("getCommandSubStr\n");
	
	len = strlen(command);
	if ( len == 0 )
		return -1;
	
	temp = 0;
	j = 0;
	for ( i = 0; i < len; i ++)
	{
		if ( command[i] == ' ' || command[i] == '\t' )
			continue;
		if ( temp == num )
		{
			while( i < len )
			{
				subStr[j++] = command[i++];
				if ( command[i] == ' ' || command[i] == '\t' || command[i] == ',')
				{
					break;
				}
			}
			
			subStr[j] = 0;			
			return 0;
		}
		else
		{
			while( i < len )
			{
				if ( command[i] == ' ' || command[i] == '\t' || command[i] == ',' )
				{
					break;
				}
				i ++;
			}
		}
		temp ++;
	}
	
	return -1;
}


static void parsePara(char* recvCommandBuf, u64 *para1, u64 *para2)
{
	char subStr[CMD_BUF_LEN];

	if ( para1 != NULL )
	{
		if ( getCommandSubStr(recvCommandBuf, 1, subStr) == 0 )
		{
			*para1 = simple_strtoull(subStr, NULL, 16);
		}
		//printf("para1 = %016llx\t", *para1);
		puts("para1 = ");
		print_hex_64(*para1);
		puts("\t");
	}
	
	if ( para2 != NULL )
	{
		if ( getCommandSubStr(recvCommandBuf, 2, subStr) == 0 )
		{
			*para2 = simple_strtoull(subStr, NULL, 16);
		}
		//printf("para2 = %016llx\n", *para2);
		puts("para2 = ");
		print_hex_64(*para2);
		puts("\n");
	}
}

#define SOH		0x01
#define	EOT		0x04
#define	ACK		0x06
#define NAK		0x15
#define	CAN		0x18
#define	CSYM	0x43

int xmodem_download(unsigned long addr)
{
	unsigned char buf[256], temp, seq;
	unsigned int i, offset;
	int result;
	int flag = 0;

	puts("download into ");
	print_hex_64(addr);
	puts("\n");
	/* clear uart1 receive buf */
	while(getUartReceiveBuf(UART0_BASE, &temp) == 0);

	SendUartString(UART0_BASE, "Rdy\n");
	
	/* wait for download */
	offset = 0;
	seq = 0;	
	while(1)
	{
		result = getUartReceiveBufLenTimeout(UART0_BASE, buf, 132);
		if ( result == 0 )
		{
			if ( flag == 0 )
			{
				putc(NAK);
			}
			else
			{
				putc(ACK);
				putc(ACK);
			}				
			
			continue;
		}
		else if ( result < 132 )
		{
			if ( buf[0] == EOT )
			{
				putc(ACK);
				//printf("\nEND\n");
				puts("\nEND\n");
				break;		
			}
			else if ( buf[0] == CAN )
			{
				putc(ACK);
				//printf("CNL\n");
				puts("CNL\n");
				break;		
			}
			else
			{
				putc(ACK);
			}
			
		}
		else
		{
			if ( flag == 0 )
			{
				flag = 1;
				putc(ACK);
				putc(ACK);
			}
			
			putc(ACK);
			if ( buf[0] != SOH )
			{
				continue;
			}
		
			temp = 0;
			for ( i = 3; i < 131 ; i ++)
			{
				temp += buf[i];
			}
			if ( temp != buf[i] )
			{
				putc(CAN);
				//printf("err\n");
				puts("err\n");	
				break;
			}
			else
			{
				seq ++;
				if ( seq != buf[1] )
				{
					putc(CAN);
					//printf("err\n");		
					puts("err\n");
					break;
				}
	
				memcpy((char *)(addr + offset), buf + 3, 128);
				if ( memcmp((char *)(addr + offset), (char *)buf + 3, 128) != 0 )
				{
					putc(CAN);
					//printf("err\n");
					puts("err\n");	
					break;
				}
				
				offset += 128;
			}
		}
	}
	return 0;
}

void cmdLoop(void)
{
	unsigned int recvCommandBufLen = 0;
	char recvCommandBuf[CMDLINE_BUF_LEN];
	char subStr[CMD_BUF_LEN];
	u64 loadBeginAddr = 0x200000000;
	u64 runAddr = 0x200000000;
	u64 memAddr = 0;
    u64 memLen = 8;

	memset(recvCommandBuf, 0, sizeof(recvCommandBuf));
	puts("\nrun in bootrom:\n#");

	while(1)
	{
		unsigned char ch;
		if ( getUartReceiveBuf(UART0_BASE, &ch) == 0 )
		{
			if ( ch == 8 ) // backspace
			{
				if ( recvCommandBufLen > 0 )
				{
					putc(ch);
					putc(' ');
					putc(ch);
					recvCommandBufLen --;
					recvCommandBuf[recvCommandBufLen] = 0;
				}
				continue;
			}

			putc(ch);
			recvCommandBuf[recvCommandBufLen ++] = ch;
			
			if ( ch == '\r' || ch == '\n' )
			{
				putc('\n');
				Trim(recvCommandBuf);

				if ( getCommandSubStr(recvCommandBuf, 0, subStr) == 0 )
				{
					if ( strcmp(subStr, "xload") == 0 )
					{
						parsePara(recvCommandBuf, &loadBeginAddr, NULL);
						xmodem_download(loadBeginAddr);
					}
					else if ( strcmp(subStr, "run") == 0 )
					{
						u64 (*funPtr)(void);

						parsePara(recvCommandBuf, &runAddr, NULL);
						//printf("Run at %016llx\n", runAddr);
						puts("Run at ");
						print_hex_64(runAddr);
						puts("\n");
						funPtr = (u64 (*)(void))(runAddr);
						funPtr();
					}
					else if ( strcmp(subStr, "mem") == 0 )
					{
						memLen = 8;
						parsePara(recvCommandBuf, &memAddr, &memLen);
						memDisp((void *)memAddr, memLen);
						memAddr += memLen * 64;
					}
					else if ( strcmp(subStr, "memset") == 0 )
					{
						u64 value = 0;
						parsePara(recvCommandBuf, &memAddr, &value);
						*((u64 *)memAddr) = value;
					}
					#if 1
					else if ( strcmp(subStr, "hexdump") == 0 )
					{
						memLen = 8;
						parsePara(recvCommandBuf, &memAddr, &memLen);
						hexdump((void *)memAddr, memLen);
					}
					#endif
					else
					{
						puts("cmd err\n");
					}
				}
				putc('#');
				memset(recvCommandBuf, 0, sizeof(recvCommandBuf));
				recvCommandBufLen = 0;
			}
		}
	}
	return ;
}

