#import <AppKit/NSObjectController.h>
#import <Foundation/NSIndexPath.h>

@class NSTreeNode;


@interface NSTreeController : NSObjectController {
    id _treeControllerReserved1;
    NSArray *_modelObservingKeyPaths;
    id _treeStructureObservers;
    id _arrangedObjects;
    id _rootNode;
    id _selectionIndexPaths;
    struct __treeControllerFlags {
        unsigned int _avoidsEmptySelection:1;
        unsigned int _preservesSelection:1;
        unsigned int _selectsInsertedObjects:1;
        unsigned int _explicitlyCannotInsert:1;
        unsigned int _explicitlyCannotInsertChild:1;
        unsigned int _explicitlyCannotAddChild:1;
        unsigned int _alwaysUsesMultipleValuesMarker:1;
        unsigned int _observingThroughArrangedObjects:1;
        unsigned int _mutatingNodes:1;
        unsigned int _performingFetch:1;
        unsigned int _skipSortingAfterFetch:1;
 unsigned int _usesIdenticalComparisonOfModelObjects:1;
        unsigned int _reservedTreeController:20;
    } _treeControllerFlags;
    NSArray *_selectedObjects;
    NSString *_childrenKeyPath;
    NSString *_countKeyPath;
    NSString *_leafKeyPath;
    NSArray *_sortDescriptors;
}

- (void)rearrangeObjects;


- (id)arrangedObjects;

- (void)setChildrenKeyPath:(NSString *)keyPath;
- (NSString *)childrenKeyPath;
- (void)setCountKeyPath:(NSString *)keyPath;
- (NSString *)countKeyPath;
- (void)setLeafKeyPath:(NSString *)keyPath;
- (NSString *)leafKeyPath;

- (void)setSortDescriptors:(NSArray *)sortDescriptors;
- (NSArray *)sortDescriptors;

- (id)content;
- (void)setContent:(id)content;

- (void)add:(id)sender;
- (void)remove:(id)sender;
- (void)addChild:(id)sender;
- (void)insert:(id)sender;
- (void)insertChild:(id)sender;

- (BOOL)canInsert;
- (BOOL)canInsertChild;
- (BOOL)canAddChild;

- (void)insertObject:(id)object atArrangedObjectIndexPath:(NSIndexPath *)indexPath;
- (void)insertObjects:(NSArray *)objects atArrangedObjectIndexPaths:(NSArray *)indexPaths;
- (void)removeObjectAtArrangedObjectIndexPath:(NSIndexPath *)indexPath;
- (void)removeObjectsAtArrangedObjectIndexPaths:(NSArray *)indexPaths;


- (void)setAvoidsEmptySelection:(BOOL)flag;
- (BOOL)avoidsEmptySelection;
- (void)setPreservesSelection:(BOOL)flag;
- (BOOL)preservesSelection;
- (void)setSelectsInsertedObjects:(BOOL)flag;
- (BOOL)selectsInsertedObjects;
- (void)setAlwaysUsesMultipleValuesMarker:(BOOL)flag;
- (BOOL)alwaysUsesMultipleValuesMarker;



- (NSArray *)selectedObjects;

- (BOOL)setSelectionIndexPaths:(NSArray *)indexPaths;
- (NSArray *)selectionIndexPaths;
- (BOOL)setSelectionIndexPath:(NSIndexPath *)indexPath;
- (NSIndexPath *)selectionIndexPath;
- (BOOL)addSelectionIndexPaths:(NSArray *)indexPaths;
- (BOOL)removeSelectionIndexPaths:(NSArray *)indexPaths;

- (NSArray *)selectedNodes;

- (void)moveNode:(NSTreeNode *)node toIndexPath:(NSIndexPath *)indexPath;
- (void)moveNodes:(NSArray *)nodes toIndexPath:(NSIndexPath *)startingIndexPath;

- (NSString *)childrenKeyPathForNode:(NSTreeNode *)node;
- (NSString *)countKeyPathForNode:(NSTreeNode *)node;
- (NSString *)leafKeyPathForNode:(NSTreeNode *)node;


@end
