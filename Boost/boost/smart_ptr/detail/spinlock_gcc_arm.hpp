#ifndef BOOST_SMART_PTR_DETAIL_SPINLOCK_GCC_ARM_HPP_INCLUDED
#define BOOST_SMART_PTR_DETAIL_SPINLOCK_GCC_ARM_HPP_INCLUDED

//
//  Copyright (c) 2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/smart_ptr/detail/yield_k.hpp>

namespace boost
{

namespace detail
{

class spinlock
{
public:

    int v_;

public:

    bool try_lock()
    {
        int r;

        #if defined(__ARM_FEATURE_LSE)
        // Use LSE atomic instructions if supported
        #pragma message("LSE feature detected") // This will print a message in your build logs
        __asm__ __volatile__(
            "ldaxr %0, [%1];"   // Load-Exclusive instruction
            "cbnz %0, 1f;"       // If the value is non-zero, the lock is already acquired
            "stlxr %w0, %2, [%1];" // Store-Exclusive instruction
            "cbnz %w0, 1f;"      // If the store failed, retry
            "mov %0, #0;"        // Success, zero indicates lock acquired
            "1:"
            : "=&r"(r)
            : "r"(&v_), "r"(1)
            : "memory", "cc"
        );
        #else
        // Fallback for systems that don't support LSE
        #pragma message("LSE feature not detected") // This will print a message in your build logs if LSE is not detected
        __asm__ __volatile__(
            "swp %0, %1, [%2];"  // Swap instruction (used as a fallback)
            : "=&r"(r)            // output constraint
            : "r"(1), "r"(&v_)    // input constraints
            : "memory", "cc"      // clobbered registers
        );
        #endif

        return r == 0;
    }

    void lock()
    {
        for( unsigned k = 0; !try_lock(); ++k )
        {
            boost::detail::yield( k );
        }
    }

    void unlock()
    {
        __asm__ __volatile__( "" ::: "memory" );
        *const_cast< int volatile* >( &v_ ) = 0;
    }

public:

    class scoped_lock
    {
    private:

        spinlock & sp_;

        scoped_lock( scoped_lock const & );
        scoped_lock & operator=( scoped_lock const & );

    public:

        explicit scoped_lock( spinlock & sp ): sp_( sp )
        {
            sp.lock();
        }

        ~scoped_lock()
        {
            sp_.unlock();
        }
    };
};

} // namespace detail
} // namespace boost

#define BOOST_DETAIL_SPINLOCK_INIT {0}

#endif // #ifndef BOOST_SMART_PTR_DETAIL_SPINLOCK_GCC_ARM_HPP_INCLUDED
