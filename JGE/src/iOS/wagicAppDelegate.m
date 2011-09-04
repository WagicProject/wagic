#import "wagicAppDelegate.h"
#import "EAGLView.h"
#import "EAGLViewController.h"


@implementation wagicAppDelegate

@synthesize window;
@synthesize glViewController;


- (void) applicationDidFinishLaunching:(UIApplication *)application   
{
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

- (void)dealloc
{
    [window release];
    [glViewController release];

    [super dealloc];
}

@end
