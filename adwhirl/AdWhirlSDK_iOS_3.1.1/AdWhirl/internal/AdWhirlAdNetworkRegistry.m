/*

 AdWhirlAdNetworkRegistry.m

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

#import "AdWhirlAdNetworkRegistry.h"
#import "AdWhirlAdNetworkAdapter.h"
#import "AdWhirlClassWrapper.h"

@implementation AdWhirlAdNetworkRegistry

+ (AdWhirlAdNetworkRegistry *)sharedRegistry {
  static AdWhirlAdNetworkRegistry *registry = nil;
  if (registry == nil) {
    registry = [[AdWhirlAdNetworkRegistry alloc] init];
  }
  return registry;
}

- (id)init {
  self = [super init];
  if (self != nil) {
    adapterDict = [[NSMutableDictionary alloc] init];
  }
  return self;
}

- (void)registerClass:(Class)adapterClass {
  // have to do all these to avoid compiler warnings...
  NSInteger (*netTypeMethod)(id, SEL);
  netTypeMethod = (NSInteger (*)(id, SEL))[adapterClass methodForSelector:@selector(networkType)];
  NSInteger netType = netTypeMethod(adapterClass, @selector(networkType));
  NSNumber *key = [[NSNumber alloc] initWithInteger:netType];
  AdWhirlClassWrapper *wrapper = [[AdWhirlClassWrapper alloc] initWithClass:adapterClass];
  [adapterDict setObject:wrapper forKey:key];
  [key release];
  [wrapper release];
}

- (AdWhirlClassWrapper *)adapterClassFor:(NSInteger)adNetworkType {
  return [adapterDict objectForKey:[NSNumber numberWithInteger:adNetworkType]];
}

- (void)dealloc {
  [adapterDict release], adapterDict = nil;
  [super dealloc];
}

@end
