/*

 AdWhirlAdNetworkAdapter+Helpers.m
 
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

#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlView.h"
#import "AdWhirlView+.h"
#import "AdWhirlConfig.h"
#import "AdWhirlAdNetworkConfig.h"

@implementation AdWhirlAdNetworkAdapter (Helpers)

- (void)helperNotifyDelegateOfFullScreenModal {
  // don't request new ad when modal view is on
  adWhirlView.showingModalView = YES;
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlWillPresentFullScreenModal)]) {
    [adWhirlDelegate adWhirlWillPresentFullScreenModal];
  }
}

- (void)helperNotifyDelegateOfFullScreenModalDismissal {
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlDidDismissFullScreenModal)]) {
    [adWhirlDelegate adWhirlDidDismissFullScreenModal];
  }
  adWhirlView.showingModalView = NO;
}

- (UIColor *)helperBackgroundColorToUse {
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlAdBackgroundColor)]) {
    UIColor *color = [adWhirlDelegate adWhirlAdBackgroundColor];
    if (color != nil) return color;
  }
  if ([adWhirlDelegate respondsToSelector:@selector(backgroundColor)]) {
    UIColor *color = [adWhirlDelegate backgroundColor];
    if (color != nil) return color;
  }
  return adWhirlConfig.backgroundColor;
}

- (UIColor *)helperTextColorToUse {
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlTextColor)]) {
    UIColor *color = [adWhirlDelegate adWhirlTextColor];
    if (color != nil) return color;
  }
  if ([adWhirlDelegate respondsToSelector:@selector(textColor)]) {
    UIColor *color = [adWhirlDelegate textColor];
    if (color != nil) return color;
  }
  return adWhirlConfig.textColor;
}

- (UIColor *)helperSecondaryTextColorToUse {
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlSecondaryTextColor)]) {
    UIColor *color = [adWhirlDelegate adWhirlSecondaryTextColor];
    if (color != nil) return color;
  }
  return nil;
}

- (NSInteger)helperCalculateAge {
  NSDate *birth = [adWhirlDelegate dateOfBirth];
  if (birth == nil) {
    return -1;
  }
  NSDate *today = [[NSDate alloc] init];
  NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
  NSDateComponents *components = [gregorian components:NSYearCalendarUnit
                                              fromDate:birth
                                                toDate:today
                                               options:0];
  NSInteger years = [components year];
  [gregorian release];
  [today release];
  return years;
}

@end
