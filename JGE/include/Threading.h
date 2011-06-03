#ifndef THREADING_H
#define THREADING_H

#if !defined(PSP)
#include <boost/date_time.hpp>

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable : 4244)
#endif
#include <boost/thread.hpp>
#ifdef WIN32
#pragma warning (pop)
#endif

#include <boost/thread/mutex.hpp>
#else
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "pspthreadman.h"

#include "../include/DebugRoutines.h"
#include "../include/JLogger.h"

namespace boost
{
    /**
    ** PSP specific variant of a boost mutex & scoped_lock 
    */

    template <class Mutex>
    struct unique_lock
    {
        unique_lock(Mutex& inMutex) : mMutex(&inMutex)
        {
            mMutex->lock();
        }

        ~unique_lock()
        {
            mMutex->unlock();
        }

        Mutex* mMutex;
    };

    class mutex
    {
    public:

        typedef unique_lock<mutex> scoped_lock;

        mutex()
        {
            mID = sceKernelCreateSema("Unnamed", 0, 1, 1, 0);
        }

        ~mutex()
        {
            sceKernelDeleteSema(mID);
        }
        
        void lock()
        {
            int result = sceKernelWaitSema(mID, 1, 0);
            if (result < 0)
            {
                LOG("Semaphore error on lock acquire, mutex id: ");
                LOG((char*)mID);
            }
        }

        void unlock()
        {
            int result = sceKernelSignalSema(mID, 1);
            if (result < 0)
            {
                LOG("Semaphore error on lock release, mutex id: ");
                LOG((char*)mID);
            }
        }

        int mID;

    private:
        mutex(mutex const&);
        mutex& operator=(mutex const&);
    };



    class recursive_mutex
    {
    public:

        typedef unique_lock<recursive_mutex> scoped_lock;

        recursive_mutex() : mRecursionCount(0)
        {
            mID = sceKernelCreateSema("Unnamed", 0, 1, 1, 0);
        }

        ~recursive_mutex()
        {
            sceKernelDeleteSema(mID);
        }

        void lock()
        {
            int currentThreadID = sceKernelGetThreadId();

            if (!try_recursive_lock(currentThreadID))
            {
                int result = sceKernelWaitSema(mID, 1, 0);
                if (result < 0)
                {
                    LOG("Semaphore error on lock acquire, mutex id: ");
                    LOG((char*)mID);
                }
                else
                {
                    mThreadID = currentThreadID;
                    mRecursionCount = 1;
                }
            }
        }

        void unlock()
        {

            if(!--mRecursionCount)
            {
                mThreadID = 0;

                int result = sceKernelSignalSema(mID, 1);
                if (result < 0)
                {
                    LOG("Semaphore error on lock release, mutex id: ");
                    LOG((char*)mID);
                }
            }

        }
        
        int mID;
        int mThreadID;
        volatile int mRecursionCount;

    private:
        recursive_mutex(recursive_mutex const&);
        recursive_mutex& operator=(recursive_mutex const&);

        bool try_recursive_lock(int inThreadID)
        {
            if (mThreadID == inThreadID)
            {
                ++mRecursionCount;
                return true;
            }
            return false;
        }
    };


    /**
    ** Emulating boost::thread configuration glue, with some shortcuts 
    ** This detail namespace is a distillation of boost's thread.hpp, thread_data.hpp.
    */
    namespace detail
    {
        struct thread_data_base
        {
            thread_data_base()
            {
            }

            virtual ~thread_data_base()
            {
            }

            virtual void run() = 0;
        };

        typedef boost::shared_ptr<detail::thread_data_base> thread_data_ptr;

        template<typename F>
        class thread_data : public detail::thread_data_base
        {
        public:
            thread_data(F f_) : f(f_)
            {
            }

            void run()
            {
                f();
            }

        private:
            F f;

            void operator=(thread_data&);
            thread_data(thread_data&);
        };

    } //namespace detail


    /**
    ** A simplistic implementation of boost::thread, using pspsdk calls for the thread invocation.
    **
    ** The intent of its usage is this form only:
    ** mWorkerThread = boost::thread(ThreadProc, this);
	** where ThreadProc is a static member function of the 'this' class,eg:
	** static void FOO::ThreadProc(void* inParam)
	** {
	**		FOO* instance = reinterpret_cast<FOO*>(inParam);
	**		// now you have class instance data available...
	** }
	** 
    ** Any other variant of a thread proc with more than one param is unimplemented.
    */
    class thread
    {
        /*
        ** Helper class for sceKernelStartThread, which passes args by value, not by reference
        ** We use this struct to wrap any pointers that we want to pass to the worker thread.
        */
        struct CallbackData 
        {
            CallbackData(detail::thread_data_ptr inThreadInfo)
                : mThreadInfo(inThreadInfo)
            {
            }

            detail::thread_data_ptr mThreadInfo;
        };

    public:

        thread()
        {
        }

        template <class F,class A1>
        thread(F f, A1 a1) : mThreadInfo(make_thread_info(boost::bind(boost::type<void>(), f, a1)))
        {
            LOG("Calling bind on threadproc func");
            CallbackData callbackData(mThreadInfo);

            LOG("Creating SCE Thread");
            mThreadProcID = sceKernelCreateThread( typeid(a1).name(), thread::ThreadProc, 0x15, 0x40000, PSP_THREAD_ATTR_USER, NULL);
            if (mThreadProcID > 0)
            {
                sceKernelStartThread(mThreadProcID, sizeof(CallbackData), &callbackData);
            }
        }

        ~thread()
        {
        }

        void join()
        {
            sceKernelTerminateDeleteThread(mThreadProcID);
        }

    private:

        static int ThreadProc(SceSize args, void *inParam)
        {
            LOG("Entering thread::ThreadProc");
            CallbackData* callbackData = reinterpret_cast<CallbackData* >(inParam);
            if (callbackData)
            {
                callbackData->mThreadInfo->run();
            }

            return 0;
        }

        template<typename F>
        static inline detail::thread_data_ptr make_thread_info(F f)
        {
            return detail::thread_data_ptr(new detail::thread_data<F>(f));
        }

        detail::thread_data_ptr mThreadInfo;
        SceUID mThreadProcID;
    };

    namespace posix_time
    {
        typedef unsigned int milliseconds;
    }

    /**
    ** boost's platform neutral sleep call.
    */
    namespace this_thread
    {
        inline void sleep(boost::posix_time::milliseconds const& time)
        {
            sceKernelDelayThread(time * 1000);
        }
    }
}

#endif

#endif // THREADING_H
