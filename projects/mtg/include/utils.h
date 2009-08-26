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

#include <psptypes.h>
#include <malloc.h>

#endif


#include <math.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdlib.h>


using std::string;



int filesize(const char * filename);
int fileExists(const char * filename);

#ifdef LINUX
void dumpStack();
#endif


/* RAM simple check functions header */

// *** DEFINES ***

#if defined (WIN32) || defined (LINUX)
#define RAM_BLOCK      (100 * 1024 * 1024)
#else
#define RAM_BLOCK      (1024 * 1024)
#endif

// *** FUNCTIONS DECLARATIONS ***

u32 ramAvailableLineareMax (void);
u32 ramAvailable (void);

#ifdef WIN32
#include <direct.h>
#define MAKEDIR(name) mkdir(name)
#else
#include <sys/stat.h>
#define MAKEDIR(name) mkdir(name, 0777)
#endif

#endif

