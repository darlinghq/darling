program="libpthread_workqueue"
version="0.8.2"
abi_major="0"
abi_minor="0"
abi_version="$abi_major.$abi_minor"
cflags="-Wall -Wextra -Werror -D_XOPEN_SOURCE=600 -D__EXTENSIONS__ -D_GNU_SOURCE -std=c99 -I./include -I./src"
ldflags=""
ldadd="-lpthread -lrt"
sources='src/api.c src/$(API)/manager.c src/$(API)/thread_info.c src/witem_cache.c src/$(API)/thread_rt.c'
libdepends=""
deps="src/*.h"
mans="pthread_workqueue.3"
headers="include/pthread_workqueue.h"
extra_dist="LICENSE"
subdirs=""

# Package metadata
pkg_summary="pthread_workqueue library"
pkg_description="pthread_workqueue library"
license="BSD"
author="Mark Heily"

pre_configure_hook() {
  if [ "$debug" = "yes" ] ; then
      cflags="$cflags -g3 -O0 -DPTHREAD_WORKQUEUE_DEBUG -rdynamic"
  else
      cflags="$cflags -g -O2"
  fi
  check_header err.h
}

post_configure_hook() {

    cflags="$cflags"
    case "$target" in
    windows)
        cflags="$cflags -mthreads"
        ldflags="$ldflags -mthreads"
    	;;
    solaris)
        # TODO: would like to have -fvisibility=hidden but not supported
        #       by SFWgcc
        #
        cflags="$cflags -m64 -fpic"
    	ldflags="$ldflags -m64 -fpic -lumem"
    	;;
    *)
        if [ "`uname -m`" = "x86_64" ] ; then
            arch_flags="-m64"
        else
            arch_flags=""
        fi
        cflags="$cflags $arch_flags -fpic -fvisibility=hidden -pthread"
    	ldflags="$ldflags $arch_flags -fpic -pthread"
	;;
    esac
}
