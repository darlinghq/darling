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


#ifndef _Collaboration_H_
#define _Collaboration_H_

#import <Foundation/Foundation.h>

#import <Collaboration/NSControlTextEditingDelegate.h>
#import <Collaboration/NSTableViewDataSource.h>
#import <Collaboration/NSTableViewDelegate.h>
#import <Collaboration/CBIdentityAuthority.h>
#import <Collaboration/CBIdentityPicker.h>
#import <Collaboration/CBIdentity.h>
#import <Collaboration/CBUserIdentity.h>
#import <Collaboration/CBGroupIdentity.h>
#import <Collaboration/SidebarDataSource.h>
#import <Collaboration/IdentityPickerInternal.h>
#import <Collaboration/ListObject.h>
#import <Collaboration/IdentityDataSource.h>
#import <Collaboration/CBIdentityQuery.h>

void* SortABGroupsByName(void);
void* SortABPeopleByFirstName(void);
void* SortIdentitiesByFirstName(void);

#endif
