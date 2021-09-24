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
#include <linux/poll.h>
#include <linux/reservation.h>
#include <linux/bitmap.h>

#include <linux/dma-fence.h>
#include <linux/sync_file.h>
#include <linux/dma-buf.h>
#include <linux/reservation.h>


#define BUF_SIZE (0x1024)
#define IOC_MAGIC	'E'
#define OPEN_DMABUF_FD	_IO(IOC_MAGIC, 0)
#define CLEAR_EXCL_FENCE _IO(IOC_MAGIC, 1)



#define FAKE_TIMER 1
#define DBUF_MAX_COUNT (1)

struct test_dmabuf_info {
	int dfd;
	spinlock_t lock;
	struct dma_buf *dmabuf;
	struct dma_fence *excl_fence;
	struct dma_fence *shared_fence;
};

struct test_info {
	struct test_dmabuf_info dinfo; // 目前只支持1个 dma-buf
	unsigned long dflags; // bitmap 来 表示三个dma-buf是否busy
	unsigned int dbus_pos;

	// 利用定时器模拟vblank中断，代表回写完成
#ifdef FAKE_TIMER
	struct timer_list my_timer;
	atomic_t timer_flags;
#endif
};
struct test_info g_tinfo;



const char * char_buf[][DBUF_MAX_COUNT] = {
	{"辉哥是很绅士的帅哥"},
	{"稳哥是带酒窝的渣男"},
	{"白阳是很正直的胖子"},
	{"亮哥是很努力的孩子"},
};
static unsigned char current_buf = 0;


// 这里不应该这么写，但临时测试使用
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
#if 0 //这部分内核已经实现了
	rcu_read_lock();
	fence = rcu_dereference(dmabuf->resv->fence_excl);
	rcu_read_unlock();

	dma_fence_put(fence);
	rcu_assign_pointer(dmabuf->resv->fence_excl, NULL);
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
// 回调接口
static void dma_fence_cb_func(struct dma_fence *fence,
				 struct dma_fence_cb *cb)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
}

const char *dmafence_test_name(struct dma_fence *fence)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return "dmafence_name";
}

const char *dmafence_test_timeline(struct dma_fence *fence)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return "dmafence_timeline";
}

bool dmafence_test_signal(struct dma_fence * fence)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return true; // TMD: 这里之前返回值没注意，return 0是 false，调试了大半天
}

void dmafence_test_release(struct dma_fence *fence)
{

#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	kfree(fence);
}

struct dma_fence_ops g_fence_ops = {
	.get_driver_name = dmafence_test_name,
	.get_timeline_name = dmafence_test_timeline,
	.enable_signaling = dmafence_test_signal,
	.wait = dma_fence_default_wait,
	.release = dmafence_test_release,
};


static const struct dma_buf_ops test_dmabuf_ops = {
	// 以下两个与设备dma相关
	.attach = test_dmabuf_ops_attach,
	.detach = test_dmabuf_ops_detach,

	// 以下几个基本必须实现
	.map_dma_buf = test_dmabuf_ops_map,
	.unmap_dma_buf = test_dmabuf_ops_unmap,
	.map = test_dmabuf_ops_kmap,
	.map_atomic = test_dmabuf_ops_kmap,
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

	// 4.注册 dma_buf的fence接口来清理dma_buf
	// 支持两个回调，都是给用户层使用，但poll使用同一个
	INIT_LIST_HEAD(&dmabuf->cb_excl.cb.node); // 测试用例暂时只用一种,但为了测试，两个都回调下 
	INIT_LIST_HEAD(&dmabuf->cb_shared.cb.node);
	// dmabuf->cb_excl.cb.func = test_dmabuf_excl_fence_func_t;  // TBD: 这种做法不是很规范，没有使用到 dma-buf的 resv功能;
	// dmabuf->cb_shared.cb.func = test_dmabuf_shared_fence_func_t;

	return dmabuf;
}


void dmafence_clear_flags(struct dma_fence *fence, struct dma_fence_cb *cb)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	fence->flags = 0;
}


