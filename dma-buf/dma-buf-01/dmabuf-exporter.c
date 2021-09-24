/*
 * dmabuf.
 *
 * (C) 2021.07.28 <baiyang0223.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*************************************************************************
	> File Name: dma-buf-test01.c
	> Author: baiy
	> Mail: baiyang0223@163.com 
	> Created Time: 2021-07-25-14:05:37
	> Func: 测试dma-buf的基础功能
 ************************************************************************/
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


#include <linux/dma-fence.h>
#include <linux/sync_file.h>
#include <linux/dma-buf.h>

#define BUF_SIZE (0x1024)
#define IOC_MAGIC	'E'
#define OPEN_DMABUF_FD	_IO(IOC_MAGIC, 0)


struct test_info {
	struct dma_buf *dmabuf;
};
struct test_info g_tinfo;


static int test_dmabuf_ops_attach(struct dma_buf *dbuf, struct device *dev,
	struct dma_buf_attachment *dbuf_attach)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return 0;
}

static void test_dmabuf_ops_detach(struct dma_buf *dbuf,
	struct dma_buf_attachment *db_attach)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return;
}

static struct sg_table *test_dmabuf_ops_map(
	struct dma_buf_attachment *db_attach, enum dma_data_direction dma_dir)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return NULL;

}

static void test_dmabuf_ops_unmap(struct dma_buf_attachment *db_attach,
	struct sg_table *sgt, enum dma_data_direction dma_dir)
{
	/* nothing to be done here */
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return;
}


static void test_dmabuf_ops_release(struct dma_buf *dbuf)
{
	/* drop reference obtained in test_get_dmabuf */
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	kfree(dbuf->priv);
	return;
}

static void *test_dmabuf_ops_kmap(struct dma_buf *dbuf, unsigned long pgnum)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return dbuf->priv;

}

static void *test_dmabuf_ops_vmap(struct dma_buf *dbuf)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return dbuf->priv;
}

static int test_dmabuf_ops_mmap(struct dma_buf *dbuf,
	struct vm_area_struct *vma)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return 0;

}

static const struct dma_buf_ops test_dmabuf_ops = {
	// 以下两个与设备dma相关
	.attach = test_dmabuf_ops_attach,
	.detach = test_dmabuf_ops_detach,

	// 以下几个基本必须实现
	.map_dma_buf = test_dmabuf_ops_map,
	.unmap_dma_buf = test_dmabuf_ops_unmap,
	.map = test_dmabuf_ops_kmap,
	//.map_atomic = test_dmabuf_ops_kmap,
	.vmap = test_dmabuf_ops_vmap,
	.mmap = test_dmabuf_ops_mmap,
	.release = test_dmabuf_ops_release,
};

static struct dma_buf *dmabuf_buf_init(void)
{
	struct dma_buf *dmabuf;

	// 1. 准备 buf
	void *buf  = kzalloc(BUF_SIZE, GFP_KERNEL);

	// 2.准备 dma_buf_export_info
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

	if(!buf) {
		pr_err("Short of memory\n");
		return NULL;
	}
	strcpy(buf, "Hello World\n");
	exp_info.ops = &test_dmabuf_ops;
	exp_info.size = BUF_SIZE;
	exp_info.flags = O_RDWR;
	exp_info.priv = buf;

	// 3.获取 dma_buf, dma_buf_release 函数释放
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		kfree(buf);
		return NULL;
	}
	return dmabuf;
}

static long misc_demo_ioctl(struct file * filp, unsigned int cmd, unsigned long args)
{
	switch(cmd){
	case OPEN_DMABUF_FD: // dmabuf-1:创建dma-buffer，并获取到用户态描述符 fd
	{
		int fd;
		g_tinfo.dmabuf = dmabuf_buf_init();
		if (!g_tinfo.dmabuf)
			return -ENOMEM;
		fd = dma_buf_fd(g_tinfo.dmabuf, O_CLOEXEC);
		pr_info("start create fd is %d\n\n", fd);
		if (copy_to_user((void __user *)args, &fd, sizeof(int)) != 0)
			return -EFAULT;
	}
	break;
	default:
		break;
	}
	return 0;
}

static int misc_demo_open(struct inode * inode, struct file * filp)
{
	return 0;
}
static int misc_demo_release(struct inode * inode, struct file * filp)
{
	// 用户close 描述符时 ，已经释放了dma-buf,且在 dma_buf_ops->release释放了自己的buffer
	// 用户一旦打开，必须要关闭dma-buf的描述符，要不驱动无法卸载；

	return 0;
}

static ssize_t
misc_demo_read(struct file * filp, char __user * buf, size_t size, loff_t * offset)
{
	return 0;
}

static ssize_t
misc_demo_write(struct file * filp, const char __user * buf, size_t size, loff_t * offset)
{

	return 0;
}

static struct file_operations misc_demo_fops = {
	.owner = THIS_MODULE,
	.open  = misc_demo_open,
	.release = misc_demo_release,
	.read  = misc_demo_read,
	.write = misc_demo_write,
	.unlocked_ioctl = misc_demo_ioctl,
	.compat_ioctl   = misc_demo_ioctl,
};

static struct miscdevice misc_demo_drv = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "exporter",
	.fops  = &misc_demo_fops,
};

static int __init dmabuf_exporter_init(void)
	
{
	pr_info("register\n");
	return misc_register(&misc_demo_drv);
	return 0;
}
module_init(dmabuf_exporter_init);

static void __exit dmabuf_exporter_exit(void)
{
	pr_info("unregister\n");
	misc_deregister(&misc_demo_drv);
}
module_exit(dmabuf_exporter_exit);


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("TEST DMA-BUF");
MODULE_VERSION("1.0.0.0");
