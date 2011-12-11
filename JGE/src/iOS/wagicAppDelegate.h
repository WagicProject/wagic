#import <UIKit/UIKit.h>
#import "Reachability.h"
#import "WagicDownloadProgressViewController.h"
@class EAGLViewController;

@interface wagicAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    EAGLViewController *glViewController;
    //Reachability variables
    Reachability* hostReach;
    Reachability* internetReach;
    Reachability* wifiReach;

}
- (void) rotateBackgroundImage:(UIInterfaceOrientation)fromInterfaceOrientation toInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation;

- (void) handleWEngineCommand:(NSString *) command;

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) EAGLViewController *glViewController;
@property (nonatomic, retain) WagicDownloadProgressViewController *wagicDownloadController;
@property (nonatomic, retain) Reachability *hostReach, *internetReach, *wifiReach;

@end

