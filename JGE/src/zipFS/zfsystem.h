//Important: This file has been modified in order to be integrated in to JGE++
//

// zfsystem.h: interface for the zip file system classes.
//
//////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2004 Tanguy Fautré.
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//
//  Tanguy Fautré
//  softdev@telenet.be
//
//////////////////////////////////////////////////////////////////////
//
//							Zip File System.
//							****************
//
// Current version: 1.00 BETA 2 (16/07/2004)
//
// Comment: -
//
// History: - 1.00 BETA 2 (16/07/2004) - Updated to follow latest version
//                                       of fileio.h
//          - 1.00 BETA 1 (21/07/2002) - First public release
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "stdafx.h"
#include "ziphdr.h"		// Zip file header
#include "zstream.h"	// Zip Stream



// Zip File System Namespace
namespace zip_file_system {




class filesystem;



// Input Zip File class
class izfstream : public izstream
{
public:
	izfstream(filesystem * pFS = pDefaultFS);
	izfstream(const char * FilePath, filesystem * pFS = pDefaultFS);

	void open(const char * FilePath, filesystem * pFS = pDefaultFS);
	void close();

	bool is_open() const;

    void setFS(filesystem * pFS = pDefaultFS);

	bool Zipped() const;
    bool isBeingUsed() const;
	const std::string & FilePath() const;
	const std::string & FullFilePath() const;
    size_t getUncompSize();
    size_t getOffset();
    size_t getCompSize();

protected:
	friend class filesystem;

	// Default File System Pointer (default = NULL)
	static filesystem * pDefaultFS;

	std::string m_FilePath;
	std::string m_FullFilePath;
	filesystem * m_pFS;
	bool m_Zipped;
    size_t m_UncompSize;
	size_t	m_Offset;
	size_t	m_CompSize;
    bool m_Used;
        
};




// Zip File System central class
class filesystem  
{
public:
	// "local" file info class
	class file_info
	{
	public:
		file_info() : m_PackID(0), m_Offset(0), m_Size(0), m_CompSize(0), m_CompMethod(0), m_Directory(true) { }
		file_info(size_t PackID, size_t Offset, size_t Size, size_t CompSize, short CompMethod, bool Directory) :
			m_PackID(PackID), m_Offset(Offset), m_Size(Size), m_CompSize(CompSize), m_CompMethod(CompMethod), m_Directory(Directory) { }

		size_t	m_PackID;
		size_t	m_Offset;
		size_t	m_Size;
		size_t	m_CompSize;
		short	m_CompMethod;
		bool	m_Directory;
	};

	class limited_file_info
	{
	public:
		limited_file_info() :  m_Offset(0), m_Size(0) { }
		limited_file_info(size_t Offset, size_t Size) :
			m_Offset(Offset), m_Size(Size) { }

		size_t	m_Offset;
		size_t	m_Size;
	};

    class pooledBuffer
    {
    public:
        pooledBuffer(std::string filename, std::string externalFilename ) : filename(filename), externalFilename(externalFilename), buffer(NULL) {}
        ~pooledBuffer() { if (buffer) { delete buffer; } }
        std::string filename;
        std::string externalFilename;
        zbuffer * buffer;
    };


	filesystem(const char * BasePath = "", const char * FileExt = "zip", bool DefaultFS = true); 
	~filesystem();

	void MakeDefault();
	void Open(izfstream & File, const char * Filename);
    bool DirExists(const std::string & folderName);
    bool FileExists(const std::string & fileName);
    bool PreloadZip(const char * Filename, std::map<std::string, limited_file_info>& target);
    static std::string getCurrentZipName();
    static filesystem * getCurrentFS();
    static std::streamoff SkipLFHdr(std::istream & File, std::streamoff LFHdrPos);
    void unuse(izfstream & File);

    //Fills the vector results with a list of children of the given folder
    std::vector<std::string>& scanfolder(const std::string& folderName, std::vector<std::string>& results);

	friend std::ostream & operator << (std::ostream & Out, const filesystem & FS);
    static void closeTempFiles();
    
protected:



	// Zip file info class
	class zipfile_info
	{
	public:
		zipfile_info() : m_NbEntries(0), m_FilesSize(0), m_FilesCompSize(0) { }

