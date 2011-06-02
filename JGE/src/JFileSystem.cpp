//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include "../include/JGE.h"
#include "../include/JFileSystem.h"
#include "../include/JLogger.h"
#include "tinyxml/tinyxml.h"
#include "unzip/unzip.h"


#include <stdio.h>
#include <vector>
#include <map>
#include <string>

#include "../include/DebugRoutines.h"
#include <fstream>


JFile::JFile() : mFile(0), mFileSize(0)
{
}

JFile::~JFile()
{
}

bool JFile::OpenFile(const string &filename)
{
    bool result = false;
    mFilename = JFileSystem::GetInstance()->GetResourceRoot() + filename;

#if defined (PSP)
    mFile = sceIoOpen(mFilename.c_str(), PSP_O_RDONLY, 0777);
    if (mFile > 0)
    {
        mFileSize = sceIoLseek(mFile, 0, PSP_SEEK_END);
        sceIoLseek(mFile, 0, PSP_SEEK_SET);
        result = true;
    }
#else
    mFile = fopen(mFilename.c_str(), "rb");
    if (mFile != NULL)
    {
        fseek(mFile, 0, SEEK_END);
        mFileSize = ftell(mFile);
        fseek(mFile, 0, SEEK_SET);
        result = true;
    }
#endif

    return result;
}

int JFile::ReadFile(void *buffer, int size)
{
#if defined (PSP)
    return sceIoRead(mFile, buffer, size);        
#else
    return fread(buffer, 1, size, mFile);
#endif
}

int JFile::GetFileSize()
{
    return mFileSize;
}

void JFile::CloseFile()
{

#if defined (PSP)
    if (mFile > 0)
        sceIoClose(mFile);    
#else
    if (mFile != NULL)
        fclose(mFile);
#endif

    mFile = 0;
    mFileSize = 0;
}


JZipFile::JZipFile(const std::string& inZipFilename)
    : mZipFilename(inZipFilename)
{
    mZipFile = unzOpen(mZipFilename.c_str());
    assert(mZipFile);

    if (mZipFile == NULL)
        throw;
}

JZipFile::~JZipFile()
{
    unzClose(mZipFile);
}

const int kCaseInsensitive = 2;

bool JZipFile::OpenFile(const string &filename)
{
    bool result = false;
    mFilename = JFileSystem::GetInstance()->GetResourceRoot() + filename;

    if (mZipFile)
    {
        int fileAttempt = unzLocateFile(mZipFile, mFilename.c_str(), kCaseInsensitive);
        if (fileAttempt != UNZ_END_OF_LIST_OF_FILE)
        {
            result = (unzOpenCurrentFile(mZipFile) == UNZ_OK);
        }
    }

    return result;
}

int JZipFile::ReadFile(void *buffer, int size)
{
    return unzReadCurrentFile(mZipFile, buffer, size);
}

int JZipFile::GetFileSize()
{
    int result = 0;
    if (mZipFile != NULL)
    {
        unz_file_info info;
        unzGetCurrentFileInfo(mZipFile, &info, NULL, 0, NULL, 0, NULL, 0);
        result = info.uncompressed_size;
    }
    return result;
}

void JZipFile::CloseFile()
{
    if (mZipFile != NULL)
    {
        unzCloseCurrentFile(mZipFile);
    }
}




JZipCache::JZipCache()
{}

JZipCache::~JZipCache()
{
    map<string,unz_file_pos *>::iterator it;
    for (it = dir.begin(); it != dir.end(); ++it)
    {
        delete(it->second);
    }
    dir.clear();
}

void JFileSystem::preloadZip(const string& filename)
{
    map<string,JZipCache *>::iterator it = mZipCache.find(filename);
    if (it != mZipCache.end()) return;

    JZipCache * cache = new JZipCache();
    mZipCache[filename] = cache;

    if (!mZipAvailable || !mZipFile)
    {
        AttachZipFile(filename);
        if (!mZipAvailable || !mZipFile) return;
    }
    int err = unzGoToFirstFile (mZipFile);
    while (err == UNZ_OK)
    {
        unz_file_pos* filePos = new unz_file_pos();
        char filenameInzip[4096];
        if (unzGetCurrentFileInfo(mZipFile, NULL, filenameInzip, sizeof(filenameInzip), NULL, 0, NULL, 0) == UNZ_OK)
        {
            unzGetFilePos(mZipFile, filePos);
            string name = filenameInzip;
            cache->dir[name] = filePos;
        }
        err = unzGoToNextFile(mZipFile);
    }
}

JFileSystem* JFileSystem::mInstance = NULL;

JFileSystem* JFileSystem::GetInstance()
{
    if (mInstance == NULL)
    {
        mInstance = new JFileSystem();
    }

    return mInstance;
}


void JFileSystem::Destroy()
{
    if (mInstance)
    {
        delete mInstance;
        mInstance = NULL;
    }
}


