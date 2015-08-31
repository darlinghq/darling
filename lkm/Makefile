ccflags-y := -I$(src)/../kernel-include \
		-I$(src)/../platform-include \
		-I$(src)/../libc/include \
		-DMACH_KERNEL_PRIVATE \
		-Wno-undef

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	obj-m := darling-mach.o
	darling-mach-objs := linuxmach.o ipc_space.o ipc_port.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
