/*

 AWNetworkReachabilityWrapper.h

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
#import <SystemConfiguration/SystemConfiguration.h>
#import <sys/socket.h>
#import <netinet/in.h>
#import <netinet6/in6.h>
#import <arpa/inet.h>
#import <netdb.h>
#import "AWNetworkReachabilityDelegate.h"

@class AWNetworkReachabilityWrapper;


// Created for ease of mocking (hence testing)
@interface AWNetworkReachabilityWrapper : NSObject {
  NSString *hostname_;
  SCNetworkReachabilityRef reachability_;
  id<AWNetworkReachabilityDelegate> delegate_;
}

@property (nonatomic,readonly) NSString *hostname;
@property (nonatomic,assign) id<AWNetworkReachabilityDelegate> delegate;

+ (AWNetworkReachabilityWrapper *) reachabilityWithHostname:(NSString *)host
          callbackDelegate:(id<AWNetworkReachabilityDelegate>)delegate;

- (id)initWithHostname:(NSString *)host
      callbackDelegate:(id<AWNetworkReachabilityDelegate>)delegate;

- (BOOL)scheduleInCurrentRunLoop;

- (BOOL)unscheduleFromCurrentRunLoop;

@end
