#ifndef _DARLING_COREDUMP_ARM64_H_
#define _DARLING_COREDUMP_ARM64_H_

// Linux
// Structs taken from <sys/user.h>

struct nt_prstatus_registers_aarch64
{
  unsigned long long regs[31];
  unsigned long long sp;
  unsigned long long pc;
  unsigned long long pstate;
};

// struct user_fpsimd_struct
// {
//   __uint128_t  vregs[32];
//   unsigned int fpsr;
//   unsigned int fpcr;
// };

// XNU
// Taken from the following headers
// * <mach/arm/_structs.h>
// * <mach/arm/thread_status.h>

typedef struct arm_thread_state64
{
	__uint64_t x[29]; /* General purpose registers x0-x28 */
	__uint64_t fp;    /* Frame pointer x29 */
	__uint64_t lr;    /* Link register x30 */
	__uint64_t sp;    /* Stack pointer x31 */
	__uint64_t pc;    /* Program counter */
	__uint32_t cpsr;  /* Current program status register */
	__uint32_t __pad; /* Same size for 32-bit or 64-bit clients */
} arm_thread_state64_t;

// typedef struct arm_neon_state64
// {
// 	__uint128_t q[32];
// 	uint32_t    fpsr;
// 	uint32_t    fpcr;
// } arm_neon_state64_t;

typedef struct arm_exception_state64
{
	__uint64_t far;         /* Virtual Fault Address */
	__uint32_t esr;         /* Exception syndrome */
	__uint32_t exception;   /* number of arm exception taken */
} arm_exception_state64_t;

typedef unsigned int            __darwin_natural_t;
typedef __darwin_natural_t      natural_t;
typedef natural_t mach_msg_type_number_t;

#define ARM_THREAD_STATE64       6
#define ARM_EXCEPTION_STATE64    7

// #define ARM_NEON_STATE64         17

#define ARM_THREAD_STATE64_COUNT ((mach_msg_type_number_t) (sizeof (arm_thread_state64_t)/sizeof(uint32_t)))
#define ARM_EXCEPTION_STATE64_COUNT ((mach_msg_type_number_t) (sizeof (arm_exception_state64_t)/sizeof(uint32_t)))
// #define ARM_NEON_STATE64_COUNT ((mach_msg_type_number_t) (sizeof (arm_neon_state64_t)/sizeof(uint32_t)))


#endif // _DARLING_COREDUMP_ARM64_H_