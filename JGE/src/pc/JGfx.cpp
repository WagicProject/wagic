//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in root folder for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------
#define GL_GLEXT_PROTOTYPES

#if (!defined IOS)
#ifdef WIN32
  #pragma warning(disable : 4786)
  #pragma comment( lib, "giflib.lib" )
#endif

#include <png.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XMD_H
#include <jpeglib.h>

#ifdef __cplusplus
}
#endif
#endif //IOS

#include "../../include/JGE.h"
#include "../../include/JRenderer.h"
#include "../../include/JResourceManager.h"
#include "../../include/JFileSystem.h"
#include "../../include/JAssert.h"

#ifdef WIN32
#ifndef __attribute__
#define __attribute__((a))
#endif
#endif

#ifdef _DEBUG
#define checkGlError()            \
{                                 \
    GLenum glError = glGetError();  \
    if(glError != 0)                \
    printf("%s : %u : GLerror is %u\n", __FUNCTION__, __LINE__, glError); \
}
#else
#define checkGlError() (void(0))
#endif

//#define FORCE_GL2
#ifdef FORCE_GL2
// This code is to force the windows code to use GL_VERSION_2_0 even if it's not defined in the header files
// It's mostly to try to debug the shaders on Windows.
typedef GLuint (APIENTRY *_glCreateShader) (GLenum);
typedef void (APIENTRY *_glShaderSource) (GLuint, GLsizei, const char **, const GLint *);
typedef void (APIENTRY *_glShaderBinary) (GLint, const GLuint*, GLenum, const void*, GLint);
typedef void (APIENTRY *_glCompileShader) (GLuint);
typedef void (APIENTRY *_glDeleteShader) (GLuint);
typedef GLboolean (APIENTRY *_glIsShader) (GLuint);

typedef GLuint (APIENTRY *_glCreateProgram) ();
typedef void (APIENTRY *_glAttachShader) (GLuint, GLuint);
typedef void (APIENTRY *_glDetachShader) (GLuint, GLuint);
typedef void (APIENTRY *_glLinkProgram) (GLuint);
typedef void (APIENTRY *_glUseProgram) (GLuint);
typedef void (APIENTRY *_glDeleteProgram) (GLuint);
typedef GLboolean (APIENTRY *_glIsProgram) (GLuint);

typedef void (APIENTRY *_glGetShaderInfoLog) (GLuint, GLsizei, GLsizei *, char *);
typedef void (APIENTRY *_glGetShaderiv) (GLuint, GLenum, GLint *);
typedef void (APIENTRY *_glGetShaderSource) (GLuint, GLsizei, GLsizei *, char *);
typedef void (APIENTRY *_glGetProgramiv) (GLuint, GLenum, GLint *);
typedef void (APIENTRY *_glGetProgramInfoLog) (GLuint, GLsizei, GLsizei *, char *);

typedef GLuint (APIENTRY *_glGetUniformLocation) (GLuint, const char*);
typedef void (APIENTRY *_glUniform4fv) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *_glUniform3fv) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *_glUniform2fv) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *_glUniform1fv) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *_glUniform1i) (GLint, GLint);
typedef void (APIENTRY *_glUniform1iv) (GLint, GLsizei, const GLint *);
typedef void (APIENTRY *_glUniformMatrix2fv) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *_glUniformMatrix3fv) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *_glUniformMatrix4fv) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *_glUniformMatrix2x3fv) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *_glUniformMatrix2x4fv) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *_glUniformMatrix3x2fv) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *_glUniformMatrix3x4fv) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *_glUniformMatrix4x2fv) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *_glUniformMatrix4x3fv) (GLint, GLsizei, GLboolean, const GLfloat *);

typedef void (APIENTRY *_glBindAttribLocation) (GLuint, GLuint, const char *);
typedef GLint (APIENTRY *_glGetAttribLocation) (GLuint, const char *);
typedef void (APIENTRY *_glVertexAttrib1fv) (GLuint, const GLfloat *);
typedef void (APIENTRY *_glVertexAttrib2fv) (GLuint, const GLfloat *);
typedef void (APIENTRY *_glVertexAttrib3fv) (GLuint, const GLfloat *);
typedef void (APIENTRY *_glVertexAttrib4fv) (GLuint, const GLfloat *);
typedef void (APIENTRY *_glVertexAttribPointer) (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
typedef void (APIENTRY *_glDisableVertexAttribArray) (GLuint);
typedef void (APIENTRY *_glEnableVertexAttribArray) (GLuint);

typedef void (APIENTRY *_glGetProgramBinaryOES) (GLuint, GLsizei, GLsizei *, GLenum *, void *);
typedef void (APIENTRY *_glProgramBinaryOES) (GLuint, GLenum, const void *, GLint);


typedef void (APIENTRY *_glMultiTexCoord4f) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *_glActiveStencilFaceEXT) (GLenum );

typedef void (APIENTRY *_glStencilOpSeparate) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (APIENTRY *_glActiveTexture) (GLenum);
typedef void (APIENTRY *_glBlendColor) (GLclampf, GLclampf, GLclampf, GLclampf);

#define GLSLGETPROC(func)     a ## func = (_ ## func)wglGetProcAddress(#func);

struct glslFunctions
{
  void load(){
    GLSLGETPROC(glCreateShader);
    GLSLGETPROC(glShaderSource);
    GLSLGETPROC(glShaderBinary);
    GLSLGETPROC(glCompileShader);
    GLSLGETPROC(glDeleteShader);
    GLSLGETPROC(glIsShader);
    GLSLGETPROC(glCreateProgram);
    GLSLGETPROC(glAttachShader);
    GLSLGETPROC(glDetachShader);
    GLSLGETPROC(glLinkProgram);
    GLSLGETPROC(glUseProgram);
    GLSLGETPROC(glDeleteProgram);
    GLSLGETPROC(glIsProgram);
    GLSLGETPROC(glGetShaderInfoLog);
    GLSLGETPROC(glGetShaderiv);
    GLSLGETPROC(glGetShaderSource);
    GLSLGETPROC(glGetProgramiv);
    GLSLGETPROC(glGetProgramInfoLog);
    GLSLGETPROC(glGetUniformLocation);
    GLSLGETPROC(glUniform4fv);
    GLSLGETPROC(glUniform3fv);
    GLSLGETPROC(glUniform2fv);
    GLSLGETPROC(glUniform1fv);
    GLSLGETPROC(glUniform1i);
    GLSLGETPROC(glUniform1iv);
    GLSLGETPROC(glUniformMatrix2fv);
    GLSLGETPROC(glUniformMatrix3fv);
    GLSLGETPROC(glUniformMatrix4fv);
    GLSLGETPROC(glUniformMatrix2x3fv);
    GLSLGETPROC(glUniformMatrix2x4fv);
    GLSLGETPROC(glUniformMatrix3x2fv);
    GLSLGETPROC(glUniformMatrix3x4fv);
    GLSLGETPROC(glUniformMatrix4x2fv);
    GLSLGETPROC(glUniformMatrix4x3fv);
    GLSLGETPROC(glBindAttribLocation);
    GLSLGETPROC(glGetAttribLocation);
    GLSLGETPROC(glVertexAttrib1fv);
    GLSLGETPROC(glVertexAttrib2fv);
    GLSLGETPROC(glVertexAttrib3fv);
    GLSLGETPROC(glVertexAttrib4fv);
    GLSLGETPROC(glVertexAttribPointer);
    GLSLGETPROC(glDisableVertexAttribArray);
    GLSLGETPROC(glEnableVertexAttribArray);
    GLSLGETPROC(glGetProgramBinaryOES);
    GLSLGETPROC(glProgramBinaryOES);
    GLSLGETPROC(glMultiTexCoord4f);
    GLSLGETPROC(glActiveStencilFaceEXT);
    GLSLGETPROC(glStencilOpSeparate);
    GLSLGETPROC(glActiveTexture);
    GLSLGETPROC(glBlendColor);
  };

  _glCreateShader aglCreateShader;
  _glShaderSource aglShaderSource;
  _glShaderBinary aglShaderBinary;
  _glCompileShader aglCompileShader;
  _glDeleteShader aglDeleteShader;
  _glIsShader aglIsShader;
  _glCreateProgram aglCreateProgram;
  _glAttachShader aglAttachShader;
  _glDetachShader aglDetachShader;
  _glLinkProgram aglLinkProgram;
  _glUseProgram aglUseProgram;
  _glDeleteProgram aglDeleteProgram;
  _glIsProgram aglIsProgram;
  _glGetShaderInfoLog aglGetShaderInfoLog;
  _glGetShaderiv aglGetShaderiv;
  _glGetShaderSource aglGetShaderSource;
  _glGetProgramiv aglGetProgramiv;
  _glGetProgramInfoLog aglGetProgramInfoLog;

  _glGetUniformLocation aglGetUniformLocation;
  _glUniform4fv aglUniform4fv;
  _glUniform3fv aglUniform3fv;
  _glUniform2fv aglUniform2fv;
  _glUniform1fv aglUniform1fv;
  _glUniform1i aglUniform1i;
  _glUniform1iv aglUniform1iv;
  _glUniformMatrix2fv aglUniformMatrix2fv;
  _glUniformMatrix3fv aglUniformMatrix3fv;
  _glUniformMatrix4fv aglUniformMatrix4fv;
  _glUniformMatrix2x3fv aglUniformMatrix2x3fv;
  _glUniformMatrix2x4fv aglUniformMatrix2x4fv;
  _glUniformMatrix3x2fv aglUniformMatrix3x2fv;
  _glUniformMatrix3x4fv aglUniformMatrix3x4fv;
  _glUniformMatrix4x2fv aglUniformMatrix4x2fv;
  _glUniformMatrix4x3fv aglUniformMatrix4x3fv;

  _glBindAttribLocation aglBindAttribLocation;
  _glGetAttribLocation aglGetAttribLocation;
  _glVertexAttrib1fv aglVertexAttrib1fv;
  _glVertexAttrib2fv aglVertexAttrib2fv;
  _glVertexAttrib3fv aglVertexAttrib3fv;
  _glVertexAttrib4fv aglVertexAttrib4fv;
  _glVertexAttribPointer aglVertexAttribPointer;
  _glDisableVertexAttribArray aglDisableVertexAttribArray;
  _glEnableVertexAttribArray aglEnableVertexAttribArray;

  _glGetProgramBinaryOES aglGetProgramBinaryOES;
  _glProgramBinaryOES aglProgramBinaryOES;
  _glMultiTexCoord4f aglMultiTexCoord4f;
  _glActiveStencilFaceEXT aglActiveStencilFaceEXT;
  _glStencilOpSeparate aglStencilOpSeparate;
  _glActiveTexture aglActiveTexture;
  _glBlendColor aglBlendColor;
};

static glslFunctions g_glslfuncts;

