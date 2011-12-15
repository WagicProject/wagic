/*

 AdWhirlView.m

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

#import "AdWhirlView.h"
#import "AdWhirlView+.h"
#import "AdWhirlConfigStore.h"
#import "AdWhirlAdNetworkConfig.h"
#import "CJSONDeserializer.h"
#import "AdWhirlLog.h"
#import "AdWhirlAdNetworkAdapter.h"
#import "AdWhirlError.h"
#import "AdWhirlConfigStore.h"
#import "AWNetworkReachabilityWrapper.h"

#define kAdWhirlViewAdSubViewTag   1000


NSInteger adNetworkPriorityComparer(id a, id b, void *ctx) {
  AdWhirlAdNetworkConfig *acfg = a, *bcfg = b;
  if(acfg.priority < bcfg.priority)
    return NSOrderedAscending;
  else if(acfg.priority > bcfg.priority)
    return NSOrderedDescending;
  else
    return NSOrderedSame;
}


@implementation AdWhirlView

#pragma mark Properties getters/setters

@synthesize delegate;
@synthesize config;
@synthesize prioritizedAdNetCfgs;
@synthesize currAdapter;
@synthesize lastAdapter;
@synthesize lastRequestTime;
@synthesize refreshTimer;
@synthesize lastError;
@synthesize showingModalView;
@synthesize configStore;
@synthesize rollOverReachability;
@synthesize testDarts;

- (void)setDelegate:(id <AdWhirlDelegate>)theDelegate {
  [self willChangeValueForKey:@"delegate"];
  delegate = theDelegate;
  if (self.currAdapter) {
    self.currAdapter.adWhirlDelegate = theDelegate;
  }
  if (self.lastAdapter) {
    self.lastAdapter.adWhirlDelegate = theDelegate;
  }
  [self didChangeValueForKey:@"delegate"];
}


#pragma mark Life cycle methods

+ (AdWhirlView *)requestAdWhirlViewWithDelegate:(id<AdWhirlDelegate>)delegate {
  if (![delegate respondsToSelector:
                        @selector(viewControllerForPresentingModalView)]) {
    [NSException raise:@"AdWhirlIncompleteDelegateException"
                format:@"AdWhirlDelegate must implement"
                       @" viewControllerForPresentingModalView"];
  }
  AdWhirlView *adView
    = [[[AdWhirlView alloc] initWithDelegate:delegate] autorelease];
  [adView startGetConfig];  // errors are communicated via delegate
  return adView;
}

- (id)initWithDelegate:(id<AdWhirlDelegate>)d {
  self = [super initWithFrame:kAdWhirlViewDefaultFrame];
  if (self != nil) {
    delegate = d;
    self.backgroundColor = [UIColor clearColor];
    // to prevent ugly artifacts if ad network banners are bigger than the
    // default frame
    self.clipsToBounds = YES;
    showingModalView = NO;
    appInactive = NO;

    // default config store. Can be overridden for testing
    self.configStore = [AdWhirlConfigStore sharedStore];

    // get notified of app activity
    NSNotificationCenter *notifCenter = [NSNotificationCenter defaultCenter];
    [notifCenter addObserver:self
                    selector:@selector(resignActive:)
                        name:UIApplicationWillResignActiveNotification
                      object:nil];
    [notifCenter addObserver:self
                    selector:@selector(becomeActive:)
                        name:UIApplicationDidBecomeActiveNotification
                      object:nil];

    // remember pending ad requests, so we don't make more than one
    // request per ad network at a time
    pendingAdapters = [[NSMutableDictionary alloc] initWithCapacity:30];
  }
  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  [rollOverReachability setDelegate:nil];
  [rollOverReachability release], rollOverReachability = nil;
  delegate = nil;
  [config removeDelegate:self];
  [config release], config = nil;
  [prioritizedAdNetCfgs release], prioritizedAdNetCfgs = nil;
  totalPercent = 0.0;
  requesting = NO;
  currAdapter.adWhirlDelegate = nil, currAdapter.adWhirlView = nil;
  [currAdapter release], currAdapter = nil;
  lastAdapter.adWhirlDelegate = nil, lastAdapter.adWhirlView = nil;
  [lastAdapter release], lastAdapter = nil;
  [lastRequestTime release], lastRequestTime = nil;
  [pendingAdapters release], pendingAdapters = nil;
  if (refreshTimer != nil) {
    [refreshTimer invalidate];
    [refreshTimer release], refreshTimer = nil;
  }
  [lastError release], lastError = nil;

  [super dealloc];
}


#pragma mark Config and setup methods

static id<AdWhirlDelegate> classAdWhirlDelegateForConfig = nil;

+ (void)startPreFetchingConfigurationDataWithDelegate:
                                            (id<AdWhirlDelegate>)delegate {
  if (classAdWhirlDelegateForConfig != nil) {
    AWLogWarn(@"Called startPreFetchingConfig when another fetch is"
              @" in progress");
    return;
  }
  classAdWhirlDelegateForConfig = delegate;
  [[AdWhirlConfigStore sharedStore] getConfig:[delegate adWhirlApplicationKey]
                                     delegate:(id<AdWhirlConfigDelegate>)self];
}

+ (void)updateAdWhirlConfigWithDelegate:(id<AdWhirlDelegate>)delegate {
  if (classAdWhirlDelegateForConfig != nil) {
    AWLogWarn(@"Called updateConfig when another fetch is in progress");
    return;
  }
  classAdWhirlDelegateForConfig = delegate;
  [[AdWhirlConfigStore sharedStore]
                              fetchConfig:[delegate adWhirlApplicationKey]
                                 delegate:(id<AdWhirlConfigDelegate>)self];
}

- (void)startGetConfig {
  // Invalidate ad refresh timer as it may change with the new config
  if (self.refreshTimer) {
    [self.refreshTimer invalidate];
    self.refreshTimer = nil;
  }

  configFetchAttempts = 0;
  AdWhirlConfig *cfg = [configStore getConfig:[delegate adWhirlApplicationKey]
                                     delegate:(id<AdWhirlConfigDelegate>)self];
  self.config = cfg;
}

- (void)attemptFetchConfig {
  AdWhirlConfig *cfg = [configStore
                                  fetchConfig:[delegate adWhirlApplicationKey]
                                     delegate:(id<AdWhirlConfigDelegate>)self];
  if (cfg != nil) {
    self.config = cfg;
  }
}

- (void)updateAdWhirlConfig {
  // Invalidate ad refresh timer as it may change with the new config
  if (self.refreshTimer) {
    [self.refreshTimer invalidate];
    self.refreshTimer = nil;
  }

  // Request new config
  AWLogDebug(@"======== Updating config ========");
  configFetchAttempts = 0;
  [self attemptFetchConfig];
}

#pragma mark Ads management private methods

- (void)buildPrioritizedAdNetCfgsAndMakeRequest {
  NSMutableArray *freshNetCfgs = [[NSMutableArray alloc] init];
  for (AdWhirlAdNetworkConfig *cfg in config.adNetworkConfigs) {
    // do not add the ad network in rotation if there's already a stray
    // pending ad request to this ad network (due to network outage or plain
    // slow request)
    NSNumber *netKey = [NSNumber numberWithInt:(int)cfg.networkType];
    if ([pendingAdapters objectForKey:netKey] == nil) {
      [freshNetCfgs addObject:cfg];
    }
    else {
      AWLogDebug(@"Already has pending ad request for network type %d,"
                 @" not adding ad network config %@",
                 cfg.networkType, cfg);
    }
  }
  [freshNetCfgs sortUsingFunction:adNetworkPriorityComparer context:nil];
  totalPercent = 0.0;
  for (AdWhirlAdNetworkConfig *cfg in freshNetCfgs) {
    totalPercent += cfg.trafficPercentage;
  }
  self.prioritizedAdNetCfgs = freshNetCfgs;
  [freshNetCfgs release];

  [self makeAdRequest:YES];
}

static BOOL randSeeded = NO;
- (double)nextDart {
  if (testDarts != nil) {
    if (testDartIndex >= [testDarts count]) {
      testDartIndex = 0;
    }
    NSNumber *nextDartNum = [testDarts objectAtIndex:testDartIndex];
    double dart = [nextDartNum doubleValue];
    if (dart >= totalPercent) {
      dart = totalPercent - 0.001;
    }
    testDartIndex++;
    return dart;
  }
  else {
    if (!randSeeded) {
      srandom(CFAbsoluteTimeGetCurrent());
      randSeeded = YES;
    }
    return ((double)(random()-1)/RAND_MAX) * totalPercent;
  }
}

- (AdWhirlAdNetworkConfig *)nextNetworkCfgByPercent {
  if ([prioritizedAdNetCfgs count] == 0) {
    return nil;
  }

  double dart = [self nextDart];

  double tempTotal = 0.0;

  AdWhirlAdNetworkConfig *result = nil;
  for (AdWhirlAdNetworkConfig *network in prioritizedAdNetCfgs) {
    result = network; // make sure there is always a network chosen
    tempTotal += network.trafficPercentage;
    if (dart < tempTotal) {
      // this is the one to use.
      break;
    }
  }

  AWLogDebug(@">>>> By Percent chosen %@ (%@), dart %lf in %lf",
        result.nid, result.networkName, dart, totalPercent);
  return result;
}

- (AdWhirlAdNetworkConfig *)nextNetworkCfgByPriority {
  if ([prioritizedAdNetCfgs count] == 0) {
    return nil;
  }
  AdWhirlAdNetworkConfig *result = [prioritizedAdNetCfgs objectAtIndex:0];
  AWLogDebug(@">>>> By Priority chosen %@ (%@)",
             result.nid, result.networkName);
  return result;
}

- (void)makeAdRequest:(BOOL)isFirstRequest {
  if ([prioritizedAdNetCfgs count] == 0) {
    // ran out of ad networks
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestNoMoreAdNetworks
                            description:@"No more ad networks to roll over"];
    return;
  }

  if (showingModalView) {
    AWLogDebug(@"Modal view is active, not going to request another ad");
    return;
  }
  [self.rollOverReachability setDelegate:nil];
  self.rollOverReachability = nil;  // stop any roll over reachability checks

  if (requesting) {
    // it is OK to request a new one while another one is in progress
    // the adapter callbacks from the old adapter will be ignored.
    // User-initiated request ad will be blocked in requestFreshAd.
    AWLogDebug(@"Already requesting ad, will request a new one.");
  }
  requesting = YES;

  AdWhirlAdNetworkConfig *nextAdNetCfg = nil;

  if (isFirstRequest && totalPercent > 0.0) {
    nextAdNetCfg = [self nextNetworkCfgByPercent];
  }
  else {
    nextAdNetCfg = [self nextNetworkCfgByPriority];
  }
  if (nextAdNetCfg == nil) {
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestNoMoreAdNetworks
                            description:@"No more ad networks to request"];
    return;
  }

  AdWhirlAdNetworkAdapter *adapter =
    [[nextAdNetCfg.adapterClass alloc] initWithAdWhirlDelegate:delegate
                                                           view:self
                                                         config:config
                                                  networkConfig:nextAdNetCfg];
  // keep the last adapter around to catch stale ad network delegate calls
  // during transitions
  self.lastAdapter = self.currAdapter;
  self.currAdapter = adapter;
  [adapter release];

  // take nextAdNetCfg out so we don't request again when we roll over
  [prioritizedAdNetCfgs removeObject:nextAdNetCfg];

  if (lastRequestTime) {
    [lastRequestTime release];
  }
  lastRequestTime = [[NSDate date] retain];

  // remember this pending request so we do not request again when we make
  // new ad requests
  NSNumber *netTypeKey = [NSNumber numberWithInt:(int)nextAdNetCfg.networkType];
  [pendingAdapters setObject:currAdapter forKey:netTypeKey];

  // If last adapter is of the same network type, make the last adapter stop
  // being an ad network view delegate to prevent the last adapter from calling
  // back to this AdWhirlView during the transition and afterwards.
  // We should not do this for all adapters, because if the last adapter is
  // still in progress, we need to know about it in the adapter callbacks.
  // That the last adapter is the same type as the new adapter is possible only
  // if the last ad request finished, i.e. called back to its adapters. There
  // are cases, e.g. iAd, when the ad network may call back multiple times,
  // because of internal refreshes.
  if (self.lastAdapter.networkConfig.networkType ==
                                  self.currAdapter.networkConfig.networkType) {
    [self.lastAdapter stopBeingDelegate];
  }

  [currAdapter getAd];
}

- (BOOL)canRefresh {
  return !(ignoreNewAdRequests
           || ignoreAutoRefreshTimer
           || appInactive
           || showingModalView);
}

- (void)timerRequestFreshAd {
  if (![self canRefresh]) {
    AWLogDebug(@"Not going to refresh due to flags, app not active or modal");
    return;
  }
  if (lastRequestTime != nil) {
    NSTimeInterval sinceLast = -[lastRequestTime timeIntervalSinceNow];
    if (sinceLast <= kAWMinimumTimeBetweenFreshAdRequests) {
      AWLogDebug(@"Ad refresh timer fired too soon after last ad request,"
                 @" ignoring");
      return;
    }
  }
  AWLogDebug(@"======== Refreshing ad due to timer ========");
  [self buildPrioritizedAdNetCfgsAndMakeRequest];
}

#pragma mark Ads management public methods

- (void)requestFreshAd {
  // only make request in main thread
  if (![NSThread isMainThread]) {
    [self performSelectorOnMainThread:@selector(requestFreshAd)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }
  if (ignoreNewAdRequests) {
    // don't request new ad
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestIgnoredError
                            description:@"ignoreNewAdRequests flag set"];
    return;
  }
  if (requesting) {
    // don't request if there's a request outstanding
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestInProgressError
                            description:@"Ad request already in progress"];
    return;
  }
  if (showingModalView) {
    // don't request if there's a modal view active
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestModalActiveError
                            description:@"Modal view active"];
    return;
  }
  if (!config) {
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestNoConfigError
                            description:@"No ad configuration"];
    return;
  }
  if (lastRequestTime != nil) {
    NSTimeInterval sinceLast = -[lastRequestTime timeIntervalSinceNow];
    if (sinceLast <= kAWMinimumTimeBetweenFreshAdRequests) {
      NSString *desc
        = [NSString stringWithFormat:
           @"Requesting fresh ad too soon! It has been only %lfs. Minimum %lfs",
           sinceLast, kAWMinimumTimeBetweenFreshAdRequests];
      [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestTooSoonError
                              description:desc];
      return;
    }
  }
  [self buildPrioritizedAdNetCfgsAndMakeRequest];
}

- (void)rollOver {
  if (ignoreNewAdRequests) {
    return;
  }
  // only make request in main thread
  if (![NSThread isMainThread]) {
    [self performSelectorOnMainThread:@selector(rollOver)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }
  [self makeAdRequest:NO];
}

- (BOOL)adExists {
  UIView *currAdView = [self viewWithTag:kAdWhirlViewAdSubViewTag];
  return currAdView != nil;
}

- (NSString *)mostRecentNetworkName {
  if (currAdapter == nil) return nil;
  return currAdapter.networkConfig.networkName;
}

- (void)ignoreAutoRefreshTimer {
  ignoreAutoRefreshTimer = YES;
}

- (void)doNotIgnoreAutoRefreshTimer {
  ignoreAutoRefreshTimer = NO;
}

- (BOOL)isIgnoringAutoRefreshTimer {
  return ignoreAutoRefreshTimer;
}

- (void)ignoreNewAdRequests {
  ignoreNewAdRequests = YES;
}

- (void)doNotIgnoreNewAdRequests {
  ignoreNewAdRequests = NO;
}

- (BOOL)isIgnoringNewAdRequests {
  return ignoreNewAdRequests;
}


#pragma mark Stats reporting methods

- (void)metricPing:(NSURL *)endPointBaseURL
               nid:(NSString *)nid
           netType:(AdWhirlAdNetworkType)type {
  // use config.appKey not from [delegate adWhirlApplicationKey] as delegate
  // can be niled out at this point. Attempt at Issue #42 .
  NSString *query
    = [NSString stringWithFormat:
       @"?appid=%@&nid=%@&type=%d&country_code=%@&appver=%d&client=1",
       config.appKey,
       nid,
       type,
       [[NSLocale currentLocale] localeIdentifier],
       kAdWhirlAppVer];
  NSURL *metURL = [NSURL URLWithString:query
                         relativeToURL:endPointBaseURL];
  AWLogDebug(@"Sending metric ping to %@", metURL);
  NSURLRequest *metRequest = [NSURLRequest requestWithURL:metURL];
  [NSURLConnection connectionWithRequest:metRequest
                                delegate:nil]; // fire and forget
}

- (void)reportExImpression:(NSString *)nid netType:(AdWhirlAdNetworkType)type {
  NSURL *baseURL = nil;
  if ([delegate respondsToSelector:@selector(adWhirlImpMetricURL)]) {
    baseURL = [delegate adWhirlImpMetricURL];
  }
  if (baseURL == nil) {
    baseURL = [NSURL URLWithString:kAdWhirlDefaultImpMetricURL];
  }
  [self metricPing:baseURL nid:nid netType:type];
}

- (void)reportExClick:(NSString *)nid netType:(AdWhirlAdNetworkType)type {
  NSURL *baseURL = nil;
  if ([delegate respondsToSelector:@selector(adWhirlClickMetricURL)]) {
    baseURL = [delegate adWhirlClickMetricURL];
  }
  if (baseURL == nil) {
    baseURL = [NSURL URLWithString:kAdWhirlDefaultClickMetricURL];
  }
  [self metricPing:baseURL nid:nid netType:type];
}


#pragma mark UI methods

- (CGSize)actualAdSize {
  if (currAdapter == nil || currAdapter.adNetworkView == nil)
    return kAdWhirlViewDefaultSize;
  return currAdapter.adNetworkView.frame.size;
}

- (void)rotateToOrientation:(UIInterfaceOrientation)orientation {
  if (currAdapter == nil) return;
  [currAdapter rotateToOrientation:orientation];
}

- (void)transitionToView:(UIView *)view {
  UIView *currAdView = [self viewWithTag:kAdWhirlViewAdSubViewTag];
  if (view == currAdView) {
    AWLogDebug(@"ignoring ad transition to itself");
    return; // no need to transition to itself
  }
  view.tag = kAdWhirlViewAdSubViewTag;
  if (currAdView) {
    // swap
    currAdView.tag = 0;

    AWBannerAnimationType animType;
    if (config.bannerAnimationType == AWBannerAnimationTypeRandom) {
      if (!randSeeded) {
        srandom(CFAbsoluteTimeGetCurrent());
      }
      // range is 1 to 7, inclusive
      animType = (random() % 7) + 1;
      AWLogDebug(@"Animation type chosen by random is %d", animType);
    }
    else {
      animType = config.bannerAnimationType;
    }
    if (![currAdapter isBannerAnimationOK:animType]) {
      animType = AWBannerAnimationTypeNone;
    }

    if (animType == AWBannerAnimationTypeNone) {
      [currAdView removeFromSuperview];
      [self addSubview:view];
      if ([delegate respondsToSelector:
                                    @selector(adWhirlDidAnimateToNewAdIn:)]) {
        // no animation, callback right away
        [(NSObject *)delegate
              performSelectorOnMainThread:@selector(adWhirlDidAnimateToNewAdIn:)
                               withObject:self
                            waitUntilDone:NO];
      }
    }
    else {
      switch (animType) {
        case AWBannerAnimationTypeSlideFromLeft:
        {
          CGRect f = view.frame;
          f.origin.x = -f.size.width;
          view.frame = f;
          [self addSubview:view];
          break;
        }
        case AWBannerAnimationTypeSlideFromRight:
        {
          CGRect f = view.frame;
          f.origin.x = self.frame.size.width;
          view.frame = f;
          [self addSubview:view];
          break;
        }
        case AWBannerAnimationTypeFadeIn:
          view.alpha = 0;
          [self addSubview:view];
          break;
        default:
          // no setup required for other animation types
          break;
      }

      [currAdView retain]; // will be released when animation is done
      AWLogDebug(@"Beginning AdWhirlAdTransition animation"
                 @" currAdView %x incoming %x", currAdView, view);
      [UIView beginAnimations:@"AdWhirlAdTransition" context:currAdView];
      [UIView setAnimationDelegate:self];
      [UIView setAnimationDidStopSelector:
            @selector(newAdAnimationDidStopWithAnimationID:finished:context:)];
      [UIView setAnimationBeginsFromCurrentState:YES];
      [UIView setAnimationDuration:1.0];
      // cache has to set to NO because of VideoEgg
      switch (animType) {
        case AWBannerAnimationTypeFlipFromLeft:
          [self addSubview:view];
          [currAdView removeFromSuperview];
          [UIView setAnimationTransition:UIViewAnimationTransitionFlipFromLeft
                                 forView:self
                                   cache:NO];
          break;
        case AWBannerAnimationTypeFlipFromRight:
          [self addSubview:view];
          [currAdView removeFromSuperview];
          [UIView setAnimationTransition:UIViewAnimationTransitionFlipFromRight
                                 forView:self
                                   cache:NO];
          break;
        case AWBannerAnimationTypeCurlUp:
          [self addSubview:view];
          [currAdView removeFromSuperview];
          [UIView setAnimationTransition:UIViewAnimationTransitionCurlUp
                                 forView:self
                                   cache:NO];
          break;
        case AWBannerAnimationTypeCurlDown:
          [self addSubview:view];
          [currAdView removeFromSuperview];
          [UIView setAnimationTransition:UIViewAnimationTransitionCurlDown
                                 forView:self
                                   cache:NO];
          break;
        case AWBannerAnimationTypeSlideFromLeft:
        case AWBannerAnimationTypeSlideFromRight:
        {
          CGRect f = view.frame;
          f.origin.x = 0;
          view.frame = f;
          break;
        }
        case AWBannerAnimationTypeFadeIn:
          view.alpha = 1.0;
          break;
        default:
          [self addSubview:view];
          AWLogWarn(@"Unrecognized Animation type: %d", animType);
          break;
      }
      [UIView commitAnimations];
    }
  }
  else {  // currAdView
    // new
    [self addSubview:view];
    if ([delegate respondsToSelector:@selector(adWhirlDidAnimateToNewAdIn:)]) {
      // no animation, callback right away
      [(NSObject *)delegate
              performSelectorOnMainThread:@selector(adWhirlDidAnimateToNewAdIn:)
                               withObject:self
                            waitUntilDone:NO];
    }
  }
}

- (void)replaceBannerViewWith:(UIView*)bannerView {
  [self transitionToView:bannerView];
}

// Called at the end of the new ad animation; we use this opportunity to do
// memory management cleanup. See the comment in adDidLoad:.
- (void)newAdAnimationDidStopWithAnimationID:(NSString *)animationID
                                    finished:(BOOL)finished
                                     context:(void *)context
{
  AWLogDebug(@"animation %@ finished %@ context %x",
             animationID, finished? @"YES":@"NO", context);
  UIView *adViewToRemove = (UIView *)context;
  [adViewToRemove removeFromSuperview];
  [adViewToRemove release]; // was retained before beginAnimations
  lastAdapter.adWhirlDelegate = nil, lastAdapter.adWhirlView = nil;
  self.lastAdapter = nil;
  if ([delegate respondsToSelector:@selector(adWhirlDidAnimateToNewAdIn:)]) {
    [delegate adWhirlDidAnimateToNewAdIn:self];
  }
}


#pragma mark UIView touch methods

- (BOOL)_isEventATouch30:(UIEvent *)event {
  if ([event respondsToSelector:@selector(type)]) {
    return event.type == UIEventTypeTouches;
  }
  return YES; // UIEvent in 2.2.1 has no type property, so assume yes.
}

- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *)event {
  BOOL itsInside = [super pointInside:point withEvent:event];
  if (itsInside && currAdapter != nil && lastNotifyAdapter != currAdapter
      && [self _isEventATouch30:event]
      && [currAdapter shouldSendExMetric]) {
    lastNotifyAdapter = currAdapter;
    [self reportExClick:currAdapter.networkConfig.nid
                netType:currAdapter.networkConfig.networkType];
  }
  return itsInside;
}


#pragma mark UIView methods

- (void)willMoveToSuperview:(UIView *)newSuperview {
  if (newSuperview == nil) {
    [refreshTimer invalidate];
    self.refreshTimer = nil;
  }
}


#pragma mark Adapter callbacks

// Chores that are common to all adapter callbacks
- (void)adRequestReturnsForAdapter:(AdWhirlAdNetworkAdapter *)adapter {
  // no longer pending. Need to retain and autorelease the adapter
  // since the adapter may not be retained anywhere else other than the pending
  // dict
  NSNumber *netTypeKey
    = [NSNumber numberWithInt:(int)adapter.networkConfig.networkType];
  AdWhirlAdNetworkAdapter *pendingAdapter
    = [pendingAdapters objectForKey:netTypeKey];
  if (pendingAdapter != nil) {
    if (pendingAdapter != adapter) {
      // Possible if the ad refreshes itself and sends callbacks doing so, while
      // a new ad of the same network is pending (e.g. iAd)
      AWLogError(@"Stored pending adapter %@ for network type %@ is different"
                 @" from the one sending the adapter callback %@",
                 pendingAdapter,
                 netTypeKey,
                 adapter);
    }
    [[pendingAdapter retain] autorelease];
    [pendingAdapters removeObjectForKey:netTypeKey];
  }
}

- (void)adapter:(AdWhirlAdNetworkAdapter *)adapter
          didReceiveAdView:(UIView *)view {
  [self adRequestReturnsForAdapter:adapter];
  if (adapter != currAdapter) {
    AWLogDebug(@"Received didReceiveAdView from a stale adapter %@", adapter);
    return;
  }
  AWLogDebug(@"Received ad from adapter (nid %@)", adapter.networkConfig.nid);

  // UIView operations should be performed on main thread
  [self performSelectorOnMainThread:@selector(transitionToView:)
                         withObject:view
                      waitUntilDone:NO];
  requesting = NO;

  // report impression and notify delegate
  if ([adapter shouldSendExMetric]) {
    [self reportExImpression:adapter.networkConfig.nid
                     netType:adapter.networkConfig.networkType];
  }
  if ([delegate respondsToSelector:@selector(adWhirlDidReceiveAd:)]) {
    [delegate adWhirlDidReceiveAd:self];
  }
}

- (void)adapter:(AdWhirlAdNetworkAdapter *)adapter didFailAd:(NSError *)error {
  [self adRequestReturnsForAdapter:adapter];
  if (adapter != currAdapter) {
    AWLogDebug(@"Received didFailAd from a stale adapter %@: %@",
               adapter, error);
    return;
  }
  AWLogDebug(@"Failed to receive ad from adapter (nid %@): %@",
             adapter.networkConfig.nid, error);
  requesting = NO;

  if ([prioritizedAdNetCfgs count] == 0) {
    // we have run out of networks to try and need to error out.
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestNoMoreAdNetworks
                            description:@"No more ad networks to roll over"];
    return;
  }

  // try to roll over, but before we do, check to see if the failure is because
  // network has gotten unreachable. If so, don't roll over. Use www.google.com
  // as test, assuming www.google.com itself is always up if there's network.
  self.rollOverReachability
    = [AWNetworkReachabilityWrapper reachabilityWithHostname:@"www.google.com"
                                            callbackDelegate:self];
  if (self.rollOverReachability == nil) {
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestNoNetworkError
                            description:@"Failed network reachability test"];
    return;
  }
  if (![self.rollOverReachability scheduleInCurrentRunLoop]) {
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestNoNetworkError
                            description:@"Failed network reachability test"];
    return;
  }
}

- (void)adapterDidFinishAdRequest:(AdWhirlAdNetworkAdapter *)adapter {
  [self adRequestReturnsForAdapter:adapter];
  if (adapter != currAdapter) {
    AWLogDebug(@"Received adapterDidFinishAdRequest from a stale adapter");
    return;
  }
  // view is supplied via other mechanism (e.g. Generic Notification or Event)
  requesting = NO;

  // report impression. No need to notify delegate because delegate is notified
  // via Generic Notification or event.
  if ([adapter shouldSendExMetric]) {
    [self reportExImpression:adapter.networkConfig.nid
                     netType:adapter.networkConfig.networkType];
  }
}


#pragma mark AWNetworkReachabilityDelegate methods

- (void)reachabilityNotReachable:(AWNetworkReachabilityWrapper *)reach {
  if (reach == self.rollOverReachability) {
    [self.rollOverReachability setDelegate:nil];
    self.rollOverReachability = nil;  // release it and unschedule
    [self notifyDelegateOfErrorWithCode:AdWhirlAdRequestNoNetworkError
                            description:@"No network connection for rollover"];
    return;
  }
  AWLogWarn(@"Unrecognized reachability called not reachable %s:%d",
            __FILE__, __LINE__);
}

- (void)reachabilityBecameReachable:(AWNetworkReachabilityWrapper *)reach {
  if (reach == self.rollOverReachability) {
    // not an error, just need to rollover
    [lastError release], lastError = nil;
    if ([delegate respondsToSelector:
         @selector(adWhirlDidFailToReceiveAd:usingBackup:)]) {
      [delegate adWhirlDidFailToReceiveAd:self usingBackup:YES];
    }
    [self.rollOverReachability setDelegate:nil];
    self.rollOverReachability = nil;   // release it and unschedule
    [self rollOver];
    return;
  }
  AWLogWarn(@"Unrecognized reachability called reachable %s:%d",
            __FILE__, __LINE__);
}


#pragma mark AdWhirlConfigDelegate methods

+ (NSURL *)adWhirlConfigURL {
  if (classAdWhirlDelegateForConfig != nil
      && [classAdWhirlDelegateForConfig respondsToSelector:
                                        @selector(adWhirlConfigURL)]) {
    return [classAdWhirlDelegateForConfig adWhirlConfigURL];
  }
  return nil;
}

+ (void)adWhirlConfigDidReceiveConfig:(AdWhirlConfig *)config {
  AWLogDebug(@"Fetched Ad network config: %@", config);
  if (classAdWhirlDelegateForConfig != nil
      && [classAdWhirlDelegateForConfig respondsToSelector:
                                        @selector(adWhirlDidReceiveConfig:)]) {
    [classAdWhirlDelegateForConfig adWhirlDidReceiveConfig:nil];
  }
  classAdWhirlDelegateForConfig = nil;
}

+ (void)adWhirlConfigDidFail:(AdWhirlConfig *)cfg error:(NSError *)error {
  AWLogError(@"Failed pre-fetching AdWhirl config: %@", error);
  classAdWhirlDelegateForConfig = nil;
}

- (void)adWhirlConfigDidReceiveConfig:(AdWhirlConfig *)cfg {
  if (self.config != cfg) {
    AWLogWarn(@"AdWhirlView: getting adWhirlConfigDidReceiveConfig callback"
              @" from unknown AdWhirlConfig object");
    return;
  }
  AWLogDebug(@"Fetched Ad network config: %@", cfg);
  if ([delegate respondsToSelector:@selector(adWhirlDidReceiveConfig:)]) {
    [delegate adWhirlDidReceiveConfig:self];
  }
  if (cfg.adsAreOff) {
    if ([delegate respondsToSelector:
                        @selector(adWhirlReceivedNotificationAdsAreOff:)]) {
      // to prevent self being freed before this returns, in case the
      // delegate decides to release this
      [self retain];
      [delegate adWhirlReceivedNotificationAdsAreOff:self];
      [self autorelease];
    }
    return;
  }

  // Perform ad network data structure build and request in main thread
  // to avoid contention
  [self performSelectorOnMainThread:
                            @selector(buildPrioritizedAdNetCfgsAndMakeRequest)
                         withObject:nil
                      waitUntilDone:NO];

  // Setup recurring timer for ad refreshes, if required
  if (config.refreshInterval > kAWMinimumTimeBetweenFreshAdRequests) {
    self.refreshTimer
      = [NSTimer scheduledTimerWithTimeInterval:config.refreshInterval
                                         target:self
                                       selector:@selector(timerRequestFreshAd)
                                       userInfo:nil
                                        repeats:YES];
  }
}

- (void)adWhirlConfigDidFail:(AdWhirlConfig *)cfg error:(NSError *)error {
  if (self.config != nil && self.config != cfg) {
    // self.config could be nil if this is called before init is finished
    AWLogWarn(@"AdWhirlView: getting adWhirlConfigDidFail callback from unknown"
              @" AdWhirlConfig object");
    return;
  }
  configFetchAttempts++;
  if (configFetchAttempts < 3) {
    // schedule in run loop to avoid recursive calls to this function
    [self performSelectorOnMainThread:@selector(attemptFetchConfig)
                           withObject:self
                        waitUntilDone:NO];
  }
  else {
    AWLogError(@"Failed fetching AdWhirl config: %@", error);
    [self notifyDelegateOfError:error];
  }
}

- (NSURL *)adWhirlConfigURL {
  if ([delegate respondsToSelector:@selector(adWhirlConfigURL)]) {
    return [delegate adWhirlConfigURL];
  }
  return nil;
}


#pragma mark Active status notification callbacks

- (void)resignActive:(NSNotification *)notification {
  AWLogDebug(@"App become inactive, AdWhirlView will stop requesting ads");
  appInactive = YES;
}

- (void)becomeActive:(NSNotification *)notification {
  AWLogDebug(@"App become active, AdWhirlView will resume requesting ads");
  appInactive = NO;
}


#pragma mark AdWhirlDelegate helper methods

- (void)notifyDelegateOfErrorWithCode:(NSInteger)errorCode
                          description:(NSString *)desc {
  NSError *error = [[AdWhirlError alloc] initWithCode:errorCode
                                          description:desc];
  [self notifyDelegateOfError:error];
  [error release];
}

- (void)notifyDelegateOfError:(NSError *)error {
  [error retain];
  [lastError release];
  lastError = error;
  if ([delegate respondsToSelector:
                          @selector(adWhirlDidFailToReceiveAd:usingBackup:)]) {
    // to prevent self being freed before this returns, in case the
    // delegate decides to release this
    [self retain];
    [delegate adWhirlDidFailToReceiveAd:self usingBackup:NO];
    [self autorelease];
  }
}

@end
