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

-(id)initWithFrame:(CGRect)frame {
	
	NSLog(@"EAGL View - init With Frame: origin(%f %f) size(%f %f)", 
		  frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
	
    if ((self = [super initWithFrame:frame])) {
		
		self = [self initialize];
		
    } // if ((self = [super initWithFrame:frame]))
	
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
     Create a double tap recognizer to handle OK.
     */
    recognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleOK:)];
    [self addGestureRecognizer:recognizer];
	((UITapGestureRecognizer*)recognizer).numberOfTapsRequired = 2;
    [recognizer release];
	
    /*
     Create a 2 fingers right swipe gesture recognizer to handle next phase.
     */
    recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleNextPhase:)];
	((UISwipeGestureRecognizer*)recognizer).direction = UISwipeGestureRecognizerDirectionRight;
	((UISwipeGestureRecognizer*)recognizer).numberOfTouchesRequired = 2;
    [self addGestureRecognizer:recognizer];
    [recognizer release];
    
    /*
     Create a 2 fingers left swipe gesture recognizer to handle interruption.
     */
    recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleInterrupt:)];
	((UISwipeGestureRecognizer*)recognizer).direction = UISwipeGestureRecognizerDirectionLeft;
	((UISwipeGestureRecognizer*)recognizer).numberOfTouchesRequired = 2;
    [self addGestureRecognizer:recognizer];
    [recognizer release];	
	
    /*
     Create a 3 fingers up swipe gesture recognizer to handle menu.
     */
    recognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(handleMenu:)];
	((UISwipeGestureRecognizer*)recognizer).direction = UISwipeGestureRecognizerDirectionUp;
	((UISwipeGestureRecognizer*)recognizer).numberOfTouchesRequired = 3;
    [self addGestureRecognizer:recognizer];
    [recognizer release];	
	
	/*
     Create a 2 fingers long press gesture recognizer to handle hand display.
     */
    recognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleHand:)];
	((UILongPressGestureRecognizer*)recognizer).minimumPressDuration = 1;
	((UILongPressGestureRecognizer*)recognizer).numberOfTouchesRequired = 2;
	
    [self addGestureRecognizer:recognizer];
    [recognizer release];
	
	return self;	
}

- (void)handleHand:(UILongPressGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
}

- (void)handleMenu:(UISwipeGestureRecognizer *)recognizer {
	g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
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
		
		if (!InitGame())
		{
			//return 1;
		}
		
		JGECreateDefaultBindings();
		
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

// Handles the start of a touch
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch*            touch = [[event touchesForView:self] anyObject];
	
    // Convert touch point from UIView referential to OpenGL one (upside-down flip)
    currentLocation = [touch previousLocationInView:self];
	ES2Renderer* es2renderer = (ES2Renderer*)renderer;
	
	int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
	int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();
	if (currentLocation.y >= es2renderer.viewPort.top && 
		currentLocation.y <= es2renderer.viewPort.bottom &&
		currentLocation.x <= es2renderer.viewPort.right &&
		currentLocation.x >= es2renderer.viewPort.left) {
		g_engine->LeftClicked(
							  ((currentLocation.x-es2renderer.viewPort.left)*SCREEN_WIDTH)/actualWidth, 
							  ((currentLocation.y-es2renderer.viewPort.top)*SCREEN_HEIGHT)/actualHeight);
	} else if(currentLocation.y<es2renderer.viewPort.top) {
		g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);		
	} else if(currentLocation.y>es2renderer.viewPort.bottom) {
		g_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
	}
}

// Handles the continuation of a touch.
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{  
    UITouch*            touch = [[event touchesForView:self] anyObject];
	
    // Convert touch point from UIView referential to OpenGL one (upside-down flip)
    currentLocation = [touch previousLocationInView:self];
	ES2Renderer* es2renderer = (ES2Renderer*)renderer;
	
	int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
	int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();
	if (currentLocation.y >= es2renderer.viewPort.top && 
		currentLocation.y <= es2renderer.viewPort.bottom &&
		currentLocation.x <= es2renderer.viewPort.right &&
		currentLocation.x >= es2renderer.viewPort.left) {
		g_engine->LeftClicked(
			((currentLocation.x-es2renderer.viewPort.left)*SCREEN_WIDTH)/actualWidth, 
			((currentLocation.y-es2renderer.viewPort.top)*SCREEN_HEIGHT)/actualHeight);
	} else if(currentLocation.y<es2renderer.viewPort.top) {
		g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);		
	} else if(currentLocation.y>es2renderer.viewPort.bottom) {
		g_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
	}
}

// Handles the end of a touch event.
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    // If appropriate, add code necessary to save the state of the application.
    // This application is not saving state.
}

@end

