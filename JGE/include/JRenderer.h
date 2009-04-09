//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#ifndef _JRENDERER_H_
#define _JRENDERER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <gif_lib.h>

#include "JTypes.h"


#if defined (WIN32)

	#include <windows.h>

#elif defined (LINUX)

#else

	#include <pspgu.h>
	#include <pspkernel.h>
	#include <pspdisplay.h>
	#include <pspdebug.h>
	#include <pspctrl.h>
	#include <time.h>
	#include <string.h>
	#include <pspaudiolib.h>
	#include <psprtc.h>

#endif


#include "Vector2D.h"

#define USING_MATH_TABLE

#ifdef USING_MATCH_TABLE
	#define SINF(x)		mSinTable[x]
	#define COSF(x)		mCosTable[x]
#else
	#define SINF(x)		sinf(x*DEG2RAD)
	#define COSF(x)		cosf(x*DEG2RAD)
#endif


//////////////////////////////////////////////////////////////////////////
/// A collection of core rendering functions.
///
//////////////////////////////////////////////////////////////////////////
class JRenderer
{
protected:

	JRenderer();
	~JRenderer();
	void InitRenderer();
	void DestroyRenderer();



public:

	//////////////////////////////////////////////////////////////////////////
	/// Get the singleton instance
	///
	//////////////////////////////////////////////////////////////////////////
	static JRenderer* GetInstance();

	static void Destroy();

	static void Set3DFlag(bool flag);

	void BeginScene();
	void EndScene();

	//////////////////////////////////////////////////////////////////////////
	/// Render a textured quad with rotation and scaling.
	///
	/// @param quad - Quad with texturing info.
	/// @param xo - x position.
	/// @param yo - y position.
	/// @param angle - Rotation (radian).
	/// @param xScale - Horizontal rendering scale.
	/// @param yScale - Vertical rendering scale.
	///
	//////////////////////////////////////////////////////////////////////////
	void RenderQuad(JQuad* quad, float xo, float yo, float angle=0.0f, float xScale=1.0f, float yScale=1.0f);


	//////////////////////////////////////////////////////////////////////////
	/// Render a textured quad with new texture mapping information.
	///
	/// @param quad - Quad with texturing information.
	/// @param points - Array of vertices with new texture mapping information.
	///
	//////////////////////////////////////////////////////////////////////////
	void RenderQuad(JQuad* quad, VertexColor* points);

	//////////////////////////////////////////////////////////////////////////
	/// Taking a screen shot.
	///
	/// @note This function works on PSP only. A PNG file will be saved in
	/// the current folder of the game applicaton.
	///
	//////////////////////////////////////////////////////////////////////////
	void ScreenShot(const char* filename);

	//////////////////////////////////////////////////////////////////////////
	/// Load a texture from file.
	///
	/// @param filename - Name of file.
	/// @param mode - Choose to put texture in VRAM (PSP only).
	///
	//////////////////////////////////////////////////////////////////////////
	JTexture* LoadTexture(const char* filename, int mode = 0, int textureFormat = TEXTURE_FORMAT);

	//////////////////////////////////////////////////////////////////////////
	/// Create texture from memory on the fly.
	///
	/// @param width - Width of texture.
	/// @param height - Height of texture.
	/// @param mode - Choose to put texture in VRAM (PSP only)
	///
	//////////////////////////////////////////////////////////////////////////
	JTexture* CreateTexture(int width, int height, int mode = 0);

