#import "EAGLView.h"

#import "ES2Renderer.h"

#include <sys/time.h>
#include "JGE.h"
#include "JTypes.h"
#include "JApp.h"
#include "JFileSystem.h"
#include "JRenderer.h"
#include "JGameLauncher.h"

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
	return CFAbsoluteTimeGetCurrent() * 1000;
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


@implementation EAGLView

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
	
    if ((self = [super initWithFrame:frame])) {
		
		self = [self initialize];
		
    } 
	
    return self;
}


//The EAGL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder
{    
    if ((self = [super initWithCoder:coder]))
    {
		self = [self initialize];
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
/*
    // doesn't work as expected.  Need to figure out correct algorithm.  Double tap or double swipe down will execute OK button.
        NSInteger xDiff = abs(static_cast<int>(currentX) - xOffset);
        NSInteger yDiff = abs(static_cast<int>(currentY) - yOffset);
        
        if (xDiff <= 60 && yDiff <= 60)
        {
            g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
        }
*/
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

    CGPoint translatedPoint = [recognizer locationInView: self];
    currentX = translatedPoint.x;
    currentY = translatedPoint.y;
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


- (void) handleTouchEvent: (NSSet *) touches withEvent: (UIEvent *) event
{
    UITouch     *touch = [[event touchesForView:self] anyObject];
    NSUInteger  numberOfTouches = [[event touchesForView: self] count];
	
    // Convert touch point from UIView referential to OpenGL one (upside-down flip)
    currentLocation = [touch previousLocationInView:self];
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
        
        if (touch.tapCount == 1 && numberOfTouches == 1)
        {
            g_engine->LeftClicked(xOffset, yOffset);
            if ( touch.phase == UITouchPhaseEnded)
                g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
        }
        
	}
    
    else if((currentLocation.y > es2renderer.viewPort.bottom) && (currentLocation.x < actualWidth/2)) {

        if (touch.phase == UITouchPhaseEnded)
            g_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
	}
    else if((currentLocation.y > es2renderer.viewPort.bottom) && (currentLocation.x >= actualWidth/2)) {

        if (touch.phase == UITouchPhaseEnded)
            g_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
	}    
}

// Handles the start of a touch
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self handleTouchEvent: touches withEvent: event];
}


// Handles the continuation of a touch.
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{  
    //[self handleTouchEvent: touches withEvent: event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    //[self handleTouchEvent:touches withEvent:event];
}

// Handles the end of a touch event.
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    // If appropriate, add code necessary to save the state of the application.
    // This application is not saving state.
}

#pragma mark -
@end

