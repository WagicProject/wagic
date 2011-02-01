#ifndef THREADING_H
#define THREADING_H

#if defined (WIN32) || defined (LINUX)

#include <boost/date_time.hpp>
#include <boost/thread.hpp>

#else
#include "pspthreadman.h"

#include "JLogger.h"

namespace boost
{

    class mutex
    {
    public:
        struct scoped_lock
        {
            scoped_lock(mutex& inMutex) : mID(inMutex.mID)
            {
                int result = sceKernelWaitSema(mID, 1, 0);
                if (result < 0)
                {
                    LOG("Semaphore error on lock acquire, mutex id: ");
                    LOG((char*)mID);
                }
            }

            ~scoped_lock()
            {
                int result = sceKernelSignalSema(mID, 1);
                if (result < 0)
                {
                    LOG("Semaphore error on lock release, mutex id: ");
                    LOG((char*)mID);
                }
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