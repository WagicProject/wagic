#import "wagicAppDelegate.h"
#import "EAGLView.h"
#import "EAGLViewController.h"
#import "ASIHTTPRequest.h"
#import "ZipArchive.h"

#include <CommonCrypto/CommonDigest.h>

@implementation wagicAppDelegate

@synthesize window;
@synthesize glViewController;
@synthesize wagicDownloadController;
@synthesize hostReach, wifiReach, internetReach;

- (void) updateComplete: (id) notificationMsg
{
    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
 
    [dnc postNotificationName: @"initializeGame" object: self];
    [dnc removeObserver: self name: @"coreComplete" object: nil];
    [dnc removeObserver: self name: @"iosConfigComplete" object: nil];

}

- (void) initIosUpdate: (id) notificationMsg
{
    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
    [dnc addObserver: self selector: @selector(updateComplete:) name: @"iosConfigComplete" object: nil];
    [wagicDownloadController performSelectorInBackground: @selector(startDownload:) withObject:@"iosConfig"];
}

- (void) downloadResources
{
    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
    
	[dnc addObserver:self selector:@selector(initIosUpdate:) name:@"coreComplete" object: nil];

    wagicDownloadController = [[WagicDownloadProgressViewController alloc] init];
    [wagicDownloadController performSelectorInBackground: @selector(startDownload:) withObject:@"core"];
    
    [self.window addSubview: wagicDownloadController.view];
    [self.window makeKeyWindow];

}

/* unpackage the zip file and delete it afterwards */
- (void) unpackageResources: (NSString *) folderName pathPrefixName: (NSString *) pathNamePrefix
{
    NSString *pathPrefix = nil;
    if ( pathNamePrefix == nil ) // default to User
        pathPrefix = @"User";
    else 
        pathPrefix = pathNamePrefix;
    
    NSError *error = nil;
    
    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *userDocumentsDirectory = [paths objectAtIndex:0];
    NSString *downloadFilePath =  [userDocumentsDirectory stringByAppendingString: [NSString stringWithFormat: @"/%@/%@.zip", pathPrefix, folderName]];
    
    ZipArchive *za = [[ZipArchive alloc] init];
    if ([za UnzipOpenFile: downloadFilePath])
    {
        NSString *destinationFilePath = [NSString stringWithFormat: @"%@/%@/",userDocumentsDirectory, pathPrefix];
        NSLog(@"Unzipping %@ to %@", downloadFilePath, destinationFilePath);
        BOOL ret = [za UnzipFileTo: destinationFilePath overWrite: YES];
        if (ret == NO)
        {
            // some error occurred
            NSLog(@"An Error occurred while unpacking zip file.");
        }
        [za UnzipCloseFile];
        
        if (ret == YES)
        {
            // delete the archive
            [fm removeItemAtPath: downloadFilePath error: &error];
            if (error != nil)
            {
                NSLog(@"error occurred while trying to delete zip file! %@\n%@", downloadFilePath, [error localizedDescription] );
            }
        }
    }
    [za release], za = nil;
    
}


- (NSString *) getDirContents: (NSString *) path
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSArray *dirContents = [fileManager contentsOfDirectoryAtPath: path error:nil];
    NSMutableString *data = [[NSMutableString alloc] init ];
    for (NSString *filename in dirContents)
    {
        NSString *pathname = [NSString stringWithFormat: @"%@/%@", path, filename];
        [data appendFormat: @"%@\n", pathname];
        
        BOOL isDirectory = [[fileManager attributesOfItemAtPath: pathname error: nil] objectForKey: NSFileType] == NSFileTypeDirectory;
        if (isDirectory)
            [data appendString: [self getDirContents: pathname]];
    }
    
    NSString *manifestList = [data stringByAppendingFormat: @"\n"];
    [data release];
    
    return manifestList;
}

- (void) createManifest: (NSString *)docsPath
{
    NSString *manifestFile = [docsPath stringByAppendingPathComponent:@"Manifest"];
    [[self getDirContents: docsPath] writeToFile:manifestFile atomically:YES encoding:NSUTF8StringEncoding error: nil];
   
}

/**
 check for any zip files dropped into the documents directory before loading the game.  
 If so, move the "core" files into the "Res" directory and move all other zip files into the "User" directory. 
 Check for a "core" zip file in the Res directory. If it exists, then return YES. Otherwise, return NO.
 */

