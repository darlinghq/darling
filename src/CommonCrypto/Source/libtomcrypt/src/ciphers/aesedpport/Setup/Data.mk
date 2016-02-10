default:
	@echo "This makefile builds Data.s, which contains constant data for the"
	@echo "AES implementation.  This file does not normally need to be rebuilt,"
	@echo "so it is checked into the source code repository.  It should be"
	@echo "changed only when the implementation changes and needs data in a"
	@echo "different format.  (This file can also build a C version, Data.c,"
	@echo "but that is not currently in use.)"
	@echo ""
	@echo "To rebuild the file(s), execute \"make -f Data.mk all\"."

.PHONY:	all clean
Targets = Data.s
all:	$(Targets)

CFLAGS += -O3 -std=c99 -Wmost -Werror

.INTERMEDIATE:	MakeData
MakeData:	MakeData.c

# Do not leave bad output files if the build fails.
.DELETE_ON_ERROR:	$(Targets)

Data.c:	MakeData
	./$< >$@ C

Data.s:	MakeData
	./$< >$@ Intel

clean:
	-rm $(Targets)