#define glCreateShader            g_glslfuncts.aglCreateShader
#define glCompileShader           g_glslfuncts.aglCompileShader
#define glGetShaderiv             g_glslfuncts.aglGetShaderiv
#define glGetShaderInfoLog        g_glslfuncts.aglGetShaderInfoLog
#define glDeleteShader            g_glslfuncts.aglDeleteShader
#define glCreateProgram           g_glslfuncts.aglCreateProgram
#define glDeleteProgram           g_glslfuncts.aglDeleteProgram
#define glShaderSource            g_glslfuncts.aglShaderSource
#define glAttachShader            g_glslfuncts.aglAttachShader
#define glLinkProgram             g_glslfuncts.aglLinkProgram
#define glGetProgramiv            g_glslfuncts.aglGetProgramiv
#define glGetProgramInfoLog       g_glslfuncts.aglGetProgramInfoLog
#define glGetAttribLocation       g_glslfuncts.aglGetAttribLocation
#define glGetUniformLocation      g_glslfuncts.aglGetUniformLocation
#define glGetProgramiv            g_glslfuncts.aglGetProgramiv
#define glVertexAttribPointer     g_glslfuncts.aglVertexAttribPointer
#define glEnableVertexAttribArray g_glslfuncts.aglEnableVertexAttribArray
#define glUniform4fv              g_glslfuncts.aglUniform4fv
#define glActiveTexture           g_glslfuncts.aglActiveTexture
#define glUniform1i               g_glslfuncts.aglUniform1i
#define glUseProgram              g_glslfuncts.aglUseProgram
#define glUniformMatrix4fv        g_glslfuncts.aglUniformMatrix4fv

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_CUBE 0x8B60
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1

#define GL_VERSION_2_0
#endif

JQuad::JQuad(JTexture *tex, float x, float y, float width, float height)
		:mTex(tex), mX(x), mY(y), mWidth(width), mHeight(height)
{
	JASSERT(tex != NULL);
    JRenderer::GetInstance()->TransferTextureToGLContext(*tex);

	mHotSpotX = 0.0f;
	mHotSpotY = 0.0f;
	//mBlend = BLEND_DEFAULT;
	for (int i=0;i<4;i++)
		mColor[i].color = 0xFFFFFFFF;

	mHFlipped = false;
	mVFlipped = false;

	SetTextureRect(x, y, width, height);
}

void JQuad::SetTextureRect(float x, float y, float w, float h)
{
	mX = x;
	mY = y;
	mWidth = w;
	mHeight = h;

	mTX0 = x/mTex->mTexWidth;
	mTY0 = y/mTex->mTexHeight;
	mTX1 = (x+w)/mTex->mTexWidth;
	mTY1 = (y+h)/mTex->mTexHeight;
}


void JQuad::GetTextureRect(float *x, float *y, float *w, float *h)
{
	*x=mX; *y=mY; *w=mWidth; *h=mHeight;
}


void JQuad::SetColor(PIXEL_TYPE color)
{
	for (int i=0;i<4;i++)
		mColor[i].color = color;
}



void JQuad::SetHotSpot(float x, float y)
{
	mHotSpotX = x;
	mHotSpotY = y;
}


//////////////////////////////////////////////////////////////////////////

JTexture::JTexture() : mBuffer(NULL)
{
	mTexId = -1;
}

JTexture::~JTexture()
{
    checkGlError();
    if (mTexId != (GLuint)-1)
        glDeleteTextures(1, &mTexId);
    checkGlError();

    if (mBuffer)
    {
        delete [] mBuffer;
        mBuffer = NULL;
    }
}


void JTexture::UpdateBits(int x, int y, int width, int height, PIXEL_TYPE* bits)
{
  checkGlError();
  JRenderer::GetInstance()->BindTexture(this);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bits);
  checkGlError();
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

		JASSERT(mInstance != NULL);

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

JRenderer::JRenderer() : mActualWidth(SCREEN_WIDTH_F), mActualHeight(SCREEN_HEIGHT_F)
{
}


JRenderer::~JRenderer()
{

}

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
void esMatrixLoadIdentity(ESMatrix *result)
{
    memset(result, 0x0, sizeof(ESMatrix));
    result->m[0][0] = 1.0f;
    result->m[1][1] = 1.0f;
    result->m[2][2] = 1.0f;
    result->m[3][3] = 1.0f;
}

void esScale(ESMatrix *result, GLfloat sx, GLfloat sy, GLfloat sz)
{
    result->m[0][0] *= sx;
    result->m[0][1] *= sx;
    result->m[0][2] *= sx;
    result->m[0][3] *= sx;

    result->m[1][0] *= sy;
    result->m[1][1] *= sy;
    result->m[1][2] *= sy;
    result->m[1][3] *= sy;

    result->m[2][0] *= sz;
    result->m[2][1] *= sz;
    result->m[2][2] *= sz;
    result->m[2][3] *= sz;
}

void esTranslate(ESMatrix *result, GLfloat tx, GLfloat ty, GLfloat tz)
{
    result->m[3][0] += (result->m[0][0] * tx + result->m[1][0] * ty + result->m[2][0] * tz);
    result->m[3][1] += (result->m[0][1] * tx + result->m[1][1] * ty + result->m[2][1] * tz);
    result->m[3][2] += (result->m[0][2] * tx + result->m[1][2] * ty + result->m[2][2] * tz);
    result->m[3][3] += (result->m[0][3] * tx + result->m[1][3] * ty + result->m[2][3] * tz);
}

void esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB)
{
    ESMatrix    tmp;
    int         i;

        for (i=0; i<4; i++)
        {
                tmp.m[i][0] =   (srcA->m[i][0] * srcB->m[0][0]) +
                                                (srcA->m[i][1] * srcB->m[1][0]) +
                                                (srcA->m[i][2] * srcB->m[2][0]) +
                                                (srcA->m[i][3] * srcB->m[3][0]) ;

                tmp.m[i][1] =   (srcA->m[i][0] * srcB->m[0][1]) +
                                                (srcA->m[i][1] * srcB->m[1][1]) +
                                                (srcA->m[i][2] * srcB->m[2][1]) +
                                                (srcA->m[i][3] * srcB->m[3][1]) ;

                tmp.m[i][2] =   (srcA->m[i][0] * srcB->m[0][2]) +
                                                (srcA->m[i][1] * srcB->m[1][2]) +
                                                (srcA->m[i][2] * srcB->m[2][2]) +
                                                (srcA->m[i][3] * srcB->m[3][2]) ;

                tmp.m[i][3] =   (srcA->m[i][0] * srcB->m[0][3]) +
                                                (srcA->m[i][1] * srcB->m[1][3]) +
                                                (srcA->m[i][2] * srcB->m[2][3]) +
                                                (srcA->m[i][3] * srcB->m[3][3]) ;
        }
/*
 * Actually, srcA and srcB are column-major order matrixes, while they
 * use row-major multiplication. Then above result equals to (B * A) likes these:

  for (i=0; i<4; i++){
    tmp.m[0][i] = (srcB->m[0][i] * srcA->m[0][0]) +
                  (srcB->m[1][i] * srcA->m[0][1]) +
                  (srcB->m[2][i] * srcA->m[0][2]) +
                  (srcB->m[3][i] * srcA->m[0][3]) ;
    
    tmp.m[1][i] = (srcB->m[0][i] * srcA->m[1][0]) +
                  (srcB->m[1][i] * srcA->m[1][1]) +
                  (srcB->m[2][i] * srcA->m[1][2]) +
                  (srcB->m[3][i] * srcA->m[1][3]) ;
    
    tmp.m[2][i] = (srcB->m[0][i] * srcA->m[2][0]) +
                  (srcB->m[1][i] * srcA->m[2][1]) +
                  (srcB->m[2][i] * srcA->m[2][2]) +
                  (srcB->m[3][i] * srcA->m[2][3]) ;
    
    tmp.m[3][i] = (srcB->m[0][i] * srcA->m[3][0]) +
                  (srcB->m[1][i] * srcA->m[3][1]) +
                  (srcB->m[2][i] * srcA->m[3][2]) +
                  (srcB->m[3][i] * srcA->m[3][3]) ;
  }

 * So, it works. (Refer to math method of Homogeneous Coordinate)
 */
    memcpy(result, &tmp, sizeof(ESMatrix));
}

void esRotate(ESMatrix *result, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat sinAngle, cosAngle;
   GLfloat mag = sqrtf(x * x + y * y + z * z);

   sinAngle = sinf ( angle * M_PI / 180.0f );
   cosAngle = cosf ( angle * M_PI / 180.0f );
   if ( mag > 0.0f )
   {
      GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs;
      GLfloat oneMinusCos;
      ESMatrix rotMat;

      x /= mag;
      y /= mag;
      z /= mag;

      xx = x * x;
      yy = y * y;
      zz = z * z;
      xy = x * y;
      yz = y * z;
      zx = z * x;
      xs = x * sinAngle;
      ys = y * sinAngle;
      zs = z * sinAngle;
      oneMinusCos = 1.0f - cosAngle;

      // Note: matrixes in OpenGL ES are stored in column-major order!

      rotMat.m[0][0] = (oneMinusCos * xx) + cosAngle;
      rotMat.m[1][0] = (oneMinusCos * xy) - zs;
      rotMat.m[2][0] = (oneMinusCos * zx) + ys;
      rotMat.m[3][0] = 0.0F;

      rotMat.m[0][1] = (oneMinusCos * xy) + zs;
      rotMat.m[1][1] = (oneMinusCos * yy) + cosAngle;
      rotMat.m[2][1] = (oneMinusCos * yz) - xs;
      rotMat.m[3][1] = 0.0F;

      rotMat.m[0][2] = (oneMinusCos * zx) - ys;
      rotMat.m[1][2] = (oneMinusCos * yz) + xs;
      rotMat.m[2][2] = (oneMinusCos * zz) + cosAngle;
      rotMat.m[3][2] = 0.0F;

      rotMat.m[0][3] = 0.0F;
      rotMat.m[1][3] = 0.0F;
      rotMat.m[2][3] = 0.0F;
      rotMat.m[3][3] = 1.0F;

      esMatrixMultiply( result, &rotMat, result );
   }
}

void esOrtho(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    ESMatrix    ortho;

    if ( (deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f) )
        return;

    esMatrixLoadIdentity(&ortho);
    ortho.m[0][0] = 2.0f / deltaX;
    ortho.m[3][0] = -(right + left) / deltaX;
    ortho.m[1][1] = 2.0f / deltaY;
    ortho.m[3][1] = -(top + bottom) / deltaY;
    ortho.m[2][2] = -2.0f / deltaZ;
    ortho.m[3][2] = -(nearZ + farZ) / deltaZ;

    esMatrixMultiply(result, &ortho, result);
}

