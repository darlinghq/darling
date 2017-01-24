#ifndef __PLATFORM_INTROSPECTION_H__
#define __PLATFORM_INTROSPECTION_H__

#include <mach/mach_types.h>
#include <mach/thread_info.h>
#include <mach/thread_status.h>

#include <sys/types.h>

#include <stdbool.h>

typedef struct platform_task_s *platform_task_t;
typedef struct platform_thread_s *platform_thread_t;

/*!
 * @typedef platform_thread_id_t
 *
 * @discussion
 * The type of the 64-bit system-wide unique thread ID.
 */
typedef uint64_t platform_thread_id_t;

/*! @functiongroup Tasks */

/*!
 * @function platform_task_attach
 *
 * @discussion
 * Attach to a process (specified by its mach task port) for debugging. This
 * function creates a new task handle which must be disposed by a call to
 * platform_task_detach().
 *
 * @param task
 * On output, a newly created task handle.
 *
 * @param target
 * The mach task port of the target process.
 *
 * @return
 * KERN_SUCCESS if the process was successfully attached, otherwise a mach
 * error code.
 */
kern_return_t
platform_task_attach(platform_task_t *task, task_t target);

/*!
 * @function platform_task_detach
 *
 * @discussion
 * Detaches from the target task and deallocates all memory associated with
 * the task handle.
 *
 * @param task
 * The task handle to detach.
 *
 * @return
 * KERN_SUCCESS if the process was successfully detached, otherwise a mach
 * error code.
 */
kern_return_t
platform_task_detach(platform_task_t task);

/*!
 * @function platform_task_is_64_bit
 *
 * @discussion
 * Returns true if the target task is LP64.
 *
 * @param task
 * A handle to the target task.
 *
 * @return
 * true if the target task is LP64, otherwise false.
 */
bool
platform_task_is_64_bit(platform_task_t task);

/*!
 * @function platform_task_suspend_threads
 *
 * @discussion
 * Suspends all the threads in the target task. This differs from task_suspend
 * in that the task itself is not suspended, only the individual threads. While
 * this suspension is in effect, any newly created threads will be created in
 * a suspended state. The debuger may resume an individual thread for execution
 * using platform_thread_resume() or evaluate an expression in the context of
 * the task or a specific thread using platform_task_perform() and
 * platform_thread_perform(), respectively. All threads in the task may be
 * resumed with platform_task_resume_threads().
 *
 * @param task
 * A handle to the target task.
 *
 * @return
 * KERN_SUCCESS if the threads were successfully suspended, otherwise a mach
 * error code.
 */
kern_return_t
platform_task_suspend_threads(platform_task_t task);

/*!
 * @function platform_task_resume_threads
 *
 * @discussion
 * Resumes the threads in the target task. See platform_task_suspend_threads().
 *
 * @param task
 * A handle to the target task.
 */
kern_return_t
platform_task_resume_threads(platform_task_t task);

/*!
 * @function platform_task_perform
 *
 * @discussion
 * Performs the specified function on a newly created thread in the target task.
 * This newly created thread will execute even if the threads in the task are
 * suspended as the result of a call to platform_task_suspend_threads().
 *
 * The function and context addresses are in the virtual address space of the
 * target task. It is the responsiblity of the debugger to have previously
 * mapped executable text and data at these addresses in the target task.
 *
 * @param task
 * A handle to the target task.
 *
 * @param func_addr
 * The address (in the virtual address space of the target task) of the
 * function to perform. The function should be of type (void (*)(void *))
 * and will be passed the value of the data_addr parameter.
 *
 * @param data_addr
 * The address (in the virtual address space of the target task) of the
 * data to pass as a parameter to the function to perform.
 *
 * @return
 * KERN_SUCCESS if the function was successfully performed, otherwise a mach
 * error code.
 */
kern_return_t
platform_task_perform(platform_task_t task,
		      mach_vm_address_t func_addr,
		      mach_vm_address_t data_addr);

/*!
 * @function platform_task_update_threads
 *
 * @discussion
 * Updates an internal representation of all threads in the target task. The
 * list of threads may then be iterated using platform_task_copy_next_thread().
 *
 * Calling this function resets any iteration currently in progress and a
 * subsequent call to platform_task_copy_next_thread() will return the first
 * thread in the list.
 *
 * @param task
 * A handle to the target task.
 *
 * @return
 * KERN_SUCCESS if the threads were successfully updated, otherwise a mach
 * error code.
 */
