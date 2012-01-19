//
//  WagicDownloadProgressView.m
//  wagic
//
//  Created by Michael Nguyen on 12/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "WagicDownloadProgressViewController.h"
#import "wagicAppDelegate.h"
#import "ASIHTTPRequest.h"
#import "QuartzCore/QuartzCore.h"

@implementation WagicDownloadProgressViewController

@synthesize downloadProgressView;
@synthesize downloadMessageStatus;

static NSString *kDownloadUrlPath = @"http://wagic.googlecode.com/files/";
static NSString *kDownloadFileName = @"core_0171.zip";
static NSString *kDownloadIosUpdateFileName = @"core_0171_iOS.zip";



- (void) handleFailedDownload: (NSNotification *) sender
{
    NSString *downloadType = [sender object];
    // figure out what to do.  There could be other types of downloads, for now there is only "core"
    if ( [downloadType isEqualToString: @"core"] )
    {
        NSLog(@"Download Core files failed.  Retrying... ");
        [self.downloadMessageStatus setText: @"Download Core files failed.  Retrying... "];

            UIAlertView *noNetworkConnectionAlert = [[UIAlertView alloc] initWithTitle: @"No Network Connection" message: @"Internet connection not found.  Download can not continue until it is restored. Restore Wifi or Cellular connection and retry" delegate: self cancelButtonTitle: @"Retry Download" otherButtonTitles: nil];
            [self.view addSubview: noNetworkConnectionAlert];
            [noNetworkConnectionAlert show];
            [noNetworkConnectionAlert release];
    }
    
}

- (void)alertView:(UIAlertView *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
    // the user clicked one of the OK/Cancel buttons
    if (buttonIndex == 0 && [[actionSheet buttonTitleAtIndex: 0] isEqualToString: @"Retry Download"])
    {
        [self startDownload: @"core"];
    }
    else
    {
        NSLog(@"cancel");
    }
}

- (void) startDownload: (NSString *) downloadType
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    [self.downloadMessageStatus performSelectorOnMainThread:@selector(setText:) withObject:[NSString stringWithFormat: @"Please wait while the %@ files are being downloaded.", downloadType] waitUntilDone: NO ];
    
    if ( downloadProgressView != nil )
    {
        [downloadProgressView removeFromSuperview];
        [downloadProgressView release], downloadProgressView = nil;
    }
    
    downloadProgressView = [[UIProgressView alloc] initWithProgressViewStyle: UIProgressViewStyleDefault];
    [self.downloadProgressView setFrame: CGRectMake(0, 0, 250, 50)];
    [self.downloadProgressView setAutoresizingMask: UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight ];
    
    [self.view addSubview: downloadProgressView];
    [self handleRotation: self.interfaceOrientation];

    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory,
                                                         NSUserDomainMask, YES);
    NSString *systemResourceDirectory = [[paths objectAtIndex:0] stringByAppendingString: @"/Res"];
    NSString *userResourceDirectory = [[paths objectAtIndex: 0] stringByAppendingString: @"/User"];

    NSError *error = nil;
    // make sure Res directory exists
    if ( ![[NSFileManager defaultManager] fileExistsAtPath: systemResourceDirectory] ) 
        [[NSFileManager defaultManager] createDirectoryAtPath:systemResourceDirectory withIntermediateDirectories: YES attributes:nil error: &error];

    if (error != nil)
    {
        NSLog(@"Error in creating System Directory! %@", [error localizedDescription]);
        error = nil;
    }
    
    // make sure the User directory exists as well
    if ( ![[NSFileManager defaultManager] fileExistsAtPath: userResourceDirectory] )
        [[NSFileManager defaultManager] createDirectoryAtPath: userResourceDirectory withIntermediateDirectories: YES attributes:nil error: &error];

    if (error != nil)
    {
        NSLog(@"Error in creating User Directory! %@", [error localizedDescription]);
        error = nil;
    }
    
    // if an error occurred while creating the directory, game can't really run so do something
    // TODO: throw out a notification and deal with error
    
    NSURL *url = nil;
    NSString *downloadFilename = nil;
    // determine which file to download
    if ([downloadType isEqualToString: @"core"])
    {
        downloadFilename = kDownloadFileName;
    }
    else if ( [downloadType isEqualToString: @"iosConfig"] )
    {
        downloadFilename = kDownloadIosUpdateFileName;
    }
    else if ( [downloadType isEqualToString: @"someOtherType"] )
    {
        NSLog( @"Not Implemented for type: %@", downloadType);
    }

    url = [NSURL URLWithString: [NSString stringWithFormat: @"%@/%@", kDownloadUrlPath, downloadFilename]];    
    NSString *downloadFilePath =  [systemResourceDirectory stringByAppendingString: [NSString stringWithFormat: @"/%@",  downloadFilename]];

    NSLog(@"Downloading %@", [url absoluteURL]);
    
    __block ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
    [request setTemporaryFileDownloadPath: [NSString stringWithFormat: @"%@/%@.tmp", systemResourceDirectory, downloadFilename]];

    [request setDownloadDestinationPath: downloadFilePath];
    [request setDownloadProgressDelegate: downloadProgressView];
    [request setShouldContinueWhenAppEntersBackground: YES];
    [request setAllowCompressedResponse: YES];
    
    [request setCompletionBlock:^{
        wagicAppDelegate *appDelegate = (wagicAppDelegate *)[[UIApplication sharedApplication] delegate];        
        NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
        NSLog(@"Saving to %@", downloadFilePath);
        [dnc postNotificationName: [NSString stringWithFormat: @"%@Complete", downloadType] object: appDelegate];

    }];
    [request setFailedBlock:^{
        // if there is an error in downloading the package what do we do?
        // TODO: do something useful if there's an error in the download
        // perhaps restart the download

        NSError *error = [request error];
        NSString *errorMessage = [NSString stringWithFormat: @"There was an error in downloading your files. %@", [error localizedDescription]];
        NSLog(@"Error with download: %@", errorMessage);
        // post a notification that a download error has occurred.
        NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
        [dnc postNotificationName:@"fileDownloadFailed" object: downloadType];
        
    }];
    
    [request startAsynchronous];
    
    [pool drain], pool = nil;
}

