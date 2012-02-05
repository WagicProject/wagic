#import <UIKit/UIKit.h>
#import "AdWhirlDelegateProtocol.h"

@interface EAGLViewController : UIViewController<UITextFieldDelegate> {

    BOOL bannerIsVisible;
}
void pauseGame();
void resumeGame();

@property (nonatomic, retain) id eaglView;
@property (nonatomic, retain) UITextField *inputField;
@property (nonatomic, assign) BOOL bannerIsVisible;

@end
