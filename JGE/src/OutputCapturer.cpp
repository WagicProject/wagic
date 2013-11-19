#include "../include/OutputCapturer.h"

std::ostringstream OutputCapturer::stream;
boost::mutex OutputCapturer::mMutex;

void OutputCapturer::add(const std::string& s)
{
    boost::mutex::scoped_lock lock(mMutex);
    stream << s << "\n";
}

void OutputCapturer::debugAndClear()
{
    stream.flush();
    qDebug("%s", stream.str().c_str());
    stream.str("");
}

void OutputCapturer::clear()
{
    stream.str("");
}
