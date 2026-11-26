/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <DataDetectors/DDActionContext.h>

// The context of a data detector action whose details stay with the system rather than the
// app. On macOS 26.7.3 it is a subclass of DDActionContext; WebKit checks objects against it
// when it encodes them for IPC. Test: tests/datadetectors-secure-action-context.m in Darling's
// main repository.
@interface DDSecureActionContext : DDActionContext

@end
