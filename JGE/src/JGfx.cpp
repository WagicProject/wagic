//-------------------------------------------------------------------------------------
//
// JGE is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#include <malloc.h>
#include <pspgu.h>
#include <pspgum.h>
#include <fastmath.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <png.h>
#include "../include/vram.h"
#include "../include/JLogger.h"

#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif

#include <jpeglib.h>

#ifdef __cplusplus
}
#endif

#include "../include/JGE.h"
#include "../include/JRenderer.h"
#include "../include/JFileSystem.h"

static unsigned int __attribute__((aligned(16))) list[262144];


extern void SwizzlePlot(u8* out, PIXEL_TYPE color, int i, int j, unsigned int width);

void Swap(float *a, float *b)
{
	float n=*a;
	*a = *b;
	*b = n;
}

JQuad::JQuad(JTexture *tex, float x, float y, float width, float height)
		:mTex(tex), mX(x), mY(y), mWidth(width), mHeight(height)
{
	mHotSpotX = 0.0f;
	mHotSpotY = 0.0f;
	mBlend = DEFAULT_BLEND;		//GU_TFX_MODULATE;
	for (int i=0;i<4;i++)
		mColor[i] = ARGB(255,255,255,255);

	mHFlipped = false;
	mVFlipped = false;
}


void JQuad::GetTextureRect(float *x, float *y, float *w, float *h)
{
	*x=mX; *y=mY; *w=mWidth; *h=mHeight;
}


void JQuad::SetTextureRect(float x, float y, float w, float h)
{
	mX = x; mY = y; mWidth = w; mHeight = h;
}


void JQuad::SetColor(PIXEL_TYPE color)
{
	for (int i=0;i<4;i++)
		mColor[i] = color;
}


void JQuad::SetHotSpot(float x, float y)
{
	mHotSpotX = x;
	mHotSpotY = y;
}

//////////////////////////////////////////////////////////////////////////

JTexture::JTexture()
{
	mBits = NULL;
	mInVideoRAM = false;
  mTextureFormat = TEXTURE_FORMAT;
}

JTexture::~JTexture()
{
	if (mBits)
	{
		if (mInVideoRAM)
			vfree(mBits);
		else
			free(mBits);
	}
}


void JTexture::UpdateBits(int x, int y, int width, int height, PIXEL_TYPE* bits)
{
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++)
		{
			SwizzlePlot((u8*)mBits, *(bits++), (x+j)*PIXEL_SIZE, y+i, mTexWidth*PIXEL_SIZE);
		}
	}
	sceKernelDcacheWritebackAll();
}

//////////////////////////////////////////////////////////////////////////

JRenderer* JRenderer::mInstance = NULL;

bool JRenderer::m3DEnabled = false;

void JRenderer::Set3DFlag(bool flag) { m3DEnabled = flag; }


JRenderer* JRenderer::GetInstance()
{
	if (mInstance == NULL)
	{
		mInstance = new JRenderer();
		mInstance->InitRenderer();
	}

	return mInstance;
}


void JRenderer::Destroy()
{
	if (mInstance)
	{
		mInstance->DestroyRenderer();
		delete mInstance;
		mInstance = NULL;
	}
}

JRenderer::JRenderer()
{
}


JRenderer::~JRenderer()
{

}


void JRenderer::InitRenderer()
{

	mCurrentRenderMode = MODE_2D;

#ifdef USING_MATH_TABLE
	for (int i=0;i<360;i++)
	{
		mSinTable[i] = sinf(i*DEG2RAD);
		mCosTable[i] = cosf(i*DEG2RAD);
	}
#endif

  mCurrTexBlendSrc = BLEND_SRC_ALPHA;
  mCurrTexBlendDest = BLEND_ONE_MINUS_SRC_ALPHA;

	mSwizzle = 1;
	mVsync = false;

	mTexCounter = 0;
	mCurrentTex = -1;
	mCurrentBlend = -1;
  mCurrentTextureFormat = TEXTURE_FORMAT;

	mFOV = 75.0f;

	mImageFilter = NULL;

	mCurrentTextureFilter = TEX_FILTER_LINEAR;

	sceGuInit();

  fbp0 = ( u32* ) valloc ( FRAME_BUFFER_SIZE ) ;
  fbp1 = ( u32* ) valloc ( FRAME_BUFFER_SIZE );
  zbp = NULL;

	// setup GU
	sceGuStart(GU_DIRECT,list);

	sceGuDrawBuffer(BUFFER_FORMAT, vrelptr (fbp0), FRAME_BUFFER_WIDTH);
	sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, vrelptr (fbp1), FRAME_BUFFER_WIDTH);
	if (m3DEnabled)
	{
    zbp = ( u16* )  valloc ( FRAME_BUFFER_WIDTH*SCREEN_HEIGHT*2);
		sceGuDepthBuffer(vrelptr (zbp), FRAME_BUFFER_WIDTH);
	}

	sceGuOffset(2048 - (SCREEN_WIDTH/2), 2048 - (SCREEN_HEIGHT/2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	//sceGuFrontFace(GU_CW);
	sceGuFrontFace(GU_CCW);
	sceGuEnable(GU_TEXTURE_2D);

	sceGuShadeModel(GU_SMOOTH);

	sceGuTexWrap(GU_REPEAT, GU_REPEAT);

	// enable alpha channel
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

	sceGuTexFilter(GU_LINEAR,GU_LINEAR);

	if (m3DEnabled)
	{

		sceGuDepthRange(65535,0);
		sceGuEnable(GU_DEPTH_TEST);
		sceGuDepthFunc(GU_GEQUAL);

		sceGuEnable(GU_CULL_FACE);
		sceGuEnable(GU_CLIP_PLANES);

		sceGuClearColor(0x00ff0000);
		sceGuClearDepth(0);

		sceGuTexEnvColor(0xffffffff);

		sceGuTexScale(1.0f,1.0f);
		sceGuTexOffset(0.0f,0.0f);
		sceGuAmbientColor(0xffffffff);

		sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumPerspective(mFOV,16.0f/9.0f,0.5f,1000.0f);
		//sceGumPerspective(90.0f,480.0f/272.0f,0.5f,1000.0f);

	}

	//sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);

}

void JRenderer::SetTexBlend(int src, int dest)
{

	if (src != mCurrTexBlendSrc || dest != mCurrTexBlendDest)
	{
		mCurrTexBlendSrc = src;
		mCurrTexBlendDest = dest;

		int fixSrc = 0;
		int fixDest = 0;
		if (src == BLEND_ZERO)
			src = GU_FIX;
		else if (src == BLEND_ONE)
		{
			src = GU_FIX;
			fixSrc = 0x00FFFFFF;
		}
		if (dest == BLEND_ZERO)
			dest = GU_FIX;
		else if (dest == BLEND_ONE)
		{
			dest = GU_FIX;
			fixDest = 0x00FFFFFF;
		}

		//glBlendFunc(src, dest);
		sceGuBlendFunc(GU_ADD, src, dest, fixSrc, fixDest);
	}
}


void JRenderer::EnableTextureFilter(bool flag)
{
	if (flag)
		mCurrentTextureFilter = TEX_FILTER_LINEAR;
	else
		mCurrentTextureFilter = TEX_FILTER_NEAREST;
}


void JRenderer::DestroyRenderer()
{
  sceGuDisplay(GU_FALSE);
	sceGuTerm();
  vfree(fbp0);
  vfree(fbp1);
  //debugged = 0;
  if (zbp) vfree(zbp);
}


void JRenderer::BeginScene()
{
	sceGuStart(GU_DIRECT, list);

	if (m3DEnabled)
	{
		//if (mMode3D)
		sceGuClear(GU_DEPTH_BUFFER_BIT|GU_COLOR_BUFFER_BIT);
	}

	sceGuTexMode(TEXTURE_FORMAT, 0, 0, mSwizzle);
  mCurrentTextureFormat = TEXTURE_FORMAT;

	if (mCurrentTextureFilter == TEX_FILTER_NEAREST)
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);				// GU_NEAREST good for tile-map
	else
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);				// GU_LINEAR good for scaling

  //Keep this until we get rev 2489 (or better) of the SDK
  //See http://code.google.com/p/wagic/issues/detail?id=92
  sceGuSendCommandi(210,BUFFER_FORMAT); 
}


