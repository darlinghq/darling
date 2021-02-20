#ifndef __SANDBOX_ROOTLESS_H__
#define __SANDBOX_ROOTLESS_H__

#ifdef __cplusplus
extern "C" {
#endif

int rootless_allows_task_for_pid(pid_t pid);

int rootless_restricted_environment();
int rootless_check_trusted(const char* path);
int rootless_check_datavault_flag(const char* path, const char* storage_class);

#ifdef __cplusplus
};
#endif

#endif
