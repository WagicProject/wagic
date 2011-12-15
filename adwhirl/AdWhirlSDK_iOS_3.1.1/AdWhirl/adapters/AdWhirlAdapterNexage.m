/*

 AdWhirlAdapterNexage.m
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

#import "AdWhirlAdapterNexage.h"
#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdNetworkRegistry.h"
#import "AdWhirlView.h"
#import "AdWhirlConfig.h"
#import "NexageAdViewController.h"
#import "NexageAdParameters.h"
#import "AdWhirlAdNetworkConfig.h"
#import "AdWhirlError.h"

@implementation AdWhirlAdapterNexage

+ (AdWhirlAdNetworkType)networkType {
  return AdWhirlAdNetworkTypeNexage;
}

+ (void)load {
  [[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
}

- (void)getAd{
  NSDictionary* atts = [NSDictionary dictionaryWithObjectsAndKeys:
    [self dateOfBirth], @"u(dob)",
    [self country], @"u(country)",
    [self city], @"u(city)",
    [self designatedMarketArea], @"u(dma)",
    [self ethnicity], @"u(eth)",
    [self gender], @"u(gender)",
    [NSNumber numberWithDouble:[self houseIncome]], @"u(hhi)",
    [self keywords], @"u(keywords)",
    [self maritalStatus], @"u(marital)",
    [self postCode], @"u(zip)",
    nil];

  NSDictionary* credDict;
  if ([adWhirlDelegate respondsToSelector:@selector(nexageDictionary)]) {
    credDict = [adWhirlDelegate nexageDictionary];
  }
  else {
    credDict = [networkConfig credentials];
  }

  BOOL testMode = NO;
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlTestMode)]
      && [adWhirlDelegate adWhirlTestMode]) {
    testMode = YES;
  }

  // Nexage does weird things with position which can result in an over-release,
  // so we're basically forced to leak this...
  position = [[credDict objectForKey:@"position"] copy];
  if(position == nil){
    [adWhirlView adapter:self didFailAd:nil];
    return;
  }

  adViewController =
      [[NexageAdViewController alloc] initWithDelegate:position delegate:self];
  [adViewController setEnable:YES];


  [adViewController setAttributes:atts];
  [adViewController setTestMode:testMode];
  [adViewController locationAware:adWhirlConfig.locationOn];
#ifdef ADWHIRL_DEBUG
  [adViewController enableLogging:YES];
#endif
  self.adNetworkView = adViewController.view;
}

- (void)stopBeingDelegate {
  if (adViewController != nil) {
    adViewController.delegate = nil;
  }
}

- (void)dealloc {
  [self stopBeingDelegate];
  [adViewController setAttributes:nil];
  [adViewController release];
  adViewController = nil;
  [super dealloc];
}

#pragma mark NexageDelegateProtocol

- (void)adReceived:(UIView *)ad {
  [adWhirlView adapter:self didReceiveAdView:ad];
}
/**
 * This method will be called when user clicks the ad banner.
 * The URL is an optional parameter, if Ad is from the Nexage mediation
 * platform, you will get validate url, if it is nil, that means the action
 * is from integrated sdk. Please check if (url == nil). The return YES, means
 * the sdk will handle click event, otherwise sdk will ignore the user action.
 * Basic Ad network principle should always return YES. Please refer our dev
 * document for details
 */
- (BOOL)adActionShouldBegin:(NSURLRequest *)request
       willLeaveApplication:(BOOL)willLeave {
  [self helperNotifyDelegateOfFullScreenModal];
  return YES;
}

/**
 * The delegate will be called when full screen web browser is closed
 */
- (void)adFullScreenWebBrowserWillClose {
  [self helperNotifyDelegateOfFullScreenModalDismissal];
}
/**
 * identify the ad did not receive at this momnent.
 */
- (void)didFailToReceiveAd {
  [adWhirlView adapter:self didFailAd:nil];
}

- (NSString *)dcnForAd {
  NSDictionary *credDict;
  if ([adWhirlDelegate respondsToSelector:@selector(nexageDictionary)]) {
    credDict = [adWhirlDelegate nexageDictionary];
  }
  else {
    credDict = [networkConfig credentials];
  }
  return [credDict objectForKey:@"dcn"];
}

- (UIViewController*)currentViewController {
  return [adWhirlDelegate viewControllerForPresentingModalView];
}

#pragma mark user profiles

- (NSDate *)dateOfBirth {
  if([adWhirlDelegate respondsToSelector:@selector(dateOfBirth)])
    return [adWhirlDelegate dateOfBirth];
  return nil;
}

- (NSString *)postCode {
  if([adWhirlDelegate respondsToSelector:@selector(postalCode)])
    return [adWhirlDelegate postalCode];
  else return nil;
}

- (NSString *)gender {
  if([adWhirlDelegate respondsToSelector:@selector(gender)])
    return [adWhirlDelegate gender];
  else return nil;
}

- (NSString *)keywords {
  if([adWhirlDelegate respondsToSelector:@selector(keywords)])
    return [adWhirlDelegate keywords];
  else return nil;
}

- (NSInteger)houseIncome {
  if([adWhirlDelegate respondsToSelector:@selector(incomeLevel)])
    return [adWhirlDelegate incomeLevel];
  return 0;
}

- (NSString *)city {
  if([adWhirlDelegate respondsToSelector:@selector(nexageCity)])
    return [adWhirlDelegate nexageCity];
  else return nil;
}

- (NSString *)designatedMarketArea {
  if([adWhirlDelegate respondsToSelector:@selector(nexageDesignatedMarketArea)])
    return [adWhirlDelegate nexageDesignatedMarketArea];
  else return nil;
}

- (NSString *)country {
  if([adWhirlDelegate respondsToSelector:@selector(nexageCountry)])
    return [adWhirlDelegate nexageCountry];
  else return nil;
}

- (NSString *)ethnicity {
  if([adWhirlDelegate respondsToSelector:@selector(nexageEthnicity)])
    return [adWhirlDelegate nexageEthnicity];
  else return nil;
}

- (NSString *)maritalStatus {
  if([adWhirlDelegate respondsToSelector:@selector(nexageMaritalStatus)])
    return [adWhirlDelegate nexageMaritalStatus];
  else return nil;
}

- (NSString *)areaCode {
  if([adWhirlDelegate respondsToSelector:@selector(areaCode)])
    return [adWhirlDelegate areaCode];
  else return nil;
}
@end