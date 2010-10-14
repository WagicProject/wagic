#include "../include/config.h"
#include "../include/WFont.h"
#include "../include/WResourceManager.h"
#include "JFileSystem.h"

#define ISGBK(c) ((c) > 0x80 || (c) < 0x30 || (c) == '-' || (c) == '/')

static PIXEL_TYPE gencolor(int id, PIXEL_TYPE color)
{
  unsigned int a, r, g, b, r0, g0, b0;
#if defined (WIN32) || defined (LINUX)
  a = (color >> 24) & 0xFF;
  r = (color >> 16) & 0xFF;
  g = (color >>  8) & 0xFF;
  b = (color >>  0) & 0xFF;
#else // PSP
# if defined (ABGR8888)
  a = (color >> 24) & 0xFF;
  b = (color >> 16) & 0xFF;
  g = (color >>  8) & 0xFF;
  r = (color >>  0) & 0xFF;
# elif defined (ABGR5551)
  a = ((color >> 15) & 0x01) << 7;
  b = ((color >> 10) & 0x1F) << 3;
  g = ((color >>  5) & 0x1F) << 3;
  r = ((color >>  0) & 0x1F) << 3;
# elif defined (ABGR4444)
  a = ((color >> 12) & 0x0F) << 4;
  b = ((color >>  8) & 0x0F) << 4;
  g = ((color >>  4) & 0x0F) << 4;
  r = ((color >>  0) & 0x0F) << 4;
# endif
#endif
  r0 = g0 = b0 = 255;

  switch (id) {
  case 0: // simon 245, 228, 156
    r0 = 245; g0 = 228; b0 = 156;
    break;
  case 1: // f3 255, 252, 175
    r0 = 255; g0 = 252; b0 = 175;
    break;
  case 2: // magic 219, 255, 151
    r0 = 219; g0 = 255; b0 = 151;
    break;
  case 3: // smallface 255, 255, 255
    r0 = 255; g0 = 255; b0 = 255;
    break;
  default:
    ;
  }
  r = r * r0 / 255;
  g = g * g0 / 255;
  b = b * b0 / 255;

  return ARGB(a,r,g,b);
}

static int genmana(u8 c)
{
  int mana = -1;
  switch (c) {
  case 0xC7:
    mana = Constants::MTG_COLOR_GREEN;
    break;
  case 0xD5:
    mana = Constants::MTG_COLOR_BLUE;
    break;
  case 0xD2:
    mana = Constants::MTG_COLOR_RED;
    break;
  case 0xC2:
    mana = Constants::MTG_COLOR_BLACK;
    break;
  case 0xD7:
    mana = Constants::MTG_COLOR_WHITE;
    break;
  case 0xD4: // T
  case 0xD8: // X
  case 0xD9: // Y
    mana = Constants::MTG_UNCOLORED;
    break;
  default:
    if (c >= 0xB0 && c <= 0xB9)
      mana = Constants::MTG_UNCOLORED;
    else
      mana = -1;
  }
  return mana;
}

//

JRenderer * WFBFont::mRenderer = NULL;

WFBFont::WFBFont(const char *fontname, int lineheight, bool useVideoRAM)
{
  mRenderer = JRenderer::GetInstance();

  mCharBuffer = NULL;
  mSprites = NULL;
  mGBCode = NULL;
  mCurr = 0;

  char chnFileName[32], engFileName[32];
  sprintf(chnFileName, "%s.gbk", fontname);
  sprintf(engFileName, "%s.asc", fontname);
  JFileSystem *fileSys = JFileSystem::GetInstance();
  int size = 0;

  if (!fileSys->OpenFile(chnFileName))
    return;
  size = fileSys->GetFileSize();
  mChnFont = NEW u8[size];
  fileSys->ReadFile(mChnFont, size);
  fileSys->CloseFile();

  if (!fileSys->OpenFile(engFileName))
    return;
  size = fileSys->GetFileSize();
  mEngFont = NEW u8[size];
  fileSys->ReadFile(mEngFont, size);
  fileSys->CloseFile();

  mColor0 = ARGB(255, 255, 255, 255);
  mColor = mColor0;
  mFontSize = lineheight;
  mScale = 1.0f;

  // using 4-bit(half-byte) to store 1 pixel
  mBytesPerRow = static_cast<unsigned int>(mFontSize / 2);
  mBytesPerChar =  static_cast<unsigned int>(mBytesPerRow*mFontSize);

  mCacheImageWidth = 256;
  mCacheImageHeight = 256;
  mCol = mCacheImageWidth / mFontSize;
  mRow = mCacheImageHeight / mFontSize;
  mCacheSize = mCol * mRow;

  mSprites = NEW JQuad*[mCacheSize];
  mGBCode = NEW int[mCacheSize];

#if defined (WIN32) || defined (LINUX)
  mCharBuffer = NEW u32[mFontSize*mFontSize];
#endif

  mTexture = mRenderer->CreateTexture(mCacheImageWidth, mCacheImageHeight, true);

  int index = 0;
  for (int y = 0; y < mRow; y++) {
    for (int x = 0; x<mCol; x++) {
      mGBCode[index] = -1;
      mSprites[index] = NEW JQuad(mTexture, 
                                  static_cast<float>(x*mFontSize), 
                                  static_cast<float>(y*mFontSize),
                                  static_cast<float>(mFontSize),
                                  static_cast<float>(mFontSize));
      mSprites[index]->SetHotSpot(static_cast<float>(mFontSize / 2), static_cast<float>(mFontSize / 2));
      index++;
    }
  }
}

