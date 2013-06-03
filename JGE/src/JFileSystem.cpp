/* JFileSystem centralizes all access to resources in JGE.
It allows to have files for the game split in two subfolders, a "system" subfolder (read only) and a "user" subfolder (read/write)
Additionally, these two subfolders can have some of their resources in zip file (see zfsystem.h).
Zip files can contain duplicates of the same file, the one that will eventually be used is the one is the latest zip file (by alphabetical order)

Read access priority:
User folder, real file
User folder, zip file
System folder, real file
System folder, zip file

Write access:
User folder, real file

User folder is the only one that is really needed to guarantee both read and write access, the system folder is not necessary but provides a nice way to distinguish
The content that users should not be touching.
*/

#if defined(ANDROID)
#include "../../include/PrecompiledHeader.h"
#endif

#ifdef WIN32
#pragma warning(disable : 4786)
#include <direct.h>
#define MAKEDIR(name) _mkdir(name)
#else
#include <sys/stat.h>
#define MAKEDIR(name) mkdir(name, 0777)
#endif

#include "../include/JGE.h"
#include "../include/JFileSystem.h"
#include "../include/JLogger.h"
#include <dirent.h>

JFileSystem* JFileSystem::mInstance = NULL;

JZipCache::JZipCache()
{}

JZipCache::~JZipCache()
{
    dir.clear();
}

void JFileSystem::Pause() 
{
    filesystem::closeTempFiles();
}

void JFileSystem::preloadZip(const string& filename)
{
    map<string,JZipCache *>::iterator it = mZipCache.find(filename);
    if (it != mZipCache.end()) return;

    //random number of files stored in the cache.
    // This is based on the idea that an average filepath (in Wagic) for image is 37 characters (=bytes) + 8 bytes to store the fileinfo = 45 bytes,
    //so 4500 files represent roughly 200kB, an "ok" size for the PSP
    if (mZipCachedElementsCount > 4500) 
    {
        clearZipCache();
    }

    JZipCache * cache = new JZipCache();
    mZipCache[filename] = cache;

    if (!mZipAvailable || !mZipFile) {
		AttachZipFile(filename);
		if (!mZipAvailable || !mZipFile) return;
	}

    if ((mUserFS->PreloadZip(filename.c_str(), cache->dir) || (mSystemFS && mSystemFS->PreloadZip(filename.c_str(), cache->dir))))
    {
        mZipCachedElementsCount+= cache->dir.size();
    }
    else
    {
        DetachZipFile();
    }
}


void JFileSystem::init(const string & userPath, const string & systemPath)
{
    Destroy();
    mInstance = new JFileSystem(userPath, systemPath);
}

JFileSystem* JFileSystem::GetInstance()
{
    if (!mInstance)
    {
#ifdef RESPATH
    init( RESPATH"/");
#else
    init("Res/");
#endif
    }
    return mInstance;
}

// Tries to set the system and user paths.
// On some OSes, the parameters get overriden by hardcoded values
JFileSystem::JFileSystem(const string & _userPath, const string & _systemPath)

