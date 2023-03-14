#if defined(__x86_64__)

.macro trampoline_enter
pushq %rbp
movq %rsp, %rbp

# Align the stack
andq $$~15, %rsp
# There's an odd number of pushq's below
subq $$8, %rsp

# Re-push possible 7th and 8th arg
movq 40(%rbp), %r11
pushq %r11
movq 32(%rbp), %r11
pushq %r11

# Push args from 1st to 6th
pushq %r9
pushq %r8
pushq %rcx
pushq %rdx
pushq %rsi
pushq %rdi
pushq %rax

# Pass the syscall num and the pointer
# to the on-stack buffer as arguments
movq %rax, %rdi
leaq 8(%rsp), %rsi
.endmacro

.macro trampoline_leave
# Read back the registers
popq %rax
popq %rdi
popq %rsi
popq %rdx
popq %rcx
popq %r8
popq %r9
leave
ret
.endmacro

#elif defined(__i386__)

.macro trampoline_enter
pushl %ebp
movl %esp, %ebp

# align the stack
andl $$~15, %esp

#define copy_arg(off) \
	movl 16+off(%ebp), %ecx ;\
	movl %ecx, -56+off(%esp)

// copy the arguments (same as in syscalls-table.S and darling_mach_syscall.S)
copy_arg(0)
copy_arg(4)
copy_arg(8)
copy_arg(12)
copy_arg(16)
copy_arg(20)
copy_arg(24)
copy_arg(28)
copy_arg(32)
copy_arg(36)
copy_arg(40)
copy_arg(44)
copy_arg(48)
copy_arg(52)

# make space on the stack now for the arguments we just copied
subl $$56, %esp

# push the address of the argument area as the second argument
pushl %esp

# push the call number or return value as the first argument
pushl %eax
.endmacro

.macro trampoline_leave
# restore eax (i.e. the call number or return value)
popl %eax
leave
ret
.endmacro

#endif

.private_extern _darling_mach_syscall_entry_trampoline
_darling_mach_syscall_entry_trampoline:
	trampoline_enter
	call _darling_mach_syscall_entry_print
	trampoline_leave

.private_extern _darling_mach_syscall_exit_trampoline
_darling_mach_syscall_exit_trampoline:
	trampoline_enter
	call _darling_mach_syscall_exit_print
	trampoline_leave

.private_extern _darling_bsd_syscall_entry_trampoline
_darling_bsd_syscall_entry_trampoline:
	trampoline_enter
	call _darling_bsd_syscall_entry_print
	trampoline_leave

.private_extern _darling_bsd_syscall_exit_trampoline
_darling_bsd_syscall_exit_trampoline:
	trampoline_enter
	call _darling_bsd_syscall_exit_print
	trampoline_leave
