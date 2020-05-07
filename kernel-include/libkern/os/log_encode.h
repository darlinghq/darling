/*
 * Copyright (c) 2015-2016 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef log_encode_h
#define log_encode_h

#include "log_encode_types.h"
#include <sys/param.h>

#if __has_feature(ptrauth_calls)
#include <mach/vm_param.h>
#include <ptrauth.h>
#endif /* __has_feature(ptrauth_calls) */

#ifdef KERNEL
#define isdigit(ch) (((ch) >= '0') && ((ch) <= '9'))
extern boolean_t doprnt_hide_pointers;
#endif

static bool
_encode_data(os_log_buffer_value_t content, const void *arg, uint16_t arg_len, os_log_buffer_context_t context)
{
	struct os_log_arginfo_s arginfo;
	void *databuf;

	if (content->flags & OS_LOG_CONTENT_FLAG_PRIVATE) {
		databuf = context->privdata + context->privdata_off;
		arginfo.length = MIN(arg_len, (context->privdata_sz - context->privdata_off));
		arginfo.offset = context->privdata_off;
	} else {
		databuf = context->pubdata + context->pubdata_off;
		arginfo.length = MIN(arg_len, (context->pubdata_sz - context->pubdata_off));
		arginfo.offset = context->pubdata_off;
	}

	if (context->arg_content_sz > 0) {
		arginfo.length = MIN(context->arg_content_sz, arginfo.length);
	}

	memcpy(content->value, &arginfo, sizeof(arginfo));
	content->size = sizeof(arginfo);

	if (arginfo.length) {
		if (content->type == OS_LOG_BUFFER_VALUE_TYPE_STRING
#ifndef KERNEL
		    || content->type == OS_LOG_BUFFER_VALUE_TYPE_OBJECT
#endif
		    ) {
			strlcpy(databuf, arg, arginfo.length);
		} else {
			memcpy(databuf, arg, arginfo.length);
		}
	}

	if (content->flags & OS_LOG_CONTENT_FLAG_PRIVATE) {
		context->privdata_off += arginfo.length;
	} else {
		context->pubdata_off += arginfo.length;
	}

	context->content_off += sizeof(*content) + content->size;
	context->arg_content_sz = 0;

	return true;
}

#ifndef KERNEL
static void
_os_log_parse_annotated(char *annotated, const char **visibility, const char **library, const char **type)
{
	char *values[3] = { NULL };
	int cnt = 0;
	int idx = 0;

	for (; cnt < 3;) {
		char *token = strsep(&annotated, ", {}");
		if (token == NULL) {
			break;
		}

		if (*token == '\0') {
			continue;
		}

		values[cnt++] = token;
	}

	if ((cnt > 0) && (!strcmp(values[0], "public") || !strcmp(values[0], "private"))) {
		if (visibility != NULL) {
			(*visibility) = values[0];
		}

		idx++;
	}

	if (idx < cnt && (library != NULL) && (type != NULL)) {
		char *decoder = values[idx];

		for (cnt = 0; cnt < 3;) {
			char *token = strsep(&decoder, ": {}");
			if (token == NULL) {
				break;
			}

			if (*token == '\0') {
				continue;
			}

			values[cnt++] = token;
		}

		if (cnt == 2) {
			(*library) = values[0];
			(*type) = values[1];
		}

		if (cnt == 1) {
			(*library) = "builtin";
			(*type) = values[0];
		}
	}
}
#endif /* !KERNEL */

