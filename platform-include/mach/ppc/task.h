#ifndef	_task_user_
#define	_task_user_

/* Module task */

#include <string.h>
#include <mach/ndr.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/notify.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#include <mach/port.h>

#ifdef AUTOTEST
#ifndef FUNCTION_PTR_T
#define FUNCTION_PTR_T
typedef void (*function_ptr_t)(mach_port_t, char *, mach_msg_type_number_t);
typedef struct {
        char            *name;
        function_ptr_t  function;
} function_table_entry;
typedef function_table_entry   *function_table_t;
#endif /* FUNCTION_PTR_T */
#endif /* AUTOTEST */

#ifndef	task_MSG_COUNT
#define	task_MSG_COUNT	35
#endif	/* task_MSG_COUNT */

#include <mach/std_types.h>
#include <mach/mig.h>
#include <mach/mig.h>
#include <mach/mach_types.h>

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


/* Routine task_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_create
(
	task_t target_task,
	ledger_array_t ledgers,
	mach_msg_type_number_t ledgersCnt,
	boolean_t inherit_memory,
	task_t *child_task
);

/* Routine task_terminate */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_terminate
(
	task_t target_task
);

/* Routine task_threads */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_threads
(
	task_t target_task,
	thread_act_array_t *act_list,
	mach_msg_type_number_t *act_listCnt
);

/* Routine mach_ports_register */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_ports_register
(
	task_t target_task,
	mach_port_array_t init_port_set,
	mach_msg_type_number_t init_port_setCnt
);

/* Routine mach_ports_lookup */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_ports_lookup
(
	task_t target_task,
	mach_port_array_t *init_port_set,
	mach_msg_type_number_t *init_port_setCnt
);

/* Routine task_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_info
(
	task_name_t target_task,
	task_flavor_t flavor,
	task_info_t task_info_out,
	mach_msg_type_number_t *task_info_outCnt
);

/* Routine task_set_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_info
(
	task_t target_task,
	task_flavor_t flavor,
	task_info_t task_info_in,
	mach_msg_type_number_t task_info_inCnt
);

/* Routine task_suspend */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_suspend
(
	task_t target_task
);

/* Routine task_resume */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_resume
(
	task_t target_task
);

/* Routine task_get_special_port */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_special_port
(
	task_t task,
	int which_port,
	mach_port_t *special_port
);

/* Routine task_set_special_port */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_special_port
(
	task_t task,
	int which_port,
	mach_port_t special_port
);

/* Routine thread_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t thread_create
(
	task_t parent_task,
	thread_act_t *child_act
);

/* Routine thread_create_running */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t thread_create_running
(
	task_t parent_task,
	thread_state_flavor_t flavor,
	thread_state_t new_state,
	mach_msg_type_number_t new_stateCnt,
	thread_act_t *child_act
);

/* Routine task_set_exception_ports */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_exception_ports
(
	task_t task,
	exception_mask_t exception_mask,
	mach_port_t new_port,
	exception_behavior_t behavior,
	thread_state_flavor_t new_flavor
);

/* Routine task_get_exception_ports */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_exception_ports
(
	task_t task,
	exception_mask_t exception_mask,
	exception_mask_array_t masks,
	mach_msg_type_number_t *masksCnt,
	exception_handler_array_t old_handlers,
	exception_behavior_array_t old_behaviors,
	exception_flavor_array_t old_flavors
);

/* Routine task_swap_exception_ports */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_swap_exception_ports
(
	task_t task,
	exception_mask_t exception_mask,
	mach_port_t new_port,
	exception_behavior_t behavior,
	thread_state_flavor_t new_flavor,
	exception_mask_array_t masks,
	mach_msg_type_number_t *masksCnt,
	exception_handler_array_t old_handlerss,
	exception_behavior_array_t old_behaviors,
	exception_flavor_array_t old_flavors
);

/* Routine lock_set_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t lock_set_create
(
	task_t task,
	lock_set_t *new_lock_set,
	int n_ulocks,
	int policy
);

/* Routine lock_set_destroy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t lock_set_destroy
(
	task_t task,
	lock_set_t lock_set
);

/* Routine semaphore_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t semaphore_create
(
	task_t task,
	semaphore_t *semaphore,
	int policy,
	int value
);

/* Routine semaphore_destroy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t semaphore_destroy
(
	task_t task,
	semaphore_t semaphore
);

/* Routine task_policy_set */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_policy_set
(
	task_t task,
	task_policy_flavor_t flavor,
	task_policy_t policy_info,
	mach_msg_type_number_t policy_infoCnt
);

/* Routine task_policy_get */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_policy_get
(
	task_t task,
	task_policy_flavor_t flavor,
	task_policy_t policy_info,
	mach_msg_type_number_t *policy_infoCnt,
	boolean_t *get_default
);

