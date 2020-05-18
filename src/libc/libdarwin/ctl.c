#include "internal.h"

#pragma mark Definitions
#define CTL_OUTPUT_WIDTH (80)
#define CTL_OUTPUT_OPTARG_PAD (28)
#define CTL_OUTPUT_LIST_PAD (4)
#define SUBCOMMAND_LINKER_SET "__subcommands"

#define OS_SUBCOMMAND_OPTIONS_FOREACH(_osco_i, _osc, _which, _i) \
		while (((_osco_i) = &osc->osc_ ## _which[(_i)]) && \
			((_i) += 1, 1) && \
			!((_osco_i)->osco_flags & OS_SUBCOMMAND_OPTION_FLAG_TERMINATOR))

#pragma mark Types
OS_ENUM(os_subcommand_option_spec_fmt, uint64_t,
	OS_SUBCOMMAND_OPTION_SPEC_SHORT,
	OS_SUBCOMMAND_OPTION_SPEC_LONG,
	OS_SUBCOMMAND_OPTION_SPEC_COMBINED,
);

#pragma mark Forward Declarations
static void _print_header(
		FILE *f,
		const char *hdr,
		bool *already_done);
static const os_subcommand_t *_os_subcommand_find(
		const char *name);
static void _os_subcommand_print_usage(
		const os_subcommand_t *osc,
		FILE *f);
static void _os_subcommand_print_help_line(
		const os_subcommand_t *osc,
		FILE *f);

#pragma mark Module Globals
static const os_subcommand_t __help_cmd;
static const os_subcommand_t *_help_cmd = &__help_cmd;

static const os_subcommand_t __main_cmd;
static const os_subcommand_t *_main_cmd = &__main_cmd;
static const os_subcommand_t *_internal_main_cmd = &__main_cmd;

#pragma mark Main Subcommand
static int _main_invoke(const os_subcommand_t *osc,
	int argc,
	const char *argv[]);

static const os_subcommand_option_t _main_positional[] = {
	[0] = {
		.osco_version = OS_SUBCOMMAND_OPTION_VERSION,
		.osco_flags = 0,
		.osco_option = NULL,
		.osco_argument_usage = "subcommand",
		.osco_argument_human = "The subcommand to invoke",
	},
	OS_SUBCOMMAND_OPTION_TERMINATOR,
};

static const os_subcommand_t __main_cmd = {
	.osc_version = OS_SUBCOMMAND_VERSION,
	.osc_flags = OS_SUBCOMMAND_FLAG_MAIN,
	.osc_name = "_main",
	.osc_desc = "main command",
	.osc_optstring = NULL,
	.osc_options = NULL,
	.osc_required = NULL,
	.osc_optional = NULL,
	.osc_positional = _main_positional,
	.osc_invoke = &_main_invoke,
};

static int
_main_invoke(const os_subcommand_t *osc, int argc, const char *argv[])
{
	return 0;
}

#pragma mark Help Subcommand
static int _help_invoke(const os_subcommand_t *osc,
	int argc,
	const char *argv[]);

static const os_subcommand_option_t _help_positional[] = {
	[0] = {
		.osco_version = OS_SUBCOMMAND_OPTION_VERSION,
		.osco_flags = 0,
		.osco_option = NULL,
		.osco_argument_usage = "subcommand",
		.osco_argument_human = "The subcommand to query for help",
	},
	OS_SUBCOMMAND_OPTION_TERMINATOR,
};

static const os_subcommand_t __help_cmd = {
	.osc_version = OS_SUBCOMMAND_VERSION,
	.osc_flags = 0,
	.osc_name = "help",
	.osc_desc = "prints helpful information",
	.osc_optstring = NULL,
	.osc_options = NULL,
	.osc_required = NULL,
	.osc_optional = NULL,
	.osc_positional = _help_positional,
	.osc_invoke = &_help_invoke,
};

static int
_help_invoke(const os_subcommand_t *osc, int argc, const char *argv[])
{
	int xit = -1;
	const char *cmdname = NULL;
	const os_subcommand_t *target = NULL;
	FILE *f = stdout;

	if (argc > 1) {
		cmdname = argv[1];
	}

	if (cmdname) {
		// Print usage information for the requested subcommand.
		target = _os_subcommand_find(argv[1]);
		if (!target) {
			os_subcommand_fprintf(osc, stderr, "unrecognized subcommand: %s",
					argv[1]);
			xit = EX_USAGE;
		} else {
			xit = 0;
		}
	} else {
		// Print general, top-level usage followed by a list of subcommands.
		target = _main_cmd;
		xit = 0;
	}

	if (xit) {
		f = stderr;
	}

	_os_subcommand_print_usage(target, f);

	if (target == _main_cmd) {
		const os_subcommand_t **oscip = NULL;
		const os_subcommand_t *osci = NULL;
		bool header_printed = false;

		LINKER_SET_FOREACH(oscip, const os_subcommand_t **,
				SUBCOMMAND_LINKER_SET) {
			osci = *oscip;

			_print_header(f, "subcommands", &header_printed);
			_os_subcommand_print_help_line(osci, f);
		}

		// Print the help subcommand last.
		_os_subcommand_print_help_line(osc, f);
	}

	return xit;
}

#pragma mark Utilities
static void
_print_header(FILE *f, const char *hdr, bool *already_done)
{
	if (*already_done) {
		return;
	}

	crfprintf_np(f, "");
	crfprintf_np(f, "%s:", hdr);
	*already_done = true;
}

#pragma mark Module Routines
static char *
_os_subcommand_copy_option_spec_short(const os_subcommand_t *osc,
		const os_subcommand_option_t *osco)
{
	const struct option *opt = osco->osco_option;
	char optbuff[64] = "";
	char argbuff[64] = "";
	char *final = NULL;
	int ret = -1;

	if (opt) {
		snprintf(optbuff, sizeof(optbuff), "-%c", opt->val);

		switch (opt->has_arg) {
		case no_argument:
			break;
		case optional_argument:
			snprintf(argbuff, sizeof(argbuff), "[%s]",
					osco->osco_argument_usage);
			break;
		case required_argument:
			snprintf(argbuff, sizeof(argbuff), "<%s>",
					osco->osco_argument_usage);
			break;
		default:
			__builtin_unreachable();
		}
	} else {
		snprintf(optbuff, sizeof(optbuff), "%s", osco->osco_argument_usage);
	}

	ret = asprintf(&final, "%s%s", optbuff, argbuff);
	if (ret < 0) {
		os_assert_zero(ret);
	}

	return final;
}

static char *
_os_subcommand_copy_option_spec_long(const os_subcommand_t *osc,
		const os_subcommand_option_t *osco)
{
	const struct option *opt = osco->osco_option;
	char optbuff[64] = "";
	char argbuff[64] = "";
	char *final = NULL;
	int ret = -1;

	if (opt) {
		snprintf(optbuff, sizeof(optbuff), "--%s", opt->name);

		switch (opt->has_arg) {
		case no_argument:
			break;
		case optional_argument:
			snprintf(argbuff, sizeof(argbuff), "[=%s]",
					osco->osco_argument_usage);
			break;
		case required_argument:
			snprintf(argbuff, sizeof(argbuff), "=<%s>",
					osco->osco_argument_usage);
			break;
		default:
			__builtin_unreachable();
		}
	} else {
		snprintf(optbuff, sizeof(optbuff), "%s", osco->osco_argument_usage);
	}

	ret = asprintf(&final, "%s%s", optbuff, argbuff);
	if (ret < 0) {
		os_assert_zero(ret);
	}

	return final;
}

static char *
_os_subcommand_copy_option_spec(const os_subcommand_t *osc,
		const os_subcommand_option_t *osco, os_subcommand_option_spec_fmt_t fmt)
{
	int ret = -1;
	char *spec = NULL;
	char *__os_free spec_old = NULL;

	switch (fmt) {
	case OS_SUBCOMMAND_OPTION_SPEC_SHORT:
		_os_subcommand_copy_option_spec_short(osc, osco);
		break;
	case OS_SUBCOMMAND_OPTION_SPEC_LONG:
		_os_subcommand_copy_option_spec_long(osc, osco);
		break;
	case OS_SUBCOMMAND_OPTION_SPEC_COMBINED:
		spec = _os_subcommand_copy_option_spec_long(osc, osco);
		if (osco->osco_option) {
			spec_old = spec;

			ret = asprintf(&spec, "%s | -%c", spec, osco->osco_option->val);
			if (ret < 0) {
				os_assert_zero(ret);
			}
		}

		break;
	default:
		__builtin_unreachable();
	}

	return spec;
}

static char *
_os_subcommand_copy_usage_line(const os_subcommand_t *osc)
{
	char *usage_line = NULL;
	size_t i = 0;
	const os_subcommand_option_t *osco_i = NULL;
	const char *optional_spec = "";
	char subcmd_name[64];
	int ret = -1;

	// The usage line does not enumerate all possible optional options, just the
	// required options. If there are optional options, then display that but
	// otherwise leave them to be described by more extensive usage information.
	if (osc->osc_optional) {
		optional_spec = " [options]";
	}

	if (osc == _main_cmd) {
		strlcpy(subcmd_name, "", sizeof(subcmd_name));
	} else {
		snprintf(subcmd_name, sizeof(subcmd_name), " %s", osc->osc_name);
	}

	ret = asprintf(&usage_line, "%s%s%s",
			getprogname(), subcmd_name, optional_spec);
	if (ret < 0) {
		os_assert_zero(ret);
	}

	i = 0;
	OS_SUBCOMMAND_OPTIONS_FOREACH(osco_i, osc, required, i) {
		char *__os_free usage_line_old = NULL;
		char *__os_free osco_spec = NULL;

		usage_line_old = usage_line;

		osco_spec = _os_subcommand_copy_option_spec_long(osc, osco_i);
		ret = asprintf(&usage_line, "%s %s", usage_line, osco_spec);
		if (ret < 0) {
			os_assert_zero(ret);
		}
	}

	i = 0;
	OS_SUBCOMMAND_OPTIONS_FOREACH(osco_i, osc, positional, i) {
		char *__os_free usage_line_old = NULL;
		char *__os_free osco_spec = NULL;
		const char *braces[] = {
			"<",
			">",
		};

		if (osco_i->osco_flags & OS_SUBCOMMAND_OPTION_FLAG_OPTIONAL_POS) {
			braces[0] = "[";
			braces[1] = "]";
		}

		usage_line_old = usage_line;

		osco_spec = _os_subcommand_copy_option_spec_long(osc, osco_i);
		ret = asprintf(&usage_line, "%s %s%s%s",
				usage_line, braces[0], osco_spec, braces[1]);
		if (ret < 0) {
			os_assert_zero(ret);
		}
	}

	if (osc == _main_cmd && osc != _internal_main_cmd) {
		// Always include the positional subcommand when printing usage for the
		// main subcommand. We do not expect it to be specified in a user-
		// provided main subcommand.
		char *__os_free usage_line_old = NULL;
		char *__os_free osco_spec = NULL;

		usage_line_old = usage_line;

		osco_spec = _os_subcommand_copy_option_spec_long(osc,
				&_main_positional[0]);
		ret = asprintf(&usage_line, "%s <%s>", usage_line, osco_spec);
		if (ret < 0) {
			os_assert_zero(ret);
		}
	}

	return usage_line;
}

static void
_os_subcommand_print_option_usage(const os_subcommand_t *osc,
		const os_subcommand_option_t *osco, FILE *f)
{
	char *__os_free opt_spec = NULL;
	ssize_t initpad = -CTL_OUTPUT_OPTARG_PAD;

	opt_spec = _os_subcommand_copy_option_spec(osc, osco,
			OS_SUBCOMMAND_OPTION_SPEC_COMBINED);
	fprintf(f, "    %-24s", opt_spec);

	// If the usage specifier is long, start the description on the next line.
	if (strlen(opt_spec) >= CTL_OUTPUT_OPTARG_PAD) {
		initpad = CTL_OUTPUT_OPTARG_PAD;
		crfprintf_np(f, "");
	}

	wfprintf_np(f, initpad, CTL_OUTPUT_OPTARG_PAD,
			CTL_OUTPUT_WIDTH, "%s",
			osco->osco_argument_human);
}

static void
_os_subcommand_print_help_line(const os_subcommand_t *osc, FILE *f)
{
	ssize_t initpad = -CTL_OUTPUT_OPTARG_PAD;

	fprintf(f, "    %-24s", osc->osc_name);

	// If the usage specifier is long, start the description on the next line.
	if (strlen(osc->osc_name) >= CTL_OUTPUT_OPTARG_PAD) {
		initpad = CTL_OUTPUT_OPTARG_PAD;
		crfprintf_np(f, "");
	}

	wfprintf_np(f, initpad, CTL_OUTPUT_OPTARG_PAD,
			CTL_OUTPUT_WIDTH, "%s",
			osc->osc_desc);
}

static void
_os_subcommand_print_usage(const os_subcommand_t *osc, FILE *f)
{
	size_t i = 0;
	const os_subcommand_option_t *osco_i = NULL;
	char *__os_free usage_line = NULL;
	bool header_printed = false;

	usage_line = _os_subcommand_copy_usage_line(osc);
	wfprintf_np(f, 0, 0, CTL_OUTPUT_WIDTH, "usage: %s", usage_line);

	if (osc->osc_required || osc->osc_positional || osc == _main_cmd) {
		i = 0;
		OS_SUBCOMMAND_OPTIONS_FOREACH(osco_i, osc, required, i) {
			_print_header(f, "required", &header_printed);

			crfprintf_np(f, "");
			_os_subcommand_print_option_usage(osc, osco_i, f);
		}

		i = 0;
		OS_SUBCOMMAND_OPTIONS_FOREACH(osco_i, osc, positional, i) {
			_print_header(f, "required", &header_printed);

			if (osco_i->osco_flags & OS_SUBCOMMAND_OPTION_FLAG_OPTIONAL_POS) {
				continue;
			}

			crfprintf_np(f, "");
			_os_subcommand_print_option_usage(osc, osco_i, f);
		}

		if (osc == _main_cmd && osc != _internal_main_cmd) {
			// We do not expect the user's main command to specify that a
			// subcommand must follow, so always defer to ours.
			_print_header(f, "required", &header_printed);

			crfprintf_np(f, "");
			_os_subcommand_print_option_usage(osc, &_main_positional[0], f);
		}
	}

	header_printed = false;

	if (osc->osc_optional || osc->osc_positional) {
		i = 0;
		OS_SUBCOMMAND_OPTIONS_FOREACH(osco_i, osc, optional, i) {
			_print_header(f, "optional", &header_printed);

			crfprintf_np(f, "");
			_os_subcommand_print_option_usage(osc, osco_i, f);
		}

		i = 0;
		OS_SUBCOMMAND_OPTIONS_FOREACH(osco_i, osc, positional, i) {
			if (osco_i->osco_flags & OS_SUBCOMMAND_OPTION_FLAG_OPTIONAL_POS) {
				_print_header(f, "optional", &header_printed);

				crfprintf_np(f, "");
				_os_subcommand_print_option_usage(osc, osco_i, f);
			}
		}
	}
}

static const os_subcommand_t *
_os_subcommand_find(const char *name)
{
	const os_subcommand_t **oscip = NULL;

	if (strcmp(_help_cmd->osc_name, name) == 0) {
		return &__help_cmd;
	}

	LINKER_SET_FOREACH(oscip, const os_subcommand_t **, SUBCOMMAND_LINKER_SET) {
		const os_subcommand_t *osci = *oscip;

		if (osci->osc_flags & OS_SUBCOMMAND_FLAG_MAIN) {
			// The main subcommand cannot be invoked directly.
			continue;
		}

		if (strcmp(osci->osc_name, name) == 0) {
			return osci;
		}
	}

	return NULL;
}

#pragma mark API
int
os_subcommand_main(int argc, const char *argv[],
		os_subcommand_main_flags_t flags)
{
	int xit = -1;
	const char *cmdname = NULL;
	const os_subcommand_t *osc = NULL;
	const os_subcommand_t **oscip = NULL;

	if (argc < 2) {
		os_subcommand_fprintf(NULL, stderr, "please provide a subcommand");
		xit = EX_USAGE;
		goto __out;
	}

	// Find the main subcommand if any exists. Otherwise we'll just use our pre-
	// canned main subcommand.
	LINKER_SET_FOREACH(oscip, const os_subcommand_t **, SUBCOMMAND_LINKER_SET) {
		osc = *oscip;
		if (osc->osc_flags & OS_SUBCOMMAND_FLAG_MAIN) {
			_main_cmd = osc;
			break;
		}
	}

	osc = NULL;

	// Invoke the main subcommand to snarf any global options. Our default
	// implementation does nothing and just returns 0.
	xit = _main_cmd->osc_invoke(_main_cmd, argc, argv);
	if (xit) {
		goto __out;
	}

	// Advance argument pointer and make the subcommand argv[0].
	argc -= optind;
	argv += optind;
	cmdname = argv[0];

	if (argc < 1) {
		os_subcommand_fprintf(NULL, stderr, "please provide a subcommand");
		xit = EX_USAGE;
		goto __out;
	}

	osc = _os_subcommand_find(cmdname);
	if (osc) {
		if (osc->osc_flags & OS_SUBCOMMAND_FLAG_REQUIRE_ROOT) {
			if (geteuid()) {
				os_subcommand_fprintf(osc, stderr,
						"subcommand requires root: %s",
						cmdname);
				xit = EX_NOPERM;
				goto __out;
			}
		}

		if (osc->osc_flags & OS_SUBCOMMAND_FLAG_TTYONLY) {
			if (!isatty(STDOUT_FILENO) || !isatty(STDIN_FILENO)) {
				os_subcommand_fprintf(osc, stderr,
						"subcommand requires a tty: %s",
						cmdname);
				xit = EX_UNAVAILABLE;
				goto __out;
			}
		}

		xit = osc->osc_invoke(osc, argc, argv);
	} else {
		os_subcommand_fprintf(NULL, stderr, "unknonwn subcommand: %s", cmdname);
		xit = EX_USAGE;
	}

__out:
	if (xit == EX_USAGE) {
		if (!osc) {
			osc = _main_cmd;
		}

		_os_subcommand_print_usage(osc, stderr);
	}

	return xit;
}

void
os_subcommand_fprintf(const os_subcommand_t *osc, FILE *f,
		const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vcrfprintf_np(f, fmt, ap);
	va_end(ap);
}

void
os_subcommand_vfprintf(const os_subcommand_t *osc, FILE *f,
		const char *fmt, va_list ap)
{
	if (!osc || (osc->osc_flags & OS_SUBCOMMAND_FLAG_MAIN)) {
		fprintf(f, "%s: ", getprogname());
	} else {
		fprintf(f, "%s::%s: ", getprogname(), osc->osc_name);
	}

	vcrfprintf_np(f, fmt, ap);
}
