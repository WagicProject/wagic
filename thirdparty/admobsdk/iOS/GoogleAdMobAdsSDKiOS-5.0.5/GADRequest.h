//
//  GADRequest.h
//  Google AdMob Ads SDK
//
//  Copyright 2011 Google Inc. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

// Constant for getting test ads on the simulator using the testDevices method.
#define GAD_SIMULATOR_ID @"Simulator"

// Genders to help deliver more relevant ads.
typedef enum {
  kGADGenderUnknown,
  kGADGenderMale,
  kGADGenderFemale
} GADGender;

// Specifies optional parameters for ad requests.
@interface GADRequest : NSObject <NSCopying>

// Creates an autoreleased GADRequest.
+ (GADRequest *)request;

// Passes extra details in ad requests.
//
// One case is for Ad Network Mediation. Some Ad Networks may ask for additional
// information about the ad request. Consult with the individual Ad Network
// on what to send. Place the information in a dictionary and put that in
// another dictionary under the key "mediation". An example might be:
//
//   additionalParameters = {
//     mediation: {
//       MyAdNetwork: {
//         market_segment: "abc",
//         some_info: "xyz",
//         some_num: 1000
//       },
//       AdNetworkX: {
//         key1: "val1",
//         key2: "val2"
//       }
//     }
//   }
//
// To create such a dictionary, do the following:
//
//    NSDictionary *myAdNetwork = [NSDictionary dictionaryWithObjectsAndKeys:
//                                 @"abc", @"market_segment",
//                                 @"xyz", @"some_info",
//                                 [NSNumber numberWithInt:1000], @"some_num",
//                                 nil];
//    NSDictionary *adNetwokX = [NSDictionary dictionaryWithObjectsAndKeys:
//                               @"val1", @"key1",
//                               @"val2", @"key2",
//                               nil];
//    NSDictionary *mediation = [NSDictionary dictionaryWithObjectsAndKeys:
//                               myAdNetwork, @"MyAdNetwork",
//                               adNetwokX, @"AdNetworkX",
//                               nil];
//    NSDictionary *params = [NSDictionary dictionaryWithObjectsAndKeys:
//                            mediation, @"mediation",
//                            nil];
//    gadRequest.additionalParameters = params;
@property (nonatomic, retain) NSDictionary *additionalParameters;

#pragma mark Collecting SDK Information

// Returns the version of the SDK.
+ (NSString *)sdkVersion;

#pragma mark Testing

// Test ads are returned to these devices.  Device identifiers are the same used
// to register as a development device with Apple.  To obtain a value open the
// Organizer (Window -> Organizer from Xcode), control-click or right-click on
// the device's name, and choose "Copy Device Identifier".  Alternatively you
// can obtain it through code using [[UIDevice currentDevice] uniqueIdentifier].
//
// For example:
//   request.testDevices = [NSArray arrayWithObjects:
//       GAD_SIMULATOR_ID,                               // Simulator
//       //@"28ab37c3902621dd572509110745071f0101b124",  // Test iPhone 3G 3.0.1
//       @"8cf09e81ef3ec5418c3450f7954e0e95db8ab200",    // Test iPod 4.3.1
//       nil];
@property (nonatomic, retain) NSArray *testDevices;

#pragma mark User Information

// The user's gender may be used to deliver more relevant ads.
@property (nonatomic, assign) GADGender gender;

// The user's birthday may be used to deliver more relevant ads.
@property (nonatomic, retain) NSDate *birthday;
- (void)setBirthdayWithMonth:(NSInteger)m day:(NSInteger)d year:(NSInteger)y;

// The user's current location may be used to deliver more relevant ads.
// However do not use Core Location just for advertising, make sure it is used
// for more beneficial reasons as well.  It is both a good idea and part of
// Apple's guidelines.
- (void)setLocationWithLatitude:(CGFloat)latitude longitude:(CGFloat)longitude
                       accuracy:(CGFloat)accuracyInMeters;

// When Core Location isn't available but the user's location is known supplying
// it here may deliver more relevant ads.  It can be any free-form text such as
// @"Champs-Elysees Paris" or @"94041 US".
- (void)setLocationWithDescription:(NSString *)locationDescription;

#pragma mark Contextual Information

// A keyword is a word or phrase describing the current activity of the user
// such as @"Sports Scores".  Each keyword is an NSString in the NSArray.  To
// clear the keywords set this to nil.
@property (nonatomic, retain) NSMutableArray *keywords;

// Convenience method for adding keywords one at a time such as @"Sports Scores"
// and then @"Football".
- (void)addKeyword:(NSString *)keyword;

#pragma mark -
#pragma mark Deprecated Methods

// Please use testDevices instead.
@property (nonatomic, getter=isTesting) BOOL testing;

@end
