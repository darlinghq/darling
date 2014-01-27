#import <AppKit/NSTextField.h>
#import <AppKit/NSTextFieldCell.h>

@interface NSSecureTextField : NSTextField {

}
@end

@interface NSSecureTextFieldCell : NSTextFieldCell {

    @private
    BOOL _echosBullets;
    BOOL _csMode;
}

- (void)setEchosBullets:(BOOL)flag;
- (BOOL)echosBullets;

@end
