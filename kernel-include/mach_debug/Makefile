export MakeInc_cmd=${SRCROOT}/makedefs/MakeInc.cmd
export MakeInc_def=${SRCROOT}/makedefs/MakeInc.def
export MakeInc_rule=${SRCROOT}/makedefs/MakeInc.rule
export MakeInc_dir=${SRCROOT}/makedefs/MakeInc.dir

include $(MakeInc_cmd)
include $(MakeInc_def)

MIG_DEFS = mach_debug_types.defs

DATAFILES = \
	mach_debug.h \
	hash_info.h ipc_info.h vm_info.h zone_info.h \
	page_info.h mach_debug_types.h lockgroup_info.h \
	${MIG_DEFS}

INSTALL_MI_LIST	= ${DATAFILES}

INSTALL_MI_DIR = mach_debug

EXPORT_MI_LIST	= ${DATAFILES}

EXPORT_MI_DIR = mach_debug

include $(MakeInc_rule)
include $(MakeInc_dir)


