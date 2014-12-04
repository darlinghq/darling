#ifndef	_mach_voucher_user_
#define	_mach_voucher_user_

/* Module mach_voucher */

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

#ifndef	mach_voucher_MSG_COUNT
#define	mach_voucher_MSG_COUNT	5
#endif	/* mach_voucher_MSG_COUNT */

#include <mach/std_types.h>
#include <mach/mig.h>
#include <mach/mig.h>
#include <mach/mach_types.h>

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


/* Routine mach_voucher_extract_attr_content */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_voucher_extract_attr_content
(
	ipc_voucher_t voucher,
	mach_voucher_attr_key_t key,
	mach_voucher_attr_content_t content,
	mach_msg_type_number_t *contentCnt
);

/* Routine mach_voucher_extract_attr_recipe */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_voucher_extract_attr_recipe
(
	ipc_voucher_t voucher,
	mach_voucher_attr_key_t key,
	mach_voucher_attr_raw_recipe_t recipe,
	mach_msg_type_number_t *recipeCnt
);

/* Routine mach_voucher_extract_all_attr_recipes */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_voucher_extract_all_attr_recipes
(
	ipc_voucher_t voucher,
	mach_voucher_attr_raw_recipe_array_t recipes,
	mach_msg_type_number_t *recipesCnt
);

/* Routine mach_voucher_attr_command */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_voucher_attr_command
(
	ipc_voucher_t voucher,
	mach_voucher_attr_key_t key,
	mach_voucher_attr_command_t command,
	mach_voucher_attr_content_t in_content,
	mach_msg_type_number_t in_contentCnt,
	mach_voucher_attr_content_t out_content,
	mach_msg_type_number_t *out_contentCnt
);

/* Routine mach_voucher_debug_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_voucher_debug_info
(
	ipc_space_t task,
	mach_port_name_t voucher_name,
	mach_voucher_attr_raw_recipe_array_t recipes,
	mach_msg_type_number_t *recipesCnt
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

#ifndef __Request__mach_voucher_subsystem__defined
#define __Request__mach_voucher_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		mach_voucher_attr_key_t key;
		mach_msg_type_number_t contentCnt;
	} __Request__mach_voucher_extract_attr_content_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		mach_voucher_attr_key_t key;
		mach_msg_type_number_t recipeCnt;
	} __Request__mach_voucher_extract_attr_recipe_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		mach_msg_type_number_t recipesCnt;
	} __Request__mach_voucher_extract_all_attr_recipes_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		mach_voucher_attr_key_t key;
		mach_voucher_attr_command_t command;
		mach_msg_type_number_t in_contentCnt;
		uint8_t in_content[4096];
		mach_msg_type_number_t out_contentCnt;
	} __Request__mach_voucher_attr_command_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		mach_port_name_t voucher_name;
		mach_msg_type_number_t recipesCnt;
	} __Request__mach_voucher_debug_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__mach_voucher_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion__mach_voucher_subsystem__defined
#define __RequestUnion__mach_voucher_subsystem__defined
union __RequestUnion__mach_voucher_subsystem {
	__Request__mach_voucher_extract_attr_content_t Request_mach_voucher_extract_attr_content;
	__Request__mach_voucher_extract_attr_recipe_t Request_mach_voucher_extract_attr_recipe;
	__Request__mach_voucher_extract_all_attr_recipes_t Request_mach_voucher_extract_all_attr_recipes;
	__Request__mach_voucher_attr_command_t Request_mach_voucher_attr_command;
	__Request__mach_voucher_debug_info_t Request_mach_voucher_debug_info;
};
#endif /* !__RequestUnion__mach_voucher_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__mach_voucher_subsystem__defined
#define __Reply__mach_voucher_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t contentCnt;
		uint8_t content[4096];
	} __Reply__mach_voucher_extract_attr_content_t;
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
		mach_msg_type_number_t recipeCnt;
		uint8_t recipe[4096];
	} __Reply__mach_voucher_extract_attr_recipe_t;
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
		mach_msg_type_number_t recipesCnt;
		uint8_t recipes[5120];
	} __Reply__mach_voucher_extract_all_attr_recipes_t;
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
		mach_msg_type_number_t out_contentCnt;
		uint8_t out_content[4096];
	} __Reply__mach_voucher_attr_command_t;
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
		mach_msg_type_number_t recipesCnt;
		uint8_t recipes[5120];
	} __Reply__mach_voucher_debug_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__mach_voucher_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion__mach_voucher_subsystem__defined
#define __ReplyUnion__mach_voucher_subsystem__defined
union __ReplyUnion__mach_voucher_subsystem {
	__Reply__mach_voucher_extract_attr_content_t Reply_mach_voucher_extract_attr_content;
	__Reply__mach_voucher_extract_attr_recipe_t Reply_mach_voucher_extract_attr_recipe;
	__Reply__mach_voucher_extract_all_attr_recipes_t Reply_mach_voucher_extract_all_attr_recipes;
	__Reply__mach_voucher_attr_command_t Reply_mach_voucher_attr_command;
	__Reply__mach_voucher_debug_info_t Reply_mach_voucher_debug_info;
};
#endif /* !__RequestUnion__mach_voucher_subsystem__defined */

#ifndef subsystem_to_name_map_mach_voucher
#define subsystem_to_name_map_mach_voucher \
    { "mach_voucher_extract_attr_content", 5400 },\
    { "mach_voucher_extract_attr_recipe", 5401 },\
    { "mach_voucher_extract_all_attr_recipes", 5402 },\
    { "mach_voucher_attr_command", 5403 },\
    { "mach_voucher_debug_info", 5404 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _mach_voucher_user_ */
