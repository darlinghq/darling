#import <AppKit/NSDocument.h>
@class NSManagedObjectModel, NSManagedObjectContext, NSMutableDictionary;

@interface NSPersistentDocument : NSDocument {
@private
    NSManagedObjectModel *_managedObjectModel;
    NSManagedObjectContext *_managedObjectContext;
    id _store;
    uintptr_t _pDocFlags;
    id _relatedRequestURLs;
    void *_reserved3;
    void *_reserved4;
}


- (NSManagedObjectContext *)managedObjectContext;
- (void)setManagedObjectContext:(NSManagedObjectContext *)managedObjectContext;

- (id)managedObjectModel;



- (BOOL)configurePersistentStoreCoordinatorForURL:(NSURL *)url ofType:(NSString *)fileType modelConfiguration:(NSString *)configuration storeOptions:(NSDictionary *)storeOptions error:(NSError **)error;


- (NSString *)persistentStoreTypeForFileType:(NSString *)fileType;

- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation originalContentsURL:(NSURL *)absoluteOriginalContentsURL error:(NSError **)error;

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)error;

- (BOOL)revertToContentsOfURL:(NSURL *)inAbsoluteURL ofType:(NSString *)inTypeName error:(NSError **)outError;

@end

@interface NSPersistentDocument (NSDeprecated)


- (BOOL)configurePersistentStoreCoordinatorForURL:(NSURL *)url ofType:(NSString *)fileType error:(NSError **)error;

@end
