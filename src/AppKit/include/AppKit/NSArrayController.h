#import <AppKit/NSObjectController.h>
#import <Foundation/NSPredicate.h>


@class NSArray, NSIndexSet, NSMutableArray, NSMutableIndexSet;

@interface NSArrayController : NSObjectController {
@private
 void *_reserved4;
 id _rearrangementExtensions;
    NSMutableArray *_temporaryWorkObjects;
    struct __arrayControllerFlags {
        unsigned int _avoidsEmptySelection:1;
        unsigned int _preservesSelection:1;
        unsigned int _selectsInsertedObjects:1;
        unsigned int _alwaysUsesMultipleValuesMarker:1;
        unsigned int _refreshesAllModelObjects:1;
        unsigned int _filterRestrictsInsertion:1;
        unsigned int _overridesArrangeObjects:1;
        unsigned int _overridesDidChangeArrangementCriteria:1;
        unsigned int _explicitlyCannotInsert:1;
        unsigned int _generatedEmptyArray:1;
        unsigned int _isObservingKeyPathsThroughArrangedObjects:1;
        unsigned int _arrangedObjectsIsMutable:1;
        unsigned int _clearsFilterPredicateOnInsertion:1;
        unsigned int _skipSortingAfterFetch:1;
        unsigned int _automaticallyRearrangesObjects:1;
        unsigned int _reservedArrayController:17;
    } _arrayControllerFlags;
    NSUInteger _observedIndexHint;
    NSMutableIndexSet *_selectionIndexes;
    NSMutableArray *_objects;
    NSIndexSet *_cachedSelectedIndexes;
    NSArray *_cachedSelectedObjects;
    NSArray *_arrangedObjects;
}

- (void)rearrangeObjects;

- (void)setAutomaticallyRearrangesObjects:(BOOL)flag;
- (BOOL)automaticallyRearrangesObjects;

- (NSArray *)automaticRearrangementKeyPaths;
- (void)didChangeArrangementCriteria;

- (void)setSortDescriptors:(NSArray *)sortDescriptors;
- (NSArray *)sortDescriptors;

- (void)setFilterPredicate:(NSPredicate *)filterPredicate;
- (NSPredicate *)filterPredicate;



- (void)setClearsFilterPredicateOnInsertion:(BOOL)flag;
- (BOOL)clearsFilterPredicateOnInsertion;

- (NSArray *)arrangeObjects:(NSArray *)objects;
- (id)arrangedObjects;

- (void)setAvoidsEmptySelection:(BOOL)flag;
- (BOOL)avoidsEmptySelection;
- (void)setPreservesSelection:(BOOL)flag;
- (BOOL)preservesSelection;
- (void)setSelectsInsertedObjects:(BOOL)flag;
- (BOOL)selectsInsertedObjects;



- (void)setAlwaysUsesMultipleValuesMarker:(BOOL)flag;
- (BOOL)alwaysUsesMultipleValuesMarker;



- (BOOL)setSelectionIndexes:(NSIndexSet *)indexes;
- (NSIndexSet *)selectionIndexes;
- (BOOL)setSelectionIndex:(NSUInteger)index;
- (NSUInteger)selectionIndex;
- (BOOL)addSelectionIndexes:(NSIndexSet *)indexes;
- (BOOL)removeSelectionIndexes:(NSIndexSet *)indexes;

- (BOOL)setSelectedObjects:(NSArray *)objects;
- (NSArray *)selectedObjects;
- (BOOL)addSelectedObjects:(NSArray *)objects;
- (BOOL)removeSelectedObjects:(NSArray *)objects;

- (void)add:(id)sender;
- (void)remove:(id)sender;
- (void)insert:(id)sender;
- (BOOL)canInsert;
- (void)selectNext:(id)sender;
- (void)selectPrevious:(id)sender;
- (BOOL)canSelectNext;
- (BOOL)canSelectPrevious;

- (void)addObject:(id)object;
- (void)addObjects:(NSArray *)objects;
- (void)insertObject:(id)object atArrangedObjectIndex:(NSUInteger)index;
- (void)insertObjects:(NSArray *)objects atArrangedObjectIndexes:(NSIndexSet *)indexes;
- (void)removeObjectAtArrangedObjectIndex:(NSUInteger)index;
- (void)removeObjectsAtArrangedObjectIndexes:(NSIndexSet *)indexes;
- (void)removeObject:(id)object;
- (void)removeObjects:(NSArray *)objects;

@end
