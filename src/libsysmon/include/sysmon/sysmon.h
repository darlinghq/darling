/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Foundation/Foundation.h>

void* sysmon_copy_process_info(void);
void* sysmon_release(void);
void* sysmon_request_add_attribute(void);
void* sysmon_request_add_attributes(void);
void* sysmon_request_alloc(void);
void* sysmon_request_cancel(void);
void* sysmon_request_create(void);
void* sysmon_request_execute(void);
void* sysmon_request_set_flags(void);
void* sysmon_request_set_interval(void);
void* sysmon_retain(void);
void* sysmon_row_alloc(void);
void* sysmon_row_apply(void);
void* sysmon_row_get_value(void);
void* sysmon_table_alloc(void);
void* sysmon_table_apply(void);
void* sysmon_table_copy_row(void);
void* sysmon_table_get_count(void);
void* sysmon_table_get_row(void);
void* sysmon_table_get_timestamp(void);

#import <sysmon/OS_sysmon_object.h>
#import <sysmon/OS_sysmon_request.h>
#import <sysmon/OS_sysmon_table.h>
#import <sysmon/OS_sysmon_row.h>
