#include <functional>
#include <stdint.h>
#include <iostream>
#include <type_traits>
#include <stdexcept>
#include <vector>
#include <string>

struct AssemblyString
{
	std::string name;
	std::string value;
};
static std::vector<AssemblyString> _strings;

template <typename Retval>
void handle_retval() { std::cout << "\t// retval - nop (void or 4-byte integer)\n"; }

template <> void handle_retval<long long>()
{
	std::cout << "\t// retval - 8-byte integer\n"
		"\tmovq %rax, %rdx\n"
		"\tshrq $32, %rdx\n"; // place upper 32-bits into edx
	std::cout << "\t// end retval\n";
}
template <> void handle_retval<void*>() { handle_retval<long long>(); }
template <> void handle_retval<unsigned long long>() { handle_retval<long long>(); }

class Context
{
public:
	Context(const char* name)
		: _name(name)
	{
	}

	bool has_int_registers_left() const
	{
		return _int_count < 6;
	}

	const char* next_int_register(bool _32bit = false)
	{
		if (!has_int_registers_left())
			throw std::logic_error("No registers left");

		const char* ARGUMENT_REGS[] = { "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9" };
		const char* ARGUMENT_REGS32[] = { "%edi", "%esi", "%edx", "%ecx", "%r8", "%r9" };
		return _32bit ? ARGUMENT_REGS32[_int_count++] : ARGUMENT_REGS[_int_count++];
	}

	bool has_fp_registers_left() const
	{
		return _fp_count < 8;
	}

	const char* next_fp_register()
	{
		if (!has_fp_registers_left())
			throw std::logic_error("No fp registers left");

		const char* FP_ARGUMENT_REGS[] = { "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7" };
		return FP_ARGUMENT_REGS[_fp_count++];
	}

	int esp_advance(int how_much)
	{
		int rv = _esp_offset;
		_esp_offset += how_much;
		return rv;
	}

	int next_int_stack_offset()
	{
		if (has_int_registers_left())
			throw std::logic_error("Registers are still left");

		int offset = (_int_count-6) * 8;
		offset += std::max(0, (_fp_count-8) * 16);

		_int_count++;
		return offset;
	}

	int next_fp_stack_offset()
	{
		if (has_fp_registers_left())
			throw std::logic_error("FP registers are still left");

		int offset = (_fp_count-8) * 16;
		offset += std::max(0, (_int_count-6) * 8);

		_fp_count++;
		return offset;
	}

protected:
	const char* _name;
	// Next offset in 32-bit argument stack
	int _esp_offset = 4;
	// How many integer arguments have been converted so far
	int _int_count = 0;
	// How many floating point arguments have been converted so far
	int _fp_count = 0;
};

template <typename Argtype>
	void copy_argument(Context& ctxt)
{
	std::cout << "\tmovl " << ctxt.esp_advance(4) << "(%rsp), " << ctxt.next_int_register(true) << std::endl;
}

template <>
	void copy_argument<long long>(Context& ctxt)
{
	std::cout << "\tmovq " << ctxt.esp_advance(8) << "(%rsp), " << ctxt.next_int_register() << std::endl;
}

template <typename Argtype> void count_argument(int& ints, int& fps) { ints++; }
template <> void count_argument<float>(int& ints, int& fps) { fps++; }
template <> void count_argument<double>(int& ints, int& fps) { fps++; }

template <typename ...Args>
class handle_arguments;

template <>
class handle_arguments<>
{
public:
	static void generate(Context& context, int index = 0)
	{
	}

	static void count(int& ints, int& fps)
	{
	}
};

template <typename Arg0, typename ...Args>
class handle_arguments<Arg0, Args...>
{
public:
	static void generate(Context& ctxt, int index = 0)
	{
		std::cout << "\t// argument #" << index << "\n";
		copy_argument<Arg0>(ctxt);
		handle_arguments<Args...>::generate(ctxt, index+1);
	}

	static void count(int& ints, int& fps)
	{
		count_argument<Arg0>(ints, fps);
		handle_arguments<Args...>::count(ints, fps);
	}
};

template <typename Retval, typename ...Args>
class Stubifier;

template <typename Retval, typename ...Args>
class Stubifier<Retval(Args...)> : public Context
{
public:
	Stubifier(const char* name)
		: Context(name)
	{
	}

