#ifndef	_clock_reply_user_
#define	_clock_reply_user_

/* Module clock_reply */

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

#ifndef	clock_reply_MSG_COUNT
#define	clock_reply_MSG_COUNT	1
#endif	/* clock_reply_MSG_COUNT */

#include <mach/std_types.h>
#include <mach/mig.h>
#include <mach/mach_types.h>

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


/* SimpleRoutine clock_alarm_reply */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t clock_alarm_reply
(
	clock_reply_t alarm_port,
	mach_msg_type_name_t alarm_portPoly,
	kern_return_t alarm_code,
	alarm_type_t alarm_type,
	mach_timespec_t alarm_time
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

#ifndef __Request__clock_reply_subsystem__defined
#define __Request__clock_reply_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t alarm_code;
		alarm_type_t alarm_type;
		mach_timespec_t alarm_time;
	} __Request__clock_alarm_reply_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__clock_reply_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion__clock_reply_subsystem__defined
#define __RequestUnion__clock_reply_subsystem__defined
union __RequestUnion__clock_reply_subsystem {
	__Request__clock_alarm_reply_t Request_clock_alarm_reply;
};
#endif /* !__RequestUnion__clock_reply_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__clock_reply_subsystem__defined
#define __Reply__clock_reply_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__clock_alarm_reply_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__clock_reply_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion__clock_reply_subsystem__defined
#define __ReplyUnion__clock_reply_subsystem__defined
union __ReplyUnion__clock_reply_subsystem {
	__Reply__clock_alarm_reply_t Reply_clock_alarm_reply;
};
#endif /* !__RequestUnion__clock_reply_subsystem__defined */

#ifndef subsystem_to_name_map_clock_reply
#define subsystem_to_name_map_clock_reply \
    { "clock_alarm_reply", 3125107 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _clock_reply_user_ */
