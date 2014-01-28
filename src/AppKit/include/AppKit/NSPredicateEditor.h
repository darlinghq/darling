#import <AppKit/NSRuleEditor.h>

@class NSPredicateTemplate, NSArray;


@interface NSPredicateEditor : NSRuleEditor {
    id _allTemplates;
    NSArray *_rootTrees;
    NSArray *_rootHeaderTrees;
    id _predicateTarget;
    SEL _predicateAction;
    struct {
 unsigned suppressAction:4;
 unsigned reserved:28;
    } _peFlags;
    id _predicateEditorReserved[3];
}



- (void)setRowTemplates:(NSArray *)rowTemplates;



- (NSArray *)rowTemplates;

@end