/// \brief Load a shader, check for compile errors, print error messages to output log
/// \param type Type of shader (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
/// \param shaderSrc Shader source string
/// \return A new shader object on success, 0 on failure
//
GLuint esLoadShader ( GLenum type, const char *shaderSrc )
{
  checkGlError();
   GLuint shader;
   GLint compiled;

   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
        return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );

   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled )
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         char* infoLog = (char*)malloc (sizeof(char) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         printf ( "Error compiling shader:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   checkGlError();
   return shader;

}


/// \brief Load a vertex and fragment shader, create a program object, link program.
//         Errors output to log.
/// \param vertShaderSrc Vertex shader source code
/// \param fragShaderSrc Fragment shader source code
/// \return A new program object linked with the vertex/fragment shader pair, 0 on failure
//
GLuint esLoadProgram ( const char *vertShaderSrc, const char *fragShaderSrc )
{
  checkGlError();

   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = esLoadShader ( GL_VERTEX_SHADER, vertShaderSrc );
   if ( vertexShader == 0 )
      return 0;

   fragmentShader = esLoadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );
   if ( fragmentShader == 0 )
   {
      glDeleteShader( vertexShader );
      return 0;
   }

   // Create the program object
   programObject = glCreateProgram ( );

   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked )
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         char* infoLog = (char*)malloc (sizeof(char) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         printf( "Error linking program:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return 0;
   }

   // Free up no longer needed shader resources
   glDeleteShader ( vertexShader );
   glDeleteShader ( fragmentShader );

   checkGlError();

   return programObject;
}

#endif /* GL_ES_VERSION_2_0 || GL_VERSION_2_0*/

void JRenderer::InitRenderer()
{
  checkGlError();
  mCurrentTextureFilter = TEX_FILTER_NONE;
	mImageFilter = NULL;

	mCurrTexBlendSrc = BLEND_SRC_ALPHA;
	mCurrTexBlendDest = BLEND_ONE_MINUS_SRC_ALPHA;

//	mLineWidth = 1.0f;
	mCurrentTex = -1;
	mFOV = 75.0f;

#ifdef USING_MATH_TABLE
	for (int i=0;i<360;i++)
	{
		mSinTable[i] = sinf(i*DEG2RAD);
		mCosTable[i] = cosf(i*DEG2RAD);
	}
#endif

	mCurrentRenderMode = MODE_UNKNOWN;


#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
#ifdef FORCE_GL2
  g_glslfuncts.load();
#endif //FORCE_GL2

  char vShader[] =
      "uniform mat4 u_mvp_matrix;                         \n"
      "attribute vec4 a_position;                         \n"
      "attribute vec4 a_color;                            \n"
      "varying vec4 v_color;                              \n"
      "void main()                                        \n"
      "{                                                  \n"
      "   gl_Position = u_mvp_matrix*a_position;          \n"
      "   v_color = a_color;                              \n"
      "}                                                  \n";
  char fShader[] =
#ifdef GL_ES_VERSION_2_0 // This line works fine on Windows, but not on Linux and it's required for GL_ES_2... hoh well
      "precision mediump float;                           \n"
#endif //GL_ES_VERSION_2_0
      "varying vec4 v_color;                              \n"
      "void main()                                        \n"
      "{                                                  \n"
      " gl_FragColor = v_color;                           \n"
      "}                                                  \n";
  char vShaderWithTexture[] =
      "uniform mat4 u_mvp_matrix;                         \n"
      "attribute vec4 a_position;                         \n"
      "attribute vec2 a_texCoord;                         \n"
      "attribute vec4 a_color;                            \n"
      "varying vec2 v_texCoord;                           \n"
      "varying vec4 v_color;                              \n"
      "void main()                                        \n"
      "{                                                  \n"
      "   gl_Position = u_mvp_matrix*a_position;          \n"
      "   v_texCoord = a_texCoord;                        \n"
      "   v_color = a_color;                              \n"
      "}                                                  \n";
  char fShaderWithTexture[] =
#ifdef GL_ES_VERSION_2_0 // This line works fine on Windows, but not on Linux and it's required for GL_ES_2... hoh well
      "precision mediump float;                           \n"
#endif //GL_ES_VERSION_2_0
      "varying vec2 v_texCoord;                           \n"
      "varying vec4 v_color;                              \n"
      "uniform sampler2D s_texture;                       \n"
      "void main()                                        \n"
      "{                                                  \n"
      " vec4 texColor;                                    \n"
      " texColor = texture2D(s_texture, v_texCoord);      \n"
      " gl_FragColor = v_color*texColor;                  \n"
      "}                                                  \n";

  // Load the vertex/fragment shaders
  prog1 = esLoadProgram(vShader, fShader);
  // Get the attribute locations
  prog1_positionLoc = glGetAttribLocation ( prog1, "a_position" );
  prog1_colorLoc = glGetAttribLocation ( prog1, "a_color" );
  // Get the uniform locations
  prog1_mvpLoc = glGetUniformLocation( prog1, "u_mvp_matrix" );

  // Load the vertex/fragment shaders
  prog2 = esLoadProgram(vShaderWithTexture, fShaderWithTexture);
  // Get the attribute locations
  prog2_positionLoc = glGetAttribLocation ( prog2, "a_position" );
  prog2_texCoordLoc = glGetAttribLocation ( prog2, "a_texCoord" );
  prog2_colorLoc = glGetAttribLocation ( prog2, "a_color" );
  // Get the uniform locations
  prog2_mvpLoc = glGetUniformLocation( prog2, "u_mvp_matrix" );
  // Get the sampler location
  prog2_samplerLoc = glGetUniformLocation ( prog2, "s_texture" );
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  checkGlError();
}

void JRenderer::DestroyRenderer()
{
  checkGlError();
#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
    // Delete program object
    glDeleteProgram ( prog1 );
    glDeleteProgram ( prog2 );
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
}

void JRenderer::BeginScene()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
  glLoadIdentity ();											// Reset The Modelview Matrix
#else
  esMatrixLoadIdentity(&theMvpMatrix);
  esOrtho(&theMvpMatrix, 0.0f, SCREEN_WIDTH_F, 0.0f, SCREEN_HEIGHT_F-1.0f,-1.0f, 1.0f);
#endif //(!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
#ifdef WIN32
  float scaleH = mActualHeight/SCREEN_HEIGHT_F;
  float scaleW = mActualWidth/SCREEN_WIDTH_F;
  glScalef(scaleW,scaleH,1.f);
#endif
  checkGlError();
}


void JRenderer::EndScene()
{
  checkGlError();
  glFlush ();
  checkGlError();
}

void JRenderer::BindTexture(JTexture *tex)
{
  checkGlError();
  if (mCurrentTex != tex->mTexId)
	{
		mCurrentTex = tex->mTexId;

		glBindTexture(GL_TEXTURE_2D, mCurrentTex);

		//if (mCurrentTextureFilter != tex->mFilter)
		{
			//mCurrentTextureFilter = tex->mFilter;
			if (mCurrentTextureFilter == TEX_FILTER_LINEAR)
			{
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			}
			else if (mCurrentTextureFilter == TEX_FILTER_NEAREST)
			{
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			}
		}
	}
  checkGlError();
}


void JRenderer::EnableTextureFilter(bool flag)
{
	if (flag)
		mCurrentTextureFilter = TEX_FILTER_LINEAR;
	else
		mCurrentTextureFilter = TEX_FILTER_NEAREST;

	mCurrentTex = -1;
}

void Swap(float *a, float *b)
{
	float n=*a;
	*a = *b;
	*b = n;
}

void JRenderer::RenderQuad(JQuad* quad, float xo, float yo, float angle, float xScale, float yScale)
{
  checkGlError();

  //yo = SCREEN_HEIGHT-yo-1;//-(quad->mHeight);
	float width = quad->mWidth;
	float height = quad->mHeight;
	float x = -quad->mHotSpotX;
	float y = quad->mHotSpotY;

	Vector2D pt[4];
	pt[3] = Vector2D(x, y);
	pt[2] = Vector2D(x+width, y);
	pt[1] = Vector2D(x+width, y-height);
	pt[0] = Vector2D(x, y-height);


	Vector2D uv[4];
	uv[0] = Vector2D(quad->mTX0, quad->mTY1);
	uv[1] = Vector2D(quad->mTX1, quad->mTY1);
	uv[2] = Vector2D(quad->mTX1, quad->mTY0);
	uv[3] = Vector2D(quad->mTX0, quad->mTY0);

	if (quad->mHFlipped)
	{
		Swap(&uv[0].x, &uv[1].x);
		Swap(&uv[2].x, &uv[3].x);
	}

	if (quad->mVFlipped)
	{
		Swap(&uv[0].y, &uv[2].y);
		Swap(&uv[1].y, &uv[3].y);
	}

        BindTexture(quad->mTex);


	////glRasterPos2f(x, y);


        yo = SCREEN_HEIGHT_F - yo;


#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
        ESMatrix  mvpMatrix;
        memcpy(&mvpMatrix, &theMvpMatrix, sizeof(ESMatrix));

        esTranslate(&mvpMatrix, xo, yo, 0.0f);

        // see http://code.google.com/p/wagic/issues/detail?id=460
        esRotate(&mvpMatrix, -angle*RAD2DEG, 0.0f, 0.0f, 1.0f);
        esScale(&mvpMatrix, xScale, yScale, 1.0f);


        GLfloat vVertices[] = {
            pt[0].x, pt[0].y, 0.0f,
            uv[0].x, uv[0].y,
            pt[1].x, pt[1].y, 0.0f,
            uv[1].x, uv[1].y,
            pt[3].x, pt[3].y, 0.0f,
            uv[3].x, uv[3].y,
            pt[2].x, pt[2].y, 0.0f,
            uv[2].x, uv[2].y,
        };

        GLubyte colorCoords[] = {
            quad->mColor[0].r, quad->mColor[0].g, quad->mColor[0].b, quad->mColor[0].a,
            quad->mColor[1].r, quad->mColor[1].g, quad->mColor[1].b, quad->mColor[1].a,
            quad->mColor[3].r, quad->mColor[3].g, quad->mColor[3].b, quad->mColor[3].a,
            quad->mColor[2].r, quad->mColor[2].g, quad->mColor[2].b, quad->mColor[2].a,
        };

        // Use the program object
        glUseProgram ( prog2 );

        // Load the vertex position
        glVertexAttribPointer ( prog2_positionLoc, 3, GL_FLOAT,
                                GL_FALSE, 5 * sizeof(GLfloat), vVertices );
        // Load the texture coordinate
        glVertexAttribPointer ( prog2_texCoordLoc, 2, GL_FLOAT,
                                GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );
        // Load the colors
        glVertexAttribPointer ( prog2_colorLoc, 4, GL_UNSIGNED_BYTE,
                                GL_TRUE, 4 * sizeof(GLubyte), colorCoords );

        glEnableVertexAttribArray ( prog2_positionLoc );
        glEnableVertexAttribArray ( prog2_texCoordLoc );
        glEnableVertexAttribArray ( prog2_colorLoc );

        // Load the MVP matrix
        glUniformMatrix4fv( prog2_mvpLoc, 1, GL_FALSE, (GLfloat*) &mvpMatrix.m[0][0] );

        // Bind the texture
        glActiveTexture ( GL_TEXTURE0 );
        glBindTexture ( GL_TEXTURE_2D, mCurrentTex );

        // Set the sampler texture unit to 0
        glUniform1i ( prog2_samplerLoc, 0 );

        glDrawArrays(GL_TRIANGLE_STRIP,0,4);

#else
        glPushMatrix();
        glTranslatef(xo, yo, 0.0f);
        glRotatef(-angle*RAD2DEG, 0.0f, 0.0f, 1.0f);
        glScalef(xScale, yScale, 1.0f);

#if (defined GL_ES_VERSION_1_1) || (defined GL_VERSION_1_1)
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        GLfloat vertCoords[] = {
            pt[0].x, pt[0].y,
            pt[1].x, pt[1].y,
            pt[3].x, pt[3].y,
            pt[2].x, pt[2].y,
        };

        GLfloat texCoords[] = {
            uv[0].x, uv[0].y,
            uv[1].x, uv[1].y,
            uv[3].x, uv[3].y,
            uv[2].x, uv[2].y,
        };

        GLubyte colorCoords[] = {
            quad->mColor[0].r, quad->mColor[0].g, quad->mColor[0].b, quad->mColor[0].a,
            quad->mColor[1].r, quad->mColor[1].g, quad->mColor[1].b, quad->mColor[1].a,
            quad->mColor[3].r, quad->mColor[3].g, quad->mColor[3].b, quad->mColor[3].a,
            quad->mColor[2].r, quad->mColor[2].g, quad->mColor[2].b, quad->mColor[2].a,
        };

        glVertexPointer(2,GL_FLOAT,0,vertCoords);
        glTexCoordPointer(2,GL_FLOAT,0, texCoords);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, colorCoords );
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
#else
        glBegin(GL_QUADS);

        glColor4ub(quad->mColor[0].r, quad->mColor[0].g, quad->mColor[0].b, quad->mColor[0].a);
        glTexCoord2f(uv[0].x, uv[0].y); glVertex2f(pt[0].x, pt[0].y);

        // bottom right corner
        glColor4ub(quad->mColor[1].r, quad->mColor[1].g, quad->mColor[1].b, quad->mColor[1].a);
        glTexCoord2f(uv[1].x, uv[1].y); glVertex2f(pt[1].x, pt[1].y);

        // top right corner
        glColor4ub(quad->mColor[2].r, quad->mColor[2].g, quad->mColor[2].b, quad->mColor[2].a);
        glTexCoord2f(uv[2].x, uv[2].y); glVertex2f(pt[2].x, pt[2].y);

        // top left corner
        glColor4ub(quad->mColor[3].r, quad->mColor[3].g, quad->mColor[3].b, quad->mColor[3].a);
        glTexCoord2f(uv[3].x, uv[3].y); glVertex2f(pt[3].x, pt[3].y);

        glEnd();
