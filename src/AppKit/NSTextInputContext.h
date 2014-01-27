#import <AppKit/AppKitDefines.h>
#import <AppKit/NSTextInputClient.h>


@class NSEvent;


@interface NSTextInputContext : NSObject {
@private
    id _client;

    NSArray *_inputLocales;

    void *_documentID;
    id _keyBindings;
    id _keyBindingManager;

    NSString *_commandCharacters;

    NSInteger _documentRefcon;

    id _reserved[2];
    id _auxiliary;

    struct {
 unsigned int _acceptsGlyphInfo:1;
        unsigned int _secureInput:1;

 unsigned int _attributedString:1;
 unsigned int _fractionOfDistance:1;
 unsigned int _baselineDelta:1;
 unsigned int _windowLevel:1;
 unsigned int _drawsVertically:1;

 unsigned int _compatibilityMode:1;
 unsigned int _hasTS:1;
 unsigned int _hasLM:1;

 unsigned int _inputLocaleGeneration:16;
 unsigned int _blockTSM:1;
 unsigned int _keyProcess:1;
 unsigned int _preflight:1;
 unsigned int _bindingFound:1;

 unsigned int _reserved:2;
    } _ticFlags;
}



+ (NSTextInputContext *)currentInputContext;



- (id)initWithClient:(id <NSTextInputClient>)theClient;




@property(readonly) id <NSTextInputClient> client;



@property BOOL acceptsGlyphInfo;



@property(copy) NSArray *allowedInputSourceLocales;





- (void)activate;
- (void)deactivate;




- (BOOL)handleEvent:(NSEvent *)theEvent;



- (void)discardMarkedText;



- (void)invalidateCharacterCoordinates;
@property(readonly) NSArray *keyboardInputSources;



@property(copy) NSString *selectedKeyboardInputSource;





+ (NSString *)localizedNameForInputSource:(NSString *)inputSourceIdentifier;
@end




EXTERN_C NSString *NSTextInputContextKeyboardSelectionDidChangeNotification;
