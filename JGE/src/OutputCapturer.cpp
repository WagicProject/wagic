#include "../include/OutputCapturer.h"

std::ostringstream OutputCapturer::stream;
boost::mutex mMutex;
