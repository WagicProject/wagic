/*
 
 AdWhirlAdapterGreystripe.m
 
 Copyright 2010 Greystripe, Inc.
 
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
#import "GreystripeDelegate.h"

/**
 * Banner slot name used to identify the banner ad slot within the Greystripe 
 * SDK.
 */
extern NSString * const kGSBannerSlotName;

/**
 * Full-screen slot name used to identify the full-screen ad slot within the
 * Greystripe SDK. Use this slot name to display full-screen ads as follows:
 *
 * [GSAdEngine displayFullScreenAdForSlotNamed:kGSFullScreenSlotName];
 * 
 * If you need to check whether an ad is available for this slot, simply use:
 * 
 * [GSAdEngine isAdReadyForSlotNamed:kGSFullScreenSlotName];
 */
extern NSString * const kGSFullScreenSlotName;

@class GSAdView;

@interface AdWhirlAdapterGreystripe : AdWhirlAdNetworkAdapter <GreystripeDelegate> {
  UIView *innerContainer;
  UIView *outerContainer;
}

@end
