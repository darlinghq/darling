project(libc-stdio)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_C99_SOURCE -D_FORTIFY_SOURCE=0 -D__va_list=__builtin_va_list")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../gdtoa/FreeBSD")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../gdtoa")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/FreeBSD")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(stdio_sources
	xprintf_all_in_one.c
	xprintf_comp.c
	xprintf_domain.c
	xprintf_exec.c
	FreeBSD/asprintf.c
	FreeBSD/clrerr.c
	FreeBSD/dprintf.c
	FreeBSD/fclose.c
	FreeBSD/fdopen.c
	FreeBSD/feof.c
	FreeBSD/ferror.c
	FreeBSD/fflush.c
	FreeBSD/fgetc.c
	FreeBSD/fgetln.c
	FreeBSD/fgetpos.c
	FreeBSD/fgets.c
	FreeBSD/fgetwc.c
	FreeBSD/fgetwln.c
	FreeBSD/fgetws.c
	FreeBSD/fileno.c
	FreeBSD/findfp.c
	FreeBSD/flags.c
	FreeBSD/_flock_stub.c
	FreeBSD/fopen.c
	FreeBSD/fprintf.c
	FreeBSD/fpurge.c
	FreeBSD/fputc.c
	FreeBSD/fputs.c
	FreeBSD/fputwc.c
	FreeBSD/fputws.c
	FreeBSD/fread.c
	FreeBSD/freopen.c
	FreeBSD/fscanf.c
	FreeBSD/fseek.c
	FreeBSD/fsetpos.c
	FreeBSD/ftell.c
	FreeBSD/funopen.c
	FreeBSD/fvwrite.c
	FreeBSD/fwalk.c
	FreeBSD/fwide.c
	FreeBSD/fwprintf.c
	FreeBSD/fwrite.c
	FreeBSD/fwscanf.c
	FreeBSD/getc.c
	FreeBSD/getdelim.c
	FreeBSD/getchar.c
	FreeBSD/getline.c
	FreeBSD/gets.c
	FreeBSD/getw.c
	FreeBSD/getwc.c
	FreeBSD/getwchar.c
	FreeBSD/makebuf.c
	FreeBSD/mktemp.c
	FreeBSD/perror.c
	FreeBSD/printf.c
	FreeBSD/printf-pos.c
	FreeBSD/putc.c
	FreeBSD/putchar.c
	FreeBSD/puts.c
	FreeBSD/putw.c
	FreeBSD/putwc.c
	FreeBSD/putwchar.c
	FreeBSD/refill.c
	FreeBSD/remove.c
	FreeBSD/rewind.c
	FreeBSD/rget.c
	FreeBSD/scanf.c
	FreeBSD/setbuf.c
	FreeBSD/setbuffer.c
	FreeBSD/setvbuf.c
	FreeBSD/snprintf.c
	FreeBSD/sprintf.c
	FreeBSD/sscanf.c
	FreeBSD/stdio.c
	FreeBSD/swprintf.c
	FreeBSD/swscanf.c
	FreeBSD/tempnam.c
	FreeBSD/tmpfile.c
	FreeBSD/tmpnam.c
	FreeBSD/ungetc.c
	FreeBSD/ungetwc.c
	FreeBSD/vasprintf.c
	FreeBSD/vdprintf.c
	FreeBSD/vfprintf.c
	FreeBSD/vfscanf.c
	FreeBSD/vfwprintf.c
	FreeBSD/vfwscanf.c
	FreeBSD/vprintf.c
	FreeBSD/vscanf.c
	FreeBSD/vsnprintf.c
	FreeBSD/vsprintf.c
	FreeBSD/vsscanf.c
	FreeBSD/vswprintf.c
	FreeBSD/vswscanf.c
	FreeBSD/vwprintf.c
	FreeBSD/vwscanf.c
	FreeBSD/wbuf.c
	FreeBSD/wprintf.c
	FreeBSD/wscanf.c
	FreeBSD/wsetup.c
	FreeBSD/xprintf.c
	FreeBSD/xprintf_errno.c
	FreeBSD/xprintf_float.c
	FreeBSD/xprintf_hexdump.c
	FreeBSD/xprintf_int.c
	FreeBSD/xprintf_quote.c
	FreeBSD/xprintf_str.c
	FreeBSD/xprintf_time.c
	FreeBSD/xprintf_vis.c
)

set_source_files_properties(FreeBSD/fopen.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_FOPEN")
set_source_files_properties(FreeBSD/fdopen.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_FDOPEN")
set_source_files_properties(FreeBSD/fputs.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_FPUTS")
set_source_files_properties(FreeBSD/freopen.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_FREOPEN")
set_source_files_properties(FreeBSD/fwrite.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_FWRITE")
set_source_files_properties(FreeBSD/tempnam.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_TEMPNAM")

add_darling_object_library(libc-stdio ${stdio_sources})
#SET_TARGET_PROPERTIES(libc-stdio PROPERTIES COMPILE_DEFINITIONS "VARIANT_DARWINEXTSN")

# ***
# Legacy variants
# ***

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_DARWIN_C_SOURCE -U_POSIX_C_SOURCE")

if (TARGET_i386)
	set(stdio-legacy_sources
		 FreeBSD/fopen.c FreeBSD/fdopen.c
		FreeBSD/fputs.c FreeBSD/tempnam.c
		FreeBSD/freopen.c FreeBSD/fwrite.c)

	add_library(libc-stdio_legacy OBJECT ${stdio-legacy_sources})
	SET_TARGET_PROPERTIES(libc-stdio_legacy PROPERTIES COMPILE_DEFINITIONS "BUILDING_VARIANT;VARIANT_LEGACY")
endif (TARGET_i386)

# ***
# DARWIN_EXTSN variants
# ***

set(stdio-extsn_sources FreeBSD/fopen.c FreeBSD/fdopen.c)
add_library(libc-stdio_extsn OBJECT ${stdio-extsn_sources})
SET_TARGET_PROPERTIES(libc-stdio_extsn PROPERTIES COMPILE_DEFINITIONS "BUILDING_VARIANT;VARIANT_DARWINEXTSN")


# ***
# DYLD variants
# ***
#add_library(libc-stdio_dyld OBJECT FreeBSD/sprintf.c FreeBSD/printf.c)
#SET_TARGET_PROPERTIES(libc-stdio_dyld PROPERTIES COMPILE_FLAGS "-UBUILDING_VARIANT -DVARIANT_DYLD -DVARIANT_CANCELABLE -DVARIANT_DARWINEXTSN")
