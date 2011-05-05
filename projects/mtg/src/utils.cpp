#include "PrecompiledHeader.h"

#include "utils.h"
#include "MTGDefinitions.h"
#include "Subtypes.h"
#include "WResourceManager.h"
#include "WFont.h"
#include <sys/stat.h>

#ifdef PSP
#include "pspsdk.h"
#endif

namespace wagic
{
#ifdef TRACK_FILE_USAGE_STATS
    std::map<std::string, int> ifstream::sFileMap;
#endif
}

using std::vector;

int randValuesCursor = -1;
vector<int> randValues;

int loadRandValues(string s)
{
    randValues.clear();
    randValuesCursor = -1;
    while (s.size())
    {
        unsigned int value;
        size_t limiter = s.find(",");
        if (limiter != string::npos)
        {
            value = atoi(s.substr(0, limiter).c_str());
            s = s.substr(limiter + 1);
        }
        else
        {
            value = atoi(s.c_str());
            s = "";
        }
        if (value) randValues.push_back(value);
    }
    if (randValues.size()) randValuesCursor = 0;
    return 1;
}

int WRand()
{
    if (randValuesCursor == -1) return rand();
    int result = randValues[randValuesCursor];
    randValuesCursor++;
    if ((size_t) randValuesCursor >= randValues.size()) randValuesCursor = 0;
    return result;
}

int filesize(const char * filename)
{
    int file_size = 0;
#if defined (PSP)
    int file = sceIoOpen(filename, PSP_O_RDONLY, 0777);
    if (file > 0)
    {
        file_size = sceIoLseek(file, 0, PSP_SEEK_END);
        sceIoClose(file);
    }
#else
    FILE * file = fopen(filename, "rb");
    if (file != NULL)
    {
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fclose(file);
    }
#endif
    return file_size;
}


// check to see if a file exists on the file system.
int fileExists(const char * filename)
{
    wagic::ifstream fichier(filename);
    if (fichier)
    {
        fichier.close();
        return 1;
    }

    wagic::ifstream fichier2(JGE_GET_RES(filename).c_str());
    if (fichier2)
    {
        fichier2.close();
        return 1;
    }
    return 0;
}

// check to see if a file exists on the file system.
// this can be used to extract last modified date of a file.
bool FileExists(const string& strFilename)
{ 
    struct stat stFileInfo; 
    bool blnReturn = false;
    int intStat; 

    // Attempt to get the file attributes 
    intStat = stat(strFilename.c_str(),&stFileInfo); 
    if(intStat == 0)
    {
        // We were able to get the file attributes 
        // so the file obviously exists. 
        blnReturn = true; 
    }
    else
    {
        // We were not able to get the file attributes. 
        // This may mean that we don't have permission to 
        // access the folder which contains this file. If you 
        // need to do that level of checking, lookup the 
        // return values of stat which will give you 
        // more details on why stat failed. 
        
        // try to search in the resource directory 
        if ( stat( JGE_GET_RES(strFilename).c_str(), &stFileInfo ) == 0)
            blnReturn = true;
        else
            blnReturn = false; 
    }

    return(blnReturn); 
}


/*
#ifdef LINUX

#include <execinfo.h>
void dumpStack()
{
void* buffer[50];
int s = backtrace(buffer, 50);
char** tab = backtrace_symbols(buffer, s);
for (int i = 1; i < s; ++i) printf("%s\n", tab[i]);
printf("\n");
free(tab);
}

#endif
*/

/* RAM simple check functions source */

// *** FUNCTIONS ***

u32 ramAvailableLineareMax(void)
{
    u32 size, sizeblock;
    u8 *ram;

    // Init variables
    size = 0;
    sizeblock = RAM_BLOCK;

#ifdef PSP
    int disableInterrupts = pspSdkDisableInterrupts();
#endif

    // Check loop
    while (sizeblock)
    {
        // Increment size
        size += sizeblock;

        // Allocate ram
        ram = (u8 *) malloc(size);

        // Check allocate
        if (!(ram))
        {
            // Restore old size
            size -= sizeblock;

            // Size block / 2
            sizeblock >>= 1;
        }
        else
            free(ram);
    }

#ifdef PSP
    pspSdkEnableInterrupts(disableInterrupts);
#endif

    return size;
}

