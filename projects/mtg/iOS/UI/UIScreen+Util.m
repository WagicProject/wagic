//
//  UIScreen+Util.m
//  wagic
//
//  Created by Michael Nguyen on 11/27/13.
//
//
#include "UIScreen+Util.h"

static BOOL isRetinaScreen = NO;
static BOOL didRetinaCheck = NO;

@implementation UIScreen (Util)
+ (BOOL)isRetinaDisplay
{
    if (!didRetinaCheck) {
        isRetinaScreen = ([[self mainScreen] respondsToSelector:@selector(displayLinkWithTarget:selector:)] &&
                          ([self mainScreen].scale == 2.0));
        didRetinaCheck = YES;
    }
    return isRetinaScreen;
}
@end