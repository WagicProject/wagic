#include "PrecompiledHeader.h"

#include "WFont.h"
#include "WResourceManager.h"
#include "JFileSystem.h"
#include "GameApp.h"

#define ISGBK(c) ((c) > 0x80 || (c) < 0x30 || (c) == '-' || (c) == '/')

static PIXEL_TYPE gencolor(int id, PIXEL_TYPE color)
{
    unsigned int a, r, g, b, r0, g0, b0;
#if defined (PSP)
# if defined (ABGR8888)
    a = (color >> 24) & 0xFF;
    b = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    r = (color >> 0) & 0xFF;
# elif defined (ABGR5551)
    a = ((color >> 15) & 0x01) << 7;
    b = ((color >> 10) & 0x1F) << 3;
    g = ((color >> 5) & 0x1F) << 3;
    r = ((color >> 0) & 0x1F) << 3;
# elif defined (ABGR4444)
    a = ((color >> 12) & 0x0F) << 4;
    b = ((color >> 8) & 0x0F) << 4;
    g = ((color >> 4) & 0x0F) << 4;
    r = ((color >> 0) & 0x0F) << 4;
# endif
#else // non PSP
    a = (color >> 24) & 0xFF;
    r = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    b = (color >> 0) & 0xFF;
#endif
    r0 = g0 = b0 = 255;

    switch (id)
    {
    case Fonts::MAIN_FONT: // simon 245, 228, 156
        r0 = 245;
        g0 = 228;
        b0 = 156;
        break;
    case Fonts::MENU_FONT: // f3 255, 252, 175
        r0 = 255;
        g0 = 252;
        b0 = 175;
        break;
    case Fonts::MAGIC_FONT: // magic 219, 255, 151
        r0 = 219;
        g0 = 255;
        b0 = 151;
        break;
    case Fonts::SMALLFACE_FONT: // smallface 255, 255, 255
        r0 = 255;
        g0 = 255;
        b0 = 255;
        break;
    default:
        ;
    }
    r = r * r0 / 255;
    g = g * g0 / 255;
    b = b * b0 / 255;

    return ARGB(a,r,g,b);
}

static inline bool doubleWidthChar(const u8* const src)
{
    // Determination of this is done with < 0xC0, which is not
    // right but okay for now (in particular it will return 2
    // for european accented characters)
    return *src >= 0xC0;
}

static inline bool GBKDoubleWidthChar(const u8* const src)
{
    // Determination of this is done with < 0xC0, which is not
    // right but okay for now (in particular it will return 2
    // for european accented characters)
    return *src >= 0x80;
}

// This returns the size in bytes of a character.
// The 4 first bits of the leading byte indicate the length.
// The zeroes in the middle match the 10xx forms, which are
// only found in trailing bytes.
static inline int charWidth(const u8 s)
{
    // The following is the "correct" one, but if called on incorrect strings
    // the presence of 0's may create an infinite loop, so replace them with
    // 1's. This may happen when changing from chinese to japanese because the
    // encoding for chinese is not regular.
    //  static const int sizes[] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 3, 4};
    static const int sizes[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4 };
#if 0
    // Reactivate to support being called on trailing bytes. This is not needed at the moment
    // as this function will always be passed leading bytes.
    while (0 == sizes[s >> 4]) --s;
#endif
    return sizes[s >> 4];
}

//

JRenderer * WFBFont::mRenderer = NULL;

WLBFont::WLBFont(int inFontID, const char *fontname, int lineheight, bool useVideoRAM) :
    WFont(inFontID)
{
    string path(fontname);
    if (path.size() > 4) path = path.substr(0, path.size() - 4); //some stupid manipulation because of the way Font works in JGE
    it = NEW JLBFont(path.c_str(), lineheight, useVideoRAM);
}

void WLBFont::FormatText(string &s, vector<string>& output)
{
    // The way of CardPrimitive::formattedText() in r2081.
    std::string::size_type len = 30;
    while (s.length() > 0)
    {
        std::string::size_type cut = s.find_first_of("., \t)", 0);
        if (cut >= len || cut == string::npos)
        {
            output.push_back(s.substr(0,len));
            if (s.length() > len)
                s = s.substr(len, s.length() - len);
            else
                s = "";
        }
        else
        {
            std::string::size_type newcut = cut;
            while (newcut < len && newcut != string::npos)
            {
                cut = newcut;
                newcut = s.find_first_of("., \t)", newcut + 1);
            }
            output.push_back(s.substr(0,cut+1));
            if (s.length() > cut+1)
                s = s.substr(cut+1,s.length() - cut - 1);
            else
                s = "";
        }
    }
}