/* Routine task_sample */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_sample
(
	task_t task,
	mach_port_t reply
);

/* Routine task_policy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_policy
(
	task_t task,
	policy_t policy,
	policy_base_t base,
	mach_msg_type_number_t baseCnt,
	boolean_t set_limit,
	boolean_t change
);

/* Routine task_set_emulation */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_emulation
(
	task_t target_port,
	vm_address_t routine_entry_pt,
	int routine_number
);

/* Routine task_get_emulation_vector */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_emulation_vector
(
	task_t task,
	int *vector_start,
	emulation_vector_t *emulation_vector,
	mach_msg_type_number_t *emulation_vectorCnt
);

/* Routine task_set_emulation_vector */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_emulation_vector
(
	task_t task,
	int vector_start,
	emulation_vector_t emulation_vector,
	mach_msg_type_number_t emulation_vectorCnt
);

/* Routine task_set_ras_pc */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_ras_pc
(
	task_t target_task,
	vm_address_t basepc,
	vm_address_t boundspc
);

/* Routine task_assign */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_assign
(
	task_t task,
	processor_set_t new_set,
	boolean_t assign_threads
);

/* Routine task_assign_default */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_assign_default
(
	task_t task,
	boolean_t assign_threads
);

/* Routine task_get_assignment */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_assignment
(
	task_t task,
	processor_set_name_t *assigned_set
);

/* Routine task_set_policy */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_policy
(
	task_t task,
	processor_set_t pset,
	policy_t policy,
	policy_base_t base,
	mach_msg_type_number_t baseCnt,
	policy_limit_t limit,
	mach_msg_type_number_t limitCnt,
	boolean_t change
);

/* Routine task_get_state */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_get_state
(
	task_t task,
	thread_state_flavor_t flavor,
	thread_state_t old_state,
	mach_msg_type_number_t *old_stateCnt
);

/* Routine task_set_state */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_set_state
(
	task_t task,
	thread_state_flavor_t flavor,
	thread_state_t new_state,
	mach_msg_type_number_t new_stateCnt
);

__END_DECLS

/********************** Caution **************************/
/* The following data types should be used to calculate  */
/* maximum message sizes only. The actual message may be */
/* smaller, and the position of the arguments within the */
/* message layout may vary from what is presented here.  */
/* For example, if any of the arguments are variable-    */
/* sized, and less than the maximum is sent, the data    */
/* will be packed tight in the actual message to reduce  */
/* the presence of holes.                                */
/********************** Caution **************************/

/* typedefs for all requests */

#ifndef __Request__task_subsystem__defined
#define __Request__task_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_ports_descriptor_t ledgers;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t ledgersCnt;
		boolean_t inherit_memory;
	} __Request__task_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__task_terminate_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__task_threads_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_ports_descriptor_t init_port_set;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t init_port_setCnt;
	} __Request__mach_ports_register_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__mach_ports_lookup_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		task_flavor_t flavor;
		mach_msg_type_number_t task_info_outCnt;
	} __Request__task_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		task_flavor_t flavor;
		mach_msg_type_number_t task_info_inCnt;
		integer_t task_info_in[10];
	} __Request__task_set_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__task_suspend_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__task_resume_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		int which_port;
	} __Request__task_get_special_port_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t special_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int which_port;
	} __Request__task_set_special_port_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__thread_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		thread_state_flavor_t flavor;
		mach_msg_type_number_t new_stateCnt;
		natural_t new_state[144];
	} __Request__thread_create_running_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		exception_mask_t exception_mask;
		exception_behavior_t behavior;
		thread_state_flavor_t new_flavor;
	} __Request__task_set_exception_ports_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		exception_mask_t exception_mask;
	} __Request__task_get_exception_ports_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		exception_mask_t exception_mask;
		exception_behavior_t behavior;
		thread_state_flavor_t new_flavor;
	} __Request__task_swap_exception_ports_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		int n_ulocks;
		int policy;
	} __Request__lock_set_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t lock_set;
		/* end of the kernel processed data */
	} __Request__lock_set_destroy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		int policy;
		int value;
	} __Request__semaphore_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t semaphore;
		/* end of the kernel processed data */
	} __Request__semaphore_destroy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		task_policy_flavor_t flavor;
		mach_msg_type_number_t policy_infoCnt;
		integer_t policy_info[16];
	} __Request__task_policy_set_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		task_policy_flavor_t flavor;
		mach_msg_type_number_t policy_infoCnt;
		boolean_t get_default;
	} __Request__task_policy_get_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t reply;
		/* end of the kernel processed data */
	} __Request__task_sample_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		policy_t policy;
		mach_msg_type_number_t baseCnt;
		integer_t base[5];
		boolean_t set_limit;
		boolean_t change;
	} __Request__task_policy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		vm_address_t routine_entry_pt;
		int routine_number;
	} __Request__task_set_emulation_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__task_get_emulation_vector_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t emulation_vector;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int vector_start;
		mach_msg_type_number_t emulation_vectorCnt;
	} __Request__task_set_emulation_vector_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		vm_address_t basepc;
		vm_address_t boundspc;
	} __Request__task_set_ras_pc_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_set;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t assign_threads;
	} __Request__task_assign_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		boolean_t assign_threads;
	} __Request__task_assign_default_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__task_get_assignment_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t pset;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		policy_t policy;
		mach_msg_type_number_t baseCnt;
		integer_t base[5];
		mach_msg_type_number_t limitCnt;
		integer_t limit[1];
		boolean_t change;
	} __Request__task_set_policy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		thread_state_flavor_t flavor;
		mach_msg_type_number_t old_stateCnt;
	} __Request__task_get_state_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		thread_state_flavor_t flavor;
		mach_msg_type_number_t new_stateCnt;
		natural_t new_state[144];
	} __Request__task_set_state_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__task_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion__task_subsystem__defined
