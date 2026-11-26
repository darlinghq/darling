// Copyright (C) 2026 Lubos Dolezel
//
// The Swift spellings of CoreData's renamed Objective-C methods, initializers and enumerators
// (public API; the same file typechecks against the macOS SDK). Typecheck only; the renames
// are the ones in CoreData.apinotes.
import CoreData

func use<T>(_ x: T) {}

use(NSAtomicStore.objectID(for:withReferenceObject:))
use(NSEntityDescription.entity(forEntityName:in:))
use(NSEntityDescription.insertNewObject(forEntityName:into:))
use(NSEntityDescription.relationships(forDestination:))
use(NSManagedObjectContext.registeredObject(for:))
use(NSManagedObjectContext.refresh(_:mergeChanges:))
use(NSManagedObjectModel.entities(forConfigurationName:))
use(NSManagedObjectModel.setEntities(_:forConfigurationName:))
use(NSPersistentStore.setMetadata(_:forPersistentStoreAt:))
use(NSPersistentStore.init(persistentStoreCoordinator:configurationName:at:options:))
use(NSPersistentStoreCoordinator.addPersistentStore(ofType:configurationName:at:options:))
use(NSPersistentStoreCoordinator.remove(_:))
use(NSPersistentStoreCoordinator.metadataForPersistentStore(ofType:at:))
use(NSPersistentStoreCoordinator.setMetadata(_:forPersistentStoreOfType:at:))

let r: NSFetchRequestResultType = .managedObjectResultType
use(r)

let context: NSManagedObjectContext? = NSManagedObjectContext(concurrencyType: .privateQueueConcurrencyType)
context?.performAndWait { }
