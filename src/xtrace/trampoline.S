.macro trampoline_enter
push %r9
push %r8
push %rcx
push %rdx
push %rsi
push %rdi
push %rax
movq %rax, %rdi
leaq 8(%rsp), %rsi
.endmacro

.macro trampoline_leave
pop %rax
pop %rdi
pop %rsi
pop %rdx
pop %rcx
pop %r8
pop %r9
.endmacro

.private_extern _darling_mach_syscall_entry_trampoline
_darling_mach_syscall_entry_trampoline:
	trampoline_enter
	call _darling_mach_syscall_entry_print
	trampoline_leave
	ret

.private_extern _darling_mach_syscall_exit_trampoline
_darling_mach_syscall_exit_trampoline:
	trampoline_enter
	call _darling_mach_syscall_exit_print
	trampoline_leave
	ret

.private_extern _darling_bsd_syscall_entry_trampoline
_darling_bsd_syscall_entry_trampoline:
	trampoline_enter
	call _darling_bsd_syscall_entry_print
	trampoline_leave
	ret

.private_extern _darling_bsd_syscall_exit_trampoline
_darling_bsd_syscall_exit_trampoline:
	trampoline_enter
	call _darling_bsd_syscall_exit_print
	trampoline_leave
	ret