#endif //(defined GL_ES_VERSION_1_1) || (defined GL_VERSION_1_1)

        glPopMatrix();

        //glDisable(GL_BLEND);

        // default color
        glColor4ub(255, 255, 255, 255);
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

  checkGlError();
}


void JRenderer::RenderQuad(JQuad* quad, VertexColor* pt)
{
  checkGlError();

	for (int i=0;i<4;i++)
	{
		pt[i].y = SCREEN_HEIGHT_F - pt[i].y;
		quad->mColor[i].color = pt[i].color;
	}

	Vector2D uv[4];
	uv[0] = Vector2D(quad->mTX0, quad->mTY1);
	uv[1] = Vector2D(quad->mTX1, quad->mTY1);
	uv[2] = Vector2D(quad->mTX1, quad->mTY0);
	uv[3] = Vector2D(quad->mTX0, quad->mTY0);

	BindTexture(quad->mTex);

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  GLfloat vVertices[] = {
      pt[0].x, pt[0].y, 0.0f,
      uv[0].x, uv[0].y,
      pt[1].x, pt[1].y, 0.0f,
      uv[1].x, uv[1].y,
      pt[3].x, pt[3].y, 0.0f,
      uv[3].x, uv[3].y,
      pt[2].x, pt[2].y, 0.0f,
      uv[2].x, uv[2].y,
  };

  GLubyte colorCoords[] = {
      quad->mColor[0].r, quad->mColor[0].g, quad->mColor[0].b, quad->mColor[0].a,
      quad->mColor[1].r, quad->mColor[1].g, quad->mColor[1].b, quad->mColor[1].a,
      quad->mColor[3].r, quad->mColor[3].g, quad->mColor[3].b, quad->mColor[3].a,
      quad->mColor[2].r, quad->mColor[2].g, quad->mColor[2].b, quad->mColor[2].a,
  };

  // Use the program object
  glUseProgram ( prog2 );

  // Load the vertex position
  glVertexAttribPointer ( prog2_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), vVertices );
  // Load the texture coordinate
  glVertexAttribPointer ( prog2_texCoordLoc, 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );
  // Load the colors
  glVertexAttribPointer ( prog2_colorLoc, 4, GL_UNSIGNED_BYTE,
                          GL_TRUE, 4 * sizeof(GLubyte), colorCoords );

  glEnableVertexAttribArray ( prog2_positionLoc );
  glEnableVertexAttribArray ( prog2_texCoordLoc );
  glEnableVertexAttribArray ( prog2_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog2_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  // Bind the texture
  glActiveTexture ( GL_TEXTURE0 );
  glBindTexture ( GL_TEXTURE_2D, mCurrentTex );

  // Set the sampler texture unit to 0
  glUniform1i ( prog2_samplerLoc, 0 );

  //glDrawElements ( GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, indices );
  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
#else
	glRasterPos2f(pt[0].x, pt[0].y);

	//float w = quad->mWidth;
	//float h = quad->mHeight;

	glBegin(GL_QUADS);
		// bottom left corner
		glColor4ub(quad->mColor[0].r, quad->mColor[0].g, quad->mColor[0].b, quad->mColor[0].a);
		glTexCoord2f(uv[0].x, uv[0].y); glVertex2f(pt[0].x, pt[0].y);

		// bottom right corner
		glColor4ub(quad->mColor[1].r, quad->mColor[1].g, quad->mColor[1].b, quad->mColor[1].a);
		glTexCoord2f(uv[1].x, uv[1].y); glVertex2f(pt[1].x, pt[1].y);

		// top right corner
		glColor4ub(quad->mColor[2].r, quad->mColor[2].g, quad->mColor[2].b, quad->mColor[2].a);
		glTexCoord2f(uv[2].x, uv[2].y); glVertex2f(pt[2].x, pt[2].y);

		// top left corner
		glColor4ub(quad->mColor[3].r, quad->mColor[3].g, quad->mColor[3].b, quad->mColor[3].a);
		glTexCoord2f(uv[3].x, uv[3].y); glVertex2f(pt[3].x, pt[3].y);
	glEnd();
  // default color
  glColor4ub(255, 255, 255, 255);
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

	//glDisable(GL_BLEND);

  checkGlError();
}


void JRenderer::FillRect(float x, float y, float width, float height, PIXEL_TYPE color)
{
  checkGlError();

  y = SCREEN_HEIGHT_F - y - height;

	JColor col;
	col.color = color;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  GLfloat vVertices[] = {
      x,        y+height, 0.0f,
      x,        y,        0.0f,
      x+width,  y+height, 0.0f,
      x+width,  y,        0.0f,
  };

  GLubyte colors[] = {
      col.r, col.g, col.b, col.a,
      col.r, col.g, col.b, col.a,
      col.r, col.g, col.b, col.a,
      col.r, col.g, col.b, col.a,
  };

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);

	glBegin(GL_QUADS);
		// top left corner
		glVertex2f(x, y+height);

		// bottom left corner
		glVertex2f(x, y);

		// bottom right corner
		glVertex2f(x+width, y);

		// top right corner
		glVertex2f(x+width, y+height);

	glEnd();
  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)


  checkGlError();
}


void JRenderer::DrawRect(float x, float y, float width, float height, PIXEL_TYPE color)
{
  checkGlError();

  y = SCREEN_HEIGHT_F - y - height;

	JColor col;
	col.color = color;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  GLfloat vVertices[] = {
      x,        y,        0.0f,
      x,        y+height, 0.0f,
      x+width,  y+height, 0.0f,
      x+width,  y,        0.0f,
  };

  GLubyte colors[] = {
      col.r, col.g, col.b, col.a,
      col.r, col.g, col.b, col.a,
      col.r, col.g, col.b, col.a,
      col.r, col.g, col.b, col.a,
  };

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_LINE_LOOP,0,4);
#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_LINES);

		glVertex2f(x, y);
		glVertex2f(x, y+height);

		glVertex2f(x, y+height);
		glVertex2f(x+width, y+height);

		glVertex2f(x+width, y+height);
		glVertex2f(x+width, y);

		glVertex2f(x+width, y);
		glVertex2f(x, y);

	glEnd();

  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)

  checkGlError();
}


void JRenderer::FillRect(float x, float y, float width, float height, PIXEL_TYPE* colors)
{
	JColor col[4];
	for (int i=0;i<4;i++)
		col[i].color = colors[i];

	FillRect(x, y, width, height, col);
}


void JRenderer::FillRect(float x, float y, float width, float height, JColor* colors)
{
  checkGlError();
  y = SCREEN_HEIGHT_F - y - height;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  GLfloat vVertices[] = {
      x,        y+height, 0.0f,
      x,        y,        0.0f,
      x+width,  y+height, 0.0f,
      x+width,  y,        0.0f,
  };

  GLubyte cols[] = {
      colors[0].r, colors[0].g, colors[0].b, colors[0].a,
      colors[2].r, colors[2].g, colors[2].b, colors[2].a,
      colors[1].r, colors[1].g, colors[1].b, colors[1].a,
      colors[3].r, colors[3].g, colors[3].b, colors[3].a,
  };

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), cols);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
#else
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
		// top left corner
		glColor4ub(colors[0].r, colors[0].g, colors[0].b, colors[0].a);
		glVertex2f(x, y+height);

		// bottom left corner
		glColor4ub(colors[2].r, colors[2].g, colors[2].b, colors[2].a);
		glVertex2f(x, y);

		// bottom right corner
		glColor4ub(colors[3].r, colors[3].g, colors[3].b, colors[3].a);
		glVertex2f(x+width, y);

		// top right corner
		glColor4ub(colors[1].r, colors[1].g, colors[1].b, colors[1].a);
		glVertex2f(x+width, y+height);

	glEnd();

  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

	//glDisable(GL_BLEND);
  checkGlError();
}


void JRenderer::DrawLine(float x1, float y1, float x2, float y2, PIXEL_TYPE color)
{
  checkGlError();
//	glLineWidth (mLineWidth);
	JColor col;
	col.color = color;
#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  GLfloat vVertices[] = {
      x1, SCREEN_HEIGHT_F-y1, 0.0f,
      x2, SCREEN_HEIGHT_F-y2, 0.0f,
  };

  GLubyte cols[] = {
      col.r, col.g, col.b, col.a,
      col.r, col.g, col.b, col.a,
  };

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), cols);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_LINES,0,2);
#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_LINES);
		glVertex2f(x1, SCREEN_HEIGHT_F-y1);
		glVertex2f(x2, SCREEN_HEIGHT_F-y2);
	glEnd();
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
  checkGlError();
}


