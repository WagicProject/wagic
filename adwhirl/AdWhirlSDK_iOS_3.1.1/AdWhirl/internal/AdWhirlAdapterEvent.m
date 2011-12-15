/*

 AdWhirlAdapterEvent.m

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

#import "AdWhirlAdapterEvent.h"
#import "AdWhirlView.h"
#import "AdWhirlLog.h"
#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdNetworkRegistry.h"
#import "AdWhirlAdNetworkConfig.h"

@implementation AdWhirlAdapterEvent

+ (AdWhirlAdNetworkType)networkType {
	return AdWhirlAdNetworkTypeEvent;
}

+ (void)load {
	[[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
}

- (void)getAd {
	NSArray *eventKeys = [networkConfig.pubId componentsSeparatedByString:@"|;|"];
	NSString *eventSelectorStr = [eventKeys objectAtIndex:1];
	SEL eventSelector = NSSelectorFromString(eventSelectorStr);

	if ([adWhirlDelegate respondsToSelector:eventSelector]) {
		[adWhirlDelegate performSelector:eventSelector];
		[adWhirlView adapterDidFinishAdRequest:self];
	}
	else {
    NSString *eventSelectorColonStr = [NSString stringWithFormat:@"%@:", eventSelectorStr];
    SEL eventSelectorColon = NSSelectorFromString(eventSelectorColonStr);
    if ([adWhirlDelegate respondsToSelector:eventSelectorColon]) {
      [adWhirlDelegate performSelector:eventSelectorColon withObject:adWhirlView];
      [adWhirlView adapterDidFinishAdRequest:self];
    }
    else {
      AWLogWarn(@"Delegate does not implement function %@ nor %@", eventSelectorStr, eventSelectorColonStr);
      [adWhirlView adapter:self didFailAd:nil];
    }
	}
}

- (void)stopBeingDelegate {
  // Nothing to do
}

- (void)dealloc {
	[super dealloc];
}

@end