{
    string systemPath = _systemPath;
    string userPath = _userPath;

#ifdef IOS
    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    
    userPath = [[documentsDirectory  stringByAppendingString: @"/User/"] cStringUsingEncoding:1];
    systemPath = [[documentsDirectory  stringByAppendingString: @"/Res/"] cStringUsingEncoding:1];

#elif defined (ANDROID)
    userPath = JGE::GetInstance()->getFileSystemLocation();
    systemPath = "";

	DebugTrace("User path " << userPath);
#elif defined (QT_CONFIG)
    QDir dir(QDir::homePath());
    dir.cd(USERDIR);

    userPath = QDir::toNativeSeparators(dir.absolutePath()).toStdString();
    systemPath = "";
#else
    //Find the Res.txt file and matching Res folders for backwards compatibility
    ifstream mfile("Res.txt");
    string resPath;
    if (mfile)
    {
        bool found = false;
        while (!found && std::getline(mfile, resPath))
        {
            if (!resPath.size())
                continue;

            if (resPath[resPath.size() - 1] == '\r')
                resPath.erase(resPath.size() - 1); //Handle DOS files
            string testfile = resPath;
            testfile.append("graphics/simon.dat");
            ifstream file(testfile.c_str());
            if (file)
            {
                userPath = resPath;
                systemPath = "";
                found = true;
                file.close();
            }
        }
        mfile.close();
    }
#endif

	// Make sure the base paths finish with a '/' or a '\'
	if (! userPath.empty()) {
                string::iterator c = userPath.end();//userPath.at(userPath.size()-1);
                c--;
		if ((*c != '/') && (*c != '\\'))
			userPath += '/';
	}

	if (! systemPath.empty()) {
                string::iterator c = systemPath.end();//systemPath.at(systemPath.size()-1);
                c--;
                if ((*c != '/') && (*c != '\\'))
			systemPath += '/';
	}

	mUserFSPath = userPath;
    MAKEDIR(userPath.c_str());

    mSystemFSPath = systemPath;
   
    mUserFS = new filesystem(userPath.c_str());
    mSystemFS = (mSystemFSPath.size() && (mSystemFSPath.compare(mUserFSPath) != 0)) ? new filesystem(systemPath.c_str()) : NULL;

    mZipAvailable = false;
    mZipCachedElementsCount = 0;
    mPassword = NULL;
    mFileSize = 0;
    mCurrentFileInZip = NULL;

};

void JFileSystem::Destroy()
{
    if (mInstance)
    {
        delete mInstance;
        mInstance = NULL;
    }
}

bool JFileSystem::DirExists(const string& strDirname)
{ 
    return (mSystemFS && mSystemFS->DirExists(strDirname)) || mUserFS->DirExists(strDirname);
}

bool JFileSystem::FileExists(const string& strFilename)
{ 
    if (strFilename.length() < 1 ) return false;
    
    return (mSystemFS && mSystemFS->FileExists(strFilename)) || mUserFS->FileExists(strFilename);
}

bool JFileSystem::MakeDir(const string & dir)
{
    string fullDir = mUserFSPath + dir;
    MAKEDIR(fullDir.c_str());
    return true;
}

JFileSystem::~JFileSystem()
{
    clearZipCache();
    filesystem::closeTempFiles();
    SAFE_DELETE(mUserFS);
    SAFE_DELETE(mSystemFS);
}


void JFileSystem::clearZipCache()
{
    DetachZipFile();

    map<string,JZipCache *>::iterator it;
    for (it = mZipCache.begin(); it != mZipCache.end(); ++it){
        delete(it->second);
    }
    mZipCache.clear();
    mZipCachedElementsCount = 0;
}

bool JFileSystem::AttachZipFile(const string &zipfile, char *password /* = NULL */)
{
    if (mZipAvailable && mZipFile != NULL)
    {
        if (mZipFileName != zipfile)
            DetachZipFile();		// close the previous zip file
        else
            return true;
    }

    mZipFileName = zipfile;
    mPassword = password;

    openForRead(mZipFile, mZipFileName);

    if (!mZipFile)
        return false;


    //A hack for a zip inside a zip: instead we open the zip containing it
    if (mZipFile.Zipped())
    {
        mZipFile.close();
        assert(filesystem::getCurrentFS());
        mZipFile.open(filesystem::getCurrentZipName().c_str(), filesystem::getCurrentFS()); 
        assert(mZipFile);
    }
    mZipAvailable = true;
    return true;

}


void JFileSystem::DetachZipFile()
{
    if (mZipFile)
    {
        mZipFile.close();
    }
    mCurrentFileInZip = NULL;
    mZipAvailable = false;
}

bool JFileSystem::openForRead(izfstream & File, const string & FilePath) {

    File.open(FilePath.c_str(), mUserFS);
    if (File)
        return true;
    
    if(!mSystemFS)
        return false;

    File.open(FilePath.c_str(), mSystemFS);
    if (File)
        return true;

    return false;
}

