/*

 AdWhirlAdNetworkAdapter.h

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

#import "AdWhirlDelegateProtocol.h"
#import "AdWhirlConfig.h"

typedef enum {
  AdWhirlAdNetworkTypeAdMob             = 1,
  AdWhirlAdNetworkTypeJumpTap           = 2,
  AdWhirlAdNetworkTypeVideoEgg          = 3,
  AdWhirlAdNetworkTypeMedialets         = 4,
  AdWhirlAdNetworkTypeLiveRail          = 5,
  AdWhirlAdNetworkTypeMillennial        = 6,
  AdWhirlAdNetworkTypeGreyStripe        = 7,
  AdWhirlAdNetworkTypeQuattro           = 8,
  AdWhirlAdNetworkTypeCustom            = 9,
  AdWhirlAdNetworkTypeAdWhirl10         = 10,
  AdWhirlAdNetworkTypeMobClix           = 11,
  AdWhirlAdNetworkTypeMdotM             = 12,
  AdWhirlAdNetworkTypeAdWhirl13         = 13,
  AdWhirlAdNetworkTypeGoogleAdSense     = 14,
  AdWhirlAdNetworkTypeGoogleDoubleClick = 15,
  AdWhirlAdNetworkTypeGeneric           = 16,
  AdWhirlAdNetworkTypeEvent             = 17,
  AdWhirlAdNetworkTypeInMobi            = 18,
  AdWhirlAdNetworkTypeIAd               = 19,
  AdWhirlAdNetworkTypeZestADZ           = 20,
  AdWhirlAdNetworkTypeBrightRoll        = 21,
  AdWhirlAdNetworkTypeTapAd             = 22,
  AdWhirlAdNetworkTypeOneRiot           = 23,
  AdWhirlAdNetworkTypeNexage            = 24
} AdWhirlAdNetworkType;

@class AdWhirlView;
@class AdWhirlConfig;
@class AdWhirlAdNetworkConfig;

@interface AdWhirlAdNetworkAdapter : NSObject {
  id<AdWhirlDelegate> adWhirlDelegate;
  AdWhirlView *adWhirlView;
  AdWhirlConfig *adWhirlConfig;
  AdWhirlAdNetworkConfig *networkConfig;
  UIView *adNetworkView;
}

/**
 * Subclasses must implement +networkType to return an AdWhirlAdNetworkType enum.
 */
//+ (AdWhirlAdNetworkType)networkType;

/**
 * Subclasses must add itself to the AdWhirlAdNetworkRegistry. One way
 * to do so is to implement the +load function and register there.
 */
//+ (void)load;

/**
 * Default initializer. Subclasses do not need to override this method unless
 * they need to perform additional initialization. In which case, this
 * method must be called via the super keyword.
 */
- (id)initWithAdWhirlDelegate:(id<AdWhirlDelegate>)delegate
                         view:(AdWhirlView *)view
                       config:(AdWhirlConfig *)config
                networkConfig:(AdWhirlAdNetworkConfig *)netConf;

/**
 * Ask the adapter to get an ad. This must be implemented by subclasses.
 */
- (void)getAd;

/**
 * When called, the adapter must remove itself as a delegate or notification
 * observer from the underlying ad network SDK. Subclasses must implement this
 * method, even if the underlying SDK doesn't have a way of removing delegate
 * (in which case, you should contact the ad network). Note that this method
 * will be called in dealloc at AdWhirlAdNetworkAdapter, before adNetworkView
 * is released. Care must be taken if you also keep a reference of your ad view
 * in a separate instance variable, as you may have released that variable
 * before this gets called in AdWhirlAdNetworkAdapter's dealloc. Use
 * adNetworkView, defined in this class, instead of your own instance variable.
 * This function should also be idempotent, i.e. get called multiple times and
 * not crash.
 */
- (void)stopBeingDelegate;

/**
 * Subclasses return YES to ask AdWhirlView to send metric requests to the
 * AdWhirl server for ad impressions. Default is YES.
 */
- (BOOL)shouldSendExMetric;

/**
 * Tell the adapter that the interface orientation changed or is about to change
 */
- (void)rotateToOrientation:(UIInterfaceOrientation)orientation;

/**
 * Some ad transition types may cause issues with particular ad networks. The
 * adapter should know whether the given animation type is OK. Defaults to
 * YES.
 */
- (BOOL)isBannerAnimationOK:(AWBannerAnimationType)animType;

@property (nonatomic,assign) id<AdWhirlDelegate> adWhirlDelegate;
@property (nonatomic,assign) AdWhirlView *adWhirlView;
@property (nonatomic,retain) AdWhirlConfig *adWhirlConfig;
@property (nonatomic,retain) AdWhirlAdNetworkConfig *networkConfig;
@property (nonatomic,retain) UIView *adNetworkView;

@end
