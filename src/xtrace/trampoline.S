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
