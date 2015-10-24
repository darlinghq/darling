



all:
	@g++ foo.cxx -dynamiclib -o libfoo.dylib -arch i386 -arch x86_64 -Wl,-no_compact_unwind
	@gcc main.c -arch i386 -o main-i386
	@./main-i386 && echo "PASS dwarf-cache-dlclose -arch i386"
	@gcc main.c -arch x86_64 -o main-x86_64
	@./main-x86_64 && echo "PASS dwarf-cache-dlclose -arch x86_64"
#	@gcc main.c -arch ppc -o main-ppc
#	@./main-ppc && echo "PASS dwarf-cache-dlclose -arch ppc"
	
clean:
	@rm -f libfoo.dylib main-i386 main-x86_64 main-ppc


