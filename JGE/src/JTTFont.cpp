//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../include/JFileSystem.h"
#include "../include/JTTFont.h"
#include "../include/Encoding.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef WIN32
        #pragma comment( lib, "freetype.lib" )
#endif

#include "../include/JGE.h"
#include "../include/JRenderer.h"


#ifndef WIN32

// in JGBKFont.cpp
extern void SwizzlePlot(u8* out, PIXEL_TYPE color, int i, int j, unsigned int width);

#endif


JTTFont::JTTFont(int cacheImageSize)
{

  mColor = ARGB(255,255,255,255);
        mSize = 0;
	mAngle = 0.0;
	mLibrary = 0;
	mFace = 0;

	mFontLoaded = false;
	mSharingFont = false;
	mAntialias = true;

	mTexture = NULL;

	switch (cacheImageSize)
	{
	case CACHE_IMAGE_64x64:
		mTexWidth = 64;
		mTexHeight = 64;
		break;
	case CACHE_IMAGE_128x128:
		mTexWidth = 128;
		mTexHeight = 128;
		break;
	case CACHE_IMAGE_512x512:
		mTexWidth = 512;
		mTexHeight = 512;
		break;
	default:
		mTexWidth = 256;
		mTexHeight = 256;
		break;

	}

	mTexture = JRenderer::GetInstance()->CreateTexture(mTexWidth, mTexHeight);

	for (int i=0;i<TTF_CACHE_SIZE;i++)
	{
		mQuads[i] = new JQuad(mTexture, 0, 0, 16, 16);
		mCachedCode[i] = 0;
		mXAdvance[i] = 0;
	}

	mASCIIDirectMapping = false;

}

JTTFont::~JTTFont()
{

	SAFE_DELETE(mTexture);
	
	for (int i=0;i<TTF_CACHE_SIZE;i++)
		SAFE_DELETE(mQuads[i]);

}

void JTTFont::SetColor(PIXEL_TYPE color)
{
	mColor = color;
}


void JTTFont::SetAngle(float angle)
{
	mAngle = angle;

}


bool JTTFont::SetSize(int size)
{

	if(!mFace)
		return false;

	if (mSize == size)
		return true;

	// size is in 26.6 fixed point formant!
	if (FT_Set_Pixel_Sizes(mFace, size, size) == 0)
	{
		mSize = size;
		mCurr = 0;

		mMaxCharHeight = size+6;
		mMaxCharWidth = mMaxCharHeight;// + size/2 + 4;

		mColCount = (mTexWidth/mMaxCharWidth);
		mRowCount = (mTexHeight/mMaxCharHeight);

		mMaxCharCount = mColCount*mRowCount;
		if (mMaxCharCount > TTF_CACHE_SIZE)
			mMaxCharCount = TTF_CACHE_SIZE;


		FT_Set_Transform(mFace, 0, 0);

		// JTTFont.h says setting font size will clear the cache
		for (int i = 0; i < TTF_CACHE_SIZE; i++)
			mCachedCode[i] = 0;

		return true;
	}
	else
		return false;
}


FT_Library JTTFont::GetFontLibrary()
{
	return mLibrary;
}


FT_Byte* JTTFont::GetFontBits()
{
	return mFontBits;
}


int JTTFont::GetFontBitsSize()
{
	return mFontBitsSize;
}


bool JTTFont::Load(JTTFont* fontSource, int size, int mode)
{
	mLibrary = fontSource->GetFontLibrary();
	if (mLibrary)
	{
		mFontBits = fontSource->GetFontBits();
		mFontBitsSize = fontSource->GetFontBitsSize();

		if (mFontBits && FT_New_Memory_Face(mLibrary, mFontBits, mFontBitsSize, 0, &mFace ) == 0)
		{
			mSharingFont = true;
			mFontSource = fontSource;

			SetSize(size);

			if (mode == MODE_PRECACHE_ASCII)
				PreCacheASCII();
			else if (mode == MODE_PRECACHE_ASCII_EX)
				PreCacheExtendedASCII();

			mFontLoaded = true;

			return true;
		}
	}

	return false;
}


bool JTTFont::Load(const char *filename, int size, int mode)
{
	
	if (FT_Init_FreeType( &mLibrary ) == 0)
	{
		JFileSystem* fileSystem = JFileSystem::GetInstance();
		if (fileSystem->OpenFile(filename))
		{
			mFontBitsSize = fileSystem->GetFileSize();

			mFontBits = (FT_Byte*)malloc(mFontBitsSize);
			
			fileSystem->ReadFile(mFontBits, mFontBitsSize);
			fileSystem->CloseFile();

			if (FT_New_Memory_Face(mLibrary, mFontBits, mFontBitsSize, 0, &mFace ) == 0)
			{
				SetSize(size);
				mFontLoaded = true;

				if (mode == MODE_PRECACHE_ASCII)
					return PreCacheASCII();
				else if (mode == MODE_PRECACHE_ASCII_EX)
					return PreCacheExtendedASCII();


				return true;
			}
		}
		
	}

	return false;
}


