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
    static void add(const std::string& s) {
        boost::mutex::scoped_lock lock(mMutex);
        stream << s << "\n";
    }
    static void debugAndClear() {
        stream.flush();
        qDebug("%s", stream.str().c_str());
        stream.str("");
    }
    static void clear() {
        stream.str("");
    }
};
#endif

#endif // OUTPUTCAPTURER_H
