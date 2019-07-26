/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#ifndef _OpenDirectory_H_
#define _OpenDirectory_H_

#import <Foundation/Foundation.h>

typedef NSString *ODRecordType;
typedef NSString *ODAttributeType;

extern const ODAttributeType kODAttributeTypeRecordName;
extern const ODAttributeType kODAttributeTypeStandardOnly;
extern const ODAttributeType kODAttributeTypeUserShell;

extern const ODRecordType kODRecordTypeUsers;

#import <OpenDirectory/ODQueryDelegate.h>
#import <OpenDirectory/NSODSession.h>
#import <OpenDirectory/ODSession.h>
#import <OpenDirectory/NSODNode.h>
#import <OpenDirectory/ODNode.h>
#import <OpenDirectory/NSODRecord.h>
#import <OpenDirectory/ODRecord.h>
#import <OpenDirectory/NSODQuery.h>
#import <OpenDirectory/ODQuery.h>
#import <OpenDirectory/NSODContext.h>
#import <OpenDirectory/ODContext.h>
#import <OpenDirectory/ODConfiguration.h>
#import <OpenDirectory/ODModuleEntry.h>
#import <OpenDirectory/ODMappings.h>
#import <OpenDirectory/ODAttributeMap.h>
#import <OpenDirectory/ODRecordMap.h>

void* ODTrustInfoCopy(void);

#endif
