/*

 AdWhirlAdapterInMobi.m

 Copyright 2010 AdMob, Inc.

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

#import "AdWhirlAdapterInMobi.h"
#import "AdWhirlAdNetworkConfig.h"
#import "AdWhirlView.h"
#import "IMAdView.h"
#import "IMAdRequest.h"
#import "AdWhirlLog.h"
#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdNetworkRegistry.h"

@implementation AdWhirlAdapterInMobi

+ (AdWhirlAdNetworkType)networkType {
  return AdWhirlAdNetworkTypeInMobi;
}

+ (void)load {
  [[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
}

- (void)getAd {
  IMAdView *inMobiView = [[IMAdView alloc]
                             initWithFrame:kAdWhirlViewDefaultFrame
                             imAppId:[self siteId]
                             imAdUnit:IM_UNIT_320x50
                             rootViewController:[self rootViewControllerForAd]];
  [inMobiView autorelease];
  inMobiView.refreshInterval = REFRESH_INTERVAL_OFF;
  inMobiView.delegate = self;
  self.adNetworkView = inMobiView;

  IMAdRequest *request = [IMAdRequest request];

  if ([self testMode]) {
    request.testMode = true;
  }
  if ([adWhirlDelegate respondsToSelector:@selector(postalCode)]) {
    request.postalCode = [adWhirlDelegate postalCode];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(areaCode)]) {
    request.areaCode = [adWhirlDelegate areaCode];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(dateOfBirth)]) {
    NSDateFormatter *formatter = [[[NSDateFormatter alloc] init] autorelease];
    [formatter setDateFormat:@"dd-MM-yyyy"];
    request.dateOfBirth = [formatter
                              stringFromDate:[adWhirlDelegate dateOfBirth]];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(gender)]) {
    if ([adWhirlDelegate gender] == @"m") {
      request.gender = G_M;
    } else if ([adWhirlDelegate gender] == @"f") {
      request.gender = G_F;
    } else {
      request.gender = G_None;
    }
  }
  if ([adWhirlDelegate respondsToSelector:@selector(keywords)]) {
    request.keywords = [adWhirlDelegate keywords];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(searchString)]) {
    request.searchString = [adWhirlDelegate searchString];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(incomeLevel)]) {
    request.income = [adWhirlDelegate incomeLevel];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(inMobiEducation)]) {
    request.education = [adWhirlDelegate inMobiEducation];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(inMobiEthnicity)]) {
    request.ethnicity = [adWhirlDelegate inMobiEthnicity];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(dateOfBirth)]) {
    request.age = [self helperCalculateAge];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(inMobiInterests)]) {
    request.interests = [adWhirlDelegate inMobiInterests];
  }
  if ([adWhirlDelegate respondsToSelector:@selector(inMobiParamsDictionary)]) {
    request.paramsDictionary = [adWhirlDelegate inMobiParamsDictionary];
  }
  if (!adWhirlConfig.locationOn) {
    request.isLocationEnquiryAllowed = false;
  }

  [inMobiView loadIMAdRequest:request];
}

- (void)stopBeingDelegate {
  InMobiAdView *inMobiView = (InMobiAdView *)self.adNetworkView;
  if (inMobiView != nil) {
    [inMobiView setDelegate:nil];
  }
}

- (void)dealloc {
  [super dealloc];
}

#pragma mark IMAdView helper methods

- (NSString *)siteId {
  if ([adWhirlDelegate respondsToSelector:@selector(inMobiAppId)]) {
    return [adWhirlDelegate inMobiAppID];
  }
  return networkConfig.pubId;
}

- (UIViewController *)rootViewControllerForAd {
  return [adWhirlDelegate viewControllerForPresentingModalView];
}

- (BOOL)testMode {
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlTestMode)])
    return [adWhirlDelegate adWhirlTestMode];
  return NO;
}

- (Gender)gender {
  if ([adWhirlDelegate respondsToSelector:@selector(gender)]) {
    NSString *genderStr = [adWhirlDelegate gender];
    if ([genderStr isEqualToString:@"f"]) {
      return G_F;
    } else if ([genderStr isEqualToString:@"m"]) {
      return G_M;
    }
  }
  return G_None;
}

#pragma mark IMAdDelegate methods

- (void)adViewDidFinishRequest:(IMAdView *)adView {
  [adWhirlView adapter:self didReceiveAdView:adView];
}

- (void)adView:(IMAdView *)view didFailRequestWithError:(IMAdError *)error {
  NSLog(@"Error %@", error);
  [adWhirlView adapter:self didFailAd:nil];
}

- (void)adViewWillPresentScreen:(IMAdView *)adView {
  [self helperNotifyDelegateOfFullScreenModal];
}

- (void)adViewWillDismissScreen:(IMAdView *)adView {
  [self helperNotifyDelegateOfFullScreenModalDismissal];
}

- (void)adViewWillLeaveApplication:(IMAdView *)adView {
  [self helperNotifyDelegateOfFullScreenModal];
}

@end