	//////////////////////////////////////////////////////////////////////////
	/// Clear entire screen to a particular color.
	///
	/// @param color - Color to fill the screen.
	///
	//////////////////////////////////////////////////////////////////////////
	void ClearScreen(PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Enable VSync for the smoothness of moving objects. (PSP only)
	///
	/// @param flag - true to enable, false to disable.
	///
	//////////////////////////////////////////////////////////////////////////
	void EnableVSync(bool flag);

	//////////////////////////////////////////////////////////////////////////
	/// Enable bi-linear filtering for better looking on-screen images.
	///
	/// @param flag - true to enable, false to disable.
	///
	//////////////////////////////////////////////////////////////////////////
	void EnableTextureFilter(bool flag);

	//////////////////////////////////////////////////////////////////////////
	/// Remove all textures from VRAM (PSP only)
	///
	//////////////////////////////////////////////////////////////////////////
	void ResetPrivateVRAM();

	//////////////////////////////////////////////////////////////////////////
	/// Enable/disable swizzle optimization. (PSP only)
	///
	/// @param s - 1 to enable, 0 to disable.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetSwizzle(int s) { mSwizzle = s; }

	//////////////////////////////////////////////////////////////////////////
	/// Bind texture to be used for the rendering followed.
	///
	/// @param tex - Texture to use.
	///
	//////////////////////////////////////////////////////////////////////////
	void BindTexture(JTexture *tex);

	//////////////////////////////////////////////////////////////////////////
	/// Set texture blending options.
	///
	/// @par Blending options:
	///
	/// @code
	///
	///		BLEND_ZERO
	///		BLEND_ONE
	///		BLEND_SRC_COLOR
	///		BLEND_ONE_MINUS_SRC_COLOR
	///		BLEND_SRC_ALPHA
	///		BLEND_ONE_MINUS_SRC_ALPHA
	///		BLEND_DST_ALPHA
	///		BLEND_ONE_MINUS_DST_ALPHA
	///		BLEND_DST_COLOR
	///		BLEND_ONE_MINUS_DST_COLOR
	///		BLEND_SRC_ALPHA_SATURATE
	///
	/// @endcode
	///
	/// @param src - Blending option for source image.
	/// @param dest - Blending option for destination image.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetTexBlend(int src, int dest);

	//////////////////////////////////////////////////////////////////////////
	/// Set texture blending option for source image.
	///
	/// @param src - Blending option for source image.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetTexBlendSrc(int src);

	//////////////////////////////////////////////////////////////////////////
	/// Set texture blending option for destination image.
	///
	/// @param dest - Blending option for destination image.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetTexBlendDest(int dest);

	//////////////////////////////////////////////////////////////////////////
	/// Enable rendering in 2D mode.
	///
	/// @note To be implemented.
	///
	//////////////////////////////////////////////////////////////////////////
	void Enable2D();


	//////////////////////////////////////////////////////////////////////////
	/// Enable rendering in 3D mode.
	///
	/// @note To be implemented.
	///
	//////////////////////////////////////////////////////////////////////////
	void Enable3D();

	//////////////////////////////////////////////////////////////////////////
	/// Restrict all rendering to a rectangular area.
	///
	/// @note This is just the glScissor() function of OpenGL.
	///
	/// @param x - Left of the clipping area.
	/// @param y - Top of the clipping area.
	/// @param width - Width of the clipping area.
	/// @param height - Height of the clipping area.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetClip(int x, int y, int width, int height);

	//////////////////////////////////////////////////////////////////////////
	/// Reset Modelview Identity.
	///
	//////////////////////////////////////////////////////////////////////////
	void LoadIdentity();

	//////////////////////////////////////////////////////////////////////////
	/// Tranlate position in 3D space.
	///
	/// @param x - X position.
	/// @param y - Y position.
	/// @param z - Z position.
	///
	//////////////////////////////////////////////////////////////////////////
	void Translate(float x, float y, float z);

	//////////////////////////////////////////////////////////////////////////
	/// Rotate along X axis.
	///
	/// @param angle - Angle to rotate ( in radians).
	///
	//////////////////////////////////////////////////////////////////////////
	void RotateX(float angle);

	//////////////////////////////////////////////////////////////////////////
	/// Rotate along Y axis.
	///
	/// @param angle - Angle to rotate ( in radians).
	///
	//////////////////////////////////////////////////////////////////////////
	void RotateY(float angle);

	//////////////////////////////////////////////////////////////////////////
	/// Rotate along Z axis.
	///
	/// @param angle - Angle to rotate ( in radians).
	///
	//////////////////////////////////////////////////////////////////////////
	void RotateZ(float angle);

	//////////////////////////////////////////////////////////////////////////
	/// Push matrix.
	///
	//////////////////////////////////////////////////////////////////////////
	void PushMatrix();

	//////////////////////////////////////////////////////////////////////////
	/// Pop matrix.
	///
	//////////////////////////////////////////////////////////////////////////
	void PopMatrix();

	//////////////////////////////////////////////////////////////////////////
	/// Set the field of view angle (in degrees), effective at the next
	/// Enable3D() call.
	///
	/// @param fov - The field of view angle (in degrees).
	///
	//////////////////////////////////////////////////////////////////////////
	void SetFOV(float fov);

	//////////////////////////////////////////////////////////////////////////
	/// Render triangles.
	///
	/// @param texture - Texture for the triangles.
	/// @param tris - List of triangles.
	/// @param start - starting index (Note: Index of triangles, NOT vertices).
	/// @param count - Number of triangles (Note: NOT number of vertices).
	///
	//////////////////////////////////////////////////////////////////////////
	void RenderTriangles(JTexture* texture, Vertex3D *tris, int start, int count);

	//////////////////////////////////////////////////////////////////////////
	/// Fill a rectangular area with a specified color.
	///
	/// @param x - Starting x position.
	/// @param y - Starting y position.
	/// @param width - Width of the rectangle.
	/// @param height - Height of the rectangle.
	/// @param color - Filling color.
	///
	//////////////////////////////////////////////////////////////////////////
	void FillRect(float x, float y, float width, float height, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Fill a rectangular area with a single color for each vertex.
	///
	/// @param x - Starting x position.
	/// @param y - Starting y position.
	/// @param width - Width of the rectangle.
	/// @param height - Height of the rectangle.
	/// @param color - Array of colors.
	///
	//////////////////////////////////////////////////////////////////////////
	void FillRect(float x, float y, float width, float height, JColor* color);
	void FillRect(float x, float y, float width, float height, PIXEL_TYPE* color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw a rectangle.
	///
	/// @param x - Starting x position.
	/// @param y - Starting y position.
	/// @param width - Width of the rectangle.
	/// @param height - Height of the rectangle.
	/// @param color - Filling color.
	///
	//////////////////////////////////////////////////////////////////////////
	void DrawRect(float x, float y, float width, float height, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw a single line.
	///
	/// @param x1 - Starting vertex, x.
	/// @param y1 - Starting vertex, y.
	/// @param x2 - Ending vertex, x.
	/// @param y2 - Ending vertex, y.
	/// @param color - Filling color.
	///
	//////////////////////////////////////////////////////////////////////////
	void DrawLine(float x1, float y1, float x2, float y2, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw thick line.
	///
	/// @param x1 - Starting vertex, x.
	/// @param y1 - Starting vertex, y.
	/// @param x2 - Ending vertex, x.
	/// @param y2 - Ending vertex, y.
	/// @param lineWidth - Line width.
	/// @param color - Filling color.
	///
	//////////////////////////////////////////////////////////////////////////
	void DrawLine(float x1, float y1, float x2, float y2, float lineWidth, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Plot a pixel on screen.
	///
	/// @param x - X position of the pixel.
	/// @param y - Y position of the pixel.
	/// @param color - Draw colour.
	///
	//////////////////////////////////////////////////////////////////////////
	void Plot(float x, float y, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Plot an array of pixels.
	///
	/// @param x - Array of X positions.
	/// @param y - Array of Y positions.
	/// @param count - Number of pixels to plot.
	/// @param color - Color of pixel.
	///
	//////////////////////////////////////////////////////////////////////////
	void PlotArray(float *x, float *y, int count, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw polygon with filled colour.
	///
	/// @param x - Array of X positions.
	/// @param y - Array of Y positions.
	/// @param count - Side count of the polygon.
	/// @param color - Filling colour.
	///
	//////////////////////////////////////////////////////////////////////////
	void FillPolygon(float* x, float* y, int count, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw polygon.
	///
	/// @param x - Array of X positions.
	/// @param y - Array of Y positions.
	/// @param count - Side count of the polygon.
	/// @param color - Draw colour.
	///
	//////////////////////////////////////////////////////////////////////////
	void DrawPolygon(float* x, float* y, int count, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw symmetric polygon with certain number of sides.
	///
	/// @param x - X positions of center of the polygon.
	/// @param y - Y positions of center of the polygon.
	/// @param size - Size of polygon.
	/// @param count - Side count of the polygon.
	/// @param startingAngle - Rotation angle of the polygon.
	/// @param color - Draw colour.
	///
	//////////////////////////////////////////////////////////////////////////
	void DrawPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw solid symmetric polygon with certain number of sides.
	///
	/// @param x - X positions of center of the polygon.
	/// @param y - Y positions of center of the polygon.
	/// @param size - Size of polygon.
	/// @param count - Side count of the polygon.
	/// @param startingAngle - Rotation angle of the polygon.
	/// @param color - Filling colour.
	///
	//////////////////////////////////////////////////////////////////////////
	void FillPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw circle with filled colour.
	///
	/// @param x - X positions of center of the circle.
	/// @param y - Y positions of center of the circle.
	/// @param radius - Radius of circle.
	/// @param color - Filling colour.
	///
	//////////////////////////////////////////////////////////////////////////
	void FillCircle(float x, float y, float radius, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw circle.
	///
	/// @param x - X positions of center of the circle.
	/// @param y - Y positions of center of the circle.
	/// @param radius - Radius of circle.
	/// @param color - Draw colour.
	///
	//////////////////////////////////////////////////////////////////////////
	void DrawCircle(float x, float y, float radius, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw a rectangle with round corners.
	///
	/// @param x - Starting x position.
	/// @param y - Starting y position.
	/// @param w - Width of the rectangle.
	/// @param h - Height of the rectangle.
	/// @param radius - Radius of the round corners.
	/// @param color - Drawing color.
	///
	//////////////////////////////////////////////////////////////////////////
	void DrawRoundRect(float x, float y, float w, float h, float radius, PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Draw filled rectangle with round corners.
	///
	/// @param x - Starting x position.
	/// @param y - Starting y position.
	/// @param w - Width of the rectangle.
	/// @param h - Height of the rectangle.
	/// @param radius - Radius of the round corners.
	/// @param color - Filling color.
	///
	//////////////////////////////////////////////////////////////////////////
	void FillRoundRect(float x, float y, float w, float h, float radius, PIXEL_TYPE color);


	//////////////////////////////////////////////////////////////////////////
	/// Set custom image filter to be used at texture loading.
	///
	/// @param imageFilter - Custom image filter.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetImageFilter(JImageFilter* imageFilter);

private:

	struct TextureInfo
	{
		u8 *mBits;
		int mWidth;
		int mHeight;
		int mTexWidth;
		int mTexHeight;
		bool mVRAM;
	};

	void LoadJPG(TextureInfo &textureInfo, const char *filename, int mode = 0, int TextureFormat = TEXTURE_FORMAT);
	void LoadPNG(TextureInfo &textureInfo, const char *filename, int mode = 0, int TextureFormat = TEXTURE_FORMAT);
	void LoadGIF(TextureInfo &textureInfo, const char *filename, int mode = 0, int TextureFormat = TEXTURE_FORMAT);
	int image_readgif(void * handle, TextureInfo &textureInfo, DWORD * bgcolor, InputFunc readFunc,int mode = 0, int TextureFormat = TEXTURE_FORMAT);

	static JRenderer* mInstance;


#if defined (WIN32) || defined (LINUX)

	GLuint mCurrentTex;


#else

	u32 mVideoBufferStart;
	//u32 mCurrentPointer;
	PIXEL_TYPE* mVRAM;
	int mCurrentTex;
	int mCurrentBlend;
  int mCurrentTextureFormat;

  int PixelSize(int textureMode);

#endif

	bool mVsync;

	int mSwizzle;
	int mTexCounter;

	//int mTexFilter;

	int mCurrentTextureFilter;

	int mCurrTexBlendSrc;
	int mCurrTexBlendDest;

	JImageFilter* mImageFilter;

	int mCurrentRenderMode;

	float mFOV;

#ifdef USING_MATH_TABLE
	float mSinTable[360];
	float mCosTable[360];
#endif

	static bool m3DEnabled;
};


#endif
