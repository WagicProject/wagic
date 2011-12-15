/*

 AdWhirlAdapterMdotM.m
 
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

#import "AdWhirlAdapterMdotM.h"
#import "AdWhirlView.h"
#import "AdWhirlConfig.h"
#import "AdWhirlAdNetworkConfig.h"
#import "AdWhirlDelegateProtocol.h"
#import "AdWhirlLog.h"
#import "AdWhirlAdNetworkAdapter+Helpers.h"
#import "AdWhirlAdNetworkRegistry.h"
#import "AdWhirlAdapterCustom.h"
#import "AdWhirlError.h"
#import "CJSONDeserializer.h"
#import "AdWhirlCustomAdView.h"


@interface AdWhirlAdapterMdotM ()

- (BOOL)parseAdData:(NSData *)data error:(NSError **)error;

@property (nonatomic,readonly) CLLocationManager *locationManager;
@property (nonatomic,retain) NSURLConnection *adConnection;
@property (nonatomic,retain) NSURLConnection *imageConnection;
@property (nonatomic,retain) AdWhirlCustomAdView *adView;
@property (nonatomic,retain) AdWhirlWebBrowserController *webBrowserController;

@end


@implementation AdWhirlAdapterMdotM

@synthesize adConnection;
@synthesize imageConnection;
@synthesize adView;
@synthesize webBrowserController;


+ (AdWhirlAdNetworkType)networkType {
  return AdWhirlAdNetworkTypeMdotM;
}

+ (void)load {
  [[AdWhirlAdNetworkRegistry sharedRegistry] registerClass:self];
}

- (BOOL)useTestAd {
  if ([adWhirlDelegate respondsToSelector:@selector(adWhirlTestMode)])
    return [adWhirlDelegate adWhirlTestMode];
  return NO;
}


- (id)initWithAdWhirlDelegate:(id<AdWhirlDelegate>)delegate
       view:(AdWhirlView *)view
           config:(AdWhirlConfig *)config
    networkConfig:(AdWhirlAdNetworkConfig *)netConf {
  self = [super initWithAdWhirlDelegate:delegate
    view:view
    config:config
    networkConfig:netConf];
  if (self != nil) {
    adData = [[NSMutableData alloc] init];
    imageData = [[NSMutableData alloc] init];
  }
  return self;
}

- (NSMutableString *)appendUserContextDic:(NSDictionary *)dic withUrl:(NSString *)sUrl {
  NSArray *keyArray = [dic allKeys];
  NSMutableString *str = [NSMutableString stringWithString:sUrl];
       
  //Iterate over the context disctionary and for each kay-value pair create a string of the format &key=value
  for (int i = 0; i < [keyArray count]; i++) {
    [str appendFormat:@"&%@=%@",[keyArray objectAtIndex:i], [dic objectForKey:[keyArray objectAtIndex:i]]];
  }
  return str;
}


- (void)getAd {
  @synchronized(self) {
    if (requesting) return;
    requesting = YES;
  }
           
  NSString *appKey = networkConfig.pubId;

  if ([adWhirlDelegate respondsToSelector:@selector(MdotMApplicationKey)] ) {
    appKey = [adWhirlDelegate MdotMApplicationKey];
  }

  UIDevice *device = [UIDevice currentDevice];
  NSBundle *bundle = [NSBundle mainBundle];
  NSLocale *locale = [NSLocale currentLocale];
  NSString *userAgent = [NSString stringWithFormat:@"%@ %@ (%@; %@ %@; %@)",
          [bundle objectForInfoDictionaryKey:@"CFBundleDisplayName"],
          [bundle objectForInfoDictionaryKey:@"CFBundleVersion"],
          [device model],
          [device systemName], [device systemVersion],
          [locale localeIdentifier]];
  int test;
  if ( [self useTestAd] ) {
    test = 1;
  } else
    test = 0;
       
  NSString *str = [NSString stringWithFormat:
            @"http://ads.mdotm.com/ads/feed.php?appver=%d&v=%@&apikey=mdotm&appkey=%@&deviceid=%@&width=320&height=50&fmt=json&ua=%@&test=%d",
          kAdWhirlAppVer, [[UIDevice currentDevice] systemVersion],
          appKey, [[UIDevice currentDevice] uniqueIdentifier], userAgent, test];
               
  NSMutableDictionary *userContextDic = [[NSMutableDictionary alloc] initWithCapacity:2];
  if ( [userContextDic count] > 0 ) {
    str = [self appendUserContextDic:userContextDic withUrl:str];
  }              
       
  NSString *urlString = [str stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];

  NSURL *adRequestURL =  [[NSURL alloc] initWithString:urlString];
  AWLogDebug(@"Requesting MdotM ad (%@) %@", str, adRequestURL);
  NSURLRequest *adRequest = [NSURLRequest requestWithURL:adRequestURL];
       

  NSURLConnection *conn = [[NSURLConnection alloc] initWithRequest:adRequest
               delegate:self];
  self.adConnection = conn;
  [conn release];
  [adRequestURL release];
  [userContextDic release];
}




#pragma mark MdotMDelegate optional methods

- (BOOL)respondsToSelector:(SEL)selector {
  if (selector == @selector(location)
      && ![adWhirlDelegate respondsToSelector:@selector(location)]) {
    return NO;
  }
  else if (selector == @selector(userContext)
     && ![adWhirlDelegate respondsToSelector:@selector(userContext)]) {
    return NO;
  }  return [super respondsToSelector:selector];
}


- (CLLocationManager *)locationManager {
  if (locationManager == nil) {
    locationManager = [[CLLocationManager alloc] init];
  }
  return locationManager;
}

- (BOOL)parseEnums:(int *)val
            adInfo:(NSDictionary*)info
            minVal:(int)min
            maxVal:(int)max
         fieldName:(NSString *)name
             error:(NSError **)error {
  NSString *str = [info objectForKey:name];
  if (str == nil) {
    if (error != nil)
      *error = [AdWhirlError errorWithCode:AdWhirlCustomAdDataError
                               description:[NSString stringWithFormat:
                                            @"MdotM ad data has no '%@' field", name]];
    return NO;
  }
  int intVal = [str intValue];
  if (intVal <= min || intVal >= max) {
    if (error != nil)
      *error = [AdWhirlError errorWithCode:AdWhirlCustomAdDataError
                               description:[NSString stringWithFormat:
                                            @"MdotM ad data: Invalid value for %@ - %d", name, intVal]];
    return NO;
  }
  *val = intVal;
  return YES;
}  

- (BOOL)parseAdData:(NSData *)data error:(NSError **)error {
  NSError *jsonError = nil;
  id parsed = [[CJSONDeserializer deserializer] deserialize:data error:&jsonError];
  if (parsed == nil) {
    if (error != nil)
      *error = [AdWhirlError errorWithCode:AdWhirlCustomAdParseError
                               description:@"Error parsing MdotM ad JSON from server"
                           underlyingError:jsonError];
    return NO;
  }
  if ([parsed isKindOfClass:[NSArray class]]) {
    NSArray *ads = parsed;
    NSDictionary *adInfo = nil;
    if ( [ads count] == 0 ) {
      return(NO);
    } else {
      id parsed0 =[ads objectAtIndex:0];
      if ( [parsed0 isKindOfClass:[NSDictionary class]] ) {
        adInfo = parsed0;
        
        // gather up and validate ad info
        NSString *text = [adInfo objectForKey:@"ad_text"];
        NSString *redirectURLStr = [adInfo objectForKey:@"landing_url"];
        
        int adTypeInt;
        if (![self parseEnums:&adTypeInt
                       adInfo:adInfo
                       minVal:AWCustomAdTypeMIN
                       maxVal:AWCustomAdTypeMAX
                    fieldName:@"ad_type"
                        error:error]) {
          return NO;
        }
        AWCustomAdType adType = adTypeInt;
        
        int launchTypeInt;
        if (![self parseEnums:&launchTypeInt
                       adInfo:adInfo
                       minVal:AWCustomAdLaunchTypeMIN
                       maxVal:AWCustomAdLaunchTypeMAX
                    fieldName:@"launch_type"
                        error:error]) {
          return NO;
        }      
        AWCustomAdLaunchType launchType = launchTypeInt;
        AWCustomAdWebViewAnimType animType = AWCustomAdWebViewAnimTypeCurlDown;
        
        NSURL *redirectURL = nil;
        if (redirectURLStr == nil) {
          AWLogWarn(@"No redirect URL for MdotM ad");
        } else {
          redirectURL = [[NSURL alloc] initWithString:redirectURLStr];
          if (!redirectURL)
            AWLogWarn(@"MdotM ad: Malformed redirect URL string %@", redirectURLStr);
        }
        AWLogDebug(@"Got MdotM ad %@ %@ %d %d %d", text, redirectURL,
                   adType, launchType, animType);
        
        self.adView = [[AdWhirlCustomAdView alloc] initWithDelegate:self
                                                               text:text
                                                        redirectURL:redirectURL
                                                    clickMetricsURL:nil
                                                             adType:adType
                                                         launchType:launchType
                                                           animType:animType
                                                    backgroundColor:[self helperBackgroundColorToUse]
                                                          textColor:[self helperTextColorToUse]];
        [self.adView release];
        self.adNetworkView = adView;
        [redirectURL release];
        if (adView == nil) {
          if (error != nil)
            *error = [AdWhirlError errorWithCode:AdWhirlCustomAdDataError
                                     description:@"Error initializing MdotM ad view"];
          return NO;
        }      
        
        // fetch image
        id imageURL = [adInfo objectForKey:@"img_url"];
        if ( [imageURL isKindOfClass:[NSString class]]) {
          AWLogDebug(@"Request MdotM ad image at %@", imageURL);
          NSURLRequest *imageRequest = [NSURLRequest requestWithURL:[NSURL URLWithString:imageURL]];
          NSURLConnection *conn = [[NSURLConnection alloc] initWithRequest:imageRequest
                                                                  delegate:self];
          self.imageConnection = conn;
          [conn release];
        } else {
          return(NO);
        }
      } else {
        return(NO);
      }
    }
  } else {
    if (error != nil)
      *error = [AdWhirlError errorWithCode:AdWhirlCustomAdDataError
                               description:@"Expected top-level dictionary in MdotM ad data"];
    return NO;
  }
  return YES;
}

- (void)stopBeingDelegate {
    AdWhirlCustomAdView *theAdView = (AdWhirlCustomAdView *)self.adNetworkView;
    if (theAdView != nil) {
        theAdView.delegate = nil;
    }
}

- (void)dealloc {
  [locationManager release], locationManager = nil;
  [adConnection release], adConnection = nil;
  [adData release], adData = nil;
  [imageConnection release], imageConnection = nil;
  [imageData release], imageData = nil;
  [adView release], adView = nil;
  [webBrowserController release], webBrowserController = nil;
  [super dealloc];
}

#pragma mark NSURLConnection delegate methods.

- (void)connection:(NSURLConnection *)conn didReceiveResponse:(NSURLResponse *)response {
  if (conn == adConnection) {
    [adData setLength:0];
  }
  else if (conn == imageConnection) {
    [imageData setLength:0];
  }
}

- (void)connection:(NSURLConnection *)conn didFailWithError:(NSError *)error {
  if (conn == adConnection) {
    [adWhirlView adapter:self didFailAd:[AdWhirlError errorWithCode:AdWhirlCustomAdConnectionError
                                                        description:@"Error connecting to MdotM ad server"
                                                    underlyingError:error]];
    requesting = NO;
  } else if (conn == imageConnection) {
    [adWhirlView adapter:self didFailAd:[AdWhirlError errorWithCode:AdWhirlCustomAdConnectionError
                                                        description:@"Error connecting to MdotM to fetch image"
                                                    underlyingError:error]];
    requesting = NO;
  }
}

- (void)connectionDidFinishLoading:(NSURLConnection *)conn {
  if (conn == adConnection) {
    NSError *error = nil;
    if (![self parseAdData:adData error:&error]) {
      [adWhirlView adapter:self didFailAd:error];
      requesting = NO;
      return;
    }
  }
  else if (conn == imageConnection) {
    UIImage *image = [[UIImage alloc] initWithData:imageData];
    if (image == nil) {
      [adWhirlView adapter:self
                 didFailAd:[AdWhirlError errorWithCode:AdWhirlCustomAdImageError
                                           description:@"Cannot initialize MdotM ad image from data"]];
      requesting = NO;
      return;
    }
    adView.image = image;
    [adView setNeedsDisplay];
    [image release];
    requesting = NO;
    [adWhirlView adapter:self didReceiveAdView:self.adView];
  }
}

- (void)connection:(NSURLConnection *)conn didReceiveData:(NSData *)data {
  if (conn == adConnection) {
    [adData appendData:data];
  }
  else if (conn == imageConnection) {
    [imageData appendData:data];
  }
}

#pragma mark AdWhirlCustomAdViewDelegate methods

- (void)adTapped:(AdWhirlCustomAdView *)ad {
  if (ad != adView) return;
  if (ad.clickMetricsURL != nil) {
    NSURLRequest *metRequest = [NSURLRequest requestWithURL:ad.clickMetricsURL];
    [NSURLConnection connectionWithRequest:metRequest
         delegate:nil]; // fire and forget
  }
  if (ad.redirectURL == nil) {
    AWLogError(@"MdotM ad redirect URL is nil");
    return;
  }
  switch (ad.launchType) {
  case AWCustomAdLaunchTypeSafari:
    [[UIApplication sharedApplication] openURL:ad.redirectURL];
    break;
  case AWCustomAdLaunchTypeCanvas:
    if (self.webBrowserController == nil) {
      AdWhirlWebBrowserController *ctrlr = [[AdWhirlWebBrowserController alloc] init];
      self.webBrowserController = ctrlr;
      [ctrlr release];
    }
    webBrowserController.delegate = self;
    [webBrowserController presentWithController:[adWhirlDelegate viewControllerForPresentingModalView] 
                                     transition:ad.animType];
    [self helperNotifyDelegateOfFullScreenModal];
    [webBrowserController loadURL:ad.redirectURL];
    break;
  default:
    AWLogError(@"MdotM ad: Unsupported launch type %d", ad.launchType);
    break;
  }
}

#pragma mark AdWhirlWebBrowserControllerDelegate methods

- (void)webBrowserClosed:(AdWhirlWebBrowserController *)controller {
  if (controller != webBrowserController) return;
  self.webBrowserController = nil; // don't keep around to save memory
  [self helperNotifyDelegateOfFullScreenModalDismissal];
}

@end
