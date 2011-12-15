/*

 AdWhirlLog.h

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

#import <Foundation/Foundation.h>

typedef enum {
  AWLogLevelNone  = 0,
  AWLogLevelCrit  = 10,
  AWLogLevelError = 20,
  AWLogLevelWarn  = 30,
  AWLogLevelInfo  = 40,
  AWLogLevelDebug = 50
} AWLogLevel;

void AWLogSetLogLevel(AWLogLevel level);

// The actual function name has an underscore prefix, just so we can
// hijack AWLog* with other functions for testing, by defining
// preprocessor macros
void _AWLogCrit(NSString *format, ...);
void _AWLogError(NSString *format, ...);
void _AWLogWarn(NSString *format, ...);
void _AWLogInfo(NSString *format, ...);
void _AWLogDebug(NSString *format, ...);

#ifndef AWLogCrit
#define AWLogCrit(...) _AWLogCrit(__VA_ARGS__)
#endif

#ifndef AWLogError
#define AWLogError(...) _AWLogError(__VA_ARGS__)
#endif

#ifndef AWLogWarn
#define AWLogWarn(...) _AWLogWarn(__VA_ARGS__)
#endif

#ifndef AWLogInfo
#define AWLogInfo(...) _AWLogInfo(__VA_ARGS__)
#endif

#ifndef AWLogDebug
#define AWLogDebug(...) _AWLogDebug(__VA_ARGS__)
#endif