- (void) initializeResources
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsPath = [paths objectAtIndex: 0];
    NSArray *docsPathContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath: docsPath error:nil];
    NSCompoundPredicate *compoundPredicate = [[NSCompoundPredicate alloc] initWithType:NSAndPredicateType subpredicates: [NSArray arrayWithObjects: [NSPredicate predicateWithFormat:@"self ENDSWITH '.zip'"], [NSPredicate predicateWithFormat:@"NOT (self  BEGINSWITH 'core_')"], nil]];
    
    NSArray *coreFiles = [docsPathContents filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"self BEGINSWITH 'core_0171'"]];

    NSArray *resourceZipFiles = [docsPathContents filteredArrayUsingPredicate: compoundPredicate];
    NSString *userPath = [NSString stringWithFormat: @"%@/User", docsPath];
    NSString *resPath = [NSString stringWithFormat: @"%@/Res", docsPath];
    NSError *error = nil;
    
    [compoundPredicate release], compoundPredicate = nil;
    
    if ( ([resourceZipFiles count]  > 0 ) &&  ![fileManager fileExistsAtPath: userPath] )
        [fileManager createDirectoryAtPath: userPath withIntermediateDirectories: YES attributes:nil error:nil ];
    
    for (NSString *zipFile in resourceZipFiles)
    {
        NSString *oldPath = [NSString stringWithFormat: @"%@/%@", docsPath, zipFile];
        NSString *newPath = [NSString stringWithFormat: @"%@/%@", userPath, zipFile];
        
        [fileManager moveItemAtPath: oldPath toPath:newPath error: &error];
        NSLog(@"Moving %@ to %@", oldPath, newPath);
        if ( error != nil )
        {
            NSLog(@"Error happened while trying to move %@ to %@: \n%@", oldPath, newPath, [error localizedDescription]);
            error = nil;
        }
    }

    for (NSString *zipFile in coreFiles)
    {
        NSString *oldPath = [NSString stringWithFormat: @"%@/%@", docsPath, zipFile];
        NSString *newPath = [NSString stringWithFormat: @"%@/%@", resPath, zipFile];
        
        [fileManager moveItemAtPath: oldPath toPath:newPath error: &error];
        NSLog(@"Moving %@ to %@", oldPath, newPath);
        if ( error != nil )
        {
            NSLog(@"Error happened while trying to move %@ to %@: \n%@", oldPath, newPath, [error localizedDescription]);
            error = nil;
        }
    }
}

- (BOOL) hasResourceFiles
{   
    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsPath = [paths objectAtIndex: 0];
    NSString *resPath = [NSString stringWithFormat: @"%@/Res", docsPath];
    
    NSArray *resDirContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath: resPath error:nil];
    NSArray *coreFiles = [resDirContents filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"self BEGINSWITH 'core_'"]];
    
    if ([coreFiles count] >= 2)
    {
        return YES;
    }
    
    return NO;
}


- (void)preGameInitialization
{
    NSString *docsPath = [NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex: 0];
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *userPath = [docsPath stringByAppendingString: @"/User"];
    
    NSArray *userDirectories = [fm contentsOfDirectoryAtPath: userPath error: nil];

    for (NSString *userFilename in userDirectories)
    {
        NSString *userPathname = [userPath stringByAppendingFormat: @"/%@", userFilename];
        NSString *zipFileName = [userPathname stringByAppendingString: @".zip"];
        
        if ([[fm attributesOfItemAtPath: userPathname error: nil] objectForKey: NSFileType] == NSFileTypeDirectory && ([fm fileExistsAtPath:zipFileName]))
        {
            [self unpackageResources: userFilename pathPrefixName: @"User"];
            [fm removeItemAtPath: zipFileName error: nil];
        }
        
        else if ( [userFilename hasPrefix: @"ai_decks"] ) // special case to allow manual override of AI decks in User directory
        {
            [self unpackageResources: @"ai_decks" pathPrefixName: @"User/ai/baka"];
        }
    }
    // scan for deck*.txt and collection.dat as well as options.txt in the Documents directory and copy them into the player directory
    NSArray *playerDataFilePredicates = [NSArray arrayWithObjects: 
                                         [NSPredicate predicateWithFormat: @"SELF BEGINSWITH[cd] 'deck' AND SELF ENDSWITH '.txt'"], 
                                         [NSPredicate predicateWithFormat: @"SELF BEGINSWITH[cd] 'options' AND SELF ENDSWITH '.txt'"], 
                                         [NSPredicate predicateWithFormat: @"SELF BEGINSWITH[cd] 'tasks' AND SELF ENDSWITH '.dat'"], 
                                         [NSPredicate predicateWithFormat: @"SELF BEGINSWITH[cd] 'collection' AND SELF ENDSWITH '.dat'"], nil];

    NSCompoundPredicate *playerDataPredicate = [[NSCompoundPredicate alloc] initWithType:NSOrPredicateType subpredicates: playerDataFilePredicates];
    NSArray *playerDataFiles = [[fm contentsOfDirectoryAtPath: docsPath error: nil] filteredArrayUsingPredicate: playerDataPredicate];

    for (NSString *file in playerDataFiles)
    {
        NSString *fromPath = [docsPath stringByAppendingFormat: @"/%@", file];
        NSString *toPath = [docsPath stringByAppendingFormat: @"/User/player/%@", [file lowercaseString]];
        [fm moveItemAtPath: fromPath toPath: toPath error: nil];
    }
    
    [playerDataPredicate release], playerDataPredicate = nil;
    
    [self createManifest: docsPath];
    
    [[NSNotificationCenter defaultCenter] postNotificationName: @"readyToStartGame" object: nil];
}