	void generate()
	{
		int stackalign = 4; // for return address of the call to this stub
		int ints = 0, fps = 0;

		_strings.push_back(AssemblyString{ std::string("_name_") + _name, _name });

		std::cout << ".globl _" << _name << "\n_"
			<< _name << ":\n"
			"\tprologue\n";

		std::cout << "\t// Get real implementation\n";
		std::cout << "\tmovq _" << _name << "_impl(%rip), %r10\n";
		std::cout << "\ttestq %r10, %r10\n"
			"\tjnz 1f\n"
			"\tloadfunc _name_" << _name << ", _" << _name << "_impl\n";

		std::cout << "1:\n";
		handle_arguments<Args...>::count(ints, fps);

		if (ints >= 1)
		{
			stackalign += 4; // calee save edi
			if (ints >= 2)
			{
				stackalign += 4; // calee save esi
				if (ints > 6)
					stackalign += (ints-6) * 8;
			}
		}
		if (fps > 8)
			stackalign += (fps-8) * 16;

		// The stack must be 16-byte aligned
		stackalign = (stackalign + 15) / 16;
		stackalign *= 16;
		stackalign -= 4; // subtract the 4 bytes we had from the start

		// Arguments on the stack are now farther away
		_esp_offset += stackalign;

		std::cout << "\t// shift stack for alignment and/or extra stuff on stack\n";
		std::cout << "\tsubq $" << stackalign << ", %rsp\n";

		// esi and edi are calee save
		if (ints >= 1)
		{
			std::cout << "\t// calee save registers\n";
			std::cout << "\tmovl %edi, " << stackalign-4 << "(%rsp)\n";
			if (ints >= 2)
				std::cout << "\tmovl %esi, " << stackalign-8 << "(%rsp)\n";
		}

		handle_arguments<Args...>::generate(*this);

		// call the implementation
		std::cout << "\t// call the implementation\n"
			"\tcallq *%r10\n";

		handle_retval<Retval>();

		// restore edi and esi
		if (ints >= 1)
		{
			std::cout << "\t// restore calee save registers\n";
			if (ints >= 2)
				std::cout << "\tmovl " << stackalign-8 << "(%rsp), %esi\n";
			std::cout << "\tmovl " << stackalign-4 << "(%rsp), %edi\n";
		}

		std::cout << "\taddq $" << stackalign << ", %rsp\n";

		std::cout << "\tepilogue\n"
			"// end of " << _name << "\n\n";
		std::cout << ".zerofill __DATA,__bss,_" << _name << "_impl,8,3\n";
	}

};

void print_init()
{
	std::cout << ".zerofill __DATA,__bss,_lib_handle,8,3\n";

	std::cout << ".code64\n";
	std::cout << "_initializer:\n"
				"\tpushq   %rdi\n"
        		"\tmovq    L__darling_elfcalls$non_lazy_ptr(%rip), %rax\n"
		        "\tmovq    (%rax), %rax\n"
        		"\tmovq    _library_name(%rip), %rdi\n"
       			"\tcallq   *(%rax)\n"
        		"\tmovq    %rax, _lib_handle(%rip)\n"
				"\tpopq    %rdi\n"
		        "\tret\n\n";

	std::cout << "_destructor:\n"
				"\tpushq   %rdi\n"
				"\tmovq    L__darling_elfcalls$non_lazy_ptr(%rip), %rcx\n"
				"\tmovq    (%rcx), %rcx\n"
				"\tmovq    _lib_handle(%rip), %rdi\n"
				"\tcallq   *8(%rcx)\n"
				"\tpopq    %rdi\n"
				"\tret\n\n";

	std::cout << ".code32\n";
	std::cout << ".section __DATA,__mod_init_func,mod_init_funcs\n"
        ".align  2\n"
        ".long   _initializer\n"
        ".section __DATA,__mod_term_func,mod_term_funcs\n"
        ".align  2\n"
        ".long   _destructor\n\n";
}

#define STUBIFY(name) Stubifier<decltype(name)>(#name).generate()

void produce_stubs();
extern const char* library_name;

int main()
{
	_strings.push_back(AssemblyString{ "_library_name", library_name });

	std::cout << ".section __TEXT,__text,regular,pure_instructions\n\n";

	std::cout << ".macro prologue\n"
			"\tsubl $$8, %esp\n"
			"\tmovl $$0x33, 4(%esp)\n"
			"\tleal 2f, %eax\n"
			"\tmovl %eax, (%esp)\n"
			"\tlret\n"
			/* Now in 64-bit mode */
			"2:\n" \
			".code64\n" \
			".endmacro\n\n";

	std::cout << ".macro epilogue\n"
			/* Transition 64->32 */
			"\tsubq $$8, %rsp\n"
			"\tleaq 3f, %rcx\n"
			"\tmovl $$0x23, 4(%rsp)\n"
			"\tmovl %ecx, (%rsp)\n"
			"\tlret\n"
			"3:\n"
			".code32\n"
			"\tpush $$0x2b\n"
			"\tpop %ds\n"
			"\tret\n"
			".endmacro\n\n";

	std::cout << ".macro loadfunc\n"
			"\tpushq %rdi\n"
			"\tpushq %rsi\n"
			"\tmovq _lib_handle(%rip), %rdi\n"
			"\tmovq $0(%rip), %rsi\n"
			"\tmovq L__darling_elfcalls$$non_lazy_ptr(%rip), %rax\n"
			"\tmovq (%rax), %rax\n"
			"\tcallq *16(%rax)\n"
			"\tmovq %rax, $1\n"
			"\tmovq %rax, %r10\n"
			"\tpopq %rsi\n"
			"\tpopq %rdi\n"
			".endmacro\n\n";

	print_init();
	produce_stubs();

	std::cout << ".section __TEXT,__cstring,cstring_literals\n";
	for (const AssemblyString& str : _strings)
	{
		std::cout << "L_." << str.name.c_str() << ": .asciz \"" << str.value.c_str() << "\"\n";
		std::cout << str.name.c_str() << ": .long L_." << str.name.c_str() << std::endl;
	}
	std::cout << std::endl;

	std::cout << ".section __IMPORT,__pointers,non_lazy_symbol_pointers\n"
				"L__darling_elfcalls$non_lazy_ptr:\n"
		        ".indirect_symbol __darling_elfcalls\n"
				".long   0\n\n";

	std::cout << ".subsections_via_symbols\n\n";

	return 0;
}

#include "produce_stubs.h"


