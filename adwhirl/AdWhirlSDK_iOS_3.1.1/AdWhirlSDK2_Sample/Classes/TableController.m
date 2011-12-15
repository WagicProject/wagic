/*

 TableController.m

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

#import "AdWhirlSDK2_SampleAppDelegate.h"
#import "TableController.h"
#import "AdWhirlView.h"
#import "SampleConstants.h"


@implementation TableController

@synthesize adView;

- (id)init {
  if (self = [super initWithNibName:@"TableController" bundle:nil]) {
    self.title = @"Ad In Table";
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  self.adView = [AdWhirlView requestAdWhirlViewWithDelegate:self];
  self.adView.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin|UIViewAutoresizingFlexibleRightMargin;
}

/*
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
*/
/*
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}
*/
/*
- (void)viewWillDisappear:(BOOL)animated {
	[super viewWillDisappear:animated];
}
*/
/*
- (void)viewDidDisappear:(BOOL)animated {
	[super viewDidDisappear:animated];
}
*/

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
  return YES;
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
                                duration:(NSTimeInterval)duration {
  [super willRotateToInterfaceOrientation:toInterfaceOrientation duration:duration];
  [self.adView rotateToOrientation:toInterfaceOrientation];
  [self adjustAdSize];
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
  [super didReceiveMemoryWarning];

	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}

- (UILabel *)label {
  return (UILabel *)[self.view viewWithTag:1337];
}

- (UITableView *)table {
  return (UITableView *)[self.view viewWithTag:3337];
}

- (void)adjustAdSize {
  [UIView beginAnimations:@"AdResize" context:nil];
  [UIView setAnimationDuration:0.7];
  CGSize adSize = [adView actualAdSize];
  CGRect newFrame = adView.frame;
  newFrame.size.height = adSize.height;
  newFrame.size.width = adSize.width;
  newFrame.origin.x = (self.view.bounds.size.width - adSize.width)/2;
  adView.frame = newFrame;
  [UIView commitAnimations];
}

- (void)dealloc {
  self.adView.delegate = nil;
  self.adView = nil;
  [super dealloc];
}


#pragma mark Table view methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
  return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
  return 10;
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {

  static NSString *CellIdentifier = @"Cell";
  static NSString *AdCellIdentifier = @"AdCell";

  NSString *cellId = CellIdentifier;
  if (indexPath.row == 0) {
    cellId = AdCellIdentifier;
  }

  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellId];
  if (cell == nil) {
    if ([UITableViewCell instancesRespondToSelector:@selector(initWithStyle:reuseIdentifier:)]) {
      // iPhone SDK 3.0
      cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellId] autorelease];
    }
    else {
      // iPhone SDK 2.2.1
      cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:cellId] autorelease];
    }
    if (cellId == AdCellIdentifier) {
      [cell.contentView addSubview:adView];
    }
  }

  switch (indexPath.row) {
    case 0:
      break;
    case 1:
      if ([cell respondsToSelector:@selector(textLabel)]) {
        // iPhone SDK 3.0
        cell.textLabel.text = @"Request New Ad";
      }
      else {
        // iPhone SDK 2.2.1
        cell.text = @"Request New Ad";
      }
      break;
    case 2:
      if ([cell respondsToSelector:@selector(textLabel)]) {
        // iPhone SDK 3.0
        cell.textLabel.text = @"Roll Over";
      }
      else {
        // iPhone SDK 2.2.1
        cell.text = @"Roll Over";
      }
      break;
    default:
      if ([cell respondsToSelector:@selector(textLabel)]) {
        // iPhone SDK 3.0
        cell.textLabel.text = [NSString stringWithFormat:@"Cell %d", indexPath.row];
      }
      else {
        // iPhone SDK 2.2.1
        cell.text = [NSString stringWithFormat:@"Cell %d", indexPath.row];
      }
  }

  return cell;
}


- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  switch (indexPath.row) {
    case 1:
      self.label.text = @"Request New Ad pressed! Requesting...";
      [adView requestFreshAd];
      break;
    case 2:
      self.label.text = @"Roll Over pressed! Requesting...";
      [adView rollOver];
      break;
  }
  [tableView deselectRowAtIndexPath:indexPath animated:YES];
}


- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
  if (indexPath.section == 0 && indexPath.row == 0) {
    return CGRectGetHeight(adView.bounds);
  }
  return self.table.rowHeight;
}