WFBFont::~WFBFont()
{
  SAFE_DELETE(mEngFont);
  SAFE_DELETE(mChnFont);
  SAFE_DELETE(mTexture);

  if (mSprites) {
    for (int i = 0; i < mCacheSize; i++) {
      if (mSprites[i])
        delete mSprites[i];
    }
    delete [] mSprites;
  }

  if (mGBCode)
    delete [] mGBCode;

  if (mCharBuffer)
    delete [] mCharBuffer;
}

#if defined (WIN32) || defined (LINUX)
#else
static void SwizzlePlot(u8* out, PIXEL_TYPE color, int i, int j, unsigned int width)
{
  unsigned int rowblocks = (width >> 4);

  unsigned int blockx = (i >> 4);
  unsigned int blocky = (j >> 3);

  unsigned int x = (i - (blockx << 4));
  unsigned int y = (j - (blocky << 3));
  unsigned int block_index = blockx + ((blocky) * rowblocks);
  unsigned int block_address = block_index << 7;

  u8* p = out + (block_address + x + (y << 4));
  PIXEL_TYPE* dest = (PIXEL_TYPE *)p;
  *dest = color;
}
#endif

int WFBFont::PreCacheChar(const u8 *ch)
{
  int code;
  bool isChinese = true;
  u8 * src;
  unsigned int size, offset;
  u8 gray;

  if (*ch > 0xA0 && *(ch + 1) > 0xA0)
    // get offset to the proper character bits (GB2312 encoding)
    code = (((u32)(*ch - 0xA1)) * 0x5E + ((u32)(*(ch + 1) - 0xA1)));
  else if (*ch > 0x80) {
    // get offset to the character space's bits (GBK encoding)
    code = 0;
  }
  else {
    code = ((u32)*ch)|0x10000;
    isChinese = false;
  }

  if (mGBCode[mCurr] != -1) {
    for (int i = 0; i < mCacheSize; i++) {
      if (mGBCode[i] == code)
        return i;
    }
  }
  int index = mCurr++;
  if (mCurr >= mCacheSize)
    mCurr = 0;

#if defined (WIN32) || defined (LINUX)
  int x = 0;
  int y = 0;
  memset(mCharBuffer, 0, sizeof(u32) * mFontSize * mFontSize);
#else
  u8* pTexture = (u8*) mTexture->mBits;
  int x;
  int y = (int)mSprites[index]->mY;
#endif

  if (isChinese) {
    size = mFontSize;
    src = mChnFont + code * mBytesPerChar;
    offset = 0;
  }
  else {
    size = mFontSize / 2;
    src = mEngFont +  (code - 0x10000) * (mFontSize * size / 2);
    offset = 0;
  }

  // set up the font texture buffer
  for (unsigned int i = 0; i < mFontSize; i++) {
#if defined (WIN32) || defined (LINUX)
    x = 0;
#else
    x = (int)mSprites[index]->mX;
#endif
    unsigned int j = 0;
#if 1
    for (; j < offset; j++) {
#if defined (WIN32) || defined (LINUX)
      mCharBuffer[y * mFontSize + x] = ARGB(0, 0, 0, 0);
#else
      SwizzlePlot(pTexture, ARGB(0, 0, 0, 0), x * PIXEL_SIZE, y, mTexture->mTexWidth * PIXEL_SIZE);
#endif
      x++;
    }
#endif
    for (; j < offset + size; j++) {
      // as 4-bit(half-byte) stores 1 pixel
      // get out the proper data according to the even or odd quality of the counter
      gray = src[(i * size + j - offset) / 2];
      gray = ((j - offset) & 1) ? (gray & 0xF0) : ((gray & 0x0F) << 4);
      if (gray) gray |= 0x0F;
#if defined (WIN32) || defined (LINUX)
      mCharBuffer[y * mFontSize + x] = ARGB(gray, 255, 255, 255);
#else
      SwizzlePlot(pTexture, ARGB(gray, 255, 255, 255), x * PIXEL_SIZE, y, mTexture->mTexWidth * PIXEL_SIZE);
#endif
      x++;
    }
    for (; j < mFontSize; j++) {
#if defined (WIN32) || defined (LINUX)
      mCharBuffer[y * mFontSize + x] = ARGB(0, 0, 0, 0);
#else
      SwizzlePlot(pTexture, ARGB(0, 0, 0, 0), x * PIXEL_SIZE, y, mTexture->mTexWidth * PIXEL_SIZE);
#endif
      x++;
    }
    y++;
  }

  mGBCode[index] = code;

#if defined (WIN32) || defined (LINUX)
  x = (int)mSprites[index]->mX;
  y = (int)mSprites[index]->mY;
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, mFontSize, mFontSize, GL_RGBA, GL_UNSIGNED_BYTE, mCharBuffer);
#else
  sceKernelDcacheWritebackAll();
