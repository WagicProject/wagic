/*

 AdWhirlView+.h

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


@class AdWhirlConfigStore;


@interface AdWhirlView ()

// Only initializes default values for member variables
- (id)initWithDelegate:(id<AdWhirlDelegate>)delegate;

// Kicks off getting config from AdWhirlConfigStore
- (void)startGetConfig;

- (void)buildPrioritizedAdNetCfgsAndMakeRequest;
- (AdWhirlAdNetworkConfig *)nextNetworkCfgByPercent;
- (AdWhirlAdNetworkConfig *)nextNetworkCfgByPriority;
- (void)makeAdRequest:(BOOL)isFirstRequest;
- (void)reportExImpression:(NSString *)nid netType:(AdWhirlAdNetworkType)type;
- (void)reportExClick:(NSString *)nid netType:(AdWhirlAdNetworkType)type;
- (BOOL)canRefresh;
- (void)resignActive:(NSNotification *)notification;
- (void)becomeActive:(NSNotification *)notification;

- (void)notifyDelegateOfErrorWithCode:(NSInteger)errorCode
                          description:(NSString *)desc;
- (void)notifyDelegateOfError:(NSError *)error;

@property (retain) AdWhirlConfig *config;
@property (retain) NSMutableArray *prioritizedAdNetCfgs;
@property (nonatomic,retain) AdWhirlAdNetworkAdapter *currAdapter;
@property (nonatomic,retain) AdWhirlAdNetworkAdapter *lastAdapter;
@property (nonatomic,retain) NSDate *lastRequestTime;
@property (nonatomic,retain) NSTimer *refreshTimer;
@property (nonatomic) BOOL showingModalView;
@property (nonatomic,assign) AdWhirlConfigStore *configStore;
@property (nonatomic,retain) AWNetworkReachabilityWrapper *rollOverReachability;
@property (nonatomic,retain) NSArray *testDarts;

@end
