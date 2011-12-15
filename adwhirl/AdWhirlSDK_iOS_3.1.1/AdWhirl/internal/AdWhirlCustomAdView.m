/*

 AdWhirlCustomAdView.m
 
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

#import "AdWhirlCustomAdView.h"
#import "AdWhirlView.h"
#import "AdWhirlLog.h"


@implementation AdWhirlCustomAdView

@synthesize delegate;
@synthesize image;
@synthesize textLabel;
@synthesize redirectURL;
@synthesize clickMetricsURL;
@synthesize adType;
@synthesize launchType;
@synthesize animType;
@synthesize backgroundColor;
@synthesize textColor;

- (id)initWithDelegate:(id<AdWhirlCustomAdViewDelegate>)d
                  text:(NSString *)txt
           redirectURL:(NSURL *)rURL
       clickMetricsURL:(NSURL *)cURL
                adType:(AWCustomAdType)aType
            launchType:(AWCustomAdLaunchType)launch
              animType:(AWCustomAdWebViewAnimType)anim
       backgroundColor:(UIColor *)bgColor
             textColor:(UIColor *)fgColor {
  
  self = [super initWithFrame:kAdWhirlViewDefaultFrame];
  if (self != nil) {
    delegate = d;
    redirectURL = [rURL retain];
    clickMetricsURL = [cURL retain];
    adType = aType;
    launchType = launch;
    animType = anim;
    backgroundColor = [bgColor retain];
    textColor = [fgColor retain];
    
    if (adType == AWCustomAdTypeText) {
      textLabel = [[UILabel alloc] initWithFrame:CGRectMake(50, 0, 270, CGRectGetHeight(self.bounds))];
      textLabel.text = txt;
      textLabel.textColor = fgColor;
      textLabel.numberOfLines = 3;
      textLabel.backgroundColor = [UIColor clearColor];
      textLabel.font = [UIFont fontWithName:@"TrebuchetMS-Bold" size:13.0];
      [self addSubview:textLabel];
    }
    
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
    button.frame = self.bounds;
    button.showsTouchWhenHighlighted = YES;
    [button addTarget:self action:@selector(buttonTapUp:) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:button];
  }
  return self;
}

#define kNumBgColors 3
#define kImageLeft 4
#define kCornerRadius 7.0
#define kImageDim 39 // assume square, so this is length of each side
#define kChamferLight [UIColor colorWithWhite:0.9 alpha:1].CGColor
#define kChamferDark [UIColor colorWithWhite:0.4 alpha:1].CGColor

- (void)drawRect:(CGRect)rect {

  CGContextRef ctx = UIGraphicsGetCurrentContext();
  if (adType == AWCustomAdTypeText) {

    // draw background
    CGFloat locations[kNumBgColors] = {0.0, 0.7, 1.0};
    CGColorRef colorArray[kNumBgColors] =
      {[backgroundColor colorWithAlphaComponent:0.6].CGColor,
       backgroundColor.CGColor,
       backgroundColor.CGColor};
    CFArrayRef colors = CFArrayCreate(kCFAllocatorDefault,
                                      (const void **)colorArray,
                                      kNumBgColors,
                                      &kCFTypeArrayCallBacks);
    CGGradientRef gradient = CGGradientCreateWithColors(NULL, colors, locations);
    CFRelease(colors);
    CGContextSetFillColorWithColor(ctx, [UIColor whiteColor].CGColor);
    CGContextFillRect(ctx, CGRectMake(0, 0, CGRectGetWidth(self.bounds), CGRectGetHeight(self.bounds)));
    CGPoint midY = CGPointMake(0.0, CGRectGetHeight(self.bounds)/2);
    CGPoint lowY = CGPointMake(0.0, CGRectGetHeight(self.bounds));
    CGContextDrawLinearGradient(ctx, gradient, CGPointZero, midY, 0);
    CGContextDrawLinearGradient(ctx, gradient, lowY, midY, 0);
    CGGradientRelease(gradient);
    
    // draw image and chamfer
    CGFloat imageTop = (CGRectGetHeight(self.bounds) - kImageDim)/2.0;
    CGPoint tl = CGPointMake(kImageLeft+kCornerRadius, imageTop+kCornerRadius);
    CGPoint tr = CGPointMake(kImageLeft+kImageDim-kCornerRadius, imageTop+kCornerRadius);
    CGPoint br = CGPointMake(kImageLeft+kImageDim-kCornerRadius, imageTop+kImageDim-kCornerRadius);
    CGPoint bl = CGPointMake(kImageLeft+kCornerRadius, imageTop+kImageDim-kCornerRadius);
    CGContextSaveGState(ctx);
    CGContextMoveToPoint(ctx, kImageLeft, imageTop+kCornerRadius);
    CGContextAddArc(ctx, tl.x, tl.y, kCornerRadius, M_PI, 3*M_PI/2, 0);
    CGContextAddArc(ctx, tr.x, tr.y, kCornerRadius, 3*M_PI/2, 0, 0);
    CGContextAddArc(ctx, br.x, br.y, kCornerRadius, 0, M_PI/2, 0);
    CGContextAddArc(ctx, bl.x, bl.y, kCornerRadius, M_PI/2, M_PI, 0);
    CGContextClosePath(ctx);
    CGContextClip(ctx);
    [image drawAtPoint:CGPointMake(kImageLeft, imageTop)];
    CGContextSetLineWidth(ctx, 0.5);
    CGContextMoveToPoint(ctx, kImageLeft, imageTop+kImageDim-kCornerRadius);
    CGContextSetStrokeColorWithColor(ctx, kChamferDark);
    CGContextAddArc(ctx, tl.x, tl.y, kCornerRadius, M_PI, 5*M_PI/4, 0);
    CGContextStrokePath(ctx);
    CGContextSetStrokeColorWithColor(ctx, kChamferLight);
    CGContextAddArc(ctx, tl.x, tl.y, kCornerRadius, 5*M_PI/4, 3*M_PI/2, 0);
    CGContextAddArc(ctx, tr.x, tr.y, kCornerRadius, 3*M_PI/2, 0, 0);
    CGContextAddArc(ctx, br.x, br.y, kCornerRadius, 0, M_PI/4, 0);
    CGContextStrokePath(ctx);
    CGContextSetStrokeColorWithColor(ctx, kChamferDark);
    CGContextAddArc(ctx, br.x, br.y, kCornerRadius, M_PI/4, M_PI/2, 0);
    CGContextAddArc(ctx, bl.x, bl.y, kCornerRadius, M_PI/2, M_PI, 0);
    CGContextStrokePath(ctx);
    CGContextRestoreGState(ctx);
    
  } // text ad
  else if (adType == AWCustomAdTypeBanner) {
    // draw image, place image in center of frame
    [image drawAtPoint:CGPointMake((self.frame.size.width-image.size.width)/2,
                                   (self.frame.size.height-image.size.height)/2)];
  } // banner ad
}

- (void)dealloc {
  [image release], image = nil;
  [textLabel release], textLabel = nil;
  [redirectURL release], redirectURL = nil;
  [clickMetricsURL release], clickMetricsURL = nil;
  [backgroundColor release], backgroundColor = nil;
  [textColor release], textColor = nil;
  [super dealloc];
}

#pragma mark UIButton control events

- (void)buttonTapUp:(id)sender {
  if (delegate != nil) {
    [delegate adTapped:self];
  }
}

@end
