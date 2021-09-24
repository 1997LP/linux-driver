/*************************************************************************
	> File Name: dmabuf-exporter.c
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
#include <linux/mutex.h>

#define BUF_SIZE (0x1024)
#define IOC_MAGIC	'E'
#define OPEN_DMABUF_FD	_IO(IOC_MAGIC, 0)
#define CLEAR_EXCL_FENCE _IO(IOC_MAGIC, 1)
#define SET_WM_FD  _IO(IOC_MAGIC, 2)


#define FAKE_TIMER 1
#define DBUF_MAX_COUNT (3)

struct test_dmabuf_info {
	struct list_head node;
	spinlock_t lock;
	struct dma_buf *dmabuf;
	int dfd;
	void *buf;
};

struct test_info {
	unsigned int dbus_pos;
	struct dma_buf * dbuf;
	struct list_head head;
#ifdef FAKE_TIMER 	// 利用定时器模拟vblank中断，代表回写完成
	struct timer_list my_timer;
#endif
};
struct test_info g_tinfo;

const char * char_buf[4] = {
	"辉哥是很绅士的帅哥",
	"稳哥是带酒窝的渣男",
	"白阳是很正直的胖子",
	"今天下午又要对绩效了，着急不！！！"
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
	struct test_dmabuf_info *pdinfo = dbuf->priv;
	/* drop reference obtained in test_get_dmabuf */
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	pr_info("dbuf is %p, priv is %p ref is %ld\n", dbuf, dbuf->priv, atomic64_read(&dbuf->file->f_count));
	do{
		struct dma_fence *efence;
		struct dma_fence *sfence;
		struct reservation_object_list *fobj;
		struct reservation_object *resv;
		unsigned int rcnt;
		int i = 0;

		resv = dbuf->resv;
		// 以下部分在 reservation_object_fini实现，但好像未release掉fence，这里调试下
		efence = rcu_dereference_protected(resv->fence_excl, 1);
		if (efence) {
			dma_fence_put(efence);
			rcnt = kref_read(&efence->refcount);
			pr_info("dbuf efence rcnt is %u\n", rcnt);
		}

		fobj = rcu_dereference_protected(resv->fence, 1);
		if (fobj) {
			for (i=0; i<fobj->shared_count; ++i) {
				sfence = rcu_dereference_protected(fobj->shared[i], i);
				dma_fence_put(sfence);
				rcnt = kref_read(&sfence->refcount);
				pr_info("dbuf sfence %d rcnt is %u\n", i, rcnt);
			}
		}
	}while(0);
	if (pdinfo->buf) {
		kfree(pdinfo->buf);
		pdinfo->buf = NULL;
	}
	if (pdinfo) {
		kfree(pdinfo);
	}
	dbuf->priv = NULL;
	return;
}

static void *test_dmabuf_ops_kmap(struct dma_buf *dbuf, unsigned long pgnum)
{
	struct test_dmabuf_info *pdinfo = dbuf->priv;
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif

	return pdinfo->buf;

}

static void *test_dmabuf_ops_vmap(struct dma_buf *dbuf)
{
	struct test_dmabuf_info *pdinfo = dbuf->priv;
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	return pdinfo->buf;
}

static int test_dmabuf_ops_mmap(struct dma_buf *dbuf,
	struct vm_area_struct *vma)
{
	struct test_dmabuf_info *pdinfo = dbuf->priv;
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif

#if 0
	// vma->vm_flags |= VM_IO;//表示对设备IO空间的映射
	// vma->vm_flags |= VM_RESERVED;//标志该内存区不能被换出，在设备驱动中虚拟页和物理页的关系应该是长期的，应该保留起来，不能随便被别的虚拟页换出
	if(remap_pfn_range(vma,//虚拟内存区域，即设备地址将要映射到这里
			vma->vm_start,//虚拟空间的起始地址
			virt_to_phys(pdinfo->buf)>>PAGE_SHIFT,//与物理内存对应的页帧号，物理地址右移12位
			vma->vm_end - vma->vm_start,//映射区域大小，一般是页大小的整数倍
			vma->vm_page_prot))//保护属性，
	{
		return -EAGAIN;
	}
#endif
	return 0;

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
	pr_info(" called by  %pF, fence =%p\n", __builtin_return_address(0), fence);
#endif
	return "dmafence_timeline";
}

