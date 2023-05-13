/*
 * This file is part of Darling.
 *
 * Copyright (C) 2023 Darling Developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <objc/NSObject.h>

@interface NSString : NSObject
@end

#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void __simple_kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define LOG_FUNC __simple_kprintf

NSString *const NSAffectedObjectsErrorKey = @"NSAffectedObjectsErrorKey";
NSString *const NSAffectedStoresErrorKey = @"NSAffectedStoresErrorKey";
NSString *const NSDeletedObjectsKey = @"NSDeletedObjectsKey";
NSString *const NSDetailedErrorsKey = @"NSDetailedErrorsKey";
NSString *const NSInferMappingModelAutomaticallyOption = @"NSInferMappingModelAutomaticallyOption";
NSString *const NSInsertedObjectsKey = @"NSInsertedObjectsKey";
NSString *const NSInvalidatedObjectsKey = @"NSInvalidatedObjectsKey";
NSString *const NSManagedObjectContextDidSaveNotification = @"NSManagedObjectContextDidSaveNotification";
NSString *const NSMergeByPropertyStoreTrumpMergePolicy = @"NSMergeByPropertyStoreTrumpMergePolicy";
NSString *const NSMigratePersistentStoresAutomaticallyOption = @"NSMigratePersistentStoresAutomaticallyOption";
NSString *const NSOverwriteMergePolicy = @"NSOverwriteMergePolicy";
NSString *const NSPersistentStoreSaveConflictsErrorKey = @"NSPersistentStoreSaveConflictsErrorKey";
NSString *const NSRefreshedObjectsKey = @"NSRefreshedObjectsKey";
NSString *const NSRollbackMergePolicy = @"NSRollbackMergePolicy";
NSString *const NSSQLiteStoreType = @"NSSQLiteStoreType";
NSString *const NSStoreModelVersionHashesKey = @"NSStoreModelVersionHashesKey";
NSString *const NSUpdatedObjectsKey = @"NSUpdatedObjectsKey";
NSString *const NSValidationKeyErrorKey = @"NSValidationKeyErrorKey";
NSString *const NSValidationObjectErrorKey = @"NSValidationObjectErrorKey";
NSString *const NSValidationPredicateErrorKey = @"NSValidationPredicateErrorKey";
NSString *const NSValidationValueErrorKey = @"NSValidationValueErrorKey";
NSString *const NSXMLStoreType = @"NSXMLStoreType";


