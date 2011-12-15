/*

 AdWhirlAdNetworkAdapter+Helpers.h
 
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

@interface AdWhirlAdNetworkAdapter (Helpers)

/**
 * Subclasses call this to notify delegate that there's going to be a full
 * screen modal (usually after tap).
 */
- (void)helperNotifyDelegateOfFullScreenModal;

/**
 * Subclasses call this to notify delegate that the full screen modal has
 * been dismissed.
 */
- (void)helperNotifyDelegateOfFullScreenModalDismissal;

/*
 * Subclasses call to get various configs to use, from the AdWhirlDelegate or
 * config from server.
 */
- (UIColor *)helperBackgroundColorToUse;
- (UIColor *)helperTextColorToUse;
- (UIColor *)helperSecondaryTextColorToUse;
- (NSInteger)helperCalculateAge;

@end
