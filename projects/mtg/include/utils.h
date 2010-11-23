#ifndef _UTILS_H_
#define _UTILS_H_

#include <JGE.h>

#if defined (WIN32) || defined (LINUX) || defined (IOS) 

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


//string manipulation methods
string& trim(string &str);
string& ltrim(string &str);
string& rtrim(string &str);

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim); //splits a string with "delim" and returns a vector of strings.
std::string wordWrap(std::string s, int width);

void PopulateColorIndexVector( list<int>& colors, const string& colorsString, char delimiter = ',');
void PopulateAbilityIndexVector( list<int>& abilities, const string& abilitiesString, char delimiter = ',');
void PopulateSubtypesIndexVector( list<int>& subtypes, const string& subtypesString, char delimiter = ' ');


int loadRandValues(string s);
int filesize(const char * filename);
int fileExists(const char * filename);
int WRand();

#ifdef LINUX
void dumpStack();
#endif


/* RAM simple check functions header */

// *** DEFINES ***

#if defined (WIN32) || defined (LINUX) || defined (IOS)
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

