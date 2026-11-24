// The host signal stack of each host thread that runs emulated code.
//
// Every host handler runs on it (SA_ONSTACK), which leaves the interrupted stack below its stack pointer free
// for the dispatcher that runs a guest handler. The delivery state of the thread sits in front of it, where a
// handler finds it from the alternate stack the kernel reports, without touching thread-local storage:
//   [HostThreadSignals, page aligned][guard page][stack]

#include "darling_fexcore_signals.h"

#include <FEXCore/Utils/MathUtils.h>

#include <atomic>
#include <pthread.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <unistd.h>

namespace darling_fex {
namespace {

constexpr uint64_t kHostSignalsMagic = 0x44617273'69677374ULL;
constexpr size_t kHostSignalStackSize = 256 * 1024;
size_t g_page_size = 0;
size_t g_header_size = 0;
pthread_key_t g_host_stack_key;
pthread_once_t g_host_stack_key_once = PTHREAD_ONCE_INIT;

HostThreadSignals* host_signals_for(const stack_t& stack) {
  if ((stack.ss_flags & SS_DISABLE) || stack.ss_size != kHostSignalStackSize) {
    return nullptr;
  }
  auto* hs = reinterpret_cast<HostThreadSignals*>(static_cast<uint8_t*>(stack.ss_sp) - g_page_size - g_header_size);
  return hs->magic == kHostSignalsMagic ? hs : nullptr;
}

void destroy_host_stack(void* base) {
  stack_t off {};
  off.ss_flags = SS_DISABLE;
  sigaltstack(&off, nullptr);
  munmap(base, g_header_size + g_page_size + kHostSignalStackSize);
}

void create_host_stack_key() {
  pthread_key_create(&g_host_stack_key, destroy_host_stack);
}

HostThreadSignals* create_host_stack() {
  pthread_once(&g_host_stack_key_once, create_host_stack_key);
  const size_t total = g_header_size + g_page_size + kHostSignalStackSize;
  void* base = mmap(nullptr, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (base == MAP_FAILED) {
    return nullptr;
  }
  uint8_t* guard = static_cast<uint8_t*>(base) + g_header_size;
  stack_t stack {};
  stack.ss_sp = guard + g_page_size;
  stack.ss_size = kHostSignalStackSize;
  if (mprotect(guard, g_page_size, PROT_NONE) != 0 || sigaltstack(&stack, nullptr) != 0) {
    munmap(base, total);
    return nullptr;
  }
  auto* hs = static_cast<HostThreadSignals*>(base);
  hs->magic = kHostSignalsMagic;
  pthread_setspecific(g_host_stack_key, base);
  return hs;
}

} // namespace

HostThreadSignals* host_signals_of_this_thread() {
  stack_t current {};
  return sigaltstack(nullptr, &current) == 0 ? host_signals_for(current) : nullptr;
}

void host_signal_stacks_init() {
  g_page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));
  g_header_size = FEXCore::AlignUp(sizeof(HostThreadSignals), g_page_size);
}

HostThreadSignals* host_signals_in_handler(void* host_ucontext) {
  const stack_t& stack = static_cast<ucontext_t*>(host_ucontext)->uc_stack;
  // SS_ONSTACK: the interrupted code was already on the signal stack, so it was a host handler, not guest code.
  if (stack.ss_flags & SS_ONSTACK) {
    return nullptr;
  }
  return host_signals_for(stack);
}

bool signals_enter_thread(FEXCore::Core::InternalThreadState* thread, FEXCore::Core::InternalThreadState** previous) {
  HostThreadSignals* hs = host_signals_of_this_thread();
  if (!hs) {
    hs = create_host_stack();
    if (!hs) {
      return false;
    }
  }
  *previous = hs->thread;
  if (!*previous) {
    // An outermost run. The emulator thread is reused, and a guest that left the last run from inside a
    // handler (thread exit) left its handler count and deferred signals behind.
    thread->CurrentFrame->SignalHandlerRefCounter = 0;
    hs->deferred_count = 0;
    mprotect(thread->InterruptFaultPage, sizeof(thread->InterruptFaultPage), PROT_READ | PROT_WRITE);
  }
  __atomic_store_n(&hs->thread, thread, __ATOMIC_RELAXED);
  std::atomic_signal_fence(std::memory_order_seq_cst);
  return true;
}

void signals_leave_thread(FEXCore::Core::InternalThreadState* previous) {
  HostThreadSignals* hs = host_signals_of_this_thread();
  if (hs) {
    std::atomic_signal_fence(std::memory_order_seq_cst);
    __atomic_store_n(&hs->thread, previous, __ATOMIC_RELAXED);
  }
}

} // namespace darling_fex