WFBFont::WFBFont(int inFontID, const char *fontname, int lineheight, bool useVideoRAM) :
    WFont(inFontID)
{
    mRenderer = JRenderer::GetInstance();

    mCharBuffer = NULL;
    mSprites = NULL;
    mGBCode = NULL;
    mCurr = 0;

    char tmpFileName[32], engFileName[32];
    strcpy(tmpFileName, fontname);
    char * ep = strrchr(tmpFileName, '.');
    *ep = '\0';
    sprintf(engFileName, "%s.asc", tmpFileName);
    JFileSystem *fileSys = JFileSystem::GetInstance();
    int size = 0;
    struct
    {
        unsigned short chars;
        unsigned char width;
        unsigned char height;
    } sizeStr = { 0, 0, 0 };

    if (!fileSys->OpenFile(engFileName)) return;
    size = fileSys->GetFileSize();
    mStdFont = NEW u8[size];
    fileSys->ReadFile(mStdFont, size);
    fileSys->CloseFile();

    if (!fileSys->OpenFile(fontname)) return;
    fileSys->ReadFile(&sizeStr, 4); // Works only for little-endian machines (PSP and PC are)
    size = sizeStr.chars * sizeStr.width * sizeStr.height / 2;
    mExtraFont = NEW u8[size]; // 4 bits for a pixel
    mIndex = NEW u16[65536];
    fileSys->ReadFile(mIndex, 65536 * sizeof(u16));
    fileSys->ReadFile(mExtraFont, size);
    fileSys->CloseFile();

    mColor0 = ARGB(255, 255, 255, 255);
    mColor = mColor0;
    mFontSize = lineheight;
    mScale = 1.0f;

    // using 4-bit(half-byte) to store 1 pixel
    mBytesPerRow = static_cast<unsigned int> (mFontSize / 2);
    mBytesPerChar = static_cast<unsigned int> (mBytesPerRow * mFontSize);

    mCacheImageWidth = 256;
    mCacheImageHeight = 256;
    mCol = mCacheImageWidth / mFontSize;
    mRow = mCacheImageHeight / mFontSize;
    mCacheSize = mCol * mRow;

    mSprites = NEW JQuad*[mCacheSize];
    mGBCode = NEW int[mCacheSize];

#if !defined (PSP)
    mCharBuffer = NEW u32[mFontSize*mFontSize];
#endif

    mTexture = mRenderer->CreateTexture(mCacheImageWidth, mCacheImageHeight, true);

    int index = 0;
    for (int y = 0; y < mRow; y++)
    {
        for (int x = 0; x < mCol; x++)
        {
            mGBCode[index] = -1;
            mSprites[index] = NEW JQuad(mTexture, static_cast<float> (x * mFontSize), static_cast<float> (y * mFontSize),
                            static_cast<float> (mFontSize), static_cast<float> (mFontSize));
            mSprites[index]->SetHotSpot(static_cast<float> (mFontSize / 2), static_cast<float> (mFontSize / 2));
            index++;
        }
    }
}

WFBFont::~WFBFont()
{
    SAFE_DELETE_ARRAY(mStdFont);
    SAFE_DELETE_ARRAY(mExtraFont);
    SAFE_DELETE(mTexture);

    if (mSprites)
    {
        for (int i = 0; i < mCacheSize; i++)
        {
            if (mSprites[i]) delete mSprites[i];
        }
        delete[] mSprites;
    }

    if (NULL != mIndex) delete[] mIndex;

    if (mGBCode) delete[] mGBCode;

    if (mCharBuffer) delete[] mCharBuffer;
}

#if defined (PSP)
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
    PIXEL_TYPE* dest = (PIXEL_TYPE *) p;
    *dest = color;
}
#endif

