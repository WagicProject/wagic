/*

 AdWhirlConfigStoreTest.m

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
#import <OCMock/OCMock.h>
#import "GTMSenTestCase.h"
#import "GTMUnitTestDevLog.h"
#import "AdWhirlConfigStore.h"
#import "AdWhirlConfig.h"
#import "AWNetworkReachabilityWrapper.h"
#import "AdWhirlView.h"
#import "AdWhirlClassWrapper.h"
#import "AdWhirlAdNetworkRegistry.h"
#import "AdWhirlAdNetworkAdapter.h"
#import "AdWhirlError.h"


@interface AdWhirlConfigStoreTest : GTMTestCase {
}
@end


// Need to be a concrete class instead of a protocol mock because
// we don't want tests to invoke performSelectorOnMainThread
@interface AdWhirlConfigDelegateConcrete : NSObject<AdWhirlConfigDelegate> {
}
@end


@implementation AdWhirlConfigDelegateConcrete
@end


@implementation AdWhirlConfigStoreTest

-(void)setUp {
  [AdWhirlConfigStore resetStore];
}

- (void)tearDown {
  // Reset to make sure no leaks
  [AdWhirlConfigStore resetStore];
}

- (void)testSingleton {
  STAssertEquals([AdWhirlConfigStore sharedStore],
                 [AdWhirlConfigStore sharedStore],
                 @"There should only be one shared AdWhirlConfigStore");
}

- (void)testFetchConfig {
  AdWhirlConfigStore *store = [AdWhirlConfigStore sharedStore];

  id mockReachability =
    [OCMockObject mockForClass:[AWNetworkReachabilityWrapper class]];
  BOOL yesVal = YES;
  [[[mockReachability expect] andReturnValue:OCMOCK_VALUE(yesVal)]
   scheduleInCurrentRunLoop];
  store.reachability = mockReachability;

  id mockURLConn = [OCMockObject mockForClass:[NSURLConnection class]];
  store.connection = mockURLConn;

  // First get call should trigger fetchConfig
  NSString *configURLString = @"http://test.adwhirl.com/getInfo.php";
  NSString *appKey = @"1234567890abcdef";
  id mockConfigDelegate1 =
    [OCMockObject mockForProtocol:@protocol(AdWhirlConfigDelegate)];
  [[[mockConfigDelegate1 expect]
    andReturn:[NSURL URLWithString:configURLString]] adWhirlConfigURL];
  AdWhirlConfig *config1 = [store getConfig:appKey
                                   delegate:mockConfigDelegate1];
  STAssertEqualStrings(config1.appKey, appKey,
                       @"returned config should have same appKey");
  STAssertEqualStrings([config1.configURL host], @"test.adwhirl.com",
                       @"returned config should have same configURL host");
  STAssertEqualStrings([config1.configURL path], @"/getInfo.php",
                       @"returned config should have same configURL path");
  NSString *expectedQuery =
    [NSString stringWithFormat:@"appid=%@&appver=%d&client=1",
     appKey, kAdWhirlAppVer];
  STAssertEqualStrings([config1.configURL query], expectedQuery,
                       @"returned config should have the right query");
  STAssertFalse(config1.hasConfig, @"returned config should not have config");

  // Second get call for the same appKey while fetch in progress should get the
  // delegate added to the list
  id mockConfigDelegate2 =
    [OCMockObject mockForProtocol:@protocol(AdWhirlConfigDelegate)];
  AdWhirlConfig *config2 = [store getConfig:appKey
                                   delegate:mockConfigDelegate2];
  STAssertEquals(config1, config2, @"same config");

  // Get call for a different appKey while fetch in progress should be rejected
  NSString *configURLString3 = @"http://test3.adwhirl.com/getInfo.php";
  id mockConfigDelegate3 =
    [OCMockObject mockForProtocol:@protocol(AdWhirlConfigDelegate)];
  [[[mockConfigDelegate3 expect]
    andReturn:[NSURL URLWithString:configURLString3]] adWhirlConfigURL];
  [GTMUnitTestDevLog expectString:
   @"Another fetch is in progress, wait until finished."];
  id nilConfig = [store getConfig:@"9876543210abcdef"
                         delegate:mockConfigDelegate3];
  STAssertNil(nilConfig, @"should get nil config for call to different appKey"
              @" while fetch in progress");

  // Simulate bad callback
  id badReach =
    [OCMockObject mockForClass:[AWNetworkReachabilityWrapper class]];
  [GTMUnitTestDevLog expectPattern:@"Unrecognized reachability object"
                                  @" called not reachable .*"];
  [store reachabilityNotReachable:badReach];
  [GTMUnitTestDevLog expectPattern:@"Unrecognized reachability object"
                                  @" called reachable .*"];
  [store reachabilityBecameReachable:badReach];

  // Simulate reachability not yet ready. There should be a checkReachability
  // method call scheduled in the current run loop to be executed after 10
  // seconds. But the run loop is not running here, so we have to
  // simulate.
  [[[mockReachability expect] andReturn:@"example.com"] hostname];
  [store reachabilityNotReachable:mockReachability];
  STAssertNil(store.reachability,
              @"reachability should be nil after not reachable");

  // Put the reachability object back
  store.reachability = mockReachability;
  [[[mockReachability expect] andReturnValue:OCMOCK_VALUE(yesVal)]
   scheduleInCurrentRunLoop];

  // Simulate run loop call of checkReachability
  [store performSelector:@selector(checkReachability)];

  // Simulate reachability ready callback in run loop
  [store reachabilityBecameReachable:mockReachability];
  STAssertNil(store.reachability,
              @"reachability should be nil after reachable");

  // Simulate bad NSURLConnection callback
  id badConn = [OCMockObject mockForClass:[NSURLConnection class]];
  [GTMUnitTestDevLog expectPattern:@"Unrecognized connection object .*"];
  [store connection:badConn didReceiveResponse:nil];
  [GTMUnitTestDevLog expectPattern:@"Unrecognized connection object .*"];
  [store connection:badConn didFailWithError:nil];
  [GTMUnitTestDevLog expectPattern:@"Unrecognized connection object .*"];
  [store connectionDidFinishLoading:badConn];
  [GTMUnitTestDevLog expectPattern:@"Unrecognized connection object .*"];
  [store connection:badConn didReceiveData:nil];

  // Simulate NSURLConnection callbacks
  id mockResp = [OCMockObject mockForClass:[NSHTTPURLResponse class]];
  [[[mockResp expect] andReturnValue:OCMOCK_VALUE(yesVal)]
   isKindOfClass:[NSHTTPURLResponse class]];
  int httpStatus = 200;
  [[[mockResp expect] andReturnValue:OCMOCK_VALUE(httpStatus)] statusCode];
  [store connection:mockURLConn didReceiveResponse:mockResp];

  // Config processing
  id mockRegistry = [OCMockObject mockForClass:[AdWhirlAdNetworkRegistry class]];
  AdWhirlClassWrapper *classWrapper
    = [[AdWhirlClassWrapper alloc] initWithClass:[AdWhirlAdNetworkAdapter class]];
  [[[mockRegistry expect] andReturn:classWrapper] adapterClassFor:1]; // AdMob
  config1.adNetworkRegistry = mockRegistry;
  [[mockConfigDelegate1 expect] adWhirlConfigDidReceiveConfig:config1];
  [[mockConfigDelegate2 expect] adWhirlConfigDidReceiveConfig:config1];
  NSString *configRaw =
  @"{\"extra\":{"
    @"\"location_on\":0,"
    @"\"background_color_rgb\":{\"red\":7,\"green\":8,\"blue\":9,\"alpha\":0.5},"
    @"\"text_color_rgb\":{\"red\":200,\"green\":150,\"blue\":100,\"alpha\":0.5},"
    @"\"cycle_time\":45,"
    @"\"transition\":4},"
  @"\"rations\":[{"
    @"\"nid\":\"9976543210abcdefabcdef0000000001\","
    @"\"type\":1,"
    @"\"nname\":\"admob\","
    @"\"weight\":0,"
    @"\"priority\":1,"
    @"\"key\":\"ADMOB_KEY\""
  @"}]}";
  NSData *configData = [configRaw dataUsingEncoding:NSUTF8StringEncoding];
  [store connection:mockURLConn didReceiveData:configData];
  [store connectionDidFinishLoading:mockURLConn];
  STAssertNil(store.connection, @"connection nil after config loading");
  [classWrapper release], classWrapper = nil;

  // Test getting cached version
  id mockConfigDelegate4 =
    [OCMockObject mockForClass:[AdWhirlConfigDelegateConcrete class]];
  AdWhirlConfig *config4 = [store getConfig:appKey
                                   delegate:mockConfigDelegate4];
  STAssertEquals(config4, config1, @"same cached config");

  // Verify
  STAssertNoThrow([mockReachability verify], @"Must call expected methods");
  STAssertNoThrow([mockConfigDelegate1 verify], @"Must call expected methods");
  STAssertNoThrow([mockConfigDelegate2 verify], @"Must call expected methods");

  // During tearDown reachability's delegate will be set to nil
  [[mockReachability expect] setDelegate:nil];
}

- (BOOL)checkReachabilityError:(id)arg1 {
  if (![arg1 isKindOfClass:[AdWhirlError class]]) return NO;
  AdWhirlError *err = arg1;
  if ([err code] != AdWhirlConfigConnectionError) return NO;
  NSString *errMsg = [err localizedDescription];
  if (errMsg == nil) return NO;
  NSString *expectMsg = @"Error scheduling reachability";
  if ([errMsg rangeOfString:expectMsg].location != 0) return NO;
  return YES;
}

- (void)testFetchConfigReachabilityFail {
  AdWhirlConfigStore *store = [AdWhirlConfigStore sharedStore];

  id mockReachability =
    [OCMockObject mockForClass:[AWNetworkReachabilityWrapper class]];
  BOOL noVal = NO;
  [[[mockReachability expect] andReturnValue:OCMOCK_VALUE(noVal)]
   scheduleInCurrentRunLoop];
  store.reachability = mockReachability;

  // First get call should trigger fetchConfig
  NSString *configURLString = @"http://test.adwhirl.com/getInfo.php";
  NSString *appKey = @"abcdefabcdef";
  id mockConfigDelegate =
  [OCMockObject mockForProtocol:@protocol(AdWhirlConfigDelegate)];
  [[[mockConfigDelegate expect]
    andReturn:[NSURL URLWithString:configURLString]] adWhirlConfigURL];
  [[mockConfigDelegate expect] adWhirlConfigDidFail:[OCMArg any]
                                              error:
   [OCMArg checkWithSelector:@selector(checkReachabilityError:) onObject:self]];
  AdWhirlConfig *nilConfig = [store getConfig:appKey
                                     delegate:mockConfigDelegate];
  STAssertNil(nilConfig, @"reachability failure result in nil config");

  // Verify
  STAssertNoThrow([mockReachability verify], @"Must call expected methods");
  STAssertNoThrow([mockConfigDelegate verify], @"Must call expected methods");
}

- (BOOL)checkFailedConnectionError:(id)arg1 {
  STAssertTrue([arg1 isKindOfClass:[AdWhirlError class]],
               @"arg1 should be AdWhirlError");
  AdWhirlError *err = arg1;
  STAssertEquals([err code], AdWhirlConfigConnectionError,
                 @"Should be AdWhirlConfigConnectionError");
  NSString *errMsg = [err localizedDescription];
  STAssertNotNil(errMsg, @"Must have error message");
  NSString *expectMsg = @"Error connecting to config server";
  STAssertEqualStrings(errMsg, expectMsg, @"Error message content");
  return YES;
}

- (void)testFetchConfigFailedConnection {
  AdWhirlConfigStore *store = [AdWhirlConfigStore sharedStore];

  id mockReachability =
    [OCMockObject mockForClass:[AWNetworkReachabilityWrapper class]];
  BOOL yesVal = YES;
  [[[mockReachability expect] andReturnValue:OCMOCK_VALUE(yesVal)]
   scheduleInCurrentRunLoop];
  store.reachability = mockReachability;

  id mockURLConn = [OCMockObject mockForClass:[NSURLConnection class]];
  store.connection = mockURLConn;

  // First get call should trigger fetchConfig
  NSString *configURLString = @"http://test.adwhirl.com/getInfo.php";
  NSString *appKey = @"fedcbafedcbafedcba";
  id mockConfigDelegate =
    [OCMockObject mockForProtocol:@protocol(AdWhirlConfigDelegate)];
  [[[mockConfigDelegate expect]
    andReturn:[NSURL URLWithString:configURLString]] adWhirlConfigURL];
  AdWhirlConfig *config = [store getConfig:appKey
                                  delegate:mockConfigDelegate];
  STAssertFalse(config.hasConfig, @"returned config should not have config");

  // Simulate reachability ready callback in run loop
  [store reachabilityBecameReachable:mockReachability];
  STAssertNil(store.reachability,
              @"reachability should be nil after reachable");

  // Simulate NSURLConnection callbacks for failed connection
  [[mockConfigDelegate expect] adWhirlConfigDidFail:config
                                              error:
   [OCMArg checkWithSelector:@selector(checkFailedConnectionError:)
                    onObject:self]];
  [store connection:mockURLConn
                               didFailWithError:[NSError errorWithDomain:@"test"
                                                                    code:1
                                                                userInfo:nil]];

  // After the failure, the config should not longer be cached, so getConfig
  // should return a new config object
  [[[mockReachability expect] andReturnValue:OCMOCK_VALUE(yesVal)]
   scheduleInCurrentRunLoop];
  store.reachability = mockReachability;
  [[[mockConfigDelegate expect]
    andReturn:[NSURL URLWithString:configURLString]] adWhirlConfigURL];
  AdWhirlConfig *config2 = [store getConfig:appKey
                                  delegate:mockConfigDelegate];
  STAssertFalse(config2.hasConfig, @"returned config should not have config");
  STAssertNotEquals(config, config2, @"failed config should have been gone");

  // Set reachability to nil
  store.reachability = nil;

  // Verify
  STAssertNoThrow([mockReachability verify], @"Must call expected methods");
  STAssertNoThrow([mockConfigDelegate verify], @"Must call expected methods");
}

- (BOOL)checkBadHTTPStatusError:(id)arg1 {
  STAssertTrue([arg1 isKindOfClass:[AdWhirlError class]],
               @"arg1 should be AdWhirlError");
  AdWhirlError *err = arg1;
  STAssertEquals([err code], AdWhirlConfigStatusError,
                 @"Should be AdWhirlConfigStatusError");
  NSString *errMsg = [err localizedDescription];
  STAssertNotNil(errMsg, @"Must have error message");
  NSString *expectMsg = @"Config server did not return status 200";
  STAssertEqualStrings(errMsg, expectMsg, @"Error message content");
  return YES;
}

- (void)testFetchConfigBadHTTPStatus {
  AdWhirlConfigStore *store = [AdWhirlConfigStore sharedStore];

  id mockReachability =
    [OCMockObject mockForClass:[AWNetworkReachabilityWrapper class]];
  BOOL yesVal = YES;
  [[[mockReachability expect] andReturnValue:OCMOCK_VALUE(yesVal)]
   scheduleInCurrentRunLoop];
  store.reachability = mockReachability;

  id mockURLConn = [OCMockObject mockForClass:[NSURLConnection class]];
  store.connection = mockURLConn;

  // First get call should trigger fetchConfig
  NSString *configURLString = @"http://test.adwhirl.com/getInfo.php";
  NSString *appKey = @"fedcbafedcbafedcba";
  id mockConfigDelegate =
    [OCMockObject mockForProtocol:@protocol(AdWhirlConfigDelegate)];
  [[[mockConfigDelegate expect]
    andReturn:[NSURL URLWithString:configURLString]] adWhirlConfigURL];
  AdWhirlConfig *config = [store getConfig:appKey
                                   delegate:mockConfigDelegate];
  STAssertFalse(config.hasConfig, @"returned config should not have config");

  // Simulate reachability ready callback in run loop
  [store reachabilityBecameReachable:mockReachability];
  STAssertNil(store.reachability,
              @"reachability should be nil after reachable");

  // Simulate NSURLConnection callbacks for bad HTTP status
  id mockResp = [OCMockObject mockForClass:[NSHTTPURLResponse class]];
  [[[mockResp expect] andReturnValue:OCMOCK_VALUE(yesVal)]
   isKindOfClass:[NSHTTPURLResponse class]];
  [[[mockResp expect] andReturn:@"http://xyz"] URL];
  int httpStatus = 500;
  [[[mockResp expect] andReturnValue:OCMOCK_VALUE(httpStatus)] statusCode];
  [GTMUnitTestDevLog expectPattern:
   @"AdWhirlConfig: HTTP 500, cancelling http://xyz"];
  [[mockURLConn expect] cancel];
  [[mockConfigDelegate expect] adWhirlConfigDidFail:config
                                              error:
   [OCMArg checkWithSelector:@selector(checkBadHTTPStatusError:)
                    onObject:self]];
  [store connection:mockURLConn didReceiveResponse:mockResp];

  // After the failure, the config should not longer be cached, so getConfig
  // should return a new config object
  [[[mockReachability expect] andReturnValue:OCMOCK_VALUE(yesVal)]
   scheduleInCurrentRunLoop];
  store.reachability = mockReachability;
  [[[mockConfigDelegate expect]
    andReturn:[NSURL URLWithString:configURLString]] adWhirlConfigURL];
  AdWhirlConfig *config2 = [store getConfig:appKey
                                   delegate:mockConfigDelegate];
  STAssertFalse(config2.hasConfig, @"returned config should not have config");
  STAssertNotEquals(config, config2, @"failed config should have been gone");

  // Verify
  STAssertNoThrow([mockReachability verify], @"Must call expected methods");
  STAssertNoThrow([mockConfigDelegate verify], @"Must call expected methods");

  // During tearDown reachability's delegate will be set to nil
  [[mockReachability expect] setDelegate:nil];
}

@end
