/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __OS_INTERNAL_CRASHLOG__
#define __OS_INTERNAL_CRASHLOG__

#ifndef __OS_EXPOSE_INTERNALS_INDIRECT__
/*
 * XXX                           /!\ WARNING /!\                           XXX
 *
 * This header file describes INTERNAL interfaces to libplatform used by other
 * libsystem targets, which are subject to change in future releases of OS X
 * and iOS. Any applications relying on these interfaces WILL break.
 *
 * If you are not a libsystem target, you should NOT EVER use these headers.
 * Not even a little.
 *
 * XXX                           /!\ WARNING /!\                           XXX
 */
#error "Please #include <os/internal/internal_shared.h> instead of this file directly."
#else


#define _os_set_crash_log_cause_and_message(ac, msg) ((void)(ac), (void)(msg))
#define _os_set_crash_log_message(msg) ((void)(msg))
#define _os_set_crash_log_message_dynamic(msg) ((void)(msg))


#endif // __OS_EXPOSE_INTERNALS_INDIRECT__

#endif // __OS_INTERNAL_CRASHLOG__
