#import <UIKit/UIKit.h>
#import "AdWhirlDelegateProtocol.h"

@interface EAGLViewController : UIViewController {

    BOOL bannerIsVisible;
}

@property (nonatomic, assign) BOOL bannerIsVisible;
@property (nonatomic, retain) id   eaglView;
@end