#define __RequestUnion__task_subsystem__defined
union __RequestUnion__task_subsystem {
	__Request__task_create_t Request_task_create;
	__Request__task_terminate_t Request_task_terminate;
	__Request__task_threads_t Request_task_threads;
	__Request__mach_ports_register_t Request_mach_ports_register;
	__Request__mach_ports_lookup_t Request_mach_ports_lookup;
	__Request__task_info_t Request_task_info;
	__Request__task_set_info_t Request_task_set_info;
	__Request__task_suspend_t Request_task_suspend;
	__Request__task_resume_t Request_task_resume;
	__Request__task_get_special_port_t Request_task_get_special_port;
	__Request__task_set_special_port_t Request_task_set_special_port;
	__Request__thread_create_t Request_thread_create;
	__Request__thread_create_running_t Request_thread_create_running;
	__Request__task_set_exception_ports_t Request_task_set_exception_ports;
	__Request__task_get_exception_ports_t Request_task_get_exception_ports;
	__Request__task_swap_exception_ports_t Request_task_swap_exception_ports;
	__Request__lock_set_create_t Request_lock_set_create;
	__Request__lock_set_destroy_t Request_lock_set_destroy;
	__Request__semaphore_create_t Request_semaphore_create;
	__Request__semaphore_destroy_t Request_semaphore_destroy;
	__Request__task_policy_set_t Request_task_policy_set;
	__Request__task_policy_get_t Request_task_policy_get;
	__Request__task_sample_t Request_task_sample;
	__Request__task_policy_t Request_task_policy;
	__Request__task_set_emulation_t Request_task_set_emulation;
	__Request__task_get_emulation_vector_t Request_task_get_emulation_vector;
	__Request__task_set_emulation_vector_t Request_task_set_emulation_vector;
	__Request__task_set_ras_pc_t Request_task_set_ras_pc;
	__Request__task_assign_t Request_task_assign;
	__Request__task_assign_default_t Request_task_assign_default;
	__Request__task_get_assignment_t Request_task_get_assignment;
	__Request__task_set_policy_t Request_task_set_policy;
	__Request__task_get_state_t Request_task_get_state;
	__Request__task_set_state_t Request_task_set_state;
};
#endif /* !__RequestUnion__task_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__task_subsystem__defined
#define __Reply__task_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t child_task;
		/* end of the kernel processed data */
	} __Reply__task_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_terminate_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_ports_descriptor_t act_list;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t act_listCnt;
	} __Reply__task_threads_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__mach_ports_register_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_ports_descriptor_t init_port_set;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t init_port_setCnt;
	} __Reply__mach_ports_lookup_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t task_info_outCnt;
		integer_t task_info_out[10];
	} __Reply__task_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_set_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_suspend_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_resume_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t special_port;
		/* end of the kernel processed data */
	} __Reply__task_get_special_port_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_set_special_port_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t child_act;
		/* end of the kernel processed data */
	} __Reply__thread_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t child_act;
		/* end of the kernel processed data */
	} __Reply__thread_create_running_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_set_exception_ports_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t old_handlers[32];
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t masksCnt;
		exception_mask_t masks[32];
		exception_behavior_t old_behaviors[32];
		thread_state_flavor_t old_flavors[32];
	} __Reply__task_get_exception_ports_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t old_handlerss[32];
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t masksCnt;
		exception_mask_t masks[32];
		exception_behavior_t old_behaviors[32];
		thread_state_flavor_t old_flavors[32];
	} __Reply__task_swap_exception_ports_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_lock_set;
		/* end of the kernel processed data */
	} __Reply__lock_set_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__lock_set_destroy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t semaphore;
		/* end of the kernel processed data */
	} __Reply__semaphore_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__semaphore_destroy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_policy_set_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t policy_infoCnt;
		integer_t policy_info[16];
		boolean_t get_default;
	} __Reply__task_policy_get_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_sample_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_policy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_set_emulation_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t emulation_vector;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int vector_start;
		mach_msg_type_number_t emulation_vectorCnt;
	} __Reply__task_get_emulation_vector_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_set_emulation_vector_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_set_ras_pc_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_assign_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_assign_default_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t assigned_set;
		/* end of the kernel processed data */
	} __Reply__task_get_assignment_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_set_policy_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t old_stateCnt;
		natural_t old_state[144];
	} __Reply__task_get_state_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_set_state_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__task_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion__task_subsystem__defined
