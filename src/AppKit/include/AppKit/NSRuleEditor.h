#import <AppKit/AppKitDefines.h>
#import <AppKit/NSControl.h>
@class NSMutableArray, NSIndexSet, NSView, NSPredicate, NSString, NSViewAnimation;
@protocol NSRuleEditorDelegate;

enum {
    NSRuleEditorNestingModeSingle,
    NSRuleEditorNestingModeList,
    NSRuleEditorNestingModeCompound,
    NSRuleEditorNestingModeSimple
};

typedef NSUInteger NSRuleEditorNestingMode;

enum {
    NSRuleEditorRowTypeSimple,
    NSRuleEditorRowTypeCompound
};

typedef NSUInteger NSRuleEditorRowType;

@interface NSRuleEditor : NSControl {
    @private
    id _ruleDataSource;
    id _ruleDelegate;
    NSIndexSet *_draggingRows;
    NSMutableArray *_rowCache;
    NSView *_slicesHolder;
    NSMutableArray *_slices;
    CGFloat _sliceHeight;
    CGFloat _alignmentGridWidth;
    NSInteger _subviewIndexOfDropLine;
    id _dropLineView;
    NSViewAnimation *_currentAnimation;
    id _unused1;
    NSString *_stringsFileName;
    id _standardLocalizer;
    id _headerLocalizer;
    NSPredicate *_predicate;
    NSInteger _nestingMode;
    struct {
 unsigned elideUpdating:1;
 unsigned lastAlternateKeyValue:1;
 unsigned extendedDelegateCalls:1;
 unsigned editable:1;
 unsigned settingSize:1;
 unsigned suppressKeyDown:1;
 unsigned dropWasSuccessful:1;
 unsigned delegateWantsValidation:1;
 unsigned disallowEmpty:1;
 unsigned lastDrewWithFRAppearance:1;
 unsigned allowsEmptyCompoundRows:1;
 unsigned dropChangedRowCount:1;
 unsigned reserved:20;
    } _ruleEditorFlags;
    NSString *_typeKeyPath;
    NSString *_itemsKeyPath;
    NSString *_valuesKeyPath;
    NSString *_subrowsArrayKeyPath;
    Class _rowClass;
    id _boundArrayOwner;
    NSString *_boundArrayKeyPath;
    id _ruleReserved1;
    NSInteger _lastRow;
    id _ruleReserved2;
}




- (void)setDelegate:(id <NSRuleEditorDelegate>)delegate;
- (id <NSRuleEditorDelegate>)delegate;


- (void)setFormattingStringsFilename:(NSString *)stringsFilename;
- (NSString *)formattingStringsFilename;


- (void)setFormattingDictionary:(NSDictionary *)dictionary;
- (NSDictionary *)formattingDictionary;


- (void)reloadCriteria;


- (void)setNestingMode:(NSRuleEditorNestingMode)mode;
- (NSRuleEditorNestingMode)nestingMode;


- (void)setRowHeight:(CGFloat)height;
- (CGFloat)rowHeight;


- (void)setEditable:(BOOL)editable;
- (BOOL)isEditable;


- (void)setCanRemoveAllRows:(BOOL)val;
- (BOOL)canRemoveAllRows;




- (NSPredicate *)predicate;


- (void)reloadPredicate;


- (NSPredicate *)predicateForRow:(NSInteger)row;




- (NSInteger)numberOfRows;


- (NSIndexSet *)subrowIndexesForRow:(NSInteger)rowIndex;


- (NSArray *)criteriaForRow:(NSInteger)row;


- (NSArray *)displayValuesForRow:(NSInteger)row;


- (NSInteger)rowForDisplayValue:(id)displayValue;


- (NSRuleEditorRowType)rowTypeForRow:(NSInteger)rowIndex;


- (NSInteger)parentRowForRow:(NSInteger)rowIndex;




- (void)addRow:(id)sender;


- (void)insertRowAtIndex:(NSInteger)rowIndex withType:(NSRuleEditorRowType)rowType asSubrowOfRow:(NSInteger)parentRow animate:(BOOL)shouldAnimate;


- (void)setCriteria:(NSArray *)criteria andDisplayValues:(NSArray *)values forRowAtIndex:(NSInteger)rowIndex;


- (void)removeRowAtIndex:(NSInteger)rowIndex;


- (void)removeRowsAtIndexes:(NSIndexSet *)rowIndexes includeSubrows:(BOOL)includeSubrows;


- (NSIndexSet *)selectedRowIndexes;


- (void)selectRowIndexes:(NSIndexSet *)indexes byExtendingSelection:(BOOL)extend;




- (void)setRowClass:(Class)rowClass;
- (Class)rowClass;


- (void)setRowTypeKeyPath:(NSString *)keyPath;
- (NSString *)rowTypeKeyPath;


- (void)setSubrowsKeyPath:(NSString *)keyPath;
- (NSString *)subrowsKeyPath;


- (void)setCriteriaKeyPath:(NSString *)keyPath;
- (NSString *)criteriaKeyPath;


- (void)setDisplayValuesKeyPath:(NSString *)keyPath;
- (NSString *)displayValuesKeyPath;

@end


@protocol NSRuleEditorDelegate <NSObject>

@required




- (NSInteger)ruleEditor:(NSRuleEditor *)editor numberOfChildrenForCriterion:(id)criterion withRowType:(NSRuleEditorRowType)rowType;


- (id)ruleEditor:(NSRuleEditor *)editor child:(NSInteger)index forCriterion:(id)criterion withRowType:(NSRuleEditorRowType)rowType;


- (id)ruleEditor:(NSRuleEditor *)editor displayValueForCriterion:(id)criterion inRow:(NSInteger)row;

@optional





- (NSDictionary *)ruleEditor:(NSRuleEditor *)editor predicatePartsForCriterion:(id)criterion withDisplayValue:(id)value inRow:(NSInteger)row;


- (void)ruleEditorRowsDidChange:(NSNotification *)notification;

@end



EXTERN_C NSString * const NSRuleEditorPredicateLeftExpression;
EXTERN_C NSString * const NSRuleEditorPredicateRightExpression;
EXTERN_C NSString * const NSRuleEditorPredicateComparisonModifier;
EXTERN_C NSString * const NSRuleEditorPredicateOptions;
EXTERN_C NSString * const NSRuleEditorPredicateOperatorType;
EXTERN_C NSString * const NSRuleEditorPredicateCustomSelector;

EXTERN_C NSString * const NSRuleEditorPredicateCompoundType;




EXTERN_C NSString *const NSRuleEditorRowsDidChangeNotification;
