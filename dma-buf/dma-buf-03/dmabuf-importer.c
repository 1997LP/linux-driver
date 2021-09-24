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
#include <linux/reservation.h>


#define BUF_SIZE (0x1024)
#define IOC_MAGIC	'E'
#define SET_DMABUF_FD	_IO(IOC_MAGIC, 10)
#define TEST_DMABUF_RW	_IO(IOC_MAGIC, 11)
#define UNSET_DMABUF_FD	_IO(IOC_MAGIC, 12)
#define TEST_DMABUF_USED	_IO(IOC_MAGIC, 13)



struct test_info {
	struct dma_buf *dmabuf;
};
struct test_info g_tinfo;

static int importer_test(struct dma_buf *dmabuf)
{
	void *vaddr;

	vaddr = dma_buf_kmap(dmabuf, 0);	// called : test_dmabuf_ops_kmap
	pr_info("read from dmabuf kmap: %s\n", (char *)vaddr);
	dma_buf_kunmap(dmabuf, 0, vaddr);	// called : dmabuf->ops->unmap 可选实现

	vaddr = dma_buf_vmap(dmabuf);	// called : test_dmabuf_ops_vmap
	pr_info("read from dmabuf vmap: %s\n", (char *)vaddr);
	dma_buf_vunmap(dmabuf, vaddr);	// called : dmabuf->ops->vunmap 可选实现

	return 0;
}

static int importer_delay_test(struct dma_buf *dmabuf)
{
	int ret;
	void *vaddr;
	unsigned long next_tm = (get_random_long() & 0x03) + 1; // wait 1-4s
	struct dma_fence **shared_excl;
	struct dma_fence *fence_excl;
	struct reservation_object *obj = dmabuf->resv;
	unsigned  int  shared_count;


	vaddr = dma_buf_kmap(dmabuf, 0);	// called : test_dmabuf_ops_kmap
	pr_err("read from dmabuf kmap: %s,then sleep %lds\n", (char *)vaddr, next_tm);
	dma_buf_kunmap(dmabuf, 0, vaddr);	// called : dmabuf->ops->unmap 可选实现

	msleep(next_tm*1000);
	pr_info("Start call back\n\n\n");
	// dmabuf->cb_shared.cb.func(NULL, &dmabuf->cb_shared.cb);

	ret = reservation_object_get_fences_rcu(obj, &fence_excl, &shared_count, &shared_excl);
	if (ret) {
		pr_err("get fences error\n");
	} else {
		pr_info("get fences ok, excl  %p, shared  %p, shardcount %d\n",
			fence_excl, shared_excl[0], shared_count);
	}
	dma_fence_signal(shared_excl[0]); // 这里使用excl任务

	return 0;
}

static long misc_demo_ioctl(struct file * filp, unsigned int cmd, unsigned long args)
{
	switch(cmd){
	case SET_DMABUF_FD: // dmabuf-importer-1 : 根据用户态fd，获取dma-buffer
	{
		int fd = 0;
		if (copy_from_user(&fd, (void __user *)args, sizeof(int)) != 0)
			return -EFAULT;
		g_tinfo.dmabuf = dma_buf_get(fd);
	}
	break;
	case TEST_DMABUF_RW: // dmabuf-import-2 : 测试dma-buffer的读写
	{
		importer_test(g_tinfo.dmabuf);
	}
	break;
	case UNSET_DMABUF_FD : //dmabuf-import-3 : 释放dma-buffer
	{
		if(g_tinfo.dmabuf)
			dma_buf_put(g_tinfo.dmabuf);
		g_tinfo.dmabuf = NULL;
	}
	break;
	case TEST_DMABUF_USED:
	{
		// 因为这里只是自己使用，用户态没有poll，所以暂时不使用dma-fence
		// 1. 等待随机时间，模拟实际应用
		// 2. dump buffer， 然后调用回调来设置flags；
		importer_delay_test(g_tinfo.dmabuf);
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
	if (g_tinfo.dmabuf) {
		dma_buf_put(g_tinfo.dmabuf);
	}
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
	.name  = "importer",
	.fops  = &misc_demo_fops,
};

static int __init dmabuf_importer_init(void)
{
	pr_info("register\n");
	return misc_register(&misc_demo_drv);
}
module_init(dmabuf_importer_init);

static void __exit dmabuf_importer_exit(void)
{
	pr_info("unregister\n");
	misc_deregister(&misc_demo_drv);
}
module_exit(dmabuf_importer_exit);


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("TEST DMA-BUF");
MODULE_VERSION("1.0.0.0");