#endif
  return index;
}

void WFBFont::DrawString(const char *s, float x, float y, int align, float leftOffset, float width)
{
  unsigned char c = *(unsigned short *)s & 0xFF;
  if (ISGBK(c) || (s[1] == ':' && s[2] == ' ')) {}
  else {
    WFont * mFont = resources.GetWLBFont(id);
    mFont->SetScale(GetScale());
    mFont->SetColor(GetColor());
    mFont->DrawString(s, x, y, align, leftOffset, width);
    return;
  }

  u8 * str = (u8 *)s;

  // (0, 0) refers to the center of the word, so fix it to the upper-left corner
  x += (mFontSize * mScale) / 2;
  y += (mFontSize * mScale) / 2;
  switch (align) {
  case JGETEXT_RIGHT:
    x -= GetStringWidth(s);
    break;
  case JGETEXT_CENTER:
    x -= GetStringWidth(s) / 2;
    break;
  case JGETEXT_LEFT:
  default:
    break;
  }

  mRenderer->BindTexture(mTexture);

  u8 * src = str;
  float xx = x;
  float yy = y;
  int index = 0;

  bool isChinese=true;

  while (*src != 0) {
    if (yy > SCREEN_HEIGHT_F)         // don't render or count outside the buttom of viewport
      return;
    else if (yy + mFontSize < 0.0f) { // don't render when outside the top of viewport, but counted
      if (*src < ' ') {     // control characters
        if (*src == 0x0a) { // NEWLINE
          xx = x;
          yy += (mFontSize*mScale);
        }
        src += 1;
      }
      else {
        if (*src > 0x80)    // Chinese characters (GBK encoding)
          src += 2;
        else
          src += 1;

        xx += (mFontSize*mScale);

        if (xx >= 480) {
          xx = x;
          yy += (mFontSize*mScale);
        }
      }
    }
    else {
      if (*src < ' ') {     // control characters
        if (*src == 0x0a) { // NEWLINE
          xx = x;
          yy += (mFontSize * mScale);
        }
        src += 1;
      }
      else {
        int mana = -1;
        if (*src > 0x80) {  // Chinese characters  (GBK encoding)
          if (*src == 0xa3) mana = genmana(*(src+1));
          index = PreCacheChar(src);
          src += 2;
          isChinese = true;
        }
        else {
          index = PreCacheChar(src);
          src += 1;
          isChinese = false;
        }

        // fix for leftoffset and witdth's setting
        float xPos, yPos, charWidth, charHeight;
        mSprites[index]->GetTextureRect(&xPos, &yPos, &charWidth, &charHeight);
        float xPos0 = xPos;
        float charWidth0 = charWidth;
        float delta = (isChinese) ? (charWidth * mScale) : (charWidth * mScale / 2);
        if (leftOffset) {
          if (leftOffset < 0) { 
            xx -= leftOffset;
            leftOffset = 0;
          }
          else if (leftOffset - delta > 0) { 
            leftOffset -= delta;
            continue;
          }
          else {
            xPos += leftOffset / mScale;
            delta -= leftOffset;
            leftOffset = 0;
            charWidth = delta / mScale;
          }   
        }
        else if (width) {
          if (xx > x + width)
            return;
          if (xx + delta > x + width) {
            delta = x + width - xx;
            charWidth = delta / mScale;
          }
        }

        if (mana >= 0) {
          if (*src == '/') { // hybrid mana cost
            src += 1;
            int mana2 = genmana(*(src+1));
            src += 2;
            unsigned char t = (JGE::GetInstance()->GetTime() / 3) & 0xFF;
            unsigned char v = t + 127;
            float scale = 0.05f * cosf(2*M_PI*((float)t)/256.0f);
            if (scale < 0) {
              mRenderer->RenderQuad(manaIcons[mana], xx + 3 * sinf(2*M_PI*((float)t)/256.0f), yy + 3 * cosf(2*M_PI*((float)(t-35))/256.0f), 0, 0.5f * mScale, 0.5f * mScale);
              mRenderer->RenderQuad(manaIcons[mana2], xx + 3 * sinf(2*M_PI*((float)v)/256.0f), yy + 3 * cosf(2*M_PI*((float)(v-35))/256.0f), 0, 0.5f * mScale, 0.5f * mScale);
            }
            else {
              mRenderer->RenderQuad(manaIcons[mana2], xx + 3 * sinf(2*M_PI*((float)v)/256.0f), yy + 3 * cosf(2*M_PI*((float)(v-35))/256.0f), 0, 0.5f * mScale, 0.5f * mScale);
              mRenderer->RenderQuad(manaIcons[mana], xx + 3 * sinf(2*M_PI*((float)t)/256.0f), yy + 3 * cosf(2*M_PI*((float)(t-35))/256.0f), 0, 0.5f * mScale, 0.5f * mScale);
            }
            mana = Constants::MTG_NB_COLORS + 1; // donot draw colorless cost in hybrid mana cost
          }
          else
            mRenderer->RenderQuad(manaIcons[mana], xx, yy, 0, 0.5f * mScale, 0.5f * mScale);
          mRenderer->BindTexture(mTexture); // manaIcons use different texture, so we need to rebind it.
        }

        if (mana <= 0) {
          mSprites[index]->SetTextureRect(xPos, yPos, charWidth, charHeight);
          mSprites[index]->SetColor(mColor);
          mRenderer->RenderQuad(mSprites[index], xx, yy, 0, mScale, mScale);
          mSprites[index]->SetTextureRect(xPos0, yPos, charWidth0, charHeight);
        }

        xx += delta;

        if (xx >= 480) {
          xx = x;
          yy += (mFontSize * mScale);
        }
      }
    }
  }
}

void WFBFont::DrawString(std::string s, float x, float y, int align, float leftOffset, float width)
{
  DrawString(s.c_str(),x,y,align,leftOffset,width);
}

void WFBFont::SetColor(PIXEL_TYPE color)
{
  mColor0 = color;
  mColor = gencolor(id, color);
}

float WFBFont::GetStringWidth(const char *s) const
{
  unsigned char c = *(unsigned short *)s & 0xFF;

  if (ISGBK(c)) {
    u8 * src = (u8 *)s;
    float xx = 0;
    bool isChinese=true;

    while (*src != 0) {
      if (*src > 0x80) { // Chinese
        src += 2;
        isChinese = true;
      }
      else {             // Non-Chinese
        src += 1;
        isChinese = false;
      }
      if (isChinese)
        xx += (mFontSize * mScale);
      else
        xx += (mFontSize * mScale) / 2;
    }
    return xx;
  }
  else {
    WFont * mFont = resources.GetWLBFont(id);
    mFont->SetScale(GetScale());
    return mFont->GetStringWidth(s);
  }
}

void WFBFont::SetScale(float scale)
{
  mScale = scale;
}

float WFBFont::GetScale() const {return mScale;}
float WFBFont::GetHeight() const {return (mFontSize * mScale);}