void JRenderer::EndScene()
{
	sceGuFinish();

	sceGuSync(0,0);

	if (mVsync)
		sceDisplayWaitVblankStart();

	sceGuSwapBuffers();

	mCurrentTex = -1;
	mCurrentBlend = -1;
}


void JRenderer::EnableVSync(bool flag)
{
	mVsync = flag;
}


void JRenderer::ClearScreen(PIXEL_TYPE color)
{
	sceGuClearColor(color);
	//sceGuClearStencil( 0 );
	sceGuClear(GU_COLOR_BUFFER_BIT);
	//sceGuClear( GU_COLOR_BUFFER_BIT | GU_STENCIL_BUFFER_BIT );
}


void JRenderer::FillRect(float x, float y, float width, float height, PIXEL_TYPE color)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(2 * sizeof(struct VertexColor));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x + width;
	vertices[1].y = y + height;
	vertices[1].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_SPRITES, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);

}


void JRenderer::FillRect(float x, float y, float width, float height, PIXEL_TYPE* colors)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(4 * sizeof(struct VertexColor));


	vertices[0].color = colors[0];
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	vertices[1].color = colors[1];
	vertices[1].x = x + width;
	vertices[1].y = y;
	vertices[1].z = 0.0f;

	vertices[2].color = colors[2];
	vertices[2].x = x;
	vertices[2].y = y + height;
	vertices[2].z = 0.0f;

	vertices[3].color = colors[3];
	vertices[3].x = x + width;
	vertices[3].y = y + height;
	vertices[3].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_STRIP, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);

}


void JRenderer::DrawRect(float x, float y, float width, float height, PIXEL_TYPE color)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(5 * sizeof(struct VertexColor));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x;
	vertices[1].y = y + height;
	vertices[1].z = 0.0f;

	vertices[2].color = color;
	vertices[2].x = x + width;
	vertices[2].y = y + height;
	vertices[2].z = 0.0f;

	vertices[3].color = color;
	vertices[3].x = x + width;
	vertices[3].y = y;
	vertices[3].z = 0.0f;

	vertices[4].color = color;
	vertices[4].x = x;
	vertices[4].y = y;
	vertices[4].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_LINE_STRIP, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 5, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);

}


void JRenderer::DrawLine(float x1, float y1, float x2, float y2, PIXEL_TYPE color)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(2 * sizeof(struct VertexColor));

	vertices[0].color = color;
	vertices[0].x = x1;
	vertices[0].y = y1;
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x2;
	vertices[1].y = y2;
	vertices[1].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_LINES, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);

}


void JRenderer::Plot(float x, float y, PIXEL_TYPE color)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(1 * sizeof(struct VertexColor));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_POINTS, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 1, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);

}


void JRenderer::PlotArray(float *x, float *y, int count, PIXEL_TYPE color)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(count * sizeof(struct VertexColor));

	for (int i=0;i<count;i++)
	{
		vertices[i].color = color;
		vertices[i].x = x[i];
		vertices[i].y = y[i];
		vertices[i].z = 0.0f;
	}

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_POINTS, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, count, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);

}


