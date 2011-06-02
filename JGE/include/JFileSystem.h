//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

#define JGE_GET_RES(filename) JFileSystem::GetInstance()->GetResourceFile(filename)
#define JGE_GET_RESPATH() JFileSystem::GetInstance()->GetResourceRoot()

#include <stdio.h>
#include <vector>
#include <map>
#include <string>

#if defined (PSP)
	#include <pspiofilemgr.h>
	#include <pspiofilemgr_fcntl.h>
#endif

#include "unzip/unzip.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
/// Interface for low level file access with ZIP archive support. All
/// file operations in JGE are handled through this class so if a ZIP
/// archive is attached, all the resources will be loaded from the
/// archive file.
///
//////////////////////////////////////////////////////////////////////////

class JZipCache {
public:
  JZipCache();
  ~JZipCache();
  map<string,unz_file_pos *> dir;
  
};

class JFileSystem
{
public:

	//////////////////////////////////////////////////////////////////////////
	/// Get the singleton instance
	///
	//////////////////////////////////////////////////////////////////////////
	static JFileSystem* GetInstance();

	static void Destroy();


	//////////////////////////////////////////////////////////////////////////
	/// Attach ZIP archive to the file system.
	///
	/// @param zipfile - Name of ZIP archive.
	/// @param password - Password for the ZIP archive. Default is NULL.
	///
	/// @return Status of the attach operation.
	///
	//////////////////////////////////////////////////////////////////////////
	bool AttachZipFile(const string &zipfile, char *password = NULL);

	//////////////////////////////////////////////////////////////////////////
	/// Release the attached ZIP archive.
	///
	//////////////////////////////////////////////////////////////////////////
	void DetachZipFile();

	//////////////////////////////////////////////////////////////////////////
	/// Open file for reading.
	///
	//////////////////////////////////////////////////////////////////////////
	bool OpenFile(const string &filename);

	//////////////////////////////////////////////////////////////////////////
	/// Read data from file.
	///
	/// @param buffer - Buffer for reading.
	/// @param size - Number of bytes to read.
	///
	/// @return Number of bytes read.
	///
	//////////////////////////////////////////////////////////////////////////
	int ReadFile(void *buffer, int size);

	//////////////////////////////////////////////////////////////////////////
	/// Get size of file.
	///
	//////////////////////////////////////////////////////////////////////////
	int GetFileSize();

	//////////////////////////////////////////////////////////////////////////
	/// Close file.
	///
	//////////////////////////////////////////////////////////////////////////
	void CloseFile();

	//////////////////////////////////////////////////////////////////////////
	/// Set root for all the following file operations
	///
	/// @resourceRoot - New root.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetResourceRoot(const string& resourceRoot);
  string GetResourceRoot();

  // Returns a string prefixed with the resource path
  string GetResourceFile(string filename);

protected:
	JFileSystem();
	~JFileSystem();

private:
	static JFileSystem* mInstance;

	map<string,JZipCache *>mZipCache;
	string mResourceRoot;
	string mZipFileName;
	char *mPassword;
	bool mZipAvailable;
  	void preloadZip(const string& filename);
#if defined (PSP)
	SceUID mFile;
#else
	FILE *mFile;
#endif
	unzFile mZipFile;
	int mFileSize;

};

#endif
