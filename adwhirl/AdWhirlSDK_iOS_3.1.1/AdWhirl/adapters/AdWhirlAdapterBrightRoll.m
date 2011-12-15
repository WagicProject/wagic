/*

 AdWhirlAdapterBrightRoll.m

 Copyright 2009 BrightRoll, Inc.

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

#import "AdWhirlAdapterBrightRoll.h"
#import "AdWhirlAdNetworkAdapter.h"
#import "AdWhirlAdNetworkRegistry.h"
#import "AdWhirlView.h"
#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdNetworkConfig.h"

@implementation AdWhirlAdapterBrightRoll

@synthesize brBannerAd;

+ (AdWhirlAdNetworkType)networkType
{
  return AdWhirlAdNetworkTypeBrightRoll;
}

+ (void)load
{
  [[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
}

- (void)stopObserving
{
  [[NSNotificationCenter defaultCenter]
    removeObserver:self
    name:@"AdWhirlViewWillAnimateToNewAd"
    object:self.adWhirlView];
}

- (void)stopBeingDelegate
{
  [self stopObserving];
  self.brBannerAd.delegate = nil;
  self.brBannerAd = nil;
}

- (void)getAd
{
  self.brBannerAd = [BRBannerAd fetchWithDelegate:self];
}

#pragma mark BRBannerAdDelegate required methods

- (NSString *)brBannerAdAppId:(BRBannerAd *)theBrBannerAd
{
  if ([adWhirlDelegate respondsToSelector:@selector(brightRollAppId)])
  {
    return [adWhirlDelegate brightRollAppId];
  }
  return networkConfig.pubId;
}

- (void)brBannerAdFetched:(BRBannerAd *)theBrBannerAd
{
  [[NSNotificationCenter defaultCenter]
   addObserver:self
   selector:@selector(adWhirlWillAnimateToNewAdIn:)
   name:@"AdWhirlViewWillAnimateToNewAd"
   object:self.adWhirlView];

  brBannerAd.fullScreenAd.delegate = self;
  self.adNetworkView = brBannerAd.view;
  [self.adWhirlView adapter:self didReceiveAdView:brBannerAd.view];
}

- (void)brBannerAdFetchFailed:(BRBannerAd *)bannerAd
{
  [adWhirlView
    adapter:self
    didFailAd:[NSError
           errorWithDomain:@"com.brightroll.BrightRoll_iPhone_SDK"
           code:404
           userInfo:[NSDictionary dictionary]]];
}

- (void)brBannerAdWillShowFullScreenAd:(BRBannerAd *)bannerAd
{
  [self helperNotifyDelegateOfFullScreenModal];
}

#pragma mark AdWhirlView notification methods

- (void)adWhirlWillAnimateToNewAdIn:(NSNotification *)notification
{
  if ([self.adWhirlView performSelector:@selector(currAdapter)] == self)
  {
    [self stopObserving];
    [self helperNotifyDelegateOfFullScreenModal];
    [brBannerAd.fullScreenAd show];
  }
}

#pragma mark BRFullScreenAdDelegate required methods

- (UIViewController *)brFullScreenAdControllerParent
{
  return [self.adWhirlDelegate viewControllerForPresentingModalView];
}

- (void)brFullScreenAdDismissed:(BRFullScreenAd *)brFullScreenAd
{
  [self helperNotifyDelegateOfFullScreenModalDismissal];
}

- (NSString *)brightRollAppId
{
  NSString *appId = [self.networkConfig.credentials objectForKey:@"pubid"];
  
  if (!appId)
  {
    appId = [self.adWhirlDelegate brightRollAppId];
  }
  
  return appId;
}

@end