kern_return_t
platform_task_update_threads(platform_task_t task);

/*!
 * @function platform_task_copy_next_thread
 *
 * @discussion
 * Iterates the list of threads in the task. Returns a copied thread handle
 * which must subsequently be released using platform_thread_release().
 *
 * The platform_task_update_threads() function must be called before this
 * function will return any thread handles. A NULL pointer is returned to
 * signify the end of the list
 *
 * @param task
 * A handle to the target task.
 *
 * @return
 * A thread handle which must be released using platform_thread_release(),
 * or NULL, signifying the end of the list.
 */
platform_thread_t
platform_task_copy_next_thread(platform_task_t task);

/*! @functiongroup Threads */

/*!
 * @function platform_thread_get_unique_id
 *
 * @discussion
 * Returns the 64-bit system-wide unique ID of the target thread.
 *
 * @param thread
 * A handle to the target thread.
 *
 * @return
 * The unique ID of the thread.
 */
platform_thread_id_t
platform_thread_get_unique_id(platform_thread_t thread);

/*!
 * @function platform_thread_release
 *
 * @discussion
 * Releases a thread handle obtained by platform_task_copy_next_thread().
 *
 * @param thread
 * The thread handle to release.
 */
void
platform_thread_release(platform_thread_t thread);

/*!
 * @function platform_thread_abort_safely
 *
 * @discussion
 * Similar to thread_abort_safely().
 *
 * @param thread
 * A handle to the thread to signal.
 *
 * @return
 * KERN_SUCCESS if the thread was successfully signaled, otherwise a mach
 * error code.
 */
kern_return_t
platform_thread_abort_safely(platform_thread_t thread);

/*!
 * @function platform_thread_suspend
 *
 * @discussion
 * Suspends execution of a thread similar to thread_suspend(). See also
 * platform_task_suspend_threads().
 *
 * @param thread
 * A handle to the thread to suspend.
 *
 * @return
 * KERN_SUCCESS if the thread was successfully suspended, otherwise a mach
 * error code.
 */
kern_return_t
platform_thread_suspend(platform_thread_t thread);

/*!
 * @function platform_thread_resume
 *
 * @discussion
 * Suspends execution of a thread similar to thread_suspend(). See also
 * platform_task_suspend_threads() and platform_task_resume_threads().
 *
 * @param thread
 * A handle to the thread to resume.
 *
 * @return
 * KERN_SUCCESS if the thread was successfully resumed, otherwise a mach
 * error code.
*/
kern_return_t
platform_thread_resume(platform_thread_t thread);

/*!
 * @function platform_thread_info
 *
 * @discussion
 * Similar to thread_info. Supported flavor structures:
 * - THREAD_BASIC_INFO: struct thread_basic_info
 * - THREAD_IDENTIFIER_INFO: struct thread_identifier_info
 *
 * @param thread
 * A handle to the target thread.
 *
 * @param flavor
 * The desired thread info structure.
 *
 * @param info
 * A pointer to storage where the thread info structure should be written.
 *
 * @param size
 * On input, the size in bytes of the storage where the thread info structure
 * is to be written. On output, the size of the thread info structure in bytes.
 *
 * @return
 * KERN_SUCCESS if the function was successfully performed, otherwise a mach
 * error code.
 */
kern_return_t
platform_thread_info(platform_thread_t thread,
		       thread_flavor_t flavor,
		       void *info,
		       size_t *size);

