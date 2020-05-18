project(libc-gen)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -fblocks -DPRIVATE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")

set(gen_sources
	authentication.c
	backtrace.c
	clock_gettime.c
	confstr.c
	crypt.c
	devname.c
	dirfd.c
	disklabel.c
	errlst.c
	filesec.c
	fts.c
	get_compat.c
	getloadavg.c
	getttyent.c
	getusershell.c
	getvfsbyname.c
	nanosleep.c
	nftw.c
	nlist.c
	oldsyslog.c
	raise.c
	setlogin.c
	sigsetops.c
	strtofflags.c
	sync_volume_np.c
	thread_stack_pcs.c
	uname.c
	utmpx-darwin.c
)

set(gen_sources ${gen_sources}
	FreeBSD/alarm.c
	FreeBSD/arc4random.c
	FreeBSD/assert.c
	FreeBSD/basename.c
	FreeBSD/clock.c
	FreeBSD/closedir.c
	FreeBSD/ctermid.c
	FreeBSD/daemon.c
	FreeBSD/dirname.c
	FreeBSD/drand48.c
	FreeBSD/erand48.c
	FreeBSD/err.c
	FreeBSD/exec.c
	FreeBSD/fmtcheck.c
	FreeBSD/fmtmsg.c
	FreeBSD/fnmatch.c
	FreeBSD/ftok.c
	FreeBSD/getbsize.c
	FreeBSD/getcap.c
	FreeBSD/getcwd.c
	FreeBSD/gethostname.c
	FreeBSD/getlogin.c
	FreeBSD/getmntinfo.c
	FreeBSD/getmntinfo64.c
	FreeBSD/getpagesize.c
	FreeBSD/getpeereid.c
	FreeBSD/getprogname.c
	FreeBSD/glob.c
	FreeBSD/isatty.c
	FreeBSD/jrand48.c
	FreeBSD/lcong48.c
	FreeBSD/lockf.c
	FreeBSD/lrand48.c
	FreeBSD/mrand48.c
	FreeBSD/nice.c
	FreeBSD/nrand48.c
	FreeBSD/opendir.c
	FreeBSD/pause.c
	FreeBSD/popen.c
	FreeBSD/psignal.c
	FreeBSD/_rand48.c
	FreeBSD/readdir.c
	FreeBSD/readpassphrase.c
	FreeBSD/rewinddir.c
	FreeBSD/scandir_b.c
	FreeBSD/scandir.c
	FreeBSD/seed48.c
	FreeBSD/seekdir.c
	FreeBSD/sethostname.c
	FreeBSD/setmode.c
	FreeBSD/setprogname.c
	FreeBSD/siginterrupt.c
	FreeBSD/siglist.c
	FreeBSD/signal.c
	FreeBSD/sleep.c
	FreeBSD/srand48.c
	FreeBSD/stringlist.c
	FreeBSD/sysconf.c
	FreeBSD/sysctlbyname.c
	FreeBSD/sysctl.c
	FreeBSD/sysctlnametomib.c
	FreeBSD/telldir.c
	FreeBSD/termios.c
	FreeBSD/time.c
	FreeBSD/times.c
	FreeBSD/timespec_get.c
	#FreeBSD/timezone.c # legacy only
	FreeBSD/ttyname.c
	FreeBSD/ttyslot.c
	FreeBSD/ualarm.c
	FreeBSD/ulimit.c
	FreeBSD/unvis.c
	FreeBSD/usleep.c
	FreeBSD/utime.c
	FreeBSD/vis.c
	FreeBSD/wait.c
	FreeBSD/waitpid.c
	FreeBSD/wait3.c
	FreeBSD/wordexp.c
	NetBSD/rb.c
	NetBSD/utmpx.c
	)

