//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#ifndef _JBGK_FONT_H_
#define _JBGK_FONT_H_


#include "JTypes.h"

#include "JRenderer.h"
#include "JSprite.h"

#define BYTE					u8
#define DWORD					u32
#define BOOL					int


// #define GB_FONT_SIZE			16
// #define GB_FONT_DATA_SIZE		GB_FONT_SIZE*GB_FONT_SIZE/8
// #define GB_FONT_BYTE_COUNT		GB_FONT_SIZE/8
// 
#define MAX_CACHE_SIZE		256
// 
// #define CFONT_TEX_WIDTH			256
// #define CFONT_TEX_HEIGHT		256

//////////////////////////////////////////////////////////////////////////
/// Chinese bitmap font encoded with GBK encoding. All popurlar font sizes
/// are supported and the following have been tested:
/// 12x12, 16x16, 18x18, 20x20, 24x24, 28x28 and 32x32.
/// 
//////////////////////////////////////////////////////////////////////////
class JGBKFont
{
public:

	//////////////////////////////////////////////////////////////////////////
	/// Constructor.
	///
	//////////////////////////////////////////////////////////////////////////
	JGBKFont();

	~JGBKFont();

	//////////////////////////////////////////////////////////////////////////
	/// Initialization of the font class. You need to provide both a Chinese 
	/// font file and an English one as well.
	/// 
	/// For example:
	/// @code
	///		mChineseFont = new JGBKFont();
	///		mChineseFont->Init("Res/ASC16", "Res/GBK16");
	/// @endcode
	/// 
	/// @param engFileName - Name of the English font file.
	/// @param chnFileName - Name of the Chinese font file.
	/// @param fontsize - Font size.
	/// @param smallEnglishFont - Indicate to use half width when rendering English characters.
	/// 
	//////////////////////////////////////////////////////////////////////////
	bool Init(const char* engFileName, const char* chnFileName, int fontsize=16, bool smallEnglishFont=false);

	//////////////////////////////////////////////////////////////////////////
	/// Rendering character into cache.
	/// 
	/// @param ch - Single byte or word of character code.
	/// 
	/// @return Index of the character in cache.
	/// 
	//////////////////////////////////////////////////////////////////////////
	int PreCacheChar(const BYTE *ch);

	//////////////////////////////////////////////////////////////////////////
	/// Scan through the string and look up the index of each character in the
	/// cache and then return all indexes in an array to be rendered later on.
	/// 
	/// @param str - String to look for cache indexes.
	/// @return dest - Indexes of characters in cache.
	/// @return Number of characters processed.
	/// 
	//////////////////////////////////////////////////////////////////////////
	int PrepareString(BYTE* str, int* dest);

	//////////////////////////////////////////////////////////////////////////
	/// Render string by using the indexes returned from PrepareString.
	/// 
	/// @param text - Cache indexes for rendering.
	/// @param count - Number of characters to render.
	/// @param x - X screen position for rendering.
	/// @param y - Y screen position for rendering.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void RenderEncodedString(const int* text, int count, float x, float y);

	//////////////////////////////////////////////////////////////////////////
	/// Render string to screen.
	/// 
	/// @param str - String to render.
	/// @param x - X screen position for rendering.
	/// @param y - Y screen position for rendering.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void RenderString(BYTE* str, float x, float y, int alignment=JGETEXT_LEFT);

	int GetStringWidth(BYTE* str);
	int GetStringHeight(BYTE* str);

	//////////////////////////////////////////////////////////////////////////
	/// Set scale for rendering.
	/// 
	/// @param scale - Scale for rendering characters.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void SetScale(float scale);


	//////////////////////////////////////////////////////////////////////////
	/// Set angle for rendering.
	/// 
	/// @param rot - Rotation angle in radian.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void SetRotation(float rot);

	//////////////////////////////////////////////////////////////////////////
	/// Set font color.
	/// 
	/// @param color - color of font.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void SetColor(PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Set background color.
	/// 
	/// @param color - Background color.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void SetBgColor(PIXEL_TYPE color);


private:

	static JRenderer* mRenderer;
	
	BYTE* mChnFont;
	BYTE* mEngFont;

	DWORD* mCharBuffer;

	PIXEL_TYPE mColor;
	PIXEL_TYPE mBgColor;

	int mFontSize;
	int mBytesPerChar;
	int mBytesPerRow;

	int mCacheSize;
	int mCacheImageWidth;
	int mCacheImageHeight;

	int mCol;
	int mRow;

//public:

	JTexture* mTexture;
	JQuad** mSprites;
	
	int *mGBCode;

	int mCurr;

	float mScale;
	float mRotation;

	int mCount;

	bool mSmallEnglishFont;

	void GetStringArea(BYTE* str,  int *w, int *h);
};


#endif
