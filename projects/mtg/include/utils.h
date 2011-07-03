#ifndef _UTILS_H_
#define _UTILS_H_

#include <JGE.h>

#if defined (PSP) 
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

#include "DebugRoutines.h"

// enable this define to collect statistics on how many times an ifstream is created for a given file.
//#define TRACK_FILE_USAGE_STATS

namespace wagic
{

#ifdef TRACK_FILE_USAGE_STATS
    class ifstream : public std::ifstream
    {
    public:
        explicit ifstream(const char *inFilename, ios_base::openmode inMode = ios_base::in) :
        std::ifstream(inFilename, inMode)
        {
            sFileMap[std::string(inFilename)] += 1;
            DebugTrace("ifstream opened on file: " << inFilename);
        }

        static void Dump()
        {
            DebugTrace("-------------------");
            DebugTrace("File Usage Statistics" << std::endl);
            std::map<std::string, int>::const_iterator iter = sFileMap.begin();
            for (; iter != sFileMap.end(); ++iter)
            {
                DebugTrace(iter->first << "  -- " << iter->second);
            }

            DebugTrace("End File Usage Statistics");
            DebugTrace("-------------------");
        }

    private:
        static std::map<std::string, int> sFileMap;
    };

#else
    typedef std::ifstream ifstream;
#endif

} //namespace wagic

using std::string;

//string manipulation methods
string& trim(string& str);
string& ltrim(string& str);
string& rtrim(string& str);

inline string trim(const string& str)
{
    string value(str);
    return trim(value);
}

std::string join(vector<string>& v, string delim = " ");

std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems);
std::vector<std::string> split(const std::string& s, char delim); //splits a string with "delim" and returns a vector of strings.

// A simple parsing function
// splits string s by finding the first occurence of start, and the first occurence of stop, and returning
// a vector of 3 strings. The first string is everything before the first occurence of start, the second string is everything between start and stop
// the third string is everything after stop.
// for example, parseBetween ("this is a function(foo) call", "function(", ")") will return: ["this is a ", "foo", " call"];
//If an error occurs, returns an empty vector.
// if "stopRequired" is set to false, the function will return a vector of 3 strings even if "stop" is not found in the string.
std::vector<std::string>&  parseBetween(const std::string& s, string start, string stop, bool stopRequired, std::vector<std::string>& elems);
std::vector<std::string> parseBetween(const std::string& s, string start, string stop, bool stopRequired = true);

std::string wordWrap(const std::string& s, float width, int fontId);

//basic hash function
unsigned long hash_djb2(const char *str);

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

u32 ramAvailableLineareMax(void);
u32 ramAvailable(void);

#ifdef WIN32
#include <direct.h>
#define MAKEDIR(name) _mkdir(name)
#else
#include <sys/stat.h>
#define MAKEDIR(name) mkdir(name, 0777)
#endif

bool FileExists(const string& strFilename);

#endif
