#ifndef IPC_TYPES_H
#define IPC_TYPES_H

struct ipc_space;
typedef struct ipc_space ipc_namespace_t;

struct darling_mach_port;
typedef struct darling_mach_port darling_mach_port_t;

struct mach_task;
typedef struct mach_task mach_task_t;

struct mach_port_right;
typedef struct mach_port_right darling_mach_port_right_t;

#endif
