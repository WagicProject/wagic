/*

 AdWhirlAdapterQuattro.m

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

#import "AdWhirlAdapterQuattro.h"
#import "AdWhirlView.h"
#import "AdWhirlConfig.h"
#import "AdWhirlAdNetworkConfig.h"
#import "QWAdView.h"
#import "QWTestMode.h"
#import "AdWhirlLog.h"
#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdNetworkRegistry.h"

@implementation AdWhirlAdapterQuattro

+ (AdWhirlAdNetworkType)networkType {
  return AdWhirlAdNetworkTypeQuattro;
}

+ (void)load {
  [[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
}

- (void)getAd {
  QWEnableLocationServicesForAds(adWhirlConfig.locationOn);

  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlTestMode)]
      && [adWhirlDelegate adWhirlTestMode]) {
    QWSetTestMode(YES);
    QWSetLogging(YES);
  }
  else {
    QWSetTestMode(NO);
    QWSetLogging(NO);
  }
  NSDictionary *credDict;
  if ([adWhirlDelegate respondsToSelector:@selector(quattroWirelessDictionary)]) {
    credDict = [adWhirlDelegate quattroWirelessDictionary];
  }
  else {
    credDict = [networkConfig credentials];
  }
  NSString *pubId = [credDict objectForKey:@"publisherID"];
  NSString *siteId = [credDict objectForKey:@"siteID"];
  QWAdType adType = QWAdTypeBanner;
  if ([adWhirlDelegate respondsToSelector:@selector(quattroWirelessAdType)]) {
    adType = (QWAdType)[adWhirlDelegate quattroWirelessAdType];
  }
  UIDeviceOrientation orientation;
  if ([self.adWhirlDelegate respondsToSelector:@selector(adWhirlCurrentOrientation)]) {
    orientation = [self.adWhirlDelegate adWhirlCurrentOrientation];
  }
  else {
    orientation = [UIDevice currentDevice].orientation;
  }
  QWAdView *quattroAd = [QWAdView adViewWithType:adType
                                     publisherID:pubId
                                          siteID:siteId
                                     orientation:orientation
                                        delegate:self];
  quattroAd.textColor = [self helperTextColorToUse];
  quattroAd.backgroundColor = [self helperBackgroundColorToUse];
  [quattroAd displayNewAd];
  self.adNetworkView = quattroAd;
}

- (void)stopBeingDelegate {
  QWAdView *quattroAd = (QWAdView *)self.adNetworkView;
  if (quattroAd != nil) {
    quattroAd.delegate = nil;
  }
}

- (void)dealloc {
  [super dealloc];
}

#pragma mark QWAdViewDelegate methods

- (void)adView:(QWAdView *)adView didDisplayAd:(QWAd *)ad {
  // somehow the test banner ad is showing 80 pixels as height sometimes.
  // check for that and adjust
  AWLogDebug(@"Quattro reported frame %@", NSStringFromCGRect(self.adNetworkView.frame));
  if (self.adNetworkView.frame.size.height > 50.0) {
    CGRect f = adNetworkView.frame;
    f.size.height = 50;
    adNetworkView.frame = f;
  }
  [adWhirlView adapter:self didReceiveAdView:adView];
}

- (void)dispatchError:(NSError*)error
{
  [adWhirlView adapter:self didFailAd:error];
}

- (void)adView:(QWAdView *)adView failedWithError:(NSError *)error {
  [self performSelectorOnMainThread:@selector(dispatchError:) withObject:error waitUntilDone:NO];
}

- (void)adView:(QWAdView *)adView displayLandingPage:(UIViewController *)controller {
  [self helperNotifyDelegateOfFullScreenModal];
  [[adWhirlDelegate viewControllerForPresentingModalView] presentModalViewController:controller
                                                                            animated:YES];
}

- (void)adView:(QWAdView *)adView dismiss:(UIViewController *)controller {
  [[adWhirlDelegate viewControllerForPresentingModalView] dismissModalViewControllerAnimated:YES];
  [self helperNotifyDelegateOfFullScreenModalDismissal];
}

#pragma mark QWAdViewDelegate optional methods

- (BOOL)respondsToSelector:(SEL)selector {
  if (selector == @selector(latitude:)
      && ![adWhirlDelegate respondsToSelector:@selector(locationInfo)]) {
    return NO;
  }
  else if (selector == @selector(longitude:)
           && ![adWhirlDelegate respondsToSelector:@selector(locationInfo)]) {
    return NO;
  }
  else if (selector == @selector(age:)
           && (![adWhirlDelegate respondsToSelector:@selector(dateOfBirth)]
               || [self age:nil] < 0)) {
    return NO;
  }
  else if (selector == @selector(zipcode:)
           && ![adWhirlDelegate respondsToSelector:@selector(postalCode)]) {
    return NO;
  }
  else if (selector == @selector(gender:)
           && ![adWhirlDelegate respondsToSelector:@selector(gender)]) {
    return NO;
  }
  else if (selector == @selector(income:)
           && ![adWhirlDelegate respondsToSelector:@selector(incomeLevel)]) {
    return NO;
  }
  else if (selector == @selector(education:)
           && ![adWhirlDelegate respondsToSelector:@selector(quattroWirelessEducationLevel)]) {
    return NO;
  }
  else if (selector == @selector(birthdate:)
           && ![adWhirlDelegate respondsToSelector:@selector(dateOfBirth)]) {
    return NO;
  }
  else if (selector == @selector(ethnicity:)
           && ![adWhirlDelegate respondsToSelector:@selector(quattroWirelessEthnicity)]) {
    return NO;
  }
  return [super respondsToSelector:selector];
}

- (double)latitude:(QWAdView *)adView {
  CLLocation *loc = [adWhirlDelegate locationInfo];
  if (loc == nil) return 0.0;
  return loc.coordinate.latitude;
}

- (double)longitude:(QWAdView *)adView {
  CLLocation *loc = [adWhirlDelegate locationInfo];
  if (loc == nil) return 0.0;
  return loc.coordinate.longitude;
}

- (NSString *)zipcode:(QWAdView *)adView {
  return [adWhirlDelegate postalCode];
}

- (NSUInteger)age:(QWAdView *)adView {
  return [self helperCalculateAge];
}

- (QWGender)gender:(QWAdView *)adView {
  NSString *gender = [adWhirlDelegate gender];
  QWGender sex = QWGenderUnknown;
  if (gender == nil)
    return sex;
  if ([gender compare:@"m"] == NSOrderedSame) {
    sex = QWGenderMale;
  }
  else if ([gender compare:@"f"] == NSOrderedSame) {
    sex = QWGenderFemale;
  }
  return sex;
}

- (NSUInteger)income:(QWAdView *)adView {
  return [adWhirlDelegate incomeLevel];
}

- (QWEducationLevel)education:(QWAdView *)adView {
  return [adWhirlDelegate quattroWirelessEducationLevel];
}

- (NSDate *)birthdate:(QWAdView *)adView {
  return [adWhirlDelegate dateOfBirth];
}

- (QWEthnicity)ethnicity:(QWAdView *)adView {
  return [adWhirlDelegate quattroWirelessEthnicity];
}

@end
