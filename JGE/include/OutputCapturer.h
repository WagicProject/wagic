#ifndef OUTPUTCAPTURER_H
#define OUTPUTCAPTURER_H

#if defined(QT_CONFIG)
#include <Qt>
#include <string>
#include <sstream>

class OutputCapturer
{
private:
    static std::ostringstream stream;
public:
    static void add(const std::string& s) {
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
