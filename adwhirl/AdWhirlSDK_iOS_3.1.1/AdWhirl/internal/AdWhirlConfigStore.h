/*

 AdWhirlConfigStore.h

 Copyright 2010 Google Inc.

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
#import "AdWhirlConfigStore.h"
#import "AdWhirlConfig.h"
#import "AWNetworkReachabilityDelegate.h"

@class AWNetworkReachabilityWrapper;

// Singleton class to store AdWhirl configs, keyed by appKey. Fetched config
// is cached unless it is force-fetched using fetchConfig. Checks network
// reachability using AWNetworkReachabilityWrapper before making connections to
// fetch configs, so that that means it will wait forever until the config host
// is reachable.
@interface AdWhirlConfigStore : NSObject <AWNetworkReachabilityDelegate> {
  NSMutableDictionary *configs_;
  AdWhirlConfig *fetchingConfig_;

  AWNetworkReachabilityWrapper *reachability_;
  NSURLConnection *connection_;
  NSMutableData *receivedData_;
}

// Returns the singleton AdWhirlConfigStore object.
+ (AdWhirlConfigStore *)sharedStore;

// Deletes all existing configs.
+ (void)resetStore;

// Returns config for appKey. If config does not exist for appKey, goes and
// fetches the config from the server, the URL of which is taken from
// [delegate adWhirlConfigURL].
// Returns nil if appKey is nil or empty, another fetch is in progress, or
// error setting up reachability check.
- (AdWhirlConfig *)getConfig:(NSString *)appKey
                    delegate:(id<AdWhirlConfigDelegate>)delegate;

// Fetches (or re-fetch) the config for the given appKey. Always go to the
// network. Call this to get a new version of the config from the server.
// Returns nil if appKey is nil or empty, another fetch is in progress, or
// error setting up reachability check.
- (AdWhirlConfig *)fetchConfig:(NSString *)appKey
                      delegate:(id <AdWhirlConfigDelegate>)delegate;

// For testing -- set mocks here.
@property (nonatomic,retain) AWNetworkReachabilityWrapper *reachability;
@property (nonatomic,retain) NSURLConnection *connection;

@end
