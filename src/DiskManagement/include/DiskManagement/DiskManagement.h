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

void* DMLocalizedUserErrorString(void);
void* DMUnlocalizedTechnicalErrorString(void);

#import <Foundation/Foundation.h>
#import <DiskManagement/_DMListDA.h>
#import <DiskManagement/_DMClaimDA.h>
#import <DiskManagement/_DMWaitIdleDA.h>
#import <DiskManagement/DMUDSWrapper.h>
#import <DiskManagement/DMFilesystemPrivVars.h>
#import <DiskManagement/DMFilesystem.h>
#import <DiskManagement/DMManagerPrivVars.h>
#import <DiskManagement/DMManager.h>
#import <DiskManagement/DMPartitionDiskPrivVars.h>
#import <DiskManagement/DMPartitionDisk.h>
#import <DiskManagement/DMEraseDiskPrivVars.h>
#import <DiskManagement/DMEraseDisk.h>
#import <DiskManagement/DMRepairDiskPrivVars.h>
#import <DiskManagement/DMRepairDisk.h>
#import <DiskManagement/DMAPFSPrivVars.h>
#import <DiskManagement/DMAPFS.h>
#import <DiskManagement/DMCoreStoragePrivVars.h>
#import <DiskManagement/DMCoreStorage.h>
#import <DiskManagement/DMAppleRAIDPrivVars.h>
#import <DiskManagement/DMAppleRAID.h>
#import <DiskManagement/DMDisk.h>