- (id) init
{
    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
    [dnc addObserver: self selector: @selector(handleFailedDownload:) name: @"fileDownloadFailed" object: nil];

    self = [super init];
    if (self) {
        bool isPhone = (UI_USER_INTERFACE_IDIOM()) == UIUserInterfaceIdiomPhone;

        [self.view setFrame: CGRectMake(0, 0, 320, 480)];
        [self.view setAutoresizingMask: UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight];
        [[self view] setBackgroundColor: [UIColor clearColor]];
        // Initialization code

        downloadMessageStatus = [[UITextView alloc] initWithFrame: CGRectMake(0, 0, 320, 320)];        
        [self.downloadMessageStatus setBackgroundColor:[UIColor clearColor]];
        [downloadMessageStatus setEditable: NO];
        [self.view setBackgroundColor:[UIColor clearColor]];
        [self.downloadMessageStatus setTextColor: [UIColor whiteColor]];
        [self.downloadMessageStatus setTextAlignment: UITextAlignmentCenter];
        self.downloadMessageStatus.clipsToBounds = YES;
        self.downloadMessageStatus.layer.cornerRadius = 10.0f;
        [self.downloadMessageStatus setAutoresizingMask: UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight ];
        
        if (isPhone)
            [self.downloadMessageStatus setFont: [UIFont systemFontOfSize: 20]];
        else
            [self.downloadMessageStatus setFont: [UIFont systemFontOfSize: 35]];
        
        [self.view addSubview: downloadMessageStatus];

    }
    return self;
}

#pragma mark - Application Lifecycle


- (void) didReceiveMemoryWarning 
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc that aren't in use.
}


#pragma mark - UIView Delegate

- (void) handleRotation: (UIInterfaceOrientation) interfaceOrientation
{
    // position the activityIndicator
    bool isPhone = (UI_USER_INTERFACE_IDIOM()) == UIUserInterfaceIdiomPhone;
    bool isLandscapeOrientation = (UIInterfaceOrientationIsLandscape(interfaceOrientation));

    if (!isPhone)
    {
        CGSize messageSize = [self.downloadMessageStatus.text sizeWithFont: [downloadMessageStatus font]];
        CGFloat messageStatusWidth = messageSize.width;
        CGFloat messageStatusHeight = messageSize.height;
        CGFloat screenWidth = isLandscapeOrientation ? 1024 : 768;
        CGFloat barWidth = MIN(messageStatusWidth, screenWidth) - 100;
        CGFloat xOffset =  (MAX(messageStatusWidth, screenWidth) - barWidth)/2;
        [self.downloadProgressView setFrame: CGRectMake( xOffset, messageStatusHeight + 60, barWidth, 50)];
    }
    else
    {
        if (isLandscapeOrientation)
        {
            CGFloat height = self.view.bounds.size.width;
            [self.downloadProgressView setCenter: CGPointMake( height/2, 150)];
        }

    }
}


- (void) viewDidAppear:(BOOL)animated
{
    [self handleRotation: self.interfaceOrientation];
}


- (void) viewWillAppear:(BOOL)animated
{
    [super viewWillAppear: animated];
    [self handleRotation: self.interfaceOrientation];
}


- (void) viewDidLoad
{
    [super viewDidLoad];
    [self handleRotation: self.interfaceOrientation];

}

- (void) viewDidUnload
{
    NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
    [dnc removeObserver: self name: @"coreFileDownloadFailed" object: nil];
}

// Only allow auto rotation on iPads.
- (void) didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    [self handleRotation: self.interfaceOrientation];
}


- (void) willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval) duration
{
	wagicAppDelegate *appDelegate = (wagicAppDelegate *)[[UIApplication sharedApplication] delegate];
	[appDelegate rotateBackgroundImage: self.interfaceOrientation toInterfaceOrientation: toInterfaceOrientation];
}

 
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{

	bool isPhone = (UI_USER_INTERFACE_IDIOM()) == UIUserInterfaceIdiomPhone;
    BOOL rotateDevice = !((interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown) 
                         || (interfaceOrientation == UIInterfaceOrientationPortrait));
	if (isPhone)
		return rotateDevice;

	return YES;	
}


#pragma mark -


/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
