#import "EAGLViewController.h"
#import "EAGLView.h"



@interface EAGLViewController (PrivateMethods)
- (NSString*)interfaceOrientationName:(UIInterfaceOrientation) interfaceOrientation;
- (NSString*)deviceOrientationName:(UIDeviceOrientation) deviceOrientation;
@end

@implementation EAGLViewController

- (id)init {
    self = [super init];
    if (self) {
        // Custom initialization.
    }
    return self;
}

- (void)dealloc {
    [super dealloc];
}


// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
	NSLog(@"EAGL ViewController - loadView");
	
	CGRect frame = [[UIScreen mainScreen] applicationFrame];
	
	EAGLView *eaglView = [[[EAGLView alloc] initWithFrame:frame] autorelease];
	
	self.view = eaglView;
}


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
	NSLog(@"EAGL ViewController - view Did Load");
	[super viewDidLoad];
}

- (void)viewWillAppear:(BOOL)animated {
	
	NSLog(@"EAGL ViewController - view Will Appear");
	
}


- (void)viewDidAppear:(BOOL)animated {
	
	NSLog(@"EAGL ViewController - view Did Appear");
	
	UIDeviceOrientation currentDeviceOrientation = [UIDevice currentDevice].orientation;
	UIInterfaceOrientation currentInterfaceOrientation	= self.interfaceOrientation;
	
	NSLog(@"Current Interface: %@. Current Device: %@", 
		  [self interfaceOrientationName:currentInterfaceOrientation], 
		  [self deviceOrientationName:currentDeviceOrientation]);
}



- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Overriden to allow any orientation.
    return YES;
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration {
	
	UIDeviceOrientation currentDeviceOrientation = [UIDevice currentDevice].orientation;
	UIInterfaceOrientation currentInterfaceOrientation	= self.interfaceOrientation;
	
	NSLog(@"EAGL ViewController - will Rotate To Interface: %@. Current Interface: %@. Current Device: %@", 
		  [self interfaceOrientationName:toInterfaceOrientation], 
		  [self interfaceOrientationName:currentInterfaceOrientation], 
		  [self deviceOrientationName:currentDeviceOrientation]);
	
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation {
	
	UIDeviceOrientation currentDeviceOrientation = [UIDevice currentDevice].orientation;
	UIInterfaceOrientation currentInterfaceOrientation	= self.interfaceOrientation;
	
	NSLog(@"EAGL ViewController - did Rotate From Interface: %@. Current Interface: %@. Current Device: %@", 
		  [self interfaceOrientationName:fromInterfaceOrientation], 
		  [self interfaceOrientationName:currentInterfaceOrientation], 
		  [self deviceOrientationName:currentDeviceOrientation]);
	
}


- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}


- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (NSString*)interfaceOrientationName:(UIInterfaceOrientation) interfaceOrientation {
	
	NSString* result = nil;
	
	switch (interfaceOrientation) {
			
		case UIInterfaceOrientationPortrait:
			result = @"Portrait";
			break;
		case UIInterfaceOrientationPortraitUpsideDown:
			result = @"Portrait UpsideDown";
			break;
		case UIInterfaceOrientationLandscapeLeft:
			result = @"LandscapeLeft";
			break;
		case UIInterfaceOrientationLandscapeRight:
			result = @"LandscapeRight";
			break;
		default:
			result = @"Unknown Interface Orientation";
	}
	
	return result;
};

- (NSString*)deviceOrientationName:(UIDeviceOrientation) deviceOrientation {
	
	NSString* result = nil;
	
	switch (deviceOrientation) {
			
		case UIDeviceOrientationUnknown:
			result = @"Unknown";
			break;
		case UIDeviceOrientationPortrait:
			result = @"Portrait";
			break;
		case UIDeviceOrientationPortraitUpsideDown:
			result = @"Portrait UpsideDown";
			break;
		case UIDeviceOrientationLandscapeLeft:
			result = @"LandscapeLeft";
			break;
		case UIDeviceOrientationLandscapeRight:
			result = @"LandscapeRight";
			break;
		case UIDeviceOrientationFaceUp:
			result = @"FaceUp";
			break;
		case UIDeviceOrientationFaceDown:
			result = @"FaceDown";
			break;
		default:
			result = @"Unknown Device Orientation";
	}
	
	return result;
};



@end
