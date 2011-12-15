/*

 ARRollerView.m
 
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

#import "ARRollerView.h"
#import "AdWhirlView+.h"

@interface ARRollerView ()
- (id)initWithDelegate:(id<ARRollerDelegate>)delegate;
@end

@implementation ARRollerView

+ (ARRollerView*)requestRollerViewWithDelegate:(id<ARRollerDelegate>)delegate {
  return [[[ARRollerView alloc] initWithDelegate:delegate] autorelease];
}

- (id)initWithDelegate:(id<ARRollerDelegate>)d {
  return [super initWithDelegate:d];
}

- (void)getNextAd {
  [self requestFreshAd];
}

- (void)setDelegateToNil {
  self.delegate = nil;
}

@end