void JRenderer::Plot(float x, float y, PIXEL_TYPE color)
{
  checkGlError();
  glDisable(GL_TEXTURE_2D);
	JColor col;
	col.color = color;
#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_POINTS);
		glVertex2f(x, SCREEN_HEIGHT_F-y);
	glEnd();
  glColor4ub(255, 255, 255, 255);
#else
  // FIXME, not used
#endif //#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
  glEnable(GL_TEXTURE_2D);
  checkGlError();
}


void JRenderer::PlotArray(float *x, float *y, int count, PIXEL_TYPE color)
{
  checkGlError();
  glDisable(GL_TEXTURE_2D);
	JColor col;  
	col.color = color;
#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_POINTS);
		for (int i=0;i<count;i++)
			glVertex2f(x[i], SCREEN_HEIGHT_F-y[i]);
	glEnd();
  glColor4ub(255, 255, 255, 255);
#else
  // FIXME, not used
#endif //#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
  glEnable(GL_TEXTURE_2D);
  checkGlError();
}



void JRenderer::ScreenShot(const char* filename __attribute__((unused)))
{

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


#if (!defined IOS)
static void jpg_null(j_decompress_ptr cinfo __attribute__((unused)))
{
}


static boolean jpg_fill_input_buffer(j_decompress_ptr cinfo __attribute__((unused)))
{
	////    ri.Con_Printf(PRINT_ALL, "Premature end of JPEG data\n");
	return 1;
}

static void jpg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{

	cinfo->src->next_input_byte += (size_t) num_bytes;
	cinfo->src->bytes_in_buffer -= (size_t) num_bytes;

	//// if (cinfo->src->bytes_in_buffer < 0)
	////		ri.Con_Printf(PRINT_ALL, "Premature end of JPEG data\n");
}

static void jpeg_mem_src(j_decompress_ptr cinfo, byte *mem, int len)
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

/*
==============
LoadJPG
==============
*/
void JRenderer::LoadJPG(TextureInfo &textureInfo, const char *filename, int mode __attribute__((unused)), int TextureFormat __attribute__((unused)))
{
	textureInfo.mBits = NULL;

	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr jerr;
	BYTE *rawdata, *rgbadata, *scanline, *p, *q;
	int	rawsize, i;

        JFileSystem* fileSystem = JFileSystem::GetInstance();
        if (!fileSystem->OpenFile(filename)) return;

        rawsize = fileSystem->GetFileSize();

        rawdata = new BYTE[rawsize];

        if (!rawdata)
        {
                fileSystem->CloseFile();
                return;
        }

        fileSystem->ReadFile(rawdata, rawsize);
        fileSystem->CloseFile();

      	// Initialize libJpeg Object
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

        jpeg_mem_src(&cinfo, rawdata, rawsize);

	// Process JPEG header
	jpeg_read_header(&cinfo, true);



	// Start Decompression
	jpeg_start_decompress(&cinfo);

	// Check Colour Components
	if(cinfo.output_components != 3 && cinfo.output_components != 4)
	{
		////		ri.Con_Printf(PRINT_ALL, "Invalid JPEG colour components\n");
		jpeg_destroy_decompress(&cinfo);
		////		ri.FS_FreeFile(rawdata);
		return;
	}

	int tw = getNextPower2(cinfo.output_width);
	int th = getNextPower2(cinfo.output_height);


	// Allocate Memory for decompressed image
	rgbadata = new BYTE[tw * th * 4];
	if(!rgbadata)
	{
		////		ri.Con_Printf(PRINT_ALL, "Insufficient RAM for JPEG buffer\n");
		jpeg_destroy_decompress(&cinfo);
		////		ri.FS_FreeFile(rawdata);
		delete [] rgbadata;
		return;
	}


	// Pass sizes to output

	// Allocate Scanline buffer
	scanline = (byte *)malloc(cinfo.output_width * 3);
	if(!scanline)
	{
		////		ri.Con_Printf(PRINT_ALL, "Insufficient RAM for JPEG scanline buffer\n");

		jpeg_destroy_decompress(&cinfo);
		////		ri.FS_FreeFile(rawdata);

		delete [] rgbadata;
		return;
	}

	// Read Scanlines, and expand from RGB to RGBA
	BYTE* currRow = rgbadata;

	while(cinfo.output_scanline < cinfo.output_height)
	{
		p = scanline;
		jpeg_read_scanlines(&cinfo, &scanline, 1);

		q = currRow;
		for(i=0; i<(int)cinfo.output_width; i++)
		{
			q[0] = p[0];
			q[1] = p[1];
			q[2] = p[2];
			q[3] = 255;

			p+=3; q+=4;
		}
		currRow += tw*4;
	}

	// Free the scanline buffer
	free(scanline);

	textureInfo.mBits = rgbadata;
	textureInfo.mWidth = cinfo.output_width;
	textureInfo.mHeight = cinfo.output_height;
	textureInfo.mTexWidth = tw;
	textureInfo.mTexHeight = th;

	// Finish Decompression
        try {
          jpeg_finish_decompress(&cinfo);
        } catch(...) {}

	// Destroy JPEG object
	jpeg_destroy_decompress(&cinfo);

        delete[] rawdata;
}


static void PNGCustomWarningFn(png_structp png_ptr __attribute__((unused)), png_const_charp warning_msg __attribute__((unused)))
{
        // ignore PNG warnings
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

JTexture* JRenderer::LoadTexture(const char* filename, int mode, int TextureFormat __attribute__((unused)))
{
    TextureInfo textureInfo;

    textureInfo.mBits = NULL;

    if (strstr(filename, ".jpg")!=NULL || strstr(filename, ".JPG")!=NULL)
        LoadJPG(textureInfo, filename);
    else if(strstr(filename, ".gif")!=NULL || strstr(filename, ".GIF")!=NULL)
        LoadGIF(textureInfo,filename);
    else if(strstr(filename, ".png")!=NULL || strstr(filename, ".PNG")!=NULL)
        LoadPNG(textureInfo, filename);

    if (textureInfo.mBits == NULL) {
        printf("Texture %s failed to load\n", filename);
        return NULL;
    }

    bool ret = false;

    JTexture *tex = new JTexture();

    if (tex)
    {
        if (mImageFilter != NULL)
            mImageFilter->ProcessImage((PIXEL_TYPE*)textureInfo.mBits, textureInfo.mWidth, textureInfo.mHeight);

        tex->mFilter = TEX_FILTER_LINEAR;
        tex->mWidth = textureInfo.mWidth;
        tex->mHeight = textureInfo.mHeight;
        tex->mTexWidth = textureInfo.mTexWidth;
        tex->mTexHeight = textureInfo.mTexHeight;

        tex->mBuffer = textureInfo.mBits;
    }
 
    return tex;
}

int JRenderer::LoadPNG(TextureInfo &textureInfo, const char *filename, int mode __attribute__((unused)), int TextureFormat __attribute__((unused)))
{
	textureInfo.mBits = NULL;

	DWORD* p32;

    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    png_uint_32 width, height, tw, th;
    int bit_depth, color_type, interlace_type, x, y;
    DWORD* line;

	JFileSystem* fileSystem = JFileSystem::GetInstance();
	if (!fileSystem->OpenFile(filename)) return JGE_ERR_CANT_OPEN_FILE;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
	{
		fileSystem->CloseFile();

        return JGE_ERR_PNG;
    }

    png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, PNGCustomWarningFn);
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
	{
        //fclose(fp);
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

	line = (DWORD*) malloc(width * 4);
    if (!line)
	{
        //fclose(fp);
		fileSystem->CloseFile();

        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return JGE_ERR_MALLOC_FAILED;
    }


	tw = getNextPower2(width);
	th = getNextPower2(height);

	int size = tw * th * 4;			// RGBA

	BYTE* buffer = new BYTE[size];

	//JTexture *tex = new JTexture();

	if (buffer)
	{


			p32 = (DWORD*) buffer;

			for (y = 0; y < (int)height; y++)
			{
				png_read_row(png_ptr, (BYTE*) line, png_bytep_NULL);
				for (x = 0; x < (int)width; x++)
				{
					DWORD color32 = line[x];
					int a = (color32 >> 24) & 0xff;
					int r = color32 & 0xff;
					int g = (color32 >> 8) & 0xff;
					int b = (color32 >> 16) & 0xff;

					color32 = r | (g << 8) | (b << 16) | (a << 24);
					*(p32+x) = color32;

				}
				p32 += tw;

			}

		}


    free (line);

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

	fileSystem->CloseFile();


	textureInfo.mBits = buffer;
	textureInfo.mWidth = width;
	textureInfo.mHeight = height;
	textureInfo.mTexWidth = tw;
	textureInfo.mTexHeight = th;

  return 1;
	//return textureInfo;

}


// void JRenderer::FreeTexture(JTexture* tex)
// {
// 	glDeleteTextures(1, &tex->mTexId);
//
// 	delete tex;
// 	tex = NULL;
// }



//////////////////////////////////////////////////////////////////////////
/// GIF Support
int JRenderer::image_readgif(void * handle, TextureInfo &textureInfo, DWORD * bgcolor, InputFunc readFunc, int mode __attribute__((unused)), int TextureFormat __attribute__((unused)))
{

	//	pixel ** image_data=NULL;
	DWORD *p32=NULL;
	//	DWORD *buff=NULL;
	//#define gif_color(c) RGB(palette->Colors[c].Red, palette->Colors[c].Green, palette->Colors[c].Blue)
#define gif_color32(c) ARGB(255,palette->Colors[c].Blue,palette->Colors[c].Green,  palette->Colors[c].Red)
	GifRecordType RecordType;
	GifByteType *Extension;
	GifRowType LineIn = NULL;
	GifFileType *GifFileIn = NULL;
	ColorMapObject *palette;
	int ExtCode;
	if ((GifFileIn = DGifOpen(handle, readFunc)) == NULL)
		return 1;
	*bgcolor = 0;
	textureInfo.mWidth = 0;
	textureInfo.mHeight = 0;
	//*image_data = NULL;

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
					textureInfo.mTexWidth = getNextPower2(GifFileIn->Image.Width);
					textureInfo.mTexHeight = getNextPower2(GifFileIn->Image.Height);

					//if((*image_data = (pixel *)malloc(sizeof(pixel) * GifFileIn->Image.Width * GifFileIn->Image.Height)) == NULL)
					if((p32 = (DWORD *)malloc(sizeof(PIXEL_TYPE) * textureInfo.mTexWidth * textureInfo.mTexHeight)) == NULL)
					{
						free((void *)LineIn);
						DGifCloseFile(GifFileIn);
						return 1;
					}
					DWORD * curr = p32;
					DWORD * imgdata;
					for (GifWord i = 0; i < GifFileIn->Image.Height; i ++)
					{
						imgdata = curr;
						if (DGifGetLine(GifFileIn, LineIn, GifFileIn->Image.Width) == GIF_ERROR)
						{
							free((void *)p32);
							free((void *)LineIn);
							DGifCloseFile(GifFileIn);
							return 1;
						}
						for (GifWord j = 0; j < GifFileIn->Image.Width; j ++)
						{
							DWORD color32 = gif_color32(LineIn[j]);

							//if(mTexLoadingCB) mTexLoadingCB(color32);
							//if(JRenderer::GetInstance()->GetTextureLoadingCallback())
							//	JRenderer::GetInstance()->GetTextureLoadingCallback()(color32);

							*imgdata++ = color32;
						}

						curr += textureInfo.mTexWidth;
					}
					textureInfo.mBits = (u8 *)p32;
					break;
				}
			case EXTENSION_RECORD_TYPE:
				if (DGifGetExtension(GifFileIn, &ExtCode, &Extension) == GIF_ERROR)
				{
					if(textureInfo.mBits != NULL)
					{
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
	//return fread(buf, 1, size, (FILE *)ft->UserData);
	if (fileSys->ReadFile(buf, size))
		return size;
	else
		return 0;

}

void JRenderer::LoadGIF(TextureInfo &textureInfo, const char *filename, int mode, int TextureFormat __attribute__((unused)))
{
	///*
	//FILE * fp = fopen(filename, "rb");
	JFileSystem *fileSys = JFileSystem::GetInstance();
	if (!fileSys->OpenFile(filename))
		return;

	//if(fp == NULL)
	//	return;
	DWORD bkcol;
	int result = image_readgif(fileSys, textureInfo, &bkcol, image_gif_read, mode);
	if(result!=0)
		textureInfo.mBits=NULL;
	//fclose(fp);
	fileSys->CloseFile();
	return ;//*/
}

#elif (defined IOS)

#include <UIKit/UIImage.h>

JTexture* JRenderer::LoadTexture(const char* filename, int mode, int TextureFormat __attribute__((unused)))
{
    TextureInfo textureInfo;
    JTexture *tex = NULL;
    textureInfo.mBits = NULL;
    int rawsize = 0;
    BYTE* rawdata = NULL;
    JFileSystem* fileSystem = JFileSystem::GetInstance();
    NSData *texData = NULL;
    UIImage *image = NULL;

    do {
        if (!fileSystem->OpenFile(filename))
                break;

        rawsize = fileSystem->GetFileSize();
        rawdata = new BYTE[rawsize];

        if (!rawdata)
        {
                fileSystem->CloseFile();
                break;
        }

        fileSystem->ReadFile(rawdata, rawsize);
        fileSystem->CloseFile();

        texData = [[NSData alloc] initWithBytes:rawdata length:rawsize];
        image = [[UIImage alloc] initWithData:texData];
        CGImageAlphaInfo info;
        BOOL hasAlpha;

        info = CGImageGetAlphaInfo(image.CGImage);
        hasAlpha = ((info == kCGImageAlphaPremultipliedLast) || (info == kCGImageAlphaPremultipliedFirst) || (info == kCGImageAlphaLast) || (info == kCGImageAlphaFirst) ? YES : NO);

        if (image == nil) {
                NSLog(@"Loading Texture : %s failed", filename);
                break;
        }

        textureInfo.mWidth = CGImageGetWidth(image.CGImage);
        textureInfo.mHeight = CGImageGetHeight(image.CGImage);
        textureInfo.mTexWidth = getNextPower2(textureInfo.mWidth);
        textureInfo.mTexHeight = getNextPower2(textureInfo.mHeight);

        NSLog(@"Loading Texture : %s : %s : %ux%u", filename, (hasAlpha?"Alpha ":"No Alpha "), textureInfo.mWidth, textureInfo.mHeight);

        textureInfo.mBits = new u8 [ textureInfo.mTexHeight * textureInfo.mTexWidth * 4 ];
        if (textureInfo.mBits == NULL) {
                NSLog(@"Texture %s failed to load\n", filename);
                break;
        }

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        info = /*hasAlpha ?*/ kCGImageAlphaPremultipliedLast /*: kCGImageAlphaNoneSkipLast*/;

        CGContextRef context =
        CGBitmapContextCreate(textureInfo.mBits,
                                                  textureInfo.mTexWidth,
                                                  textureInfo.mTexHeight, 8,
                                                  4 * textureInfo.mTexWidth,
                                                  colorSpace,
                                                  info /*| kCGBitmapByteOrder32Big*/);
        CGColorSpaceRelease(colorSpace);
        CGContextClearRect( context, CGRectMake( 0, 0, textureInfo.mTexWidth, textureInfo.mTexHeight ) );
        CGContextTranslateCTM( context, 0, textureInfo.mTexHeight - textureInfo.mHeight );
        CGContextDrawImage( context, CGRectMake( 0, 0, textureInfo.mWidth, textureInfo.mHeight ), image.CGImage );

        tex = new JTexture();

        if (tex)
        {
                if (mImageFilter != NULL)
                        mImageFilter->ProcessImage((PIXEL_TYPE*)textureInfo.mBits, textureInfo.mWidth, textureInfo.mHeight);

                tex->mFilter = TEX_FILTER_LINEAR;
                tex->mWidth = textureInfo.mWidth;
                tex->mHeight = textureInfo.mHeight;
                tex->mTexWidth = textureInfo.mTexWidth;
                tex->mTexHeight = textureInfo.mTexHeight;
                tex->mBuffer = textureInfo.mBits;
        } else {
                NSLog(@"JTexture for %s not created\n", filename);
        }

        CGContextRelease(context);
    } while(0);

    if(rawdata)
        delete[] rawdata;

    [image release];
    [texData release];

    return tex;
}
#elif (defined QT_CONFIG)
JTexture* JRenderer::LoadTexture(const char* filename, int mode, int TextureFormat __attribute__((unused)))
{
  JTexture *tex = NULL;
  int rawsize = 0;
  BYTE* rawdata = NULL;
  JFileSystem* fileSystem = JFileSystem::GetInstance();

  do {
    if (!fileSystem->OpenFile(filename))
      break;

    rawsize = fileSystem->GetFileSize();
    rawdata = new BYTE[rawsize];

    if (!rawdata)
    {
      fileSystem->CloseFile();
      break;
    }

    fileSystem->ReadFile(rawdata, rawsize);
    fileSystem->CloseFile();

    QImage tmpImage = QImage::fromData(rawdata, rawsize);
    if(tmpImage.isNull())
      break;

    tex = new JTexture();
    if (tex)
    {
      tmpImage = tmpImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
      tmpImage = tmpImage.rgbSwapped();

      if (mImageFilter != NULL)
          mImageFilter->ProcessImage((PIXEL_TYPE*)tmpImage.bits(), tmpImage.width(), tmpImage.height());

      tex->mFilter = TEX_FILTER_LINEAR;
      tex->mWidth = tmpImage.width();
      tex->mHeight = tmpImage.height();
      tex->mTexWidth = getNextPower2(tmpImage.width());
      tex->mTexHeight = getNextPower2(tmpImage.height());;
      tex->mBuffer = new BYTE[tex->mTexWidth*tex->mTexHeight*4];

      for(int i=0; i < tex->mHeight; i++)
      {
        memcpy(tex->mBuffer+(i*4*tex->mTexWidth), tmpImage.constScanLine(i), tmpImage.bytesPerLine());
      }
    }
  } while(false);

  if(rawdata)
    delete[] rawdata;

  return tex;
}
#endif //IOS

void JRenderer::TransferTextureToGLContext(JTexture& inTexture)
{
    if (inTexture.mBuffer != NULL)
    {
        GLuint texid;
        checkGlError();
        glGenTextures(1, &texid);
        inTexture.mTexId = texid;
        mCurrentTex = texid;

        //    glError = glGetError();

        if (1)///*texid*/ glError == 0)
        {

            // OpenGL texture has (0,0) at lower-left
            // Pay attention when doing texture mapping!!!

            glBindTexture(GL_TEXTURE_2D, mCurrentTex);    // Bind To The Texture ID

            /* NOT USED
            if (mode == TEX_TYPE_MIPMAP)			// generate mipmaps
            {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, textureInfo.mTexWidth, textureInfo.mTexHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureInfo.mBits);
            }
            else if (mode == TEX_TYPE_SKYBOX)		// for skybox
            {
            #define GL_CLAMP_TO_EDGE	0x812F

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, textureInfo.mTexWidth, textureInfo.mTexHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureInfo.mBits);
            }
            else									// single texture
            */
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inTexture.mTexWidth, inTexture.mTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, inTexture.mBuffer);
            }
        }
        delete [] inTexture.mBuffer;
        inTexture.mBuffer = NULL;

        checkGlError();
    }
}

JTexture* JRenderer::CreateTexture(int width, int height, int mode __attribute__((unused)))
{
    checkGlError();
    JTexture *tex = new JTexture();

    if (tex)
    {
        int size = width * height * sizeof(PIXEL_TYPE);			// RGBA
        BYTE* buffer = new BYTE[size];
        if (buffer)
        {
            tex->mFilter = TEX_FILTER_LINEAR;
            tex->mWidth = width;
            tex->mHeight = height;
            tex->mTexWidth = width;
            tex->mTexHeight = height;

            GLuint texid;
            glGenTextures(1, &texid);
            tex->mTexId = texid;

            memset(buffer, 0, size);

            mCurrentTex = texid;
            glBindTexture(GL_TEXTURE_2D, mCurrentTex);

            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

            delete[] buffer;

	        checkGlError();

        }
        else
        {
            delete tex;
            tex = NULL;
        }
    }

    return tex;
}


void JRenderer::EnableVSync(bool flag __attribute__((unused)))
{
//	if (flag)
//		hge->System_SetState(HGE_FPS, 60);	// HGEFPS_VSYNC
//	else
//		hge->System_SetState(HGE_FPS, HGEFPS_UNLIMITED);
}


void JRenderer::ClearScreen(PIXEL_TYPE color)
{
  checkGlError();
    JColor col;
    col.color = color;

    glClearColor(col.r, col.g, col.b, col.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	FillRect(0.0f, 0.0f, SCREEN_WIDTH_F, SCREEN_HEIGHT_F, color);
  checkGlError();
}


void JRenderer::SetTexBlend(int src, int dest)
{
  checkGlError();
  if (src != mCurrTexBlendSrc || dest != mCurrTexBlendDest)
	{
		mCurrTexBlendSrc = src;
		mCurrTexBlendDest = dest;

		glBlendFunc(src, dest);
	}
  checkGlError();
}


void JRenderer::SetTexBlendSrc(int src)
{
  checkGlError();
  if (src != mCurrTexBlendSrc)
	{
		mCurrTexBlendSrc = src;
		glBlendFunc(mCurrTexBlendSrc, mCurrTexBlendDest);
	}
  checkGlError();
}


void JRenderer::SetTexBlendDest(int dest)
{
  checkGlError();
  if (dest != mCurrTexBlendDest)
	{
		mCurrTexBlendDest = dest;
		glBlendFunc(mCurrTexBlendSrc, mCurrTexBlendDest);
	}
  checkGlError();
}


void JRenderer::Enable2D()
{
  checkGlError();
  if (mCurrentRenderMode == MODE_2D)
		return;

	mCurrentRenderMode = MODE_2D;

	glViewport (0, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);	// Reset The Current Viewport
#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
        glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
	glLoadIdentity ();													// Reset The Projection Matrix

	gluOrtho2D(0.0f, SCREEN_WIDTH_F, 0.0f, SCREEN_HEIGHT_F-1.0f);

	glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity ();													// Reset The Modelview Matrix
#else
        // all the matrix code is in shaders calls
#endif //(!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)

	glDisable (GL_DEPTH_TEST);
  checkGlError();
}


void JRenderer::Enable3D()
{ /* NOT USED
	if (!m3DEnabled)
		return;

	if (mCurrentRenderMode == MODE_3D)
		return;

	mCurrentRenderMode = MODE_3D;

	glViewport (0, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);		// Reset The Current Viewport
	glMatrixMode (GL_PROJECTION);												// Select The Projection Matrix
	glLoadIdentity ();															// Reset The Projection Matrix
	gluPerspective (mFOV, (GLfloat)SCREEN_WIDTH/(GLfloat)SCREEN_HEIGHT,	// Calculate The Aspect Ratio Of The Window
					0.5f, 1000.0f);
	glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity ();													// Reset The Modelview Matrix

        glEnable (GL_DEPTH_TEST); */
}


void JRenderer::SetClip(int x, int y, int width, int height)
{// NOT USED
        //glScissor(x, y, width, height);
}


void JRenderer::LoadIdentity()
{// NOT USED
        //glLoadIdentity();
}


void JRenderer::Translate(float x, float y, float z)
{// NOT USED
        //glTranslatef(x, y, z);
}


void JRenderer::RotateX(float angle)
{// NOT USED
        //glRotatef(angle*RAD2DEG, 1.0f, 0.0f, 0.0f);
}


void JRenderer::RotateY(float angle)
{// NOT USED
        //glRotatef(angle*RAD2DEG, 0.0f, 1.0f, 0.0f);
}


void JRenderer::RotateZ(float angle)
{// NOT USED
        //glRotatef(angle*RAD2DEG, 0.0f, 0.0f, 1.0f);
}


void JRenderer::PushMatrix()
{// NOT USED
        //glPushMatrix();
}


void JRenderer::PopMatrix()
{// NOT USED
        //glPopMatrix();
}

void JRenderer::RenderTriangles(JTexture* texture, Vertex3D *vertices, int start, int count)
{
  checkGlError();
  if (texture)
		BindTexture(texture);

  int index = start*3;
#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  GLubyte* colorCoords = new GLubyte[count*3*4];
  memset(colorCoords, 255, count*3*4);

  // Use the program object
  glUseProgram ( prog2 );

  // Load the vertex position
  glVertexAttribPointer ( prog2_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &vertices[index].x );
  // Load the texture coordinate
  glVertexAttribPointer ( prog2_texCoordLoc, 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &vertices[index].u );
  // Load the colors
  glVertexAttribPointer ( prog2_colorLoc, 4, GL_UNSIGNED_BYTE,
                          GL_TRUE, 4 * sizeof(GLubyte), colorCoords );

  glEnableVertexAttribArray ( prog2_positionLoc );
  glEnableVertexAttribArray ( prog2_texCoordLoc );
  glEnableVertexAttribArray ( prog2_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog2_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  // Bind the texture
  glActiveTexture ( GL_TEXTURE0 );
  glBindTexture ( GL_TEXTURE_2D, mCurrentTex );

  // Set the sampler texture unit to 0
  glUniform1i ( prog2_samplerLoc, 0 );

  glDrawArrays(GL_TRIANGLES,0,count*3);

  delete[] colorCoords;
#else
  glBegin(GL_TRIANGLES);
		for (int i = 0; i < count; i++)
		{
			glTexCoord2f(vertices[index].u, vertices[index].v);
			//glNormal3f(vertices[index].nx, vertices[index].ny, vertices[index].nz);
			glVertex3f(vertices[index].x, vertices[index].y, vertices[index].z);

			index++;

			glTexCoord2f(vertices[index].u, vertices[index].v);
			//glNormal3f(vertices[index].nx, vertices[index].ny, vertices[index].nz);
			glVertex3f(vertices[index].x, vertices[index].y, vertices[index].z);

			index++;

			glTexCoord2f(vertices[index].u, vertices[index].v);
			//glNormal3f(vertices[index].nx, vertices[index].ny, vertices[index].nz);
			glVertex3f(vertices[index].x, vertices[index].y, vertices[index].z);

			index++;
		}
	glEnd();
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  checkGlError();
}


void JRenderer::SetFOV(float fov)
{
	mFOV = fov;
}


void JRenderer::FillPolygon(float* x, float* y, int count, PIXEL_TYPE color)
{
  checkGlError();
  JColor col;
	col.color = color;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  int i;
  GLubyte* colors = new GLubyte[count*4];
  GLfloat* vVertices = new GLfloat[count*3];

  for(i = 0; i < count; i++)
  {
    colors[4*i+0]= col.r;
    colors[4*i+1]= col.g;
    colors[4*i+2]= col.b;
    colors[4*i+3]= col.a;
  }

  for(i=0; i < count;i++)
  {
    vVertices[3*i+0] = x[i];
    vVertices[3*i+1] = SCREEN_HEIGHT_F-y[i];
    vVertices[3*i+2] = 0.0f;
  }

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_TRIANGLE_FAN,0,count);

  delete[] vVertices;
  delete[] colors;

#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_TRIANGLE_FAN);

	for(int i=0; i<count;i++)
	{
		glVertex2f(x[i],SCREEN_HEIGHT_F-y[i]);
	}

	glEnd();  
  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

  checkGlError();
}


void JRenderer::DrawPolygon(float* x, float* y, int count, PIXEL_TYPE color)
{
  checkGlError();
  JColor col;
	col.color = color;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  int i;
  int number = count+1;
  GLfloat* vVertices = new GLfloat[3*number];
  GLubyte* colors = new GLubyte[4*number];

  for(i = 0; i < number; i++)
  {
    colors[4*i+0]= col.r;
    colors[4*i+1]= col.g;
    colors[4*i+2]= col.b;
    colors[4*i+3]= col.a;
  }

  for(i=0; i < count;i++)
  {
    vVertices[3*i+0] = x[i];
    vVertices[3*i+1] = SCREEN_HEIGHT_F-y[i];
    vVertices[3*i+2] = 0.0f;
  }

  vVertices[3*(number-1)+0] = x[0];
  vVertices[3*(number-1)+1] = SCREEN_HEIGHT_F-y[0];
  vVertices[3*(number-1)+2] = 0.0f;

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_LINE_STRIP,0,number);

  delete[] vVertices;
  delete[] colors;
#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_LINE_STRIP);

	for(int i=0; i<count;i++)
	{
		glVertex2f(x[i],SCREEN_HEIGHT_F-y[i]);
	}

	glVertex2f(x[0],SCREEN_HEIGHT_F-y[0]);

	glEnd();

  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

  checkGlError();
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
  checkGlError();
  JColor col;
	col.color = color;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  int i, index;
  const int number = 181;
  GLfloat vVertices[number][3];
  GLubyte colors[number][4];

  for(i = 0; i < number; i++)
  {
    colors[i][0]= col.r;
    colors[i][1]= col.g;
    colors[i][2]= col.b;
    colors[i][3]= col.a;
  }

  index = 0;
  for(i=0; i < 360;i+=2, index++)
  {
    vVertices[index][0] = x+radius*COSF(i);
    vVertices[index][1] = SCREEN_HEIGHT_F-y+radius*SINF(i);
    vVertices[index][2] = 0.0f;
  }

  vVertices[number-1][0] = x+radius*COSF(0);
  vVertices[number-1][1] = SCREEN_HEIGHT_F-y+radius*SINF(0);
  vVertices[number-1][2] = 0.0f;

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_LINE_STRIP,0,number);
#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_LINE_STRIP);

		for(int i=0; i<360;i+=2)
		{
			glVertex2f(x+radius*COSF(i), SCREEN_HEIGHT_F-y+radius*SINF(i));
		}

		glVertex2f(x+radius*COSF(0), SCREEN_HEIGHT_F-y+radius*SINF(0));

	glEnd();
  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

  checkGlError();
}

