//
//  GADBannerView.h
//  Google AdMob Ads SDK
//
//  Copyright 2011 Google Inc. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GADRequest.h"
#import "GADRequestError.h"
#import "GADBannerViewDelegate.h"

#pragma mark -
#pragma mark Ad Sizes

// iPhone and iPod Touch ad size.
#define GAD_SIZE_320x50     CGSizeMake(320, 50)

// Medium Rectangle size for the iPad (especially in a UISplitView's left pane).
#define GAD_SIZE_300x250    CGSizeMake(300, 250)

// Full Banner size for the iPad (especially in a UIPopoverController or in
// UIModalPresentationFormSheet).
#define GAD_SIZE_468x60     CGSizeMake(468, 60)

// Leaderboard size for the iPad.
#define GAD_SIZE_728x90     CGSizeMake(728, 90)

#pragma mark -
#pragma mark Banner Ad View

// The view that displays banner ads.  A minimum implementation to get an ad
// from within a UIViewController class is:
//
//   // Place an ad at the top of the screen of an iPhone/iPod Touch.
//   CGRect adFrame = CGRectZero;
//   adFrame.size = GAD_SIZE_320x50;
//
//   // Create and setup the ad view.
//   GADBannerView *adView = [[GADBannerView alloc] initWithFrame:adFrame];
//   adView.rootViewController = self;
//   adView.adUnitID = @"ID created when registering my app";
//
//   // Place the ad view onto the screen.
//   [self.view addSubview:adView];
//   [adView release];
//
//   // Request an ad without any additional targeting information.
//   [adView loadRequest:nil];
//
@interface GADBannerView : UIView

#pragma mark Pre-Request

// Required value created in the AdSense website.  Create a new ad unit for
// every unique placement of an ad in your application.  Set this to the ID
// assigned for this placement.  Ad units are important for targeting and stats.
// Example values for different request types:
//     AdMob: a0123456789ABCD
//       DFP: /0123/ca-pub-0123456789012345/my-ad-identifier
//   AdSense: ca-mb-app-pub-0123456789012345/my-ad-identifier
// Mediation: AB123456789ABCDE
@property (nonatomic, copy) NSString *adUnitID;

// Required reference to the current root view controller.  For example the root
// view controller in tab-based application would be the UITabViewController.
@property (nonatomic, assign) UIViewController *rootViewController;

// Optional delegate object that receives state change notifications from this
// GADBannerView.  Typically this is a UIViewController, however, if you are
// unfamiliar with the delegate pattern it is recommended you subclass this
// GADBannerView and make it the delegate.  That avoids any chance of your
// application crashing if you forget to nil out the delegate.  For example:
//
//   @interface MyAdView : GADBannerView <GADBannerViewDelegate>
//   @end
//
//   @implementation MyAdView
//   - (id)initWithFrame:(CGRect)frame {
//     if ((self = [super initWithFrame:frame])) {
//       self.delegate = self;
//     }
//     return self;
//   }
//
//   - (void)dealloc {
//     self.delegate = nil;
//     [super dealloc];
//   }
//
//   @end
//
@property (nonatomic, assign) NSObject<GADBannerViewDelegate> *delegate;

#pragma mark Making an Ad Request

// Makes an ad request.  Additional targeting options can be supplied with a
// request object.  Refresh the ad by calling this method again.
- (void)loadRequest:(GADRequest *)request;

#pragma mark Ad Request

// YES, if the currently displayed ad (or most recent failure) was a result of
// auto refreshing as specified on server.  This will be set to NO after each
// loadRequest: method.
@property (nonatomic, readonly) BOOL hasAutoRefreshed;

@end
