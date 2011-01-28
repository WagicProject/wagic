//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#ifndef _JTTFONT_H
#define _JTTFONT_H

#include "../../JGE/include/JGE.h"

#include <ft2build.h>
#include <freetype/freetype.h>

#define TTF_CACHE_SIZE		256


//////////////////////////////////////////////////////////////////////////
/// True Type font support with the help of Freetype library. JTTFont has
/// a simple caching system so that a character which has been rendered before
/// can be retrieved from the cache instead of drawing it again by the
/// Freetype library. This can give you a much faster rendering speed.
/// Also, if you only need to use a limited number of characters
/// in your game, you can actually cache all your characters in the cache  
/// beforehand and unload the font to save memory.
/// 
/// @par For example, if you only want to use the standard ASCII characters in
/// your game:
/// 
/// @code
/// 
/// // in Create()
/// mTTFont = new JTTFont();
/// mTTFont->Load("arial.ttf", 32);		// size 32
/// 
/// if (mTTFont->PreCacheASCII())
///		mTTFont->Unload();
/// ...
/// 
/// // in Render()
/// mTTFont->RenderString("Hello World!", 240, 80, JGETEXT_CENTER);
/// 
/// @endcode
/// 
//////////////////////////////////////////////////////////////////////////
class JTTFont
{

public:

	//////////////////////////////////////////////////////////////////////////
	/// Constructor.
	/// 
	/// @param cacheImageSize - Size of the texture used for caching. This can
	///							be 64x64, 128x128(default), 256x256 or 512x512.
	/// 
	//////////////////////////////////////////////////////////////////////////
	JTTFont(int cacheImageSize=CACHE_IMAGE_256x256);

	~JTTFont();

	//////////////////////////////////////////////////////////////////////////
	/// \enum FONT_LOADING_MODE
	/// 
	/// Font loading options.
	/// 
	//////////////////////////////////////////////////////////////////////////
	enum FONT_LOADING_MODE
	{
		MODE_NORMAL,			///< Load only.
		MODE_PRECACHE_ASCII,	///< Load the font and cache all ASCII characters.
		MODE_PRECACHE_ASCII_EX	///< Load the font and cache all Extended ASCII characters.
	};

	//////////////////////////////////////////////////////////////////////////
	/// \enum CACHE_IMAGE_SIZE
	/// 
	/// Size of the texture used for caching.
	/// 
	//////////////////////////////////////////////////////////////////////////
	enum CACHE_IMAGE_SIZE
	{
		CACHE_IMAGE_64x64,		///< 64x64
		CACHE_IMAGE_128x128,	///< 128x128
		CACHE_IMAGE_256x256,	///< 256x256
		CACHE_IMAGE_512x512 	///< 512x512
	};

	//////////////////////////////////////////////////////////////////////////
	/// Set color of font.
	/// 
	/// @param color - Font color.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void SetColor(PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Set angle of the font for rendering.
	/// 
	/// @param angle - Angle in radians.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void SetAngle(float angle);

	//////////////////////////////////////////////////////////////////////////
	/// Set font size.
	/// 
	/// @param size - Font size.
	/// 
	/// @note Setting font size will clear the cache.
	/// 
	//////////////////////////////////////////////////////////////////////////
	bool SetSize(int size);

	//////////////////////////////////////////////////////////////////////////
	/// Load font file.
	/// 
	/// @param filename - Name of True Type font.
	/// @param size - Initial font size. Default is 12.
	/// @param mode - Loading mode.
	/// 
	/// @return - True if no error.
	/// 
	//////////////////////////////////////////////////////////////////////////
	bool Load(const char *filename, int size=12, int mode=MODE_NORMAL);

	//////////////////////////////////////////////////////////////////////////
	/// Create font using font data from another JTTFont instance.
	/// 
	/// @param fontSource - Source of font data.
	/// @param size - Initial font size. Default is 12.
	/// @param mode - Loading mode.
	/// 
	/// @return - True if no error.
	/// 
	//////////////////////////////////////////////////////////////////////////
	bool Load(JTTFont *fontSource, int size=12, int mode=MODE_NORMAL);

	//////////////////////////////////////////////////////////////////////////
	/// Unload font file and related Freetype objects from memory.
	///
	//////////////////////////////////////////////////////////////////////////
	void Unload(void);

	//////////////////////////////////////////////////////////////////////////
	/// Render Unicode string to screen.
	/// 
	/// @param text - NULL terminated Unicode-16 string.
	/// @param x - X position.
	/// @param y - Y position.
	/// @param alignment - Text alignment: JGETEXT_LEFT, JGETEXT_RIGHT, JGETEXT_CENTER
	///
	//////////////////////////////////////////////////////////////////////////
	void RenderString(const u16 *text, float x, float y, int alignment=JGETEXT_LEFT);