//////////////////////////////////////////////////////////////////////////
//
//		v1---v2
//			/
//		   /
//		v3---v4
void JRenderer::RenderQuad(JQuad* quad, float xo, float yo, float angle, float xScale, float yScale)
{
  if (mCurrentTextureFormat != quad->mTex->mTextureFormat){
    mCurrentTextureFormat = quad->mTex->mTextureFormat;
    sceGuTexMode(mCurrentTextureFormat, 0, 0, mSwizzle);
  }
  
  if (mCurrentTex != quad->mTex->mTexId)
	{
		sceGuTexImage(0, quad->mTex->mTexWidth, quad->mTex->mTexHeight, quad->mTex->mTexWidth, quad->mTex->mBits);
		mCurrentTex = quad->mTex->mTexId;
	}

	if (mCurrentBlend != quad->mBlend)
	{
		sceGuTexFunc(quad->mBlend, GU_TCC_RGBA);
		mCurrentBlend = quad->mBlend;
	}



	//float destWidth = quad->mWidth*quad->mScaleX;
	float destHeight = quad->mHeight*yScale;
	float x = xo - quad->mHotSpotX*xScale;
	float y = yo - quad->mHotSpotY*yScale;

	float start, end;

	float width;
	float destWidth;
	float fixedWidth = SLICE_SIZE_F*xScale;
	float xx, yy;
	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);

	if (quad->mHFlipped)// || quad->mVFlipped)
	{

		for (end = quad->mX, start = quad->mX+quad->mWidth; start > end; start -= SLICE_SIZE_F)
		{
			// allocate memory on the current display list for temporary storage
			// in order to rotate, we use 4 vertices this time
			struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(4 * sizeof(struct Vertex));
			if ((start - SLICE_SIZE_F) > end)
			{
				width = SLICE_SIZE_F;
				destWidth = fixedWidth;
			}
			else
			{
				width = start-end;
				destWidth = width*xScale;
			}

			vertices[0].u = start;
			vertices[0].v = quad->mY;
			vertices[0].color = quad->mColor[0];//.color;
			vertices[0].x = x;
			vertices[0].y = y;
			vertices[0].z = 0.0f;

			vertices[2].u = start - width;
			vertices[2].v = quad->mY;
			vertices[2].color = quad->mColor[2];//.color;
			vertices[2].x = x + destWidth;
			vertices[2].y = y;
			vertices[2].z = 0.0f;

			vertices[1].u = start;
			vertices[1].v = quad->mY + quad->mHeight;
			vertices[1].color = quad->mColor[1];//.color;
			vertices[1].x = x;
			vertices[1].y = y + destHeight;
			vertices[1].z = 0.0f;

			vertices[3].u = start - width;
			vertices[3].v = quad->mY + quad->mHeight;
			vertices[3].color = quad->mColor[3];//.color;
			vertices[3].x = x + destWidth;
			vertices[3].y = y + destHeight;
			vertices[3].z = 0.0f;

			if (quad->mVFlipped)
			{
				Swap(&vertices[0].v, &vertices[2].v);
				Swap(&vertices[1].v, &vertices[3].v);
			}

			if (angle != 0.0f)
			{
				for (int i=0;i<4;i++)
				{
					xx = (cosAngle*(vertices[i].x-xo) - sinAngle*(vertices[i].y-yo) + xo);
					yy = (sinAngle*(vertices[i].x-xo) + cosAngle*(vertices[i].y-yo) + yo);
					vertices[i].x = xx;
					vertices[i].y = yy;
				}
			}

			x += destWidth;

			sceGuDrawArray(GU_TRIANGLE_STRIP,GU_TEXTURE_32BITF|TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
		}
	}
	else
	{
		for (start = quad->mX, end = quad->mX+quad->mWidth; start < end; start += SLICE_SIZE_F)
		{
			// allocate memory on the current display list for temporary storage
			// in order to rotate, we use 4 vertices this time
			struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(4 * sizeof(struct Vertex));
			if ((start + SLICE_SIZE_F) < end)
			{
				width = SLICE_SIZE_F;
				destWidth = fixedWidth;
			}
			else
			{
				width = end-start;
				destWidth = width*xScale;
			}

			vertices[0].u = start;
			vertices[0].v = quad->mY;
			vertices[0].color = quad->mColor[0];//.color;
			vertices[0].x = x;
			vertices[0].y = y;
			vertices[0].z = 0.0f;

			vertices[2].u = start + width;
			vertices[2].v = quad->mY;
			vertices[2].color = quad->mColor[2];//.color;
			vertices[2].x = x + destWidth;
			vertices[2].y = y;
			vertices[2].z = 0.0f;

			vertices[1].u = start;
			vertices[1].v = quad->mY + quad->mHeight;
			vertices[1].color = quad->mColor[1];//.color;
			vertices[1].x = x;
			vertices[1].y = y + destHeight;
			vertices[1].z = 0.0f;

			vertices[3].u = start + width;
			vertices[3].v = quad->mY + quad->mHeight;
			vertices[3].color = quad->mColor[3];//.color;
			vertices[3].x = x + destWidth;
			vertices[3].y = y + destHeight;
			vertices[3].z = 0.0f;

			if (quad->mVFlipped)
			{
				Swap(&vertices[0].v, &vertices[2].v);
				Swap(&vertices[1].v, &vertices[3].v);
			}

			if (angle != 0.0f)
			{
				for (int i=0;i<4;i++)
				{
					xx = (cosAngle*(vertices[i].x-xo) - sinAngle*(vertices[i].y-yo) + xo);
					yy = (sinAngle*(vertices[i].x-xo) + cosAngle*(vertices[i].y-yo) + yo);
					vertices[i].x = xx;
					vertices[i].y = yy;
				}
			}

			x += destWidth;

			sceGuDrawArray(GU_TRIANGLE_STRIP,GU_TEXTURE_32BITF|TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
		}
	}
}


void JRenderer::RenderQuad(JQuad* quad, VertexColor* points)
{
  if (mCurrentTextureFormat != quad->mTex->mTextureFormat){
    mCurrentTextureFormat = quad->mTex->mTextureFormat;
    sceGuTexMode(mCurrentTextureFormat, 0, 0, mSwizzle);
  }

	if (mCurrentTex != quad->mTex->mTexId)
	{
		sceGuTexImage(0, quad->mTex->mTexWidth, quad->mTex->mTexHeight, quad->mTex->mTexWidth, quad->mTex->mBits);
		mCurrentTex = quad->mTex->mTexId;
	}

	if (mCurrentBlend != quad->mBlend)
	{
		sceGuTexFunc(quad->mBlend, GU_TCC_RGBA);
		mCurrentBlend = quad->mBlend;
	}



	// allocate memory on the current display list for temporary storage
	// in order to rotate, we use 4 vertices this time
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(4 * sizeof(struct Vertex));

	vertices[0].u = quad->mX;
	vertices[0].v = quad->mY;

	vertices[1].u = quad->mX;
	vertices[1].v = quad->mY + quad->mHeight;

	vertices[2].u = quad->mX + quad->mWidth;
	vertices[2].v = quad->mY;

	vertices[3].u = quad->mX + quad->mWidth;
	vertices[3].v = quad->mY + quad->mHeight;

	vertices[0].color = points[3].color;
	vertices[0].x = points[3].x;
	vertices[0].y = points[3].y;
	vertices[0].z = points[3].z;

	vertices[1].color = points[0].color;
	vertices[1].x = points[0].x;
	vertices[1].y = points[0].y;
	vertices[1].z = points[0].z;

	vertices[2].color = points[2].color;
	vertices[2].x = points[2].x;
	vertices[2].y = points[2].y;
	vertices[2].z = points[2].z;

	vertices[3].color = points[1].color;
	vertices[3].x = points[1].x;
	vertices[3].y = points[1].y;
	vertices[3].z = points[1].z;


	sceGuDrawArray(GU_TRIANGLE_STRIP,GU_TEXTURE_32BITF|TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);


}


//------------------------------------------------------------------------------------------------
// Taken from:
// http://svn.ps2dev.org/filedetails.php?repname=psp&path=/trunk/libpng/screenshot/main.c&rev=0&sc=0
// Save current visible screen as PNG
//------------------------------------------------------------------------------------------------
void JRenderer::ScreenShot(const char* filename)
{
        u32* vram32;
        u16* vram16;
        void* temp;
        int bufferwidth;
        int pixelformat;
        int i, x, y;
        png_structp png_ptr;
        png_infop info_ptr;
        FILE* fp;
        u8* line;

        fp = fopen(filename, "wb");
        if (!fp) return;
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) return;
        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
                png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
                fclose(fp);
                return;
        }
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, SCREEN_WIDTH, SCREEN_HEIGHT,
                8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);
        line = (u8*) malloc(SCREEN_WIDTH * 3);
        sceDisplayWaitVblankStart();  // if framebuf was set with PSP_DISPLAY_SETBUF_NEXTFRAME, wait until it is changed
        sceDisplayGetFrameBuf(&temp, &bufferwidth, &pixelformat, PSP_DISPLAY_SETBUF_NEXTFRAME);
		//temp = (void*)(0x04000000+0x40000000);
        vram32 = (u32*) temp;
        vram16 = (u16*) vram32;
        for (y = 0; y < SCREEN_HEIGHT; y++) {
                for (i = 0, x = 0; x < SCREEN_WIDTH; x++) {
                        u32 color = 0;
                        u8 r = 0, g = 0, b = 0;
                        switch (pixelformat) {
                                case PSP_DISPLAY_PIXEL_FORMAT_565:
                                        color = vram16[x + y * bufferwidth];
                                        r = (color & 0x1f) << 3;
                                        g = ((color >> 5) & 0x3f) << 2 ;
                                        b = ((color >> 11) & 0x1f) << 3 ;
                                        break;
                                case PSP_DISPLAY_PIXEL_FORMAT_5551:
                                        color = vram16[x + y * bufferwidth];
                                        r = (color & 0x1f) << 3;
                                        g = ((color >> 5) & 0x1f) << 3 ;
                                        b = ((color >> 10) & 0x1f) << 3 ;
                                        break;
                                case PSP_DISPLAY_PIXEL_FORMAT_4444:
                                        color = vram16[x + y * bufferwidth];
                                        r = (color & 0xf) << 4;
                                        g = ((color >> 4) & 0xf) << 4 ;
                                        b = ((color >> 8) & 0xf) << 4 ;
                                        break;
                                case PSP_DISPLAY_PIXEL_FORMAT_8888:
                                        color = vram32[x + y * bufferwidth];
                                        r = color & 0xff;
                                        g = (color >> 8) & 0xff;
                                        b = (color >> 16) & 0xff;
                                        break;
                        }
                        line[i++] = r;
                        line[i++] = g;
                        line[i++] = b;
                }
                png_write_row(png_ptr, line);
        }
        free(line);
        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        fclose(fp);
}


