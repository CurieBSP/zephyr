CONF_basic_minuteia = ia32/prj.conf
CONF_quark_d2000_crb= quark_d2000/quark_d2000_prj.conf
CONF_quark_se_ctb   = quark_se/quark_se_prj.conf
CONF_qemu_x86       = ia32/prj_expert_test.conf
CONF_galileo        = ia32_pci/prj.conf
CONF_minnowboard     = ia32/prj_expert_test.conf
CONF_arduino_101    = quark_se/quark_se_prj.conf

KERNEL_TYPE = nano
BOARD ?= qemu_x86
CONF_FILE = $(CONF_$(BOARD))
SOURCE_DIR = $(ZEPHYR_BASE)/samples/microkernel/benchmark/boot_time/src/

include $(ZEPHYR_BASE)/Makefile.inc
