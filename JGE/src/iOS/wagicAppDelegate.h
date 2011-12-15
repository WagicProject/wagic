#import <UIKit/UIKit.h>
#import "WagicDownloadProgressViewController.h"
#import "Reachability.h"

@class EAGLViewController;

@interface wagicAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    EAGLViewController *glViewController;
    //Reachability variables
    Reachability  *hostReach, *internetReach, *wifiReach;

}
- (void) rotateBackgroundImage:(UIInterfaceOrientation)fromInterfaceOrientation toInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation;

- (void) handleWEngineCommand:(NSString *) command withParameter: (NSString *) parameter;

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) EAGLViewController *glViewController;
@property (nonatomic, retain) WagicDownloadProgressViewController *wagicDownloadController;
@property (nonatomic, retain) Reachability *hostReach, *internetReach, *wifiReach;

@end