static void PNGCustomWarningFn(png_structp png_ptr, png_const_charp warning_msg)
{
	JLOG("PNG error callback fired!");
	JLOG(warning_msg);
}

static void PNGCustomReadDataFn(png_structp png_ptr, png_bytep data, png_size_t length)
{
   png_size_t check;

   JFileSystem *fileSystem = (JFileSystem*)png_ptr->io_ptr;

   check = fileSystem->ReadFile(data, length);

   if (check != length)
   {
      png_error(png_ptr, "Read Error!");
   }
}



static int getNextPower2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

/*
** Alternate swizzle function that can handle any number of lines (as opposed to swizzle_fast, which is
** hardcoded to do 8 at a time)
*/
static void swizzle_lines(const u8* inSrc, u8* inDst, unsigned int inWidth, unsigned int inLines)
{
	unsigned int rowblocks = (inWidth * sizeof(u32) / 16);
	for (unsigned int j = 0; j < inLines; ++j)
	{
		for (unsigned int i = 0; i < inWidth * sizeof(u32); ++i)
		{
			unsigned int blockx = i / 16;
			unsigned int blocky = j / 8;
	 
			unsigned int x = (i - blockx * 16);
			unsigned int y = (j - blocky * 8);
			unsigned int block_index = blockx + ((blocky) * rowblocks);
			unsigned int block_address = block_index * 16 * 8;
	 
			inDst[block_address + x + y * 16] = inSrc[i + j * inWidth * sizeof(u32)];
		}
	}
}


typedef u32* u32_ptr;
static void swizzle_row(const u8* inSrc, u32_ptr& inDst, unsigned int inBlockWidth, unsigned int inPitch)
{
  for (unsigned int blockx = 0; blockx < inBlockWidth; ++blockx)
  {
		const u32* src = (u32*)inSrc;
    for (unsigned int j = 0; j < 8; ++j)
    {
      *(inDst++) = *(src++);
      *(inDst++) = *(src++);
      *(inDst++) = *(src++);
      *(inDst++) = *(src++);
      src += inPitch;
    }
    inSrc += 16;
  }
}


static void swizzle_fast(u8* out, const u8* in, unsigned int width, unsigned int height)
{
   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);

   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;

   const u8* ysrc = in;
   u32* dst = (u32*)out;

   for (unsigned int blocky = 0; blocky < height_blocks; ++blocky)
   {
      const u8* xsrc = ysrc;
			swizzle_row(xsrc, dst, width_blocks, src_pitch);
     	ysrc += src_row;
   }
}


static void jpg_null(j_decompress_ptr cinfo)
{
}

static boolean jpg_fill_input_buffer(j_decompress_ptr cinfo)
{
	return 1;
}

static void jpg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{

	cinfo->src->next_input_byte += (size_t) num_bytes;
	cinfo->src->bytes_in_buffer -= (size_t) num_bytes;

}

static void jpeg_mem_src(j_decompress_ptr cinfo, u8 *mem, int len)
{
	cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
	cinfo->src->init_source = jpg_null;
	cinfo->src->fill_input_buffer = jpg_fill_input_buffer;
	cinfo->src->skip_input_data = jpg_skip_input_data;
	cinfo->src->resync_to_restart = jpeg_resync_to_restart;
	cinfo->src->term_source = jpg_null;
	cinfo->src->bytes_in_buffer = len;
	cinfo->src->next_input_byte = mem;
}


int JRenderer::PixelSize(int textureMode){
 switch (textureMode) {
      case GU_PSM_5650:
      case GU_PSM_5551:
      case GU_PSM_4444:
             return 2;
      case GU_PSM_8888:
              return 4;
       }
 return PIXEL_SIZE;
}

void JRenderer::LoadJPG(TextureInfo &textureInfo, const char *filename, int mode, int textureMode)
{
  JLOG("JRenderer::LoadJPG");
	textureInfo.mBits = NULL;
	char filenamenew[4096];
	sprintf(filenamenew, JGE_GET_RES(filename).c_str());

	bool useVideoRAM = (mode == TEX_TYPE_USE_VRAM);

	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr jerr;
	u8 *rawdata, *scanline, *p;
  u16 *rgbadata16, *q16, *bits16;
  u32 *rgbadata32, *q32, *bits32;
	int	rawsize, i;
  int pixelSize = PixelSize(textureMode);
  bits16 = NULL;
  bits32 = NULL;

        JFileSystem* fileSystem = JFileSystem::GetInstance();
        if (!fileSystem->OpenFile(filename))
        {
                return;
        }

        rawsize = fileSystem->GetFileSize();

        rawdata = new u8[rawsize];

        if (!rawdata)
        {
                fileSystem->CloseFile();
                return;
        }

        fileSystem->ReadFile(rawdata, rawsize);
        fileSystem->CloseFile();


        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);

        jpeg_mem_src(&cinfo, rawdata, rawsize);


	jpeg_read_header(&cinfo, true);

	jpeg_start_decompress(&cinfo);

	if(cinfo.output_components != 3 && cinfo.output_components != 4)
	{
		jpeg_destroy_decompress(&cinfo);
		return;
	}

	int tw = getNextPower2(cinfo.output_width);
	int th = getNextPower2(cinfo.output_height);


	bool videoRAMUsed = false;

	int size = tw * th * pixelSize;

	if (useVideoRAM)
	{

    if (pixelSize == 2){
		  bits16 = (u16*)valloc(size);
    }else{
      bits32 = (u32*)valloc(size);
    }
		videoRAMUsed = true;
	}

	//else
	if (bits16 == NULL && bits32 == NULL)
	{
		videoRAMUsed = false;
    if (pixelSize == 2){
		  bits16 = (u16*)memalign(16, size);
    }else{
		  bits32 = (u32*)memalign(16, size);
    }
	}


  rgbadata16 = bits16;
  rgbadata32 = bits32;
	if (mSwizzle)
	{
    if (rgbadata16) rgbadata16 = (u16*) memalign(16, size);
    if (rgbadata32) rgbadata32 = (u32*) memalign(16, size);
		if(!rgbadata16 && !rgbadata32)
		{
			jpeg_destroy_decompress(&cinfo);
      if (videoRAMUsed){
				if (bits16) vfree(bits16);
        if (bits32) vfree(bits32);
      }else{
				if (bits16) free(bits16);
        if (bits32) free(bits32);
      }
			return;
		}
	}

	scanline = (u8 *)malloc(cinfo.output_width * 3);
	if(!scanline)
	{
		jpeg_destroy_decompress(&cinfo);

    if (videoRAMUsed){
				if (bits16) vfree(bits16);
        if (bits32) vfree(bits32);
    }else{
				if (bits16) free(bits16);
        if (bits32) free(bits32);
    }
    if (mSwizzle){
			if (rgbadata16)
				free(rgbadata16);
			if (rgbadata32)
				free(rgbadata32);
    }
		return;
	}

	u16 * currRow16 = rgbadata16;
  u32 * currRow32 = rgbadata32;
  u16 color16;
  u32 color32;
	while(cinfo.output_scanline < cinfo.output_height)
	{
		p = scanline;
		jpeg_read_scanlines(&cinfo, &scanline, 1);

		q16 = currRow16;
    q32 = currRow32;
		for(i=0; i<(int)cinfo.output_width; i++){
      int a = 255;
      int r = p[0];
      int g = p[1];
      int b = p[2];
      switch (textureMode) {
      case GU_PSM_5650:
              color16 = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
              *(q16) = color16;
              break;
      case GU_PSM_5551:
              color16 = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | ((a >> 7) << 15);
              *(q16) = color16;
              break;
      case GU_PSM_4444:
              color16 = (r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8) | ((a >> 4) << 12);
              *(q16) = color16;
              break;
      case GU_PSM_8888:
              color32 = r | (g << 8) | (b << 16) | (a << 24);
              *(q32) = color32;
              break;
       }

			p+=3; 
      if (q16) q16+=1;
      if (q32) q32+=1;
		}
		if (currRow32)  currRow32+= tw;
    if (currRow16)  currRow16+= tw;
	}

	free(scanline);

	try{
    jpeg_finish_decompress(&cinfo);
  }catch(...){}


	if (mSwizzle)
	{
    if (rgbadata16){
		  swizzle_fast((u8*)bits16, (const u8*)rgbadata16, tw*pixelSize, th/*cinfo.output_height*/);
		  free (rgbadata16);
    }
    if (rgbadata32){
		  swizzle_fast((u8*)bits32, (const u8*)rgbadata32, tw*pixelSize, th/*cinfo.output_height*/);
		  free (rgbadata32);
    }
	}



	if (bits16) textureInfo.mBits = (u8 *)bits16;
  else textureInfo.mBits = (u8 *)bits32;
	textureInfo.mWidth = cinfo.output_width;
	textureInfo.mHeight = cinfo.output_height;
	textureInfo.mTexWidth = tw;
	textureInfo.mTexHeight = th;
	textureInfo.mVRAM =videoRAMUsed;

	jpeg_destroy_decompress(&cinfo);
  delete [] rawdata;
  JLOG("-- OK  -- JRenderer::LoadJPG");
}