JFileSystem::JFileSystem()
{
    mZipAvailable = false;
#if defined (PSP)
	mFile = -1;
#else
    mFile = NULL;
#endif
    mPassword = NULL;
    mZipFile = NULL;
    mFileSize = 0;

#ifdef RESPATH
    SetResourceRoot(RESPATH"/");
#else
    SetResourceRoot("Res/");				// default root folder
#endif
}


JFileSystem::~JFileSystem()
{
    DetachZipFile();

    map<string,JZipCache *>::iterator it;
    for (it = mZipCache.begin(); it != mZipCache.end(); ++it){
        delete(it->second);
    }
    mZipCache.clear();	
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

    mZipFile = unzOpen(mZipFileName.c_str());

    if (mZipFile != NULL)
    {
        mZipAvailable = true;
        return true;
    }

    return false;
}


void JFileSystem::DetachZipFile()
{
    if (mZipAvailable && mZipFile != NULL)
    {
        int error = unzCloseCurrentFile(mZipFile);
        if (error < 0 )
            JLOG("error calling unzCloseCurrentFile");

        error = unzClose(mZipFile);
        if (error < 0)
            JLOG("Error calling unzClose");
    }

    mZipFile = NULL;
    mZipAvailable = false;
}


bool JFileSystem::OpenFile(const string &filename)
{
    string path = mResourceRoot + filename;

    if (mZipAvailable && mZipFile != NULL)
    {
        preloadZip(mZipFileName);
        map<string,JZipCache *>::iterator it = mZipCache.find(mZipFileName);
        if (it == mZipCache.end())
        {
            DetachZipFile();
            return OpenFile(filename);  
        }
        JZipCache * zc = it->second;
        map<string,unz_file_pos *>::iterator it2 = zc->dir.find(filename);
        if (it2 == zc->dir.end())
        {
            DetachZipFile();
            return OpenFile(filename);  
        }
        unzGoToFilePos(mZipFile,it2->second);
        char filenameInzip[256];
        unz_file_info fileInfo;

        if (unzGetCurrentFileInfo(mZipFile, &fileInfo, filenameInzip, sizeof(filenameInzip), NULL, 0, NULL, 0) == UNZ_OK)
            mFileSize = fileInfo.uncompressed_size;
        else
            mFileSize = 0;

        return (unzOpenCurrentFilePassword(mZipFile, mPassword) == UNZ_OK);
    }
    else
    {
#if defined (PSP)
        mFile = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0777);
        if (mFile > 0)
        {
            mFileSize = sceIoLseek(mFile, 0, PSP_SEEK_END);
            sceIoLseek(mFile, 0, PSP_SEEK_SET);
            return true;
        }
#else
        mFile = fopen(path.c_str(), "rb");
        if (mFile != NULL)
        {
            fseek(mFile, 0, SEEK_END);
            mFileSize = ftell(mFile);
            fseek(mFile, 0, SEEK_SET);
            return true;
        }
#endif
    }

    return false;
}


void JFileSystem::CloseFile()
{
    if (mZipAvailable && mZipFile != NULL)
    {
        unzCloseCurrentFile(mZipFile);
        return;
    }

#if defined (PSP)
    if (mFile > 0)
        sceIoClose(mFile);    
#else
    if (mFile != NULL)
        fclose(mFile);
#endif
}


int JFileSystem::ReadFile(void *buffer, int size)
{
	if (mZipAvailable && mZipFile != NULL)
	{
		return unzReadCurrentFile(mZipFile, buffer, size);
	}
	else
	{
#if defined (PSP)
        return sceIoRead(mFile, buffer, size);        
#else
        return fread(buffer, 1, size, mFile);
#endif
	}
}


int JFileSystem::GetFileSize()
{
    return mFileSize;
}


void JFileSystem::SetResourceRoot(const string& resourceRoot)
{
#ifdef IOS
    //copy the RES folder over to the Documents folder
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error;
    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory,
                                                         NSUserDomainMask, YES);
    NSString *documentsDirectory = [[paths objectAtIndex:0] stringByAppendingString: @"/Res"];
    
    NSString *resourceDBFolderPath = [[[NSBundle mainBundle] resourcePath] 
                                      stringByAppendingPathComponent:@"Res"];
    // copy the Res folder over to the Documents directory if it doesn't exist.
    if ( ![fileManager fileExistsAtPath: documentsDirectory])
        [fileManager copyItemAtPath:resourceDBFolderPath toPath:documentsDirectory error:&error];
    
    mResourceRoot = [documentsDirectory cStringUsingEncoding:1];
    mResourceRoot += "/";
#elif defined (ANDROID)
    mResourceRoot = "/mnt/sdcard-ext/Wagic/Res/";

    DebugTrace("test: writing to /sdcard-ext/ ");
    std::ofstream file("/mnt/sdcard-ext/Foo.txt");
    if (file)
    {
        DebugTrace("successfully opened foo.txt...");
        file << "test";
        file.close();
    }
#else
    mResourceRoot = resourceRoot;
#endif
}

string JFileSystem::GetResourceRoot() 
{
    return mResourceRoot;
}

string JFileSystem::GetResourceFile(string filename) 
{
    string result = mResourceRoot;
    return result.append(filename);
}
