#define pr_fmt(fmt)  "[%s:%d] : " fmt, __func__, __LINE__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <linux/version.h>
#include <linux/errno.h>
#include <linux/string.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/iommu.h>
#include <linux/msi.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/timer.h>

#include <linux/slab.h>
#include <linux/export.h>
#include <linux/atomic.h>
#include <linux/sched/signal.h>
#include <linux/poll.h>
#include <linux/reservation.h>
#include <linux/bitmap.h>

#include <linux/dma-fence.h>
#include <linux/sync_file.h>
#include <linux/dma-buf.h>
#include <linux/reservation.h>
#include <linux/mutex.h>


#define MAJOR_NUM 990
#define MM_SIZE 4096

char *buf = NULL;


static int misc_demo_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_IO;//表示对设备IO空间的映射
	// vma->vm_flags |= VM_RESERVED;//标志该内存区不能被换出，在设备驱动中虚拟页和物理页的关系应该是长期的，应该保留起来，不能随便被别的虚拟页换出
	if(remap_pfn_range(vma,//虚拟内存区域，即设备地址将要映射到这里
			vma->vm_start,//虚拟空间的起始地址
			virt_to_phys(buf)>>PAGE_SHIFT,//与物理内存对应的页帧号，物理地址右移12位
			vma->vm_end - vma->vm_start,//映射区域大小，一般是页大小的整数倍
			vma->vm_page_prot))//保护属性，
	{
		return -EAGAIN;
	}
	return 0;
}

static int misc_demo_open(struct inode * inode, struct file * filp)
{
	pr_info("Start open \n");
	buf = (char *)kmalloc(MM_SIZE, GFP_KERNEL);//内核申请内存只能按页申请，申请该内存以便后面把
	return 0;
}

static int misc_demo_release(struct inode * inode, struct file * filp)
{
	// 用户close 自己的dma-buf描述符时 ，已经释放了dma-buf,且在 dma_buf_ops->release释放了自己的buffer
	pr_info("Start close \n");
	if(buf)
		kfree(buf);

	return 0;
}

static ssize_t misc_demo_read(struct file * filp, char __user * buf, size_t size, loff_t * offset)
{
	return 0;
}

static ssize_t misc_demo_write(struct file * filp, const char __user * buf, size_t size, loff_t * offset)
{

	return 0;
}

static struct file_operations misc_demo_fops = {
	.owner = THIS_MODULE,
	.open  = misc_demo_open,
	.release = misc_demo_release,
	.read  = misc_demo_read,
	.write = misc_demo_write,
	.mmap = misc_demo_mmap,
};

static struct miscdevice misc_demo_drv = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "test",
	.fops  = &misc_demo_fops,
};

static int __init char_device_init(void)

{
	pr_info("register\n");
	return misc_register(&misc_demo_drv);
}
module_init(char_device_init);

static void __exit char_device_exit(void)
{
	pr_info("unregister\n");
	misc_deregister(&misc_demo_drv);
}
module_exit(char_device_exit);


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("TEST DMA-BUF");
MODULE_VERSION("1.0.0.0");
