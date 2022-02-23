#include "config.h"
#include "hardware_test_dw_apb_uart.h"
#include "xload.h"
#include "types.h"
#include "common.h"

static void HardwareInit(void)
{
	
	int i = 0;
	
	InitUart(UART0_BASE);

	for(i=0;i<10;++i)
		 putc(0xA5);
	putc('\n');
	
	puts("Bootrom Init...\n");
	printf("Bootrom Init...\n");
}

void board_init_f(unsigned long bootflag)
{
    HardwareInit();
    cmdLoop();
}
