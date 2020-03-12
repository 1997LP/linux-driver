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
#include <linux/of_gpio.h>


#define GPIOLED_CNT   1             /*设备号个数*/
#define GPIOLED_NAME  "gpioled" 
#define LEDON  1 
#define LEDOFF 0

struct gpioled_dev{
    dev_t          devid ;             /*设备号*/
    struct cdev    cdev;
    struct class  *class;
    struct device *devices;
    int major;
    int minor;
    struct device_node *nd;  /*设备节点*/
    int led_gpio;            /*led所使用的编号*/
};
struct gpioled_dev gpioled;    /*led设备*/

static int led_open(struct inode *inode,struct file *filp)
{
    filp->private_data = &gpioled;/*设置私有数据*/
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
    struct gpioled_dev *dev = filp->private_data;
    retvalue=copy_from_user(databuf,buf,cnt);
    if(retvalue<0){
        printk("kernel write error");
        return 1;
    }
    ledstat=databuf[0];
    if(ledstat==LEDON)
        gpio_set_value(dev->led_gpio,0);
    else if(ledstat==LEDOFF){
        gpio_set_value(dev->led_gpio,1);
    }
    return 0;
}
static int led_release(struct inode *inode,struct file *filp)
{
	return 0;
}
static struct file_operations gpioled_fops={
    .owner= THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write= led_write,
    .release= led_release
};

static int __init led_init(void)
{
    int ret;
    /*获取设备节点*/
    gpioled.nd = of_find_node_by_path("/gpioled");
    if(gpioled.nd==NULL){
        printk("alphaled node not find! \r\n");
        return -EINVAL;
    }else {
        printk("alphaled node find!\r\n");
    }
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd,"led-gpio",0);
    if(gpioled.led_gpio < 0 ){
        printk("can't get led-gpio \r\n");
        return -EINVAL;
    }
    printk("led-gpio num is %d\r\n" , gpioled.led_gpio);
    ret = gpio_request(gpioled.led_gpio, "GPIO1");
    if(ret <0 ){
        printk("GPIO request error \r\n");
    }
    printk("GPIO request OK \r\n");

    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if(ret <0 ){
        printk("GPIO direction set ERROR \r\n");
    }
    printk("GPIO direction set OK \r\n");

    /*创建设备号*/
    if(gpioled.major){    /*如果定义了设备号*/
        gpioled.devid = MKDEV(gpioled.major,0);  //确定设备号
        register_chrdev_region(gpioled.devid,GPIOLED_CNT,
                        GPIOLED_NAME);  /*注册设备号*/
    }else {
        alloc_chrdev_region(&gpioled.devid,0,GPIOLED_CNT,
                        GPIOLED_NAME);  /*申请设备号*/
        register_chrdev_region(gpioled.devid,GPIOLED_CNT,
                        GPIOLED_NAME);  /*注册设备号*/               
        gpioled.major = MAJOR(gpioled.devid);
        gpioled.minor = MINOR(gpioled.devid);
    }
    printk("gpioled major = %d, minor = %d \r\n",
                   gpioled.major,gpioled.minor);
    gpioled.cdev.owner = THIS_MODULE;  /*cdev结构体初始化*/
    cdev_init(&gpioled.cdev,&gpioled_fops);
    cdev_add( &gpioled.cdev,gpioled.devid,GPIOLED_CNT);  /*添加字符设备*/
    /*创建设备节点*/
    gpioled.class = class_create(THIS_MODULE,GPIOLED_NAME);/*创建类*/
    if(IS_ERR(gpioled.class)){
        return PTR_ERR(gpioled.class);
    }
    gpioled.devices = device_create(gpioled.class,NULL ,   /*创建设备*/
                    gpioled.devid,NULL,GPIOLED_NAME);
    if(IS_ERR(gpioled.devices)){
        return PTR_ERR(gpioled.devices);
    }
    return 0;
}

static void __exit led_exit(void)
{

    unregister_chrdev_region(gpioled.devid,1);//释放掉设备号
    cdev_del(&gpioled.cdev);
    class_destroy(gpioled.class);
    device_destroy(gpioled.class,gpioled.devid);

}
module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liupeng Peng");

