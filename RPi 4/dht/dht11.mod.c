#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x92997ed8, "_printk" },
	{ 0xfe990052, "gpio_free" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xb4f9efb7, "__register_chrdev" },
	{ 0x8d30dba0, "__class_create" },
	{ 0xbf3f457f, "device_create" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x882ce6fa, "gpio_to_desc" },
	{ 0x183a612f, "gpiod_direction_output_raw" },
	{ 0xc62a4969, "gpiod_direction_input" },
	{ 0x907b1f36, "gpiod_get_raw_value" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x4b0a3f52, "gic_nonsecure_priorities" },
	{ 0xd697e69a, "trace_hardirqs_on" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0xec3d2e1b, "trace_hardirqs_off" },
	{ 0x8f80e6e5, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "71809C7C01EEBF3E0E53F3C");
