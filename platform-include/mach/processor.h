#ifndef	_processor_user_
#define	_processor_user_

/* Module processor */

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

#ifndef	processor_MSG_COUNT
#define	processor_MSG_COUNT	6
#endif	/* processor_MSG_COUNT */

#include <mach/std_types.h>
#include <mach/mig.h>
#include <mach/mig.h>
#include <mach/mach_types.h>

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


/* Routine processor_start */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_start
(
	processor_t processor
);

/* Routine processor_exit */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_exit
(
	processor_t processor
);

/* Routine processor_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_info
(
	processor_t processor,
	processor_flavor_t flavor,
	host_t *host,
	processor_info_t processor_info_out,
	mach_msg_type_number_t *processor_info_outCnt
);

/* Routine processor_control */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_control
(
	processor_t processor,
	processor_info_t processor_cmd,
	mach_msg_type_number_t processor_cmdCnt
);

/* Routine processor_assign */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_assign
(
	processor_t processor,
	processor_set_t new_set,
	boolean_t wait
);

/* Routine processor_get_assignment */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_get_assignment
(
	processor_t processor,
	processor_set_name_t *assigned_set
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

#ifndef __Request__processor_subsystem__defined
#define __Request__processor_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__processor_start_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__processor_exit_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		processor_flavor_t flavor;
		mach_msg_type_number_t processor_info_outCnt;
	} __Request__processor_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		mach_msg_type_number_t processor_cmdCnt;
		integer_t processor_cmd[12];
	} __Request__processor_control_t;
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
		boolean_t wait;
	} __Request__processor_assign_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__processor_get_assignment_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__processor_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion__processor_subsystem__defined
#define __RequestUnion__processor_subsystem__defined
union __RequestUnion__processor_subsystem {
	__Request__processor_start_t Request_processor_start;
	__Request__processor_exit_t Request_processor_exit;
	__Request__processor_info_t Request_processor_info;
	__Request__processor_control_t Request_processor_control;
	__Request__processor_assign_t Request_processor_assign;
	__Request__processor_get_assignment_t Request_processor_get_assignment;
};
#endif /* !__RequestUnion__processor_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__processor_subsystem__defined
#define __Reply__processor_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__processor_start_t;
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
	} __Reply__processor_exit_t;
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
		mach_msg_port_descriptor_t host;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t processor_info_outCnt;
		integer_t processor_info_out[12];
	} __Reply__processor_info_t;
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
	} __Reply__processor_control_t;
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
	} __Reply__processor_assign_t;
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
	} __Reply__processor_get_assignment_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__processor_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion__processor_subsystem__defined
#define __ReplyUnion__processor_subsystem__defined
union __ReplyUnion__processor_subsystem {
	__Reply__processor_start_t Reply_processor_start;
	__Reply__processor_exit_t Reply_processor_exit;
	__Reply__processor_info_t Reply_processor_info;
	__Reply__processor_control_t Reply_processor_control;
	__Reply__processor_assign_t Reply_processor_assign;
	__Reply__processor_get_assignment_t Reply_processor_get_assignment;
};
#endif /* !__RequestUnion__processor_subsystem__defined */

#ifndef subsystem_to_name_map_processor
#define subsystem_to_name_map_processor \
    { "processor_start", 3000 },\
    { "processor_exit", 3001 },\
    { "processor_info", 3002 },\
    { "processor_control", 3003 },\
    { "processor_assign", 3004 },\
    { "processor_get_assignment", 3005 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _processor_user_ */