void JRenderer::FillCircle(float x, float y, float radius, PIXEL_TYPE color)
{
  checkGlError();
  JColor col;
	col.color = color;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  int i, index;
  const int number = 182;
  GLfloat vVertices[number][3];
  GLubyte colors[number][4];

  for(i = 0; i < number; i++)
  {
    colors[i][0]= col.r;
    colors[i][1]= col.g;
    colors[i][2]= col.b;
    colors[i][3]= col.a;
  }

  vVertices[0][0] = x;
  vVertices[0][1] = SCREEN_HEIGHT_F-y;
  vVertices[0][2] = 0.0f;

  index = 1;
  for(i=0; i < 360;i+=2, index++)
  {
    vVertices[index][0] = x+radius*COSF(i);
    vVertices[index][1] = SCREEN_HEIGHT_F-y+radius*SINF(i);
    vVertices[index][2] = 0.0f;
  }

  vVertices[number-1][0] = x+radius*COSF(0);
  vVertices[number-1][1] = SCREEN_HEIGHT_F-y+radius*SINF(0);
  vVertices[number-1][2] = 0.0f;

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_TRIANGLE_FAN,0,number);
#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_TRIANGLE_FAN);

		glVertex2f(x, SCREEN_HEIGHT_F-y);

		for(int i=0; i<360;i+=2)
		{
			glVertex2f(x+radius*COSF(i), SCREEN_HEIGHT_F-y+radius*SINF(i));
		}

		glVertex2f(x+radius*COSF(0), SCREEN_HEIGHT_F-y+radius*SINF(0));

	glEnd();

  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

  checkGlError();
}