void JTTFont::Unload(void)
{
	FT_Done_Face(mFace);
	mFace = 0;
	mFontLoaded = false;

	if (!mSharingFont)
	{
		FT_Done_FreeType(mLibrary);
		mLibrary = 0;
		free(mFontBits);
	}
}


int JTTFont::PreCacheChar(u16 ch, u16 cachedCode)
{
	for (int i=0;i<mMaxCharCount&&mCachedCode[i];i++)
	{
		if (mCachedCode[i] == cachedCode)
			return i;
	}

	if (mASCIIDirectMapping)
	{
		mASCIIDirectMapping = false;
		mCurr = 0;
	}

	if (!mFontLoaded) return -1;

	if (mSharingFont && mFontSource->GetFontLibrary() == NULL)
		return -1;

	FT_GlyphSlot slot = mFace->glyph;

	#if defined (WIN32) || defined (LINUX) || defined (IOS)
		DWORD *texBuffer = new DWORD[mMaxCharWidth*mMaxCharHeight];
		memset(texBuffer, 0, mMaxCharWidth*mMaxCharHeight*sizeof(DWORD));
	#else

		u8* pTexture = (u8*) mTexture->mBits;

	#endif

	int y = (mCurr/mColCount)*mMaxCharHeight;
	int x = (mCurr%mColCount)*mMaxCharWidth;
	int ret = -1;

	int renderFlag = FT_LOAD_RENDER;
	if (!mAntialias)
		renderFlag |= FT_LOAD_TARGET_MONO;
	if (FT_Load_Char(mFace, ch, renderFlag) == 0)
	{
		int top = mSize-slot->bitmap_top+1;

		#if defined (WIN32) || defined (LINUX) || defined (IOS)
			int offset = top*mMaxCharWidth + slot->bitmap_left + 2;
		#else
			int xx = x + slot->bitmap_left + 2;
			int yy = y + top;

			for (int i=0;i<mMaxCharHeight;i++)
			{
				for (int j=0;j<mMaxCharWidth;j++)
				{
					SwizzlePlot(pTexture, ARGB(0,0,0,0), (x+j)*PIXEL_SIZE, y+i, mTexWidth*PIXEL_SIZE);
				}
			}
		#endif

		int rows = mMaxCharHeight - top - 1;
		if (mAntialias)
		{
			u8 grey;
			for (int i=0;i<slot->bitmap.rows&&rows>=0;i++)
			{
				for (int j=0;j<slot->bitmap.width;j++)
				{
					grey = slot->bitmap.buffer[i * slot->bitmap.width + j];

					#if defined (WIN32) || defined (LINUX) || defined (IOS)
						texBuffer[i*mMaxCharWidth+j+offset] = RGBA(255, 255, 255, grey);
					#else
						SwizzlePlot(pTexture, ARGB(grey,255,255,255), (xx+j)*PIXEL_SIZE, yy+i, mTexWidth*PIXEL_SIZE);
					#endif
				}
				rows--;

			}
		}
		else
		{
			u8 bits, mask;
			for (int i=0;i<slot->bitmap.rows&&rows>=0;i++)
			{
				for (int j=0;j<slot->bitmap.pitch;j++)
				{
					bits = slot->bitmap.buffer[i * slot->bitmap.pitch + j];
					mask = 0x80;
					for (int k=0;k<8;k++)
					{
						if (bits&mask)
						{
							#if defined (WIN32) || defined (LINUX) || defined (IOS)
							texBuffer[i*mMaxCharWidth+j*8+k+offset] = RGBA(255, 255, 255, 255);
							#else
							SwizzlePlot(pTexture, ARGB(255,255,255,255), (xx+j*8+k)*PIXEL_SIZE, yy+i, mTexWidth*PIXEL_SIZE);
							#endif
						}

						mask>>=1;
					}
				}
				rows--;

			}
		}
	}

	mXAdvance[mCurr] = (u8)(slot->advance.x>>6);

	#if defined (WIN32) || defined (LINUX) || defined (IOS)
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, mMaxCharWidth, mMaxCharHeight, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer);
	#else
		sceKernelDcacheWritebackAll();
	#endif

	mCachedCode[mCurr] = cachedCode;
	ret = mCurr;

	mQuads[mCurr++]->SetTextureRect((float)(x+2), (float)(y+1), (float)(slot->bitmap_left+slot->bitmap.width), (float)mMaxCharHeight-1);

	if (mCurr >= mMaxCharCount)
		mCurr = 0;

	#if defined (WIN32) || defined (LINUX)
		delete [] texBuffer;
	#endif
	
	return ret;
}


int JTTFont::GetCachedChar(u16 cachedCode)
{
	for (int i=0;i<mMaxCharCount&&mCachedCode[i];i++)
	{
		if (mCachedCode[i] == cachedCode)
			return i;
	}

	return -1;
}


void JTTFont::RenderString(const u16 *text, float x, float y, int alignment)
{
	if (alignment == JGETEXT_LEFT)
		RenderString(text, x, y, true);
	else if (alignment == JGETEXT_RIGHT)
	{
		int width = RenderString(text, 0, 0, false);
		RenderString(text, x-width, y, true);
	}
	else if (alignment == JGETEXT_CENTER)
	{
		int width = RenderString(text, 0, 0, false);
		RenderString(text, x-(width/2), y, true);
	}

}


