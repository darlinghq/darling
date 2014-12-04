#
# @OSF_COPYRIGHT@
#
#
# HISTORY
# 
# Revision 1.1.1.1  1998/09/22 21:05:45  wsanchez
# Import of Mac OS X kernel (~semeria)
#
# Revision 1.1.1.1  1998/03/07 02:26:17  wsanchez
# Import of OSF Mach kernel (~mburg)
#
# Revision 1.1.8.2  1994/09/23  02:46:03  ezf
# 	change marker to not FREE
# 	[1994/09/22  21:44:25  ezf]
#
# Revision 1.1.8.1  1994/06/13  19:58:36  dlb
# 	Merge MK6 and NMK17
# 	[1994/06/13  16:22:58  dlb]
# 
# Revision 1.1.6.1  1994/03/07  16:41:51  paire
# 	Added MIGKSARGS and MIGKSENV variables to MIGKSFLAGS definition.
# 	[94/02/28            paire]
# 
# Revision 1.1.2.2  1993/08/04  19:32:37  gm
# 	CR9605: Add SUBDIRS to mach_kernel build process.
# 	[1993/08/03  13:30:22  gm]
# 
# $EndLog$

VPATH		= ..:../..

MIGFLAGS	= -MD ${IDENT}
MIGKSFLAGS	= -DKERNEL_SERVER ${MIGKSARGS} ${MIGKSENV}

MACH_DEBUG_FILES = mach_debug_server.h mach_debug_server.c

OTHERS		= ${MACH_DEBUG_FILES}

INCFLAGS	= -I.. -I../..
MDINCFLAGS	= -I.. -I../..

DEPENDENCIES	=

.include <${RULES_MK}>

.ORDER: ${MACH_DEBUG_FILES}

${MACH_DEBUG_FILES}: mach_debug/mach_debug.defs
	${_MIG_} ${_MIGFLAGS_} ${MIGKSFLAGS}		\
		-header /dev/null			\
		-user /dev/null				\
		-sheader mach_debug_server.h		\
		-server mach_debug_server.c		\
		${mach_debug/mach_debug.defs:P}

.if exists(depend.mk)
.include "depend.mk"
.endif
