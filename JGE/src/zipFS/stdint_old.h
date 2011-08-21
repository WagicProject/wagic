#ifndef _ZIPFS_STDINT_H_
#define _ZIPFS_STDINT_H_

#include "static_assert.h"


#if defined _MSC_VER

typedef signed char			int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed __int64		int64_t;
typedef unsigned __int64	uint64_t;

typedef signed char			int_least8_t;
typedef unsigned char		uint_least8_t;
typedef signed short		int_least16_t;
typedef unsigned short		uint_least16_t;
typedef signed int			int_least32_t;
typedef unsigned int		uint_least32_t;
typedef signed __int64		int_least64_t;
typedef unsigned __int64	uint_least64_t;

#elif defined UNIX

typedef signed char			int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;

typedef signed char			int_least8_t;
typedef unsigned char		uint_least8_t;
typedef signed short		int_least16_t;
typedef unsigned short		uint_least16_t;
typedef signed int			int_least32_t;
typedef unsigned int		uint_least32_t;
typedef signed long long	int_least64_t;
typedef unsigned long long	uint_least64_t;

#endif





inline void __CheckSizedTypes()
{
	// one byte must be exactly 8 bits
	//static_assert(CHAR_BIT == 8);

	mstatic_assert(sizeof(int8_t) == 1);
	mstatic_assert(sizeof(uint8_t) == 1);
	mstatic_assert(sizeof(int16_t) == 2);
	mstatic_assert(sizeof(uint16_t) == 2);
	mstatic_assert(sizeof(int32_t) == 4);
	mstatic_assert(sizeof(uint32_t) == 4);
	mstatic_assert(sizeof(int64_t) == 8);
	mstatic_assert(sizeof(uint64_t) == 8);

	mstatic_assert(sizeof(int_least8_t) >= 1);
	mstatic_assert(sizeof(uint_least8_t) >= 1);
	mstatic_assert(sizeof(int_least16_t) >= 2);
	mstatic_assert(sizeof(uint_least16_t) >= 2);
	mstatic_assert(sizeof(int_least32_t) >= 4);
	mstatic_assert(sizeof(uint_least32_t) >= 4);
	mstatic_assert(sizeof(int_least64_t) >= 8);
	mstatic_assert(sizeof(uint_least64_t) >= 8);
}

#endif