bool dmafence_test_signal(struct dma_fence * fence)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF, fence =%p\n", __builtin_return_address(0), fence);
#endif
	// 这个函数在首次wait/添加任务时，会调用，必须返回true才能继续
	if (fence)
		return true; // TMD: 这里之前返回值没注意，return 0，调试了大半天
	else
		return false;
}

void dmafence_test_release(struct dma_fence *fence)
{

#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF, fence =%p\n", __builtin_return_address(0), fence);
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
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

	// 1. 准备 buf
	void *buf  = kzalloc(BUF_SIZE, GFP_KERNEL);
	if(!buf) {
		pr_err("Short of memory\n");
		return NULL;
	}

	// 2.准备 dma_buf_export_info
	exp_info.ops = &test_dmabuf_ops;
	exp_info.size = BUF_SIZE;
	exp_info.flags = O_RDWR;
	exp_info.priv = buf;

	// 3.获取 dma_buf, dma_buf_release 函数释放
	dmabuf = dma_buf_export(&exp_info); // 使用默认的 resv分配
	if (IS_ERR(dmabuf)) {
		kfree(buf);
		return NULL;
	}

	// 4.注册 dma_buf的fence接口来清理dma_buf
	// 支持两个回调，都是给用户层使用，但poll使用同一个
	INIT_LIST_HEAD(&dmabuf->cb_excl.cb.node); // 测试用例暂时只用一种,但为了测试，两个都回调下
	INIT_LIST_HEAD(&dmabuf->cb_shared.cb.node);

	// pr_info("dbuf is %p, ref is %ld\n", dmabuf, atomic64_read(&dmabuf->file->f_count));
	return dmabuf;
}


void dmafence_clear_flags(struct dma_fence *fence, struct dma_fence_cb *cb)
{
#ifdef CONFIG_KALLSYMS
	pr_info(" called by  %pF\n", __builtin_return_address(0));
#endif
	if (fence)
		fence->flags = 0;
}


static void misc_clearall_fence(void)
{
	struct test_dmabuf_info *pdinfo;
	struct test_dmabuf_info *tmp;

	struct reservation_object *obj;
	struct dma_fence *excl;
	struct dma_fence *shared;
	struct reservation_object_list *fobj;
	int i = 0;
	unsigned long flags;

	list_for_each_entry_safe(pdinfo, tmp, &g_tinfo.head, node) {
		if (!pdinfo->dmabuf)
			return;

		// BUG_ON(dmabuf->cb_shared.active || dmabuf->cb_excl.active);
		pdinfo->dmabuf->cb_shared.active = 0;
		pdinfo->dmabuf->cb_excl.active = 0;

		obj = pdinfo->dmabuf->resv;
		if (!obj)
			continue;

		excl = rcu_dereference_protected(obj->fence_excl,  1);
		if (excl) {
			if (!list_empty(&excl->cb_list)) {
				struct dma_fence_cb *cur, *tmp;

				spin_lock_irqsave(excl->lock, flags);
				list_for_each_entry_safe(cur, tmp, &excl->cb_list, node) {
					list_del_init(&cur->node);
				}
				spin_unlock_irqrestore(excl->lock, flags);
			}
		}

		fobj = rcu_dereference_protected(obj->fence, 1);
		if (fobj) {
			for (i = 0; i < fobj->shared_count; ++i) {
				shared  = rcu_dereference_protected(fobj->shared[i], 1);
				if (shared && !list_empty(&shared->cb_list)) {
					struct dma_fence_cb *cur, *tmp;
					spin_lock_irqsave(shared->lock, flags);
					list_for_each_entry_safe(cur, tmp, &shared->cb_list, node) {
						list_del_init(&cur->node);
					}
					spin_unlock_irqrestore(shared->lock, flags);
				}
			}
		}
	}
}

