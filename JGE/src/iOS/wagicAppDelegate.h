#import <UIKit/UIKit.h>

@class EAGLViewController;

@interface wagicAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    EAGLViewController *glViewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLViewController *glViewController;

@end

