#import "EAGLView.h"

#import "ES2Renderer.h"

#include <sys/time.h>
#include "JGE.h"
#include "JTypes.h"
#include "JApp.h"
#include "JFileSystem.h"
#include "JRenderer.h"
#include "JGameLauncher.h"

#include "GameApp.h"

#import "AdWhirlView.h"
#import "wagicAppDelegate.h"


uint64_t		lastTickCount;
JGE*		g_engine = NULL;
static JApp*	g_app = NULL;
static JGameLauncher* g_launcher = NULL;
CGFloat lastScale;
CGFloat lastRotation;

CGFloat firstX;
CGFloat firstY;

CGFloat currentX;
CGFloat currentY;

void JGECreateDefaultBindings()
{
}

int JGEGetTime()
{
    timeval time;
    gettimeofday(&time, NULL);
    long millis = ((time.tv_sec * 1000) + (time.tv_usec / 1000)/ 1000);
    return millis;
}

bool InitGame(void)
{
	g_engine = JGE::GetInstance();
	g_app = g_launcher->GetGameApp();
	g_app->Create();
	g_engine->SetApp(g_app);
	
	JRenderer::GetInstance()->Enable2D();
	struct timeval tv;
	gettimeofday(&tv, NULL);
	lastTickCount = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    
	return true;
}

void DestroyGame(void)
{
	g_engine->SetApp(NULL);
	if (g_app)
	{
		g_app->Destroy();
		delete g_app;
		g_app = NULL;
	}
	
	JGE::Destroy();
	
	g_engine = NULL;
}


#pragma mark Ad management constants
static NSString *ADMOB_PUBLISHERID_IPHONE = @"a14edb722890ca1"; // Erwans-a14e9009f88864f
static NSString *ADMOB_PUBLISHERID_IPAD = @"a14edb7c59598f7";
static NSString *_MY_AD_WHIRL_APPLICATION_KEY_IPHONE = @"b86aba511597401ca6b41c1626aa3013";
static NSString *_MY_AD_WHIRL_APPLICATION_KEY_IPAD = @"2e70e3f3da40408588b9a3170c8d268f";

#pragma mark -

@implementation EAGLView

@synthesize adView;
@synthesize animating;
@dynamic animationFrameInterval;
@synthesize currentLocation;

#pragma mark class initialization methods

// You must implement this method
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (void)dealloc
{
    [renderer release];
    [self removeAds];	
    [super dealloc];
}


