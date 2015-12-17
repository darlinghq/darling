default:
	@echo This makefile builds Data.c and Intel/Data.s, which contain constant
	@echo data for the AES implementation.  These files do not normally need to
	@echo be rebuilt, so they are checked into the source code repository.  They
	@echo should be changed only when the implementation changes and needs
	@echo data in a different format.
	@echo
	@echo To rebuild the files, execute "make all".

.PHONY:	all clean
Targets = Data.c Intel/Data.s
all:	$(Targets)

CFLAGS += -O3 -std=c99 -Wmost -Werror

.INTERMEDIATE:	MakeData
MakeData:	MakeData.c

# Do not leave bad output files if the build fails.
.DELETE_ON_ERROR:	$(Targets)

Data.c:	MakeData
	./$< >$@ C

Intel/Data.s:	MakeData
	./$< >$@ Intel

clean:
	-rm $(Targets)