	//////////////////////////////////////////////////////////////////////////
	/// Render ASCII string to screen.
	/// 
	/// @param text - NULL terminated ASCII string.
	/// @param x - X position.
	/// @param y - Y position.
	/// @param alignment - Text alignment: JGETEXT_LEFT, JGETEXT_RIGHT, JGETEXT_CENTER
	///
	//////////////////////////////////////////////////////////////////////////
	void RenderString(const char *text, float x, float y, int alignment=JGETEXT_LEFT);

	//////////////////////////////////////////////////////////////////////////
	/// Render Chinese (GBK) string to screen.
	/// 
	/// @param text - NULL terminated GBK encoded string.
	/// @param x - X position.
	/// @param y - Y position.
	/// @param alignment - Text alignment: JGETEXT_LEFT, JGETEXT_RIGHT, JGETEXT_CENTER
	///
	//////////////////////////////////////////////////////////////////////////
	void RenderString(const u8 *text, float x, float y, int alignment=JGETEXT_LEFT);

	//////////////////////////////////////////////////////////////////////////
	/// Put characters of an Unicode string into cache
	/// 
	/// @param text - NULL terminated Unicode-16 string.
	///
	//////////////////////////////////////////////////////////////////////////
	void PreCacheString(const u16 *text);

	//////////////////////////////////////////////////////////////////////////
	/// Put characters of an ASCII string into cache.
	/// 
	/// @param text - NULL terminated ASCII string.
	///
	//////////////////////////////////////////////////////////////////////////
	void PreCacheString(const char *text);

	//////////////////////////////////////////////////////////////////////////
	/// Put characters of a Chinese (GBK) string into cache.
	/// 
	/// @param text - NULL terminated GBK encoded string.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void PreCacheString(const u8 *text);

	//////////////////////////////////////////////////////////////////////////
	/// Get width of Unicode string on screen.
	/// 
	/// @param text - NULL terminated Unicode-16 string.
	/// 
	/// @return - Width in pixels
	/// 
	//////////////////////////////////////////////////////////////////////////
	int GetWidth(const u16 *text);

	//////////////////////////////////////////////////////////////////////////
	/// Get width of ASCII string on screen.
	/// 
	/// @param text - NULL terminated ASCII string.
	/// 
	/// @return - Width in pixels
	/// 
	//////////////////////////////////////////////////////////////////////////
	int GetWidth(const char *text);

	//////////////////////////////////////////////////////////////////////////
	/// Get width of Chinese (GBK) string on screen.
	/// 
	/// @param text - NULL terminated GBK encoded string.
	/// 
	/// @return - Width in pixels
	/// 
	//////////////////////////////////////////////////////////////////////////
	int GetWidth(const u8 *text);

	//////////////////////////////////////////////////////////////////////////
	/// Put all standard ASCII characters (0x20-0x7F) into cache.
	/// 
	/// @return - True if success.
	///
	//////////////////////////////////////////////////////////////////////////
	bool PreCacheASCII();

	//////////////////////////////////////////////////////////////////////////
	/// Put all ASCII characters (0x20-0xFF) into cache.
	/// 
	/// @return - True if success.
	///
	//////////////////////////////////////////////////////////////////////////
	bool PreCacheExtendedASCII();

	void SetAntialias(bool flag);

protected:
	FT_Library GetFontLibrary();
	FT_Byte* GetFontBits();
	int GetFontBitsSize();

private:

	int RenderString(const u16 *text, float x, float y, bool render);
	int RenderString(const char *text, float x, float y, bool render);
	int RenderString(const u8 *text, float x, float y, bool render);


	int PreCacheChar(u16 ch, u16 cachedCode);
	int GetCachedChar(u16 cachedCode);
	void DrawBitmap(void *image, FT_Bitmap *bitmap, FT_Int x, FT_Int y, int width, int height);


	JTexture* mTexture;

	JQuad* mQuads[TTF_CACHE_SIZE];
	u16 mCachedCode[TTF_CACHE_SIZE];
	u8 mXAdvance[TTF_CACHE_SIZE];
	
	int mCurr;

	int mTexWidth;
	int mTexHeight;
	int mMaxCharWidth;
	int mMaxCharHeight;
	int mMaxCharCount;
	int mColCount;
	int mRowCount;

	bool mASCIIDirectMapping;

	JTTFont* mFontSource;
	bool mSharingFont;

	int mSize;
	PIXEL_TYPE mColor;
	float mAngle;
	FT_Library mLibrary;
	FT_Face mFace;

	FT_Byte* mFontBits;
	int mFontBitsSize;

	bool mAntialias;

	bool mFontLoaded;

};

#endif
