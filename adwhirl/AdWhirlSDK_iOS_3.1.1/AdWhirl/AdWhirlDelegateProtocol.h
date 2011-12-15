/*

 AdWhirlDelegateProtocol.h

 Copyright 2009 AdMob, Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

*/

#import <CoreLocation/CoreLocation.h>
#import <UIKit/UIKit.h>

@class AdWhirlView;

@protocol AdWhirlDelegate<NSObject>

@required

- (NSString *)adWhirlApplicationKey;

/**
 * The view controller with which the ad network will display a modal view
 * (web view, canvas), such as when the user clicks on the ad. You must
 * supply a view controller. You should return the root view controller
 * of your application, such as the root UINavigationController, or
 * any controllers that are pushed/added directly to the root view controller.
 * For example, if your app delegate has a pointer to the root view controller:
 *
 * return [(MyAppDelegate *)[[UIApplication sharedApplication] delegate] rootViewController]
 *
 * will suffice.
 */
- (UIViewController *)viewControllerForPresentingModalView;

@optional

#pragma mark server endpoints
/**
 * If you are running your own AdWhirl server instance, make sure you
 * implement the following to return the URL that points to the endpoints
 * on your server.
 */
- (NSURL *)adWhirlConfigURL;
- (NSURL *)adWhirlImpMetricURL;
- (NSURL *)adWhirlClickMetricURL;
- (NSURL *)adWhirlCustomAdURL;


#pragma mark notifications
/**
 * You can listen to callbacks from AdWhirl via these methods.  When AdWhirl is
 * notified that an ad request is fulfilled, it will notify you immediately.
 * Thus, when notified that an ad request succeeded, you can choose to add the
 * AdWhirlView object as a subview to your view.  This view contains the ad.
 * When you are notified that an ad request failed, you are also informed if the
 * AdWhirlView is fetching a backup ad.  The backup fetching order is specified
 * by you in adwhirl.com or your own server instance.  When all backup sources
 * are attempted and the last ad request still fails, the usingBackup parameter
 * will be set to NO.  You can use this notification to try again and perhaps
 * request another AdWhirlView via [AdWhirlView requestAdWhirlViewWithDelegate:]
 */
- (void)adWhirlDidReceiveAd:(AdWhirlView *)adWhirlView;
- (void)adWhirlDidFailToReceiveAd:(AdWhirlView *)adWhirlView usingBackup:(BOOL)yesOrNo;

/**
 * You can get notified when the transition animation to a new ad is completed
 * so you can make necessary adjustments to the size of the adWhirlView and
 * surrounding views after the animation.
 */
- (void)adWhirlDidAnimateToNewAdIn:(AdWhirlView *)adWhirlView;

/**
 * This function is your integration point for Generic Notifications. You can
 * control when this notification occurs via the developers member section.  You
 * can allocate a percentage of your ad requests to initiate this callback.  When
 * you receive this notification, you can execute any code block that you own.
 * For example, you can replace the ad in AdWhirlView after getting this callback
 * by calling replaceBannerViewWith: . Note that the ad refresh cycle is still
 * alive, so your view could be replaced by other ads when it's time for an
 * ad refresh.
 */
- (void)adWhirlReceivedRequestForDeveloperToFufill:(AdWhirlView *)adWhirlView;

/**
 * In the event that ads are OFF, you can listen to this callback method to
 * determine that ads have been turned off.
 */
- (void)adWhirlReceivedNotificationAdsAreOff:(AdWhirlView *)adWhirlView;

/**
 * These notifications will let you know when a user is being shown a full screen
 * webview canvas with an ad because they tapped on an ad.  You should listen to
 * these notifications to determine when to pause/resume your game--if you're
 * building a game app.
 */
- (void)adWhirlWillPresentFullScreenModal;
- (void)adWhirlDidDismissFullScreenModal;

/**
 * An ad request is a two step process: first the SDK must go to the AdWhirl
 * server to retrieve configuration information. Then, based on the configuration
 * information, it chooses an ad network and fetch an ad. The following call
 * is for users to get notified when the first step is complete. The
 * adWhirlView passed could be null if you had called the AdWhirlView class
 * method +startPreFetchingConfigurationDataWithDelegate .
 */
- (void)adWhirlDidReceiveConfig:(AdWhirlView *)adWhirlView;


#pragma mark behavior configurations

