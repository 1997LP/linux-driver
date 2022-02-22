#define CCM_CCGR0           0X020C4068
#define CCM_CCGR1           0X020C406C
#define CCM_CCGR2           0X020C4070
#define CCM_CCGR3           0X020C4074
#define CCM_CCGR4           0X020C4078
#define CCM_CCGR5           0X020C407C
#define CCM_CCGR6           0X020C4080

#define SW_MUX_GPIO1_IO03   0X020E0068
#define SW_PAD_GPIO1_IO03   0X020E02F4
  
#define GPIO1_DR            0X0209C000
#define GPIO1_GDIR          0X0209C004
#define GPIO1_PSR           0X0209C008
#define GPIO1_ICR1          0X0209C00C
#define GPIO1_ICR2          0X0209C010
#define GPIO1_IMR           0X0209C014
#define GPIO1_ISR           0X0209C018
#define GPIO1_EDGE_SEL      0X0209C01C


#define VA_CCM_CCGR0           (0X020C4068+0x100000)
#define VA_CCM_CCGR1           (0X020C406C+0x100000)
#define VA_CCM_CCGR2           (0X020C4070+0x100000)
#define VA_CCM_CCGR3           (0X020C4074+0x100000)
#define VA_CCM_CCGR4           (0X020C4078+0x100000)
#define VA_CCM_CCGR5           (0X020C407C+0x100000)
#define VA_CCM_CCGR6           (0X020C4080+0x100000)

#define VA_SW_MUX_GPIO1_IO03   (0X020E0068+0x100000)
#define VA_SW_PAD_GPIO1_IO03   (0X020E02F4+0x100000)
  
#define VA_GPIO1_DR            (0X0209C000+0x100000)
#define VA_GPIO1_GDIR          (0X0209C004+0x100000)
#define VA_GPIO1_PSR           (0X0209C008+0x100000)
#define VA_GPIO1_ICR1          (0X0209C00C+0x100000)
#define VA_GPIO1_ICR2          (0X0209C010+0x100000)
#define VA_GPIO1_IMR           (0X0209C014+0x100000)
#define VA_GPIO1_ISR           (0X0209C018+0x100000)
#define VA_GPIO1_EDGE_SEL      (0X0209C01C+0x100000)

#define MMU_SECTION_AP              (0x3<<10)   /*访问权限*/
#define MMU_SECTION_DOMAIN          (0<<5)      /*属于哪个域*/
#define MMU_SECTION_SPECIAL         (1<<4)      /*必须是1,决定该区域是否可以执行*/ 
#define MMU_SECTION_CACHEEABLE      (1<<3)      /*cacheable*/ 
#define MMU_SECTION_BUFFERABLE      (1<<2)      /*bufferable*/
#define MMU_SECTION_SECTION         (2)         /*段映射*/ 
#define MMU_SECDESC    (MMU_SECTION_AP|MMU_SECTION_DOMAIN|MMU_SECTION_SPECIAL|MMU_SECTION_SECTION)
#define MMU_SECDESC_WB (MMU_SECTION_AP|MMU_SECTION_DOMAIN|MMU_SECTION_SPECIAL|MMU_SECTION_SECTION| \
						MMU_SECTION_CACHEEABLE|MMU_SECTION_BUFFERABLE)


void clk_enable(void)
{
   *((volatile unsigned int *) VA_CCM_CCGR0) = 0xffffffff;
   *((volatile unsigned int *) VA_CCM_CCGR1) = 0xffffffff;
   *((volatile unsigned int *) VA_CCM_CCGR2) = 0xffffffff;
   *((volatile unsigned int *) VA_CCM_CCGR3) = 0xffffffff;
   *((volatile unsigned int *) VA_CCM_CCGR4) = 0xffffffff;
   *((volatile unsigned int *) VA_CCM_CCGR5) = 0xffffffff;
   *((volatile unsigned int *) VA_CCM_CCGR6) = 0xffffffff;
}

void led_init(void)
{
   *((volatile unsigned int *)VA_SW_MUX_GPIO1_IO03) = 0x5;
   *((volatile unsigned int *)VA_SW_PAD_GPIO1_IO03) = 0x10B0;
   *((volatile unsigned int *)VA_GPIO1_GDIR) = 0x0000008;
   *((volatile unsigned int *)VA_GPIO1_DR)  = 0x0;//打开LED
}

static void create_tlb(unsigned int *ttb,unsigned int va,unsigned pa,unsigned char WB)
{
    if(WB==1)
	{
		*(ttb+(va>>20))=(pa&0xfff00000)|MMU_SECDESC_WB;
	}
	else
	{
		*(ttb+(va>>20))=(pa&0xfff00000)|MMU_SECDESC;
	}
}

void create_page_table(void)
{
    /*512M DDR MAP*/
    unsigned int *ttb = (unsigned int *)0x80000000;
    unsigned int va = 0x80000000;
    unsigned int pa = 0x80000000;
    unsigned char loop = 0;
    for(;va<0xA0000000;va+=0x100000,pa+=0x100000)/*DDR映射的虚拟地址，物理地址不变，512MB*/
    {
        create_tlb(ttb,va,pa,1);/*Cache ，write buffer有效*/
    }
    /*register map*/
    create_tlb(ttb,0x02100000,0x02000000,0);/*只映射了1MB的物理内存，包含了时钟使能，GPIO控制寄存器地址*/
}
static void mmu_init(void)
{
    __asm__(
        "ldr r0,=0x80000000\n"
        "mcr p15,0,r0,c2,c0,0\n"  /*设置页表基址寄存器*/
        
	//	"ldr r0,=0x0\n"
	//	"mcr p15,0,r0,c7,c7,0\n"  /*无效ICache DCache*/
	//	"mcr p15,0,r0,c7,c10,4\n" /*drain write buffer on v4*/
	//	"mcr p15,0,r0,c8,c7,0\n"  /*使无效指令，数据TLB*/
		
		"ldr r0,=0xffffffff\n"
        "mcr p15,0,r0,c3,c0,0\n"  /*域访问控制寄存器为0xffffffff,不进行权限检查,允许任何访问*/
        
		"mrc p15,0,r0,c1,c0,0\n"  /*读取控制寄存器的值*/
    //  "bic r0,r0,#0x3000\n"     
	//	"bic r0,r0,#0x0300\n"
	//	"bic r0,r0,#0x0087\n"
	//	"orr r0,r0,#0x0002\n"    /*开启对齐检查*/
		"orr r0,r0,#0x0004\n"    /*开启Dcache*/
		"orr r0,r0,#0x1000\n"    /*开启Icache*/
		"orr r0,r0,#0x0001\n"    /*使能MMU*/
        "mcr p15,0,r0,c1,c0,0\n" /*写入c1,使之有效*/ 
        :
        :
    );
}

int main(void)
{
    create_page_table();/*创建页表*/
    mmu_init();         /*MMU使能*/
    clk_enable();       /*操作虚拟地址，使能时钟*/ 
    led_init();         /*led_init 打开led*/
    while(1);
    return 0;
}




