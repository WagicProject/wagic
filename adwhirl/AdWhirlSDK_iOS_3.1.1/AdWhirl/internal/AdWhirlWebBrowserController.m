/*

 AdWhirlWebBrowserController.m

 Copyright 2009 AdMob, Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

*/

#import "AdWhirlWebBrowserController.h"
#import "AdWhirlLog.h"

#define kAWWebViewAnimDuration 1.0

@interface AdWhirlWebBrowserController ()
@property (nonatomic,retain) NSArray *loadingButtons;
@property (nonatomic,retain) NSArray *loadedButtons;
@end


@implementation AdWhirlWebBrowserController

@synthesize delegate;
@synthesize viewControllerForPresenting;
@synthesize loadingButtons;
@synthesize loadedButtons;

@synthesize webView;
@synthesize toolBar;
@synthesize backButton;
@synthesize forwardButton;
@synthesize reloadButton;
@synthesize stopButton;
@synthesize linkOutButton;
@synthesize closeButton;


- (id)init {
  if ((self = [super initWithNibName:@"AdWhirlWebBrowser" bundle:nil])) {
  }
  return self;
}

- (void)viewWillAppear:(BOOL)animated {
	[super viewWillAppear:animated];
	if (self.webView.request) {
    // has content from before, clear by creating another UIWebView
    CGRect frame = self.webView.frame;
    NSInteger tag = self.webView.tag;
    UIWebView *newView = [[UIWebView alloc] initWithFrame:frame];
    newView.tag = tag;
    UIWebView *oldView = self.webView;
    [oldView removeFromSuperview];
    [self.view addSubview:newView];
    newView.delegate = self;
    newView.scalesPageToFit = YES;
    [newView release];
	}
  self.toolBar.items = self.loadedButtons;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  NSArray *items = self.toolBar.items;

  NSMutableArray *loadingItems = [[NSMutableArray alloc] init];
  [loadingItems addObjectsFromArray:items];
  [loadingItems removeObjectAtIndex:4];
  self.loadingButtons = loadingItems;
  [loadingItems release], loadingItems = nil;

  NSMutableArray *loadedItems = [[NSMutableArray alloc] init];
  [loadedItems addObjectsFromArray:items];
  [loadedItems removeObjectAtIndex:5];
  self.loadedButtons = loadedItems;
  [loadedItems release], loadedItems = nil;
}

- (void)viewDidDisappear:(BOOL)animated {
  if (self.delegate) {
    [delegate webBrowserClosed:self];
  }
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
  return [viewControllerForPresenting shouldAutorotateToInterfaceOrientation:interfaceOrientation];
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];

	// Release any cached data, images, etc that aren't in use.
}

- (void)presentWithController:(UIViewController *)viewController transition:(AWCustomAdWebViewAnimType)animType {
  self.viewControllerForPresenting = viewController;

  if ([self respondsToSelector:@selector(setModalTransitionStyle:)]) {
    switch (animType) {
      case AWCustomAdWebViewAnimTypeFlipFromLeft:
      case AWCustomAdWebViewAnimTypeFlipFromRight:
        self.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
        break;
      case AWCustomAdWebViewAnimTypeFadeIn:
        self.modalTransitionStyle = UIModalTransitionStyleCrossDissolve;
      case AWCustomAdWebViewAnimTypeModal:
      default:
        self.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
        break;
    }
  }
  [viewController presentModalViewController:self animated:YES];
}

- (void)loadURL:(NSURL *)url {
	NSURLRequest *urlRequest = [NSURLRequest requestWithURL:url];
	[self.webView loadRequest:urlRequest];
}

- (void)dealloc {
  [loadingButtons release], loadingButtons = nil;
  [loadedButtons release], loadedButtons = nil;

  // IBOutlets were retained automatically
  webView.delegate = nil;
  [webView release], webView = nil;
  [toolBar release], toolBar = nil;
  [backButton release], backButton = nil;
  [forwardButton release], forwardButton = nil;
  [reloadButton release], reloadButton = nil;
  [stopButton release], stopButton = nil;
  [linkOutButton release], linkOutButton = nil;
  [closeButton release], closeButton = nil;
  [super dealloc];
}

#pragma mark -
#pragma mark UIWebViewDelegate methods

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request
                                                 navigationType:(UIWebViewNavigationType)navigationType {
  if ([request URL] != nil && [[request URL] scheme] != nil) {
    if ([[[request URL] scheme] isEqualToString:@"mailto"]) {
      // need to explicitly call out to the Mail app
      [[UIApplication sharedApplication] openURL:[request URL]];
    }
  }
  return YES;
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error {
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
  self.toolBar.items = self.loadedButtons;
	if (self.webView.canGoForward) {
		self.forwardButton.enabled = YES;
	}
	if (self.webView.canGoBack) {
		self.backButton.enabled = YES;
	}
	self.reloadButton.enabled = YES;
	self.stopButton.enabled = NO;
	if (self.webView.request) {
    self.linkOutButton.enabled = YES;
  }
}

- (void)webViewDidFinishLoad:(UIWebView *)webView {
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
  self.toolBar.items = self.loadedButtons;
	if (self.webView.canGoForward) {
		self.forwardButton.enabled = YES;
	}
	if (self.webView.canGoBack) {
		self.backButton.enabled = YES;
	}
	self.reloadButton.enabled = YES;
	self.stopButton.enabled = NO;
	if (self.webView.request) {
    self.linkOutButton.enabled = YES;
  }

//  // extract title of page
//  NSString* title = [self.webView stringByEvaluatingJavaScriptFromString: @"document.title"];
//  self.navigationItem.title = title;
}

- (void)webViewDidStartLoad:(UIWebView *)webView {
	[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
  self.toolBar.items = self.loadingButtons;
	self.forwardButton.enabled = NO;
	self.backButton.enabled = NO;
	self.reloadButton.enabled = NO;
	self.stopButton.enabled = YES;
}

#pragma mark -
#pragma mark button targets

- (IBAction)forward:(id)sender {
	[self.webView goForward];
}

- (IBAction)back:(id)sender {
	[self.webView goBack];
}

- (IBAction)stop:(id)sender {
	[self.webView stopLoading];
}

- (IBAction)reload:(id)sender {
  [self.webView reload];
}

- (IBAction)linkOut:(id)sender {
  [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
  [[UIApplication sharedApplication] openURL:self.webView.request.URL];
}

- (IBAction)close:(id)sender {
  [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
  [viewControllerForPresenting dismissModalViewControllerAnimated:YES];
}

@end


@implementation AdWhirlBackButton

- (void)awakeFromNib {
  // draw the back image
  CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx = CGBitmapContextCreate(nil, 25, 25, 8, 0, colorspace,
                                           kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(colorspace);
  CGPoint bot = CGPointMake(19, 2);
  CGPoint top = CGPointMake(19, 20);
  CGPoint tip = CGPointMake(4, 11);
  CGContextSetFillColorWithColor(ctx, [UIColor whiteColor].CGColor);
  CGContextMoveToPoint(ctx, bot.x, bot.y);
  CGContextAddLineToPoint(ctx, tip.x, tip.y);
  CGContextAddLineToPoint(ctx, top.x, top.y);
  CGContextFillPath(ctx);

  // set the image
  CGImageRef backImgRef = CGBitmapContextCreateImage(ctx);
  CGContextRelease(ctx);
  UIImage* backImage = [[UIImage alloc] initWithCGImage:backImgRef];
  CGImageRelease(backImgRef);
  self.image = backImage;
  [backImage release];
}

@end
