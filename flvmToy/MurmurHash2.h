#pragma once

void SetAffinity ( int cpu );
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
  extern "C" {
#endif

#if _GNC_
	#define	FORCE_INLINE __attribute__((always_inline))
#else 
  #define FORCE_INLINE 
#endif#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)
#define	ROTR32(x,y)	rotr32(x,y)
#define ROTR64(x,y)	rotr64(x,y)
#define BIG_CONSTANT(x) (x##LLU)
#define _stricmp strcasecmp

/* MURMUR_PLATFORM_H */


uint32_t MurmurHash2        ( const void * key, int len, uint32_t seed );
uint32_t MurmurHash2A       ( const void * key, int len, uint32_t seed );
uint32_t MurmurHashNeutral2 ( const void * key, int len, uint32_t seed );
uint32_t MurmurHashAligned2 ( const void * key, int len, uint32_t seed );
uint64_t MurmurHash64A      ( const void * key, int len, uint64_t seed );
uint64_t MurmurHash64B      ( const void * key, int len, uint64_t seed );

#ifdef __cplusplus
  }
#endif

