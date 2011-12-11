//
//  WagicDownloadProgressView.m
//  wagic
//
//  Created by Michael Nguyen on 12/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "WagicDownloadProgressViewController.h"
#import "wagicAppDelegate.h"
#import "ZipArchive.h"
#import "ASIHTTPRequest.h"
#import "QuartzCore/QuartzCore.h"

@implementation WagicDownloadProgressViewController

@synthesize downloadProgressView;
@synthesize downloadMessageStatus;

//static NSString *kDownloadUrlPath = @"http://wololo.net/files/wagic/";
static NSString *kDownloadUrlPath = @"http://forevernow.net/wagic/";
static NSString *kDownloadFileName = @"core_017_iOS.zip";


- (void) unpackageResources
{
    NSError *error = nil;
    
    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory,
                                                         NSUserDomainMask, YES);
    NSString *userDocumentsDirectory = [paths objectAtIndex:0];
    NSString *downloadFilePath =  [[paths objectAtIndex: 0] stringByAppendingString: [NSString stringWithFormat: @"/%@", kDownloadFileName]];
    
    ZipArchive *za = [[ZipArchive alloc] init];
    if ([za UnzipOpenFile: downloadFilePath])
    {
        BOOL ret = [za UnzipFileTo: [NSString stringWithFormat: @"%@/Res/",userDocumentsDirectory] overWrite: YES];
        if (ret == NO)
        {
            // some error occurred
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
            else
            {
                wagicAppDelegate *appDelegate = (wagicAppDelegate *)[[UIApplication sharedApplication] delegate];

                NSNotificationCenter *dnc = [NSNotificationCenter defaultCenter];
                [dnc postNotificationName:@"readyToStartGame" object: appDelegate];
            }
        }
    }
    [za release], za = nil;
}



- (id) init
{
    self = [super init];
    if (self) {

        [self.view setFrame: CGRectMake(0, 0, 320, 480)];
        [self.view setAutoresizingMask: UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight];
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
        [self.downloadMessageStatus setFont: [UIFont systemFontOfSize: 20]];
        [self.downloadMessageStatus setText: @"Please wait while the core files are being downloaded."];

        downloadProgressView = [[UIProgressView alloc] initWithProgressViewStyle: UIProgressViewStyleDefault];
        [self.downloadProgressView setFrame: CGRectMake(0, 0, 250, 50)];
        [self.downloadProgressView setAutoresizingMask: UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight ];

        NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory,
                                                             NSUserDomainMask, YES);
        NSString *userResourceDirectory = [[paths objectAtIndex:0] stringByAppendingString: @"/Res"];
        NSString *downloadFilePath =  [[paths objectAtIndex: 0] stringByAppendingString: [NSString stringWithFormat: @"/%@",  kDownloadFileName]];
        
        // download the zip file but show a splash screen
        NSURL *url = [NSURL URLWithString: [NSString stringWithFormat: @"%@/%@", kDownloadUrlPath, kDownloadFileName]];
        __block ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
        [request setTemporaryFileDownloadPath: [NSString stringWithFormat: @"%@.tmp", userResourceDirectory]];
        [request setDownloadDestinationPath: downloadFilePath];
        [request setDownloadProgressDelegate: downloadProgressView];
        
        [request setCompletionBlock:^{
            [self unpackageResources];        
        }];
        [request setFailedBlock:^{
            NSError *error = [request error];
        }];
        
        [request startAsynchronous];
        [self.view addSubview: downloadMessageStatus];
        [self.view addSubview: downloadProgressView];

    }
    return self;
}


#pragma mark Application Lifecycle

- (void) didReceiveMemoryWarning 
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc that aren't in use.
}


#pragma mark -
#pragma mark UIView Delegate

- (void) handleRotation: (UIInterfaceOrientation) interfaceOrientation
{
    // position the activityIndicator
    bool isPhone = (UI_USER_INTERFACE_IDIOM()) == UIUserInterfaceIdiomPhone;
    bool isLandscapeOrientation = (UIInterfaceOrientationIsLandscape(interfaceOrientation));

    if (isLandscapeOrientation)
    {
        CGFloat height = self.view.bounds.size.width;
        [self.downloadProgressView setCenter: CGPointMake( height/2, 150)];
    }
    if (!isPhone)
    {
        CGFloat messageStatusHeight = [self.downloadMessageStatus.text sizeWithFont: [downloadMessageStatus font]].height;
        CGFloat logoCenterPointX = isLandscapeOrientation ? 512 : 384;
        [downloadProgressView setCenter: CGPointMake( logoCenterPointX, messageStatusHeight )];
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
