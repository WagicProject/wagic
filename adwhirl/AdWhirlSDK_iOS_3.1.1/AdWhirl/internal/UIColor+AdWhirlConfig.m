/*

 UIColor+AdWhirlConfig.m

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

#import "UIColor+AdWhirlConfig.h"
#import "AdWhirlConfig.h"

@implementation UIColor (AdWhirlConfig)

- (id)initWithDict:(NSDictionary *)dict {
  id red, green, blue, alpha;
  CGFloat r, g, b, a;

  red   = [dict objectForKey:@"red"];
  if (red == nil) {
    [self release];
    return nil;
  }
  green = [dict objectForKey:@"green"];
  if (green == nil) {
    [self release];
    return nil;
  }
  blue  = [dict objectForKey:@"blue"];
  if (blue == nil) {
    [self release];
    return nil;
  }

  NSInteger temp;
  if (!awIntVal(&temp, red)) {
    [self release];
    return nil;
  }
  r = (CGFloat)temp/255.0;
  if (!awIntVal(&temp, green)) {
    [self release];
    return nil;
  }
  g = (CGFloat)temp/255.0;
  if (!awIntVal(&temp, blue)) {
    [self release];
    return nil;
  }
  b = (CGFloat)temp/255.0;

  a = 1.0; // default 1.0
  alpha = [dict objectForKey:@"alpha"];
  CGFloat temp_f;
  if (alpha != nil && awFloatVal(&temp_f, alpha)) {
    a = (CGFloat)temp_f;
  }

  return [self initWithRed:r green:g blue:b alpha:a];
}

@end
