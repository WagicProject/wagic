#import "wagicAppDelegate.h"
#import "EAGLView.h"
#import "EAGLViewController.h"
#import "ASIHTTPRequest.h"
#import "ZipArchive.h"

@implementation wagicAppDelegate

@synthesize window;
@synthesize glViewController;
@synthesize wagicDownloadController;
@synthesize hostReach, wifiReach, internetReach;


- (void) downloadResources
{
    wagicDownloadController = [[WagicDownloadProgressViewController alloc] init];

    [self.window addSubview: wagicDownloadController.view];
    [self.window makeKeyWindow];

}


- (BOOL) hasResourceFiles
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory,
                                                         NSUserDomainMask, YES);
    NSString *userResourceDirectory = [[paths objectAtIndex:0] stringByAppendingString: @"/Res"];
    
    if (![fileManager fileExistsAtPath: userResourceDirectory] )
    {
        return NO;
    }
    
    return YES;
}


- (void) startGame
{
    glViewController = [[EAGLViewController alloc] init];
    [self.wagicDownloadController release];
    [self.window addSubview:self.glViewController.view];

    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
	[dnc removeObserver: self name: @"readyToStartGame" object: nil];
}


- (void)dealloc
{
    [window release];
    [glViewController release];
    [hostReach release];
    [wifiReach release];
    [internetReach release];
    
    [super dealloc];
}


- (void) applicationDidFinishLaunching:(UIApplication *)application   
{
    self.glViewController = nil;
    
    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
	[dnc addObserver:self selector:@selector(startGame) name:@"readyToStartGame" object: nil];
    
    // check to see if the Res folder exists.  If it does continue
    // otherwise bring up the download dialog and download the core files
    // once downloaded, extract the files and kick off the game.
    BOOL requiresResourceUpdate = [self hasResourceFiles];
    if (!requiresResourceUpdate)
    {
        [self downloadResources];
    }
    else
    {
        [self startGame];
    }
    
    [self.window makeKeyAndVisible];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view startAnimation];
}


- (void)applicationWillEnterForeground:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view startAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view stopAnimation];
}


- (void)applicationWillTerminate:(UIApplication *)application
{
    if ( [self.glViewController.view respondsToSelector: @selector(stopAnimation)])
        [self.glViewController.view stopAnimation];
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



- (void) rotateBackgroundImage:(UIInterfaceOrientation)fromInterfaceOrientation toInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	bool isPhone = (UI_USER_INTERFACE_IDIOM()) == UIUserInterfaceIdiomPhone;
    
	if (isPhone) 
    {
        UIImage *bgImage = [UIImage imageNamed: @"Default-Portrait.png"];
        [[[self.window subviews] objectAtIndex: 0] setBackgroundColor: [UIColor colorWithPatternImage: bgImage]];
	}
	else 
    {
        [self.window setBackgroundColor: [UIColor clearColor]];
		if (UIInterfaceOrientationIsLandscape( toInterfaceOrientation)) {
            UIImage *bgImage = [UIImage imageNamed: @"Default-Landscape"];
            [[[self.window subviews] objectAtIndex: 0] setBackgroundColor: [UIColor colorWithPatternImage: bgImage]];
		}
		else {
            UIImage *bgImage = [UIImage imageNamed: @"Default-Portrait.png"];
            [[[self.window subviews] objectAtIndex: 0] setBackgroundColor: [UIColor colorWithPatternImage: bgImage]];
            [self.window setBackgroundColor: [UIColor colorWithPatternImage: bgImage]];
		}
	}
}


#pragma mark Application Contents

- (BOOL) isNetworkAvailable
{
	BOOL netAvailable = NO;
    NSDate *startTime = [[[NSDate alloc ] init] autorelease];
	
    hostReach = [[Reachability reachabilityForGoogleDNS] retain];
    
    NetworkStatus netStatus = [hostReach currentReachabilityStatus];
    
	
	if (netStatus == ReachableViaWiFi || netStatus == ReachableViaWWAN)	{
		netAvailable = YES;
	}
    
    //[[QLog log] logOption: kLogOptionTiming withFormat:@"isNetworkAvailble? %5.2f", [startTime timeIntervalSinceNow]];
	
	[hostReach release];
	
	return netAvailable;
}


#pragma mark -

@end
