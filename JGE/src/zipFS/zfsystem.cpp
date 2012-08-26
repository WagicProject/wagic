//Important: This file has been modified in order to be integrated in to JGE++
//

// zfsystem.cpp: implementation of the zip file system classes.
//
// Copyright (C) 2004 Tanguy Fautre
// For conditions of distribution and use,
// see copyright notice in zfsystem.h
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "zfsystem.h"

// Debug
#include "../../include/JLogger.h"

#include "fileio.h"		// I/O facilities


#if defined (WIN32)
#include <sys/types.h>
#endif

#include <sys/stat.h>

namespace zip_file_system {

using namespace std;




//////////////////////////////////////////////////////////////////////
// Static variables initialization
//////////////////////////////////////////////////////////////////////

filesystem * izfstream::pDefaultFS = NULL;
string filesystem::CurrentZipName = "";
ifstream filesystem::CurrentZipFile;
filesystem * filesystem::pCurrentFS = NULL;
std::vector<filesystem::pooledBuffer *> filesystem::m_Buffers;

static const int STORED = 0;
static const int DEFLATED = 8;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

filesystem::filesystem(const char * BasePath, const char * FileExt, bool DefaultFS)
	: m_BasePath(BasePath), m_FileExt(FileExt)
{
	using io_facilities::search_iterator;

	// Init m_BasePath and be sure the base path finish with a '/' or a '\'
	if (! m_BasePath.empty()) {
		string::iterator c = m_BasePath.end();
		c--;
		if ((*c != '/') && (*c != '\\'))
			m_BasePath += '/';
	}

	// Search all *.zip files (or whatever the ZipExt specify as the file extension)
    // Search is case insensitive (see fileio.h for details)
    // The case insensitive state is mostly because some of the filesystems we support such as FAT32 are case insensitive
    // Being case sensitive would lead to weird bugs on these systems.
	vector<string> ZipFiles;

	for (search_iterator ZSrch = (m_BasePath + "*." + m_FileExt).c_str(); ZSrch != ZSrch.end(); ++ZSrch)
		ZipFiles.push_back(ZSrch.Name());

	// Open each zip files that have been found, in alphabetic order
	sort(ZipFiles.begin(), ZipFiles.end());

	for (vector<string>::const_iterator ZipIt = ZipFiles.begin(); ZipIt != ZipFiles.end(); ++ZipIt)
		InsertZip(ZipIt->c_str(), ZipIt - ZipFiles.begin());

	// Should we make this the default File System for ifile?
	if (DefaultFS)
		MakeDefault();
}



//////////////////////////////////////////////////////////////////////
// File System Member Functions
//////////////////////////////////////////////////////////////////////

zbuffer * filesystem::getValidBuffer(const std::string & filename, const std::string & externalFilename, std::streamoff Offset, std::streamoff Size )
{
    //if exists filename in pool and is not in use, return that
    for (size_t i = 0; i < m_Buffers.size(); ++i)
    {
        if (m_Buffers[i]->filename != filename)
            continue;
        zbuffer * buffer = m_Buffers[i]->buffer;
        if (buffer && !buffer->is_used())
        {
            buffer->use(Offset, Size);
            return buffer;
        }
    }

    // if more than 3 objects in the pool, delete and close the first one that is unused
    if (m_Buffers.size() > 3) 
    {
        for (size_t i = 0; i < m_Buffers.size(); ++i)
        {
            zbuffer * buffer = m_Buffers[i]->buffer;
            if (buffer && !buffer->is_used())
            {
                delete m_Buffers[i];
                m_Buffers.erase(m_Buffers.begin() + i);
                break;
            }
        }
    }

    //No possiblility to open more files for now
    if (m_Buffers.size() > 3)
        return NULL;

    //create a new buffer object, add it to the pool, and return that
    pooledBuffer * pb = new pooledBuffer(filename, externalFilename);

        zbuffer * buffer = new zbuffer_stored();
        buffer->open(filename.c_str(), Offset, Size);
        pb->buffer = buffer;

    m_Buffers.push_back(pb);
    return pb->buffer;


}


void filesystem::closeBufferPool()
{
    for (size_t i = 0; i < m_Buffers.size(); ++i)
    {
        if (m_Buffers[i])
        {
            if (m_Buffers[i]->buffer && m_Buffers[i]->buffer->is_used())
            {
                LOG("FATAL: File Buffer still in use but need to close");
            }

            delete m_Buffers[i];
        }
    }
    m_Buffers.clear();
}

void filesystem::unuse(izfstream & File)
{

    File.setstate(std::ios::badbit);

    if (!File.Zipped())
    {
        delete(File.rdbuf(NULL));
    }
    else
    {
        zbuffer * buffer = static_cast<zbuffer *>(File.rdbuf());
        if (buffer)
            buffer->unuse();
    }
}

void filesystem::Open(izfstream & File, const char * Filename)
{
	// Close the file if it was opened;
	File.close();
    File.setFS(this);

	// Generate the path and see if the file is zipped or not
	string FullPath = m_BasePath + Filename;

	// File is not zipped
	if (FileNotZipped(FullPath.c_str())) {

		// Link the izfile object with an opened filebuf
        filebuf * FileBuf = new filebuf;
		FileBuf->open(FullPath.c_str(), ios::binary | ios::in);

		if (FileBuf->is_open()) {
#ifdef USE_ZBUFFER_POOL 
			File.rdbuf(FileBuf);
#else
            delete File.rdbuf(FileBuf);
#endif
			File.clear(ios::goodbit);
			File.m_FilePath = Filename;
			File.m_FullFilePath = FullPath;
			File.m_Zipped = false;
		}

	// File is maybe zipped
	} else {

		file_info FileInfo;
		string ZipPath;

		// Check whether the file is zipped, whether the file is a directory and try to open.
		if (FindFile(Filename, &FileInfo) && (! FileInfo.m_Directory) && (! ((ZipPath = FindZip(FileInfo.m_PackID)).empty()))) {

			// Get the position of the compressed data
            if (CurrentZipName.size())
            {
                if ((pCurrentFS!= this) || (CurrentZipName.compare(ZipPath) != 0))
                {
                    CurrentZipFile.close();
                    CurrentZipName = "";
                    pCurrentFS = NULL;
                }
            }
            if (!CurrentZipName.size())
            {
                CurrentZipName = ZipPath;
                string zipName = m_BasePath + CurrentZipName;
                CurrentZipFile.open(zipName.c_str(), ios::binary);
                pCurrentFS = this;
            }

			if (!CurrentZipFile) {
                CurrentZipName = "";
                pCurrentFS = NULL;
                return;
            }

			streamoff DataPos = SkipLFHdr(CurrentZipFile, streamoff(FileInfo.m_Offset));

			if (DataPos != streamoff(-1)) {
                string zipName = m_BasePath + CurrentZipName;
				// Open the file at the right position

#ifdef USE_ZBUFFER_POOL 
                zbuffer * buffer = getValidBuffer(zipName, Filename,  streamoff(DataPos), streamoff(FileInfo.m_CompSize));

				if (!buffer) 
                {
                    File.setstate(ios::badbit);
                } 
                else
                {
                    File.rdbuf(buffer);
                    File._SetCompMethod(FileInfo.m_CompMethod);
#else
                ((izstream &) File).open(
                    zipName.c_str(),
                    streamoff(DataPos),
                    streamoff(FileInfo.m_CompSize),
                    FileInfo.m_CompMethod
                    );
                if (File) {
#endif
					File.m_FilePath = Filename;
					File.m_FullFilePath = FullPath;
					File.m_Zipped = true;
                    File.m_UncompSize = FileInfo.m_Size;
                    File.m_CompSize = FileInfo.m_CompSize;
                    File.m_Offset = FileInfo.m_Offset;
				}
			}

		}
	}
}

bool filesystem::DirExists(const std::string & folderName)
{

    //Check in zip
	file_info FileInfo;

	// Check whether the file is zipped, whether the file is a directory and try to open.
    if (FindFile(folderName.c_str(), &FileInfo) && (FileInfo.m_Directory))
        return true;

    //check real folder
    string FullPath = m_BasePath + folderName;

#if defined (WIN32)
    struct _stat statBuffer;
    if ((_stat(FullPath.c_str(), &statBuffer) >= 0 && // make sure it exists
        statBuffer.st_mode & S_IFDIR)) // and it's not a file
        return true;
#else
    struct stat st;
    if (stat(FullPath.c_str(), &st) == 0)
        return true;
#endif

    //Neither in real folder nor in zip
    return false;
}

bool filesystem::FileExists(const std::string & fileName)
{
    if (fileName.length() < 1) return false;
    //Check in zip
	file_info FileInfo;

	// Check whether the file is zipped, whether the file is a directory and try to open.
    if (FindFile(fileName.c_str(), &FileInfo) && (!FileInfo.m_Directory))
        return true;

    //check real folder
    string FullPath = m_BasePath + fileName;

#if defined (WIN32)
    struct _stat statBuffer;
    if (_stat(FullPath.c_str(), &statBuffer) >= 0) // make sure it exists
        return true;
#else
    struct stat st;
    if (stat(FullPath.c_str(), &st) == 0)
        return true;
#endif

    //Neither in real folder nor in zip
    return false;
}


// Note: this doesn't scan the folders outside of the zip...should we add that here ?
std::vector<std::string>& filesystem::scanfolder(const std::string& folderName, std::vector<std::string>& results)
{
    filemap_const_iterator folderPos = m_Files.find(folderName);

    if (folderPos == m_Files.end())
        return results;

    filemap_const_iterator It = folderPos;

    string folderNameLC = folderName;
    std::transform(folderNameLC.begin(), folderNameLC.end(), folderNameLC.begin(), ::tolower);
    size_t length = folderNameLC.length();

    while(++It != m_Files.end())
    {
        string currentFile = (* It).first;
        string currentFileLC = currentFile;
        std::transform(currentFileLC.begin(), currentFileLC.end(), currentFileLC.begin(), ::tolower);
        if (currentFileLC.find(folderNameLC) == 0)
        {
            string relativePath = currentFile.substr(length);
            size_t pos = relativePath.find_first_of("/\\");
            //Only add direct children, no recursive browse
            if (pos == string::npos || pos == (relativePath.length() - 1))
                results.push_back(relativePath);
        }
        else
        {
            break;
            //We know other files will not belong to that folder because of the order of the map
        }
    }

    return results;
}


//////////////////////////////////////////////////////////////////////
// File System Protected Member Functions
//////////////////////////////////////////////////////////////////////

bool filesystem::FileNotZipped(const char * FilePath) const
{
	//return io_facilities::search_iterator(FilePath);
	// follow new search_iterator implementation
	std::ifstream File(FilePath);

	if (! File)
		return false;

	return true;
}



bool filesystem::FindFile(const char * Filename, file_info * FileInfo) const
{
	filemap_const_iterator It = m_Files.find(Filename);

	if (It == m_Files.end())
		return false;	// File not found

	* FileInfo = (* It).second;
	return true;
}



const string & filesystem::FindZip(size_t PackID) const
{
	static const string EmptyString;

	zipmap_const_iterator It = m_Zips.find(PackID);

	if (It  == m_Zips.end())
		return EmptyString;	// PackID not valid

	return (* It).second.m_Filename;
}



void filesystem::InsertZip(const char * Filename, const size_t PackID)
{
	zipfile_info ZipInfo;

	// Get full path to the zip file and prepare ZipInfo
	ZipInfo.m_Filename = Filename;
	string ZipPath = m_BasePath + Filename;

	// Open zip
    LOG(("opening zip:" + ZipPath).c_str());
    ifstream File(ZipPath.c_str(), ios::binary);



	if (! File)
		return;

	// Find the start of the central directory
	if (! File.seekg(CentralDir(File)))
    {
        File.close();
        return;
    }

    LOG("open zip ok");

	// Check every headers within the zip file
	file_header FileHdr;

	while ((NextHeader(File) == FILE) && (FileHdr.ReadHeader(File))) {

		// Include files into Files map
		const char * Name = &(* FileHdr.m_Filename.begin());
		const unsigned short i = FileHdr.m_FilenameSize - 1;
		if (FileHdr.m_FilenameSize != 0) {
			m_Files[Name] = file_info(
				PackID,									// Package ID
				FileHdr.m_RelOffset,					// "Local File" header offset position
				FileHdr.m_UncompSize,					// File Size
				FileHdr.m_CompSize,						// Compressed File Size
				FileHdr.m_CompMethod,					// Compression Method;
				((Name[i] == '/') || (Name[i] == '\\'))	// Is a directory?
			);

            ++(ZipInfo.m_NbEntries);
			ZipInfo.m_FilesSize += FileHdr.m_UncompSize;
			ZipInfo.m_FilesCompSize += FileHdr.m_CompSize;
		}
	}

	File.close();

	// Add zip file to Zips data base (only if not empty)
	if (ZipInfo.m_NbEntries != 0)
		m_Zips[PackID] = ZipInfo;

    LOG("--zip file loading DONE");
}


bool filesystem::PreloadZip(const char * Filename, map<string, limited_file_info>& target)
{
	zipfile_info ZipInfo;

	// Open zip
	izfstream File;
    File.open(Filename, this);

	if (! File)
		return false;

	// Find the start of the central directory
    if (File.Zipped())
    {
        streamoff realBeginOfFile =  SkipLFHdr(CurrentZipFile, File.getOffset());
        if (! CurrentZipFile.seekg(CentralDirZipped(CurrentZipFile, realBeginOfFile, File.getCompSize())))
        {
            File.close();
            return false;
        }

	    // Check every headers within the zip file
	    file_header FileHdr;

	    while ((NextHeader(CurrentZipFile) == FILE) && (FileHdr.ReadHeader(CurrentZipFile))) {

		    // Include files into Files map
		    const char * Name = &(* FileHdr.m_Filename.begin());
		    if (FileHdr.m_FilenameSize != 0) {

                // The zip in zip method only supports stored Zips because of JFileSystem limitations
                if ((FileHdr.m_UncompSize != FileHdr.m_CompSize) || FileHdr.m_CompMethod != STORED)
                    continue;

			    target[Name] = limited_file_info(
				    realBeginOfFile + FileHdr.m_RelOffset,					// "Local File" header offset position
				    FileHdr.m_UncompSize					// File Size
			    );
		    }
	    }

	    File.close();
        return (target.size() ? true : false);
    }
    else
    {
	    if (! File.seekg(CentralDir(File)))
        {
            File.close();
            return false;
        }

	    // Check every headers within the zip file
	    file_header FileHdr;

	    while ((NextHeader(File) == FILE) && (FileHdr.ReadHeader(File))) {

		    // Include files into Files map
		    const char * Name = &(* FileHdr.m_Filename.begin());
		    if (FileHdr.m_FilenameSize != 0) {

                // The zip in zip method only supports stored Zips because of JFileSystem limitations
                if ((FileHdr.m_UncompSize != FileHdr.m_CompSize) || FileHdr.m_CompMethod != STORED)
                    continue;

			    target[Name] = limited_file_info(
				    FileHdr.m_RelOffset,					// "Local File" header offset position
				    FileHdr.m_UncompSize					// File Size
			    );
		    }
	    }

	    File.close();
        return (target.size() ? true : false);
    }


}



//////////////////////////////////////////////////////////////////////
// File System Friend Functions
//////////////////////////////////////////////////////////////////////

ostream & operator << (ostream & Out, const filesystem & FS)
{
	size_t NbFiles = 0;
	filesystem::zipfile_info AllZipsInfo;

	for (filesystem::zipmap_const_iterator It = FS.m_Zips.begin(); It != FS.m_Zips.end(); ++It) {

		const filesystem::zipfile_info & ZInfo = (* It).second;

		// Print zip filename
		Out << setiosflags(ios::left) << setw(32) << "-> \"" + ZInfo.m_Filename + "\"" << resetiosflags(ios::left);
		// Print number of entries found in this zip file
		Out << "  " << setw(5) << ZInfo.m_NbEntries << " files";
		// Print the uncompressed size of all included files
		Out << "  " << setw(7) << ZInfo.m_FilesSize / 1024 << " KB";
		// Print the compressed size of all these files
		Out << "  " << setw(7) << ZInfo.m_FilesCompSize / 1024 << " KB packed" << endl;

		++NbFiles;
		AllZipsInfo.m_NbEntries += ZInfo.m_NbEntries;
		AllZipsInfo.m_FilesSize += ZInfo.m_FilesSize;
		AllZipsInfo.m_FilesCompSize += ZInfo.m_FilesCompSize;
	}

	// Print the general info
	Out << "\nTotal:  ";
	Out << NbFiles << " packs   ";
	Out << AllZipsInfo.m_NbEntries << " files   ";
	Out << float(AllZipsInfo.m_FilesSize) / (1024 * 1024) << " MB   ";
	Out << float(AllZipsInfo.m_FilesCompSize) / (1024 * 1024) << " MB packed." << endl;

	return Out;
}



//////////////////////////////////////////////////////////////////////
// "Less Than" Comparaison lt_path_str Member Functions
//////////////////////////////////////////////////////////////////////

bool filesystem::lt_path::operator () (const string & s1, const  string & s2) const
{
	const char * A = s1.c_str();
	const char * B = s2.c_str();

	for (size_t i = 0; ; ++i) {

		if ((A[i] == '\0') && (B[i] == '\0'))
			return false;

        // '/' is the same as '\'
		if (! (
				(A[i] == B[i])						||
				((A[i] == '\\') && (B[i] == '/'))	||
				((A[i] == '/') && (B[i] == '\\'))
			)) {
// This line puts uppercases first
			if ((A[i] == '\0') || (A[i] < B[i]))
				return true;
			else
				return false;
		}
	}
}



//////////////////////////////////////////////////////////////////////
// Zip Header Classes Related Member Functions
//////////////////////////////////////////////////////////////////////


streamoff filesystem::CentralDirZipped(std::istream & File, std::streamoff begin, std::size_t size) const
{
	using io_facilities::readvar;

    std::streamoff eof = begin + size;

	// Look for the "end of central dir" header. Start minimum 22 bytes before end.
    if (! File.seekg(eof - 22, ios::beg))
        return -1;

	streamoff EndPos;
	streamoff StartPos = File.tellg();

	if (StartPos == streamoff(0))
        return -1;

	if (StartPos <= begin + streamoff(65536))
		EndPos = 1;
	else
		EndPos = StartPos - streamoff(65536);

	// Start the scan
	do {
		unsigned int RawSignature;

		if (! readvar(File, RawSignature, 4))
            return -1;

		eofcd_header Header;
		streampos Pos = File.tellg();

		// Found a potential "eofcd" header?
		if ((RawSignature == ENDOFDIR) && (File.seekg(-4, ios::cur)) && (Header.ReadHeader(File))) {

			// Check invariant values (1 disk only)
			if ((Header.m_NbDisks == 0) && (0 == Header.m_DirDisk) && (Header.m_LocalEntries == Header.m_TotalEntries)) {

				// Check comment ends at eof
				if (! File.seekg(eof - 1 , ios::beg))
                    return -1;
				if ((File.tellg() + streamoff(1)) == (Pos + streamoff(Header.m_CommentSize + 22 - 4))) {

					// Check the start offset leads to a correct directory/file header;
					if (! File.seekg(begin + Header.m_Offset)) return -1;
					if (! readvar(File, RawSignature, 4)) return -1;
					if (RawSignature == FILE)
						return begin + Header.m_Offset;
				}
			}
		}

		File.seekg(Pos);

	} while ((File.seekg(-5, ios::cur)) && (File.tellg() > EndPos) && (File.tellg() > begin));

    return -1;
}

streamoff filesystem::CentralDir(istream & File) const
{
	using io_facilities::readvar;

	// Look for the "end of central dir" header. Start minimum 22 bytes before end.
	if (! File.seekg(-22, ios::end)) return -1;

	streamoff EndPos;
	streamoff StartPos = File.tellg();

	if (StartPos == streamoff(0)) return -1;

	if (StartPos <= streamoff(65536))
		EndPos = 1;
	else
		EndPos = StartPos - streamoff(65536);

	// Start the scan
	do {
		unsigned int RawSignature;

		if (! readvar(File, RawSignature, 4)) return -1;

		eofcd_header Header;
		streampos Pos = File.tellg();

		// Found a potential "eofcd" header?
		if ((RawSignature == ENDOFDIR) && (File.seekg(-4, ios::cur)) && (Header.ReadHeader(File))) {

			// Check invariant values (1 disk only)
			if ((Header.m_NbDisks == 0) && (0 == Header.m_DirDisk) && (Header.m_LocalEntries == Header.m_TotalEntries)) {

				// Check comment ends at eof
				if (! File.seekg(-1, ios::end)) return -1;
				if ((File.tellg() + streamoff(1)) == (Pos + streamoff(Header.m_CommentSize + 22 - 4))) {

					// Check the start offset leads to a correct directory/file header;
					if (! File.seekg(Header.m_Offset)) return -1;
					if (! readvar(File, RawSignature, 4)) return -1;
					if (RawSignature == FILE)
						return Header.m_Offset;
				}
			}
		}

		File.seekg(Pos);

	} while ((File.seekg(-5, ios::cur)) && (File.tellg() > EndPos));

    return -1;
}



streamoff filesystem::SkipLFHdr(istream & File, streamoff LFHdrPos)
{
	using io_facilities::readvar;

	unsigned short NameSize;
	unsigned short FieldSize;
	unsigned int RawSignature;

	// verify it's a local header
	if (! File.seekg(LFHdrPos)) return -1;
	if (! readvar(File, RawSignature, 4)) return -1;
	if (RawSignature != LOCALFILE) return -1;

	// Skip and go directly to comment/field size
	if (! File.seekg(22, ios::cur)) return -1;
	if (! readvar(File, NameSize, 2)) return -1;
	if (! readvar(File, FieldSize, 2)) return -1;

	// Skip comment and extra field
	if (! File.seekg(NameSize + FieldSize, ios::cur)) return -1;

	// Now we are at the compressed data position
	return (File.tellg());
}



headerid filesystem::NextHeader(istream & File) const
{
	using io_facilities::readvar;

	unsigned int RawSignature;

	if (! readvar(File, RawSignature, 4))
		return READERROR;

	if (! File.seekg(-4, ios::cur))
		return READERROR;

	headerid Signature = headerid(RawSignature);

	switch (Signature) {
	case FILE:
	case LOCALFILE:
	case ENDOFDIR:
		return Signature;
	default:
		return UNKNOWN;
	}
}




} // namespace zip_file_system
