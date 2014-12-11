#!/usr/bin/env ruby
#
# Copyright (c) 2012 Mark Heily <mark@heily.com>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

begin
require 'makeconf'
rescue LoadError
  $LOAD_PATH << "makeconf"
  require 'makeconf'
end

# Determine the list of compiler flags
def get_cflags
  cflags='-I./src/common -I./include -Wall -Wextra -Wno-missing-field-initializers -Werror -g -O2 -std=c99 -D_XOPEN_SOURCE=600'.split(/ /) 

  if Platform.is_linux?
    # TODO - note this as a GCC 4.X dependency
    cflags.push ' -fvisibility=hidden'
  end
  if Platform.is_solaris?
    cflags.push "-D__EXTENSIONS__"
  end
  cflags
end

# Determine the list of source code files for libkqueue
def get_source_list(project)
  src = %w{
       src/common/filter.c 
       src/common/knote.c 
       src/common/map.c 
       src/common/kevent.c
       src/common/kqueue.c
  }

  if Platform.is_solaris?
    src.push 'src/solaris/signal.c',
             'src/solaris/timer.c',
             'src/solaris/platform.c',
             'src/solaris/user.c'
  end

  if Platform.is_linux? or Platform.is_solaris?
    src.push 'src/posix/platform.c'
  end

  if Platform.is_linux?
    src.push 'src/linux/platform.c',
             'src/linux/read.c',
             'src/linux/write.c',
             'src/linux/user.c',
             'src/linux/vnode.c'

    # FIXME: needed for RHEL5
    #src.push 'src/posix/user.c'

    if project.check_header('sys/signalfd.h')
        src.push 'src/linux/signal.c'
    else
        src.push 'src/posix/signal.c'
    end

    src.push 'src/linux/timer.c'
  end

  if Platform.is_windows?
    src.push 'src/windows/timer.c',
             'src/windows/platform.c',
             'src/windows/read.c',
             'src/windows/user.c'
  end

  src
end

# Generate the linker flags
def get_ldadd
  ldadd = ''

  if Platform.is_linux? or Platform.is_solaris?
    ldadd += ' -lpthread'
  end

  if Platform.is_linux?
    ldadd += ' -lrt'
  end

  if Platform.is_windows?
    ldadd += ' ws2_32.lib'
  end

  ldadd
end

#
# MAIN()
#

project = Project.new \
 :id => 'libkqueue', 
 :version => '2.0.1'

kq = Library.new( 
 :id => 'libkqueue', 
 :cflags => get_cflags(),
 :ldadd => get_ldadd(),
 :sources => get_source_list(project)
    )
                

project.check_header 'sys/event.h'
if Platform.is_linux?                
  project.check_decl 'EPOLLRDHUP', :include => 'sys/epoll.h'
  project.check_decl 'ppoll', :cflags => '#define _GNU_SOURCE', :include => 'poll.h'


  project.check_header('sys/epoll.h') or throw 'epoll is required'
  project.check_header('sys/inotify.h') or throw 'inotify is required'
  project.check_header %w{ sys/signalfd.h sys/timerfd.h sys/eventfd.h }
end

project.add(kq)

project.add(Header.new(
            :id => 'event.h',
            :sources => 'include/sys/event.h',
            :namespace => 'kqueue/sys'
            ))

project.add(Manual.new('kqueue.2', :alias => 'kevent.2'))

test_ldadd = get_ldadd()
test_ldadd += ' libkqueue.a'
if Platform.is_windows?
  project.add(
   Test.new(
      :id => 'kqtest', 
      :cflags => '-g -O0 -Wall -Werror -Iinclude -Itest',
      :sources => %w{
          test/main.c 
          test/kevent.c
          test/test.c
          test/read.c
          test/timer.c
          test/vnode.c
          test/user.c
          },            # NOTE: signal.c and proc.c are removed
      :ldadd => test_ldadd.split(' ')
      )
    )
else
  project.add(
   Test.new(
      :id => 'kqtest', 
      :cflags => '-g -O0 -Wall -Werror -I./include -I./test',
      :sources => %w{
          test/main.c 
          test/kevent.c
          test/test.c
          test/proc.c
          test/read.c
          test/signal.c
          test/timer.c
          test/vnode.c
          test/user.c
          },
      :ldadd => test_ldadd.split(' ')
      )
    )
end


