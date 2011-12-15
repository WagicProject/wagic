/*

 UIColor+AdWhirlConfigTest.m

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
#import "GTMSenTestCase.h"


@interface UIColor_AdWhirlConfigTest : GTMTestCase {
}

- (void)compareDict:(NSDictionary *)dict
            withRed:(CGFloat)red
              green:(CGFloat)green
               blue:(CGFloat)blue
              alpha:(CGFloat)alpha
            message:(NSString *)message;

@end


@implementation UIColor_AdWhirlConfigTest

-(void)setUp {
}

- (void)tearDown {
}

- (void)compareDict:(NSDictionary *)dict
            withRed:(CGFloat)red
              green:(CGFloat)green
               blue:(CGFloat)blue
              alpha:(CGFloat)alpha
            message:(NSString *)message {
  STAssertNotNil(dict, @"Input dict is nil, message passed: %@", message);
  UIColor *color = [[UIColor alloc] initWithDict:dict];
  STAssertNotNil(color, @"Dict should yield a UIColor.  dict: %@  message: %@",
                 dict, message);
  UIColor *compColor = [[UIColor alloc] initWithRed:red
                                              green:green
                                               blue:blue
                                              alpha:alpha];
  STAssertNotNil(compColor,
                 @"Comparison color should not be nil."
                 @" r:%lf g:%lf b:%lf a:%lf  message:%@",
                 red, green, blue, alpha, message);
  STAssertTrue(CGColorEqualToColor(color.CGColor, compColor.CGColor), message);
  [color release];
  [compColor release];
}

- (void)testGoodColors {
  NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
                        @"127", @"red",
                        @"133", @"green",
                        @"122", @"blue",
                        @"0.37", @"alpha",
                        nil];
  [self compareDict:dict
            withRed:127.0/255
              green:133.0/255.0
               blue:122.0/255.0
              alpha:0.37
            message:@"Dict color with strings should be equal to colors"];

  dict = [NSDictionary dictionaryWithObjectsAndKeys:
          [NSNumber numberWithInt:127], @"red",
          [NSNumber numberWithInt:133], @"green",
          [NSNumber numberWithInt:122], @"blue",
          [NSNumber numberWithDouble:0.37], @"alpha",
          nil];
  [self compareDict:dict
            withRed:127.0/255
              green:133.0/255.0
               blue:122.0/255.0
              alpha:0.37
            message:@"Dict color with NSNumber should be equal to colors"];
}

- (void)testMissingColors {
  NSDictionary *dictMissingRed = [NSDictionary dictionaryWithObjectsAndKeys:
                                  @"133", @"green",
                                  @"122", @"blue",
                                  nil];
  STAssertNil([[UIColor alloc] initWithDict:dictMissingRed],
              @"Dict missing red should yield nil UIColor");

  NSDictionary *dictMissingGreen = [NSDictionary dictionaryWithObjectsAndKeys:
                                    @"127", @"red",
                                    @"122", @"blue",
                                    nil];
  STAssertNil([[UIColor alloc] initWithDict:dictMissingGreen],
              @"Dict missing green should yield nil UIColor");

  NSDictionary *dictMissingBlue = [NSDictionary dictionaryWithObjectsAndKeys:
                                   @"127", @"red",
                                   @"133", @"green",
                                   nil];
  STAssertNil([[UIColor alloc] initWithDict:dictMissingBlue],
              @"Dict missing blue should yield nil UIColor");
}

- (void)testBadAlpha {
  NSDictionary *dictMissingAlpha = [NSDictionary dictionaryWithObjectsAndKeys:
                                    @"127", @"red",
                                    @"133", @"green",
                                    @"122", @"blue",
                                    nil];
  [self compareDict:dictMissingAlpha
            withRed:127.0/255.0
              green:133.0/255.0
               blue:122.0/255.0
              alpha:1.0
            message:@"Missing alpha should default to 1.0"];

  NSDictionary *dictBadAlpha = [NSDictionary dictionaryWithObjectsAndKeys:
                                @"127", @"red",
                                @"133", @"green",
                                @"122", @"blue",
                                @"blah", @"alpha",
                                nil];
  [self compareDict:dictBadAlpha
            withRed:127.0/255.0
              green:133.0/255.0
               blue:122.0/255.0
              alpha:0.0
            message:@"Non-numeric alpha should make 0.0"];

  NSDictionary *dictNegAlpha = [NSDictionary dictionaryWithObjectsAndKeys:
                                @"127", @"red",
                                @"133", @"green",
                                @"122", @"blue",
                                @"-0.4", @"alpha",
                                nil];
  [self compareDict:dictNegAlpha
            withRed:127.0/255.0
              green:133.0/255.0
               blue:122.0/255.0
              alpha:0.0
            message:@"Negative alpha should make 0.0"];

  NSDictionary *dictTooBigAlpha = [NSDictionary dictionaryWithObjectsAndKeys:
                                @"127", @"red",
                                @"133", @"green",
                                @"122", @"blue",
                                @"100", @"alpha",
                                nil];
  [self compareDict:dictTooBigAlpha
            withRed:127.0/255.0
              green:133.0/255.0
               blue:122.0/255.0
              alpha:1.0
            message:@"Out of range alpha should default to 1.0"];
}

- (void)testBadDictValues {
  NSValue *dummy = [NSValue valueWithPointer:"dummy"];

  NSDictionary *dictBadRed = [NSDictionary dictionaryWithObjectsAndKeys:
                              dummy, @"red",
                              @"133", @"green",
                              @"122", @"blue",
                              nil];
  STAssertNil([[UIColor alloc] initWithDict:dictBadRed],
              @"Dict with invalid red value should yield nil UIColor");

  NSDictionary *dictBadGreen = [NSDictionary dictionaryWithObjectsAndKeys:
                                @"127", @"red",
                                dummy, @"green",
                                @"122", @"blue",
                                nil];
  STAssertNil([[UIColor alloc] initWithDict:dictBadGreen],
              @"Dict with invalid green value should yield nil UIColor");

  NSDictionary *dictBadBlue = [NSDictionary dictionaryWithObjectsAndKeys:
                               @"127", @"red",
                               @"133", @"green",
                               dummy, @"blue",
                               nil];
  STAssertNil([[UIColor alloc] initWithDict:dictBadBlue],
              @"Dict with invalid blue value should yield nil UIColor");

  NSDictionary *dictBadAlphaType = [NSDictionary dictionaryWithObjectsAndKeys:
                                    @"127", @"red",
                                    @"133", @"green",
                                    @"122", @"blue",
                                    @"100", dummy,
                                    nil];
  [self compareDict:dictBadAlphaType
            withRed:127.0/255.0
              green:133.0/255.0
               blue:122.0/255.0
              alpha:1.0
            message:@"Alpha with invalid type should default to 1.0"];
}

@end
