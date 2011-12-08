#import "wagicAppDelegate.h"
#import "EAGLView.h"
#import "EAGLViewController.h"


@implementation wagicAppDelegate

@synthesize window;
@synthesize glViewController;


- (void) applicationDidFinishLaunching:(UIApplication *)application   
{
    glViewController = [[EAGLViewController alloc] init];
    
    [self.window addSubview:self.glViewController.view];
    [self.window makeKeyAndVisible];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	EAGLView *eaglView = (EAGLView *)self.glViewController.view;
	[eaglView stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	EAGLView *eaglView = (EAGLView *)self.glViewController.view;
    [eaglView startAnimation];
}


- (void)applicationWillEnterForeground:(UIApplication *)application
{
	EAGLView *eaglView = (EAGLView *)self.glViewController.view;
	[eaglView startAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	EAGLView *eaglView = (EAGLView *)self.glViewController.view;
	[eaglView stopAnimation];    
}


- (void)applicationWillTerminate:(UIApplication *)application
{
	EAGLView *eaglView = (EAGLView *)self.glViewController.view;
	[eaglView stopAnimation];
}


- (void)handleWEngineCommand:(NSString *) command
{
    BOOL isDevicePhone = (UI_USER_INTERFACE_IDIOM()) == UIUserInterfaceIdiomPhone;

    if ([command isEqualToString: @"entergamestate:menu"] )
        [glViewController.eaglView displayAds];
    
    else if ([command isEqualToString: @"enterduelphase:end"] && isDevicePhone)
        [glViewController.eaglView displayAds];
    
    else if ([command isEqualToString: @"leaveduelphase:end"] || 
                [command isEqualToString: @"leavegamestate:menu"])
    {
        if (isDevicePhone)
            [glViewController.eaglView removeAds];
    }
}


- (void)dealloc
{
    [window release];
    [glViewController release];

    [super dealloc];
}

@end