void JRenderer::DrawPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color)
{
  checkGlError();
  JColor col;
	col.color = color;

	float angle = startingAngle*RAD2DEG;
	float steps = 360.0f/count;
	size /= 2;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  int i;
  GLfloat* vVertices = new GLfloat[3*count];
  GLubyte* colors = new GLubyte[4*count];

  for(i = 0; i < count; i++)
  {
    colors[4*i+0]= col.r;
    colors[4*i+1]= col.g;
    colors[4*i+2]= col.b;
    colors[4*i+3]= col.a;
  }

  for(i=0; i<count;i++)
  {
    vVertices[3*i+0] = x+size*COSF((int)angle);
    vVertices[3*i+1] = SCREEN_HEIGHT_F-(y+size*SINF((int)angle));
    vVertices[3*i+2] = 0.0f;
    angle += steps;
    if (angle >= 360.0f)
      angle -= 360.0f;
  }

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_LINE_LOOP,0,count);

  delete[] vVertices;
  delete[] colors;
#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_LINE_LOOP);

		for(int i=0; i<count;i++)
		{
			glVertex2f(x+size*COSF((int)angle), SCREEN_HEIGHT_F-(y+size*SINF((int)angle)));

			angle += steps;
			if (angle >= 360.0f)
				angle -= 360.0f;
		}

	glEnd();
  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

  checkGlError();
}


