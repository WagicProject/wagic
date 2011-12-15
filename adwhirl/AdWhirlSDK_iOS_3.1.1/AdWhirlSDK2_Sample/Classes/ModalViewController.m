//
//  ModalViewController.m
//  AdWhirlSDK2_Sample
//
//  Created by Nigel Choi on 3/11/10.
//  Copyright 2010 Admob. Inc. All rights reserved.
//

#import "ModalViewController.h"


@implementation ModalViewController

- (id)init {
  if (self = [super initWithNibName:@"ModalViewController" bundle:nil]) {
    self.title = @"Modal View";
    if ([self respondsToSelector:@selector(setModalTransitionStyle)]) {
      [self setModalTransitionStyle:UIModalTransitionStyleCoverVertical];
    }
  }
  return self;
}

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
  return YES;
}

- (IBAction)dismiss:(id)sender {
  [self dismissModalViewControllerAnimated:YES];
}

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end
