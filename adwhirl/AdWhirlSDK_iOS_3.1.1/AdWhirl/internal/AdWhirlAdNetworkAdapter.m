/*

 AdWhirlAdNetworkAdapter.m

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

#import "AdWhirlAdNetworkAdapter.h"
#import "AdWhirlView.h"
#import "AdWhirlConfig.h"
#import "AdWhirlAdNetworkConfig.h"
#import "AdWhirlLog.h"
#import "AdWhirlAdNetworkRegistry.h"

@implementation AdWhirlAdNetworkAdapter

@synthesize adWhirlDelegate;
@synthesize adWhirlView;
@synthesize adWhirlConfig;
@synthesize networkConfig;
@synthesize adNetworkView;

- (id)initWithAdWhirlDelegate:(id<AdWhirlDelegate>)delegate
                         view:(AdWhirlView *)view
                       config:(AdWhirlConfig *)config
                networkConfig:(AdWhirlAdNetworkConfig *)netConf {
  self = [super init];
  if (self != nil) {
    self.adWhirlDelegate = delegate;
    self.adWhirlView = view;
    self.adWhirlConfig = config;
    self.networkConfig = netConf;
  }
  return self;
}

- (void)getAd {
  AWLogCrit(@"Subclass of AdWhirlAdNetworkAdapter must implement -getAd.");
  [self doesNotRecognizeSelector:_cmd];
}

- (void)stopBeingDelegate {
  AWLogCrit(@"Subclass of AdWhirlAdNetworkAdapter must implement -stopBeingDelegate.");
  [self doesNotRecognizeSelector:_cmd];
}

- (BOOL)shouldSendExMetric {
  return YES;
}

- (void)rotateToOrientation:(UIInterfaceOrientation)orientation {
  // do nothing by default. Subclasses implement specific handling.
  AWLogDebug(@"rotate to orientation %d called for adapter %@",
             orientation, NSStringFromClass([self class]));
}

- (BOOL)isBannerAnimationOK:(AWBannerAnimationType)animType {
  return YES;
}

- (void)dealloc {
  [self stopBeingDelegate];
  adWhirlDelegate = nil;
  adWhirlView = nil;
  [adWhirlConfig release], adWhirlConfig = nil;
  [networkConfig release], networkConfig = nil;
  [adNetworkView release], adNetworkView = nil;
  [super dealloc];
}

@end