bool JFileSystem::readIntoString(const string & FilePath, string & target)
{
    izfstream file;
    if (!openForRead(file, FilePath))
        return false;

    int fileSize = GetFileSize(file);

    try {
        target.resize((std::string::size_type) fileSize);
    } catch (bad_alloc&) {
        return false;
    }


    if (fileSize)
        file.read(&target[0], fileSize);

    file.close();
    return true;
}

bool JFileSystem::openForWrite(ofstream & File, const string & FilePath, ios_base::openmode mode)
{
    string filename = mUserFSPath;
    filename.append(FilePath);

    #if defined(ANDROID)
    DebugTrace("ANDROID");
    std::vector<string> dirs;
    string path = filename.substr( 0, filename.find_last_of( '/' ) + 1 );

    // put it into list
    dirs.push_back(path);

    //make list of directories that need to be created
    do
    {
        path = path.substr( 0, path.find_last_of( '/', path.size() - 2 ) + 1 );
        dirs.push_back(path);
    
    } while (path.compare(mUserFSPath) != 0);
    
    // remove mUserFSPath from list
    dirs.pop_back();

    // create missing directories
    for (std::vector<string>::reverse_iterator it = dirs.rbegin(); it != dirs.rend(); ++it)
    {
        if(!DirExists(*it))
        {
            MAKEDIR((*it).c_str());
        }
    }
    #endif

    File.open(filename.c_str(), mode);

    if (File)
    {
        return true;
    }
    return false;
}

bool JFileSystem::OpenFile(const string &filename)
{
    mCurrentFileInZip = NULL;

    if (!mZipAvailable || !mZipFile)
        return openForRead(mFile, filename);

    preloadZip(mZipFileName);
    map<string,JZipCache *>::iterator it = mZipCache.find(mZipFileName);
    if (it == mZipCache.end())
    {
        //DetachZipFile();
        //return OpenFile(filename); 
        return openForRead(mFile, filename);
    }
    JZipCache * zc = it->second;
    map<string,  filesystem::limited_file_info>::iterator it2 = zc->dir.find(filename);
    if (it2 == zc->dir.end())
    {
        /*DetachZipFile();
        return OpenFile(filename); */
        return openForRead(mFile, filename);
    }

    mCurrentFileInZip = &(it2->second);
    mFileSize = it2->second.m_Size;
    return true;

}


void JFileSystem::CloseFile()
{
    if (mZipAvailable && mZipFile)
    {
        mCurrentFileInZip = NULL;
    }

    if (mFile)
        mFile.close();
}

//returns 0 if less than "size" bits were read
int JFileSystem::ReadFile(void *buffer, int size)
{
    if (mCurrentFileInZip)
    {
        assert(mZipFile);
        if((size_t)size > mCurrentFileInZip->m_Size) //only support "store" method for zip inside zips
            return 0;
        std::streamoff offset = filesystem::SkipLFHdr(mZipFile, mCurrentFileInZip->m_Offset);
        if (!mZipFile.seekg(offset))
            return 0;
        mZipFile.read((char *) buffer, size);
        //TODO what if can't read
        return size;
    }

    if (!mFile)
        return 0;

    assert(!mFile.Zipped() || (size_t)size <= mFile.getUncompSize());
	mFile.read((char *)buffer, size);
    if (mFile.eof())
        return 0;
    return size;
}

std::vector<std::string>& JFileSystem::scanRealFolder(const std::string& folderName, std::vector<std::string>& results)
{
    DIR *dip = opendir(folderName.c_str());
    if (!dip)
        return results;

    while (struct dirent * dit = readdir(dip))
    {
        results.push_back(dit->d_name);
    }

    closedir(dip);

    return results;
}

