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
    mResourceRoot = "/sdcard/Wagic/Res/";
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
