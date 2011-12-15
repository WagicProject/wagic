/*

 AdWhirlAdapterGoogleAdSense.m

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

#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdapterGoogleAdSense.h"
#import "AdWhirlAdNetworkConfig.h"
#import "AdWhirlConfig.h"
#import "AdWhirlLog.h"

static NSDictionary *GASParamNameToSel;

@implementation AdWhirlAdapterGoogleAdSense

@synthesize adViewController;

+ (NSInteger)networkType {
  return AdWhirlAdNetworkTypeGoogleAdSense;
}

+ (void)load {
	[[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
  GASParamNameToSel = [[NSDictionary alloc] initWithObjectsAndKeys:
                     @"googleAdSenseKeywords", kGADAdSenseKeywords,
                     @"googleAdSenseAppWebContentURL", kGADAdSenseAppWebContentURL,
                     @"googleAdSenseChannelIDs", kGADAdSenseChannelIDs,
                     @"googleAdSenseAdType", kGADAdSenseAdType,
                     @"googleAdSenseHostID", kGADAdSenseHostID,
                     @"adWhirlAdBackgroundColor", kGADAdSenseAdBackgroundColor,
                     @"googleAdSenseAdTopBackgroundColor", kGADAdSenseAdTopBackgroundColor,
                     @"googleAdSenseAdBorderColor", kGADAdSenseAdBorderColor,
                     @"googleAdSenseAdLinkColor", kGADAdSenseAdLinkColor,
                     @"adWhirlTextColor", kGADAdSenseAdTextColor,
                     @"googleAdSenseAdURLColor", kGADAdSenseAdURLColor,
                     @"googleAdSenseAlternateAdColor", kGADAdSenseAlternateAdColor,
                     @"googleAdSenseAlternateAdURL", kGADAdSenseAlternateAdURL,
                     @"googleAdSenseAllowAdsafeMedium", kGADAdSenseAllowAdsafeMedium,
                     nil];
}

- (id)initWithAdWhirlDelegate:(id<AdWhirlDelegate>)delegate
                         view:(AdWhirlView *)view
                       config:(AdWhirlConfig *)config
                networkConfig:(AdWhirlAdNetworkConfig *)netConf {
  self = [super initWithAdWhirlDelegate:delegate
                                   view:view
                                 config:config
                          networkConfig:netConf];
  if (self != nil) {
    // Check that the required methods are implemented.
    if (![delegate respondsToSelector:@selector(googleAdSenseCompanyName)]) {
      [NSException raise:NSInvalidArgumentException format:
       @"You must implement googleAdSenseCompanyName in your AdwhirlDelegate in order to use Google AdSense"];
    }
    if (![delegate respondsToSelector:@selector(googleAdSenseAppName)]) {
      [NSException raise:NSInvalidArgumentException format:
       @"You must implement googleAdSenseAppName in your AdwhirlDelegate in order to use Google AdSense"];
    }
    if (![delegate respondsToSelector:@selector(googleAdSenseApplicationAppleID)]) {
      [NSException raise:NSInvalidArgumentException format:
       @"You must implement googleAdSenseApplicationAppleID in your AdwhirlDelegate in order to use Google AdSense"];
    }
  }
  return self;
}

- (void)getAd {
  adViewController = [[GADAdViewController alloc] initWithDelegate:self];

	NSMutableDictionary *attributes = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                     [self publisherId], kGADAdSenseClientID,
                                     [self companyName], kGADAdSenseCompanyName,
                                     [self appName], kGADAdSenseAppName,
                                     [self applicationAppleID], kGADAdSenseApplicationAppleID,
                                     [self testMode], kGADAdSenseIsTestAdRequest,
                                     nil];

  // optional params
  for (NSString *paramName in GASParamNameToSel) {
    SEL sel = NSSelectorFromString((NSString *)[GASParamNameToSel objectForKey:paramName]);
    if ([adWhirlDelegate respondsToSelector:sel]) {
      NSObject *val = [adWhirlDelegate performSelector:sel];
      if (val != nil) {
        [attributes setObject:val forKey:paramName];
      }
    }
  }

  AWLogDebug(@"Google AdSense attributes: %@", attributes);
	// load the ad
	adViewController.adSize = kGADAdSize320x50;
	[adViewController loadGoogleAd:attributes];
  adViewController.view.frame = kAdWhirlViewDefaultFrame;
	self.adNetworkView = adViewController.view;
}

- (void)stopBeingDelegate {
  if (adViewController != nil) {
    adViewController.delegate = nil;
  }
}

- (void)dealloc {
  // need to call here cos adViewController will be nil when super dealloc runs
  [self stopBeingDelegate];
  [adViewController release], adViewController = nil;
	[super dealloc];
}

#pragma mark parameter gathering methods

- (NSString *)publisherId {
	if ([adWhirlDelegate respondsToSelector:@selector(googleAdSenseClientID)]) {
		return [adWhirlDelegate googleAdSenseClientID];
	}

	return networkConfig.pubId;
}

- (NSString *)companyName {
	return [adWhirlDelegate googleAdSenseCompanyName];
}

- (NSString *)appName {
	return [adWhirlDelegate googleAdSenseAppName];
}

- (NSString *)applicationAppleID {
  return [adWhirlDelegate googleAdSenseApplicationAppleID];
}

- (NSNumber *)testMode {
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlTestMode)]
      && [adWhirlDelegate adWhirlTestMode]) {
    return [NSNumber numberWithInt:1];
  }
  return [NSNumber numberWithInt:0];
}

#pragma mark GADAdViewControllerDelegate required methods

- (UIViewController *)viewControllerForModalPresentation:(GADAdViewController *)adController {
	return [adWhirlDelegate viewControllerForPresentingModalView];
}

#pragma mark GADAdViewControllerDelegate notification methods

- (void)loadSucceeded:(GADAdViewController *)adController withResults:(NSDictionary *)results {
	[adWhirlView adapter:self didReceiveAdView:[adController view]];
}

- (void)loadFailed:(GADAdViewController *)adController withError:(NSError *) error {
	[adWhirlView adapter:self didFailAd:error];
}

- (GADAdClickAction)adControllerActionModelForAdClick:(GADAdViewController *)adController {
  [self helperNotifyDelegateOfFullScreenModal];
	return GAD_ACTION_DISPLAY_INTERNAL_WEBSITE_VIEW; // full screen web view
}

- (void)adControllerDidCloseWebsiteView:(GADAdViewController *)adController {
  [self helperNotifyDelegateOfFullScreenModalDismissal];
}

- (void)adControllerDidExpandAd:(GADAdViewController *)controller {
  [self helperNotifyDelegateOfFullScreenModal];
}

- (void)adControllerDidCollapseAd:(GADAdViewController *)controller {
  [self helperNotifyDelegateOfFullScreenModalDismissal];
}

@end
