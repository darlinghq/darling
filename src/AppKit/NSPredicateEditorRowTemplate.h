#import <Foundation/NSObject.h>


#import <Foundation/NSComparisonPredicate.h>
#import <CoreData/NSAttributeDescription.h>

@class NSPredicate, NSArray, NSEntityDescription;


@interface NSPredicateEditorRowTemplate : NSObject <NSCoding, NSCopying> {
    @private
    NSInteger _templateType;
    NSUInteger _predicateOptions;
    NSUInteger _predicateModifier;
    NSUInteger _leftAttributeType;
    NSUInteger _rightAttributeType;
    NSArray *_views;

    struct {
 unsigned leftIsWildcard:1;
 unsigned rightIsWildcard:1;
 unsigned reserved:30;
    } _ptFlags;

    id _patternReserved;
}



- (double)matchForPredicate:(NSPredicate *)predicate;



- (NSArray *)templateViews;



- (void)setPredicate:(NSPredicate *)predicate;


- (NSPredicate *)predicateWithSubpredicates:(NSArray *)subpredicates;



- (NSArray *)displayableSubpredicatesOfPredicate:(NSPredicate *)predicate;



- (id)initWithLeftExpressions:(NSArray *)leftExpressions rightExpressions:(NSArray *)rightExpressions modifier:(NSComparisonPredicateModifier)modifier operators:(NSArray *)operators options:(NSUInteger)options;



- (id)initWithLeftExpressions:(NSArray *)leftExpressions rightExpressionAttributeType:(NSAttributeType)attributeType modifier:(NSComparisonPredicateModifier)modifier operators:(NSArray *)operators options:(NSUInteger)options;



- (id)initWithCompoundTypes:(NSArray *)compoundTypes;



- (NSArray *)leftExpressions;
- (NSArray *)rightExpressions;
- (NSAttributeType)rightExpressionAttributeType;
- (NSComparisonPredicateModifier)modifier;
- (NSArray *)operators;
- (NSUInteger)options;
- (NSArray *)compoundTypes;



+ (NSArray *)templatesWithAttributeKeyPaths:(NSArray *)keyPaths inEntityDescription:(NSEntityDescription *)entityDescription;

@end
