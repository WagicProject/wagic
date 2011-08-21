#ifndef _J_FILE_SYSTEM_H_
#define _J_FILE_SYSTEM_H_

#include "zfsystem.h"
#include <string>
using zip_file_system::filesystem;
using zip_file_system::izfstream;
using namespace std;

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
  map<string, filesystem::file_info> dir;
  
};

class JFileSystem {
private:
    string mSystemFSPath, mUserFSPath;
    filesystem * mSystemFS, * mUserFS;
	static JFileSystem* mInstance;
    izfstream mFile;

	map<string,JZipCache *>mZipCache;
	string mZipFileName;
    int mFileSize;
	char *mPassword;
	bool mZipAvailable;
  	void preloadZip(const string& filename);
	izfstream mZipFile;
    filesystem::file_info * mCurrentFileInZip;

    std::vector<std::string>& scanRealFolder(const std::string& folderName, std::vector<std::string>& results);

public:


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

    // Manually Clear the zip cache
    void clearZipCache();

	//////////////////////////////////////////////////////////////////////////
	/// Get the singleton instance
	///
	//////////////////////////////////////////////////////////////////////////
	static JFileSystem* GetInstance();

	static void Destroy();

	//////////////////////////////////////////////////////////////////////////
	/// Open file for reading.
	///
	//////////////////////////////////////////////////////////////////////////
	bool OpenFile(const string &filename);

    //Fills the vector results with a list of children of the given folder
    std::vector<std::string>& scanfolder(const std::string& folderName, std::vector<std::string>& results);
    std::vector<std::string> scanfolder(const std::string& folderName);
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
    int GetFileSize(izfstream & file);

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
	void SetSystemRoot(const string& resourceRoot);
    string GetSystemRoot() { return mSystemFSPath; };

	void SetUSerRoot(const string& resourceRoot);
    string GetUserRoot() { return mUserFSPath; };

    bool openForRead(izfstream & File, const string & FilePath);
    bool readIntoString(const string & FilePath, string & target);
    bool openForWrite(ofstream & File, const string & FilePath, ios_base::openmode mode = ios_base::out );
    bool Rename(string from, string to);

    //Returns true if strFilename exists somewhere in the fileSystem
    bool FileExists(const string& strFilename);

    //Returns true if strdirname exists somewhere in the fileSystem, and is a directory
    bool DirExists(const string& strDirname);

    static void init( const string & userPath, const string & systemPath = "");



    // AVOID Using This function!!!
    /*
    This function is deprecated, but some code is still using it
    It used to give a pathname to a file in the file system.
    Now with the support of zip resources, a pathname does not make sense anymore
    However some of our code still relies on "physical" files not being in zip.
    So this call is now super heavy: it checks where the file is, and if it's in a zip, it extracts
    it to the user Filesystem, assuming that whoever called this needs to access the file through its pathname later on
    */
    string GetResourceFile(string filename);

protected:
	JFileSystem(const string & userPath, const string & systemPath = "");
	~JFileSystem();


};





#endif