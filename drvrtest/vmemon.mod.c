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
	{ 0xa5423cc4, "param_get_int" },
	{ 0xcb32da10, "param_set_int" },
	{ 0xeeb1717c, "param_array_get" },
	{ 0xab471003, "param_array_set" },
	{ 0xd1694422, "find_controller" },
	{ 0x97a23c39, "vme_register_berr_handler" },
	{ 0xbccb7cc9, "return_controller" },
	{ 0x106863e, "vme_unregister_berr_handler" },
	{ 0x1b7d4074, "printk" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=vmebus";


MODULE_INFO(srcversion, "E0FF6323C8AA9728BB5CC64");
