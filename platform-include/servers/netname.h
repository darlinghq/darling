#ifndef	_netname_user_
#define	_netname_user_

/* Module netname */

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

#ifndef	netname_MSG_COUNT
#define	netname_MSG_COUNT	4
#endif	/* netname_MSG_COUNT */

#include <mach/std_types.h>
#include <mach/mig.h>
#include <servers/netname_defs.h>

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


/* Routine netname_check_in */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t netname_check_in
(
	mach_port_t server_port,
	netname_name_t port_name,
	mach_port_t signature,
	mach_port_t port_id
);

/* Routine netname_look_up */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t netname_look_up
(
	mach_port_t server_port,
	netname_name_t host_name,
	netname_name_t port_name,
	mach_port_t *port_id
);

/* Routine netname_check_out */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t netname_check_out
(
	mach_port_t server_port,
	netname_name_t port_name,
	mach_port_t signature
);

/* Routine netname_version */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t netname_version
(
	mach_port_t server_port,
	netname_name_t version
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

#ifndef __Request__netname_subsystem__defined
#define __Request__netname_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t signature;
		mach_msg_port_descriptor_t port_id;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t port_nameOffset; /* MiG doesn't use it */
		mach_msg_type_number_t port_nameCnt;
		char port_name[80];
	} __Request__netname_check_in_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		mach_msg_type_number_t host_nameOffset; /* MiG doesn't use it */
		mach_msg_type_number_t host_nameCnt;
		char host_name[80];
		mach_msg_type_number_t port_nameOffset; /* MiG doesn't use it */
		mach_msg_type_number_t port_nameCnt;
		char port_name[80];
	} __Request__netname_look_up_t;
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
		mach_msg_port_descriptor_t signature;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t port_nameOffset; /* MiG doesn't use it */
		mach_msg_type_number_t port_nameCnt;
		char port_name[80];
	} __Request__netname_check_out_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__netname_version_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__netname_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion__netname_subsystem__defined
#define __RequestUnion__netname_subsystem__defined
union __RequestUnion__netname_subsystem {
	__Request__netname_check_in_t Request_netname_check_in;
	__Request__netname_look_up_t Request_netname_look_up;
	__Request__netname_check_out_t Request_netname_check_out;
	__Request__netname_version_t Request_netname_version;
};
#endif /* !__RequestUnion__netname_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__netname_subsystem__defined
#define __Reply__netname_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__netname_check_in_t;
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
		mach_msg_port_descriptor_t port_id;
		/* end of the kernel processed data */
	} __Reply__netname_look_up_t;
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
	} __Reply__netname_check_out_t;
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
		mach_msg_type_number_t versionOffset; /* MiG doesn't use it */
		mach_msg_type_number_t versionCnt;
		char version[80];
	} __Reply__netname_version_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__netname_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion__netname_subsystem__defined
#define __ReplyUnion__netname_subsystem__defined
union __ReplyUnion__netname_subsystem {
	__Reply__netname_check_in_t Reply_netname_check_in;
	__Reply__netname_look_up_t Reply_netname_look_up;
	__Reply__netname_check_out_t Reply_netname_check_out;
	__Reply__netname_version_t Reply_netname_version;
};
#endif /* !__RequestUnion__netname_subsystem__defined */

#ifndef subsystem_to_name_map_netname
#define subsystem_to_name_map_netname \
    { "netname_check_in", 1040 },\
    { "netname_look_up", 1041 },\
    { "netname_check_out", 1042 },\
    { "netname_version", 1043 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _netname_user_ */
