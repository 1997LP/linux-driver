#include "config.h"
#include "hardware_test_dw_apb_uart.h"
#include "xload.h"
#include "types.h"
#include "common.h"



void board_init_f(unsigned long bootflag)
{
    int i = 0;

	word32(0x11240000) &= ~(1 << 0); /*Disable watch dog*/
	InitUart(UART0_BASE);
	putc('\n');
	puts("Bootrom Init...\n");

	while (i++ < 10) {
		putc('b');
	}

	cmdLoop();
	while(1);
}