OS_ALWAYS_INLINE
static inline bool
_os_log_encode_arg(void *arg, uint16_t arg_len, os_log_value_type_t ctype, bool is_private, os_log_buffer_context_t context)
{
	os_log_buffer_value_t content = (os_log_buffer_value_t) &context->buffer->content[context->content_off];
	size_t content_sz = sizeof(*content) + arg_len;
	char tempString[OS_LOG_BUFFER_MAX_SIZE] = {};
#ifndef KERNEL
	bool obj_private = true;
#endif

#ifdef KERNEL
	/* scrub kernel pointers */
	if (doprnt_hide_pointers &&
	    ctype == OS_LOG_BUFFER_VALUE_TYPE_SCALAR &&
	    arg_len >= sizeof(void *)) {
		unsigned long long value = 0;
		memcpy(&value, arg, arg_len);

#if __has_feature(ptrauth_calls)
		/**
		 * Strip out the pointer authentication code before
		 * checking whether the pointer is a kernel address.
		 */
		value = (unsigned long long)VM_KERNEL_STRIP_PTR(value);
#endif /* __has_feature(ptrauth_calls) */

		if (value >= VM_MIN_KERNEL_AND_KEXT_ADDRESS && value <= VM_MAX_KERNEL_ADDRESS) {
			is_private = true;
			bzero(arg, arg_len);
		}
	}
#endif

	content->type = ctype;
	content->flags = (is_private ? OS_LOG_CONTENT_FLAG_PRIVATE : 0);

#ifndef KERNEL
	if (context->annotated != NULL) {
		const char *visibility = NULL;

		_os_log_parse_annotated(context->annotated, &visibility, NULL, NULL);
		if (visibility) {
			if (!strcasecmp(visibility, "private")) {
				content->flags |= OS_LOG_CONTENT_FLAG_PRIVATE;
			} else if (!strcasecmp(visibility, "public")) {
				content->flags &= ~OS_LOG_CONTENT_FLAG_PRIVATE;
			}
		}

		context->annotated = NULL;
	}
#endif /* !KERNEL */

	switch (ctype) {
	case OS_LOG_BUFFER_VALUE_TYPE_COUNT:
	case OS_LOG_BUFFER_VALUE_TYPE_SCALAR:
		if (is_private) {
			_encode_data(content, tempString, strlen(tempString) + 1, context);
		} else {
			if ((context->content_off + content_sz) > context->content_sz) {
				return false;
			}

			memcpy(content->value, arg, arg_len);
			content->size = arg_len;
			context->content_off += content_sz;
		}
		break;

	case OS_LOG_BUFFER_VALUE_TYPE_STRING:
		context->buffer->flags |= OS_LOG_BUFFER_HAS_NON_SCALAR;
		if (_os_log_string_is_public(arg)) {
			content->flags &= ~OS_LOG_CONTENT_FLAG_PRIVATE;
		}

		_encode_data(content, arg, arg_len, context);
		break;

#ifndef KERNEL
	case OS_LOG_BUFFER_VALUE_TYPE_POINTER:
		context->buffer->flags |= OS_LOG_BUFFER_HAS_NON_SCALAR;
		_encode_data(content, arg, arg_len, context);
		break;

	case OS_LOG_BUFFER_VALUE_TYPE_OBJECT:
		context->buffer->flags |= OS_LOG_BUFFER_HAS_NON_SCALAR;
		if (!_NSCF2data(arg, tempString, sizeof(tempString), &obj_private)) {
			tempString[0] = '\0';
		}

		if (!obj_private) {
			content->flags &= ~OS_LOG_CONTENT_FLAG_PRIVATE;
		}

		_encode_data(content, tempString, strlen(tempString) + 1, context);
		break;
#endif /* !KERNEL */
	}

	if (content->flags & OS_LOG_CONTENT_FLAG_PRIVATE) {
		context->buffer->flags |= OS_LOG_BUFFER_HAS_PRIVATE;
	}

	context->arg_idx++;

	return true;
}

