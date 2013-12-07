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
#include <list>

#include "DebugRoutines.h"


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

// This class wraps random generation and the pre-loading/saving of randoms
// The idea is to make it instantiable to be able to handle randoms differently per class of group of classes.
// In particular, to be able to control the AI randoms independently of the other game randoms so that we can actually test AI
class RandomGenerator
{
protected:
    list<int> loadedRandomValues;
    list<int> usedRandomValues;
    bool log;
public:
    RandomGenerator(unsigned int seed = -1, bool doLog = false) : log(doLog) { if(seed != (unsigned int)-1) srand(seed);};
    void loadRandValues(string s);
    ostream& saveUsedRandValues(ostream& out) const;
    ostream& saveLoadedRandValues(ostream& out);
    int random();
	void setSeed(unsigned int seed) { srand(seed); };
    template<typename Iter> void random_shuffle(Iter first, Iter last)
    {
        ptrdiff_t i, n;
        n = (last-first);
        for (i=n-1; i>0; --i) swap (first[i],first[random()%(i+1)]);
    };
};

int WRand(bool log = false);

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

/*
#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif
*/

bool fileExists(const char * filename);
bool FileExists(const string & filename);
std::string buildFilePath(const vector<string> & folders, const string & filename);
std::string ensureFolder(const string & folderName);
/*
template <class T> istream& operator>>(istream& in, T& p)
{
    string s;

    while(std::getline(in, s))
    {
        if(!p.parseLine(s))
        {
            break;
        }
    }

    return in;
}
*/

/* replace_all ... replacement to avoid depending on boost for that */
void ReplaceString(std::string& subject, const std::string& search, const std::string& replace);

/*! \brief Returns true if base starts with start, otherwise false
 *
 * Compares the first strlen(start) characters of base with start and
 * returns true if both match.
 */
bool StartsWith(const std::string& base, const char *start);

/*! \brief Returns true if base starts with start, otherwise false
 * 
 * This version is slightly more efficient as strlen does not need to
 * get called. Otherwise, it behaves exactly like 
 * StartsWith(const std::string& base, const char *start)
 *
 * \see StartsWith(const std::string& base, const char *start)
 */
bool StartsWith(const std::string& base, const std::string& start);
#endif