void JTTFont::RenderString(const char *text, float x, float y, int alignment)
{
	if (alignment == JGETEXT_LEFT)
		RenderString(text, x, y, true);
	else if (alignment == JGETEXT_RIGHT)
	{
		int width = RenderString(text, 0, 0, false);
		RenderString(text, x-width, y, true);
	}
	else if (alignment == JGETEXT_CENTER)
	{
		int width = RenderString(text, 0, 0, false);
		RenderString(text, x-(width/2), y, true);
	}
}


void JTTFont::RenderString(const u8 *text, float x, float y, int alignment)
{
	if (alignment == JGETEXT_LEFT)
		RenderString(text, x, y, true);
	else if (alignment == JGETEXT_RIGHT)
	{
		int width = RenderString(text, 0, 0, false);
		RenderString(text, x-width, y, true);
	}
	else if (alignment == JGETEXT_CENTER)
	{
		int width = RenderString(text, 0, 0, false);
		RenderString(text, x-(width/2), y, true);
	}
}


void JTTFont::PreCacheString(const u16 *text)
{
	RenderString(text, 0, 0, false);
}


void JTTFont::PreCacheString(const char *text)
{
	RenderString(text, 0, 0, false);
}


void JTTFont::PreCacheString(const u8 *text)
{
	RenderString(text, 0, 0, false);
}


int JTTFont::GetWidth(const u16 *text)
{
	return RenderString(text, 0, 0, false);
}


int JTTFont::GetWidth(const char *text)
{
	return RenderString(text, 0, 0, false);
}


int JTTFont::GetWidth(const u8 *text)
{
	return RenderString(text, 0, 0, false);
}


int JTTFont::RenderString(const u16 *text, float x, float y, bool render)
{

	JRenderer* renderer = JRenderer::GetInstance();
	renderer->BindTexture(mTexture);

	u16 ch;
	int index;

	while ((ch=*text++)!=0)
	{
		index = PreCacheChar(ch, ch);
		if (index != -1)
		{
			if (render)
			{
				mQuads[index]->SetColor(mColor);
				renderer->RenderQuad(mQuads[index], x, y);
			}
			x += mXAdvance[index];
		}
	}

	return (int)x;
}


int JTTFont::RenderString(const char *text, float x, float y, bool render)
{
	JRenderer* renderer = JRenderer::GetInstance();
	renderer->BindTexture(mTexture);

	const u8* str = (const u8*) text;
	u8 ch;
	int index;

	if (mASCIIDirectMapping)
	{
		while ((ch=*str++)!=0)
		{
			index = ch-32;
			if (render)
			{
				mQuads[index]->SetColor(mColor);
				renderer->RenderQuad(mQuads[index], x, y);
			}
			x += mXAdvance[index];
		}

	}
	else
	{
		while ((ch=*str++)!=0)
		{
			index = PreCacheChar(ch, ch);
			if (index != -1)
			{
				if (render)
				{
					mQuads[index]->SetColor(mColor);
					renderer->RenderQuad(mQuads[index], x, y);
				}
				x += mXAdvance[index];
			}
		}
	}

	return (int)x;
}


int JTTFont::RenderString(const u8 *text, float x, float y, bool render)
{
	JRenderer* renderer = JRenderer::GetInstance();
	renderer->BindTexture(mTexture);

	u8 ch;
	int index;

	while ((ch=*text)!=0)
	{
		if (ch < 0x80)
		{
			index = PreCacheChar(ch, ch);
			text++;
		}
		else
		{
			u8 b1 = *text;
			u8 b2 = *(text+1);
			u16 n = b2;
			n <<= 8;
			n |= b1;
			index = GetCachedChar(n);
			if (index == -1)
			{
				u16 unicode = charsets_gbk_to_ucs(text);
				index = PreCacheChar(unicode, n);			// use GBK code for caching
			}
			text += 2;
		}
		if (index != -1)
		{
			if (render)
			{
				mQuads[index]->SetColor(mColor);
				renderer->RenderQuad(mQuads[index], x, y);
			}
			x += mXAdvance[index];
		}
	}

	return (int)x;
}


bool JTTFont::PreCacheASCII()
{
	int count = 127-32+1;
	if (count > mMaxCharCount)
		count = mMaxCharCount;

	{
		int i = 32;
		mCurr = 0;
		for (int n=0;n<count;n++)
		{
			PreCacheChar(i, i);
			i++;
		}

		mASCIIDirectMapping = true;
		//return true;
	}

	return true;
}


bool JTTFont::PreCacheExtendedASCII()
{
	int count = 255-32+1;
	if (count > mMaxCharCount)
		count =  mMaxCharCount;

	{
		int i = 32;
		mCurr = 0;
		for (int n=0;n<=count;n++)
		{
			PreCacheChar(i, i);
			i++;
		}

		mASCIIDirectMapping = true;
		//return true;
	}

	return true;
}


void JTTFont::SetAntialias(bool flag)
{
	mAntialias = flag;
}
