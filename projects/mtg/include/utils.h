#ifndef _UTILS_H_
#define _UTILS_H_

#include <JGE.h>

#if defined (WIN32) || defined (LINUX)

#else
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspiofilemgr.h>
#include <pspdebug.h>
#include <psputility.h>
#include <pspgu.h>
#include <psprtc.h>

#endif


#include <math.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdlib.h>

#define BUFSIZE 600



using std::string;

template <typename T, size_t N>
char ( &_ArraySizeHelper( T (&array)[N] ))[N];
#define countof( array ) (sizeof( _ArraySizeHelper( array ) ))



int lowercase(string  source);

int substr_copy(char *source, char *target, int start, int len);
int append_str(char * s1, char * s2, char * target);
int filesize(const char * filename);
int read_file (const char * filename, char * buffer, int filesize);
int readline (char * in_buffer, char * out_buffer, int cursor);
int readfile_to_ints(const char * filename, int * out_buffer);

#endif
