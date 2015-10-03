ccflags-y := -I$(src)/../kernel-include \
		-I$(src)/../platform-include \
		-I$(src)/../libc/include \
		-DMACH_KERNEL_PRIVATE \
		-DDARLING_DEBUG \
		-D__LITTLE_ENDIAN__ \
		-Wno-undef

CFLAGS_taskServer.o := -Dsemaphore_consume_ref_t=semaphore_t

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	obj-m := darling-mach.o
	darling-mach-objs := traps.o ipc_space.o \
	    ipc_port.o ipc_right.o ipc_server.o debug.o \
	    darling_task.o ipc_msg.o \
	    servers/mach_host.o servers/clock.o \
	    servers/task.o \
	    mig/clockServer.o mig/mach_hostServer.o \
	    mig/taskServer.o \
	    mig/duct.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