JTexture* JRenderer::LoadTexture(const char* filename, int mode, int textureMode)
{
  JLOG("JRenderer::LoadTexture");
	TextureInfo textureInfo;
	textureInfo.mVRAM = false;
	textureInfo.mBits = NULL;

  int ret = 0;

	if (strstr(filename, ".jpg")!=NULL || strstr(filename, ".JPG")!=NULL)
		LoadJPG(textureInfo, filename, mode, textureMode);
  else if(strstr(filename, ".gif")!=NULL || strstr(filename, ".GIF")!=NULL) {
    textureMode = TEXTURE_FORMAT; //textureMode not supported in Gif yet
		LoadGIF(textureInfo,filename, mode, textureMode);
  }
  else if(strstr(filename, ".png")!=NULL || strstr(filename, ".PNG")!=NULL) {
    textureMode = TEXTURE_FORMAT; //textureMode not supported in PNG yet
		ret = LoadPNG(textureInfo, filename, mode, textureMode);
    if (ret < 0) {
      char buf[512];
      sprintf(buf, "--LoadPNG sent error code: %i for file %s", ret, filename);
      JLOG(buf);
    }
  }

	if (textureInfo.mBits == NULL)
		return NULL;


	bool done = false;
	JLOG("Allocating Texture");
	JTexture* tex = new JTexture();
	if (tex)
	{
		if (mImageFilter != NULL)
			mImageFilter->ProcessImage((PIXEL_TYPE*)textureInfo.mBits, textureInfo.mWidth, textureInfo.mHeight);

		tex->mTexId = mTexCounter++;
    tex->mTextureFormat = textureMode;
		tex->mWidth = textureInfo.mWidth;
		tex->mHeight = textureInfo.mHeight;
		tex->mTexWidth = textureInfo.mTexWidth;
		tex->mTexHeight = textureInfo.mTexHeight;
		tex->mInVideoRAM = textureInfo.mVRAM;
		tex->mBits = (PIXEL_TYPE *)textureInfo.mBits;

		done = true;

	}

	if (!done)
	{
		SAFE_DELETE(tex);
	}

  JLOG("-- OK  -- JRenderer::LoadTexture");
	return tex;

}

/*
** Helper function for LoadPNG
*/
void ReadPngLine( png_structp& png_ptr, u32_ptr& line, png_uint_32 width, int pixelformat, u16* p16, u32* p32 ) 
{
  png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
  for (int x = 0; x < (int)width; ++x)
  {
    u32 color32 = line[x];
    u16 color16;
    int a = (color32 >> 24) & 0xff;
    int r = color32 & 0xff;
    int g = (color32 >> 8) & 0xff;
    int b = (color32 >> 16) & 0xff;
    switch (pixelformat) {
    case PSP_DISPLAY_PIXEL_FORMAT_565:
      color16 = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
      *(p16+x) = color16;
      break;
    case PSP_DISPLAY_PIXEL_FORMAT_5551:
      color16 = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | ((a >> 7) << 15);
      *(p16+x) = color16;
      break;
    case PSP_DISPLAY_PIXEL_FORMAT_4444:
      color16 = (r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8) | ((a >> 4) << 12);
      *(p16+x) = color16;
      break;
    case PSP_DISPLAY_PIXEL_FORMAT_8888:
      color32 = r | (g << 8) | (b << 16) | (a << 24);
      *(p32+x) = color32;
      break;
    }
  }	
}