static long misc_demo_ioctl(struct file * filp, unsigned int cmd, unsigned long args)
{
	switch(cmd){
	case OPEN_DMABUF_FD: // dmabuf-1:创建dma-buffer，并获取到用户态描述符 fd
	{
		int loop = 0;
		int ret = 0;
		int dfd[DBUF_MAX_COUNT] = {0};
		// 1.申请dma-buf，申请dma-fence *excl, *shared,
		// 2.拷贝所有fd到用户空间
//		for (loop=0; loop<DBUF_MAX_COUNT; ++loop) 
		{
			struct reservation_object_list *list;
			struct test_dmabuf_info *pdinfo = &g_tinfo.dinfo;
			struct dma_fence *fence;

			// 1. 初始化fencespinlock
			spin_lock_init(&pdinfo->lock);

			// 2. 初始化dma-buf
			pdinfo->dmabuf = dmabuf_buf_init();
			pdinfo->dfd = dma_buf_fd(pdinfo->dmabuf, O_CLOEXEC);
			dfd[loop] = pdinfo->dfd;

			// 3. 初始化excl fence
			fence = kzalloc(sizeof(*fence), GFP_KERNEL);
			dma_fence_init(fence, &g_fence_ops, &pdinfo->lock, 0, 0);
			pdinfo->excl_fence = fence;

			reservation_object_lock(pdinfo->dmabuf->resv, NULL);
			// rcu_assign_pointer(pdinfo->dmabuf->resv->fence_excl, fence);
			reservation_object_add_excl_fence(pdinfo->dmabuf->resv, fence);
			reservation_object_unlock(pdinfo->dmabuf->resv);
			pr_info("excl fence is %p\n", fence);
			fence = NULL;

			// 4. 初始化share fence
			fence = kzalloc(sizeof(*fence), GFP_KERNEL);
			dma_fence_init(fence, &g_fence_ops, &pdinfo->lock, 0, 0);
			pdinfo->shared_fence = fence;

			reservation_object_lock(pdinfo->dmabuf->resv, NULL);
			ret = reservation_object_reserve_shared(pdinfo->dmabuf->resv);
			if (ret == 0) {
				pr_err("reservation shared fence OK,  fence is %p\n",  fence);
				reservation_object_add_shared_fence(pdinfo->dmabuf->resv, fence);
				list = reservation_object_get_list(pdinfo->dmabuf->resv);
			} else {
				pr_err("reservation shared fence error\n");
			}
			if (list)
				pr_info("share fence is %p, count is %d, max is %d\n", fence, list->shared_count, list->shared_max);
			else
				pr_err("list is NULL\n");
			reservation_object_unlock(pdinfo->dmabuf->resv);
		}

		if (copy_to_user((void __user *)args, dfd, sizeof(int)*DBUF_MAX_COUNT) != 0)
			return -EFAULT;
		pr_info("get fd OK\n");
	}
	break;
	case CLEAR_EXCL_FENCE:
	{
		struct dma_fence *fence_excl;
		struct test_dmabuf_info *pdinfo = &g_tinfo.dinfo;
		// g_tinfo.fence->flags  = 0;  // BY_TBD
		bitmap_clear(&g_tinfo.dflags, 0, DBUF_MAX_COUNT); // 这里暂时全部 清0，在后续需要判断 
		pdinfo->excl_fence->flags = 0;
		pdinfo->shared_fence->flags = 0;
	}
	break;
	default:
		break;
	}
	return 0;
}

#ifdef FAKE_TIMER
void test_wake_up(struct dma_buf *dmabuf)
{
	// poll会检测这种实现的dma-fence，后期要使用这种方式
	unsigned shared_count, seq;
	struct dma_fence *fence_excl;
	struct reservation_object_list *fobj;

	fobj = reservation_object_get_list(dmabuf->resv);
	if (fobj)
		shared_count = fobj->shared_count;
	else
		shared_count = 0;

	fence_excl = reservation_object_get_excl(dmabuf->resv);
	pr_info("excle fence %p, fence_excl is %p, shared count is %d\n",
		fence_excl, fence_excl, shared_count);

	dma_fence_signal(fence_excl); // 这里使用excl任务
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
static void test_timer(struct timer_list *t)
{
	struct test_info *info = from_timer(info, t, my_timer);
#else
static void test_timer(unsigned long data)
{
	struct test_info *info = (struct test_info *)data;
#endif
	unsigned long next_tm = (get_random_long() & 0x03) + 3;	// wait 3-6s,
	atomic_set(&g_tinfo.timer_flags, 0);
	if (1) { // 需要删除
		printk("\n\n\n");
		pr_info("start vblank interrupt\n");
		if (test_bit(g_tinfo.dbus_pos, &g_tinfo.dflags)) {
			pr_err("Buffer %d is busy, skip!!!\n", g_tinfo.dbus_pos);
		} else {
			struct test_dmabuf_info *pdinfo = &g_tinfo.dinfo; // TBD：需要换成数组索引

			pr_err("Buffer is free, copy: %s\n", char_buf[current_buf][g_tinfo.dbus_pos]); // 暂时只用一个buffer，不用pingpang
			memset(pdinfo->dmabuf->priv, 0, BUF_SIZE);
			strcpy(pdinfo->dmabuf->priv, char_buf[current_buf][g_tinfo.dbus_pos]);
			test_wake_up(pdinfo->dmabuf);

			set_bit(g_tinfo.dbus_pos, &g_tinfo.dflags);
			if((++g_tinfo.dbus_pos) >= DBUF_MAX_COUNT)
				g_tinfo.dbus_pos = 0;
			current_buf = (current_buf+1) & 0x03;
		}
	}
	pr_info("next timer after  %lds\n\n\n", next_tm);
	mod_timer(&info->my_timer, jiffies + msecs_to_jiffies(next_tm * 1000));
	atomic_set(&g_tinfo.timer_flags, 1);
}
#endif


static int misc_demo_open(struct inode * inode, struct file * filp)
{
	unsigned long next_tm = (get_random_long() & 0x03) + 3; // wait 3-6s,
	bitmap_clear(&g_tinfo.dflags, 0, DBUF_MAX_COUNT);
#ifdef FAKE_TIMER // 使用定时任务模拟实际场景,5s后触发
	timer_setup(&g_tinfo.my_timer, test_timer, 0);
	g_tinfo.my_timer.expires = jiffies + msecs_to_jiffies(next_tm * 1000);
	pr_info("next timer after  %lds\n", next_tm);
	add_timer(&g_tinfo.my_timer);
	atomic_set(&g_tinfo.timer_flags, 1);
#endif

	return 0;
}
static int misc_demo_release(struct inode * inode, struct file * filp)
{
	// 用户close 自己的dma-buf描述符时 ，已经释放了dma-buf,且在 dma_buf_ops->release释放了自己的buffer
	// 用户一旦打开，必须要关闭dma-buf的描述符，要不驱动无法卸载；
	bitmap_clear(&g_tinfo.dflags, 0, DBUF_MAX_COUNT);
#ifdef FAKE_TIMER
	if (atomic_read(&g_tinfo.timer_flags))
		del_timer_sync(&g_tinfo.my_timer);
	atomic_set(&g_tinfo.timer_flags, 0);
#endif

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
