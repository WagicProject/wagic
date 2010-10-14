#ifndef WFONT_H_
#define WFONT_H_

#include <JLBFont.h>
#include <JRenderer.h>
#include <JSprite.h>
#include "config.h"

class WFont
{
public:
  unsigned char id;
  // Rendering text to screen.
  virtual void DrawString(const char *s, float x, float y, int align=JGETEXT_LEFT, float leftOffset = 0, float width = 0) = 0;
  virtual void DrawString(std::string s, float x, float y, int align=JGETEXT_LEFT, float leftOffset = 0, float width = 0) = 0;
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
  virtual float	GetStringWidth(const char *s) const = 0;
  // Set font tracking.
  virtual void SetTracking(float tracking) = 0;
  // Set Base for the character set to use.
  virtual void SetBase(int base) = 0;
  virtual  ~WFont() {};
};

class WLBFont : public WFont
{
public:
  WLBFont(const char *fontname, int lineheight, bool useVideoRAM=false) {
    it = NEW JLBFont(fontname,lineheight,useVideoRAM);
  };
  ~WLBFont() {SAFE_DELETE(it);};

  void DrawString(const char *s, float x, float y, int align=JGETEXT_LEFT, float leftOffset = 0, float width = 0) {it->DrawString(s,x,y,align,leftOffset,width);};
  void DrawString(std::string s, float x, float y, int align=JGETEXT_LEFT, float leftOffset = 0, float width = 0) {it->DrawString(s,x,y,align,leftOffset,width);};
  void SetColor(PIXEL_TYPE color) {it->SetColor(color);};
  PIXEL_TYPE GetColor() const {return it->GetColor();};
  void SetScale(float scale) {it->SetScale(scale);};
  float GetScale() const {return it->GetScale();};
  float GetHeight() const {return it->GetHeight();};
  float	GetStringWidth(const char *s) const {return it->GetStringWidth(s);};
  void SetTracking(float tracking) {it->SetTracking(tracking);};
  void SetBase(int base) {it->SetBase(base);};

private:
  JLBFont * it;
};

class WFBFont : public WFont
{
public:
  WFBFont(const char *fontname, int lineheight, bool useVideoRAM=false);
  ~WFBFont();

  void DrawString(const char *s, float x, float y, int align=JGETEXT_LEFT, float leftOffset = 0, float width = 0);
  void DrawString(std::string s, float x, float y, int align=JGETEXT_LEFT, float leftOffset = 0, float width = 0);
  void SetColor(PIXEL_TYPE color);
  PIXEL_TYPE GetColor() const {return mColor0;};
  void SetScale(float scale);
  float GetScale() const;
  float GetHeight() const;
  float	GetStringWidth(const char *s) const;
  void SetTracking(float tracking) {};
  void SetBase(int base) {};

private:
  static JRenderer * mRenderer;

  u8 * mEngFont;
  u8 * mChnFont;

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

  u32 * mCharBuffer;

  int PreCacheChar(const u8 *ch);
};

#endif
