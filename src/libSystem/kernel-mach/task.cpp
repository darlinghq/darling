#include "config.h"
#include "task.h"
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <execinfo.h>
#include <fstream>
#include <sstream>

darwin_task_t mach_task_self_ = new darwin_task(::getpid());

kern_return_t mach_port_deallocate(darwin_task_t task, void* port)
{
	free(port);
	return KERN_SUCCESS;
}

kern_return_t pid_for_task(mach_port_name_t t, int *pid)
{
	// TODO: this is supposed to be for the specified task but for now, just return our pid
	if (pid)
	{
		*pid = mach_task_self_->pid;
	}
	return KERN_SUCCESS;
}

kern_return_t mach_port_mod_refs(ipc_space_t task, mach_port_name_t name, mach_port_right_t right, mach_port_delta_t delta)
{
	return KERN_SUCCESS;
}

// task_info structures

#define TASK_DYLD_INFO                  17

struct task_dyld_info {
	mach_vm_address_t       all_image_info_addr;
	mach_vm_size_t          all_image_info_size;
	integer_t               all_image_info_format;
};
typedef struct task_dyld_info   task_dyld_info_data_t;
typedef struct task_dyld_info   *task_dyld_info_t;
#define TASK_DYLD_ALL_IMAGE_INFO_32     0
#define TASK_DYLD_ALL_IMAGE_INFO_64     1

enum dyld_image_mode { dyld_image_adding=0, dyld_image_removing=1 };
typedef void (*dyld_image_notifier)(enum dyld_image_mode mode, uint32_t infoCount, const struct dyld_image_info info[]);

struct dyld_all_image_infos {
uint32_t                        version;
uint32_t                        infoArrayCount;
const struct dyld_image_info*   infoArray;
dyld_image_notifier             notification;
bool                            processDetachedFromSharedRegion;
bool                            libSystemInitialized;
const struct mach_header*       dyldImageLoadAddress;
void*                           jitInfo;
const char*                     dyldVersion;
const char*                     errorMessage;
uintptr_t                       terminationFlags;
void*                           coreSymbolicationShmPage;
uintptr_t                       systemOrderFlag;
uintptr_t                       uuidArrayCount;
const struct dyld_uuid_info*    uuidArray;
struct dyld_all_image_infos*    dyldAllImageInfosAddress;
uintptr_t                       initialImageCount;
uintptr_t                       errorKind;
const char*                     errorClientOfDylibPath;
const char*                     errorTargetDylibPath;
const char*                     errorSymbol;
uintptr_t                       sharedCacheSlide;
};

struct dyld_all_image_infos dyld_all_image_infos = { 12 };

kern_return_t task_info(task_name_t target_task, task_flavor_t flavor, task_info_t task_info_out, mach_msg_type_number_t *task_info_outCnt)
{
	std::cerr << "task_info(" << target_task << ", " << flavor << ", " << task_info_out << ", " << task_info_outCnt << " = " << *task_info_outCnt << ")" << std::endl;

	switch (flavor)
	{
		case TASK_DYLD_INFO:
		{
			task_dyld_info_t dyld_info = (task_dyld_info_t)task_info_out;
			dyld_info->all_image_info_addr = (mach_vm_address_t)&dyld_all_image_infos;
			dyld_info->all_image_info_size = sizeof(dyld_all_image_infos);

			if (sizeof(void*) == 8)
				dyld_info->all_image_info_format = TASK_DYLD_ALL_IMAGE_INFO_64;
			else
				dyld_info->all_image_info_format = TASK_DYLD_ALL_IMAGE_INFO_32;

			break;
		}
		default:
			// TODO: get the requested info flavor. For now, just zero the provided buffer
			for (int i=0; i<*task_info_outCnt; i++)
			{
				task_info_out[i] = 0;
			}
			break;
	}

	return KERN_SUCCESS;
}


