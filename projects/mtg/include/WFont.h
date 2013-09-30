#ifndef WFONT_H_
#define WFONT_H_

#include <JLBFont.h>
#include <JRenderer.h>
#include <JSprite.h>
#include "config.h"

namespace Fonts
{
    enum Font_Type
    {
        MAIN_FONT = 0,
        MENU_FONT = 1,
        OPTION_FONT = 1,
        MAGIC_FONT = 2,
        SMALLFACE_FONT = 3
    };

    // when using gbk languages and we need to keep around single byte font variants,
    // the single byte fonts will be offset by this value
    const unsigned int kSingleByteFontOffset = 100;
}

class WFont
{
public:
    int mFontID;
    // Rendering text to screen.
    // Note:
    // align=JGETEXT_LEFT,   string region (x-leftOffset, x-leftOffset+StringWidth), display window (x, x+displayWidth)
    // align=JGETEXT_CENTER, string region (x-leftOffset-StringWidth/2, x-leftOffset+StringWidth/2), display window (x-displayWidth/2, x+displayWidth/2)
    // align=JGETEXT_RIGHT,  string region (x-leftOffset-StringWidth, x-leftOffset), display window (x-displayWidth, x)
    // Only when width is NOT zero, characters outside the display window are not rendered.
    //
    virtual void DrawString(const char *s, float x, float y, int align = JGETEXT_LEFT, float leftOffset = 0, float displayWidth = 0) = 0;
    virtual void DrawString(std::string s, float x, float y, int align = JGETEXT_LEFT, float leftOffset = 0, float displayWidth = 0) = 0;
    // Set font color.
    virtual void SetColor(PIXEL_TYPE color) = 0;
    // Get font color.
    virtual PIXEL_TYPE GetColor() const = 0;
    // Set scale for rendering.
    virtual void SetScale(float scale) = 0;
    // Get rendering scale.
    virtual float GetScale() const = 0;
    // Get height of font.
    virtual float GetHeight() const = 0;
    // Get width of rendering string on screen.
    virtual float GetStringWidth(const char *s) const = 0;
    // Set font tracking.
    virtual void SetTracking(float tracking) = 0;
    // Set Base for the character set to use.
    virtual void SetBase(int base) = 0;
    // Format text.
    virtual void FormatText(string &s, vector<string>& output) = 0;
    WFont(int inID) : mFontID(inID) {};
    virtual ~WFont() {};
};

class WLBFont: public WFont
{
public:
    WLBFont(int inFontID, const char *fontname, int lineheight, bool useVideoRAM = false);
    ~WLBFont()
    {
        SAFE_DELETE(it);
    }
    ;

    void DrawString(const char *s, float x, float y, int align = JGETEXT_LEFT, float leftOffset = 0, float width = 0)
    {
        it->DrawString(s, x, y, align, leftOffset, width);
    }
    ;
    void DrawString(std::string s, float x, float y, int align = JGETEXT_LEFT, float leftOffset = 0, float width = 0)
    {
        it->DrawString(s, x, y, align, leftOffset, width);
    }
    ;
    void SetColor(PIXEL_TYPE color)
    {
        it->SetColor(color);
    }
    ;
    PIXEL_TYPE GetColor() const
    {
        return it->GetColor();
    }
    ;
    void SetScale(float scale)
    {
        it->SetScale(scale);
    }
    ;
    float GetScale() const
    {
        return it->GetScale();
    }
    ;
    float GetHeight() const
    {
        return it->GetHeight();
    }
    ;
    float GetStringWidth(const char *s) const
    {
        return it->GetStringWidth(s);
    }
    ;
    void SetTracking(float tracking)
    {
        it->SetTracking(tracking);
    }
    ;
    void SetBase(int base)
    {
        it->SetBase(base);
    }
    ;
    void FormatText(string &s, vector<string>& output);

private:
    JLBFont * it;
};

class WFBFont: public WFont
{
public:
    WFBFont(int inFontID, const char *fontname, int lineheight, bool useVideoRAM = false);
    WFBFont(int inFontID) : WFont(inFontID) {}; // Legacy : remove it when possible
    ~WFBFont();

    void DrawString(std::string s, float x, float y, int align = JGETEXT_LEFT, float leftOffset = 0, float width = 0);
    void SetColor(PIXEL_TYPE color);
    PIXEL_TYPE GetColor() const
    {
        return mColor0;
    }
    ;
    void SetScale(float scale);
    float GetScale() const;
    float GetHeight() const;
    virtual float GetStringWidth(const char *s) const;
    void SetTracking(float) {};
    void SetBase(int) {};
    void FormatText(string &, vector<string>&) {};

    virtual void DrawString(const char *s, float x, float y, int align = JGETEXT_LEFT, float leftOffset = 0, float width = 0);
    virtual int GetCode(const u8 *ch, int *charLength) const = 0;
    virtual int GetMana(const u8 *ch) const = 0;

protected:
    static JRenderer* mRenderer;

    u16* mIndex;
    u8* mStdFont;
    u8* mExtraFont;

    PIXEL_TYPE mColor0;
    PIXEL_TYPE mColor;
    unsigned int mFontSize;
    float mScale;
    unsigned int mBytesPerChar;
    unsigned int mBytesPerRow;

    int mCacheImageWidth;
    int mCacheImageHeight;
    int mCol;
    int mRow;
    int mCacheSize;
    JTexture * mTexture;
    JQuad ** mSprites;
    int *mGBCode;
    int mCurr;

	PIXEL_TYPE* mCharBuffer;
    virtual int PreCacheChar(const u8 *ch);
};

class WGBKFont: public WFBFont
{
public:
    WGBKFont(int inFontID, const char *fontname, int lineheight, bool useVideoRAM = false);

    int PreCacheChar(const u8 *ch);
    float GetStringWidth(const char *s) const;
    void DrawString(const char *s, float x, float y, int align = JGETEXT_LEFT, float leftOffset = 0, float width = 0);
    int GetCode(const u8 *ch, int *charLength) const;
    int GetMana(const u8 *ch) const;
    void FormatText(string &s, vector<string>& output);
};

class WUFont: public WFBFont
{
public:
    WUFont(int inFontID, const char *fontname, int lineheight, bool useVideoRAM = false) :
        WFBFont(inFontID, fontname, lineheight, useVideoRAM) {};

    int GetCode(const u8 *ch, int *charLength) const;
    int GetMana(const u8 *ch) const;
    void FormatText(string &s, vector<string>& output);
};

#endif