#define __ReplyUnion__task_subsystem__defined
union __ReplyUnion__task_subsystem {
	__Reply__task_create_t Reply_task_create;
	__Reply__task_terminate_t Reply_task_terminate;
	__Reply__task_threads_t Reply_task_threads;
	__Reply__mach_ports_register_t Reply_mach_ports_register;
	__Reply__mach_ports_lookup_t Reply_mach_ports_lookup;
	__Reply__task_info_t Reply_task_info;
	__Reply__task_set_info_t Reply_task_set_info;
	__Reply__task_suspend_t Reply_task_suspend;
	__Reply__task_resume_t Reply_task_resume;
	__Reply__task_get_special_port_t Reply_task_get_special_port;
	__Reply__task_set_special_port_t Reply_task_set_special_port;
	__Reply__thread_create_t Reply_thread_create;
	__Reply__thread_create_running_t Reply_thread_create_running;
	__Reply__task_set_exception_ports_t Reply_task_set_exception_ports;
	__Reply__task_get_exception_ports_t Reply_task_get_exception_ports;
	__Reply__task_swap_exception_ports_t Reply_task_swap_exception_ports;
	__Reply__lock_set_create_t Reply_lock_set_create;
	__Reply__lock_set_destroy_t Reply_lock_set_destroy;
	__Reply__semaphore_create_t Reply_semaphore_create;
	__Reply__semaphore_destroy_t Reply_semaphore_destroy;
	__Reply__task_policy_set_t Reply_task_policy_set;
	__Reply__task_policy_get_t Reply_task_policy_get;
	__Reply__task_sample_t Reply_task_sample;
	__Reply__task_policy_t Reply_task_policy;
	__Reply__task_set_emulation_t Reply_task_set_emulation;
	__Reply__task_get_emulation_vector_t Reply_task_get_emulation_vector;
	__Reply__task_set_emulation_vector_t Reply_task_set_emulation_vector;
	__Reply__task_set_ras_pc_t Reply_task_set_ras_pc;
	__Reply__task_assign_t Reply_task_assign;
	__Reply__task_assign_default_t Reply_task_assign_default;
	__Reply__task_get_assignment_t Reply_task_get_assignment;
	__Reply__task_set_policy_t Reply_task_set_policy;
	__Reply__task_get_state_t Reply_task_get_state;
	__Reply__task_set_state_t Reply_task_set_state;
};
#endif /* !__RequestUnion__task_subsystem__defined */

#ifndef subsystem_to_name_map_task
#define subsystem_to_name_map_task \
    { "task_create", 3400 },\
    { "task_terminate", 3401 },\
    { "task_threads", 3402 },\
    { "mach_ports_register", 3403 },\
    { "mach_ports_lookup", 3404 },\
    { "task_info", 3405 },\
    { "task_set_info", 3406 },\
    { "task_suspend", 3407 },\
    { "task_resume", 3408 },\
    { "task_get_special_port", 3409 },\
    { "task_set_special_port", 3410 },\
    { "thread_create", 3411 },\
    { "thread_create_running", 3412 },\
    { "task_set_exception_ports", 3413 },\
    { "task_get_exception_ports", 3414 },\
    { "task_swap_exception_ports", 3415 },\
    { "lock_set_create", 3416 },\
    { "lock_set_destroy", 3417 },\
    { "semaphore_create", 3418 },\
    { "semaphore_destroy", 3419 },\
    { "task_policy_set", 3420 },\
    { "task_policy_get", 3421 },\
    { "task_sample", 3422 },\
    { "task_policy", 3423 },\
    { "task_set_emulation", 3424 },\
    { "task_get_emulation_vector", 3425 },\
    { "task_set_emulation_vector", 3426 },\
    { "task_set_ras_pc", 3427 },\
    { "task_assign", 3429 },\
    { "task_assign_default", 3430 },\
    { "task_get_assignment", 3431 },\
    { "task_set_policy", 3432 },\
    { "task_get_state", 3433 },\
    { "task_set_state", 3434 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _task_user_ */