#pragma mark AdWhirlDelegate methods

- (NSString *)adWhirlApplicationKey {
  return kSampleAppKey;
}

- (UIViewController *)viewControllerForPresentingModalView {
  return [((AdWhirlSDK2_SampleAppDelegate *)[[UIApplication sharedApplication] delegate]) navigationController];
}

- (NSURL *)adWhirlConfigURL {
  return [NSURL URLWithString:kSampleConfigURL];
}

- (NSURL *)adWhirlImpMetricURL {
  return [NSURL URLWithString:kSampleImpMetricURL];
}

- (NSURL *)adWhirlClickMetricURL {
  return [NSURL URLWithString:kSampleClickMetricURL];
}

- (NSURL *)adWhirlCustomAdURL {
  return [NSURL URLWithString:kSampleCustomAdURL];
}

- (void)adWhirlDidReceiveAd:(AdWhirlView *)adWhirlView {
  self.label.text = [NSString stringWithFormat:
                     @"Got ad from %@, size %@",
                     [adWhirlView mostRecentNetworkName],
                     NSStringFromCGSize([adWhirlView actualAdSize])];
  [self adjustAdSize];
}

- (void)adWhirlDidFailToReceiveAd:(AdWhirlView *)adWhirlView usingBackup:(BOOL)yesOrNo {
  self.label.text = [NSString stringWithFormat:
                     @"Failed to receive ad from %@, %@. Error: %@",
                     [adWhirlView mostRecentNetworkName],
                     yesOrNo? @"will use backup" : @"will NOT use backup",
                     adWhirlView.lastError == nil? @"no error" : [adWhirlView.lastError localizedDescription]];
}

- (void)adWhirlReceivedRequestForDeveloperToFufill:(AdWhirlView *)adWhirlView {
  UILabel *replacement = [[UILabel alloc] initWithFrame:kAdWhirlViewDefaultFrame];
  replacement.backgroundColor = [UIColor redColor];
  replacement.textColor = [UIColor whiteColor];
  replacement.textAlignment = UITextAlignmentCenter;
  replacement.text = @"Generic Notification";
  [adWhirlView replaceBannerViewWith:replacement];
  [replacement release];
  [self adjustAdSize];
  self.label.text = @"Generic Notification";
}

- (void)adWhirlDidAnimateToNewAdIn:(AdWhirlView *)adWhirlView {
  [self.table reloadData];
}

- (void)adWhirlReceivedNotificationAdsAreOff:(AdWhirlView *)adWhirlView {
  self.label.text = @"Ads are off";
}

- (void)adWhirlWillPresentFullScreenModal {
  NSLog(@"TableView: will present full screen modal");
}

- (void)adWhirlDidDismissFullScreenModal {
  NSLog(@"TableView: did dismiss full screen modal");
}

- (void)adWhirlDidReceiveConfig:(AdWhirlView *)adWhirlView {
  self.label.text = @"Received config. Requesting ad...";
}

- (BOOL)adWhirlTestMode {
  return NO;
}

- (NSUInteger)jumptapTransitionType {
  return 3;
}

- (NSUInteger)quattroWirelessAdType {
  return 2;
}

- (NSString *)googleAdSenseCompanyName {
  return @"Your Company";
}

- (NSString *)googleAdSenseAppName {
  return @"AdWhirl Sample";
}

- (NSString *)googleAdSenseApplicationAppleID {
  return @"0";
}

- (NSString *)googleAdSenseKeywords {
  return @"apple,iphone,ipad,adwhirl";
}

//extern NSString* const kGADAdSenseImageAdType;
//- (NSString *)googleAdSenseAdType {
//  return kGADAdSenseImageAdType;
//}

#pragma mark event methods

- (void)performEvent {
  self.label.text = @"Event performed";
}

- (void)performEvent2:(AdWhirlView *)adWhirlView {
  UILabel *replacement = [[UILabel alloc] initWithFrame:kAdWhirlViewDefaultFrame];
  replacement.backgroundColor = [UIColor blackColor];
  replacement.textColor = [UIColor whiteColor];
  replacement.textAlignment = UITextAlignmentCenter;
  replacement.text = [NSString stringWithFormat:@"Event performed, view %x", adWhirlView];
  [adWhirlView replaceBannerViewWith:replacement];
  [replacement release];
  [self adjustAdSize];
  self.label.text = [NSString stringWithFormat:@"Event performed, view %x", adWhirlView];
}

@end