project.add(PkgConfig.new(
        :name => 'libkqueue',
        :description => 'Emulates FreeBSD kqueue(2) on other platforms',
        :requires => '',
        :libs => '-lkqueue',
        :libs_private => '-lpthread',
        :export_cflags => '-I${includedir}/kqueue'
        ))

project.packager.add 'libkqueue.spec'

mc = Makeconf.new()
mc.configure(project)

__END__
#
# BEGIN: old config.inc contents
#
program="libkqueue"
version="2.0a"
abi_major="0"
abi_minor="0"
abi_version="$abi_major.$abi_minor"
cflags="-Wall -Wextra -Wno-missing-field-initializers -Werror -g -O2 -std=c99 -D_XOPEN_SOURCE=600"
ldflags=""
sources="src/common/filter.c src/common/knote.c src/common/map.c
         src/common/kevent.c src/common/kqueue.c" 
libdepends=""
deps="src/common/private.h src/common/debug.h"
mans="kqueue.2"
headers="src/common/private.h"
extra_dist="*.in"
subdirs="src include test"

# Package metadata
pkg_summary="Emulates the kqueue and kevent system calls"
pkg_description="Emulates the kqueue and kevent system calls"
license="BSD"
author="Mark Heily"

pre_configure_hook() {
  if [ "$debug" = "yes" ] ; then
      cflags="$cflags -g3 -O0 -rdynamic"
  fi

  if [ "$target" != "windows" ] ; then
      cflags="$cflags -fpic"
  fi

  optional_headers="err.h"

  libdepends=" -L$libdir"
  if [ $target = "linux" ] ; then

      check_symbol sys/epoll.h EPOLLRDHUP

      # TODO - note this as a GCC 4.X dependency
      cflags="$cflags -fvisibility=hidden"

      libdepends="$libdepends -lpthread -lrt"
      required_headers="sys/epoll.h sys/inotify.h"
      optional_headers="sys/signalfd.h sys/timerfd.h sys/eventfd.h"
  fi

  if [ $target = "solaris" ] ; then
      cflags="$cflags -m64"
      ldflags="$ldflags -m64"
      libdepends="$libdepends -lsocket -lnsl"
  fi
}

post_configure_hook() {
    finalize target "$target"

    platform="src/posix/platform.c"
    evfilt_signal="src/posix/signal.c"
    evfilt_proc="src/$target/proc.c"
    evfilt_socket="src/$target/socket.c"
    evfilt_timer="src/posix/timer.c"
    evfilt_user="src/posix/user.c"
    evfilt_vnode="src/$target/vnode.c"

    if [ $target = "linux" ] ; then
      evfilt_user="src/linux/user.c"
      evfilt_socket="src/linux/read.c src/linux/write.c"

       #XXX-FIXME disabled
       evfilt_proc=""

      if [ "$have_sys_signalfd_h" = "yes" ] ; then
          evfilt_signal="src/linux/signal.c"
      fi
      if [ "$have_sys_timerfd_h" = "yes" ] ; then
          evfilt_timer="src/linux/timer.c"
      fi
      platform="$platform src/linux/platform.c"
    fi      

    if [ $target = "solaris" ] ; then
          cflags="$cflags -D__EXTENSIONS__"
          platform="$platform src/solaris/platform.c"
          evfilt_timer="src/solaris/timer.c"
          evfilt_user="src/solaris/user.c"
          evfilt_signal="src/solaris/signal.c"
          evfilt_proc=""
          evfilt_vnode=""
    fi      

    # FIXME: This will compile but not actually work
    if [ $target = "freebsd" ] ; then
          evfilt_signal="src/posix/signal.c"
          evfilt_proc=""
          evfilt_socket=""
          evfilt_timer=""
          evfilt_vnode=""
    fi

    if [ $target = "windows" ] ; then
          platform="src/windows/platform.c"
	  cflags="$cflags -march=i686 -lws2_32"
	  ldflags="$ldflags -march=i686"
          ldadd="-lws2_32"
          evfilt_proc=""
          evfilt_signal=""
	  #evfilt_socket="src/windows/read.c src/linux/write.c"
	  evfilt_socket="src/windows/read.c"
          evfilt_timer="src/windows/timer.c"
          evfilt_user="src/windows/user.c"
          evfilt_vnode=""
    fi
    
    sources="$sources $platform  
            $evfilt_signal $evfilt_proc 
             $evfilt_socket $evfilt_timer $evfilt_user $evfilt_vnode"
}