/**
 * Request test ads for APIs that supports it. Make sure you turn it to OFF
 * or remove the function before you submit your app to the app store.
 */
- (BOOL)adWhirlTestMode;

/**
 * Returns the device's current orientation for ad networks that relys on
 * it. If you don't implement this function, [UIDevice currentDevice].orientation
 * is used to get the current orientation.
 */
- (UIDeviceOrientation)adWhirlCurrentOrientation;

#pragma mark appearance configurations
- (UIColor *)adWhirlAdBackgroundColor;
- (UIColor *)adWhirlTextColor;
- (UIColor *)adWhirlSecondaryTextColor;
- (UIColor *)backgroundColor DEPRECATED_ATTRIBUTE; // use the one with adWhirl prefix
- (UIColor *)textColor DEPRECATED_ATTRIBUTE; // use the one with adWhirl prefix


#pragma mark hard-coded application keys
- (NSString *)admobPublisherID; // your Publisher ID from Admob.
- (NSDictionary *)quattroWirelessDictionary;  // key-value pairs for the keys "publisherID" and "siteID" provided by Quattro Wireless.  Set NSString values for these two keys.
- (NSString *)pinchApplicationKey; // your Application Code from Pinch Media.
- (NSDictionary *)videoEggConfigDictionary;  // key-value pairs for the keys "publisher" and "area" information from Video Egg.  Set NSString values for these two keys.
- (NSString *)millennialMediaApIDString;  // your ApID string from Millennial Media.
- (NSString *)MdotMApplicationKey; // your Application Code from MdotM
- (NSString *)googleAdSenseClientID; // your publisher ID from Google AdSense
- (NSString *)zestADZClientID; // your clientID from ZestADZ
- (NSString *)brightRollAppId; // your BrightRoll App ID
- (NSString *)inMobiAppID; // your inMobi app ID
- (NSString *)oneRiotAppID;
- (NSDictionary *) nexageDictionary; // your nexage dcn and position


#pragma mark demographic information optional delegate methods
- (CLLocation *)locationInfo; // user's current location
- (NSString *)postalCode; // user's postal code, e.g. "94401"
- (NSString *)areaCode; // user's area code, e.g. "415"
- (NSDate *)dateOfBirth; // user's date of birth
- (NSString *)gender; // user's gender (e.g. @"m" or @"f")
- (NSString *)keywords; // keywords the user has provided or that are contextually relevant, e.g. @"twitter client iPhone"
- (NSString *)searchString; // a search string the user has provided, e.g. @"Jasmine Tea House San Francisco"
- (NSUInteger)incomeLevel; // return actual annual income


#pragma mark QuattroWireless-specific optional delegate methods
/**
 * Return the ad type desired for Quattro
 * QWAdTypeBanner = 0,
 * QWAdTypeText=2,
 */
- (NSUInteger)quattroWirelessAdType;

/**
 * Return a value for the education level if you have access to this info.  This
 * information will be relayed to Quattro Wireless if provided.
 * QWEducationNoCollege = 0
 * QWEducationCollegeGraduate = 1
 * QWEducationGraduateSchool = 2
 * QWEducationUnknown = 3
 */
- (NSUInteger)quattroWirelessEducationLevel;

/**
 * Return a value for the ethnicity if you have access to this info.  This
 * information will be relayed to Quattro Wireless if provided.
 * QWEthnicGroupAfrican_American = 0
 * QWEthnicGroupAsian = 1
 * QWEthnicGroupHispanic = 2
 * QWEthnicGroupWhite = 3
 * QWEthnicGroupOther = 4
 */
- (NSUInteger)quattroWirelessEthnicity;


#pragma mark MillennialMedia-specific optional delegate methods
/**
 * Return the ad type desired for Millennial Media, depending on your ad position
 * MMBannerAdTop = 1,
 * MMBannerAdBottom = 2,
 */
- (NSUInteger)millennialMediaAdType;

/**
 * Return a value for the education level if you have access to this info.  This
 * information will be relayed to Millennial Media if provided
 * MMEducationUnknown = 0,
 * MMEducationHishSchool = 1,
 * MMEducationSomeCollege = 2,
 * MMEducationInCollege = 3,
 * MMEducationBachelorsDegree = 4,
 * MMEducationMastersDegree = 5,
 * MMEducationPhD = 6
 */
- (NSUInteger)millennialMediaEducationLevel;