std::vector<std::string>& JFileSystem::scanfolder(const std::string& _folderName, std::vector<std::string>& results)
{
    if (!_folderName.size())
        return results;

    map<string, bool> seen;


    string folderName = _folderName;
    if (folderName[folderName.length() - 1] != '/')
        folderName.append("/");

    //we scan zips first, then normal folders.
    // This is to avoid duplicate folders coming from the real folder 
    // (a folder "foo" in the zip comes out as "foo/", while on the real FS it comes out as "foo")

    //user zips
    {    
        //Scan the zip filesystem
        std::vector<std::string> userZips;
        mUserFS->scanfolder(folderName, userZips);

        for (size_t i = 0; i < userZips.size(); ++i)
            seen[userZips[i]] = true;
    }

    //system zips
    if (mSystemFS)
    {
        //Scan the zip filesystem
        std::vector<std::string> systemZips;
        mSystemFS->scanfolder(folderName, systemZips);

        for (size_t i = 0; i < systemZips.size(); ++i)
            seen[systemZips[i]] = true;

    }

    //user real files
    {
        //scan the real files
        std::vector<std::string> userReal;
        string realFolderName = mUserFSPath;
        realFolderName.append(folderName);
        scanRealFolder(realFolderName, userReal);

        for (size_t i = 0; i < userReal.size(); ++i)
        {
            string asFolder = userReal[i] + "/";
            if (seen.find(asFolder) == seen.end())
                seen[userReal[i]] = true;
        }
    }

    //system real files
    if (mSystemFS)
    {
        //scan the real files 
        std::vector<std::string> systemReal;
        string realFolderName = mSystemFSPath;
        realFolderName.append(folderName);
        scanRealFolder(realFolderName, systemReal);

    
        for (size_t i = 0; i < systemReal.size(); ++i)
        {
            string asFolder = systemReal[i] + "/";
            if (seen.find(asFolder) == seen.end())
                seen[systemReal[i]] = true;
        }
    }

    for(map<string,bool>::iterator it = seen.begin(); it != seen.end(); ++it) 
    {
      results.push_back(it->first);
    }

    return results;
}

std::vector<std::string> JFileSystem::scanfolder(const std::string& folderName)
{
    std::vector<std::string> result;
    return scanfolder(folderName, result);
}

int JFileSystem::GetFileSize()
{
    if (mCurrentFileInZip)
        return mFileSize;

    return GetFileSize(mFile);
}

bool JFileSystem::Rename(string _from, string _to)
{
    string from = mUserFSPath + _from;
    string to = mUserFSPath + _to;
    std::remove(to.c_str());
    return rename(from.c_str(), to.c_str()) ? true: false;
}

int JFileSystem::GetFileSize(izfstream & file)
{
    if (!file)
        return 0;

    if (file.Zipped())
    {
        //There's a bug in zipped version that prevents from sending a correct filesize with the "standard" seek method
        //The hack below only works for the "stored" version I think...
        return file.getUncompSize();
    }

    file.seekg (0, ios::end);
    int length = (int) file.tellg();
    file.seekg (0, ios::beg);
    return length;

}

// AVOID Using This function!!!
/*
This function is deprecated, but some code is still using it
It used to give a pathname to a file in the file system.
Now with the support of zip resources, a pathname does not make sense anymore
However some of our code still relies on "physical" files not being in zip.
So this call is now super heavy: it checks where the file is, and if it's in a zip, it extracts
it to the user Filesystem, assuming that whoever called this needs to access the file through its pathname later on.

As a result, this function isvery inefficient and shouldn't be used in the general case.
*/
string JFileSystem::GetResourceFile(string filename) 
{
    izfstream temp;
    bool result = openForRead(temp, filename);

    if (!temp || !result)
        return "";

    if (!temp.Zipped())
    {
        string result = temp.FullFilePath();
        temp.close();
        return result;
    }

    // File is inside a zip archive,
    //we copy it to the user FS
    string destFile = mUserFSPath + filename;
    ofstream dest;
    if (openForWrite(dest, filename, ios_base::binary))
    {
        // allocate memory:
        size_t length = temp.getUncompSize();
        char * buffer = new char [length];

        // read data as a block:
        temp.read(buffer, length);
        temp.close();

        dest.write (buffer,length);
        delete[] buffer;
        dest.close();
        return destFile;
    }
    return "";
}