int WFBFont::PreCacheChar(const u8 *ch)
{
    int code;
    int charLength = 1;
    u8 * src;
    unsigned int size, offset;
    u8 gray;

    code = this->GetCode(ch, &charLength);
    if (doubleWidthChar(ch) && mIndex) code = mIndex[code]; // mGBCode[] stores the final code.

    if (mGBCode[mCurr] != -1) for (int i = 0; i < mCacheSize; i++)
        if (mGBCode[i] == code) return i;

    int index = mCurr++;
    if (mCurr >= mCacheSize) mCurr = 0;

#if defined (PSP)
    u8* pTexture = (u8*) mTexture->mBits;
    int x;
    int y = (int) mSprites[index]->mY;
#else
    int x = 0;
    int y = 0;
    memset(mCharBuffer, 0, sizeof(u32) * mFontSize * mFontSize);
#endif

    if (doubleWidthChar(ch))
    {
        //if (mIndex) code = mIndex[code];
        size = mFontSize;
        src = mExtraFont + code * mBytesPerChar;
        offset = 0;
    }
    else
    {
        size = mFontSize / 2;
        src = mStdFont + code * (mFontSize * size / 2);
        offset = 0;
    }

    // set up the font texture buffer
    for (unsigned int i = 0; i < mFontSize; i++)
    {
#if defined (WIN32) || defined (LINUX)
        x = 0;
#else
        x = (int) mSprites[index]->mX;
#endif
        unsigned int j = 0;
#if 1
        for (; j < offset; j++)
        {
#if defined (PSP)
            SwizzlePlot(pTexture, ARGB(0, 0, 0, 0), x * PIXEL_SIZE, y, mTexture->mTexWidth * PIXEL_SIZE);
#else
            mCharBuffer[y * mFontSize + x] = ARGB(0, 0, 0, 0);
#endif
            x++;
        }
#endif
        for (; j < offset + size; j++)
        {
            // as 4-bit(half-byte) stores 1 pixel
            // get out the proper data according to the even or odd quality of the counter
            gray = src[(i * size + j - offset) / 2];
            gray = ((j - offset) & 1) ? (gray & 0xF0) : ((gray & 0x0F) << 4);
            if (gray) gray |= 0x0F;
#if defined (PSP)
            SwizzlePlot(pTexture, ARGB(gray, 255, 255, 255), x * PIXEL_SIZE, y, mTexture->mTexWidth * PIXEL_SIZE);
#else
            mCharBuffer[y * mFontSize + x] = ARGB(gray, 255, 255, 255);
#endif
            x++;
        }
        for (; j < mFontSize; j++)
        {
#if defined (PSP)
            SwizzlePlot(pTexture, ARGB(0, 0, 0, 0), x * PIXEL_SIZE, y, mTexture->mTexWidth * PIXEL_SIZE);
#else
            mCharBuffer[y * mFontSize + x] = ARGB(0, 0, 0, 0);
#endif
            x++;
        }
        y++;
    }

    mGBCode[index] = code;

#if defined (PSP)
    sceKernelDcacheWritebackAll();
#else
    x = (int)mSprites[index]->mX;
    y = (int)mSprites[index]->mY;
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, mFontSize, mFontSize, GL_RGBA, GL_UNSIGNED_BYTE, mCharBuffer);
#endif
    return index;
}

