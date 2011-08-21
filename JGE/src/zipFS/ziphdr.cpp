// ziphdr.cpp: implementation of the zip header classes.
//
// Copyright (C) 2002 Tanguy Fautré.
// For conditions of distribution and use,
// see copyright notice in ziphdr.h
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ziphdr.h"

#include "fileio.h"		// I/O facilities



namespace zip_file_system {




//////////////////////////////////////////////////////////////////////
// Zip Header Classes Member Functions
//////////////////////////////////////////////////////////////////////

bool local_file_header::ReadHeader(std::istream & File)
{
	using io_facilities::readvar;

	// quick check about char size
	//static_assert(CHAR_BIT == 8);

	if (! readvar(File, m_Signature, 4)) return false;
	if (! readvar(File, m_VersionExtract, 2)) return false;
	if (! readvar(File, m_GeneralFlags, 2)) return false;
	if (! readvar(File, m_CompMethod, 2)) return false;
	if (! readvar(File, m_Time, 2)) return false;
	if (! readvar(File, m_Date, 2)) return false;
	if (! readvar(File, m_CRC32, 4)) return false;
	if (! readvar(File, m_CompSize, 4)) return false;
	if (! readvar(File, m_UncompSize, 4)) return false;
	if (! readvar(File, m_FilenameSize, 2)) return false;
	if (! readvar(File, m_FieldSize, 2)) return false;

	m_Filename.resize(m_FilenameSize + 1);
	m_ExtraField.resize(m_FieldSize + 1);

	if (! File.read(&(m_Filename[0]), m_FilenameSize)) return false;
	if (! File.read(&(m_ExtraField[0]), m_FieldSize)) return false;

	m_Filename[m_FilenameSize] = '\0';
	m_ExtraField[m_FieldSize] = '\0';

	return true;
}



bool file_header::ReadHeader(std::istream & File)
{
	using io_facilities::readvar;

	if (! readvar(File, m_Signature, 4)) return false;
	if (! readvar(File, m_VersionMade, 2)) return false;
	if (! readvar(File, m_VersionExtract, 2)) return false;
	if (! readvar(File, m_GeneralFlags, 2)) return false;
	if (! readvar(File, m_CompMethod, 2)) return false;
	if (! readvar(File, m_Time, 2)) return false;
	if (! readvar(File, m_Date, 2)) return false;
	if (! readvar(File, m_CRC32, 4)) return false;
	if (! readvar(File, m_CompSize, 4)) return false;
	if (! readvar(File, m_UncompSize, 4)) return false;
	if (! readvar(File, m_FilenameSize, 2)) return false;
	if (! readvar(File, m_FieldSize, 2)) return false;
	if (! readvar(File, m_CommentSize, 2)) return false;
	if (! readvar(File, m_DiskNb, 2)) return false;
	if (! readvar(File, m_IntAttrib, 2)) return false;
	if (! readvar(File, m_ExtAttrib, 4)) return false;
	if (! readvar(File, m_RelOffset, 4)) return false;

	m_Filename.resize(m_FilenameSize + 1);
	m_ExtraField.resize(m_FieldSize + 1);
	m_Comment.resize(m_CommentSize + 1);

	if (! File.read(&(m_Filename[0]), m_FilenameSize)) return false;
	if (! File.read(&(m_ExtraField[0]), m_FieldSize)) return false;
	if (! File.read(&(m_Comment[0]), m_CommentSize)) return false;

	m_Filename[m_FilenameSize] = '\0';
	m_ExtraField[m_FieldSize] = '\0';
	m_Comment[m_CommentSize] = '\0';

	return true;
}



bool eofcd_header::ReadHeader(std::istream & File)
{
	using io_facilities::readvar;

	if (! readvar(File, m_Signature, 4)) return false;
	if (! readvar(File, m_NbDisks, 2)) return false;
	if (! readvar(File, m_DirDisk, 2)) return false;
	if (! readvar(File, m_LocalEntries, 2)) return false;
	if (! readvar(File, m_TotalEntries, 2)) return false;
	if (! readvar(File, m_DirSize, 4)) return false;
	if (! readvar(File, m_Offset, 4)) return false;
	if (! readvar(File, m_CommentSize, 2)) return false;

	m_Comment.resize(m_CommentSize + 1);

	if (! File.read(&(m_Comment[0]), m_CommentSize)) return false;

	m_Comment[m_CommentSize] = '\0';

	return true;
}




} // namespace zip_file_system

