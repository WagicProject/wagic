#import <UIKit/UIKit.h>
#import "AdWhirlDelegateProtocol.h"

@interface EAGLViewController : UIViewController<UITextFieldDelegate> {

    BOOL bannerIsVisible;
}

- (void)addOkButtonListener: (CGRect) frame;

@property (nonatomic, retain) id eaglView;
@property (nonatomic, retain) UITextField *inputField;
@property (nonatomic, retain) UIButton *okButtonView;
@property (nonatomic, assign) BOOL bannerIsVisible;

@end
