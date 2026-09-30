require 'tmpdir'
require 'open3'
root=ARGV.fetch(0,File.expand_path('..',__dir__))
loader=File.read("#{root}/src/startup/mldr/mldr.c")
bitmap=loader[/typedef struct socket_bitmap \{.*?(?=static int socket_bitmap_get)/m] or abort 'bitmap helpers missing'
method=File.read("#{root}/src/startup/mldr/elfcalls/elfcalls.c")[/^static int native_fork\(void\)\n\{.*?^\}/m] or abort 'fork callback missing'
Dir.mktmpdir('fork-bitmap-') do |dir|
  File.write("#{dir}/probe.c",<<~C)
    #include <pthread.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <assert.h>
    #include <stdio.h>
    #include <sys/wait.h>
    #{bitmap}
    static void __mldr_thread_postfork_child(void) {}
    #{method}
    static int ready[2];
    static void *update(void *unused) {
      pthread_mutex_lock(&socket_bitmap.mutex);
      socket_bitmap.highest=111;
      assert(write(ready[1],"x",1)==1);
      usleep(200000);
      socket_bitmap.bits=malloc(1); assert(socket_bitmap.bits);
      socket_bitmap.bits[0]=1;
      socket_bitmap.bit_length=8;
      socket_bitmap.next_index=1;
      pthread_mutex_unlock(&socket_bitmap.mutex);
      return NULL;
    }
    int main(void) {
      alarm(10);
      assert(pipe(ready)==0);
      pthread_t worker; assert(pthread_create(&worker,NULL,update,NULL)==0);
      char byte; assert(read(ready[0],&byte,1)==1);
      int child=native_fork(); assert(child>=0);
      if(!child) {
        assert(socket_bitmap.highest==111 && socket_bitmap.bit_length==8);
        assert(socket_bitmap.next_index==1 && socket_bitmap.bits[0]==1);
        assert(pthread_mutex_trylock(&socket_bitmap.mutex)==0);
        pthread_mutex_unlock(&socket_bitmap.mutex);
        _exit(0);
      }
      int status; assert(waitpid(child,&status,0)==child);
      assert(WIFEXITED(status) && WEXITSTATUS(status)==0);
      assert(pthread_join(worker,NULL)==0);
      assert(pthread_mutex_trylock(&socket_bitmap.mutex)==0);
      pthread_mutex_unlock(&socket_bitmap.mutex);
      free(socket_bitmap.bits);
      close(ready[0]); close(ready[1]);
      puts("PASS: contended bitmap transaction completes before fork; both copies unlock");
    }
  C
  out,status=Open3.capture2e('clang','-pthread','-fsanitize=address,undefined',"#{dir}/probe.c",'-o',"#{dir}/probe")
  abort out unless status.success?
  out,status=Open3.capture2e("#{dir}/probe",rlimit_core:0)
  puts out
  abort 'bitmap fork regression' unless status.success?
end
