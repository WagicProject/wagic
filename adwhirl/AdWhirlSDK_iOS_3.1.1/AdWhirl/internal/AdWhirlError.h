/*

 AdWhirlError.h

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

#define AdWhirlErrorDomain @"com.adwhirl.sdk.ErrorDomain"

enum {
  AdWhirlConfigConnectionError = 10, /* Cannot connect to config server */
  AdWhirlConfigStatusError = 11, /* config server did not return 200 */
  AdWhirlConfigParseError = 20, /* Error parsing config from server */
  AdWhirlConfigDataError = 30,  /* Invalid config format from server */
  AdWhirlCustomAdConnectionError = 40, /* Cannot connect to custom ad server */
  AdWhirlCustomAdParseError = 50, /* Error parsing custom ad from server */
  AdWhirlCustomAdDataError = 60, /* Invalid custom ad data from server */
  AdWhirlCustomAdImageError = 70, /* Cannot create image from data */
  AdWhirlAdRequestIgnoredError = 80, /* ignoreNewAdRequests flag is set */
  AdWhirlAdRequestInProgressError = 90, /* ad request in progress */
  AdWhirlAdRequestNoConfigError = 100, /* no configurations for ad request */
  AdWhirlAdRequestTooSoonError = 110, /* requesting ad too soon */
  AdWhirlAdRequestNoMoreAdNetworks = 120, /* no more ad networks for rollover */
  AdWhirlAdRequestNoNetworkError = 130, /* no network connection */
  AdWhirlAdRequestModalActiveError = 140 /* modal view active */
};

@interface AdWhirlError : NSError {

}

+ (AdWhirlError *)errorWithCode:(NSInteger)code userInfo:(NSDictionary *)dict;
+ (AdWhirlError *)errorWithCode:(NSInteger)code description:(NSString *)desc;
+ (AdWhirlError *)errorWithCode:(NSInteger)code description:(NSString *)desc underlyingError:(NSError *)uError;

- (id)initWithCode:(NSInteger)code userInfo:(NSDictionary *)dict;
- (id)initWithCode:(NSInteger)code description:(NSString *)desc;
- (id)initWithCode:(NSInteger)code description:(NSString *)desc underlyingError:(NSError *)uError;

@end
