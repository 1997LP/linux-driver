#ifndef __CONFIG__
#define __CONFIG__

#define CONFIG_ARM 1
#define CONFIG_SYS_CPU "armv8"
#define CONFIG_SYS_ARCH "arm"
#define CONFIG_ARM64 1


#define CONFIG_GICV3 1
//#define COUNTER_FREQUENCY 25000000

#define CPU_RELEASE_ADDR        secondary_boot_func
//#define GICD_BASE               0x21110000
//#define GICR_BASE               0x06100000
#define CONFIG_SYS_INIT_SP_ADDR 0x30042fff
#define CONFIG_SYS_TEXT_BASE	0x00000000
#define CONFIG_SYS_RAM_BASE	    0x30040000

#endif
