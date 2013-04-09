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
	{ 0xd72f079c, "vme_request_irq" },
	{ 0xae07c6e9, "vme_find_mapping" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x404e46ad, "vme_release_mapping" },
	{ 0x8ecccb22, "vme_free_irq" },
	{ 0xa1a6414c, "iowrite32be" },
	{ 0xd5d87388, "vme_bus_error_check" },
	{ 0x1b7d4074, "printk" },
	{ 0xbce3753, "ioread32be" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=vmebus";


MODULE_INFO(srcversion, "612C139687122D443194515");
