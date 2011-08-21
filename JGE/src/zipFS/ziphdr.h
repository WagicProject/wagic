// zfsystem.h: interface for the zip header classes.
//
//////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2002 Tanguy Fautré.
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
//							Zip File Format Headers.
//							***********************
//
// Current version: 1.00 BETA 2 (01/09/2003)
//
// Comment: Based on the ZIP file format specification from Appnote.txt
//          from the PKZip Website on July 13, 1998.
//          New implementations of the ZIP file format might not work
//          correctly (ZIP64 ?).
// 
// History: - 1.00 BETA 2 (01/09/2003) - Use stdint.h sized types
//          - 1.00 BETA 1 (12/06/2002) - First public release
//
//////////////////////////////////////////////////////////////////////

#pragma once



// Zip File System Namespace
namespace zip_file_system {




// Zip file headers
enum headerid {	LOCALFILE	= 0x04034b50,
				FILE		= 0x02014b50,
				ENDOFDIR	= 0x06054b50, 
				UNKNOWN, 
				READERROR
};



// Zip file "local file" header class
struct local_file_header
{
	bool ReadHeader(std::istream & File);

	static const uint_least32_t m_ConstSign= LOCALFILE;

	uint_least32_t		m_Signature;
	uint_least16_t		m_VersionExtract;
	uint_least16_t		m_GeneralFlags;
	uint_least16_t		m_CompMethod;
	uint_least16_t		m_Time;
	uint_least16_t		m_Date;
	uint_least32_t		m_CRC32;
	uint_least32_t		m_CompSize;
	uint_least32_t		m_UncompSize;
	uint_least16_t		m_FilenameSize;
	uint_least16_t		m_FieldSize;

	std::vector<char>	m_Filename;
	std::vector<char>	m_ExtraField;
};



// Zip file "file header" header class
struct file_header
{
	bool ReadHeader(std::istream & File);

	static const headerid m_ConstSign = FILE;

	uint_least32_t		m_Signature;
	uint_least16_t		m_VersionMade;
	uint_least16_t		m_VersionExtract;
	uint_least16_t		m_GeneralFlags;
	uint_least16_t		m_CompMethod;
	uint_least16_t		m_Time;
	uint_least16_t		m_Date;
	uint_least32_t		m_CRC32;
	uint_least32_t		m_CompSize;
	uint_least32_t		m_UncompSize;
	uint_least16_t		m_FilenameSize;
	uint_least16_t		m_FieldSize;
	uint_least16_t		m_CommentSize;
	uint_least16_t		m_DiskNb;
	uint_least16_t		m_IntAttrib;
	uint_least32_t		m_ExtAttrib;
	uint_least32_t		m_RelOffset;

	std::vector<char>	m_Filename;
	std::vector<char>	m_ExtraField;
	std::vector<char>	m_Comment;
};



// Zip file "end of central dir" header class
struct eofcd_header
{
	bool ReadHeader(std::istream & File);

	static const headerid m_ConstSign = ENDOFDIR;

	uint_least32_t		m_Signature;
	uint_least16_t		m_NbDisks;
	uint_least16_t		m_DirDisk;
	uint_least16_t		m_LocalEntries;
	uint_least16_t		m_TotalEntries;
	uint_least32_t		m_DirSize;
	uint_least32_t		m_Offset;
	uint_least16_t		m_CommentSize;

	std::vector<char>	m_Comment;
};




} // namespace zip_file_system