static bool
_os_log_encode(const char *format, va_list args, int saved_errno, os_log_buffer_context_t context)
{
	const char *percent = strchr(format, '%');
#ifndef KERNEL
	char annotated[256];
#endif

	while (percent != NULL) {
		++percent;
		if (percent[0] != '%') {
			struct os_log_format_value_s value;
			int     type = OST_INT;
#ifndef KERNEL
			bool    long_double = false;
#endif
			int     prec = 0;
			char    ch;

			for (bool done = false; !done; percent++) {
				switch (ch = percent[0]) {
				/* type of types or other */
				case 'l': // longer
					type++;
					break;

				case 'h': // shorter
					type--;
					break;

				case 'z':
					type = OST_SIZE;
					break;

				case 'j':
					type = OST_INTMAX;
					break;

				case 't':
					type = OST_PTRDIFF;
					break;

				case '.': // precision
					if ((percent[1]) == '*') {
						prec = va_arg(args, int);
						_os_log_encode_arg(&prec, sizeof(prec), OS_LOG_BUFFER_VALUE_TYPE_COUNT, false, context);
						percent++;
						continue;
					} else {
						// we have to read the precision and do the right thing
						const char *fmt = percent + 1;
						prec = 0;
						while (isdigit(ch = *fmt++)) {
							prec = 10 * prec + (ch - '0');
						}

						if (prec > 1024) {
							prec = 1024;
						}

						_os_log_encode_arg(&prec, sizeof(prec), OS_LOG_BUFFER_VALUE_TYPE_COUNT, false, context);
					}
					break;

				case '-': // left-align
				case '+': // force sign
				case ' ': // prefix non-negative with space
				case '#': // alternate
				case '\'': // group by thousands
					break;

				/* fixed types */
				case 'd': // integer
				case 'i': // integer
				case 'o': // octal
				case 'u': // unsigned
				case 'x': // hex
				case 'X': // upper-hex
					switch (type) {
					case OST_CHAR:
						value.type.ch = va_arg(args, int);
						_os_log_encode_arg(&value.type.ch, sizeof(value.type.ch), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
						break;

					case OST_SHORT:
						value.type.s = va_arg(args, int);
						_os_log_encode_arg(&value.type.s, sizeof(value.type.s), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
						break;

					case OST_INT:
						value.type.i = va_arg(args, int);
						_os_log_encode_arg(&value.type.i, sizeof(value.type.i), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
						break;

					case OST_LONG:
						value.type.l = va_arg(args, long);
						_os_log_encode_arg(&value.type.l, sizeof(value.type.l), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
						break;

					case OST_LONGLONG:
						value.type.ll = va_arg(args, long long);
						_os_log_encode_arg(&value.type.ll, sizeof(value.type.ll), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
						break;

					case OST_SIZE:
						value.type.z = va_arg(args, size_t);
						_os_log_encode_arg(&value.type.z, sizeof(value.type.z), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
						break;

					case OST_INTMAX:
						value.type.im = va_arg(args, intmax_t);
						_os_log_encode_arg(&value.type.im, sizeof(value.type.im), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
						break;

					case OST_PTRDIFF:
						value.type.pd = va_arg(args, ptrdiff_t);
						_os_log_encode_arg(&value.type.pd, sizeof(value.type.pd), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
						break;

					default:
						return false;
					}
					done = true;
					break;

#ifndef KERNEL
				case '{':
					// we do not support this for shimmed code
					if (context->shimmed) {
						return false;
					}

					for (const char *curr2 = percent + 1; (ch = (*curr2)) != NUL; curr2++) {
						if (ch == '}') {
							strlcpy(annotated, percent, MIN(curr2 - (percent + 1), sizeof(annotated)));
							context->annotated = annotated;
							percent = curr2;
							break;
						}
					}
					break;
#endif /* !KERNEL */

				case 'p': // pointer
					value.type.p = va_arg(args, void *);
					_os_log_encode_arg(&value.type.p, sizeof(value.type.p), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
					done = true;
					break;

#ifndef KERNEL
				case 'P': // pointer data
					if (context->shimmed) { // we do not support this for shimmed code
						return false;
					}

					context->buffer->flags |= OS_LOG_BUFFER_HAS_NON_SCALAR;
					value.type.p = va_arg(args, void *);

					// capture the string pointer to generate a symptom
					if (context->log && context->log->generate_symptoms && context->arg_idx == 1 && value.type.pch && prec) {
						context->symptom_ptr = value.type.p;
						context->symptom_ptr_len = prec;
					}

					_os_log_encode_arg(value.type.p, prec, OS_LOG_BUFFER_VALUE_TYPE_POINTER, false, context);
					prec = 0;
					done = true;
					break;
#endif /* !KERNEL */

#ifndef KERNEL
				case 'L': // long double
					long_double = true;
					break;

				case 'a': case 'A': case 'e': case 'E': // floating types
				case 'f': case 'F': case 'g': case 'G':
					if (long_double) {
						value.type.ld = va_arg(args, long double);
						_os_log_encode_arg(&value.type.ld, sizeof(value.type.ld), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
					} else {
						value.type.d = va_arg(args, double);
						_os_log_encode_arg(&value.type.d, sizeof(value.type.d), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
					}
					done = true;
					break;
#endif /* !KERNEL */

				case 'c': // char
					value.type.ch = va_arg(args, int);
					_os_log_encode_arg(&value.type.ch, sizeof(value.type.ch), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
					done = true;
					break;

#ifndef KERNEL
				case 'C': // wide-char
					value.type.wch = va_arg(args, wint_t);
					_os_log_encode_arg(&value.type.wch, sizeof(value.type.wch), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
					done = true;
					break;
#endif /* !KERNEL */

				case 's': // string
					value.type.pch = va_arg(args, char *);
					if (!prec && value.type.pch != NULL) {
						prec = (int) strlen(value.type.pch) + 1;
					}

#ifndef KERNEL
					// capture the string pointer to generate a symptom
					if (context->log && context->log->generate_symptoms && context->arg_idx == 0 && value.type.pch) {
						context->symptom_str = value.type.pch;
					}
#endif

					context->buffer->flags |= OS_LOG_BUFFER_HAS_NON_SCALAR;
					_os_log_encode_arg(value.type.pch, prec, OS_LOG_BUFFER_VALUE_TYPE_STRING, false, context);
					prec = 0;
					done = true;
					break;

#ifndef KERNEL
				case 'S': // wide-string
					value.type.pwch = va_arg(args, wchar_t *);
					if (!prec && value.type.pwch != NULL) {
						prec = (int) wcslen(value.type.pwch) + 1;
					}

					context->buffer->flags |= OS_LOG_BUFFER_HAS_NON_SCALAR;
					_os_log_encode_arg(value.type.pwch, prec, OS_LOG_BUFFER_VALUE_TYPE_STRING, false, context);
					prec = 0;
					done = true;
					break;
#endif /* !KERNEL */

#ifndef KERNEL
				case '@': // CFTypeRef aka NSObject *
					context->buffer->flags |= OS_LOG_BUFFER_HAS_NON_SCALAR;
					_os_log_encode_arg(va_arg(args, void *), 0, OS_LOG_BUFFER_VALUE_TYPE_OBJECT, false, context);
					done = true;
					break;
#endif /* !KERNEL */

				case 'm':
					value.type.i = saved_errno;
					_os_log_encode_arg(&value.type.i, sizeof(value.type.i), OS_LOG_BUFFER_VALUE_TYPE_SCALAR, false, context);
					done = true;
					break;

				default:
					if (isdigit(ch)) { // [0-9]
						continue;
					}
					return false;
				}

				if (done) {
					percent = strchr(percent, '%'); // Find next format
					break;
				}
			}
		} else {
			percent = strchr(percent + 1, '%'); // Find next format after %%
		}
	}

	context->buffer->arg_cnt = context->arg_idx;
	context->content_sz = context->content_off;
	context->pubdata_sz = context->pubdata_off;
	context->privdata_sz = context->privdata_off;
	context->arg_idx = context->content_off = context->pubdata_off = context->privdata_off = 0;

	return true;
}

#endif /* log_encode_h */
