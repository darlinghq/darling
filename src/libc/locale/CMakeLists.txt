project(libc-locale)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")

set(locale_sources
	frune.c
	isctype.c
	iswctype.c
	lconv.c
	mbrune.c
	runedepreciated.c
	setinvalidrune.c
	xlocale.c
)

set(locale_sources ${locale_sources}
	FreeBSD/ascii.c
	FreeBSD/big5.c
	FreeBSD/btowc.c
	FreeBSD/collate.c
	FreeBSD/collcmp.c
	FreeBSD/euc.c
	FreeBSD/fix_grouping.c
	FreeBSD/gb18030.c
	FreeBSD/gb2312.c
	FreeBSD/gbk.c
	FreeBSD/ldpart.c
	FreeBSD/lmessages.c
	FreeBSD/lmonetary.c
	FreeBSD/lnumeric.c
	FreeBSD/localeconv.c
	FreeBSD/mblen.c
	FreeBSD/mbrlen.c
	FreeBSD/mbrtowc.c
	FreeBSD/mbsinit.c
	FreeBSD/mbsnrtowcs.c
	FreeBSD/mbsrtowcs.c
	FreeBSD/mbstowcs.c
	FreeBSD/mbtowc.c
	FreeBSD/mskanji.c
	FreeBSD/nextwctype.c
	FreeBSD/nl_langinfo.c
	FreeBSD/nomacros.c
	FreeBSD/none.c
	FreeBSD/rune.c
	FreeBSD/runetype.c
	FreeBSD/setlocale.c
	FreeBSD/setrunelocale.c
	FreeBSD/table.c
	FreeBSD/tolower.c
	FreeBSD/toupper.c
	FreeBSD/utf2.c
	FreeBSD/utf8.c
	FreeBSD/wcrtomb.c
	FreeBSD/wcsftime.c
	FreeBSD/wcsnrtombs.c
	FreeBSD/wcsrtombs.c
	FreeBSD/wcstod.c
	FreeBSD/wcstof.c
	FreeBSD/wcstoimax.c
	FreeBSD/wcstol.c
	FreeBSD/wcstold.c
	FreeBSD/wcstoll.c
	FreeBSD/wcstombs.c
	FreeBSD/wcstoul.c
	FreeBSD/wcstoull.c
	FreeBSD/wcstoumax.c
	FreeBSD/wctob.c
	FreeBSD/wctomb.c
	FreeBSD/wctrans.c
	FreeBSD/wctype.c
	FreeBSD/wcwidth.c
)
SET_SOURCE_FILES_PROPERTIES(FreeBSD/wcsftime.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_WCSFTIME -DLIBC_ALIAS_WCSFTIME_L")

add_library(libc-locale OBJECT ${locale_sources})

# Legacy variant
set(locale-legacy_sources
	FreeBSD/wcsftime.c)
add_library(libc-locale_legacy OBJECT ${locale-legacy_sources})
SET_TARGET_PROPERTIES(libc-locale_legacy PROPERTIES COMPILE_FLAGS
	"-DBUILDING_VARIANT -DVARIANT_LEGACY -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")

if (NOT DARLING_NO_EXECUTABLES)
	install(DIRECTORY locale DESTINATION libexec/darling/usr/share)
endif (NOT DARLING_NO_EXECUTABLES)


