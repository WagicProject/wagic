//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
#include "../../include/JGE.h"
#include "../../include/JRenderer.h"
#include "../../include/JResourceManager.h"
#include "../../include/JFileSystem.h"
#include "../../include/JAssert.h"

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

    if(mTex)
    {
        mTX0 = x/mTex->mTexWidth;
        mTY0 = y/mTex->mTexHeight;
        mTX1 = (x+w)/mTex->mTexWidth;
        mTY1 = (y+h)/mTex->mTexHeight;
    }
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
    if (mBuffer)
    {
        delete [] mBuffer;
        mBuffer = NULL;
    }
}


void JTexture::UpdateBits(int x, int y, int width, int height, PIXEL_TYPE* bits)
{
    JRenderer::GetInstance()->BindTexture(this);
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

JRenderer::JRenderer() :
mLeft(0.0f),
mRight(SCREEN_WIDTH_F),
mTop(0.0f),
mBottom(SCREEN_HEIGHT_F)
{
}


JRenderer::~JRenderer()
{

}

void JRenderer::InitRenderer()
{
    mCurrentTextureFilter = TEX_FILTER_NONE;
    mImageFilter = NULL;

    mCurrTexBlendSrc = BLEND_SRC_ALPHA;
    mCurrTexBlendDest = BLEND_ONE_MINUS_SRC_ALPHA;

    //	mLineWidth = 1.0f;
    mFOV = 75.0f;

#ifdef USING_MATH_TABLE
    for (int i=0;i<360;i++)
    {
        mSinTable[i] = sinf(i*DEG2RAD);
        mCosTable[i] = cosf(i*DEG2RAD);
    }
#endif

    mCurrentRenderMode = MODE_UNKNOWN;
}

void JRenderer::DestroyRenderer()
{
}

void JRenderer::BeginScene()
{
}

void JRenderer::EndScene()
{
}

void JRenderer::BindTexture(JTexture *tex)
{
}


void JRenderer::EnableTextureFilter(bool flag)
{
    if (flag)
        mCurrentTextureFilter = TEX_FILTER_LINEAR;
    else
        mCurrentTextureFilter = TEX_FILTER_NEAREST;
}

void Swap(float *a, float *b)
{
    float n=*a;
    *a = *b;
    *b = n;
}

void JRenderer::RenderQuad(JQuad* quad, float xo, float yo, float angle, float xScale, float yScale)
{
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

    yo = SCREEN_HEIGHT_F - yo;
}


void JRenderer::RenderQuad(JQuad* quad, VertexColor* pt)
{
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
}


void JRenderer::FillRect(float x, float y, float width, float height, PIXEL_TYPE color)
{
    y = SCREEN_HEIGHT_F - y - height;

    JColor col;
    col.color = color;
}


void JRenderer::DrawRect(float x, float y, float width, float height, PIXEL_TYPE color)
{
    y = SCREEN_HEIGHT_F - y - height;

    JColor col;
    col.color = color;
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
    y = SCREEN_HEIGHT_F - y - height;
}


void JRenderer::DrawLine(float x1, float y1, float x2, float y2, PIXEL_TYPE color)
{
    //	glLineWidth (mLineWidth);
    JColor col;
    col.color = color;
}


void JRenderer::Plot(float x, float y, PIXEL_TYPE color)
{
}


void JRenderer::PlotArray(float *x, float *y, int count, PIXEL_TYPE color)
{
}



void JRenderer::ScreenShot(const char* filename __attribute__((unused)))
{

}

void JRenderer::TransferTextureToGLContext(JTexture& inTexture)
{
}

JTexture* JRenderer::CreateTexture(int width, int height, int mode __attribute__((unused)))
{
    JTexture *tex = new JTexture();

    return tex;
}

JTexture* JRenderer::LoadTexture(const char* filename, int mode, int textureFormat)
{
    JTexture *tex = new JTexture();

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
}


void JRenderer::SetTexBlend(int src, int dest)
{
}


void JRenderer::SetTexBlendSrc(int src)
{
}


void JRenderer::SetTexBlendDest(int dest)
{
}


void JRenderer::Enable2D()
{
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


void JRenderer::SetClip(int, int, int, int)
{// NOT USED
    //glScissor(x, y, width, height);
}


void JRenderer::LoadIdentity()
{// NOT USED
    //glLoadIdentity();
}


void JRenderer::Translate(float, float, float)
{// NOT USED
    //glTranslatef(x, y, z);
}


void JRenderer::RotateX(float)
{// NOT USED
    //glRotatef(angle*RAD2DEG, 1.0f, 0.0f, 0.0f);
}


void JRenderer::RotateY(float)
{// NOT USED
    //glRotatef(angle*RAD2DEG, 0.0f, 1.0f, 0.0f);
}


void JRenderer::RotateZ(float)
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
}


void JRenderer::SetFOV(float fov)
{
    mFOV = fov;
}


void JRenderer::FillPolygon(float* x, float* y, int count, PIXEL_TYPE color)
{
}


void JRenderer::DrawPolygon(float* x, float* y, int count, PIXEL_TYPE color)
{
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
}

void JRenderer::FillCircle(float x, float y, float radius, PIXEL_TYPE color)
{
}


void JRenderer::DrawPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color)
{
}


void JRenderer::FillPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color)
{
}


void JRenderer::SetImageFilter(JImageFilter* imageFilter)
{
    mImageFilter = imageFilter;
}



void JRenderer::DrawRoundRect(float x, float y, float w, float h, float radius, PIXEL_TYPE color)
{
}



void JRenderer::FillRoundRect(float x, float y, float w, float h, float radius, PIXEL_TYPE color)
{
}

