/*

 SimpleViewController.h
 
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

#import <UIKit/UIKit.h>
#import "AdWhirlDelegateProtocol.h"

@class AdWhirlView;
@interface SimpleViewController : UIViewController <AdWhirlDelegate> {
  AdWhirlView *adView;
  UIInterfaceOrientation currLayoutOrientation;
}

- (IBAction)requestNewAd:(id)sender;
- (IBAction)requestNewConfig:(id)sender;
- (IBAction)rollOver:(id)sender;
- (IBAction)showModalView:(id)sender;
- (IBAction)toggleRefreshAd:(id)sender;
- (void)adjustLayoutToOrientation:(UIInterfaceOrientation)newOrientation;
- (void)adjustAdSize;

@property (nonatomic,retain) AdWhirlView *adView;
@property (nonatomic,readonly) UILabel *label;

@end
