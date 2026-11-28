/*
 This file is part of Darling.

 Copyright (C) 2026 Darling developers

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

#ifndef _BAPLACEHOLDER_H_
#define _BAPLACEHOLDER_H_

#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>

// Shared by the placeholder implementations; not installed.

#define BA_STUB_CALLED() NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class])

// Runs the block once, asynchronously, on a global queue, so a handler is never called before the method that
// takes it has returned.
static inline void BAPlaceholderAsync(dispatch_block_t block)
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), block);
}

#endif
