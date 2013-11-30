#ifndef OUTPUTCAPTURER_H
#define OUTPUTCAPTURER_H

#if defined(QT_CONFIG)
#include <Qt>
#include <string>
#include <sstream>
#include "Threading.h"

class OutputCapturer
{
private:
    static std::ostringstream stream;
    static boost::mutex mMutex;

public:
    static void add(const std::string& s);
    static void debugAndClear();
    static void clear();
};
#endif

#endif // OUTPUTCAPTURER_H