void WFBFont::DrawString(const char *s, float x, float y, int align, float leftOffset, float width)
{
    u8 * str = (u8 *) s;
    if (*str < 0x80)
    {
        // tricky:  the single byte font is always mFontID + kSingleByteFontOffset!
        // See WResourceManager::InitFonts()
        WFont * mFont = WResourceManager::Instance()->GetWFont(mFontID + Fonts::kSingleByteFontOffset);
        mFont->SetScale(GetScale());
        mFont->SetColor(GetColor());
        mFont->DrawString(s, x, y, align, leftOffset, width);
        return;
    }

    // (0, 0) refers to the center of the word, so fix it to the upper-left corner
    x += (mFontSize * mScale) / 2;
    y += (mFontSize * mScale) / 2;
    // Warning : non-left alignment is not supported for multiline strings
    // (this is because GetStringWidth is not aware of line breaks).
    switch (align)
    {
    case JGETEXT_RIGHT:
        if (width) {
            x -= width;
            leftOffset += GetStringWidth(s) - width;
        }
        else
            x -= GetStringWidth(s);
        break;
    case JGETEXT_CENTER:
        if (width) {
            x -= width/2;
            leftOffset += GetStringWidth(s)/2 - width/2;
        }
        else
            x -= GetStringWidth(s)/2;
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

    while (*src != 0)
    {
        if (yy > SCREEN_HEIGHT_F) // don't render or count outside the buttom of viewport
            return;
        else if (yy + mFontSize < 0.0f)
        { // don't render when outside the top of viewport, but counted
            if (*src < 0x20)
            { // control characters
                if (*src == 0x0a)
                { // NEWLINE
                    xx = x;
                    yy += (mFontSize * mScale);
                }
                src += 1;
            }
            else
            {
                xx += (mFontSize * mScale);

                if (xx >= width)
                {
                    xx = x;
                    yy += (mFontSize * mScale);
                }
            }
        }
        else
        {
            if (*src < 0x20)
            { // control characters
                if (*src == 0x0a)
                { // NEWLINE
                    xx = x;
                    yy += (mFontSize * mScale);
                }
                src += 1;
            }
            else
            {
                int mana = this->GetMana(src);
                bool doubleW = doubleWidthChar(src);
                index = PreCacheChar(src);
                src += charWidth(*src);

                // fix for leftoffset and width's setting
                float xPos, yPos, charW, charHeight;
                mSprites[index]->GetTextureRect(&xPos, &yPos, &charW, &charHeight);
                float xPos0 = xPos;
                float charW0 = charW;
                float delta = doubleW ? (charW * mScale) : (charW * mScale / 2);
                if (leftOffset)
                {
                    if (leftOffset < 0)
                    {
                        xx -= leftOffset;
                        leftOffset = 0;
                    }
                    else if (leftOffset - delta > 0)
                    {
                        leftOffset -= delta;
                        continue;
                    }
                    else
                    {
                        xPos += leftOffset / mScale;
                        delta -= leftOffset;
                        leftOffset = 0;
                        charW = delta / mScale;
                    }
                }
                if (width)
                {
                    if (xx > x + width) return;
                    if (xx + delta > x + width)
                    {
                        delta = x + width - xx;
                        charW = delta / mScale;
                    }
                }

                if (mana >= 0)
                {
                    int mana2 = -1;
                    if (*src == '/' && (mana2 = this->GetMana(src + 1)) >= 0)
                    { // hybrid mana cost
                        src += 1 + charWidth(*src);
                        unsigned char t = (JGE::GetInstance()->GetTime() / 3) & 0xFF;
                        unsigned char v = t + 127;
                        float scale = 0.05f * cosf(2 * M_PI * ((float) t) / 256.0f);
                        if (scale < 0)
                        {
                            mRenderer->RenderQuad(manaIcons[mana].get(), xx + 3 * sinf(2 * M_PI * ((float) t) / 256.0f), yy + 3 * cosf(2
                                            * M_PI * ((float) (t - 35)) / 256.0f), 0, 0.5f * mScale, 0.5f * mScale);
                            mRenderer->RenderQuad(manaIcons[mana2].get(), xx + 3 * sinf(2 * M_PI * ((float) v) / 256.0f), yy + 3 * cosf(2
                                            * M_PI * ((float) (v - 35)) / 256.0f), 0, 0.5f * mScale, 0.5f * mScale);
                        }
                        else
                        {
                            mRenderer->RenderQuad(manaIcons[mana2].get(), xx + 3 * sinf(2 * M_PI * ((float) v) / 256.0f), yy + 3 * cosf(2
                                            * M_PI * ((float) (v - 35)) / 256.0f), 0, 0.5f * mScale, 0.5f * mScale);
                            mRenderer->RenderQuad(manaIcons[mana].get(), xx + 3 * sinf(2 * M_PI * ((float) t) / 256.0f), yy + 3 * cosf(2
                                            * M_PI * ((float) (t - 35)) / 256.0f), 0, 0.5f * mScale, 0.5f * mScale);
                        }
                        mana = Constants::NB_Colors + 1; // do not draw colorless cost in hybrid mana cost
                    }
                    else
                        mRenderer->RenderQuad(manaIcons[mana].get(), xx, yy, 0, 0.5f * mScale, 0.5f * mScale);
                    mRenderer->BindTexture(mTexture); // manaIcons use different texture, so we need to rebind it.
                }

                if (mana <= 0)
                {
                    mSprites[index]->SetTextureRect(xPos, yPos, charW, charHeight);
                    mSprites[index]->SetColor(mColor);
                    mRenderer->RenderQuad(mSprites[index], xx, yy, 0, mScale, mScale);
                    mSprites[index]->SetTextureRect(xPos0, yPos, charW0, charHeight);
                }

                xx += delta;

                if (xx >= 480)
                {
                    xx = x;
                    yy += (mFontSize * mScale);
                }
            }
        }
    }
}

void WFBFont::DrawString(std::string s, float x, float y, int align, float leftOffset, float width)
{
    DrawString(s.c_str(), x, y, align, leftOffset, width);
}

void WFBFont::SetColor(PIXEL_TYPE color)
{
    mColor0 = color;
    mColor = gencolor(mFontID, color);
}

float WFBFont::GetStringWidth(const char *s) const
{
    u8 * src = (u8 *) s;
    int width = 0;

    if (doubleWidthChar(src))
    {
        while (*src != 0)
        {
            // Add the number of single-char widths according to whether
            // this is a single or double-width char.
            width += (doubleWidthChar(src)) ? 2 : 1;
            src += charWidth(*src);
        }
        return width * mFontSize * mScale / 2;
    }
    else
    {
        WFont * mFont = WResourceManager::Instance()->GetWFont(mFontID + Fonts::kSingleByteFontOffset);
        mFont->SetScale(GetScale());
        return mFont->GetStringWidth(s);
    }

}

void WFBFont::SetScale(float scale)
{
    mScale = scale;
}

float WFBFont::GetScale() const
{
    return mScale;
}
float WFBFont::GetHeight() const
{
    return (mFontSize * mScale);
}

// Legacy : GBK encoding
WGBKFont::WGBKFont(int inFontID, const char *fontname, int lineheight, bool useVideoRAM) :
    WFBFont(inFontID)
{
    mRenderer = JRenderer::GetInstance();

    mCharBuffer = NULL;
    mSprites = NULL;
    mGBCode = NULL;
    mCurr = 0;

    char tmpFileName[32], engFileName[32];
    strcpy(tmpFileName, fontname);
    char * ep = strrchr(tmpFileName, '.');
    *ep = '\0';
    sprintf(engFileName, "%s.asc", tmpFileName);
    JFileSystem *fileSys = JFileSystem::GetInstance();
    int size = 0;

    if (!fileSys->OpenFile(fontname)) return;
    size = fileSys->GetFileSize();
    mExtraFont = NEW u8[size];
    fileSys->ReadFile(mExtraFont, size);
    fileSys->CloseFile();

    if (!fileSys->OpenFile(engFileName)) return;
    size = fileSys->GetFileSize();
    mStdFont = NEW u8[size];
    fileSys->ReadFile(mStdFont, size);
    fileSys->CloseFile();

    mIndex = 0;

    mColor0 = ARGB(255, 255, 255, 255);
    mColor = mColor0;
    mFontSize = lineheight;
    mScale = 1.0f;

    // using 4-bit(half-byte) to store 1 pixel
    mBytesPerRow = static_cast<unsigned int> (mFontSize / 2);
    mBytesPerChar = static_cast<unsigned int> (mBytesPerRow * mFontSize);

    mCacheImageWidth = 256;
    mCacheImageHeight = 256;
    mCol = mCacheImageWidth / mFontSize;
    mRow = mCacheImageHeight / mFontSize;
    mCacheSize = mCol * mRow;

    mSprites = NEW JQuad*[mCacheSize];
    mGBCode = NEW int[mCacheSize];

#if !defined (PSP)
    mCharBuffer = NEW u32[mFontSize*mFontSize];
#endif

    mTexture = mRenderer->CreateTexture(mCacheImageWidth, mCacheImageHeight, true);

    int index = 0;
    for (int y = 0; y < mRow; y++)
    {
        for (int x = 0; x < mCol; x++)
        {
            mGBCode[index] = -1;
            mSprites[index] = NEW JQuad(mTexture, static_cast<float> (x * mFontSize), static_cast<float> (y * mFontSize),
                            static_cast<float> (mFontSize), static_cast<float> (mFontSize));
            mSprites[index]->SetHotSpot(static_cast<float> (mFontSize / 2), static_cast<float> (mFontSize / 2));
            index++;
        }
    }
}

int WGBKFont::PreCacheChar(const u8 *ch)
{
    int code;
    int charLength = 1;
    u8 * src;
    unsigned int size, offset;
    u8 gray;

    code = this->GetCode(ch, &charLength);

    if (mGBCode[mCurr] != -1) for (int i = 0; i < mCacheSize; i++)
        if (mGBCode[i] == code) return i;

    int index = mCurr++;
    if (mCurr >= mCacheSize) mCurr = 0;

#if defined(PSP)
    u8* pTexture = (u8*) mTexture->mBits;
    int x;
    int y = (int) mSprites[index]->mY;
#else
    int x = 0;
    int y = 0;
    memset(mCharBuffer, 0, sizeof(u32) * mFontSize * mFontSize);
#endif

    if (mIndex) code = mIndex[code];

    if (GBKDoubleWidthChar(ch))
    {
        size = mFontSize;
        src = mExtraFont + code * mBytesPerChar;
        offset = 0;
    }
    else
    {
        size = mFontSize / 2;
        src = mStdFont + code * (mFontSize * size / 2);
        offset = 0;
    }

    // set up the font texture buffer
    for (unsigned int i = 0; i < mFontSize; i++)
    {
#if defined (PSP)
        x = (int) mSprites[index]->mX;
#else
        x = 0;
#endif
        unsigned int j = 0;
#if 1
        for (; j < offset; j++)
        {
#if defined (PSP)
            SwizzlePlot(pTexture, ARGB(0, 0, 0, 0), x * PIXEL_SIZE, y, mTexture->mTexWidth * PIXEL_SIZE);
#else
            mCharBuffer[y * mFontSize + x] = ARGB(0, 0, 0, 0);
#endif
            x++;
        }
#endif
        for (; j < offset + size; j++)
        {
            // as 4-bit(half-byte) stores 1 pixel
            // get out the proper data according to the even or odd quality of the counter
            gray = src[(i * size + j - offset) / 2];
            gray = ((j - offset) & 1) ? (gray & 0xF0) : ((gray & 0x0F) << 4);
            if (gray) gray |= 0x0F;
#if defined (PSP)
            SwizzlePlot(pTexture, ARGB(gray, 255, 255, 255), x * PIXEL_SIZE, y, mTexture->mTexWidth * PIXEL_SIZE);
#else
            mCharBuffer[y * mFontSize + x] = ARGB(gray, 255, 255, 255);
#endif
            x++;
        }
        for (; j < mFontSize; j++)
        {
#if defined (PSP)
            SwizzlePlot(pTexture, ARGB(0, 0, 0, 0), x * PIXEL_SIZE, y, mTexture->mTexWidth * PIXEL_SIZE);
#else
            mCharBuffer[y * mFontSize + x] = ARGB(0, 0, 0, 0);
#endif
            x++;
        }
        y++;
    }

    mGBCode[index] = code;

#if defined (PSP)
    sceKernelDcacheWritebackAll();
#else
    x = (int)mSprites[index]->mX;
    y = (int)mSprites[index]->mY;
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, mFontSize, mFontSize, GL_RGBA, GL_UNSIGNED_BYTE, mCharBuffer);
#endif
    return index;
}

