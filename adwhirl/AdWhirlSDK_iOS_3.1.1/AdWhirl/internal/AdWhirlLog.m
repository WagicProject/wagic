/*

 AdWhirlLog.m

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

#import "AdWhirlLog.h"

static AWLogLevel g_AWLogLevel = AWLogLevelInfo;

void AWLogSetLogLevel(AWLogLevel level) {
  g_AWLogLevel = level;
}

void _AWLogCrit(NSString *format, ...) {
  if (g_AWLogLevel < AWLogLevelCrit) return;
  va_list ap;
  va_start(ap, format);
  NSLogv(format, ap);
  va_end(ap);
}

void _AWLogError(NSString *format, ...) {
  if (g_AWLogLevel < AWLogLevelError) return;
  va_list ap;
  va_start(ap, format);
  NSLogv(format, ap);
  va_end(ap);
}

void _AWLogWarn(NSString *format, ...) {
  if (g_AWLogLevel < AWLogLevelWarn) return;
  va_list ap;
  va_start(ap, format);
  NSLogv(format, ap);
  va_end(ap);
}

void _AWLogInfo(NSString *format, ...) {
  if (g_AWLogLevel < AWLogLevelInfo) return;
  va_list ap;
  va_start(ap, format);
  NSLogv(format, ap);
  va_end(ap);
}

void _AWLogDebug(NSString *format, ...) {
  if (g_AWLogLevel < AWLogLevelDebug) return;
  va_list ap;
  va_start(ap, format);
  NSLogv(format, ap);
  va_end(ap);
}
