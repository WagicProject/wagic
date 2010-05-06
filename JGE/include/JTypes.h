//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#ifndef _JTYPES_H
#define _JTYPES_H

#if defined (WIN32) || defined (LINUX)

#include <stdint.h>

#else

#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <time.h>
#include <string.h>
#include <pspaudiolib.h>
#include <psprtc.h>

#include "JAudio.h"

#endif

#ifndef __GNUC__
#define __attribute__(arg)
#endif


#define MAX_CHANNEL		128

enum {
  JGE_ERR_CANT_OPEN_FILE = -1,
  JGE_ERR_PNG = -2,  
  JGE_ERR_MALLOC_FAILED = -4,
  JGE_ERR_GENERIC = -5,
};


#ifndef M_PI
#define M_PI	3.14159265358979323846f
#define M_PI_2	1.57079632679489661923f
#define M_PI_4	0.785398163397448309616f
#define M_1_PI	0.318309886183790671538f
#define M_2_PI	0.636619772367581343076f
#endif

#define RAD2DEG		57.29577951f
#define DEG2RAD		0.017453293f

#define SAFE_DELETE(x)		do { if (x) { delete x; x = NULL; } } while(false)
#define SAFE_DELETE_ARRAY(x)	if (x) { delete [] x; x = NULL; }


#define SCREEN_WIDTH 			480
#define SCREEN_HEIGHT 			272
#define SCREEN_WIDTH_F 			480.0f
#define SCREEN_HEIGHT_F			272.0f


#ifdef WIN32
//	#define DEFAULT_BLEND		BLEND_DEFAULT
//	#define BLEND_OPTION_ADD	BLEND_COLORADD
//	#define BLEND_OPTION_BLEND	(BLEND_COLORADD | BLEND_ALPHABLEND | BLEND_NOZWRITE)
#else
	#define DEFAULT_BLEND		GU_TFX_MODULATE
	#define BLEND_OPTION_ADD	GU_TFX_ADD
	#define BLEND_OPTION_BLEND	GU_TFX_BLEND
#endif

#ifdef WIN32
	#include <windows.h>
#endif
#ifdef LINUX
        typedef uint8_t byte;
        typedef uint32_t DWORD;
        typedef uint8_t BYTE;
        typedef bool BOOL;
#endif

#if defined (WIN32) || defined (LINUX)
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#if defined (WIN32) || defined (LINUX)

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;


	#define BLEND_ZERO					GL_ZERO
	#define BLEND_ONE					GL_ONE
	#define BLEND_SRC_COLOR				GL_SRC_COLOR
	#define BLEND_ONE_MINUS_SRC_COLOR	GL_ONE_MINUS_SRC_COLOR
	#define BLEND_SRC_ALPHA				GL_SRC_ALPHA
	#define BLEND_ONE_MINUS_SRC_ALPHA	GL_ONE_MINUS_SRC_ALPHA
	#define BLEND_DST_ALPHA				GL_DST_ALPHA
	#define BLEND_ONE_MINUS_DST_ALPHA	GL_ONE_MINUS_DST_ALPHA
	#define BLEND_DST_COLOR				GL_DST_COLOR
	#define BLEND_ONE_MINUS_DST_COLOR	GL_ONE_MINUS_DST_COLOR
	#define BLEND_SRC_ALPHA_SATURATE	GL_SRC_ALPHA_SATURATE

	#define ARGB(a, r, g, b)		(((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
	#define RGBA(r, g, b, a)		(((a) << 24) | ((b) << 16) | ((g) << 8) | (r))

  #define TEXTURE_FORMAT			0
  #define GU_PSM_8888 0
  #define GU_PSM_5551 0
  #define GU_PSM_4444 0
  #define GU_PSM_5650 0
	#define PIXEL_TYPE				DWORD

