/*

 AdWhirlCustomAdView.h

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

typedef enum {
  AWCustomAdTypeMIN = 0,
  AWCustomAdTypeBanner    = 1,
  AWCustomAdTypeText      = 2,
  AWCustomAdTypeAutoLaunchFallBackBanner = 3,
  AWCustomAdTypeAutoLaunchFallBackText   = 4,
  AWCustomAdTypeSearchBar = 5,
  AWCustomAdTypeMAX = 6
} AWCustomAdType;

typedef enum {
  AWCustomAdLaunchTypeMIN = 0,
  AWCustomAdLaunchTypeSafari   = 1,
  AWCustomAdLaunchTypeCanvas   = 2,
  AWCustomAdLaunchTypeSafariRedirectFollowThrough = 3,
  AWCustomAdLaunchTypeMAX = 4
} AWCustomAdLaunchType;

typedef enum {
  AWCustomAdWebViewAnimTypeMIN = -1,
  AWCustomAdWebViewAnimTypeNone           = 0,
  AWCustomAdWebViewAnimTypeFlipFromLeft   = 1,
  AWCustomAdWebViewAnimTypeFlipFromRight  = 2,
  AWCustomAdWebViewAnimTypeCurlUp         = 3,
  AWCustomAdWebViewAnimTypeCurlDown       = 4,
  AWCustomAdWebViewAnimTypeSlideFromLeft  = 5,
  AWCustomAdWebViewAnimTypeSlideFromRight = 6,
  AWCustomAdWebViewAnimTypeFadeIn         = 7,
  AWCustomAdWebViewAnimTypeModal          = 8,
  AWCustomAdWebViewAnimTypeRandom         = 9,
  AWCustomAdWebViewAnimTypeMAX = 10
} AWCustomAdWebViewAnimType;

@class AdWhirlCustomAdView;

@protocol AdWhirlCustomAdViewDelegate<NSObject>

- (void)adTapped:(AdWhirlCustomAdView *)adView;

@end


@interface AdWhirlCustomAdView : UIButton
{
  id<AdWhirlCustomAdViewDelegate> delegate;
  UIImage *image;
  UILabel *textLabel;
  NSURL *redirectURL;
  NSURL *clickMetricsURL;
  AWCustomAdType adType;
  AWCustomAdLaunchType launchType;
  AWCustomAdWebViewAnimType animType;
  UIColor *backgroundColor;
  UIColor *textColor;
}

- (id)initWithDelegate:(id<AdWhirlCustomAdViewDelegate>)delegate
                  text:(NSString *)text
           redirectURL:(NSURL *)redirectURL
       clickMetricsURL:(NSURL *)clickMetricsURL
                adType:(AWCustomAdType)adType
            launchType:(AWCustomAdLaunchType)launchType
              animType:(AWCustomAdWebViewAnimType)animType
       backgroundColor:(UIColor *)bgColor
             textColor:(UIColor *)fgColor;

@property (nonatomic,assign) id<AdWhirlCustomAdViewDelegate> delegate;
@property (nonatomic,retain) UIImage *image;
@property (nonatomic,readonly) UILabel *textLabel;
@property (nonatomic,readonly) NSURL *redirectURL;
@property (nonatomic,readonly) NSURL *clickMetricsURL;
@property (nonatomic,readonly) AWCustomAdType adType;
@property (nonatomic,readonly) AWCustomAdLaunchType launchType;
@property (nonatomic,readonly) AWCustomAdWebViewAnimType animType;
@property (nonatomic,readonly) UIColor *backgroundColor;
@property (nonatomic,readonly) UIColor *textColor;

@end
