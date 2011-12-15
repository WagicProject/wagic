/*

 AdWhirlConfigStore.m

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

#import "AdWhirlConfigStore.h"
#import "AdWhirlLog.h"
#import "AWNetworkReachabilityWrapper.h"
#import "AdWhirlError.h"

static AdWhirlConfigStore *gStore = nil;

@interface AdWhirlConfigStore ()

- (BOOL)checkReachability;
- (void)startFetchingAssumingReachable;
- (void)failedFetchingWithError:(AdWhirlError *)error;
- (void)finishedFetching;

@end


@implementation AdWhirlConfigStore

@synthesize reachability = reachability_;
@synthesize connection = connection_;

+ (AdWhirlConfigStore *)sharedStore {
  if (gStore == nil) {
    gStore = [[AdWhirlConfigStore alloc] init];
  }
  return gStore;
}

+ (void)resetStore {
  if (gStore != nil) {
    [gStore release], gStore = nil;
    [self sharedStore];
  }
}

- (id)init {
  self = [super init];
  if (self != nil) {
    configs_ = [[NSMutableDictionary alloc] init];
  }
  return self;
}

- (AdWhirlConfig *)getConfig:(NSString *)appKey
                    delegate:(id<AdWhirlConfigDelegate>)delegate {
  AdWhirlConfig *config = [configs_ objectForKey:appKey];
  if (config != nil) {
    if (config.hasConfig) {
      if ([delegate
           respondsToSelector:@selector(adWhirlConfigDidReceiveConfig:)]) {
        // Don't call directly, instead schedule it in the runloop. Delegate
        // may expect the message to be delivered out-of-band
        [(NSObject *)delegate
         performSelectorOnMainThread:@selector(adWhirlConfigDidReceiveConfig:)
                          withObject:config
                       waitUntilDone:NO];
      }
      return config;
    }
    // If there's already a config fetching, and another call to this function
    // add a delegate to the config
    [config addDelegate:delegate];
    return config;
  }

  // No config, create one, and start fetching it
  return [self fetchConfig:appKey delegate:delegate];
}

- (AdWhirlConfig *)fetchConfig:(NSString *)appKey
                      delegate:(id <AdWhirlConfigDelegate>)delegate {

  AdWhirlConfig *config = [[AdWhirlConfig alloc] initWithAppKey:appKey
                                                       delegate:delegate];

  if (fetchingConfig_ != nil) {
    AWLogWarn(@"Another fetch is in progress, wait until finished.");
    [config release];
    return nil;
  }
  fetchingConfig_ = config;

  if (![self checkReachability]) {
    [config release];
    return nil;
  }

  [configs_ setObject:config forKey:appKey];
  [config release];
  return config;
}

- (void)dealloc {
  if (reachability_ != nil) {
    reachability_.delegate = nil;
    [reachability_ release];
  }
  [connection_ release];
  [receivedData_ release];
  [configs_ release];
  [super dealloc];
}


#pragma mark private helper methods

// Check reachability first
- (BOOL)checkReachability {
  AWLogDebug(@"Checking if config is reachable at %@",
             fetchingConfig_.configURL);

  // Normally reachability_ should be nil so a new one will be created.
  // In a testing environment, it may already have been assigned with a mock.
  // In any case, reachability_ will be released when the config URL is
  // reachable, in -reachabilityBecameReachable.
  if (reachability_ == nil) {
    reachability_ = [AWNetworkReachabilityWrapper
                     reachabilityWithHostname:[fetchingConfig_.configURL host]
                     callbackDelegate:self];
    [reachability_ retain];
  }
  if (reachability_ == nil) {
    [fetchingConfig_ notifyDelegatesOfFailure:
     [AdWhirlError errorWithCode:AdWhirlConfigConnectionError
                     description:
      @"Error setting up reachability check to config server"]];
    return NO;
  }

  if (![reachability_ scheduleInCurrentRunLoop]) {
    [fetchingConfig_ notifyDelegatesOfFailure:
     [AdWhirlError errorWithCode:AdWhirlConfigConnectionError
                     description:
      @"Error scheduling reachability check to config server"]];
    [reachability_ release], reachability_ = nil;
    return NO;
  }

  return YES;
}

// Make connection
- (void)startFetchingAssumingReachable {
  // go fetch config
  NSURLRequest *configRequest
    = [NSURLRequest requestWithURL:fetchingConfig_.configURL];

  // Normally connection_ should be nil so a new one will be created.
  // In a testing environment, it may alreay have been assigned with a mock.
  // In any case, connection_ will be release when connection failed or
  // finished.
  if (connection_ == nil) {
    connection_ = [[NSURLConnection alloc] initWithRequest:configRequest
                                                  delegate:self];
  }

  // Error checking
  if (connection_ == nil) {
    [self failedFetchingWithError:
     [AdWhirlError errorWithCode:AdWhirlConfigConnectionError
                     description:
                                @"Error creating connection to config server"]];
    return;
  }
  receivedData_ = [[NSMutableData alloc] init];
}

// Clean up after fetching failed
- (void)failedFetchingWithError:(AdWhirlError *)error {
  // notify
  [fetchingConfig_ notifyDelegatesOfFailure:error];

  // remove the failed config from the cache
  [configs_ removeObjectForKey:fetchingConfig_.appKey];
  // the config is only retained by the dict,now released

  [self finishedFetching];
}

// Clean up after fetching, success or failed
- (void)finishedFetching {
  [connection_ release], connection_ = nil;
  [receivedData_ release], receivedData_ = nil;
  fetchingConfig_ = nil;
}


#pragma mark reachability methods

- (void)reachabilityNotReachable:(AWNetworkReachabilityWrapper *)reach {
  if (reach != reachability_) {
    AWLogWarn(@"Unrecognized reachability object called not reachable %s:%d",
              __FILE__, __LINE__);
    return;
  }
  AWLogDebug(@"Config host %@ not (yet) reachable, check back later",
             reach.hostname);
  [reachability_ release], reachability_ = nil;
  [self performSelector:@selector(checkReachability)
             withObject:nil
             afterDelay:10.0];
}

- (void)reachabilityBecameReachable:(AWNetworkReachabilityWrapper *)reach {
  if (reach != reachability_) {
    AWLogWarn(@"Unrecognized reachability object called reachable %s:%d",
              __FILE__, __LINE__);
    return;
  }
  // done with the reachability
  [reachability_ release], reachability_ = nil;

  [self startFetchingAssumingReachable];
}


#pragma mark NSURLConnection delegate methods.

- (void)connection:(NSURLConnection *)conn
                                didReceiveResponse:(NSURLResponse *)response {
  if (conn != connection_) {
    AWLogError(@"Unrecognized connection object %s:%d", __FILE__, __LINE__);
    return;
  }
  if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
    NSHTTPURLResponse *http = (NSHTTPURLResponse*)response;
    const int status = [http statusCode];

    if (status < 200 || status >= 300) {
      AWLogWarn(@"AdWhirlConfig: HTTP %d, cancelling %@", status, [http URL]);
      [connection_ cancel];
      [self failedFetchingWithError:
       [AdWhirlError errorWithCode:AdWhirlConfigStatusError
                       description:@"Config server did not return status 200"]];
      return;
    }
  }

  [receivedData_ setLength:0];
}

- (void)connection:(NSURLConnection *)conn didFailWithError:(NSError *)error {
  if (conn != connection_) {
    AWLogError(@"Unrecognized connection object %s:%d", __FILE__, __LINE__);
    return;
  }
  [self failedFetchingWithError:
   [AdWhirlError errorWithCode:AdWhirlConfigConnectionError
                   description:@"Error connecting to config server"
               underlyingError:error]];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)conn {
  if (conn != connection_) {
    AWLogError(@"Unrecognized connection object %s:%d", __FILE__, __LINE__);
    return;
  }
  [fetchingConfig_ parseConfig:receivedData_ error:nil];
  [self finishedFetching];
}

- (void)connection:(NSURLConnection *)conn didReceiveData:(NSData *)data {
  if (conn != connection_) {
    AWLogError(@"Unrecognized connection object %s:%d", __FILE__, __LINE__);
    return;
  }
  [receivedData_ appendData:data];
}

@end