static long misc_demo_ioctl(struct file * filp, unsigned int cmd, unsigned long args)
{
	switch(cmd){
	case OPEN_DMABUF_FD: // dmabuf-1:创建dma-buffer，并获取到用户态描述符 fd
	{
		// 1.申请dma-buf，申请dma-fence *excl, *shared,
		// 2.拷贝所有fd到用户空间
		struct reservation_object_list *list;
		struct test_dmabuf_info *pdinfo = kzalloc(sizeof(*pdinfo), GFP_KERNEL);
		struct dma_fence *efence;
		struct dma_fence *sfence;

		// 1. 初始化fencespinlock
		spin_lock_init(&pdinfo->lock);
		INIT_LIST_HEAD(&pdinfo->node);

		// 2. 初始化dma-buf + exporter
		pdinfo->dmabuf = dmabuf_buf_init();
		pdinfo->buf = pdinfo->dmabuf->priv;
		pdinfo->dmabuf->priv = pdinfo; // 这里用dmabuf->priv保存句柄，类似于gem object一样
		pdinfo->dfd = dma_buf_fd(pdinfo->dmabuf, O_CLOEXEC);
		list_add(&pdinfo->node, &g_tinfo.head);
		pr_info("pdinfo is %p, pfd is  %d dbuf is %p\n",pdinfo, pdinfo->dfd, pdinfo->dmabuf);
#if 0	// 使用动态创建fence
		// 3. 初始化excl fence
		efence = kzalloc(sizeof(*efence), GFP_KERNEL);
		dma_fence_init(efence, &g_fence_ops, &pdinfo->lock, 0, 0);
		pdinfo->excl_fence = efence;
		do{
			unsigned int rcnt;
			rcnt = kref_read(&efence->refcount);
			pr_info("dbuf efence rcnt is %u\n", rcnt);
		}while(0);

		reservation_object_lock(pdinfo->dmabuf->resv, NULL);
		reservation_object_add_excl_fence(pdinfo->dmabuf->resv, efence);
		reservation_object_unlock(pdinfo->dmabuf->resv);
		pr_info("loop %d excl fence is %p\n", loop, efence);
		do{
			unsigned int rcnt;
			rcnt = kref_read(&efence->refcount);
			pr_info("dbuf efence rcnt is %u\n", rcnt);
		}while(0);
#endif

#if 0
		// 4. 初始化share fence
		sfence = kzalloc(sizeof(*sfence), GFP_KERNEL);
		dma_fence_init(sfence, &g_fence_ops, &pdinfo->lock, 0, 0);
		pdinfo->shared_fence = sfence;
		do{
			unsigned int rcnt;
			rcnt = kref_read(&sfence->refcount);
			pr_info("dbuf sfence %d rcnt is %u\n", loop, rcnt);
		}while(0);

		reservation_object_lock(pdinfo->dmabuf->resv, NULL);
		ret = reservation_object_reserve_shared(pdinfo->dmabuf->resv);
		if (ret == 0) {
			reservation_object_add_shared_fence(pdinfo->dmabuf->resv, sfence);
			list = reservation_object_get_list(pdinfo->dmabuf->resv);
		} else {
			pr_err("reservation shared fence error\n");
		}
		if (list) {
			pr_info("loop %d share fence is %p, count is %d, max is %d\n",\
							loop, sfence, list->shared_count, list->shared_max);
		} else {
			pr_err("list is NULL\n");
		}
		reservation_object_unlock(pdinfo->dmabuf->resv
		do{
			unsigned int rcnt;
			rcnt = kref_read(&efence->refcount);
			pr_info("dbuf efence rcnt is %u\n", rcnt);
			rcnt = kref_read(&sfence->refcount);
			pr_info("dbuf sfence %d rcnt is %u\n", loop, rcnt);
		}while(0);
#endif
		if (copy_to_user((void __user *)args, &pdinfo->dfd, sizeof(int)) != 0)
			return -EFAULT;
	}
	break;
	case SET_WM_FD:
	{
		int pfd;
		struct dma_buf * dmabuf;
		struct reservation_object *obj;
		struct dma_fence *efence;
		unsigned long next_tm;
		struct test_dmabuf_info *pdinfo;

		get_user(pfd, (u32 __user *)args);
		dmabuf = dma_buf_get(pfd);
		obj = dmabuf->resv;
		if (!obj)
			return -ENOENT;
		pdinfo = dmabuf->priv;
		pr_info("pdinfo is %p,check  fd is %d\n", pdinfo, pfd);

		// 2. 创建efence
		efence = rcu_dereference_protected(obj->fence_excl,  1);
		if (efence) {
			pr_err("export dmabuf pfd %d is busy\n", pfd);
			return -EBUSY;
		}

		// 3. 初始化excl fence
		efence = kzalloc(sizeof(*efence), GFP_KERNEL);
		dma_fence_init(efence, &g_fence_ops, &pdinfo->lock, 0, 0);
		do{
			unsigned int rcnt;
			rcnt = kref_read(&efence->refcount);
			pr_info("dbuf efence rcnt is %u\n", rcnt);
		}while(0);

		reservation_object_lock(pdinfo->dmabuf->resv, NULL);
		reservation_object_add_excl_fence(pdinfo->dmabuf->resv, efence); // RCU_INIT_POINTER(obj->fence_excl, fence);
		reservation_object_unlock(pdinfo->dmabuf->resv);
#if 0
		pr_info("excl fence is %p\n", efence);
		do{
			unsigned int rcnt;
			rcnt = kref_read(&efence->refcount);
			pr_info("dbuf efence rcnt is %u\n", rcnt);
		}while(0);
#endif
		// 4. 添加定时器来模拟定时任务
		// mod_timer(&g_tinfo.my_timer, jiffies + _usecs_to_jiffies(16700)); // wait 16.7ms
		mod_timer(&g_tinfo.my_timer, jiffies + msecs_to_jiffies(2000)); // wait 16.7ms
		// g_tinfo.dbus_pos = pfd;
		g_tinfo.dbuf = dmabuf;
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
	struct dma_fence *fence_excl;
	unsigned int rcnt;

	fence_excl = reservation_object_get_excl(dmabuf->resv);
	pr_info("!!! wake excle fence %p, fence_excl is %p\n",
		fence_excl, fence_excl);
	if (fence_excl) {
		dma_fence_signal(fence_excl); // 这里使用excl任务
		dma_fence_put(fence_excl);
		rcnt = kref_read(&fence_excl->refcount);
		pr_info("dbuf efence rcnt is %u\n", rcnt);
		dma_fence_put(fence_excl);  // 使用完就释放
		RCU_INIT_POINTER(dmabuf->resv->fence_excl, NULL);
	}
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
	struct dma_buf * dbuf = g_tinfo.dbuf;
	printk("\n\n\n");
	pr_info("start vblank interrupt\n");
	// dbuf = dma_buf_get(g_tinfo.dbus_pos); // ???
	if (dbuf) {
		struct test_dmabuf_info *pdinfo = dbuf->priv;
		memset(pdinfo->buf, 0, BUF_SIZE);
		strcpy(pdinfo->buf, char_buf[current_buf]);
		pr_info("!!! wake dbuf is  %p, pdinfo is %p\n", dbuf, dbuf->priv);
		test_wake_up(dbuf);
		dma_buf_put(dbuf);
		g_tinfo.dbuf = NULL;
		current_buf = (current_buf + 1) & 0x3;
	} else {
		pr_err(".....dbuf is null\n");
	}
}
#endif


static int
misc_demo_open(struct inode * inode, struct file * filp)
{
#ifdef FAKE_TIMER // 使用定时任务模拟实际场景,5s后触发
	timer_setup(&g_tinfo.my_timer, test_timer, 0);
#endif

	return 0;
}
static int
misc_demo_release(struct inode * inode, struct file * filp)
{
	// 用户close 自己的dma-buf描述符时 ，已经释放了dma-buf,且在 dma_buf_ops->release释放了自己的buffer
	pr_info("Start close exporter\n");
	misc_clearall_fence();
#ifdef FAKE_TIMER
	del_timer_sync(&g_tinfo.my_timer);
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
	INIT_LIST_HEAD(&g_tinfo.head);
	return misc_register(&misc_demo_drv);
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
