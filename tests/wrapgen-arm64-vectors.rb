require 'tmpdir'
require 'open3'
require 'json'
root=File.realpath(ARGV.fetch(0))
def run(*args)
  out,status=Open3.capture2e(*args)
  abort out unless status.success?
end
Dir.mktmpdir('merged-wrapgen-') do |dir|
  File.write("#{dir}/fixture.c", "int fixture_value=7; int fixture_function(int x) { return x+fixture_value; }\n")
  run('clang','-shared','-fPIC',"#{dir}/fixture.c",'-o',"#{dir}/fixture.so")
  run('clang++','-std=c++17',"#{root}/src/libelfloader/wrapgen/wrapgen.cpp",'-ldl','-o',"#{dir}/wrapgen")
  run("#{dir}/wrapgen","#{dir}/fixture.so","#{dir}/wrapper.c","#{dir}/vars.h")
  source=File.read("#{dir}/wrapper.c")
  abort 'obsolete TLS switch remains' if source.include?('tpidr_el0')
  abort 'incorrect frame' unless source.include?('sub sp, sp, #208')
  %w[stp ldp].each do |op|
    4.times do |i|
      abort 'vector preservation missing' unless source.include?("#{op} q#{i*2}, q#{i*2+1}, [sp, ##{80+i*32}]")
    end
  end
  abort 'variable accessor missing' unless source.include?('__elf_get_fixture_value')
  run('clang','-target','arm64-apple-darwin20','-ffreestanding',"-I#{root}/src/startup/mldr/elfcalls",'-c',"#{dir}/wrapper.c",'-o',"#{dir}/wrapper.o")
  puts 'PASS generated ARM64 wrapper assembles; full q0-q7 saves/restores and variable accessor present'
  # Execute the generated common trampoline natively. Only Mach-O visibility
  # directives are translated; the generated instructions stay unchanged.
  literal=source[/__asm__\((".*?")\);/m,1]
  abort 'common trampoline missing' unless literal
  assembly=JSON.parse(literal).gsub('.private_extern', '.global')
  assembly += <<~ASM
    .global test_entry
    test_entry:
      adrp x16, test_symbol
      add x16, x16, :lo12:test_symbol
      b ___elf_lazy_common
    .section .note.GNU-stack,"",@progbits
  ASM
  File.write("#{dir}/trampoline.S",assembly)
  File.write("#{dir}/vectors.c", <<~C)
    #include <stdio.h>
    typedef unsigned int V __attribute__((vector_size(16)));
    struct symbol { void *address; const char *name; } test_symbol;
    static int resolutions;
    static V target(V a,V b,V c,V d,V e,V f,V g,V h,V stack) {
      return a+b+c+d+e+f+g+h+stack;
    }
    void *___elf_lazy_resolve(struct symbol *symbol) {
      ++resolutions;
      __asm__ volatile(
        "movi v0.16b, #0\\n movi v1.16b, #0\\n movi v2.16b, #0\\n movi v3.16b, #0\\n"
        "movi v4.16b, #0\\n movi v5.16b, #0\\n movi v6.16b, #0\\n movi v7.16b, #0"
        ::: "v0","v1","v2","v3","v4","v5","v6","v7");
      symbol->address=(void*)target;
      return symbol->address;
    }
    extern V test_entry(V,V,V,V,V,V,V,V,V);
    int main(void) {
      V a={1,2,3,4}, b={5,6,7,8}, c={9,10,11,12}, d={13,14,15,16};
      V e={17,18,19,20}, f={21,22,23,24}, g={25,26,27,28}, h={29,30,31,32}, s={33,34,35,36};
      V expected=target(a,b,c,d,e,f,g,h,s);
      for(int call=0;call<2;++call) {
        V got=test_entry(a,b,c,d,e,f,g,h,s);
        for(int lane=0;lane<4;++lane) if(got[lane]!=expected[lane]) return 1;
      }
      if(resolutions!=1) return 2;
      puts("PASS live lazy/fast calls preserve eight vector registers and stack argument");
      return 0;
    }
  C
  run('clang','-O2',"#{dir}/vectors.c","#{dir}/trampoline.S",'-o',"#{dir}/vectors")
  run("#{dir}/vectors")
  puts 'PASS live generated trampoline: slow and cached paths, all vector lanes, stack argument'
  broken=assembly.gsub(/\b(stp|ldp) q([0-7]), q([0-7])/, '\\1 d\\2, d\\3')
  File.write("#{dir}/narrow.S",broken)
  run('clang','-O2',"#{dir}/vectors.c","#{dir}/narrow.S",'-o',"#{dir}/narrow")
  _,status=Open3.capture2e("#{dir}/narrow")
  abort 'negative control did not detect narrow register saves' unless status.exitstatus==1
  puts 'PASS negative control: d-register saves fail the vector test'
end
