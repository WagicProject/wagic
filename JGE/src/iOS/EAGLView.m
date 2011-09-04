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
    
    UIGestureRecognizer *recognizer;
    
    /*
     Create a 2 fingers left swipe gesture recognizer to handle interruption.
     */
    recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleInterrupt:)];
	((UISwipeGestureRecognizer*)recognizer).direction = UISwipeGestureRecognizerDirectionLeft;
	((UISwipeGestureRecognizer*)recognizer).numberOfTouchesRequired = 2;
    [self addGestureRecognizer:recognizer];
    [recognizer release];	
    
    /* 
     Create a touch and hold to handle opening up the menu
     */
    UILongPressGestureRecognizer *menuKeyRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleMenuWithLongPress:)];
    menuKeyRecognizer.minimumPressDuration =2;
    menuKeyRecognizer.numberOfTouchesRequired = 1;
    menuKeyRecognizer.delaysTouchesBegan = YES;
    [self addGestureRecognizer:menuKeyRecognizer];
    
    
    /*
     Create a single tap recognizer to handle OK.
     */
    UITapGestureRecognizer *singleTapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleOK:)];
    singleTapRecognizer.numberOfTapsRequired = 1;
    singleTapRecognizer.numberOfTouchesRequired = 1;
    [singleTapRecognizer requireGestureRecognizerToFail: menuKeyRecognizer];
    [singleTapRecognizer setDelaysTouchesBegan: YES];
    
    [self addGestureRecognizer: singleTapRecognizer];
    
    
    /*
     Create a Pan recognizer 
     */
    UIPanGestureRecognizer *panRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget: self action: @selector(handlePan:)];
    panRecognizer.minimumNumberOfTouches = 1;
    panRecognizer.maximumNumberOfTouches = 1;
    [recognizer requireGestureRecognizerToFail: singleTapRecognizer];
    [self addGestureRecognizer: panRecognizer];
    
    /*
     Create a 2 fingers right swipe gesture recognizer to handle opening and closing of hand
     */
    recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleHand:)];
	((UISwipeGestureRecognizer*)recognizer).direction = UISwipeGestureRecognizerDirectionRight;
	((UISwipeGestureRecognizer*)recognizer).numberOfTouchesRequired = 2;
    
    [self addGestureRecognizer:recognizer];
    [recognizer release];
    
    [panRecognizer release];
    [menuKeyRecognizer release];
    [singleTapRecognizer release];
    
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


- (void)handlePan: (UIPanGestureRecognizer *)recognizer {
    
    [[[(UITapGestureRecognizer*)recognizer view] layer] removeAllAnimations];
    
	[self bringSubviewToFront:[(UIPanGestureRecognizer*)recognizer view]];
	CGPoint translatedPoint = [(UIPanGestureRecognizer*)recognizer translationInView: self];
    
	if([(UIPanGestureRecognizer*)recognizer state] == UIGestureRecognizerStateBegan) {
        
		firstX = [[recognizer view] center].x;
		firstY = [[recognizer view] center].y;
	}
    
	translatedPoint = CGPointMake(firstX+translatedPoint.x, firstY+translatedPoint.y);
    
    //	[[recognizer view] setCenter:translatedPoint];
    
	if([(UIPanGestureRecognizer*)recognizer state] == UIGestureRecognizerStateEnded) {
        
        CGFloat xVelocity = [(UIPanGestureRecognizer*)recognizer velocityInView: self].x;
        CGFloat yVelocity = [(UIPanGestureRecognizer*)recognizer velocityInView: self].y;
		CGFloat finalX = translatedPoint.x + (.35 * xVelocity);
		CGFloat finalY = translatedPoint.y + (.35 * yVelocity);
        
		if(UIDeviceOrientationIsPortrait([[UIDevice currentDevice] orientation])) {
			if(finalX < 0) 
            { 				 				
                finalX = 0;
 			} 			 			
            else if(finalX > 768) {
				finalX = 768;
			}
            
			if(finalY < 0) { 	
                finalY = 0; 		
            } 			 			
            else if(finalY > 1024) {    
				finalY = 1024;
			}
		}
        
		else {
            
			if(finalX < 0) 
            {
                finalX = 0; 			
            }
            else if(finalX > 1024) {
                
				finalX = 768;
			}
            
			if(finalY < 0) {
                finalY = 0;
            }
            else if(finalY > 768) {   
				finalY = 1024;
			}
		}
        
        int xVelocityAbs = abs(static_cast<int>(xVelocity));
        int yVelocityAbs = abs(static_cast<int>(yVelocity));

        
        if (yVelocityAbs > 300 && (xVelocityAbs < yVelocityAbs))
        {
            bool flickUpwards = yVelocity < 0;
            g_engine->HoldKey_NoRepeat(flickUpwards ? JGE_BTN_UP : JGE_BTN_DOWN);
        }
        else if (xVelocityAbs > 300  && (xVelocityAbs > yVelocityAbs))
        {
            bool flickRight = xVelocity > 0;
            g_engine->HoldKey_NoRepeat( flickRight ? JGE_BTN_RIGHT : JGE_BTN_LEFT);
        }
    }
    
}

- (void)handleHand:(UITapGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
}

- (void)handleMenu:(UISwipeGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
}


- (void)handleMenuWithLongPress:(UILongPressGestureRecognizer *)recognizer {
    if (recognizer.state == UIGestureRecognizerStateEnded) {
        NSLog(@"Long press Ended");
        g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
    }
    else {
        NSLog(@"Long press detected.");
    }
}

- (void)handleInterrupt:(UISwipeGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
    
}

- (void)handleNextPhase:(UISwipeGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
}

- (void)handleOK:(UITapGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
}

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


- (void) handleTouchEvent: (NSSet *) touches withEvent: (UIEvent *) event
{
    UITouch*            touch = [[event touchesForView:self] anyObject];
	
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
        NSLog( @"clicked %d, %d: tapCount: %d", xOffset, yOffset, touch.tapCount);
        
        if (touch.tapCount == 1)
        {            
            g_engine->LeftClicked(xOffset, yOffset);
        }
        
	}
    
    else if(currentLocation.y < es2renderer.viewPort.top) {
        NSLog(@"Menu Button");
		g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);		
	} 
    
    else if(currentLocation.y > es2renderer.viewPort.bottom) {
        NSLog( @"Right Trigger");
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
    [self handleTouchEvent: touches withEvent: event];
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

@end

