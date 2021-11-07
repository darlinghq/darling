/*
 * This function needs to be in a separate C file rather than in bsd_trace.cpp
 * because `sys/spawn_internal.h` uses some GNU C extensions that C++ doesn't seem to like.
 */

#define PRIVATE 1
#include <sys/spawn_internal.h>
#include <spawn_private.h>

#include <darling/emulation/simple.h>

extern int xtrace_format_string_literal(char* buf, const char* str);
extern int print_open_flags(char* buf, void* arg);

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

int print_arg_posix_spawn_args(char* buf, void* arg) {
	const char* initial_buf = buf;
	const struct _posix_spawn_args_desc* args = (const struct _posix_spawn_args_desc*)(arg);
	bool is_first = true;

	buf += __simple_sprintf(buf, "{ attributes = ");

	if (args && args->attrp) {
		for (size_t i = 0; all_posix_spawn_attrs[i].name != NULL; i++) {
			if (args->attrp->psa_flags & all_posix_spawn_attrs[i].flag) {
				if (is_first) {
					is_first = false;
				} else {
					*buf++ = '|';
				}

				buf += __simple_sprintf(buf, "%s", all_posix_spawn_attrs[i].name);
			}
		}
	}

	if (is_first) {
		*buf++ = '0';
	}

	buf += __simple_sprintf(buf, ", file_actions = {");

	if (args && args->file_actions) {
		for (size_t i = 0; i < args->file_actions->psfa_act_count; ++i) {
			const struct _psfa_action* action = &args->file_actions->psfa_act_acts[i];

			if (i != 0) {
				*buf++ = ',';
				*buf++ = ' ';
			}

			switch (action->psfaa_type) {
				case PSFA_OPEN:
					buf += __simple_sprintf(buf, "open(");
					buf += xtrace_format_string_literal(buf, action->psfaa_openargs.psfao_path);
					*buf++ = ',';
					*buf++ = ' ';
					buf += print_open_flags(buf, (void*)(intptr_t)(action->psfaa_openargs.psfao_oflag));
					buf += __simple_sprintf(buf, ", 0%o) to %d", action->psfaa_openargs.psfao_mode, action->psfaa_filedes);
					break;

				case PSFA_CLOSE:
					buf += __simple_sprintf(buf, "close(%d)", action->psfaa_filedes);
					break;

				case PSFA_DUP2:
					buf += __simple_sprintf(buf, "dup2(%d, %d)", action->psfaa_filedes, action->psfaa_dup2args.psfad_newfiledes);
					break;

				case PSFA_INHERIT:
					buf += __simple_sprintf(buf, "inherit(%d)", action->psfaa_filedes);
					break;

				case PSFA_FILEPORT_DUP2:
					// NOTE: if we see this in the output, that's an issue;
					//       we don't have this implemented right now
					buf += __simple_sprintf(buf, "dup2_fileport(port right %d, %d)", action->psfaa_fileport, action->psfaa_dup2args.psfad_newfiledes);
					break;

				case PSFA_CHDIR:
					buf += __simple_sprintf(buf, "chdir(");
					buf += xtrace_format_string_literal(buf, action->psfaa_chdirargs.psfac_path);
					*buf++ = ')';
					break;

				case PSFA_FCHDIR:
					buf += __simple_sprintf(buf, "fchdir(%d)", action->psfaa_filedes);
					break;

				default:
					buf += __simple_sprintf(buf, "???");
					break;
			}
		}
	}

	buf += __simple_sprintf(buf, "} }");

	return buf - initial_buf;
};
