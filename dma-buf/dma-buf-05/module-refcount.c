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
#include <linux/list.h>


static char *modname = "dmabuf_exporter";
module_param (modname, charp, S_IRUGO);


static inline void print_unload_info(struct module *mod)
{
	struct module_use *use;
	int printed_something = 0;

	pr_info("ref: %i ", module_refcount(mod));

	/*
	 * Always include a trailing , so userspace can differentiate
	 * between this and the old multi-field proc format.
	 */
	list_for_each_entry(use, &mod->source_list, source_list) {
		printed_something = 1;
		pr_info("%s,", use->source->name);
	}

	if (mod->init != NULL && mod->exit == NULL) {
		printed_something = 1;
		pr_info("[permanent],");
	}

	if (!printed_something)
		pr_info("-");
}

static int __init dmabuf_exporter_init(void)
	
{
	struct module *mod = find_module(modname);
	pr_info("dump module %s infomation\n", modname);

	if(!mod){
		pr_err("Can't find module  %s", modname);
		return -ENOENT;
	}
	pr_info( "%s %u",mod->name, mod->init_layout.size + mod->core_layout.size);
	print_unload_info(mod);
	pr_err("We will sub %s refcount to 0", modname);
	if(module_refcount(mod) > 0) {
		atomic_set(&mod->refcnt, 1);
	}
	pr_err("Module %s refcount is  %d", modname, module_refcount(mod));

	return -ENOMEM;
}
module_init(dmabuf_exporter_init);

static void __exit dmabuf_exporter_exit(void)
{

}
module_exit(dmabuf_exporter_exit);


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("TEST DMA-BUF");
MODULE_VERSION("1.0.0.0");


