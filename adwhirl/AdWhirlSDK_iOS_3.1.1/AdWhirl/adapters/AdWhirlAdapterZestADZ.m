/*

 AdWhirlAdapterZestADZ.m

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

#import "AdWhirlAdapterZestADZ.h"
#import "AdWhirlAdNetworkConfig.h"
#import "AdWhirlView.h"
#import "ZestadzView.h"
#import "AdWhirlLog.h"
#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdNetworkRegistry.h"

@implementation AdWhirlAdapterZestADZ

+ (AdWhirlAdNetworkType)networkType {
  return AdWhirlAdNetworkTypeZestADZ;
}

+ (void)load {
  [[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
}

- (void)getAd {
  ZestadzView *zestView = [ZestadzView requestAdWithDelegate:self];
  self.adNetworkView = zestView;
}

- (void)stopBeingDelegate {
  // no way to set zestView's delegate to nil
}

- (void)dealloc {
  [super dealloc];
}

#pragma mark ZestadzDelegate required methods.

- (NSString *)clientId {
	if ([adWhirlDelegate respondsToSelector:@selector(zestADZClientID)]) {
		return [adWhirlDelegate zestADZClientID];
	}
	return networkConfig.pubId;
}

- (UIViewController *)currentViewController {
  return [adWhirlDelegate viewControllerForPresentingModalView];
}

#pragma mark ZestadzDelegate notification methods

- (void)didReceiveAd:(ZestadzView *)adView {
  [adWhirlView adapter:self didReceiveAdView:adView];
}

- (void)didFailToReceiveAd:(ZestadzView *)adView {
  [adWhirlView adapter:self didFailAd:nil];
}

- (void)willPresentFullScreenModal {
  [self helperNotifyDelegateOfFullScreenModal];
}

- (void)didDismissFullScreenModal {
  [self helperNotifyDelegateOfFullScreenModalDismissal];
}

#pragma mark ZestadzDelegate config methods
- (UIColor *)adBackgroundColor {
	if ([adWhirlDelegate respondsToSelector:@selector(adWhirlAdBackgroundColor)]) {
		return [adWhirlDelegate adWhirlAdBackgroundColor];
	}

    return nil;
}

- (NSString *)keywords {
	if ([adWhirlDelegate respondsToSelector:@selector(keywords)]) {
		return [adWhirlDelegate keywords];
	}

  return @"iphone ipad ipod";
}

@end

