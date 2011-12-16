#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import "AdWhirlDelegateProtocol.h"
#import "EAGLViewController.h"
#import "EAGLView.h"
#import "ESRenderer.h"

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView<AdWhirlDelegate,UIGestureRecognizerDelegate>
{    
    AdWhirlView *adView;
    //This is a trick, AdMob uses a viewController to display its Ads, trust me, you'll need this
    EAGLViewController *viewController;
    
@private
    id <ESRenderer> renderer;

    BOOL animating;
	BOOL started;
    NSInteger animationFrameInterval;
    // Use of the CADisplayLink class is the preferred method for controlling your animation timing.
    // CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
    // The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
    // isn't available.
    id displayLink;
    CGPoint currentLocation;
}
@property (nonatomic, retain) AdWhirlView *adView;
@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
@property(nonatomic, readwrite) CGPoint currentLocation;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView:(id)sender;

- (void)updateKeyboard: (NSString *) inputString;
- (void)removeAds;
- (void)displayAds;

@end