#else	// PSP

	#ifndef ABGR8888
	#define ABGR8888
	#endif


	#if defined (ABGR8888)
		#ifndef ARGB
		#define ARGB(a, r, g, b)		((a << 24) | (b << 16) | (g << 8) | r)	// macro to assemble pixels in correct format
		#endif
		#define MAKE_COLOR(a, c)		(a << 24 | c)
		#define MASK_ALPHA				0xFF000000							// masks for accessing individual pixels
		#define MASK_BLUE				0x00FF0000
		#define MASK_GREEN				0x0000FF00
		#define MASK_RED				0x000000FF
		#define PIXEL_TYPE				u32
		#define PIXEL_SIZE				4
		#define PIXEL_FORMAT			PSP_DISPLAY_PIXEL_FORMAT_8888

		#define	BUFFER_FORMAT			GU_PSM_8888
		#define TEXTURE_FORMAT			GU_PSM_8888
		#define TEXTURE_COLOR_FORMAT	GU_COLOR_8888


	#elif defined (ABGR5551)

		#ifndef ARGB
		#define ARGB(a, r, g, b)		((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | ((a >> 7) << 15))
		#endif
		#define MAKE_COLOR(a, c)		(((a>>7)<<15) | c)
		#define MASK_ALPHA				0x8000
		#define MASK_BLUE				0x7C00
		#define MASK_GREEN				0x03E0
		#define MASK_RED				0x001F
		#define PIXEL_TYPE				u16
		#define PIXEL_SIZE				2
		#define PIXEL_FORMAT			PSP_DISPLAY_PIXEL_FORMAT_5551

		#define	BUFFER_FORMAT			GU_PSM_8888
		#define TEXTURE_FORMAT			GU_PSM_5551
		#define TEXTURE_COLOR_FORMAT	GU_COLOR_5551

	#elif defined (ABGR4444)
		#ifndef ARGB
		#define ARGB(a, r, g, b)		((r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8) | ((a >> 4) << 12))
		#endif
		#define MAKE_COLOR(a, c)		(((a>>4)<<12) | c)
		#define MASK_ALPHA				0xF000
		#define MASK_BLUE				0x0F00
		#define MASK_GREEN				0x00F0
		#define MASK_RED				0x000F
		#define PIXEL_TYPE				u16
		#define PIXEL_SIZE				2
		#define PIXEL_FORMAT			PSP_DISPLAY_PIXEL_FORMAT_4444

		#define	BUFFER_FORMAT			GU_PSM_4444
		#define TEXTURE_FORMAT			GU_PSM_4444
		#define TEXTURE_COLOR_FORMAT	GU_COLOR_4444

	#endif

	#define	FRAME_BUFFER_WIDTH 		512
	#define FRAME_BUFFER_SIZE		FRAME_BUFFER_WIDTH*SCREEN_HEIGHT*PIXEL_SIZE

	#define SLICE_SIZE_F			64.0f
	typedef unsigned int DWORD;

	#define BLEND_ZERO					0x1000
	#define BLEND_ONE					0x1002
	#define BLEND_SRC_COLOR				GU_SRC_COLOR
	#define BLEND_ONE_MINUS_SRC_COLOR	GU_ONE_MINUS_SRC_COLOR
	#define BLEND_SRC_ALPHA				GU_SRC_ALPHA
	#define BLEND_ONE_MINUS_SRC_ALPHA	GU_ONE_MINUS_SRC_ALPHA
	#define BLEND_DST_ALPHA				GU_DST_ALPHA
	#define BLEND_ONE_MINUS_DST_ALPHA	GU_ONE_MINUS_DST_ALPHA
	#define BLEND_DST_COLOR				GU_DST_COLOR
	#define BLEND_ONE_MINUS_DST_COLOR	GU_ONE_MINUS_DST_COLOR
	#define BLEND_SRC_ALPHA_SATURATE	BLEND_ONE

	typedef struct
	{
		ScePspFVector2 texture;
		ScePspFVector3 pos;
	} PSPVertex3D;


#endif



typedef enum Buttons
  {
    JGE_BTN_NONE = 0,   // No button pressed
    JGE_BTN_QUIT,   // Home on PSP
    JGE_BTN_MENU,   // Start on PSP
    JGE_BTN_CTRL,   // Select
    JGE_BTN_POWER,  // Hold
    JGE_BTN_SOUND,  // Music note
    JGE_BTN_RIGHT,
    JGE_BTN_LEFT,
    JGE_BTN_UP,
    JGE_BTN_DOWN,
    JGE_BTN_OK,     // Circle in Japan, Cross in Europe
    JGE_BTN_CANCEL, // Triangle
    JGE_BTN_PRI,    // Square (primary)
    JGE_BTN_SEC,    // Cross or Circle (secondary)
    JGE_BTN_PREV,   // Left trigger
    JGE_BTN_NEXT,    // Right trigger
    JGE_BTN_FULLSCREEN,    // Switch to fullscreen (obviously, PC only)

    JGE_BTN_MAX = JGE_BTN_NEXT + 1
  } JButton;



//------------------------------------------------------------------------------------------------
struct Vertex
{
	float u, v;
	PIXEL_TYPE color;
	float x, y, z;
};


//------------------------------------------------------------------------------------------------
struct Vertex3D
{
	float u, v;
	float x, y, z;
};


//------------------------------------------------------------------------------------------------
struct VertexColor
{
	PIXEL_TYPE color;
	float x, y, z;
};


struct JColor
{
	union
	{
		struct
		{
			u8 b;
			u8 g;
			u8 r;
			u8 a;
		};
		DWORD color;
	};
};



enum
{
	TEX_TYPE_NONE,
	TEX_TYPE_USE_VRAM,
	TEX_TYPE_MIPMAP,
	TEX_TYPE_NORMAL,
	TEX_TYPE_SKYBOX
};


enum
{
	MODE_UNKNOWN,
	MODE_2D,
	MODE_3D
};


enum
{
	TEX_FILTER_NONE,
	TEX_FILTER_LINEAR,
	TEX_FILTER_NEAREST
};

