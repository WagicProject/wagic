/*

 AWNetworkReachabilityWrapper.m

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

#import "AWNetworkReachabilityWrapper.h"
#import "AdWhirlLog.h"

static void reachabilityCallback(SCNetworkReachabilityRef reachability,
                                 SCNetworkReachabilityFlags flags,
                                 void* data);

@implementation AWNetworkReachabilityWrapper

@synthesize hostname = hostname_;
@synthesize delegate = delegate_;

+ (AWNetworkReachabilityWrapper *) reachabilityWithHostname:(NSString *)host
                callbackDelegate:(id<AWNetworkReachabilityDelegate>)delegate {
  return [[[AWNetworkReachabilityWrapper alloc] initWithHostname:host
                                                callbackDelegate:delegate]
          autorelease];
}

- (id)initWithHostname:(NSString *)host
      callbackDelegate:(id<AWNetworkReachabilityDelegate>)delegate {
  self = [super init];
  if (self != nil) {
    reachability_ = SCNetworkReachabilityCreateWithName(NULL,
                                                        [host UTF8String]);
    if (reachability_ == nil) {
      AWLogError(@"Error creating SCNetworkReachability");
      [self release];
      return nil;
    }
    hostname_ = [[NSString alloc] initWithString:host];
    self.delegate = delegate;

    // set callback
    SCNetworkReachabilityContext context = {0, self, NULL, NULL, NULL};
    if (!SCNetworkReachabilitySetCallback(reachability_,
                                          &reachabilityCallback,
                                          &context)) {
      AWLogError(@"Error setting SCNetworkReachability callback");
      [self release];
      return nil;
    }
  }
  return self;
}

- (BOOL)scheduleInCurrentRunLoop {
  return SCNetworkReachabilityScheduleWithRunLoop(reachability_,
                                                  CFRunLoopGetCurrent(),
                                                  kCFRunLoopDefaultMode);
}

- (BOOL)unscheduleFromCurrentRunLoop {
  return SCNetworkReachabilityUnscheduleFromRunLoop(reachability_,
                                                    CFRunLoopGetCurrent(),
                                                    kCFRunLoopDefaultMode);
}

- (void)dealloc {
  [self unscheduleFromCurrentRunLoop];
  if (reachability_ != NULL) CFRelease(reachability_);
  [hostname_ release];
  [super dealloc];
}

#pragma mark callback methods

static void printReachabilityFlags(SCNetworkReachabilityFlags flags)
{
  AWLogDebug(@"Reachability flag status: %c%c%c%c%c%c%c%c%c",
         (flags & kSCNetworkReachabilityFlagsTransientConnection)  ? 't' : '-',
         (flags & kSCNetworkReachabilityFlagsReachable)            ? 'R' : '-',
         (flags & kSCNetworkReachabilityFlagsConnectionRequired)   ? 'c' : '-',
         (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic)  ? 'C' : '-',
         (flags & kSCNetworkReachabilityFlagsInterventionRequired) ? 'i' : '-',
#ifdef kSCNetworkReachabilityFlagsConnectionOnDemand
         (flags & kSCNetworkReachabilityFlagsConnectionOnDemand)   ? 'D' : '-',
#else
         '-',
#endif
         (flags & kSCNetworkReachabilityFlagsIsLocalAddress)       ? 'l' : '-',
         (flags & kSCNetworkReachabilityFlagsIsDirect)             ? 'd' : '-',
         (flags & kSCNetworkReachabilityFlagsIsWWAN)               ? 'W' : '-'
         );
}

- (void)notifyDelegateNotReachable {
  if (self.delegate != nil && [self.delegate respondsToSelector:
                                        @selector(reachabilityNotReachable:)]) {
    [self.delegate reachabilityNotReachable:self];
  }
}

- (void)gotCallback:(SCNetworkReachabilityRef)reachability
              flags:(SCNetworkReachabilityFlags)flags {
  if (reachability != reachability_) {
    AWLogError(@"Unrelated reachability calling back to this object");
    return;
  }

  printReachabilityFlags(flags);
  if ((flags & kSCNetworkReachabilityFlagsReachable) == 0) {
    [self notifyDelegateNotReachable];
    return;
  }

  // even if the Reachable flag is on it may not be true for immediate use
  BOOL reachable = NO;

  if ((flags & kSCNetworkReachabilityFlagsConnectionRequired) == 0) {
    // no connection required, we should be able to connect, via WiFi presumably
    reachable = YES;
  }

  if ((
#ifdef kSCNetworkReachabilityFlagsConnectionOnDemand
       (flags & kSCNetworkReachabilityFlagsConnectionOnDemand) != 0 ||
#endif
       (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) != 0)
      && (flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0) {
    // The connection is on-demand or on-traffic and no user intervention is
    // needed, likely able to connect
    reachable = YES;
  }

	if ((flags & kSCNetworkReachabilityFlagsIsWWAN)
                                        == kSCNetworkReachabilityFlagsIsWWAN) {
		// WWAN connections are available, likely able to connect barring network
    // outage...
    reachable = YES;
	}

  if (!reachable) {
    [self notifyDelegateNotReachable];
    return;
  }

  // notify delegate that host just got reachable
  if (self.delegate != nil && [self.delegate respondsToSelector:
                                    @selector(reachabilityBecameReachable:)]) {
    [self.delegate reachabilityBecameReachable:self];
  }
}

void reachabilityCallback(SCNetworkReachabilityRef reachability,
                          SCNetworkReachabilityFlags flags,
                          void* data) {
  AWNetworkReachabilityWrapper *wrapper = (AWNetworkReachabilityWrapper *)data;
  [wrapper gotCallback:reachability flags:flags];
}

@end