//------------------------------------------------------------------------------------------------
// Based on:
// http://svn.ps2dev.org/filedetails.php?repname=psp&path=/trunk/libpng/screenshot/main.c&rev=0&sc=0
// Load PNG as texture
//------------------------------------------------------------------------------------------------
int JRenderer::LoadPNG(TextureInfo &textureInfo, const char* filename, int mode, int textureMode)
{
  JLOG("JRenderer::LoadPNG: ");
  JLOG(filename);
  textureInfo.mBits = NULL;

  bool useVideoRAM = (mode == TEX_TYPE_USE_VRAM);
  int pixelformat = PIXEL_FORMAT;

  u32* p32;
  u16* p16;
  png_structp png_ptr;
  png_infop info_ptr;
  unsigned int sig_read = 0;
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  u32* line;

  JFileSystem* fileSystem = JFileSystem::GetInstance();
  if (!fileSystem->OpenFile(filename)) return JGE_ERR_CANT_OPEN_FILE;

  JLOG("PNG opened - creating read struct");
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fileSystem->CloseFile();
    return JGE_ERR_PNG;
  }
  JLOG("Setting error callback func");
  png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, PNGCustomWarningFn);
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fileSystem->CloseFile();
    png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
    return JGE_ERR_PNG;
  }
  png_init_io(png_ptr, NULL);
  png_set_read_fn(png_ptr, (png_voidp)fileSystem, PNGCustomReadDataFn);

  png_set_sig_bytes(png_ptr, sig_read);
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
  png_set_strip_16(png_ptr);
  png_set_packing(png_ptr);
  if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
  png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
  line = (u32*) malloc(width * 4);
  if (!line) {
    fileSystem->CloseFile();
    png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
    return JGE_ERR_MALLOC_FAILED;
  }

  int texWidth = getNextPower2(width);
  int texHeight = getNextPower2(height);

  bool done = false;
  PIXEL_TYPE* bits = NULL;
  bool videoRAMUsed = false;
  int size = texWidth * texHeight * sizeof(PIXEL_TYPE);

  {
    if (useVideoRAM)
    {
      bits = (PIXEL_TYPE*) valloc(size);
      videoRAMUsed = true;
    }

    if (bits == NULL)
    {
      videoRAMUsed = false;
      bits = (PIXEL_TYPE*) memalign(16, size);
    }

    PIXEL_TYPE* buffer = bits;
    const unsigned int kVerticalBlockSize = 8;
    if (mSwizzle)
    {
      JLOG("allocating swizzle buffer");
      buffer = (PIXEL_TYPE*) memalign(16, texWidth * kVerticalBlockSize * sizeof(PIXEL_TYPE));
      if (!buffer)
      {
        JLOG("failed to allocate destination swizzle buffer!");
        std::ostringstream stream;
        stream << "Alloc failed for: Tex Width: " << texWidth << " Tex Height: " << kVerticalBlockSize << ", total bytes: " << texWidth * kVerticalBlockSize * sizeof(PIXEL_TYPE);
        JLOG(stream.str().c_str());
        fileSystem->CloseFile();
        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return JGE_ERR_MALLOC_FAILED;
      }
    }

    if (buffer)
    {
  		unsigned int src_row = texWidth * 8;
      u32* dst = (u32*)bits;
      u32* ysrc = (u32*) buffer;
      unsigned int totalVerticalBlocksToProcess = height / kVerticalBlockSize;

      p32 = (u32*) buffer;
      p16 = (u16*) p32;
      for (unsigned int block = 0; block < totalVerticalBlocksToProcess; ++block)
      {
        for (unsigned int y = 0; y < kVerticalBlockSize; ++y)
        {
          ReadPngLine(png_ptr, line, width, pixelformat, p16, p32);

          p32 += texWidth;
          p16 += texWidth;
        }

				if (mSwizzle)
				{
					swizzle_fast((u8*) dst, (const u8*) buffer, texWidth * sizeof(PIXEL_TYPE), kVerticalBlockSize);
					dst += src_row;

          // if we're swizzling, reset the read pointers to the top of the buffer, as we re-read into an 8 line
          // block of memory (if we're not swizzling, we're reading directly into the destination, so we
          // want to continue iterating through)
          p32 = (u32*) buffer;
          p16 = (u16*) p32;
				}
      }

      //now the last remaining lines (ie if the height wasn't evenly divisible by 8)
      {
        if (mSwizzle)
        {
          //clear the conversion buffer so that leftover scan lines are transparent
          memset(buffer, 255, texWidth * kVerticalBlockSize * sizeof(PIXEL_TYPE));
        }
        unsigned int remainingLines = height % kVerticalBlockSize;
        for (unsigned int y = 0; y < remainingLines; ++y)
        {
          ReadPngLine(png_ptr, line, width, pixelformat, p16, p32);

          p32 += texWidth;
          p16 += texWidth;
        }

        if (mSwizzle)
        {
          // swizzle_fast only can handle eight lines at a time, and will overrun memory in our destination,
					// which only has remainingLines to fill - use the swizzle_lines function instead
					swizzle_lines((const u8*) buffer, (u8*) dst, texWidth, remainingLines);
        }
      }

      free(buffer);
      done = true;
    }
  }

  JLOG("Freeing line");
  free (line);
  JLOG("Reading end");
  png_read_end(png_ptr, info_ptr);
  JLOG("Destroying read struct");
  png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

  JLOG("Closing PNG");
  fileSystem->CloseFile();

  if (done)
  {
    textureInfo.mBits = (u8 *)bits;
    textureInfo.mWidth = width;
    textureInfo.mHeight = height;
    textureInfo.mTexWidth = texWidth;
    textureInfo.mTexHeight = texHeight;
    textureInfo.mVRAM = videoRAMUsed;
    JLOG("-- OK -- JRenderer::LoadPNG");
    return 1;

  }
  else
  {
    JLOG("LoadPNG failure - deallocating bits");
    textureInfo.mBits = NULL;

    if (videoRAMUsed)
      vfree(bits);
    else
      free(bits);
    return JGE_ERR_GENERIC;
  }

}




//////////////////////////////////////////////////////////////////////////
/// GIF Support
int JRenderer::image_readgif(void * handle, TextureInfo &textureInfo, DWORD * bgcolor, InputFunc readFunc,int mode, int textureMode)
{
	bool useVideoRAM = (mode == TEX_TYPE_USE_VRAM);
	//	pixel ** image_data=NULL;
	DWORD *p32=NULL;
	//#define gif_color(c) RGB(palette->Colors[c].Red, palette->Colors[c].Green, palette->Colors[c].Blue)
#define gif_color32(c) ARGB(255,palette->Colors[c].Red,palette->Colors[c].Green,  palette->Colors[c].Blue)
	GifRecordType RecordType;
	GifByteType *Extension;
	GifRowType LineIn = NULL;
	GifFileType *GifFileIn = NULL;
	ColorMapObject *palette;
	int ExtCode;
	int i, j;
	if ((GifFileIn = DGifOpen(handle, readFunc)) == NULL)
		return 1;
	*bgcolor = 0;
	textureInfo.mWidth = 0;
	textureInfo.mHeight = 0;
	//*image_data = NULL;
	bool videoRAMUsed = false;
	int size = 0;

	do {
		if (DGifGetRecordType(GifFileIn, &RecordType) == GIF_ERROR)
		{
			DGifCloseFile(GifFileIn);
			return 1;
		}

		switch (RecordType) {
			case IMAGE_DESC_RECORD_TYPE:
				{
					if (DGifGetImageDesc(GifFileIn) == GIF_ERROR)
					{
						DGifCloseFile(GifFileIn);
						return 1;
					}
					if((palette = (GifFileIn->SColorMap != NULL) ? GifFileIn->SColorMap : GifFileIn->Image.ColorMap) == NULL)
					{
						DGifCloseFile(GifFileIn);
						return 1;
					}
					textureInfo.mWidth = GifFileIn->Image.Width;
					textureInfo.mHeight = GifFileIn->Image.Height;
					*bgcolor = gif_color32(GifFileIn->SBackGroundColor);
					if((LineIn = (GifRowType) malloc(GifFileIn->Image.Width * sizeof(GifPixelType))) == NULL)
					{
						DGifCloseFile(GifFileIn);
						return 1;
					}

					//---------------------------------------
					textureInfo.mTexWidth = getNextPower2(GifFileIn->Image.Width);
					textureInfo.mTexHeight = getNextPower2(GifFileIn->Image.Height);

					bool done = false;
					PIXEL_TYPE* bits = NULL;

					size = textureInfo.mTexWidth * textureInfo.mTexHeight * sizeof(PIXEL_TYPE);

					if (useVideoRAM)// && (mCurrentPointer+size)<0x200000)
					{
						//bits = (PIXEL_TYPE*) (0x04000000+0x40000000+mCurrentPointer);
						//mCurrentPointer += size;

						bits = (PIXEL_TYPE*) valloc(size);
						videoRAMUsed = true;
					}
					//else

					if (bits == NULL)
					{
						videoRAMUsed = false;
						bits = (PIXEL_TYPE*) memalign(16, size);
					}

					PIXEL_TYPE* buffer = bits;

					if (mSwizzle)
						buffer = (PIXEL_TYPE*) memalign(16, textureInfo.mTexWidth * textureInfo.mTexHeight * sizeof(PIXEL_TYPE));

					if (buffer)
					{
						p32 = (DWORD*) buffer;
					}

					//if((*image_data = (pixel *)malloc(sizeof(pixel) * GifFileIn->Image.Width * GifFileIn->Image.Height)) == NULL)
					if(p32  == NULL)
					{
						free((void *)LineIn);
						DGifCloseFile(GifFileIn);
						return 1;
					}

					DWORD * curr = p32;
					DWORD * imgdata = p32;
					for (i = 0; i < GifFileIn->Image.Height; i ++)
					{
						imgdata = curr;
						if (DGifGetLine(GifFileIn, LineIn, GifFileIn->Image.Width) == GIF_ERROR)
						{
							if (videoRAMUsed)
								vfree(bits);
							else
								free(bits);

							if (mSwizzle)
								free((void *)p32);
							free((void *)LineIn);
							DGifCloseFile(GifFileIn);
							return 1;
						}
						for(j = 0; j < GifFileIn->Image.Width; j ++)
						{
							DWORD color32 = gif_color32(LineIn[j]);


							*imgdata++ = color32;
						}

						curr += textureInfo.mTexWidth;
					}

					if (mSwizzle)
					{
						swizzle_fast((u8*)bits, (const u8*)buffer, textureInfo.mTexWidth*sizeof(PIXEL_TYPE), textureInfo.mTexHeight/*GifFileIn->Image.Height*/);
						free (buffer);
					}

					done = true;

					textureInfo.mBits = (u8 *)bits;
					textureInfo.mVRAM = videoRAMUsed;
					break;
				}
			case EXTENSION_RECORD_TYPE:
				if (DGifGetExtension(GifFileIn, &ExtCode, &Extension) == GIF_ERROR)
				{
					if(textureInfo.mBits != NULL)
					{
						if (videoRAMUsed)
							vfree(textureInfo.mBits);
						else
							free((void *)textureInfo.mBits);
						textureInfo.mBits = NULL;
					}
					if(LineIn != NULL)
						free((void *)LineIn);
					DGifCloseFile(GifFileIn);
					return 1;
				}
				while (Extension != NULL) {
					if (DGifGetExtensionNext(GifFileIn, &Extension) == GIF_ERROR)
					{
						if(textureInfo.mBits != NULL)
						{
							if (videoRAMUsed)
								vfree(textureInfo.mBits);
							else
								free((void *)textureInfo.mBits);
							textureInfo.mBits = NULL;
						}
						if(LineIn != NULL)
							free((void *)LineIn);
						DGifCloseFile(GifFileIn);
						return 1;
					}
				}
				break;
			case TERMINATE_RECORD_TYPE:
				break;
			default:
				break;
		}
	}
	while (RecordType != TERMINATE_RECORD_TYPE);

	if(LineIn != NULL)
		free((void *)LineIn);
	DGifCloseFile(GifFileIn);

	return 0;
}

