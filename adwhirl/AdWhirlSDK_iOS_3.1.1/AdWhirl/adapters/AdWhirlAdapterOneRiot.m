/*
 
 AdWhirlAdapterOneRiot.m
 
 Copyright 2010 OneRiot, Inc.
 
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

#import "AdWhirlAdapterOneRiot.h"
#import "AdWhirlAdNetworkRegistry.h"
#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdNetworkConfig.h"

@interface AdWhirlAdapterOneRiot ()

@property (nonatomic,retain) OneRiotAd *adControl;

@end

@implementation AdWhirlAdapterOneRiot

@synthesize adControl;

+ (AdWhirlAdNetworkType)networkType {
  return AdWhirlAdNetworkTypeOneRiot;
}

+ (void)load {
  [[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
}

- (void)getAd {
  NSString *appId = networkConfig.pubId;
  
  if ([adWhirlDelegate respondsToSelector:@selector(oneRiotAppID)]) {
    appId = [adWhirlDelegate oneRiotAppID];
  }
  
  adControl = [[OneRiotAd alloc] initWithAppId:appId andWidth:300
                                 andHeight:50];

  adControl.RefreshInterval = adWhirlConfig.refreshInterval;
  adControl.ReportGPS = adWhirlConfig.locationOn;
  
  if ([adWhirlDelegate
         respondsToSelector:@selector(oneRiotContextParameters)]) {
    NSArray* contextParams = [adWhirlDelegate oneRiotContextParameters];

    for (NSString* param in contextParams){
      [adControl addContextParameters:param];
    }
  }
  [adControl loadAd];
  self.adNetworkView = adControl;
}

-(void) dealloc {
  [adControl release];
  adControl = nil;
  [super dealloc];
}

@end