//------------------------------------------------------------------------------------------------
class JTexture
{
public:
	JTexture();
	~JTexture();

	void UpdateBits(int x, int y, int width, int height, PIXEL_TYPE* bits);

	int mWidth;
	int mHeight;
	int mTexWidth;
	int mTexHeight;

	int mFilter;

#if defined (WIN32) || defined (LINUX)
	GLuint mTexId;
#else
  int mTextureFormat;
	int mTexId;
	bool mInVideoRAM;
	PIXEL_TYPE* mBits;
#endif
};


//////////////////////////////////////////////////////////////////////////
/// Custom filter for processing the texture image while loading. You
/// can change the pixels by using a custom filter before the image is
/// created as a texture.
///
//////////////////////////////////////////////////////////////////////////
class JImageFilter
{
public:

	//////////////////////////////////////////////////////////////////////////
	/// Pure virtual function for the custom filter to implement.
	///
	/// @param pix - Image data.
	/// @param width - Width of the image.
	/// @param height - Height of the image.
	///
	//////////////////////////////////////////////////////////////////////////
	virtual void ProcessImage(PIXEL_TYPE* pix, int width, int height) = 0;
};


//////////////////////////////////////////////////////////////////////////
/// Image quad.
///
//////////////////////////////////////////////////////////////////////////
class JQuad
{
public:

	//////////////////////////////////////////////////////////////////////////
	/// Constructor.
	///
	/// @param tex - Texture of the quad.
	/// @param x - X position of the quad in texture.
	/// @param y - Y position of the quad in texture.
	/// @param width - Width of the quad.
	/// @param height - Height of the quad.
	///
	//////////////////////////////////////////////////////////////////////////
	JQuad(JTexture *tex, float x, float y, float width, float height);

	//////////////////////////////////////////////////////////////////////////
	/// Set blending color of the quad.
	///
	/// @param color - Color.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetColor(PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Set anchor point of the quad.
	///
	/// @param x - X position of the anchor point.
	/// @param y - Y position of the anchor point.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetHotSpot(float x, float y);

	//////////////////////////////////////////////////////////////////////////
	/// Set UV positions of the quad.
	///
	/// @param x - X position of the quad in texture.
	/// @param y - Y position of the quad in texture.
	/// @param w - Width of the quad.
	/// @param h - Height of the quad.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetTextureRect(float x, float y, float w, float h);

	//////////////////////////////////////////////////////////////////////////
	/// Get UV positions of the quad.
	///
	/// @return x - X position of the quad in texture.
	/// @return y - Y position of the quad in texture.
	/// @return w - Width of the quad.
	/// @return h - Height of the quad.
	///
	//////////////////////////////////////////////////////////////////////////
	void GetTextureRect(float *x, float *y, float *w, float *h);

	//////////////////////////////////////////////////////////////////////////
	/// Set horizontal flipping.
	///
	/// @param hflip - flipping flag;
	///
	//////////////////////////////////////////////////////////////////////////
	void SetHFlip(bool hflip) { mHFlipped = hflip; }

	//////////////////////////////////////////////////////////////////////////
	/// Set vetical flipping.
	///
	/// @param hflip - flipping flag;
	///
	//////////////////////////////////////////////////////////////////////////
	void SetVFlip(bool vflip) { mVFlipped = vflip; }

	JTexture* mTex;

#if defined (WIN32) || defined(LINUX)
	float mTX0;
	float mTY0;
	float mTX1;
	float mTY1;
	JColor mColor[4];		// up to 4 vertices
#else
	PIXEL_TYPE mColor[4];	// up to 4 vertices
	int mBlend;				// GU_TFX_MODULATE, GU_TFX_DECAL, GU_TFX_BLEND, GU_TFX_REPLACE, GU_TFX_ADD
#endif

	float mX;
	float mY;
	float mWidth;
	float mHeight;
	float mHotSpotX;
	float mHotSpotY;

	bool mHFlipped;
	bool mVFlipped;
};

//#endif


//////////////////////////////////////////////////////////////////////////
/// \enum JFONT_TEXT_ALIGNMENT
///
/// Font alignment.
///
//////////////////////////////////////////////////////////////////////////
enum JFONT_TEXT_ALIGNMENT
{
	JGETEXT_LEFT,		///< Text alignment to left.
	JGETEXT_CENTER,		///< Text alignment to center.
	JGETEXT_RIGHT		///< Text alignment to right.
};


enum JINIT_FLAG
{
	JINIT_FLAG_NORMAL,
	JINIT_FLAG_ENABLE3D
};


//------------------------------------------------------------------------------------------------
class JFont
{
public:
	JQuad* mQuad;
	int mWidth;
	int mHeight;
	int mSpacing;
	int mAlign;
	float mScale;
};


//------------------------------------------------------------------------------------------------
class Rect
{
public:
	int x;
	int y;
	int width;
	int height;

public:
	Rect(int _x, int _y, int _width, int _height): x(_x), y(_y), width(_width), height(_height) {}

};



#endif
