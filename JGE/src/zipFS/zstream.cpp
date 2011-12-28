// zstream.cpp: implementation of the zstream class.
//
// Copyright (C) 2002 Tanguy Fautré.
// For conditions of distribution and use,
// see copyright notice in zfsystem.h
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "zstream.h"



namespace zip_file_system {

using namespace std;




//////////////////////////////////////////////////////////////////////
// zstream Member Functions
//////////////////////////////////////////////////////////////////////

/*
void izstream::open(const char * Filename, streamoff Offset, streamoff Size, int CompMethod)
{
	// Change the buffer if need
	if (m_CompMethod == CompMethod) {
		if (rdbuf() != NULL)
			static_cast<zbuffer *>(rdbuf())->close();
	} else
		SetCompMethod(CompMethod);

	// clear up the file status
	clear(ios::goodbit);

	// open the buffer
	switch (m_CompMethod) {
	case STORED:
	case DEFLATED:

		if (! (static_cast<zbuffer *>(rdbuf())->open(Filename, Offset, Size))) {
			setstate(ios::badbit);
			SetCompMethod(-1);
		}

		break;

	default:
		setstate(ios::badbit);
	}
}

zbuffer * izstream::GetRightBuffer(int CompMethod) const
{
	switch (CompMethod) {
	
	// stored
	case STORED:
		return new zbuffer_stored;

	// deflated
	case DEFLATED:
		return new zbuffer_deflated;

	// else not supported
	default:
		return NULL;
	}
}*/

bool zbuffer::use(std::streamoff Offset, std::streamoff Size)
{
	if (! m_ZipFile)
		return false;

    //Don't use a buffer already used;
    if (m_Used)
        return false;

	// adjust file position
	if (! m_ZipFile.seekg(Offset, ios::beg))
		return false;

	setg( m_Buffer,					// beginning of putback area
          m_Buffer,					// read position
          m_Buffer);		// end of buffer

    m_Buffer[0] = 0;

	m_Pos = -1;
	m_Size = Size;
    m_Used = true;

    return true;
}


//////////////////////////////////////////////////////////////////////
// zbuffer_stored Member Functions
//////////////////////////////////////////////////////////////////////

zbuffer_stored * zbuffer_stored::open(const char * Filename, streamoff Offset, streamoff Size)
{
	// open main compressed file
	m_ZipFile.open(Filename, ios::binary);
	if (! m_ZipFile)
		return NULL;

	// adjust file position
	if (! use(Offset, Size))
		return NULL;

	m_Opened = true;
    m_Filename = Filename;
	return this;
}



zbuffer_stored * zbuffer_stored::close()
{
	if (! m_Opened)
		return NULL;
	else {
		m_Opened = false;
        m_Used = false;
		m_ZipFile.close();
	}

	return this;
}



int zbuffer_stored::overflow(int c)
{
	return EOF;
}



int zbuffer_stored::underflow()
{
	// Buffer Valid?
	if (! m_Opened)
		return EOF;

	// Do we really need to refill it?
	if (gptr() < egptr())
		return static_cast<unsigned char>(* gptr());

	// Refill de buffer.
	// Set the real position of the beginning of the buffer.
	if (m_Pos == streamoff(-1))
		m_Pos = 0;

	streamoff ToRead = ((m_Size - m_Pos) < BUFFERSIZE) ? (m_Size - m_Pos) : BUFFERSIZE; 
	if ((ToRead == 0) || (! m_ZipFile.read(m_Buffer, ToRead)))
		return EOF;

    m_Pos += ToRead; 
	
	// Reset buffer pointers.
	setg( m_Buffer,					// beginning of putback area
          m_Buffer,					// read position
          m_Buffer + ToRead);		// end of buffer

	return static_cast<unsigned char>(m_Buffer[0]);
}



streampos zbuffer_stored::seekoff(streamoff off, ios::seekdir dir,  ios::openmode nMode)
{
	streamoff WantedPos = 0;

	// Find out the wanted position.
	switch (dir) {
	case ios_base::cur:
		WantedPos = m_Pos + streamoff(gptr() - eback()) + off;
		break;

	case ios_base::beg:
		WantedPos = off;
		break;

	case ios_base::end:
		WantedPos = m_Size + off;
		break;
		
	default:
		assert(false);
	}

	// Is the position valid?
	if ((WantedPos < 0) || (WantedPos > m_Size))
		return streambuf::seekoff(off, dir, nMode);		// return invalid streamoff

	// Is the position already within the buffer?
	if ((WantedPos >= m_Pos) && (WantedPos - m_Pos < egptr() - eback())) {
		setg(eback(), eback() + (WantedPos - m_Pos), egptr());
		return WantedPos;
	}

	// Fill up the buffer at the right position.
	if (! m_ZipFile.seekg(WantedPos, ios::beg))
		return streambuf::seekoff(off, dir, nMode);

	m_Pos = WantedPos;
	streamoff ToRead = ((m_Size - m_Pos) < BUFFERSIZE) ? (m_Size - m_Pos) : BUFFERSIZE;

	if (ToRead == 0)
		return WantedPos;
	if (! m_ZipFile.read(m_Buffer, ToRead))
		return streambuf::seekoff(off, dir, nMode);

 	// Set the buffer at the right position
	setg(m_Buffer, m_Buffer, m_Buffer + ToRead);

	return WantedPos;
}



int zbuffer_stored::sync()
{
	return 0;
}



streambuf * zbuffer_stored::setbuf(char * pr, int nLength)
{
	return NULL;
}



//////////////////////////////////////////////////////////////////////
// zbuffer_deflated Member Functions
//////////////////////////////////////////////////////////////////////

zbuffer_deflated * zbuffer_deflated::open(const char * Filename, streamoff Offset, streamoff Size)
{
	// open main compressed file
	m_ZipFile.open(Filename, ios::binary);
	if (! m_ZipFile)
		return NULL;

	// adjust file position
	if (! use(Offset, Size))
		return NULL;

	// z_stream (NULL) Initialization 
	m_ZStream.next_in = Z_NULL;
	m_ZStream.avail_in = 0;
	m_ZStream.total_in = 0;
	m_ZStream.next_out = Z_NULL;
	m_ZStream.avail_out = 0;
	m_ZStream.total_out = 0;
	m_ZStream.zalloc = Z_NULL;
	m_ZStream.zfree = Z_NULL;
	m_ZStream.opaque = Z_NULL;

	// inflate routine Initialization: Window Size = -MAX_WBITS tells there are no header
	if (inflateInit2(&m_ZStream, -MAX_WBITS) != Z_OK)
		return NULL;

	m_Opened = true;
	m_StreamEnd = false;
	m_Pos = 0;
	m_CompPos = 0;
    m_Filename = Filename;

	return this;
}



zbuffer_deflated * zbuffer_deflated::close()
{
	if (! m_Opened)
		return NULL;
	else {
		m_Opened = false;
        m_Used = false;
		m_ZipFile.close();

		// z_stream unitialization.
		if (inflateEnd(&m_ZStream) != Z_OK)
			return NULL;
	}

	return this;
}



int zbuffer_deflated::overflow(int c)
{
	return EOF;
}



int zbuffer_deflated::underflow()
{
	// Buffer Valid?
	if (! m_Opened)
		return EOF;

	// Do we really need to refill it?
	if (gptr() < egptr())
		return static_cast<unsigned char>(* gptr());
	
	// Can we refill?
	if (m_StreamEnd)
		return EOF;

	streamoff ToRead;
	streamoff OldPos;
	bool BufferRefill = false;
	
	// Check input (compressed) buffer status
	if ((m_ZStream.avail_in == 0) && (m_CompPos < m_Size )) {
		ToRead = ((m_Size - m_CompPos) > BUFFERSIZE) ? BUFFERSIZE : (m_Size - m_CompPos);
		m_CompPos += ToRead;

		if (! m_ZipFile.read(m_CompBuffer, ToRead))
			return EOF;

		m_ZStream.next_in = reinterpret_cast<unsigned char *>(m_CompBuffer);
		m_ZStream.avail_in = ToRead;
	}

	// Ajust start read position in output buffer at the "old" end of buffer
	ToRead = m_ZStream.total_out % BUFFERSIZE;
	OldPos = m_ZStream.total_out;

	// Check output (decompressed) buffer status
	if (m_ZStream.avail_out == 0) {
		BufferRefill = true;
		m_ZStream.next_out = reinterpret_cast<unsigned char *>(m_Buffer);
		m_ZStream.avail_out = BUFFERSIZE;
	}

	// Decompress (Inflate)
	int Result = inflate(&m_ZStream, Z_SYNC_FLUSH);

	// Check decompression result
	if (Result == Z_STREAM_END)
		m_StreamEnd = true;
	else if (Result != Z_OK)
		return EOF;

	// Set the real position of the beginning of the buffer.
	if (m_Pos == streamoff(-1))
		m_Pos = 0;
	else
		if (BufferRefill)
			m_Pos += m_ZStream.total_out - OldPos; 

	// Reset buffer pointers.
    setg( m_Buffer,														// beginning of putback area
          m_Buffer + ToRead,											// read position
          m_Buffer + ((m_ZStream.total_out - 1) % (BUFFERSIZE)) + 1);	// end of buffer

	return static_cast<unsigned char>(m_Buffer[ToRead]);
}



streampos zbuffer_deflated::seekoff(std::streamoff off, std::ios::seekdir dir, std::ios::openmode nMode)
{
	streamoff WantedPos = 0;

	// Find out the wanted position.
	switch (dir) {
	case ios_base::cur:
		WantedPos = m_Pos + streamoff(gptr() - eback()) + off;
		break;

	case ios_base::beg:
		WantedPos = off;
		break;

	case ios_base::end:
		WantedPos = m_Size + off;
		break;
	
	default:
		assert(false);
	}

	// Is the position valid?
	if ((WantedPos < 0) || (WantedPos > m_Size))
		return streambuf::seekoff(off, dir, nMode);		// return invalid streamoff

	// Is the position already within the buffer?
	if ((WantedPos >= m_Pos) && (WantedPos - m_Pos < egptr() - eback())) {
		setg(eback(), eback() + (WantedPos - m_Pos), egptr());
		return WantedPos;
	}

	// Found out whether we have to decompress further or if we have to reset the decompression.
	if (WantedPos < m_Pos) {

		// Reset the decompression.
		if (inflateReset(&m_ZStream) !=  Z_OK)
			return streambuf::seekoff(off, dir, nMode);

		// z_stream Reset 
		m_ZStream.next_in = Z_NULL;
		m_ZStream.avail_in = 0;
		m_ZStream.total_in = 0;
		m_ZStream.next_out = Z_NULL;
		m_ZStream.avail_out = 0;
		m_ZStream.total_out = 0;
	}

	// call underflow() untill the right position is within the buffer.
	while (WantedPos - m_Pos >= egptr() - eback()) {
		setg(eback(), egptr(), egptr());
		if (underflow() == EOF)
			return streambuf::seekoff(off, dir, nMode);
	}

	// now the position is within the buffer.
	setg(eback(), eback() + (WantedPos - m_Pos), egptr());

	return WantedPos;
}



int zbuffer_deflated::sync()
{
	return 0;
}



streambuf * zbuffer_deflated::setbuf(char * pr, int nLength)
{
	return NULL;
}




} // namespace zip_file_system
