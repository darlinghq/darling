/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// The OpenDirectory classes wrap the CFOpenDirectory objects that do the
// work, so a node, record or query behaves the same whichever API an
// application uses.

#import <OpenDirectory/OpenDirectory.h>
#import <CFOpenDirectory/CFOpenDirectory.h>

// Hands a CF error to an NSError out-parameter (consuming it)
__attribute__((visibility("hidden")))
void ODSetError(NSError **outError, CFErrorRef error);

@interface ODSession (Internal)
- (ODSessionRef)_sessionRef;
@end

@interface ODNode (Internal)
- (ODNodeRef)_nodeRef;
@end

@interface ODRecord (Internal)
// Wraps (and retains) a CF record
+ (ODRecord *)_recordWithRecordRef:(ODRecordRef)record;
// Wraps every CF record in the array
+ (NSArray *)_recordsWithRecordRefs:(CFArrayRef)records;
@end
