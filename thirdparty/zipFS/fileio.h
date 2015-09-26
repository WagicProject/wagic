// bfileio.h: interface for the binary file i/o.
//
//////////////////////////////////////////////////////////////////////
//
//   Copyright (C) 2004 Tanguy Fautre
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
//  Tanguy FautrÅE
//  softdev@telenet.be
//
//////////////////////////////////////////////////////////////////////
//
//							File I/O Facilities.
//							********************
//
// Current version: 1.00 BETA 4 (16/07/2004)
//
// Comment: readvar() and writevar() read a little endian ordered
//          value on a file and put it in a variable.
//          search_iterator only accepts "<directory>/*.<ext>".
//          Uses ANSI C "assert()". Define NDEBUG to turn it off.
//          (note: Visual C++ define NDEBUG in Release mode)
//
// History: - 1.00 BETA 4 (16/07/2004) - Fixed small bug in UNIX search_iterator
//          - 1.00 BETA 3 (27/06/2004) - Added UNIX compatibility
//          - 1.00 BETA 2 (21/02/2003) - Now endianess independent
//          - 1.00 BETA 1 (06/09/2002) - First public release
//
//////////////////////////////////////////////////////////////////////

#pragma once 



#if defined WIN32
#include <io.h> // Windows I/O facilities (Directories)
#else
#include <dirent.h>
#include <string.h>
#endif

#include <limits.h>

namespace io_facilities {




// Global function for reading binary variables
template <class T> std::istream & readvar(std::istream & File, T & Var, const std::streamsize NbBytes);
template <class T> std::ostream & writevar(std::ostream & File, const T & Var, const std::streamsize NbBytes);



// Class for searching files and directories
// (!!! not compliant with C++ std::iterator and is thus meant for specific use !!!)
class search_iterator
{
public:
	search_iterator();
	search_iterator(const char * FileSpec);
	~search_iterator();

	operator bool () const;
	search_iterator & operator ++ ();
	search_iterator & begin(const char * FileSpec);
	search_iterator & next();
	bool end() const;
	std::string Name() const;

protected:
	bool		m_Valid;

#if defined WIN32
	intptr_t	m_hFiles;
	_finddata_t	m_FindData;
#else 
	DIR *			m_Directory;
	std::string		m_Extension;
	struct dirent *	m_DirectoryEntry;
#endif
};




//////////////////////////////////////////////////////////////////////
// io_facilities:: Inline Functions
//////////////////////////////////////////////////////////////////////

template <class T> 
inline std::istream & readvar(std::istream & File, T & Var, const std::streamsize NbBytes)
{ 
	// Debug test to ensure type size is big enough
	assert(sizeof(T) >= size_t(NbBytes));

	// Var = 0 ensure type size won't matter
	T TmpVar = Var = 0;

	for (std::streamsize i = 0; i < NbBytes; ++i) {
		File.read(reinterpret_cast<char *>(&TmpVar), 1);
		Var |= TmpVar << (i * CHAR_BIT);
	}

	return File;
}



template <class T> 
inline std::ostream & writevar(std::ostream & File, const T & Var, const std::streamsize NbBytes)
{ 
	// Debug test to ensure type size is big enough
	assert(sizeof(T) >= size_t(NbBytes));

	T TmpVar = Var;

	for (std::streamsize i = 0; i < NbBytes; ++i)
		File.write(reinterpret_cast<const char *>(&(TmpVar >>= (CHAR_BIT * i))), 1);
	
	return File;
}



//////////////////////////////////////////////////////////////////////
// io_facilities::search_iterator Inline Member Functions
//////////////////////////////////////////////////////////////////////

inline search_iterator::search_iterator()
	: m_Valid(false),
#if defined WIN32
	  m_hFiles(-1)
#else
	  m_Directory(NULL)
#endif
	{ }

inline search_iterator::search_iterator(const char * FileSpec)
	: m_Valid(false),
#if defined WIN32
	  m_hFiles(-1)
#else
	  m_Directory(NULL)
#endif
{
	begin(FileSpec);
}

inline search_iterator::~search_iterator() {
#if defined WIN32
	if (m_hFiles != -1) _findclose(m_hFiles);
#else
	if (m_Directory != NULL) closedir(m_Directory);
#endif
}

inline search_iterator::operator bool () const {
	return m_Valid;
}

inline search_iterator & search_iterator::operator ++ () {
	return next();
}

inline search_iterator & search_iterator::begin(const char * FileSpec) {
#if defined WIN32
	if (m_hFiles != -1) _findclose(m_hFiles);
	m_Valid = ((m_hFiles = _findfirst(FileSpec, &m_FindData)) != -1);
#else
	std::string DirectoryName;
	
	if (m_Directory != NULL) closedir(m_Directory);

	int i;
	for (i = strlen(FileSpec); i >= 0; --i)
		if (FileSpec[i] == '/') break;
	
	if (i < 0)
		DirectoryName = ".";
	else
		DirectoryName.assign(FileSpec + 0, FileSpec + i++);

	m_Extension = FileSpec + i + 1;
    std::transform(m_Extension.begin(), m_Extension.end(), m_Extension.begin(), ::tolower);
	m_Valid = ((m_Directory = opendir(DirectoryName.c_str())) != NULL);
	
	if (! m_Valid)
		return (* this);
	
	next();
#endif
	
	return (* this);
}

inline bool search_iterator::end() const {
	return false;
}

inline search_iterator & search_iterator::next() {
#if defined WIN32
	m_Valid = (_findnext(m_hFiles, &m_FindData) != -1);
#else
	bool Found = false;
	while (! Found) {
		m_Valid = ((m_DirectoryEntry = readdir(m_Directory)) != NULL);
		if (m_Valid) {
			std::string FileName = m_DirectoryEntry->d_name;
			if (FileName[0] == '.')
				Found = false;
			else if (FileName.size() <= m_Extension.size())
				Found = false;
			else {
                std::transform(FileName.begin(), FileName.end(), FileName.begin(), ::tolower);
                if (std::equal(m_Extension.rbegin(), m_Extension.rend(), FileName.rbegin()))
				    Found = true;
            }
		}
		else
			break;
	}
#endif
	
	return (* this);
}

inline std::string search_iterator::Name() const {
#if defined WIN32
	return (m_FindData.name);
#else
	return (m_DirectoryEntry->d_name);
#endif
}




} // namespace io_facilities


