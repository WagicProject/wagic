#ifndef OBJECTANALYTICS_H
#define OBJECTANALYTICS_H

#ifdef _DEBUG
#define TRACK_OBJECT_USAGE
#endif

#ifdef TRACK_OBJECT_USAGE

namespace ObjectAnalytics
{
    /*
    ** See ObjectAnalytics.cpp for how to add additional objects to this function's dump
    */
    void DumpStatistics();
}

#define SUPPORT_OBJECT_ANALYTICS(classname) \
template <> unsigned int InstanceCounter<classname>::sHighWaterMark = 0; \
template <> unsigned int InstanceCounter<classname>::sInstanceCount = 0; \
template <> uint64_t InstanceCounter<classname>::sDynamicMemoryAllocated = 0;


/**
**  Helper class for tracking object instances.
**  Usage:
**  class Foo
**  #ifdef TRACK_OBJECT_USAGE
**        : public InstanceCounter<Foo>
**  #endif
** 
**  Additionally, since the implementation uses static counters,
**  somewhere in your class body, you need to put in the following macro:
**  SUPPORT_OBJECT_ANALYTICS(Foo);
*/
template <typename T>
class InstanceCounter
{
public:
    InstanceCounter()
    {
        if (sHighWaterMark < ++sInstanceCount)
            sHighWaterMark = sInstanceCount;
    }

    InstanceCounter(const InstanceCounter&)
    {
        if (sHighWaterMark < ++sInstanceCount)
            sHighWaterMark = sInstanceCount;
    }
    InstanceCounter& operator =(const InstanceCounter&)
    {
    }

    ~InstanceCounter()
    {
        --sInstanceCount;
    }

    static unsigned int GetCurrentObjectCount()
    {
        return sInstanceCount;
    }

    static unsigned int GetMaximumObjectCount()
    {
        return sHighWaterMark;
    }

    static unsigned int GetCurrentByteCount()
    {
        return sizeof(T) * sInstanceCount;
    }

    static unsigned int GetMaximumByteCount()
    {
        return sizeof(T) * sHighWaterMark;
    }

    void IncrementDynamicMemoryUsage(uint64_t inMemoryIncrease)
    {
        sDynamicMemoryAllocated += inMemoryIncrease;
    }

    void DecrementDynamicMemoryUsage(uint64_t inMemoryDecrease)
    {
        sDynamicMemoryAllocated -= inMemoryDecrease;
    }

    static uint64_t GetDynamicAllocationsCount()
    {
        return sDynamicMemoryAllocated;
    }

    static unsigned int sInstanceCount;
    static unsigned int sHighWaterMark;
    static uint64_t sDynamicMemoryAllocated;
};

#else
#define SUPPORT_OBJECT_ANALYTICS(classname)
#endif //TRACK_OBJECT_USAGE

#endif //OBJECTANALYTICS_H