int image_gif_read(GifFileType * ft, GifByteType * buf, int size)
{

	JFileSystem *fileSys = (JFileSystem *)ft->UserData;
	if (fileSys->ReadFile(buf, size))
		return size;
	else
		return 0;

}

void JRenderer::LoadGIF(TextureInfo &textureInfo, const char *filename, int mode, int textureMode)
{
	textureInfo.mBits = NULL;


	JFileSystem *fileSys = JFileSystem::GetInstance();
	if (!fileSys->OpenFile(filename))
		return;

	DWORD bkcol;
	int result = image_readgif(fileSys, textureInfo, &bkcol, image_gif_read, mode);

	if(result!=0)
		textureInfo.mBits=NULL;
	fileSys->CloseFile();

	return ;
}



JTexture* JRenderer::CreateTexture(int width, int height, int mode)
{
	bool useVideoRAM = (mode == TEX_TYPE_USE_VRAM);

	JTexture* tex = new JTexture();
    if (tex)
    {
		tex->mWidth = width;
		tex->mHeight = height;


		tex->mTexWidth = getNextPower2(width);
		tex->mTexHeight = getNextPower2(height);

		int size = tex->mTexWidth * tex->mTexHeight * sizeof(PIXEL_TYPE);
		if (useVideoRAM)
		{
			tex->mInVideoRAM = true;
			tex->mBits = (PIXEL_TYPE*) valloc(size);
		}

		//else
		if (tex->mBits == NULL)
		{
			tex->mInVideoRAM = false;
			tex->mBits = (PIXEL_TYPE*) memalign(16, size);
		}

		memset(tex->mBits, 0, size);

		tex->mTexId = mTexCounter++;
	}

	return tex;
}


void JRenderer::BindTexture(JTexture *tex)
{
	if (mCurrentTex != tex->mTexId)
	{
		sceGuTexImage(0, tex->mTexWidth, tex->mTexHeight, tex->mTexWidth, tex->mBits);
		mCurrentTex = tex->mTexId;

		if (m3DEnabled)
		{
			if (mCurrentRenderMode == MODE_3D)
			{
				sceKernelDcacheWritebackAll();
				sceGuTexFunc(GU_TFX_ADD,GU_TCC_RGB);
			}
		}
	}

}


//////////////////////////////////////////////////////////////////////////
void JRenderer::Enable2D()
{

	mCurrentRenderMode = MODE_2D;

	sceGuDisable(GU_DEPTH_TEST);

}


//////////////////////////////////////////////////////////////////////////
void JRenderer::Enable3D()
{
	mCurrentRenderMode = MODE_3D;

	mCurrentBlend = -1;

	sceGuEnable(GU_DEPTH_TEST);

	LoadIdentity();
}


//////////////////////////////////////////////////////////////////////////
void JRenderer::SetClip(int x, int y, int width, int height)
{
	sceGuScissor(x, y, width, height);
}


void JRenderer::LoadIdentity()
{
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
}


void JRenderer::Translate(float x, float y, float z)
{
	ScePspFVector3 pos = { x, y, z };
	sceGumTranslate(&pos);
}


void JRenderer::RotateX(float angle)
{
	sceGumRotateX(angle);
}


void JRenderer::RotateY(float angle)
{
	sceGumRotateY(angle);
}


void JRenderer::RotateZ(float angle)
{
	sceGumRotateZ(angle);
}


void JRenderer::PushMatrix()
{
	sceGumPushMatrix();
}


void JRenderer::PopMatrix()
{
	sceGumPopMatrix();
}

void JRenderer::RenderTriangles(JTexture* texture, Vertex3D *tris, int start, int count)
{
	if (texture)
		BindTexture(texture);

    PSPVertex3D* vertices = (PSPVertex3D*) sceGuGetMemory(count * 3 * sizeof(PSPVertex3D));

    int n = 0;
	int index = start*3;
	for (int i = 0; i < count; i++)
	{
		vertices[n].texture.x = tris[index].u;
		vertices[n].texture.y = tris[index].v;

		vertices[n].pos.x = tris[index].x;
		vertices[n].pos.y = tris[index].y;
		vertices[n].pos.z = tris[index].z;


		index++;
		n++;

		vertices[n].texture.x = tris[index].u;
		vertices[n].texture.y = tris[index].v;

		vertices[n].pos.x = tris[index].x;
		vertices[n].pos.y = tris[index].y;
		vertices[n].pos.z = tris[index].z;


		index++;
		n++;

		vertices[n].texture.x = tris[index].u;
		vertices[n].texture.y = tris[index].v;

		vertices[n].pos.x = tris[index].x;
		vertices[n].pos.y = tris[index].y;
		vertices[n].pos.z = tris[index].z;


		index++;
		n++;

	}

	sceGuColor(0xff000000);
	sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_3D,count*3,0,vertices);

}