/*!
 * @function platform_thread_get_state
 *
 * @discussion
 * Similar to thread_get_state. Supported flavor structures:
 * - x86_THREAD_STATE32: struct ...
 * - x86_FLOAT_STATE32: struct ...
 * - x86_EXCEPTION_STATE32: struct ...
 * - x86_DEBUG_STATE32: struct ...
 * - x86_AVX_STATE32: struct ...
 * - x86_THREAD_STATE64: struct ...
 * - x86_FLOAT_STATE64: struct ...
 * - x86_EXCEPTION_STATE64: struct ...
 * - x86_DEBUG_STATE64: struct ...
 * - x86_AVX_STATE64: struct ...
 * - ARM_THREAD_STATE32: struct ...
 * - ARM_FLOAT_STATE32: struct ...
 * - ARM_EXCEPTION_STATE32: struct ...
 * - ARM_DEBUG_STATE32: struct ...
 * - ARM_THREAD_STATE64: struct ...
 * - ARM_FLOAT_STATE64: struct ...
 * - ARM_EXCEPTION_STATE64: struct ...
 * - ARM_DEBUG_STATE64: struct ...
 * - ...
 *
 * @param thread
 * A handle to the target thread.
 *
 * @param flavor
 * The desired thread state structure.
 *
 * @param state
 * A pointer to storage where the thread state structure should be written.
 *
 * @param size
 * On input, the size in bytes of the storage where the thread state structure
 * is to be written. On output, the size of the thread state structure in bytes.
 *
 * @return
 * KERN_SUCCESS if the function was successfully performed, otherwise a mach
 * error code.
 */
kern_return_t
platform_thread_get_state(platform_thread_t thread,
			  thread_state_flavor_t flavor,
			  void *state,
			  size_t *size);

/*!
 * @function platform_thread_set_state
 *
 * @discussion
 * Similar to thread_set_state. Supported flavor structures:
 * - x86_THREAD_STATE32: struct ...
 * - x86_FLOAT_STATE32: struct ...
 * - x86_EXCEPTION_STATE32: struct ...
 * - x86_DEBUG_STATE32: struct ...
 * - x86_AVX_STATE32: struct ...
 * - x86_THREAD_STATE64: struct ...
 * - x86_FLOAT_STATE64: struct ...
 * - x86_EXCEPTION_STATE64: struct ...
 * - x86_DEBUG_STATE64: struct ...
 * - x86_AVX_STATE64: struct ...
 * - ARM_THREAD_STATE32: struct ...
 * - ARM_FLOAT_STATE32: struct ...
 * - ARM_EXCEPTION_STATE32: struct ...
 * - ARM_DEBUG_STATE32: struct ...
 * - ARM_THREAD_STATE64: struct ...
 * - ARM_FLOAT_STATE64: struct ...
 * - ARM_EXCEPTION_STATE64: struct ...
 * - ARM_DEBUG_STATE64: struct ...
 * - ...
 *
 * @param thread
 * A handle to the target thread.
 *
 * @param flavor
 * The desired thread state structure.
 *
 * @param state
 * A pointer to storage where the thread state structure should be written.
 *
 * @param size
 * The size of the thread state structure in bytes.
 *
 * @return
 * KERN_SUCCESS if the function was successfully performed, otherwise a mach
 * error code.
 */
kern_return_t
platform_thread_set_state(platform_thread_t thread,
			  thread_state_flavor_t flavor,
			  const void *state,
			  size_t size);

/*!
 * @function platform_thread_perform
 *
 * @discussion
 * Performs the specified function within the context of the specified thread
 * in the target task. The function will execute in the style of an
 * asynchronous signal handler even if the thread is suspended as the result
 * of a call to platform_task_suspend_threads() or platform_thread_suspend().
 * The original state of the thread will be restored when the function returns.
 *
 * The function and context addresses are in the virtual address space of the
 * target task. It is the responsiblity of the debugger to have previously
 * mapped executable text and data at these addresses in the target task.
 *
 * See also platform_task_perform().
 *
 * @param thread
 * A handle to the target thread.
 *
 * @param func_addr
 * The address (in the virtual address space of the target task) of the
 * function to perform. The function should be of type (void (*)(void *))
 * and will be passed the value of the data_addr parameter.
 *
 * @param data_addr
 * The address (in the virtual address space of the target task) of the
 * data to pass as a parameter to the function to perform.
 *
 * @return
 * KERN_SUCCESS if the function was successfully performed, otherwise a mach
 * error code.
 */
kern_return_t
platform_thread_perform(platform_thread_t thread,
			mach_vm_address_t func_addr,
			mach_vm_address_t data_addr);

/*!
 * @function platform_thread_get_pthread
 *
 * @discussion
 * Returns a pointer to mapped memory which represents the pthread_t of the
 * target process. Any embedded pointers will need to be mapped into the current 
 * process space on a case-by-case basis.
 *
 * @param thread
 * A handle to the target thread.
 *
 * @return
 * A valid pointer.
 */
const void *
platform_thread_get_pthread(platform_thread_t thread);

#endif // __PLATFORM_INTROSPECTION_H__