u32 ramAvailable(void)
{
    u8 **ram, **temp;
    u32 size, count, x;

    // Init variables
    ram = NULL;
    size = 0;
    count = 0;

#ifdef PSP
    int disableInterrupts = pspSdkDisableInterrupts();
#endif

    // Check loop
    for (;;)
    {
        // Check size entries
        if (!(count % 10))
        {
            // Allocate more entries if needed
            temp = (u8**) realloc(ram, sizeof(u8 *) * (count + 10));
            if (!(temp)) break;

            // Update entries and size (size contains also size of entries)
            ram = temp;
            size += (sizeof(u8 *) * 10);
        }

        // Find max lineare size available
        x = ramAvailableLineareMax();
        if (!(x)) break;

        // Allocate ram
        ram[count] = (u8 *) malloc(x);
        if (!(ram[count])) break;

        // Update variables
        size += x;
        count++;
    }

    // Free ram
    if (ram)
    {
        for (x = 0; x < count; x++)
            free(ram[x]);
        free(ram);
    }

#ifdef PSP
    pspSdkEnableInterrupts(disableInterrupts);
#endif
    return size;
}

/* String manipulation functions */
string& trim(string& str)
{
    str = ltrim(str);
    str = rtrim(str);
    return str;
}

string& ltrim(string& str)
{
    str.erase(0, str.find_first_not_of(" \t"));
    return str;
}

string& rtrim(string& str)
{
    str.resize(str.find_last_not_of(" \t") + 1);
    return str;
}

std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::string join(vector<string>& v, string delim)
{
    std::string retVal;
    for ( vector<string>::iterator it = v.begin(); it != v.end(); ++it )
    {
        retVal.append( *it );
        retVal.append( delim );
    }

    return retVal;
}

std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

std::vector<std::string>&  parseBetween(const std::string& s, string start, string stop, bool stopRequired, std::vector<std::string>& elems)
{
    size_t found = s.find(start);
    if (found == string::npos)
        return elems;
    
    size_t offset = found + start.size();
    size_t end = s.find(stop, offset);
    if (end == string::npos && stopRequired)
        return elems;

    elems.push_back(s.substr(0,found));
    if (end != string::npos)
    {
        elems.push_back(s.substr(offset, end - offset));
        elems.push_back(s.substr(end + 1));
    }
    else
    {
        elems.push_back(s.substr(offset));
        elems.push_back("");
    }

    return elems;
}

std::vector<std::string> parseBetween(const std::string& s, string start, string stop, bool stopRequired)
{
    std::vector<std::string> elems;
    return parseBetween(s, start, stop, stopRequired, elems);
}

// This is a customized word wrap based on pixel width.  It tries it's best 
// to wrap strings using spaces as delimiters.  
// Not sure how this translates into non-english fonts.
std::string wordWrap(const std::string& sentence, float width, int fontId)
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
    float lineWidth = mFont->GetStringWidth( sentence.c_str() );
    string retVal = sentence;
    if ( lineWidth < width ) return sentence;

    int numLines = 1;
    int breakIdx = 0;
    for( size_t idx = 0; idx < sentence.length(); idx ++ )
    {
        if ( sentence[idx] == ' ' )
        {
            string currentSentence = sentence.substr(breakIdx, idx - breakIdx);
            float stringLength = mFont->GetStringWidth( currentSentence.c_str() );
            if (stringLength >= width)
            {				
                if ( stringLength > width )
                {
                    while ( sentence[idx-1] != ' ' )
                        idx--;
                }
                retVal[idx-1] = '\n';				
                breakIdx = idx;
                numLines++;
            }
        }
        else if ( sentence[idx] == '\n' )
        {
            string currentSentence = sentence.substr(breakIdx, idx - breakIdx);
            float stringLength = mFont->GetStringWidth( currentSentence.c_str() );
            if (stringLength >= width)
            {				
                if ( stringLength > width )
                {
                    while ( sentence[idx-1] != ' ' )
                        idx--;
                    retVal[idx-1] = '\n';				
                }
                numLines++;
            }
            breakIdx = idx;
            numLines++;
        }
    }

    return retVal;
}


