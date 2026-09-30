require 'tmpdir'
require 'open3'
root = ARGV.fetch(0, File.expand_path('..', __dir__))
source = File.read("#{root}/src/startup/mldr/elfcalls/elfcalls.c")
method = source[/^static int native_fork\(void\)\n\{.*?^\}/m] or abort 'native_fork missing'
Dir.mktmpdir('native-fork-errors-') do |dir|
  program = <<~C
    #include <errno.h>
    #include <assert.h>
    #include <stdio.h>
    static int result, error, threads, sockets;
    static int fixture_fork(void) { errno=error; return result; }
    static void __mldr_thread_postfork_child(void) { ++threads; errno=EIO; }
    static void __mldr_socket_bitmap_postfork_child(void) { ++sockets; errno=EBADF; }
    #define fork fixture_fork
    #{method}
    #undef fork
    int main(void) {
      int errors[]={EAGAIN,ENOMEM,EPERM};
      for (unsigned i=0;i<sizeof(errors)/sizeof(errors[0]);++i) {
        result=-1; error=errors[i];
        assert(native_fork()==-errors[i]);
        assert(threads==0 && sockets==0);
      }
      result=42; error=ENOMEM;
      assert(native_fork()==42 && threads==0 && sockets==0);
      result=0;
      assert(native_fork()==0 && threads==1 && sockets==1);
      puts("PASS: negative errno, parent PID and child reset contract");
    }
  C
  File.write("#{dir}/probe.c",program)
  out,status=Open3.capture2e('clang','-O1','-fsanitize=address,undefined',"#{dir}/probe.c",'-o',"#{dir}/probe")
  abort out unless status.success?
  out,status=Open3.capture2e("#{dir}/probe",rlimit_core:0)
  puts out
  abort 'native fork contract failed' unless status.success?
  # The filter is installed only in a disposable child and cannot affect the
  # calling Ruby process. Deny all libc fork implementation alternatives.
  actual = <<~C
    #define _GNU_SOURCE
    #include <errno.h>
    #include <assert.h>
    #include <stdio.h>
    #include <unistd.h>
    #include <stddef.h>
    #include <sys/wait.h>
    #include <sys/prctl.h>
    #include <sys/syscall.h>
    #include <linux/seccomp.h>
    #include <linux/filter.h>
    static int threads, sockets;
    static void __mldr_thread_postfork_child(void) { ++threads; }
    static void __mldr_socket_bitmap_postfork_child(void) { ++sockets; }
    #{method}
    int main(void) {
      int pid=native_fork(), status;
      assert(pid>=0);
      if(pid==0) _exit(threads==1 && sockets==1 ? 0 : 1);
      assert(waitpid(pid,&status,0)==pid && WIFEXITED(status) && WEXITSTATUS(status)==0);
      assert(threads==0 && sockets==0);
      pid=fork(); assert(pid>=0);
      if(pid==0) {
        struct sock_filter filters[]={
          BPF_STMT(BPF_LD|BPF_W|BPF_ABS,offsetof(struct seccomp_data,nr)),
    #ifdef __NR_fork
          BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,__NR_fork,0,1),
          BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EAGAIN),
    #endif
    #ifdef __NR_clone
          BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,__NR_clone,0,1),
          BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EAGAIN),
    #endif
    #ifdef __NR_clone3
          BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,__NR_clone3,0,1),
          BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ERRNO|EAGAIN),
    #endif
          BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ALLOW)
        };
        struct sock_fprog filter={sizeof(filters)/sizeof(filters[0]),filters};
        assert(prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0)==0);
        assert(prctl(PR_SET_SECCOMP,SECCOMP_MODE_FILTER,&filter)==0);
        assert(native_fork()==-EAGAIN && threads==0 && sockets==0);
        _exit(0);
      }
      assert(waitpid(pid,&status,0)==pid && WIFEXITED(status) && WEXITSTATUS(status)==0);
      puts("PASS: real libc fork success and child-local syscall-denied EAGAIN");
    }
  C
  File.write("#{dir}/actual.c",actual)
  out,status=Open3.capture2e('clang','-O1',"#{dir}/actual.c",'-o',"#{dir}/actual")
  abort out unless status.success?
  out,status=Open3.capture2e("#{dir}/actual",rlimit_core:0)
  puts out
  abort 'real libc fork contract failed' unless status.success?
end