SET_SOURCE_FILES_PROPERTIES(FreeBSD/clock.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_CLOCK")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/daemon.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_DAEMON")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/closedir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_CLOSEDIR -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/glob.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_GLOB")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/fnmatch.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_FNMATCH")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/nice.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_NICE")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/opendir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS___OPENDIR2 -DLIBC_ALIAS_OPENDIR -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/pause.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PAUSE -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/popen.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_POPEN -U_DARWIN_C_SOURCE -U_POSIX_C_SOURCE  -D__DARWIN_C_LEVEL=200000L")
#SET_SOURCE_FILES_PROPERTIES(FreeBSD/pselect.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PSELECT  -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h -U_DARWIN_C_SOURCE -U_POSIX_C_SOURCE  -D__DARWIN_C_LEVEL=200000L")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/readdir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/rewinddir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_REWINDDIR -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/seekdir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SEEKDIR -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/setmode.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SETMODE")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/sleep.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SLEEP  -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/telldir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS__SEEKDIR -DLIBC_ALIAS_TELLDIR -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/termios.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_TCDRAIN -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")
#SET_SOURCE_FILES_PROPERTIES(FreeBSD/timezone.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_TIMEZONE")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/ttyname.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_TTYNAME_R")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/usleep.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_USLEEP")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/wait.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_WAIT")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/waitpid.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_WAITPID")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/lockf.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_LOCKF  -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")
SET_SOURCE_FILES_PROPERTIES(nanosleep.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_NANOSLEEP")
SET_SOURCE_FILES_PROPERTIES(confstr.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_CONFSTR")
SET_SOURCE_FILES_PROPERTIES(crypt.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_ENCRYPT")
SET_SOURCE_FILES_PROPERTIES(fwt.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_NFTW -DLIBC_ALIAS_FTW")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/termios.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_TCDRAIN")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/wait.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_WAIT")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/waitpid.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_WAITPID")

set(gen_sources ${gen_sources}
	NetBSD/utmpx.c
)

add_darling_object_library(libc-gen ${gen_sources})

# ***
# Cancelable variants
# ***
set(gen-cancelable_sources FreeBSD/lockf.c nanosleep.c FreeBSD/pause.c FreeBSD/sleep.c FreeBSD/termios.c FreeBSD/usleep.c FreeBSD/wait.c FreeBSD/waitpid.c)
add_library(libc-gen_cancelable OBJECT ${gen-cancelable_sources})
#SET_TARGET_PROPERTIES(libc-gen_cancelable PROPERTIES COMPILE_DEFINITIONS "BUILDING_VARIANT;VARIANT_CANCELABLE")
set_target_properties(libc-gen_cancelable PROPERTIES COMPILE_FLAGS "-D__DARWIN_NON_CANCELABLE=0 -DBUILDING_VARIANT -DVARIANT_CANCELABLE")


# ***
# No-INODE64 variants
# ***
set(gen-noinode64_sources FreeBSD/seekdir.c FreeBSD/readdir.c FreeBSD/scandir.c FreeBSD/rewinddir.c FreeBSD/telldir.c fts.c FreeBSD/getmntinfo.c FreeBSD/glob.c FreeBSD/opendir.c nftw.c)
add_library(libc-gen_noinode64 OBJECT ${gen-noinode64_sources})
SET_TARGET_PROPERTIES(libc-gen_noinode64 PROPERTIES COMPILE_FLAGS "-U__DARWIN_UNIX03 -U__DARWIN_64_BIT_INO_T -DVARIANT_INODE32 -DBUILDING_VARIANT -D_DARWIN_NO_64_BIT_INODE")


# ***
# Legacy variants
# ***

if (TARGET_i386)
	set(gen-legacy_sources FreeBSD/clock.c FreeBSD/closedir.c FreeBSD/fnmatch.c
		FreeBSD/lockf.c FreeBSD/pause.c FreeBSD/popen.c FreeBSD/rewinddir.c
		FreeBSD/seekdir.c FreeBSD/setmode.c FreeBSD/sleep.c FreeBSD/telldir.c
		FreeBSD/ttyname.c FreeBSD/usleep.c FreeBSD/wait.c FreeBSD/waitpid.c
		FreeBSD/timezone.c FreeBSD/termios.c FreeBSD/nice.c
		confstr.c crypt.c nftw.c FreeBSD/opendir.c nanosleep.c)

	add_library(libc-gen_legacy OBJECT ${gen-legacy_sources})
	SET_TARGET_PROPERTIES(libc-gen_legacy PROPERTIES COMPILE_FLAGS
		"-DBUILDING_VARIANT -DVARIANT_LEGACY -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0 -U__DARWIN_64_BIT_INO_T -D_DARWIN_NO_64_BIT_INODE")
endif (TARGET_i386)

# ***
# DARWIN_EXTSN variants
#
set(gen-extsn_sources FreeBSD/popen.c)
add_library(libc-gen_extsn OBJECT ${gen-extsn_sources})
SET_TARGET_PROPERTIES(libc-gen_extsn PROPERTIES COMPILE_DEFINITIONS "_DARWIN_UNLIMITED_STREAMS;_DARWIN_UNLIMITED_SELECT;BUILDING_VARIANT")

# ***
# DARWIN_EXTSN cancelable variants
#
#set(gen-extsn_canc_sources FreeBSD/pselect.c)
#add_library(libc-gen_extsn_canc OBJECT ${gen-extsn_canc_sources})
#SET_TARGET_PROPERTIES(libc-gen_extsn_canc PROPERTIES COMPILE_DEFINITIONS "_DARWIN_UNLIMITED_STREAMS;_DARWIN_UNLIMITED_SELECT;BUILDING_VARIANT;VARIANT_CANCELABLE")

# ***
# PRE1050 variants
# ***

set(gen-pre1050_sources FreeBSD/daemon.c)

add_library(libc-gen_pre1050 OBJECT ${gen-pre1050_sources})
SET_TARGET_PROPERTIES(libc-gen_pre1050 PROPERTIES COMPILE_FLAGS "-DBUILDING_VARIANT -DVARIANT_PRE1050 -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")

# ***
# DYLD variants
# ***

add_library(libc-gen_dyld OBJECT FreeBSD/arc4random.c FreeBSD/closedir.c dirfd.c FreeBSD/getcwd.c FreeBSD/getpagesize.c nanosleep.c FreeBSD/opendir.c
	FreeBSD/readdir.c FreeBSD/scandir.c FreeBSD/sysctl.c FreeBSD/sysctlbyname.c FreeBSD/telldir.c FreeBSD/usleep.c FreeBSD/sysconf.c
	FreeBSD/isatty.c FreeBSD/termios.c FreeBSD/getprogname.c FreeBSD/vis.c FreeBSD/err.c FreeBSD/usleep.c errlst.c)

SET_TARGET_PROPERTIES(libc-gen_dyld PROPERTIES COMPILE_FLAGS "-UBUILDING_VARIANT -DVARIANT_DYLD -DVARIANT_DARWINEXTSN -D__DARWIN_NON_CANCELABLE=1 -D__DARWIN_UNIX03=1")