- (void) initGestureRecognizers {

    
    UIGestureRecognizer *recognizer;
    
    /*
     create swipe handlers for single swipe
     */
    UISwipeGestureRecognizer *singleFlickGestureRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget: self action: @selector(handleFlickGesture:)];
    singleFlickGestureRecognizer.numberOfTouchesRequired = 1;
    singleFlickGestureRecognizer.direction = UISwipeGestureRecognizerDirectionRight;
    [self addGestureRecognizer: singleFlickGestureRecognizer];
    [singleFlickGestureRecognizer release];	
    
    singleFlickGestureRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget: self action: @selector(handleFlickGesture:)];
    singleFlickGestureRecognizer.numberOfTouchesRequired = 1;
    singleFlickGestureRecognizer.direction = UISwipeGestureRecognizerDirectionLeft;
    [self addGestureRecognizer: singleFlickGestureRecognizer];
    [singleFlickGestureRecognizer release];	
    
    singleFlickGestureRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget: self action: @selector(handleFlickGesture:)];
    singleFlickGestureRecognizer.numberOfTouchesRequired = 1;
    singleFlickGestureRecognizer.direction = UISwipeGestureRecognizerDirectionUp;
    [self addGestureRecognizer: singleFlickGestureRecognizer];
    [singleFlickGestureRecognizer release];	
    
    singleFlickGestureRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget: self action: @selector(handleFlickGesture:)];
    singleFlickGestureRecognizer.numberOfTouchesRequired = 1;
    singleFlickGestureRecognizer.direction = UISwipeGestureRecognizerDirectionDown;
    [self addGestureRecognizer: singleFlickGestureRecognizer];
    [singleFlickGestureRecognizer release];	

    
    /*
     Create a 3 fingers left swipe gesture recognizer to handle left trigger operations
     */
    UISwipeGestureRecognizer *threeFingerSwipeLeftRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleNextPhase:)];
    threeFingerSwipeLeftRecognizer.direction = UISwipeGestureRecognizerDirectionLeft;
    threeFingerSwipeLeftRecognizer.numberOfTouchesRequired = 3;
    
    [self addGestureRecognizer: threeFingerSwipeLeftRecognizer];
    [threeFingerSwipeLeftRecognizer release];
    
    /*
     Create a 3 fingers right swipe gesture recognizer to handle opening and closing of hand. (right trigger)
     */
    UISwipeGestureRecognizer *threeFingerSwipeRightRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleHand:)];
    threeFingerSwipeRightRecognizer.direction = UISwipeGestureRecognizerDirectionRight;
    threeFingerSwipeRightRecognizer.numberOfTouchesRequired = 3;
    
    [self addGestureRecognizer: threeFingerSwipeRightRecognizer];
    [threeFingerSwipeRightRecognizer release];
    
    /*
     Create a 2 fingers left swipe gesture recognizer to handle interruption. (square key)
     */
    recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleInterrupt:)];
	((UISwipeGestureRecognizer*)recognizer).direction = UISwipeGestureRecognizerDirectionLeft;
	((UISwipeGestureRecognizer*)recognizer).numberOfTouchesRequired = 2;
    [self addGestureRecognizer:recognizer];
    [recognizer requireGestureRecognizerToFail: threeFingerSwipeLeftRecognizer];
    [recognizer release];	
    
    
    /*
     Create a 2 fingers right swipe gesture recognizer. (circle key)
     */
    recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleSecondary:)];
	((UISwipeGestureRecognizer*)recognizer).direction = UISwipeGestureRecognizerDirectionRight;
	((UISwipeGestureRecognizer*)recognizer).numberOfTouchesRequired = 2;
    [self addGestureRecognizer:recognizer];
    [recognizer requireGestureRecognizerToFail: threeFingerSwipeRightRecognizer];
    [recognizer release];	
    
    /*
     Create a 2 fingers down swipe gesture recognizer to handle interruption. (cross key)
     */
    recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleOK:)];
	((UISwipeGestureRecognizer*)recognizer).direction = UISwipeGestureRecognizerDirectionDown;
	((UISwipeGestureRecognizer*)recognizer).numberOfTouchesRequired = 2;
    [self addGestureRecognizer:recognizer];
    [recognizer release];	
    
    
    /*
     Create a 2 fingers up swipe gesture recognizer. (triangle key)
     */
    recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleCancel:)];
	((UISwipeGestureRecognizer*)recognizer).direction = UISwipeGestureRecognizerDirectionUp;
	((UISwipeGestureRecognizer*)recognizer).numberOfTouchesRequired = 2;
    [self addGestureRecognizer:recognizer];
    [recognizer release];

    /*
     Create a recognizer for the select key functionality
     */
    UILongPressGestureRecognizer *selectKeyRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleSelectKey:)];
    selectKeyRecognizer.minimumPressDuration =2;
    selectKeyRecognizer.numberOfTouchesRequired = 2;
    selectKeyRecognizer.delaysTouchesBegan = YES;
    [self addGestureRecognizer:selectKeyRecognizer];

    /* 
     Create a touch and hold to handle opening up the menu
     */
    UILongPressGestureRecognizer *menuKeyRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleMenuWithLongPress:)];
    menuKeyRecognizer.minimumPressDuration =2;
    menuKeyRecognizer.numberOfTouchesRequired = 1;
    [menuKeyRecognizer requireGestureRecognizerToFail: selectKeyRecognizer];
    menuKeyRecognizer.delaysTouchesBegan = YES;
    [self addGestureRecognizer:menuKeyRecognizer];
    
    /*
     Create a double tap recognizer to handle OK.
     */
    UITapGestureRecognizer *doubleTapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleOK:)];
    doubleTapRecognizer.numberOfTapsRequired = 2;
    doubleTapRecognizer.numberOfTouchesRequired = 1; 
    [self addGestureRecognizer: doubleTapRecognizer];

    /*
     Create a single tap recognizer to select the nearest object.
     */
    UITapGestureRecognizer *singleTapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleSingleTap:)];
    singleTapRecognizer.numberOfTapsRequired = 1;
    singleTapRecognizer.numberOfTouchesRequired = 1;

    [self addGestureRecognizer: singleTapRecognizer];

    
    /*
     Use the pinch gesture recognizer to zoom in and out of a location on the screen.
     */
    UIPinchGestureRecognizer *pinchGestureRecognizer = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(pinchDetected:)];
    [pinchGestureRecognizer setDelaysTouchesEnded: YES];
    [self addGestureRecognizer: pinchGestureRecognizer];
    [pinchGestureRecognizer release];
    
    [menuKeyRecognizer release];
    [selectKeyRecognizer release];
    [doubleTapRecognizer release];
    [singleTapRecognizer release];
}


