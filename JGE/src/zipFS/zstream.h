// zstream.h: interface for the zstream classes.
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
//						Zip (Input) Stream.
//						*******************
//
// Current version: 1.00 BETA 4 (02/09/2003)
//
// Comment: izstream currently only supports "stored" and "deflated"
//          compression methods.
//
//          !!!IMPORTANT!!!
//          Modify "zstream_zlib.h" for headers and lib dependencies
//          on Zlib (http://www.zlib.org)
//
// History: - 1.00 BETA 4 (02/09/2003) - Made zbuffer constructor protected
//          - 1.00 BETA 3 (21/02/2003) - Fixed bugs with seekoff()
//          - 1.00 BETA 2 (23/12/2002) - Fixed a bug with izstream
//                                       (Added m_ComMethod(-1) in constructor)
//          - 1.00 BETA 1 (29/05/2002) - First public release
//
//////////////////////////////////////////////////////////////////////

#pragma once



#include "zstream_zlib.h" // Zlib dependencies



// Zip File System Namespace
namespace zip_file_system {




// Base buffer class
class zbuffer : public std::streambuf
{
public:
	virtual ~zbuffer() { }

	virtual zbuffer * open(const char * Filename, std::streamoff Offset, std::streamoff Size) = 0;
	virtual zbuffer * close() = 0;

	bool is_open() const	{ return m_Opened; }

protected:
	zbuffer() : m_Size(0), m_Opened(false) { }

	static const int BUFFERSIZE = 4092;

	std::ifstream	m_ZipFile;
	std::streamoff	m_Pos;
	std::streamoff	m_Size;
	char			m_Buffer[BUFFERSIZE];
	bool			m_Opened;
};



// Buffer class for stored compression method.
class zbuffer_stored : public zbuffer
{
public:
	virtual ~zbuffer_stored() { }

    virtual zbuffer_stored * open(const char * Filename, std::streamoff Offset, std::streamoff Size);
    virtual zbuffer_stored * close();

 	virtual int overflow(int c = EOF);
	virtual int underflow();
	virtual int	sync();
	virtual std::streambuf * setbuf(char * pr, int nLength);
	virtual std::streampos seekoff(std::streamoff, std::ios::seekdir, std::ios::openmode);

	//	Default Implementation is enough
	//	virtual streampos seekpos(streampos, int);
};




// Buffer class for deflated compression method.
class zbuffer_deflated : public zbuffer
{
public:

	virtual ~zbuffer_deflated() {	
		if (m_Opened)
			inflateEnd(&m_ZStream);
	}

    virtual zbuffer_deflated * open(const char * Filename, std::streamoff Offset, std::streamoff Size);
    virtual zbuffer_deflated * close();

 	virtual int overflow(int c = EOF);
	virtual int underflow();
	virtual int sync();
	virtual std::streambuf * setbuf(char * pr, int nLength);
	virtual std::streampos seekoff(std::streamoff, std::ios::seekdir, std::ios::openmode);

	//	Default Implementation is enough
	//	virtual streampos seekpos(streampos, int);
	
protected:
	z_stream		m_ZStream;
	std::streamoff	m_CompPos;
	char			m_CompBuffer[BUFFERSIZE];
	bool			m_StreamEnd;
};




// main istream class for reading zipped files
class izstream : public std::istream
{
public:

	izstream() : std::istream(NULL), m_CompMethod(-1) { setstate(std::ios::badbit); }
	virtual ~izstream()					{ delete rdbuf(); }

	void open(const char * Filename, std::streamoff Offset, std::streamoff Size, int CompMethod);
	void close()						{ SetCompMethod(-1); }

protected:
	static const int STORED = 0;
	static const int DEFLATED = 8;

	zbuffer * GetRightBuffer(int CompMethod) const;

	void SetCompMethod(int CompMethod) {
        delete rdbuf(GetRightBuffer(m_CompMethod = CompMethod));

		if (rdbuf() == NULL)
			setstate(std::ios::badbit);
	}

	int	m_CompMethod;
};




} // namespace zip_file_system;