void WGBKFont::DrawString(const char *s, float x, float y, int align, float leftOffset, float width)
{
    unsigned char c = *(unsigned short *) s & 0xFF;
    if (ISGBK(c) || (s[1] == ':' && s[2] == ' '))
    {
    }
    else
    {
        // tricky:  the single byte font is always mFontID + kSingleByteFontOffset!
        // See WResourceManager::InitFonts()
        WFont * mFont = WResourceManager::Instance()->GetWFont(mFontID + Fonts::kSingleByteFontOffset);
        mFont->SetScale(GetScale());
        mFont->SetColor(GetColor());
        mFont->DrawString(s, x, y, align, leftOffset, width);
        return;
    }

    u8 * str = (u8 *) s;

    // (0, 0) refers to the center of the word, so fix it to the upper-left corner
    x += (mFontSize * mScale) / 2;
    y += (mFontSize * mScale) / 2;
    switch (align)
    {
    case JGETEXT_RIGHT:
        if (width) {
            x -= width;
            leftOffset += GetStringWidth(s) - width;
        }
        else
            x -= GetStringWidth(s);
        break;
    case JGETEXT_CENTER:
        if (width) {
            x -= width/2;
            leftOffset += GetStringWidth(s)/2 - width/2;
        }
        else
            x -= GetStringWidth(s)/2;
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

    bool dualByteFont = true;

    while (*src != 0)
    {
        if (yy > SCREEN_HEIGHT_F) // don't render or count outside the buttom of viewport
            return;
        else if (yy + mFontSize < 0.0f)
        { // don't render when outside the top of viewport, but counted
            if (*src < 0x20)
            { // control characters
                if (*src == 0x0a)
                { // NEWLINE
                    xx = x;
                    yy += (mFontSize * mScale);
                }
                src += 1;
            }
            else
            {
                if (*src > 0x80) // 2-bytes char
                    src += 2;
                else
                    src += 1;

                xx += (mFontSize * mScale);

                if (xx >= 480)
                {
                    xx = x;
                    yy += (mFontSize * mScale);
                }
            }
        }
        else
        {
            if (*src < 0x20)
            { // control characters
                if (*src == 0x0a)
                { // NEWLINE
                    xx = x;
                    yy += (mFontSize * mScale);
                }
                src += 1;
            }
            else
            {
                int mana = -1;
                if (*src > 0x80)
                { // 2-bytes char
                    mana = this->GetMana(src);
                    index = PreCacheChar(src);
                    src += 2;
                    dualByteFont = true;
                }
                else
                {
                    index = PreCacheChar(src);
                    src += 1;
                    dualByteFont = false;
                }

                // fix for leftoffset and witdth's setting
                float xPos, yPos, charW, charHeight;
                mSprites[index]->GetTextureRect(&xPos, &yPos, &charW, &charHeight);
                float xPos0 = xPos;
                float charW0 = charW;
                float delta = (dualByteFont) ? (charW * mScale) : (charW * mScale / 2);
                if (leftOffset)
                {
                    if (leftOffset < 0)
                    {
                        xx -= leftOffset;
                        leftOffset = 0;
                    }
                    else if (leftOffset - delta > 0)
                    {
                        leftOffset -= delta;
                        continue;
                    }
                    else
                    {
                        xPos += leftOffset / mScale;
                        delta -= leftOffset;
                        leftOffset = 0;
                        charW = delta / mScale;
                    }
                }
                if (width)
                {
                    if (xx > x + width) return;
                    if (xx + delta > x + width)
                    {
                        delta = x + width - xx;
                        charW = delta / mScale;
                    }
                }

                if (mana >= 0)
                {
                    int mana2 = -1;
                    if (*src == '/' && (mana2 = this->GetMana(src + 1)) >= 0)
                    { // hybrid mana cost
                        src += 3;
                        unsigned char t = (JGE::GetInstance()->GetTime() / 3) & 0xFF;
                        unsigned char v = t + 127;
                        float scale = 0.05f * cosf(2 * M_PI * ((float) t) / 256.0f);
                        if (scale < 0)
                        {
                            mRenderer->RenderQuad(manaIcons[mana].get(), xx + 3 * sinf(2 * M_PI * ((float) t) / 256.0f), yy + 3 * cosf(2
                                            * M_PI * ((float) (t - 35)) / 256.0f), 0, 0.5f * mScale, 0.5f * mScale);
                            mRenderer->RenderQuad(manaIcons[mana2].get(), xx + 3 * sinf(2 * M_PI * ((float) v) / 256.0f), yy + 3 * cosf(2
                                            * M_PI * ((float) (v - 35)) / 256.0f), 0, 0.5f * mScale, 0.5f * mScale);
                        }
                        else
                        {
                            mRenderer->RenderQuad(manaIcons[mana2].get(), xx + 3 * sinf(2 * M_PI * ((float) v) / 256.0f), yy + 3 * cosf(2
                                            * M_PI * ((float) (v - 35)) / 256.0f), 0, 0.5f * mScale, 0.5f * mScale);
                            mRenderer->RenderQuad(manaIcons[mana].get(), xx + 3 * sinf(2 * M_PI * ((float) t) / 256.0f), yy + 3 * cosf(2
                                            * M_PI * ((float) (t - 35)) / 256.0f), 0, 0.5f * mScale, 0.5f * mScale);
                        }
                        mana = Constants::NB_Colors + 1; // donot draw colorless cost in hybrid mana cost
                    }
                    else
                        mRenderer->RenderQuad(manaIcons[mana].get(), xx, yy, 0, 0.5f * mScale, 0.5f * mScale);
                    mRenderer->BindTexture(mTexture); // manaIcons use different texture, so we need to rebind it.
                }

                if (mana <= 0)
                {
                    mSprites[index]->SetTextureRect(xPos, yPos, charW, charHeight);
                    mSprites[index]->SetColor(mColor);
                    mRenderer->RenderQuad(mSprites[index], xx, yy, 0, mScale, mScale);
                    mSprites[index]->SetTextureRect(xPos0, yPos, charW0, charHeight);
                }

                xx += delta;

                if (xx >= 480)
                {
                    xx = x;
                    yy += (mFontSize * mScale);
                }
            }
        }
    }
}

float WGBKFont::GetStringWidth(const char *s) const
{
    unsigned char c = *(unsigned short *) s & 0xFF;

    if (ISGBK(c))
    {
        u8 * src = (u8 *) s;
        float xx = 0;
        bool dualByteFont = true;

        while (*src != 0)
        {
            if (*src > 0x80)
            { // Chinese and Japanese
                src += 2;
                dualByteFont = true;
            }
            else
            { // Latin 1
                src += 1;
                dualByteFont = false;
            }
            if (dualByteFont)
                xx += (mFontSize * mScale);
            else
                xx += (mFontSize * mScale) / 2;
        }
        return xx;
    }
    else
    {
        WFont * mFont = WResourceManager::Instance()->GetWFont(mFontID + Fonts::kSingleByteFontOffset);
        mFont->SetScale(GetScale());
        return mFont->GetStringWidth(s);
    }
}

int WGBKFont::GetCode(const u8 *ch, int *charLength) const
{
    int code = 0;
    *charLength = 2;

    if (*ch > 0xA0 && *(ch + 1) > 0xA0)
    {
        // get offset to the proper character bits (GB2312 encoding)
        code = (((u32)(*ch - 0xA1)) * 0x5E + ((u32)(*(ch + 1) - 0xA1)));
    }
    else if (*ch > 0x80)
    {
        // get offset to the character space's bits (GBK encoding)
        code = 0;
    }
    else
    {
        code = ((u32) * ch);
        *charLength = 1;
    }
    return code;
}

int WGBKFont::GetMana(const u8 *ch) const
{
    int mana = -1;

    if (*ch != 0xa3) return mana;
    switch (*(ch + 1))
    {
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
        if (*(ch + 1) >= 0xB0 && *(ch + 1) <= 0xB9) mana = Constants::MTG_UNCOLORED;
    }
    return mana;
}

void WGBKFont::FormatText(string &s, vector<string>& output)
{
    while (s.length() > 0)
    {
        std::string::size_type len = 24;
        std::string::size_type cut = s.find_first_of("., \t)", 0);
        if (cut >= len || cut == string::npos)
        {
            // Fix for single byte character in some complex language like Chinese
            u8 * src = (u8 *) s.c_str();
            //if (neofont)
            {
                len = 0;
                std::string::size_type limit = 24;
                while (*src != 0)
                {
                    if (*src > 0x80)
                    { // Non-ASCII
                        if (len + 2 > limit && !(((*src & 0xF0) == 0xA0) && ((*(src + 1) & 0xF0) == 0xA0)))
                            break;
                        src += 2;
                        len += 2;
                    }
                    else
                    { // ASCII
                        if (*src == '/' && (*(src + 1) & 0xF0) == 0xA0)
                            limit += 3;
                        if (len + 1 > limit && (*src == '+' || *src == '-' || *src == '/'))
                            break;
                        src += 1;
                        len += 1;
                    }
                }
            }
            output.push_back(s.substr(0, len));
            if (s.length() > len)
                s = s.substr(len, s.length() - len);
            else
                s = "";
        }
        else
        {
            std::string::size_type newcut = cut;
            while (newcut < len && newcut != string::npos)
            {
                // neofont use space to separate one line
                u8 * src = (u8 *) s.c_str();
                //if (neofont && *src > 0x80)
                if (*src > 0x80)
                    break;
                cut = newcut;
                newcut = s.find_first_of("., \t)", newcut + 1);
            }
            output.push_back(s.substr(0, cut + 1));
            if (s.length() > cut + 1)
                s = s.substr(cut + 1, s.length() - cut - 1);
            else
                s = "";
        }
    }
}

int WUFont::GetCode(const u8 *ch, int *charLength) const
{
    int code = 0;

    // This assumes the string is valid. We could test for
    // validity by ensuring all trailing bytes are & 0xC0 == F0
    // and that no first byte is.

    // For a description of the binary representation, look at wikipedia://utf-8

    if ((*ch & 0xF8) == 0xF0)
    { // Four bytes
        *charLength = 4;
        code = ((*ch & 0x7) << 18) + ((*(ch + 1) & 0x3F) << 12) + ((*(ch + 2) & 0x3F) << 6) + ((*(ch + 3) * 0x3F));
    }
    else if ((*ch & 0xF0) == 0xE0)
    { // Three bytes
        *charLength = 3;
        code = ((*ch & 0xF) << 12) + ((*(ch + 1) & 0x3F) << 6) + ((*(ch + 2) & 0x3F));
    }
    else if ((*ch & 0xE0) == 0xC0)
    { // Two bytes
        *charLength = 2;
        code = ((*ch & 0x1F) << 6) + ((*(ch + 1) & 0x3F));
    }
    else
    {
        *charLength = 1;
        code = *ch;
    }
    return code;
}

int WUFont::GetMana(const u8 *ch) const
{
    /*
     * Reactivate the following code to be able to use
     * single-width characters too, if needed.
     **/
    /*
     switch (*ch)
     {
     case 0x67: return Constants::MTG_COLOR_GREEN;
     case 0x75: return Constants::MTG_COLOR_BLUE;
     case 0x72: return Constants::MTG_COLOR_RED;
     case 0x62: return Constants::MTG_COLOR_BLACK;
     case 0x77: return Constants::MTG_COLOR_WHITE;
     case 0x74:
     case 0x78:
     case 0x79: return Constants::MTG_UNCOLORED;
     default:
     if (*ch >= 0x30 && *ch <= 0x39)
     return Constants::MTG_UNCOLORED;
     }
     */
    if (*ch != 0xef || *(ch + 1) != 0xbc) return -1;
    ch += 2;
    switch (*ch)
    {
    case 0xa7: // G: 0xefbca7
        return Constants::MTG_COLOR_GREEN;
    case 0xb5: // U: 0xefbcb5
        return Constants::MTG_COLOR_BLUE;
    case 0xb2: // R: 0xefbcb2
        return Constants::MTG_COLOR_RED;
    case 0xa2: // B: 0xefbca2
        return Constants::MTG_COLOR_BLACK;
    case 0xb7: // W: 0xefbcb7
        return Constants::MTG_COLOR_WHITE;
    case 0xb4: // T: 0xefbcb4
    case 0xb8: // X: 0xefbcb8
    case 0xb9: // Y: 0xefbcb9
        return Constants::MTG_UNCOLORED;
    default:
        if (*ch >= 0x90 && *ch <= 0x99) return Constants::MTG_UNCOLORED;
    }
    return -1;
}

void WUFont::FormatText(string &s, vector<string>& output)
{
    std::string::size_type limit = 22; //28
    string delim("., \t)");

    while (s.length() > 0)
    {
        u8 * src = (u8 *) s.c_str();
        std::string::size_type len = 0;
        std::string::size_type ctr = 0;
        std::string::size_type lastcut = 0;
        u8 ch = 0;
        while ((ch = src[len]) != 0)
        {
            ctr += 2;
            if      ((ch & 0xF8) == 0xF0) len += 4;
            else if ((ch & 0xF0) == 0xE0) len += 3;
            else if ((ch & 0xE0) == 0xC0) len += 2;
            else // ASCII
            {
                ctr--; len += 1;
                if (delim.find(ch) != string::npos) lastcut = len;
                if (ctr > limit && lastcut) {len = lastcut; break;}
                //if (ch == ' ') break; // if we use ' ' to break a line.
            }
            if (ctr > limit) break;
        }
        output.push_back(s.substr(0, len));
        if (s.length() > len)
            s = s.substr(len, s.length() - len);
        else
            s = "";
    }
}
