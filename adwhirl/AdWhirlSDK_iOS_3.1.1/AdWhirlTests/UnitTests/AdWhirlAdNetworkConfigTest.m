/*

 AdWhirlAdNetworkConfigTest.m

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

#import <OCMock/OCMock.h>
#import "GTMSenTestCase.h"
#import "GTMUnitTestDevLog.h"
#import "AdWhirlAdNetworkConfig.h"
#import "AdWhirlAdNetworkAdapter.h"
#import "AdWhirlError.h"
#import "AdWhirlAdNetworkRegistry.h"
#import "AdWhirlClassWrapper.h"


@interface AdWhirlAdNetworkConfigTest : GTMTestCase {
  id mockRegistry_;
}
@end


@implementation AdWhirlAdNetworkConfigTest

-(void) setUp {
  mockRegistry_ = [OCMockObject mockForClass:[AdWhirlAdNetworkRegistry class]];
}

- (void) tearDown {
}

- (void) testGoodConfig {
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:
                               @"custom", AWAdNetworkConfigKeyName,
                               @"14.5", AWAdNetworkConfigKeyWeight,
                               @"2798463808b1234567890abcdef5c1e9", AWAdNetworkConfigKeyNID,
                               @"__CUSTOM__", AWAdNetworkConfigKeyCred,
                               @"9", AWAdNetworkConfigKeyType,
                               @"10", AWAdNetworkConfigKeyPriority,
                               nil];
  AdWhirlClassWrapper *classWrapper
    = [[AdWhirlClassWrapper alloc] initWithClass:[AdWhirlAdNetworkAdapter class]];
  [[[mockRegistry_ expect] andReturn:classWrapper] adapterClassFor:9];
  AdWhirlError *error = nil;
  AdWhirlAdNetworkConfig *config
    = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                       adNetworkRegistry:mockRegistry_
                                                   error:&error];
  STAssertNoThrow([mockRegistry_ verify],
                  @"Must have called adapterClassFor of the ad network registry");
  STAssertNil(error, @"should have no error parsing ad network config");
  STAssertNotNil(config, @"config should be non-nil");
  STAssertEqualStrings(config.networkName, @"custom", @"network name");
  STAssertEquals(config.trafficPercentage, 14.5, @"percentage");
  STAssertEqualStrings(config.nid, @"2798463808b1234567890abcdef5c1e9", @"nid");
  STAssertNotNil(config.credentials, @"credentials exists");
  STAssertEqualStrings(config.pubId, @"__CUSTOM__", @"pubId");
  STAssertEquals(config.networkType, 9, @"network type");
  STAssertEquals(config.priority, 10, @"priority");
  STAssertNotNil([config description], @"has description");
  STAssertEquals(config.adapterClass, classWrapper.theClass, @"adapter class match");
  [config release];
  [classWrapper release];
}

- (void) testGoodConfigHashCred {
  NSDictionary *cred = [NSDictionary dictionaryWithObjectsAndKeys:
                        @"site_id", @"siteID",
                        @"spot_id", @"spotID",
                        @"pub_id", @"publisherID",
                        nil];
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:
                              @"jumptap", AWAdNetworkConfigKeyName,
                              @"30", AWAdNetworkConfigKeyWeight,
                              @"1234567890a1234567890abcdef5c1e9", AWAdNetworkConfigKeyNID,
                              cred, AWAdNetworkConfigKeyCred,
                              @"2", AWAdNetworkConfigKeyType,
                              @"2", AWAdNetworkConfigKeyPriority,
                              nil];
  AdWhirlClassWrapper *classWrapper
    = [[AdWhirlClassWrapper alloc] initWithClass:[AdWhirlAdNetworkAdapter class]];
  [[[mockRegistry_ expect] andReturn:classWrapper] adapterClassFor:2];
  AdWhirlError *error = nil;
  AdWhirlAdNetworkConfig *config
    = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                       adNetworkRegistry:mockRegistry_
                                                   error:&error];
  STAssertNoThrow([mockRegistry_ verify],
                  @"Must have called adapterClassFor of the ad network registry");
  STAssertNil(error, @"should have no error parsing ad network config");
  STAssertNotNil(config, @"config should be non-nil");
  STAssertEqualStrings(config.networkName, @"jumptap", @"network name");
  STAssertEquals(config.trafficPercentage, 30.0, @"percentage");
  STAssertEqualStrings(config.nid, @"1234567890a1234567890abcdef5c1e9", @"nid");
  STAssertNotNil(config.credentials, @"credentials exists");
  STAssertTrue([config.credentials isKindOfClass:[NSDictionary class]],
               @"credentials is a dictionary");
  STAssertNil(config.pubId, @"no single pubId");
  STAssertEqualStrings([config.credentials objectForKey:@"siteID"],
                       @"site_id", @"cred.siteId");
  STAssertEqualStrings([config.credentials objectForKey:@"spotID"],
                       @"spot_id", @"cred.spotId");
  STAssertEqualStrings([config.credentials objectForKey:@"publisherID"],
                       @"pub_id", @"cred.pubId");
  STAssertEquals(config.networkType, 2, @"network type");
  STAssertEquals(config.priority, 2, @"priority");
  STAssertNotNil([config description], @"has description");
  STAssertEquals(config.adapterClass, classWrapper.theClass, @"adapter class match");
  [config release];
  [classWrapper release];
}

- (void) testEmptyConfig {
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:nil];
  AdWhirlError *error = nil;
  AdWhirlAdNetworkConfig *config
    = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                       adNetworkRegistry:mockRegistry_
                                                   error:&error];
  STAssertNil(config, @"Bad config dict should yield nil network config");
  STAssertNotNil(error, @"Bad config dict should yield error");
  STAssertEquals([error localizedDescription],
                 @"Ad network config has no network type, network id, network name, or priority",
                 @"Bad config dict error message");
  STAssertEquals([error code], AdWhirlConfigDataError,
                 @"Bad config should give AdWhirlConfigDataError");
}

- (void) testEmptyConfigNilErrorObj {
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:nil];
  [GTMUnitTestDevLog expectString:@"Ad network config has no network type, network id, network name, or priority"];
  AdWhirlAdNetworkConfig *config
    = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                       adNetworkRegistry:mockRegistry_
                                                   error:nil];
  STAssertNil(config, @"Bad config dict should yield nil network config");
}

- (void) testNonExistentNetworkType {
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:
                              @"bogus", AWAdNetworkConfigKeyName,
                              @"50", AWAdNetworkConfigKeyWeight,
                              @"2798463808b1234567890abcdef5c1e9", AWAdNetworkConfigKeyNID,
                              @"x", AWAdNetworkConfigKeyCred,
                              @"1000000", AWAdNetworkConfigKeyType,
                              @"6", AWAdNetworkConfigKeyPriority,
                              nil];
  [[[mockRegistry_ expect] andReturn:nil] adapterClassFor:1000000];
  AdWhirlError *error = nil;
  AdWhirlAdNetworkConfig *config
    = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                       adNetworkRegistry:mockRegistry_
                                                   error:&error];
  STAssertNoThrow([mockRegistry_ verify],
                  @"Must have called adapterClassFor of the ad network registry");
  STAssertNil(config, @"Config must be null for non-existent network type");
  STAssertNotNil(error, @"Must returned error for non-existent network type");
  STAssertEqualStrings([error localizedDescription],
                       @"Ad network type 1000000 not supported, no adapter found",
                       @"Non-existent network type error string");
  STAssertEquals([error code], AdWhirlConfigDataError,
                 @"Non-existent network type should give AdWhirlConfigDataError");
}

- (void) testNonExistentNetworkTypeNilErrorObj {
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:
                              @"bogus", AWAdNetworkConfigKeyName,
                              @"50", AWAdNetworkConfigKeyWeight,
                              @"2798463808b1234567890abcdef5c1e9", AWAdNetworkConfigKeyNID,
                              @"x", AWAdNetworkConfigKeyCred,
                              @"1000000", AWAdNetworkConfigKeyType,
                              @"6", AWAdNetworkConfigKeyPriority,
                              nil];
  [[[mockRegistry_ expect] andReturn:nil] adapterClassFor:1000000];
  [GTMUnitTestDevLog expectString:@"Ad network type 1000000 not supported, no adapter found"];
  AdWhirlAdNetworkConfig *config
    = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                       adNetworkRegistry:mockRegistry_
                                                   error:nil];
  STAssertNoThrow([mockRegistry_ verify],
                  @"Must have called adapterClassFor of the ad network registry");
  STAssertNil(config, @"Config must be null for non-existent network type");
}

- (void) testNilWeight {
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:
                              @"custom", AWAdNetworkConfigKeyName,
                              @"2798463808b1234567890abcdef5c1e9", AWAdNetworkConfigKeyNID,
                              @"__CUSTOM__", AWAdNetworkConfigKeyCred,
                              @"9", AWAdNetworkConfigKeyType,
                              @"10", AWAdNetworkConfigKeyPriority,
                              nil];
  AdWhirlClassWrapper *classWrapper
  = [[AdWhirlClassWrapper alloc] initWithClass:[AdWhirlAdNetworkAdapter class]];
  [[[mockRegistry_ expect] andReturn:classWrapper] adapterClassFor:9];
  AdWhirlError *error = nil;
  AdWhirlAdNetworkConfig *config
  = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                     adNetworkRegistry:mockRegistry_
                                                 error:&error];
  STAssertNoThrow([mockRegistry_ verify],
                  @"Must have called adapterClassFor of the ad network registry");
  STAssertNil(error, @"should have no error parsing ad network config");
  STAssertNotNil(config, @"config should be non-nil");
  STAssertEqualStrings(config.networkName, @"custom", @"network name");
  STAssertEquals(config.trafficPercentage, 0.0, @"percentage should be 0");
  STAssertEqualStrings(config.nid, @"2798463808b1234567890abcdef5c1e9", @"nid");
  STAssertNotNil(config.credentials, @"credentials exists");
  STAssertEqualStrings(config.pubId, @"__CUSTOM__", @"pubId");
  STAssertEquals(config.networkType, 9, @"network type");
  STAssertEquals(config.priority, 10, @"priority");
  STAssertNotNil([config description], @"has description");
  STAssertEquals(config.adapterClass, classWrapper.theClass, @"adapter class match");
  [config release];
  [classWrapper release];
}

- (void) testNilCred {
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:
                              @"custom", AWAdNetworkConfigKeyName,
                              @"14.5", AWAdNetworkConfigKeyWeight,
                              @"2798463808b1234567890abcdef5c1e9", AWAdNetworkConfigKeyNID,
                              @"9", AWAdNetworkConfigKeyType,
                              @"10", AWAdNetworkConfigKeyPriority,
                              nil];
  AdWhirlClassWrapper *classWrapper
  = [[AdWhirlClassWrapper alloc] initWithClass:[AdWhirlAdNetworkAdapter class]];
  [[[mockRegistry_ expect] andReturn:classWrapper] adapterClassFor:9];
  AdWhirlError *error = nil;
  AdWhirlAdNetworkConfig *config
  = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                     adNetworkRegistry:mockRegistry_
                                                 error:&error];
  STAssertNoThrow([mockRegistry_ verify],
                  @"Must have called adapterClassFor of the ad network registry");
  STAssertNil(error, @"should have no error parsing ad network config");
  STAssertNotNil(config, @"config should be non-nil");
  STAssertEqualStrings(config.networkName, @"custom", @"network name");
  STAssertEquals(config.trafficPercentage, 14.5, @"percentage");
  STAssertEqualStrings(config.nid, @"2798463808b1234567890abcdef5c1e9", @"nid");
  STAssertNil(config.credentials, @"credentials exists");
  STAssertNil(config.pubId, @"credentials nil");
  STAssertEquals(config.networkType, 9, @"network type");
  STAssertEquals(config.priority, 10, @"priority");
  STAssertNotNil([config description], @"has description");
  STAssertEquals(config.adapterClass, classWrapper.theClass, @"adapter class match");
  [config release];
  [classWrapper release];
}

- (void) testNetworkTypeNotNumber {
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:
                              @"custom", AWAdNetworkConfigKeyName,
                              @"14.5", AWAdNetworkConfigKeyWeight,
                              @"2798463808b1234567890abcdef5c1e9", AWAdNetworkConfigKeyNID,
                              @"__CUSTOM__", AWAdNetworkConfigKeyCred,
                              @"somestring", AWAdNetworkConfigKeyType,
                              @"10", AWAdNetworkConfigKeyPriority,
                              nil];
  AdWhirlError *error = nil;
  AdWhirlAdNetworkConfig *config
  = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                     adNetworkRegistry:mockRegistry_
                                                 error:&error];
  STAssertNil(config, @"Non-int network type should yield nil network config");
  STAssertNotNil(error, @"Non-int network type should yield error");
  STAssertEquals([error localizedDescription],
                 @"Ad network config has invalid network type, network id, network name or priority",
                 @"Non-int network type error message");
  STAssertEquals([error code], AdWhirlConfigDataError,
                 @"Non-int network type should give AdWhirlConfigDataError");
}

- (void) testNetworkTypeNotNumberNilErrObj {
  NSDictionary *configDict = [NSDictionary dictionaryWithObjectsAndKeys:
                              @"custom", AWAdNetworkConfigKeyName,
                              @"14.5", AWAdNetworkConfigKeyWeight,
                              @"2798463808b1234567890abcdef5c1e9", AWAdNetworkConfigKeyNID,
                              @"__CUSTOM__", AWAdNetworkConfigKeyCred,
                              @"somestring", AWAdNetworkConfigKeyType,
                              @"10", AWAdNetworkConfigKeyPriority,
                              nil];
  [GTMUnitTestDevLog expectString:@"Ad network config has invalid network type, network id, network name or priority"];
  AdWhirlAdNetworkConfig *config
  = [[AdWhirlAdNetworkConfig alloc] initWithDictionary:configDict
                                     adNetworkRegistry:mockRegistry_
                                                 error:nil];
  STAssertNil(config, @"Non-int network type should yield nil network config");
}

@end
