#import "ES2Renderer.h"

#include <sys/time.h>
#include "JGE.h"
#include "JTypes.h"
#include "JApp.h"
#include "JFileSystem.h"
#include "JRenderer.h"
#include "JGameLauncher.h"

#define ACTUAL_SCREEN_WIDTH (SCREEN_WIDTH)
#define ACTUAL_SCREEN_HEIGHT (SCREEN_HEIGHT)
#define ACTUAL_RATIO ((GLfloat)ACTUAL_SCREEN_WIDTH / (GLfloat)ACTUAL_SCREEN_HEIGHT)

extern JGE*		g_engine;
extern uint64_t lastTickCount;
bool checkFramebufferStatus();


@implementation ES2Renderer

@synthesize viewPort;
@synthesize backingHeight;

// Create an OpenGL ES 2.0 context
- (id)init
{
    if ((self = [super init]))
    {
		backingWidth = -1;
		backingHeight = -1;

        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        if ( context == nil)
            context = [[EAGLContext alloc] initWithAPI: kEAGLRenderingAPIOpenGLES1];
                       
        if (!context || ![EAGLContext setCurrentContext:context])
        {
            [self release];
            return nil;
        }
	}

    return self;
}

- (void)render
{
//	NSLog(@"Renderer - render");

	struct timeval tv;
	uint dt;
    
	// This application only creates a single context which is already set current at this point.
    // This call is redundant, but needed if dealing with multiple contexts.
    [EAGLContext setCurrentContext:context];
	
	// This application only creates a single default framebuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple framebuffers.
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);

	if ((GLfloat)backingWidth / (GLfloat)backingHeight < ACTUAL_RATIO)
	{
		viewPort.left = 0;
		viewPort.top = -((backingWidth/ACTUAL_RATIO)-backingHeight)/2;
		viewPort.right = backingWidth;
		viewPort.bottom = -((backingWidth/ACTUAL_RATIO)-backingHeight)/2 + backingWidth / ACTUAL_RATIO;
	}
	else
	{
		viewPort.left = -(backingHeight*ACTUAL_RATIO-backingWidth)/2;
		viewPort.top = 0;
		viewPort.right = backingHeight * ACTUAL_RATIO;
		viewPort.bottom = -((backingWidth/ACTUAL_RATIO)-backingHeight)/2 + backingWidth / ACTUAL_RATIO + backingHeight;
	}
	
	glViewport(viewPort.left, viewPort.top, viewPort.right-viewPort.left, viewPort.bottom-viewPort.top);

	JRenderer::GetInstance()->SetActualWidth(viewPort.right-viewPort.left);
	JRenderer::GetInstance()->SetActualHeight(viewPort.bottom-viewPort.top);
	
	
	gettimeofday(&tv, NULL);
	uint64_t tickCount = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	dt = (tickCount - lastTickCount);
	lastTickCount = tickCount;
	
	g_engine->SetDelta((float)dt / 1000.0f);
	g_engine->Update((float)dt / 1000.0f);
	
	g_engine->Render();

    // This application only creates a single color renderbuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple renderbuffers.
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER];
}


- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer
{
	if(defaultFramebuffer) {
		glDeleteFramebuffers(1, &defaultFramebuffer);
		defaultFramebuffer = 0;
	}
	
	if(colorRenderbuffer) {
		glDeleteRenderbuffers(1, &colorRenderbuffer);
		colorRenderbuffer = 0;
	}
	
	glGenFramebuffers(1, &defaultFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
	
	glGenRenderbuffers(1, &colorRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
	
    // Allocate color buffer backing based on the current layer size
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);

	checkFramebufferStatus();

	glDepthFunc(GL_LEQUAL);				// The Type Of Depth Testing (Less Or Equal)
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);				// do not calculate inside of poly's
	glFrontFace(GL_CCW);	
	glEnable (GL_BLEND);
 	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glEnable(GL_SCISSOR_TEST);				// Enable Clipping

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		// Black Background (yes that's the way fuckers)
	glClearDepthf(1.0f);					// Depth Buffer Setup
		
    return YES;
}

bool checkFramebufferStatus() {
	
    GLenum status = (GLenum)glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
    switch(status) {
			
        case GL_FRAMEBUFFER_COMPLETE:
            return true;
			
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            printf("Framebuffer incomplete,incomplete attachment\n");
            return false;
			
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf("Unsupported framebuffer format\n");
            return false;
			
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            printf("Framebuffer incomplete,missing attachment\n");
            return false;
			
    }
	
	return false;
}


- (void)dealloc
{
    // Tear down GL
    if (defaultFramebuffer)
    {
        glDeleteFramebuffers(1, &defaultFramebuffer);
        defaultFramebuffer = 0;
    }

    if (colorRenderbuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderbuffer);
        colorRenderbuffer = 0;
    }

    // Tear down context
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];

    [context release];
    context = nil;

    [super dealloc];
}

@end
