#import <UIKit/UIKit.h>

@class EAGLViewController;

@interface wagicAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    EAGLViewController *glViewController;
}

- (void) handleWEngineCommand:(NSString *) command;

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) EAGLViewController *glViewController;

@end