void JRenderer::FillPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color)
{
  checkGlError();
  JColor col;
	col.color = color;

	float angle = startingAngle*RAD2DEG;
	float firstAngle = angle;
	float steps = 360.0f/count;
	size /= 2;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  int i;
  GLfloat* vVertices = new GLfloat[3*(count+2)];
  GLubyte* colors = new GLubyte[4*(count+2)];

  for(i = 0; i < count+2; i++)
  {
    colors[4*i+0]= col.r;
    colors[4*i+1]= col.g;
    colors[4*i+2]= col.b;
    colors[4*i+3]= col.a;
  }

  vVertices[0] = x;
  vVertices[1] = SCREEN_HEIGHT_F-y;
  vVertices[2] = 0.0f;

  for(i=1; i<count+1;i++)
  {
    vVertices[3*i+0] = x+size*COSF((int)angle);
    vVertices[3*i+1] = SCREEN_HEIGHT_F-y+size*SINF((int)angle);
    vVertices[3*i+2] = 0.0f;
    angle += steps;
    if (angle >= 360.0f)
      angle -= 360.0f;
  }

  vVertices[3*(1+count)+0] = x+size*COSF((int)firstAngle);
  vVertices[3*(1+count)+1] = SCREEN_HEIGHT_F-y+size*SINF((int)firstAngle);
  vVertices[3*(1+count)+2] = 0.0f;

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_TRIANGLE_FAN,0,count+2);

  delete[] vVertices;
  delete[] colors;
#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_TRIANGLE_FAN);

		glVertex2f(x, SCREEN_HEIGHT_F-y);

		for(int i=0; i<count;i++)
		{
			glVertex2f(x+size*COSF((int)angle), SCREEN_HEIGHT_F-y+size*SINF((int)angle));
			angle += steps;
			if (angle >= 360.0f)
				angle -= 360.0f;
		}

		glVertex2f(x+size*COSF((int)firstAngle), SCREEN_HEIGHT_F-y+size*SINF((int)firstAngle));

	glEnd();

  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

  checkGlError();
}


void JRenderer::SetImageFilter(JImageFilter* imageFilter)
{
	mImageFilter = imageFilter;
}



void JRenderer::DrawRoundRect(float x, float y, float w, float h, float radius, PIXEL_TYPE color)
{
  checkGlError();
  x+=w+radius;
	y+=h+radius;
	JColor col;
	col.color = color;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  int i;
  int number = 360;
  GLfloat* vVertices = new GLfloat[3*number];
  GLubyte* colors = new GLubyte[4*number];

  for(i = 0; i < number; i++)
  {
    colors[4*i+0]= col.r;
    colors[4*i+1]= col.g;
    colors[4*i+2]= col.b;
    colors[4*i+3]= col.a;
  }


  for(i=0; i<90;i++) {
    vVertices[3 * i + 0] = x+radius*COSF(i);
    vVertices[3 * i + 1] = SCREEN_HEIGHT_F-(y+radius*SINF(i));
    vVertices[3 * i + 2] = 0.0f;
  }


  for(i=90; i<180;i++)
  {
    vVertices[3 * i + 0] = x+radius*COSF(i)-w;
    vVertices[3 * i + 1] = SCREEN_HEIGHT_F-(y+radius*SINF(i));
    vVertices[3 * i + 2] = 0.0f;
  }


  for(i=180; i<270;i++)
  {
    vVertices[3 * i + 0] = x+radius*COSF(i)-w;
    vVertices[3 * i + 1] = SCREEN_HEIGHT_F-(y+radius*SINF(i)-h);
    vVertices[3 * i + 2] = 0.0f;
  }

  for(i=270; i<360;i++)
  {
    vVertices[3 * i + 0] = x+radius*COSF(i);
    vVertices[3 * i + 1] = SCREEN_HEIGHT_F-(y+radius*SINF(i)-h);
    vVertices[3 * i + 2] = 0.0f;
  }

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );
  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );
  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_LINE_LOOP,0,number);

  delete[] vVertices;
  delete[] colors;
#else
  glDisable(GL_TEXTURE_2D);
  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_LINE_LOOP);
	int i;
	for(i=0; i<90;i++)
	{
		glVertex2f(x+radius*COSF(i), SCREEN_HEIGHT_F-(y+radius*SINF(i)));
	}
	for(i=0; i<w; i++)
	{
		glVertex2f(x+radius*COSF(90)-i, SCREEN_HEIGHT_F-(y+radius*SINF(90)));
	}
	for(i=90; i<180;i++)
	{
		glVertex2f(x+radius*COSF(i)-w, SCREEN_HEIGHT_F-(y+radius*SINF(i)));
	}
	for(i=0; i<h; i++)
	{
		glVertex2f(x+radius*COSF(180)-w, SCREEN_HEIGHT_F-(y+radius*SINF(180)-i));
	}
	for(i=180; i<270;i++)
	{
		glVertex2f(x+radius*COSF(i)-w, SCREEN_HEIGHT_F-(y+radius*SINF(i)-h));
	}
	for(i=0; i<w; i++)
	{
		glVertex2f(x+radius*COSF(270)-w+i, SCREEN_HEIGHT_F-(y+radius*SINF(270)-h));
	}
	for(i=270; i<360;i++)
	{
		glVertex2f(x+radius*COSF(i), SCREEN_HEIGHT_F-(y+radius*SINF(i)-h));
	}
	for(i=0; i<h; i++)
	{
		glVertex2f(x+radius*COSF(0), SCREEN_HEIGHT_F-(y+radius*SINF(0)-h+i));
	}
	glEnd();

  // default color
  glColor4ub(255, 255, 255, 255);
  glEnable(GL_TEXTURE_2D);
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

  checkGlError();
}



void JRenderer::FillRoundRect(float x, float y, float w, float h, float radius, PIXEL_TYPE color)
{
  checkGlError();
  x+=w+radius;
	y+=radius;

	JColor col;
	col.color = color;

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  int i, offset;
  int number = 2+360;
  GLfloat* vVertices = new GLfloat[3*number];
  GLubyte* colors = new GLubyte[4*number];

  for(i = 0; i < number; i++)
  {
    colors[4*i+0]= col.r;
    colors[4*i+1]= col.g;
    colors[4*i+2]= col.b;
    colors[4*i+3]= col.a;
  }

  vVertices[0] = x-5; vVertices[1] = SCREEN_HEIGHT_F-y; vVertices[2] = 0.0f;
  offset = 1;
  for(i=0; i<90;i++) {
    vVertices[3*(offset+i)+0] = x+radius*COSF(i);
    vVertices[3*(offset+i)+1] = SCREEN_HEIGHT_F-y+radius*SINF(i);
    vVertices[3*(offset+i)+2] = 0.0f;
  }


  for(i=90; i<180;i++)
  {
    vVertices[3*(offset+i)+0] = x+radius*COSF(i)-w;
    vVertices[3*(offset+i)+1] = SCREEN_HEIGHT_F-y+radius*SINF(i);
    vVertices[3*(offset+i)+2] = 0.0f;
  }

  for(i=180; i<270;i++)
  {
    vVertices[3*(offset+i)+0] = x+radius*COSF(i)-w;
    vVertices[3*(offset+i)+1] = SCREEN_HEIGHT_F-y+radius*SINF(i)-h;
    vVertices[3*(offset+i)+2] = 0.0f;
  }


  for(i=270; i<360;i++)
  {
    vVertices[3*(offset+i)+0] = x+radius*COSF(i);
    vVertices[3*(offset+i)+1] = SCREEN_HEIGHT_F-y+radius*SINF(i)-h;
    vVertices[3*(offset+i)+2] = 0.0f;
  }

  vVertices[3*(361)+0] = x+radius*COSF(0);
  vVertices[3*(361)+1] = SCREEN_HEIGHT_F-y+radius*SINF(0);
  vVertices[3*(361)+2] = 0.0f;

  // Use the program object without texture
  glUseProgram ( prog1 );

  // Load the vertex position
  glVertexAttribPointer ( prog1_positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices );

  // Load the color
  glVertexAttribPointer(prog1_colorLoc, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, 4 * sizeof(GLubyte), colors);

  glEnableVertexAttribArray ( prog1_positionLoc );

  glEnableVertexAttribArray ( prog1_colorLoc );

  // Load the MVP matrix
  glUniformMatrix4fv( prog1_mvpLoc, 1, GL_FALSE, (GLfloat*) &theMvpMatrix.m[0][0] );

  glDrawArrays(GL_TRIANGLE_FAN,0,number);

  delete[] vVertices;
  delete[] colors;
#else
  glDisable(GL_TEXTURE_2D);

  glColor4ub(col.r, col.g, col.b, col.a);
  glBegin(GL_TRIANGLE_FAN);

	glVertex2f(x-5, SCREEN_HEIGHT_F-y);

	int i;
	for(i=0; i<90;i++)
	{
		glVertex2f(x+radius*COSF(i), SCREEN_HEIGHT_F-y+radius*SINF(i));
	}
	for(i=0; i<w; i++)
	{
		glVertex2f(x+radius*COSF(90)-i, SCREEN_HEIGHT_F-y+radius*SINF(90));
	}
	for(i=90; i<180;i++)
	{
		glVertex2f(x+radius*COSF(i)-w, SCREEN_HEIGHT_F-y+radius*SINF(i));
	}
	for(i=0; i<h; i++)
	{
		glVertex2f(x+radius*COSF(180)-w, SCREEN_HEIGHT_F-y+radius*SINF(180)-i);
	}

	for(i=180; i<270;i++)
	{
		glVertex2f(x+radius*COSF(i)-w, SCREEN_HEIGHT_F-y+radius*SINF(i)-h);
	}
	for(i=0; i<w; i++)
	{
		glVertex2f(x+radius*COSF(270)-w+i, SCREEN_HEIGHT_F-y+radius*SINF(270)-h);
	}
	for(i=270; i<360;i++)
	{
		glVertex2f(x+radius*COSF(i), SCREEN_HEIGHT_F-y+radius*SINF(i)-h);
	}
	for(i=0; i<h; i++)
	{
		glVertex2f(x+radius*COSF(0), SCREEN_HEIGHT_F-y+radius*SINF(0)-h+i);
	}

	glVertex2f(x+radius*COSF(0), SCREEN_HEIGHT_F-y+radius*SINF(0));

	glEnd();

  // default color
  glColor4ub(255, 255, 255, 255);

  glEnable(GL_TEXTURE_2D);
#endif //#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
  checkGlError();
}

