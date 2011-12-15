/*

 AdWhirlConfig.h

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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "CJSONDeserializer.h"

@class AdWhirlConfig;
@protocol AdWhirlConfigDelegate<NSObject>

@optional
- (void)adWhirlConfigDidReceiveConfig:(AdWhirlConfig *)config;
- (void)adWhirlConfigDidFail:(AdWhirlConfig *)config error:(NSError *)error;
- (NSURL *)adWhirlConfigURL;

@end

typedef enum {
  AWBannerAnimationTypeNone           = 0,
  AWBannerAnimationTypeFlipFromLeft   = 1,
  AWBannerAnimationTypeFlipFromRight  = 2,
  AWBannerAnimationTypeCurlUp         = 3,
  AWBannerAnimationTypeCurlDown       = 4,
  AWBannerAnimationTypeSlideFromLeft  = 5,
  AWBannerAnimationTypeSlideFromRight = 6,
  AWBannerAnimationTypeFadeIn         = 7,
  AWBannerAnimationTypeRandom         = 8,
} AWBannerAnimationType;

@class AdWhirlAdNetworkConfig;
@class AdWhirlAdNetworkRegistry;

@interface AdWhirlConfig : NSObject {
  NSString *appKey;
  NSURL *configURL;
  BOOL legacy;

  BOOL adsAreOff;
  NSMutableArray *adNetworkConfigs;

  UIColor *backgroundColor;
  UIColor *textColor;
  NSTimeInterval refreshInterval;
  BOOL locationOn;
  AWBannerAnimationType bannerAnimationType;
  NSInteger fullscreenWaitInterval;
  NSInteger fullscreenMaxAds;

  NSMutableArray *delegates;
  BOOL hasConfig;

  AdWhirlAdNetworkRegistry *adNetworkRegistry;
}

- (id)initWithAppKey:(NSString *)ak delegate:(id<AdWhirlConfigDelegate>)delegate;
- (BOOL)parseConfig:(NSData *)data error:(NSError **)error;
- (BOOL)addDelegate:(id<AdWhirlConfigDelegate>)delegate;
- (BOOL)removeDelegate:(id<AdWhirlConfigDelegate>)delegate;
- (void)notifyDelegatesOfFailure:(NSError *)error;

@property (nonatomic,readonly) NSString *appKey;
@property (nonatomic,readonly) NSURL *configURL;

@property (nonatomic,readonly) BOOL hasConfig;

@property (nonatomic,readonly) BOOL adsAreOff;
@property (nonatomic,readonly) NSArray *adNetworkConfigs;
@property (nonatomic,readonly) UIColor *backgroundColor;
@property (nonatomic,readonly) UIColor *textColor;
@property (nonatomic,readonly) NSTimeInterval refreshInterval;
@property (nonatomic,readonly) BOOL locationOn;
@property (nonatomic,readonly) AWBannerAnimationType bannerAnimationType;
@property (nonatomic,readonly) NSInteger fullscreenWaitInterval;
@property (nonatomic,readonly) NSInteger fullscreenMaxAds;

@property (nonatomic,assign) AdWhirlAdNetworkRegistry *adNetworkRegistry;

@end


// Convenience conversion functions, converts val into native types var.
// val can be NSNumber or NSString, all else will cause function to fail
// On failure, return NO.
BOOL awIntVal(NSInteger *var, id val);
BOOL awFloatVal(CGFloat *var, id val);
BOOL awDoubleVal(double *var, id val);
