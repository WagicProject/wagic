/*

 AdWhirlAdapterGoogleAdMobAds.m
 Copyright 2011 Google, Inc.

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

#import "AdWhirlAdapterGoogleAdMobAds.h"
#import "AdWhirlAdNetworkConfig.h"
#import "AdWhirlView.h"
#import "GADBannerView.h"
#import "AdWhirlLog.h"
#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdNetworkRegistry.h"

@implementation AdWhirlAdapterGoogleAdMobAds

+ (void)load {
  [[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
}

+ (AdWhirlAdNetworkType)networkType {
  return AdWhirlAdNetworkTypeAdMob;
}

// converts UIColor to hex string, ignoring alpha.
- (NSString *)hexStringFromUIColor:(UIColor *)color {
  CGColorSpaceModel colorSpaceModel =
      CGColorSpaceGetModel(CGColorGetColorSpace(color.CGColor));
  if (colorSpaceModel == kCGColorSpaceModelRGB
      || colorSpaceModel == kCGColorSpaceModelMonochrome) {
    const CGFloat *colors = CGColorGetComponents(color.CGColor);
    CGFloat red = 0.0, green = 0.0, blue = 0.0;
    if (colorSpaceModel == kCGColorSpaceModelRGB) {
      red = colors[0];
      green = colors[1];
      blue = colors[2];
      // we ignore alpha here.
    } else if (colorSpaceModel == kCGColorSpaceModelMonochrome) {
      red = green = blue = colors[0];
    }
    return [NSString stringWithFormat:@"%02X%02X%02X",
                (int)(red * 255), (int)(green * 255), (int)(blue * 255)];
  }
  return nil;
}

- (NSObject *)delegateValueForSelector:(SEL)selector {
  return ([adWhirlDelegate respondsToSelector:selector]) ?
      [adWhirlDelegate performSelector:selector] : nil;
}

- (void)getAd {
  GADRequest *request = [GADRequest request];
  NSObject *value;

  NSMutableDictionary *additional = [NSMutableDictionary dictionary];
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlTestMode)]
      && [adWhirlDelegate adWhirlTestMode]) {
    [additional setObject:@"on" forKey:@"adtest"];
  }

  if ((value = [self delegateValueForSelector:
                      @selector(adWhirlAdBackgroundColor)])) {
    [additional setObject:[self hexStringFromUIColor:(UIColor *)value]
                  forKey:@"color_bg"];
  }

  if ((value = [self delegateValueForSelector:
                      @selector(adWhirlAdBackgroundColor)])) {
    [additional setObject:[self hexStringFromUIColor:(UIColor *)value]
                   forKey:@"color_text"];
  }

  // deliberately don't allow other color specifications.

  if ([additional count] > 0) {
    request.additionalParameters = additional;
  }

  CLLocation *location =
      (CLLocation *)[self delegateValueForSelector:@selector(locationInfo)];

  if ((adWhirlConfig.locationOn) && (location)) {
    [request setLocationWithLatitude:location.coordinate.latitude
                           longitude:location.coordinate.longitude
                            accuracy:location.horizontalAccuracy];
  }

  NSString *string =
      (NSString *)[self delegateValueForSelector:@selector(gender)];

  if ([string isEqualToString:@"m"]) {
    request.gender = kGADGenderMale;
  } else if ([string isEqualToString:@"f"]) {
    request.gender = kGADGenderFemale;
  } else {
    request.gender = kGADGenderUnknown;
  }

  if ((value = [self delegateValueForSelector:@selector(dateOfBirth)])) {
    request.birthday = (NSDate *)value;
  }

  if ((value = [self delegateValueForSelector:@selector(keywords)])) {
    NSArray *keywordArray =
        [(NSString *)value componentsSeparatedByString:@" "];
    request.keywords = [NSMutableArray arrayWithArray:keywordArray];
  }

  // Set the frame for this view to match the bounds of the parent adWhirlView.
  GADBannerView *view =
      [[GADBannerView alloc] initWithFrame:adWhirlView.bounds];

  view.adUnitID = [self publisherId];
  view.delegate = self;
  view.rootViewController =
      [adWhirlDelegate viewControllerForPresentingModalView];

  self.adNetworkView = [view autorelease];

  [view loadRequest:request];
}

- (void)stopBeingDelegate {
  if (self.adNetworkView != nil
      && [self.adNetworkView respondsToSelector:@selector(setDelegate:)]) {
    [self.adNetworkView performSelector:@selector(setDelegate:)
         withObject:nil];
  }
}

#pragma mark Ad Request Lifecycle Notifications

// Sent when an ad request loaded an ad.  This is a good opportunity to add
// this view to the hierarchy if it has not yet been added.
- (void)adViewDidReceiveAd:(GADBannerView *)adView {
  [adWhirlView adapter:self didReceiveAdView:adView];
}

// Sent when an ad request failed.  Normally this is because no network
// connection was available or no ads were available (i.e. no fill).
- (void)adView:(GADBannerView *)adView
    didFailToReceiveAdWithError:(GADRequestError *)error {
  [adWhirlView adapter:self didFailAd:error];
}

#pragma mark Click-Time Lifecycle Notifications

// Sent just before presenting the user a full screen view, such as a browser,
// in response to clicking on an ad.  Use this opportunity to stop animations,
// time sensitive interactions, etc.
//
// Normally the user looks at the ad, dismisses it, and control returns to your
// application by calling adViewDidDismissScreen:.  However if the user hits
// the Home button or clicks on an App Store link your application will end.
// On iOS 4.0+ the next method called will be applicationWillResignActive: of
// your UIViewController (UIApplicationWillResignActiveNotification).
// Immediately after that adViewWillLeaveApplication: is called.
- (void)adViewWillPresentScreen:(GADBannerView *)adView {
  [self helperNotifyDelegateOfFullScreenModal];
}

// Sent just after dismissing a full screen view.  Use this opportunity to
// restart anything you may have stopped as part of adViewWillPresentScreen:.
- (void)adViewDidDismissScreen:(GADBannerView *)adView {
  [self helperNotifyDelegateOfFullScreenModalDismissal];
}

#pragma mark parameter gathering methods

- (SEL)delegatePublisherIdSelector {
  return @selector(admobPublisherID);
}

- (NSString *)publisherId {
  SEL delegateSelector = [self delegatePublisherIdSelector];

  if ((delegateSelector) &&
      ([adWhirlDelegate respondsToSelector:delegateSelector])) {
    return [adWhirlDelegate performSelector:delegateSelector];
  }

  return networkConfig.pubId;
}

@end