- (id)initialize {
	
	NSLog(@"EAGL View - initialize EAGL");
	
	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
	
	NSLog(@"bounds: %f %f %f %f", eaglLayer.bounds.origin.x, eaglLayer.bounds.origin.y, eaglLayer.bounds.size.width, eaglLayer.bounds.size.height);
	
	eaglLayer.opaque = TRUE;
	eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
									[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, 
									kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, 
									nil];
	
	renderer = [[ES2Renderer alloc] init];
	
	if (!renderer)
	{
		NSLog(@"OpenGl ES2 Renderer creation failed, time to code some OpenGl ES1.1 Renderer !!!");
		
		[self release];
		return nil;
	}
	
	animating = FALSE;
	started = FALSE;
	animationFrameInterval = 1;
	displayLink = nil;

    [self initGestureRecognizers];
    
	return self;	
}


-(id)initWithFrame:(CGRect)frame {
	
	NSLog(@"EAGL View - init With Frame: origin(%f %f) size(%f %f)", 
		  frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
	
    self = [super initWithFrame:frame];
    if (self)
        [self initialize];
	
    return self;
}


//The EAGL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder
{    
    self = [super initWithCoder:coder];

    if (self)
    {
        [self initialize];
    }
    
    return self;
}

#pragma mark -

#pragma mark Animation callbacks/methods

- (void)drawView:(id)sender
{
    [renderer render];
}

- (void)layoutSubviews
{
	[self stopAnimation];
    [renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
	[self startAnimation];
    
    [self drawView:nil];
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;
        
        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating)
    {
		// CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
		// if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
		// not be called in system versions earlier than 3.1.
		
		displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
		[displayLink setFrameInterval:animationFrameInterval];
		[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        animating = TRUE;
    }
	
	if(!started)
	{
		// Init JGE mess		
		g_launcher = new JGameLauncher();
		
		u32 flags = g_launcher->GetInitFlags();
		
		if ((flags&JINIT_FLAG_ENABLE3D)!=0)
		{
			JRenderer::Set3DFlag(true);
		}
		
		JGECreateDefaultBindings();
        
		if (!InitGame())
		{
			//return 1;
		}
		
		started = TRUE;
	}
}

- (void)stopAnimation
{
    if (animating)
    {
		[displayLink invalidate];
		displayLink = nil;
		
        animating = FALSE;
    }
}

#pragma mark -

#pragma mark Gesture Recognizer callbacks

- (void)handleSingleTap: (UITapGestureRecognizer *) recognizer {
    [[[recognizer view] layer] removeAllAnimations];
    currentLocation = [recognizer locationInView: self];
    ES2Renderer* es2renderer = (ES2Renderer*)renderer;
	
	int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
	int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();
	
    if (currentLocation.y > es2renderer.viewPort.top && 
		currentLocation.y < es2renderer.viewPort.bottom &&
		currentLocation.x < es2renderer.viewPort.right &&
		currentLocation.x > es2renderer.viewPort.left) 
    {
        
        int xOffset = ((currentLocation.x-es2renderer.viewPort.left)*SCREEN_WIDTH)/actualWidth;
        int yOffset = ((currentLocation.y-es2renderer.viewPort.top)*SCREEN_HEIGHT)/actualHeight;
        
        g_engine->LeftClicked(xOffset, yOffset);

    // doesn't work as expected.  Need to figure out correct algorithm.  Double tap or double swipe down will execute OK button.
        NSInteger xDiff = abs(static_cast<int>(currentX) - xOffset);
        NSInteger yDiff = abs(static_cast<int>(currentY) - yOffset);
        
        if (xDiff <= 60 && yDiff <= 60)
        {
            g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
        }

        currentX = xOffset;
        currentY = yOffset;
	}
        
    else if(currentLocation.y < es2renderer.viewPort.top) {
        NSLog(@"Menu Button");
        if (recognizer.state == UIGestureRecognizerStateEnded)
            g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);		
	} 
    
    else if((currentLocation.y > es2renderer.viewPort.bottom) && (currentLocation.x < actualWidth/2)) {
        if (recognizer.state == UIGestureRecognizerStateEnded)
            g_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
	}
    else if((currentLocation.y > es2renderer.viewPort.bottom) && (currentLocation.x >= actualWidth/2)) {
        NSLog( @"Right Trigger");
		if (recognizer.state == UIGestureRecognizerStateEnded)
            g_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
	}
}

- (void)handleFlickGesture: (UISwipeGestureRecognizer *) recognizer {

    switch (recognizer.direction) {
        case UISwipeGestureRecognizerDirectionDown:
            g_engine->HoldKey_NoRepeat( JGE_BTN_DOWN );
            break;
        case UISwipeGestureRecognizerDirectionUp:
            g_engine->HoldKey_NoRepeat( JGE_BTN_UP);
            break;
        case UISwipeGestureRecognizerDirectionLeft:
            g_engine->HoldKey_NoRepeat( JGE_BTN_LEFT );
            break;
        case UISwipeGestureRecognizerDirectionRight:
            g_engine->HoldKey_NoRepeat( JGE_BTN_RIGHT );
            break;
            
        default:
            break;
    }
/*
    CGPoint translatedPoint = [recognizer locationInView: self];
    currentX = translatedPoint.x;
    currentY = translatedPoint.y;
    g_engine->LeftClicked( currentX, currentY);
 */
}

- (void)handleHand:(UITapGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
}

/* implement a zoom in /out function call */
- (IBAction)pinchDetected:(UIGestureRecognizer *)sender {
/*    
    CGFloat scale = 
    [(UIPinchGestureRecognizer *)sender scale];
    CGFloat velocity = 
    [(UIPinchGestureRecognizer *)sender velocity];
    
    g_engine->Zoom( scale, velocity );
*/
}


- (void)handleSelectKey:(UILongPressGestureRecognizer *) recognizer {
    if ( recognizer.state == UIGestureRecognizerStateEnded )
    {
        g_engine->HoldKey_NoRepeat(JGE_BTN_CTRL);
    }
}


- (void)handleMenuWithLongPress:(UILongPressGestureRecognizer *)recognizer {
    if (recognizer.state == UIGestureRecognizerStateEnded) {
        g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
    }
}

- (void)handleNextPhase:(UISwipeGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
}

- (void)handleOK:(UITapGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
}

- (void)handleSecondary: (UISwipeGestureRecognizer *)recognizer {
    g_engine->HoldKey_NoRepeat(JGE_BTN_PRI);
}

- (void)handleInterrupt:(UISwipeGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
    
}

- (void)handleCancel:(UISwipeGestureRecognizer *) recognizer {
    g_engine->HoldKey_NoRepeat(JGE_BTN_CANCEL);
}

#pragma mark -
#include "GameOptions.h"
#pragma mark Keyboard related methods

- (void) updateKeyboard:( NSString *) inputString
{
    // send the new string to JGE to update the string
    unsigned char key = [inputString characterAtIndex: 0];
    if ([inputString length] > 1)
    {
        if ([inputString isEqualToString: @"DELETE"])
            key = 127;
        else if ([inputString isEqualToString:@"SPACE"])
            key = 32;
        else if ([inputString isEqualToString: @"SAVE"])
            key = 1;
        else if ([inputString isEqualToString: @"CANCEL"])
            key = 10;
    }
    
    options.keypadUpdateText( key );
    if ( key < 11 )
        g_engine->HoldKey_NoRepeat( JGE_BTN_OK) ;
    
}


//These are the methods for the AdWhirl Delegate, you have to implement them
#pragma mark AdWhirlDelegate methods
- (void) resumeGame {
    g_engine->Resume();
}

- (void) pauseGame
{
    g_engine->Pause();
}

- (void)adWhirlWillPresentFullScreenModal {
    //It's recommended to invoke whatever you're using as a "Pause Menu" so your
    //game won't keep running while the user is "playing" with the Ad (for example, iAds)
    [self pauseGame];

}

- (void)adWhirlDidDismissFullScreenModal {
    //Once the user closes the Ad he'll want to return to the game and continue where
    //he left it
    [self resumeGame];
}

- (NSString *)adWhirlApplicationKey {
    if ((UI_USER_INTERFACE_IDIOM()) == UIUserInterfaceIdiomPad)
        return _MY_AD_WHIRL_APPLICATION_KEY_IPAD;
    
    return _MY_AD_WHIRL_APPLICATION_KEY_IPHONE;
}

- (UIViewController *)viewControllerForPresentingModalView {
    //Remember that UIViewController we created in the Game.h file? AdMob will use it.
    //If you want to use "return self;" instead, AdMob will cancel the Ad requests.
    return viewController;
}


- (void)adWhirlDidReceiveAd:(AdWhirlView *)adWhirlView {
    [UIView beginAnimations:@"AdWhirlDelegate.adWhirlDidReceiveAd:"
                    context:nil];
    BOOL isLandscape = UIDeviceOrientationIsLandscape( [UIDevice currentDevice].orientation);
    [UIView setAnimationDuration:0.7];
    
    CGSize adSize = [adWhirlView actualAdSize];
    CGRect newFrame = [adWhirlView frame];
    CGSize screenSize = [self.window bounds].size;
    
    newFrame.size = adSize;
    // ads are 320 x 50
    newFrame.origin.x = ( (isLandscape ? screenSize.height : screenSize.width) - adSize.width)/ 2;
    newFrame.origin.y =  0;
    
    [adWhirlView setFrame: newFrame];
    
    [UIView commitAnimations];
}

-(void)adWhirlDidFailToReceiveAd:(AdWhirlView *)adWhirlView usingBackup:(BOOL)yesOrNo {
    //The code to show my own Ad banner again
    NSLog(@"failed to get an Ad");
}

-(void) removeAds {
    //There's something weird about AdWhirl because setting the adView delegate
    //to "nil" doesn't stops the Ad requests and also it doesn't remove the adView
    //from superView; do the following to remove AdWhirl from your scene.
    //
    //If adView exists, remove everything
    if (adView) {
        //Remove adView from superView
        [adView removeFromSuperview];
        //Replace adView's view with "nil"
        [adView replaceBannerViewWith:nil];
        //Tell AdWhirl to stop requesting Ads
        [adView ignoreNewAdRequests];
        //Set adView delegate to "nil"
        [adView setDelegate:nil];
        //Release adView
        [adView release];
        //set adView to "nil"
        adView = nil;
    }
}



-(void) displayAds 
{
    BOOL isLandscape = UIDeviceOrientationIsLandscape( [UIDevice currentDevice].orientation);
    
    //Assign the AdWhirl Delegate to our adView
    if ( adView != nil )
        [self removeAds];
    
    //Let's allocate the viewController (it's the same RootViewController as declared
    //in our AppDelegate; will be used for the Ads)
    viewController = [(wagicAppDelegate *)[[UIApplication sharedApplication] delegate] glViewController];

    self.adView = [AdWhirlView requestAdWhirlViewWithDelegate:self];
    //Set auto-resizing mask
    self.adView.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin|UIViewAutoresizingFlexibleRightMargin;
    //This isn't really needed but also it makes no harm. It just retrieves the configuration
    //from adwhirl.com so it knows what Ad networks to use
    [adView updateAdWhirlConfig];
    //Get the actual size for the requested Ad
    CGSize adSize = [adView actualAdSize];
    //
    //Set the position; remember that we are using 4 values (in this order): X, Y, Width, Height
    //You can comment this line if your game is in portrait mode and you want your Ad on the top
    //if you want the Ad in other position (portrait or landscape), use the following code,
    //for this example, the Ad will be positioned in the bottom+center of the screen
    //(in landscape mode):
    //Same explanation as the one in the method "adjustAdSize" for the Ad's width
    int screenWidth = [viewController.parentViewController.view bounds].size.width;
    if ( isLandscape )
        screenWidth = [viewController.parentViewController.view bounds].size.height;
    
    self.adView.frame = CGRectMake((screenWidth - adSize.width) / 2, 0, adSize.width, adSize.height);
    
    //Trying to keep everything inside the Ad bounds
    self.adView.clipsToBounds = YES;
    //Adding the adView (used for our Ads) to our viewController
    [viewController.view addSubview:adView];
    //Bring our view to front
    [viewController.view bringSubviewToFront:adView];
    
}


@end

