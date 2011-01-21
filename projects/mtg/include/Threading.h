#ifndef THREADING_H
#define THREADING_H

#if defined (WIN32) || defined (LINUX)

#include <boost/date_time.hpp>
#include <boost/thread.hpp>

#else
#include "pspthreadman.h"

namespace boost
{

    class mutex
    {
    public:
        struct scoped_lock
        {
            scoped_lock(mutex& inMutex) :
                mID(inMutex.mID)
            {
                sceKernelWaitSema(mID, 1, 0);
            }

            ~scoped_lock()
            {
                sceKernelSignalSema(mID, 1);
            }

            int mID;
        };

        mutex()
        {
            mID = sceKernelCreateSema("Unnamed", 0, 1, 1, 0);
        }

        ~mutex()
        {
            sceKernelDeleteSema(mID);
        }

        int mID;
    };

}

#endif

#endif // THREADING_H
