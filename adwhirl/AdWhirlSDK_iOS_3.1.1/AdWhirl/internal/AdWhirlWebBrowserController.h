/*

 AdWhirlWebBrowserController.h

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
#import "AdWhirlCustomAdView.h"

@class AdWhirlWebBrowserController;

@protocol AdWhirlWebBrowserControllerDelegate<NSObject>

- (void)webBrowserClosed:(AdWhirlWebBrowserController *)controller;

@end


@interface AdWhirlWebBrowserController : UIViewController <UIWebViewDelegate> {
  id<AdWhirlWebBrowserControllerDelegate> delegate;
  UIViewController *viewControllerForPresenting;
  NSArray *loadingButtons;
  NSArray *loadedButtons;
  AWCustomAdWebViewAnimType transitionType;

  UIWebView *webView;
  UIToolbar *toolBar;
  UIBarButtonItem *backButton;
  UIBarButtonItem *forwardButton;
  UIBarButtonItem *reloadButton;
  UIBarButtonItem *stopButton;
  UIBarButtonItem *linkOutButton;
  UIBarButtonItem *closeButton;
}

@property (nonatomic,assign) id<AdWhirlWebBrowserControllerDelegate> delegate;
@property (nonatomic,assign) UIViewController *viewControllerForPresenting;
@property (nonatomic,retain) IBOutlet UIWebView *webView;
@property (nonatomic,retain) IBOutlet UIToolbar *toolBar;
@property (nonatomic,retain) IBOutlet UIBarButtonItem *backButton;
@property (nonatomic,retain) IBOutlet UIBarButtonItem *forwardButton;
@property (nonatomic,retain) IBOutlet UIBarButtonItem *reloadButton;
@property (nonatomic,retain) IBOutlet UIBarButtonItem *stopButton;
@property (nonatomic,retain) IBOutlet UIBarButtonItem *linkOutButton;
@property (nonatomic,retain) IBOutlet UIBarButtonItem *closeButton;

- (void)presentWithController:(UIViewController *)viewController transition:(AWCustomAdWebViewAnimType)animType;
- (void)loadURL:(NSURL *)url;
- (IBAction)back:(id)sender;
- (IBAction)forward:(id)sender;
- (IBAction)reload:(id)sender;
- (IBAction)stop:(id)sender;
- (IBAction)linkOut:(id)sender;
- (IBAction)close:(id)sender;

@end

@interface AdWhirlBackButton : UIBarButtonItem
@end

