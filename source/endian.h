#pragma once
#ifndef _MSC_VER
#include <sys/param.h>
#else
#define LITTLE_ENDIAN 0
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#define uint unsigned int
#define ushort unsigned short
#define uchar unsigned char

/* For some reason these do not work...
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
*/

#define BYTE_SHR_SHL(a, b, c) ((((a) >> (b)) & 0xFF) << (c))

static inline uint eswap_uint(uint a)
{
	return BYTE_SHR_SHL(a, 0, 24)
	     | BYTE_SHR_SHL(a, 8, 16)
	     | BYTE_SHR_SHL(a, 16, 8)
	     | BYTE_SHR_SHL(a, 24, 0);
}

static inline uint eswap_u24(uint a)
{
	return BYTE_SHR_SHL(a, 0, 16)
	     | BYTE_SHR_SHL(a, 8,  8)
	     | BYTE_SHR_SHL(a, 16, 0);
}

static inline ushort eswap_ushort(ushort a)
{
	return BYTE_SHR_SHL(a, 0, 8) | BYTE_SHR_SHL(a, 8, 0);
}

#undef BYTE_SHR_SHL

#ifndef BYTE_ORDER
#error What's the endian of the platform you're targeting?
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
#define be_eswap_uint eswap_uint
#define be_eswap_u24 eswap_u24
#define be_eswap_ushort eswap_ushort
#define le_eswap_uint(a) ((uint)(a))
#define le_eswap_u24(a) ((uint)(a))
#define le_eswap_ushort(a) ((ushort)(a))
#elif BYTE_ORDER == BIG_ENDIAN
#define be_eswap_uint(a) ((uint)(a))
#define be_eswap_u24(a) ((uint)(a))
#define be_eswap_ushort(a) ((ushort)(a))
#define le_eswap_uint eswap_uint
#define le_eswap_u24 eswap_u24
#define le_eswap_ushort eswap_ushort
#else
#error What's the endian of the platform you're targeting?
#endif
