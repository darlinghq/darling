project(libc-gen)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fblocks -DKERNEL_PRIVATE -DPRIVATE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")

# ASL and utmpx not built

set(gen_sources #asl.c
	#asl_core.c
	#asl_fd.c
	#asl_file.c
	#asl_legacy1.c
	#asl_msg.c
	#asl_store.c
	#asl_util.c
	#assumes.c
	authentication.c
	backtrace.c
	cache.c
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
	isinf.c
	isnan.c
	#magazine_malloc.c # magazine or scalable
	malloc.c
	nanosleep.c
	nftw.c
	nlist.c
	NSSystemDirectories.c
	oldsyslog.c
	platfunc.c
	scalable_malloc.c
	setlogin.c 
	sigsetops.c
	_simple.c
	stack_logging.c
	stack_logging_disk.c 
	strtofflags.c
	sync_volume_np.c 
	#syslog.c 
	thread_stack_pcs.c 
	uname.c
	#utmpx-darwin.c 
	wordexp.c)

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
	FreeBSD/getmntinfo64.c                                                                                                                                                                                                                                                                 
	FreeBSD/getmntinfo.c                                                                                                                                                                                                                                                                   
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
	FreeBSD/pselect.c
	FreeBSD/psignal.c
	FreeBSD/raise.c
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
	# FreeBSD/timezone.c # incompatible with 'long timezone'
	FreeBSD/ttyname.c
	FreeBSD/ttyslot.c
	FreeBSD/ualarm.c
	FreeBSD/ulimit.c
	FreeBSD/unvis.c
	FreeBSD/usleep.c
	FreeBSD/utime.c
	FreeBSD/vis.c
	FreeBSD/wait3.c
	FreeBSD/wait.c
	FreeBSD/waitpid.c)

SET_SOURCE_FILES_PROPERTIES(FreeBSD/clock.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_CLOCK")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/closedir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_CLOSEDIR -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/glob.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_GLOB")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/nice.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_NICE")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/opendir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS___OPENDIR2 -DLIBC_ALIAS_OPENDIR -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/pause.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PAUSE -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/popen.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_POPEN")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/pselect.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PSELECT  -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/readdir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/rewinddir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_REWINDDIR -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/seekdir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SEEKDIR -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/sleep.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SLEEP  -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/telldir.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -include __dirent.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/termios.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_TCDRAIN -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/timezone.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/ttyname.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_TTYNAME_R")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/usleep.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_USLEEP")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/wait.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_WAIT")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/waitpid.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_WAITPID")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/lockf.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_LOCKF")
SET_SOURCE_FILES_PROPERTIES(nanosleep.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_NANOSLEEP")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/termios.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_TCDRAIN")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/wait.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_WAIT")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/waitpid.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_WAITPID")

#set(gen_sources ${gen_sources}
#	NetBSD/utmpx.c)

add_library(libc-gen OBJECT ${gen_sources})

# ***
# Cancelable variants
# ***
set(gen-cancelable_sources FreeBSD/lockf.c nanosleep.c FreeBSD/pause.c FreeBSD/pselect.c FreeBSD/sleep.c FreeBSD/termios.c FreeBSD/usleep.c FreeBSD/wait.c FreeBSD/waitpid.c)
add_library(libc-gen_cancelable OBJECT ${gen-cancelable_sources})
SET_TARGET_PROPERTIES(libc-gen_cancelable PROPERTIES COMPILE_DEFINITIONS "BUILDING_VARIANT;VARIANT_CANCELABLE")

