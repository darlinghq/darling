/*
 * This function needs to be in a separate C file rather than in bsd_trace.cpp
 * because `sys/spawn_internal.h` uses some GNU C extensions that C++ doesn't seem to like.
 */

#define PRIVATE 1
#include <sys/spawn_internal.h>
#include <spawn_private.h>

#include <darling/emulation/simple.h>
#include "bsd_trace.h"

#include "xtracelib.h"

extern "C" void print_open_flags(void* arg);

static struct {
	unsigned short flag;
	const char* name;
} all_posix_spawn_attrs[] = {
#define POSIX_SPAWN_ATTR_ENTRY(name) { name, #name }
	POSIX_SPAWN_ATTR_ENTRY(POSIX_SPAWN_RESETIDS),
	POSIX_SPAWN_ATTR_ENTRY(POSIX_SPAWN_SETPGROUP),
	POSIX_SPAWN_ATTR_ENTRY(POSIX_SPAWN_SETSIGDEF),
	POSIX_SPAWN_ATTR_ENTRY(POSIX_SPAWN_SETSIGMASK),
	POSIX_SPAWN_ATTR_ENTRY(POSIX_SPAWN_SETEXEC),
	POSIX_SPAWN_ATTR_ENTRY(POSIX_SPAWN_START_SUSPENDED),
	POSIX_SPAWN_ATTR_ENTRY(_POSIX_SPAWN_DISABLE_ASLR),
	POSIX_SPAWN_ATTR_ENTRY(_POSIX_SPAWN_NANO_ALLOCATOR),
	POSIX_SPAWN_ATTR_ENTRY(POSIX_SPAWN_SETSID),
	POSIX_SPAWN_ATTR_ENTRY(_POSIX_SPAWN_ALLOW_DATA_EXEC),
	POSIX_SPAWN_ATTR_ENTRY(POSIX_SPAWN_CLOEXEC_DEFAULT),
	POSIX_SPAWN_ATTR_ENTRY(_POSIX_SPAWN_HIGH_BITS_ASLR),
	{ 0, NULL },
#undef POSIX_SPAWN_ATTR_ENTRY
};

extern "C"
void print_arg_posix_spawn_args(void* arg) {
	const struct _posix_spawn_args_desc* args = (const struct _posix_spawn_args_desc*)(arg);
	bool is_first = true;

	xtrace_log("{ attributes = ");

	if (args && args->attrp) {
		for (size_t i = 0; all_posix_spawn_attrs[i].name != NULL; i++) {
			if (args->attrp->psa_flags & all_posix_spawn_attrs[i].flag) {
				if (is_first) {
					is_first = false;
				} else {
					xtrace_log("|");
				}

				xtrace_log("%s", all_posix_spawn_attrs[i].name);
			}
		}
	}

	if (is_first) {
		xtrace_log("0");
	}

	xtrace_log(", file_actions = {");

	if (args && args->file_actions) {
		for (size_t i = 0; i < args->file_actions->psfa_act_count; ++i) {
			const struct _psfa_action* action = &args->file_actions->psfa_act_acts[i];

			if (i != 0) {
				xtrace_log(", ");
			}

			switch (action->psfaa_type) {
				case PSFA_OPEN:
					xtrace_log("open(");
					xtrace_print_string_literal(action->psfaa_openargs.psfao_path);
					xtrace_log(", ");
					print_open_flags((void*)(intptr_t)(action->psfaa_openargs.psfao_oflag));
					xtrace_log(", 0%o) to %d", action->psfaa_openargs.psfao_mode, action->psfaa_filedes);
					break;

				case PSFA_CLOSE:
					xtrace_log("close(%d)", action->psfaa_filedes);
					break;

				case PSFA_DUP2:
					xtrace_log("dup2(%d, %d)", action->psfaa_filedes, action->psfaa_dup2args.psfad_newfiledes);
					break;

				case PSFA_INHERIT:
					xtrace_log("inherit(%d)", action->psfaa_filedes);
					break;

				case PSFA_FILEPORT_DUP2:
					// NOTE: if we see this in the output, that's an issue;
					//       we don't have this implemented right now
					xtrace_log("dup2_fileport(port right %d, %d)", action->psfaa_fileport, action->psfaa_dup2args.psfad_newfiledes);
					break;

				case PSFA_CHDIR:
					xtrace_log("chdir(");
					xtrace_print_string_literal(action->psfaa_chdirargs.psfac_path);
					xtrace_log(")");
					break;

				case PSFA_FCHDIR:
					xtrace_log("fchdir(%d)", action->psfaa_filedes);
					break;

				default:
					xtrace_log("???");
					break;
			}
		}
	}

	xtrace_log("} }");
};
