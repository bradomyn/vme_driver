#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x9a26c89a, "struct_module" },
	{ 0x3ce4ca6f, "disable_irq" },
	{ 0x72ebbb5a, "per_cpu__current_task" },
	{ 0xdfb57b95, "kmalloc_caches" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x6ad778e5, "up_read" },
	{ 0x2757bece, "pci_release_region" },
	{ 0xcff14fd3, "mem_map" },
	{ 0xa5423cc4, "param_get_int" },
	{ 0x6402aaff, "release_resource" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xa2364c52, "boot_cpu_data" },
	{ 0xa55032a3, "pci_disable_device" },
	{ 0x65167116, "remove_proc_entry" },
	{ 0xabc08a89, "device_destroy" },
	{ 0xa03d6a57, "__get_user_4" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x6ce66043, "mutex_unlock" },
	{ 0xcb32da10, "param_set_int" },
	{ 0x303d8860, "pci_bus_alloc_resource" },
	{ 0xc69cf653, "dma_pool_destroy" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x1139ffc, "max_mapnr" },
	{ 0x3da171f9, "pci_mem_start" },
	{ 0xbce67cc4, "down_read" },
	{ 0xda4008e6, "cond_resched" },
	{ 0x9a6847c2, "pci_set_master" },
	{ 0xbce3753, "ioread32be" },
	{ 0x1c44f5f8, "proc_mkdir" },
	{ 0xb12b81ac, "pci_set_dma_mask" },
	{ 0x389e200f, "ioread8" },
	{ 0xeeb20c3d, "pci_iounmap" },
	{ 0x442199dd, "mutex_lock_interruptible" },
	{ 0x2940bb15, "__mutex_init" },
	{ 0x1b7d4074, "printk" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0x78d0ffc4, "mutex_lock" },
	{ 0x625acc81, "__down_failed_interruptible" },
	{ 0x3a3187cf, "class_create" },
	{ 0x7003fcc3, "device_create" },
	{ 0x1f9b7a16, "dma_pool_free" },
	{ 0x9eac042a, "__ioremap" },
	{ 0xfe113a50, "kmem_cache_alloc" },
	{ 0xed633abc, "pv_irq_ops" },
	{ 0xa1a6414c, "iowrite32be" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0xa3f33c9, "pci_bus_read_config_dword" },
	{ 0x91ce3516, "get_user_pages" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0x2cf190e3, "request_irq" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0x4292364c, "schedule" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0x8892b5a8, "register_chrdev" },
	{ 0x3354829d, "create_proc_entry" },
	{ 0x3acdd36c, "dev_driver_string" },
	{ 0xf1099d29, "dma_pool_alloc" },
	{ 0x3c3e52d4, "pci_unregister_driver" },
	{ 0x6cb34e5, "init_waitqueue_head" },
	{ 0xc281c899, "__wake_up" },
	{ 0x37a0cba, "kfree" },
	{ 0x6783bf6a, "remap_pfn_range" },
	{ 0x7d1ed1f2, "prepare_to_wait" },
	{ 0xedc03953, "iounmap" },
	{ 0x9ef749e2, "unregister_chrdev" },
	{ 0xd18f7afe, "__pci_register_driver" },
	{ 0x1aea612c, "put_page" },
	{ 0x860feeea, "class_destroy" },
	{ 0x2a7b11d4, "finish_wait" },
	{ 0x60a4461c, "__up_wakeup" },
	{ 0xab3ebcf4, "pci_iomap" },
	{ 0x83704fdc, "pci_find_parent_resource" },
	{ 0x96b27088, "__down_failed" },
	{ 0xa7651652, "pci_enable_device" },
	{ 0x91cc2336, "dma_pool_create" },
	{ 0xd6c963c, "copy_from_user" },
	{ 0x26739595, "pci_request_region" },
	{ 0x2241a928, "ioread32" },
	{ 0xf20dabd8, "free_irq" },
	{ 0xe914e41e, "strcpy" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010E3d00000148sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "A04ED46129378E9862A44B3");
