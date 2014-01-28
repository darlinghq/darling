#import <Foundation/NSObject.h>


@class NSArray, NSMutableArray, NSIndexPath, NSTreeController;


@interface NSTreeNode : NSObject {
    id _childNodesProxy;
    id _representedObject;
    __strong void *_observationInfo;
    id _reserved2;
    NSMutableArray *_childNodes;
    NSTreeNode *_parentNode;
    struct __NSTreeNodeFlags {
 unsigned int ignoreObserving:1;
 unsigned int reserved:31;
    } _NSTreeNodeFlags;
}

+ (id)treeNodeWithRepresentedObject:(id)modelObject;
- (id)initWithRepresentedObject:(id)modelObject;

- (id)representedObject;


- (NSIndexPath *)indexPath;
- (BOOL)isLeaf;

- (NSArray *)childNodes;
- (NSMutableArray *)mutableChildNodes;
- (NSTreeNode *)descendantNodeAtIndexPath:(NSIndexPath *)indexPath;

- (NSTreeNode *)parentNode;


- (void)sortWithSortDescriptors:(NSArray *)sortDescriptors recursively:(BOOL)recursively;

@end
