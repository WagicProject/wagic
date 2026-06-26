// Compatibility stubs for libraries compiled with VS2010 (MSVC 1600)
// running against VS2022's CRT
#include <stdio.h>
#include <string.h>

// ___iob_func was removed in VS2015+; libjpeg-static (compiled for VS2010) calls it
// to get stdin/stdout/stderr as an array.
//
// Critical: VS2010's FILE struct was 32 bytes on x86 (8 pointer/int fields).
// libjpeg's compiled code contains hard-coded strides of 32 bytes for pointer
// arithmetic like (__iob_func() + 2) to reach stderr.  VS2022 FILE is 4 bytes
// (just void* _Placeholder), so returning a 3-element VS2022 FILE array causes
// the "+2" stride to land 56 bytes past the end of the array, corrupting memory.
//
// Fix: return a buffer with 32-byte slots.  Put the UCRT _Placeholder at the
// start of each slot so that UCRT I/O functions (fwrite, fprintf, etc.) still
// correctly identify stdin/stdout/stderr when they receive a pointer into arr.
#define VS2010_FILE_STRIDE 32

extern "C" FILE* __cdecl __iob_func(void) {
    // 3 slots, each large enough that VS2010's 32-byte stride stays in-bounds.
    static unsigned char arr[3 * VS2010_FILE_STRIDE];
    static int init = 0;
    if (!init) {
        init = 1;
        FILE* handles[3];
        handles[0] = __acrt_iob_func(0); // stdin
        handles[1] = __acrt_iob_func(1); // stdout
        handles[2] = __acrt_iob_func(2); // stderr
        for (int i = 0; i < 3; i++) {
            memset(arr + i * VS2010_FILE_STRIDE, 0, VS2010_FILE_STRIDE);
            // Copy the UCRT FILE struct (just _Placeholder, sizeof(FILE)==4 in VS2022)
            // into the first bytes of each slot so UCRT I/O functions find the right handle.
            memcpy(arr + i * VS2010_FILE_STRIDE, handles[i], sizeof(FILE));
        }
    }
    return (FILE*)arr;
}