void JRenderer::SetFOV(float fov)
{
	mFOV = fov;

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(mFOV,16.0f/9.0f,0.5f,1000.0f);
}

void JRenderer::FillPolygon(float* x, float* y, int count, PIXEL_TYPE color)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(count * sizeof(struct VertexColor));

	for(int i=0; i<count; i++)
	{
		vertices[i].color = color;
		vertices[i].x = x[i];
		vertices[i].y = y[i];
		vertices[i].z = 0.0f;
	}

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_FAN, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, count, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}


void JRenderer::DrawPolygon(float* x, float* y, int count, PIXEL_TYPE color)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory((count+1) * sizeof(struct VertexColor));

	for(int i=0; i<count; i++)
	{
		vertices[i].color = color;
		vertices[i].x = x[i];
		vertices[i].y = y[i];
		vertices[i].z = 0.0f;
	}

	vertices[count].color = color;
	vertices[count].x = x[0];
	vertices[count].y = y[0];
	vertices[count].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_LINE_STRIP, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, count+1, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}


void JRenderer::DrawLine(float x1, float y1, float x2, float y2, float lineWidth, PIXEL_TYPE color)
{
	float dy=y2-y1;
	float dx=x2-x1;
	if(dy==0 && dx==0)
		return;

	float l=(float)hypot(dx,dy);

	float x[4];
	float y[4];

	x[0]=x1+lineWidth*(y2-y1)/l;
	y[0]=y1-lineWidth*(x2-x1)/l;

	x[1]=x1-lineWidth*(y2-y1)/l;
	y[1]=y1+lineWidth*(x2-x1)/l;

	x[2]=x2-lineWidth*(y2-y1)/l;
	y[2]=y2+lineWidth*(x2-x1)/l;

	x[3]=x2+lineWidth*(y2-y1)/l;
	y[3]=y2-lineWidth*(x2-x1)/l;

	FillPolygon(x, y, 4, color);
}

void JRenderer::DrawCircle(float x, float y, float radius, PIXEL_TYPE color)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(181 * sizeof(struct VertexColor));

	int angle = 359;
	for(int i=0; i<180; i++)
	{
		vertices[i].color = color;
		vertices[i].x = x+radius*COSF(angle);
		vertices[i].y = y+radius*SINF(angle);
		vertices[i].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	vertices[180].color = color;
	vertices[180].x = x+radius*COSF(0);
	vertices[180].y = y+radius*SINF(0);
	vertices[180].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_LINE_STRIP, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 181, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

void JRenderer::FillCircle(float x, float y, float radius, PIXEL_TYPE color)
{
	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(182 * sizeof(struct VertexColor));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	int angle = 359;
	for(int i=0; i<180; i++)
	{
		vertices[i+1].color = color;
		vertices[i+1].x = x+radius*COSF(angle);
		vertices[i+1].y = y+radius*SINF(angle);
		vertices[i+1].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	vertices[181].color = color;
	vertices[181].x = x+radius*COSF(359);
	vertices[181].y = y+radius*SINF(359);
	vertices[181].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_FAN, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 182, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

void JRenderer::DrawPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color)
{
	float angle = startingAngle*RAD2DEG;
	float firstAngle = angle;
	float steps = 360.0f/count;
	size /= 2;

	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory((count+1) * sizeof(struct VertexColor));

	for(int i=0; i<count; i++)
	{
		vertices[i].color = color;
		vertices[i].x = x+size*COSF((int)angle);
		vertices[i].y = y+size*SINF((int)angle);
		vertices[i].z = 0.0f;
		angle -= steps;
		if (angle < 0.0f)
			angle += 360.0f;

	}

	vertices[count].color = color;
	vertices[count].x = x+size*COSF((int)firstAngle);
	vertices[count].y = y+size*SINF((int)firstAngle);
	vertices[count].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_LINE_STRIP, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, count+1, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}


void JRenderer::FillPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color)
{
	float angle = startingAngle*RAD2DEG;
	float firstAngle = angle;
	float steps = 360.0f/count;
	size /= 2;

	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory((count+2) * sizeof(struct VertexColor));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	for(int i=0; i<count; i++)
	{
		vertices[i+1].color = color;
		vertices[i+1].x = x+size*COSF((int)angle);
		vertices[i+1].y = y+size*SINF((int)angle);
		vertices[i+1].z = 0.0f;
		angle -= steps;
		if (angle < 0.0f)
			angle += 360.0f;

	}

	vertices[count+1].color = color;
	vertices[count+1].x = x+size*COSF((int)firstAngle);
	vertices[count+1].y = y+size*SINF((int)firstAngle);
	vertices[count+1].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_FAN, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, count+2, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}


void JRenderer::DrawRoundRect( float x1,float y1, float w,float h,float radius,PIXEL_TYPE color )
{
	float x2=x1+w;
	float y2=y1+h;
	for(int i=-radius;i<y2-y1+radius; i++)
	{
		float q=radius;
		float nextq = q+1;
		if(i<0)
		{
			q=(float)sqrt(radius*radius - (-i)*(-i));
			nextq=(float)sqrt(radius*radius - (-i+1)*(-i+1));
		}
		else if (i > y2-y1)
		{
			q=(float)sqrt(radius*radius - (i-(y2-y1))*(i-(y2-y1)));
			nextq=(float)sqrt(radius*radius - (i+1-(y2-y1))*(i+1-(y2-y1)));
		}
		if (nextq == q) nextq = q+1;
		if (i==-radius || i == y2-y1+radius-1){
		  DrawLine(x1+(radius-q),y1+i+radius,x2+q+radius,y1+i+radius,color);
		}else{
		  DrawLine(x1+(radius-q),y1+i+radius,x1+(radius-nextq),y1+i+radius,color);
		  DrawLine(x2+radius+q,y1+i+radius,x2+radius+nextq,y1+i+radius,color);
		}
	}
}


void JRenderer::FillRoundRect(float x, float y, float w, float h, float radius, PIXEL_TYPE color)
{
	x+=w+radius;
	y+=radius;

	struct VertexColor* vertices = (struct VertexColor*)sceGuGetMemory(182 * sizeof(struct VertexColor));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	int angle = 359;
	for(int i=0; i<45; i++)
	{
		vertices[i+1].color = color;
		vertices[i+1].x = x+radius*COSF(angle);
		vertices[i+1].y = y+radius*SINF(angle);
		vertices[i+1].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	x-=w;

	for(int i=45; i<90; i++)
	{
		vertices[i+1].color = color;
		vertices[i+1].x = x+radius*COSF(angle);
		vertices[i+1].y = y+radius*SINF(angle);
		vertices[i+1].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	y+=h;
	for(int i=90; i<135; i++)
	{
		vertices[i+1].color = color;
		vertices[i+1].x = x+radius*COSF(angle);
		vertices[i+1].y = y+radius*SINF(angle);
		vertices[i+1].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	x+=w;
	for(int i=135; i<180; i++)
	{
		vertices[i+1].color = color;
		vertices[i+1].x = x+radius*COSF(angle);
		vertices[i+1].y = y+radius*SINF(angle);
		vertices[i+1].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	y-=h;
	vertices[181].color = color;
	vertices[181].x = x+radius*COSF(359);
	vertices[181].y = y+radius*SINF(359);
	vertices[181].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_FAN, TEXTURE_COLOR_FORMAT|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 182, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}


void JRenderer::SetImageFilter(JImageFilter* imageFilter)
{
	mImageFilter = imageFilter;
}