		std::string	m_Filename;
		size_t		m_NbEntries;
		size_t		m_FilesSize;
		size_t		m_FilesCompSize;
	};

	// Class for file path string comparaison
	struct lt_path
	{
		bool operator() (const std::string & s1, const std::string & s2) const;
	};


	// Protected member functions
	// Zip file format related functions
	std::streamoff CentralDir(std::istream & File) const;
    std::streamoff CentralDirZipped(std::istream & File, std::streamoff begin, std::size_t size) const;
	headerid NextHeader(std::istream & File) const;

	// File/Zip map related functions
	bool FileNotZipped(const char * FilePath) const;
	bool FindFile(const char * Filename, file_info * FileInfo) const;
	const std::string & FindZip(size_t PackID) const;
	void InsertZip(const char * Filename, const size_t PackID);

    static zbuffer * getValidBuffer(const std::string & filename, const std::string & externalFilename, std::streamoff Offset = 0, std::streamoff Size = 0);
    static void closeBufferPool();

	// New type definitions
	typedef std::map<size_t, zipfile_info>								zipmap;
	typedef std::map<size_t, zipfile_info>::iterator					zipmap_iterator;
	typedef std::map<size_t, zipfile_info>::const_iterator				zipmap_const_iterator;
	typedef std::map<std::string, file_info, lt_path>					filemap;
	typedef std::map<std::string, file_info, lt_path>::iterator			filemap_iterator;
	typedef std::map<std::string, file_info, lt_path>::const_iterator	filemap_const_iterator;

	// Mighty protected member variables
	std::string	m_BasePath;
	std::string	m_FileExt;
	zipmap		m_Zips;
	filemap		m_Files;
    static std::vector<pooledBuffer *> m_Buffers;
    static std::ifstream CurrentZipFile;
    static std::string CurrentZipName;
    static filesystem * pCurrentFS;
};




//////////////////////////////////////////////////////////////////////
// zip_file_system::izfile Inline Functions
//////////////////////////////////////////////////////////////////////

inline izfstream::izfstream(filesystem * pFS) : m_pFS(pFS) { }

inline izfstream::izfstream(const char * FilePath, filesystem * pFS) : m_pFS(pFS) {
	open(FilePath);
}

inline void izfstream::setFS(filesystem * pFS) {
    m_pFS = pFS;
}

inline size_t izfstream::getUncompSize()
{
    return m_UncompSize;
}

inline size_t izfstream::getOffset()
{
    return m_Offset;
}

inline size_t izfstream::getCompSize()
{
    return m_CompSize;
}



inline void izfstream::open(const char * FilePath, filesystem * pFS) {
    if (pFS)
        m_pFS = pFS;

	if (m_pFS != NULL)
		m_pFS->Open(* this, FilePath);
}

inline void izfstream::close() {
#ifdef USE_ZBUFFER_POOL 
	if (m_pFS)
        m_pFS->unuse( * this);
#else
    izstream::close();
#endif
	m_FilePath = m_FullFilePath = "";
    m_UncompSize = 0;
}

inline bool izfstream::is_open() const {
	return static_cast<zbuffer *>(rdbuf())->is_open();
}

inline bool izfstream::Zipped() const {
	return m_Zipped;
}

inline bool izfstream::isBeingUsed() const {
	return m_Used;
}

inline const std::string & izfstream::FilePath() const {
	return m_FilePath;
}

inline const std::string & izfstream::FullFilePath() const {
	return m_FullFilePath;
}



//////////////////////////////////////////////////////////////////////
// zip_file_system::filesystem Inline Functions
//////////////////////////////////////////////////////////////////////

inline filesystem::~filesystem() {
	// Security mesure with izfile::pDefaultFS
	if (izfstream::pDefaultFS == this)
		izfstream::pDefaultFS = NULL;
}

inline void filesystem::closeTempFiles() {
    if (CurrentZipName.size())
    {
        CurrentZipFile.close();
        CurrentZipName = "";
    }
    closeBufferPool();
}
inline void filesystem::MakeDefault() {
	izfstream::pDefaultFS = this;
}


inline std::string filesystem::getCurrentZipName()
{
    return CurrentZipName;
}

inline filesystem * filesystem::getCurrentFS()
{
    return pCurrentFS;
}

}	// namespace zip_file_system

