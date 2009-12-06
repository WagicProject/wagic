#ifndef _VALLOC_H
#define _VALLOC_H

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

#define vrelptr	vGuPointer
#define vabsptr vCPUPointer

size_t vgetMemorySize(unsigned int width, unsigned int height, unsigned int psm);

// Return a pointer relative to VRAM Base address useable by GU
void* vGuPointer( void* ptr );
// Return an absolute pointer useable by CPU
void* vCPUPointer( void* ptr );

// Returns an absolute pointer useable by CPU
void* valloc( size_t size );
void  vfree( void* ptr );

size_t vmemavail();
size_t vlargestblock();


#ifdef __cplusplus
}
#endif

#endif	// _VALLOC_H
