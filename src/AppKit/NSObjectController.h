#import <AppKit/NSController.h>
#import <AppKit/NSUserInterfaceValidation.h>


@class NSArray, NSPredicate, NSManagedObjectContext, NSFetchRequest, NSError;

@interface NSObjectController : NSController {
@private
    void *_reserved3;
    id _managedProxy;
    struct __objectControllerFlags {
        unsigned int _editable:1;
        unsigned int _automaticallyPreparesContent:1;
        unsigned int _hasLoadedData:1;
        unsigned int _explicitlyCannotAdd:1;
        unsigned int _explicitlyCannotRemove:1;
        unsigned int _isUsingManagedProxy:1;
        unsigned int _hasFetched:1;
        unsigned int _batches:1;
        unsigned int _reservedObjectController:24;
    } _objectControllerFlags;
    NSString *_objectClassName;
    Class _objectClass;
    NSArray *_contentObjectArray;
@protected
    id _content;
    id _objectHandler;
}

- (id)initWithContent:(id)content;

- (void)setContent:(id)content;
- (id)content;

- (id)selection;
- (NSArray *)selectedObjects;

- (void)setAutomaticallyPreparesContent:(BOOL)flag;
- (BOOL)automaticallyPreparesContent;
- (void)prepareContent;

- (void)setObjectClass:(Class)objectClass;
- (Class)objectClass;
- (id)newObject;
- (void)addObject:(id)object;
- (void)removeObject:(id)object;

- (void)setEditable:(BOOL)flag;
- (BOOL)isEditable;
- (void)add:(id)sender;
- (BOOL)canAdd;
- (void)remove:(id)sender;
- (BOOL)canRemove;
- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)item;

@end


@interface NSObjectController (NSManagedController)

- (NSManagedObjectContext *)managedObjectContext;
- (void)setManagedObjectContext:(NSManagedObjectContext *)managedObjectContext;

- (NSString *)entityName;
- (void)setEntityName:(NSString *)entityName;
- (NSPredicate *)fetchPredicate;
- (void)setFetchPredicate:(NSPredicate *)predicate;

- (BOOL)fetchWithRequest:(NSFetchRequest *)fetchRequest merge:(BOOL)merge error:(NSError **)error;

- (void)fetch:(id)sender;


- (void)setUsesLazyFetching:(BOOL)enabled;
- (BOOL)usesLazyFetching;

- (NSFetchRequest *)defaultFetchRequest;


@end
