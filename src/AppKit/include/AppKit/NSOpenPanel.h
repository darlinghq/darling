#import <AppKit/NSSavePanel.h>
@class NSArray;
@class NSString;
@class NSWindow;

@interface NSOpenPanel : NSSavePanel {
@private
    char _reservedOpenPanel[4];
    void *_privateOpenPanel;
}



+ (NSOpenPanel *)openPanel;

- (NSArray *)URLs;

- (BOOL)resolvesAliases;
- (void)setResolvesAliases:(BOOL)flag;

- (BOOL)canChooseDirectories;
- (void)setCanChooseDirectories:(BOOL)flag;

- (BOOL)allowsMultipleSelection;
- (void)setAllowsMultipleSelection:(BOOL)flag;

- (BOOL)canChooseFiles;
- (void)setCanChooseFiles:(BOOL)flag;

@end

@interface NSOpenPanel (NSDeprecated)



- (NSArray *)filenames;





- (void)beginSheetForDirectory:(NSString *)path file:(NSString *)name types:(NSArray *)fileTypes modalForWindow:(NSWindow *)docWindow modalDelegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;





- (void)beginForDirectory:(NSString *)path file:(NSString *)name types:(NSArray *)fileTypes modelessDelegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;





- (NSInteger)runModalForDirectory:(NSString *)path file:(NSString *)name types:(NSArray *)fileTypes;




- (NSInteger)runModalForTypes:(NSArray *)fileTypes;

@end
