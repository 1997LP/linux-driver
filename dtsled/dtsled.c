#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>


#define DTSLED_CNT   1             /*设备号个数*/
#define DTSLED_NAME  "dtsled" 
#define LEDON  1 
#define LEDOFF 0

/*映射后的寄存器虚拟地址指针*/
static void __iomem  *IMX6U_CCM_CCGR1  ;
static void __iomem  *SW_MUX_GPIO1_IO03; 
static void __iomem  *SW_PAD_GPIO1_IO03;
static void __iomem  *GPIO1_DR;
static void __iomem  *GPIO1_GDIR;

struct dtsled_dev{
    dev_t devid ;             /*设备号*/
    struct cdev   cdev;
    struct class *class;
    struct device *devices;
    int major;
    int minor;
    struct device_node *nd;  /*设备节点*/
};
struct dtsled_dev dtsled;    /*led设备*/

void led_switch(u8 sta)
{
    u32 val=0;
    if(sta==LEDON){
        val=readl(GPIO1_DR);
        val&=~(1<<3);
        writel(val,GPIO1_DR);
    }else {
        val=readl(GPIO1_DR);
        val|=(1<<3);
        writel(val,GPIO1_DR);
    }
}

static int led_open(struct inode *inode,struct file *filp)
{
    filp->private_data = &dtsled;/*设置私有数据*/
	return 0;
}
static ssize_t led_read(struct file *filp, char __user *buf,
            size_t cnt, loff_t *offt)
{
    return 0;
}
static ssize_t led_write(struct file*filp, const char __user *buf,
            size_t cnt,loff_t *offt)
{
    int retvalue;
    u8 databuf[1];
    u8 ledstat;
    retvalue=copy_from_user(databuf,buf,cnt);
    if(retvalue<0){
        printk("kernel write error");
        return 1;
    }
    ledstat=databuf[0];
    if(ledstat==LEDON)
        led_switch(LEDON);
    else if(ledstat==LEDOFF){
        led_switch(LEDOFF);
    }
    return 0;
}
static int led_release(struct inode *inode,struct file *filp)
{
	return 0;
}
static struct file_operations dtsled_fops={
    .owner= THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write= led_write,
    .release= led_release
};


static int __init led_init(void)
{
    u32 val=0;
    int ret;
    u32 regdata[14];
    const char *str;
    struct property *proper;
    
    /*获取设备节点*/
    dtsled.nd = of_find_node_by_path("/alphaled");
    if(dtsled.nd==NULL){
        printk("alphaled node not find! \r\n");
        return -EINVAL;
    }else {
        printk("alphaled node find!\r\n");
    }

    /*获取compatible属性*/
    proper=of_find_property(dtsled.nd,"compatible",NULL);
    if(proper==NULL){
        printk("compatible property find failed !\r\n");
    }else {
        printk("compatible= %s\r\n",(char *)proper->value);
    }
    
    /*获取status属性*/
    ret = of_property_read_string(dtsled.nd,"status",&str);
    if(ret<0){
        printk("status read fail !\r\n");
    }else {
        printk("status = %s \r\n",str);
    }

    /*获取reg属性*/
    ret = of_property_read_u32_array(dtsled.nd, "reg", regdata,10);
    if(ret <0 ){
        printk ("reg property read fail !\r\n");
    }else{
        u8 i=0;
        printk("reg data:\r\n");
        for(i=0;i<10;i++)
            printk("%#x ",regdata[i]);
        printk("\r\n");
    }
#if 0
    IMX6U_CCM_CCGR1    = ioremap(CCM_CCGR1_BASE,4);
    SW_MUX_GPIO1_IO03  = ioremap(SW_MUX_GPIO1_IO03_BASE,4);
    SW_PAD_GPIO1_IO03  = ioremap(SW_PAD_GPIO1_IO03_BASE,4);
    GPIO1_DR           = ioremap(GPIO1_DR_BASE,4);
    GPIO1_GDIR         = ioremap(GPIO1_GDIR_BASE,4);      
#else 
    IMX6U_CCM_CCGR1    = of_iomap(dtsled.nd,0);
    SW_MUX_GPIO1_IO03  = of_iomap(dtsled.nd,1);
    SW_PAD_GPIO1_IO03  = of_iomap(dtsled.nd,2);
    GPIO1_DR           = of_iomap(dtsled.nd,3);
    GPIO1_GDIR         = of_iomap(dtsled.nd,4);
#endif
    val = readl(IMX6U_CCM_CCGR1);
    val&= ~(3<<26);
    val|=  (3<<26);
    writel(val,IMX6U_CCM_CCGR1);

    writel(5,SW_MUX_GPIO1_IO03);
    
    writel(0x10b0,SW_PAD_GPIO1_IO03);

    val = readl(GPIO1_GDIR);
    val|= (1<<3);
    writel(val,GPIO1_GDIR);

    val = readl(GPIO1_DR);
    val|= (1<<3);
    writel(val,GPIO1_DR);
    
    /*创建设备号*/
    if(dtsled.major){    /*如果定义了设备号*/
        dtsled.devid = MKDEV(dtsled.major,0);  //确定设备号
        register_chrdev_region(dtsled.devid,DTSLED_CNT,
                        DTSLED_NAME);  /*注册设备号*/
    }else {
        alloc_chrdev_region(&dtsled.devid,0,DTSLED_CNT,
                        DTSLED_NAME);  /*申请设备号*/
        dtsled.major = MAJOR(dtsled.devid);
        dtsled.minor = MINOR(dtsled.devid);
    }
    printk("dtsled major = %d, minor = %d \r\n",
                   dtsled.major,dtsled.minor);
    dtsled.cdev.owner = THIS_MODULE;  /*cdev结构体初始化*/
    cdev_init(&dtsled.cdev,&dtsled_fops);
    cdev_add( &dtsled.cdev,dtsled.devid,DTSLED_CNT);  /*添加字符设备*/
    /*创建设备节点*/
    dtsled.class = class_create(THIS_MODULE,DTSLED_NAME);/*创建类*/
    if(IS_ERR(dtsled.class)){
        return PTR_ERR(dtsled.class);
    }
    dtsled.devices = device_create(dtsled.class,NULL ,   /*创建设备*/
                    dtsled.devid,NULL,DTSLED_NAME);
    if(IS_ERR(dtsled.devices)){
        return PTR_ERR(dtsled.devices);
    }
    return 0;
}

static void __exit led_exit(void)
{
    iounmap(IMX6U_CCM_CCGR1)  ;
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);      
   
    unregister_chrdev_region(dtsled.devid,1);//释放掉设备号
    cdev_del(&dtsled.cdev);
    class_destroy(dtsled.class);
    device_destroy(dtsled.class,dtsled.devid);
}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liupeng Peng");