- (void) startGame
{
    if (glViewController != nil) 
        [glViewController release];
    glViewController = [[EAGLViewController alloc] init];
    
    if (wagicDownloadController != nil)
        [wagicDownloadController release];

    [self.window addSubview:self.glViewController.view];

    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
	[dnc removeObserver: self name: @"intializeGame" object: nil];
	[dnc removeObserver: self name: @"readyToStartGame" object: nil];
    [dnc addObserver: glViewController selector:@selector(pauseGame) name: UIApplicationWillResignActiveNotification object: nil];
    [dnc addObserver: glViewController selector:@selector(resumeGame) name: UIApplicationDidBecomeActiveNotification object: nil];
    [dnc addObserver: glViewController selector:@selector(resumeGame) name:UIApplicationWillEnterForegroundNotification object: nil];
    [dnc addObserver: glViewController selector:@selector(destroyGame) name:UIApplicationWillTerminateNotification object: nil];
}


- (void)dealloc
{
    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
	[dnc removeObserver: glViewController name: UIApplicationDidBecomeActiveNotification object: nil];
	[dnc removeObserver: glViewController name: UIApplicationDidEnterBackgroundNotification object: nil];
	[dnc removeObserver: glViewController name: UIApplicationWillTerminateNotification object: nil];
	[dnc removeObserver: glViewController name: UIApplicationWillResignActiveNotification object: nil];
    [window release];
    [glViewController release];
    [hostReach release];
    [wifiReach release];
    [internetReach release];
    
    [super dealloc];
}


- (void) setupNetworkListeners
{
    NSLog(@"App checking network connections");

    hostReach = [[Reachability reachabilityForGoogleDNS] retain];
    internetReach = [[Reachability reachabilityForInternetConnection] retain];
    wifiReach = [[Reachability reachabilityForLocalWiFi] retain];
    
    [hostReach startNotifier];
    [internetReach startNotifier];
    [wifiReach startNotifier];
}


- (BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions 
{
    self.glViewController = nil;
    
    [self setupNetworkListeners];

    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
	[dnc addObserver:self selector:@selector(preGameInitialization) name:@"initializeGame" object: nil];
	[dnc addObserver:self selector:@selector(startGame) name:@"readyToStartGame" object: nil];
    
    [self initializeResources];
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
        [self preGameInitialization];
    }
    
    [self.window setBackgroundColor: [UIColor blackColor]];
    [self.window makeKeyAndVisible];

    return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
        [self.glViewController.view destroyGame];
}

- (void)initializeKeyboard: (id) initialState
{
    [self.glViewController toggleKeyboardWithState: initialState];
}

- (void) handleWEngineCommand:(NSString *) command 
             withUIParameters: (CGFloat) x 
                  yCoordinate: (CGFloat) y 
                        width: (CGFloat) width 
                       height: (CGFloat) height
{
}

- (void)handleWEngineCommand:(NSString *) command withParameter: (NSString *) parameter
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
    else if ([command isEqualToString: @"displayKeyboard"])
    {
        [self initializeKeyboard: parameter];
    }
    else if ([command isEqualToString: @"combatGuiEndDamage"])
    {
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
    //NSDate *startTime = [[[NSDate alloc ] init] autorelease];
	
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