/**
 * Return a value for ethnicity if you have access to this info.  This
 * information will be relayed to Millennial Media if provided.
 * MMEthnicityUnknown = 0,
 * MMEthnicityAfricanAmerican = 1,
 * MMEthnicityAsian = 2,
 * MMEthnicityCaucasian = 3,
 * MMEthnicityHispanic = 4,
 * MMEthnicityNativeAmerican = 5,
 * MMEthnicityMixed = 6
 */
- (NSUInteger)millennialMediaEthnicity;

- (NSUInteger)millennialMediaAge DEPRECATED_ATTRIBUTE; // use dateOfBirth


#pragma mark Jumptap-specific optional delegate methods
/**
 * optional site and spot id as provided by Jumptap.
 */
- (NSString *)jumptapSiteId;
- (NSString *)jumptapSpotId;

/**
 * Find a list of valid categories at https://support.jumptap.com/index.php/Valid_Categories
 */
- (NSString *)jumptapCategory;

/**
 * Whether adult content is allowed.
 * AdultContentAllowed = 0,
 * AdultContentNotAllowed = 1,
 * AdultContentOnly = 2
 */
- (NSUInteger)jumptapAdultContent;

/**
 * The transition to use when moving from, say, a banner to full-screen.
 * TransitionHorizontalSlide = 0,
 * TransitionVerticalSlide = 1,
 * TransitionCurl = 2,
 * TransitionFlip = 3
 */
- (NSUInteger)jumptapTransitionType;


#pragma mark Google AdSense-specific delegate methods
/**
 These are *REQUIRED* for googleAdSense. If you don't implement these methods,
 your app will crash as the AdSense adapter looks for these methods.
 */
- (NSString *)googleAdSenseCompanyName;
- (NSString *)googleAdSenseAppName;
- (NSString *)googleAdSenseApplicationAppleID;

/**
 The following are optional and correspond to the optional kGADAdSense* ad attributes.
 For documentation, see GADAdSenseParameters.h .
 */
- (NSString *)googleAdSenseKeywords;
- (NSURL *)googleAdSenseAppWebContentURL;
- (NSArray *)googleAdSenseChannelIDs;
- (NSString *)googleAdSenseAdType;
- (NSString *)googleAdSenseHostID;
//- (UIColor *)googleAdSenseAdBackgroundColor; // implement adWhirlAdBackgroundColor or set in server
- (UIColor *)googleAdSenseAdTopBackgroundColor;
- (UIColor *)googleAdSenseAdBorderColor;
- (UIColor *)googleAdSenseAdLinkColor;
//- (UIColor *)googleAdSenseAdTextColor; // implement adWhirlTextColor or set in server
- (UIColor *)googleAdSenseAdURLColor;
- (UIColor *)googleAdSenseAlternateAdColor;
- (NSURL *)googleAdSenseAlternateAdURL;
- (NSNumber *)googleAdSenseAllowAdsafeMedium;


#pragma mark InMobi-specific optional delegate methods
/**
 * Education level for InMobi
 * Edu_None = 0
 * Edu_HighSchool = 1
 * Edu_SomeCollege = 2
 * Edu_InCollege = 3
 * Edu_BachelorsDegree = 4
 * Edu_MastersDegree = 5
 * Edu_DoctoralDegree = 6
 * Edu_Other = 7
 */
- (NSUInteger)inMobiEducation;

/**
 Eth_None = 0,
 Eth_Mixed = 1,
 Eth_Asian = 2,
 Eth_Black = 3,
 Eth_Hispanic = 4,
 Eth_NativeAmerican = 5,
 Eth_White = 6,
 Eth_Other = 7
 */
- (NSUInteger)inMobiEthnicity;

/**
 * See inMobi's documentation for valid values
 */
- (NSString *)inMobiInterests;

- (NSString *)iAdAdvertisingSection;

- (NSDictionary *)inMobiParamsDictionary;

#pragma mark OneRiot-specific optional delegate methods

/** Returns an array of NSStrings containing all optional context parameters
 */
- (NSArray *)oneRiotContextParameters;

#pragma mark Nexage-specific optional delegate methods
-(NSString *)nexageCity;
-(NSString *)nexageDesignatedMarketArea;
-(NSString *)nexageCountry;
-(NSString *)nexageEthnicity;
-(NSString *)nexageMaritalStatus;

@end
