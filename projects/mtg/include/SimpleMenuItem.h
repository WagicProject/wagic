#ifndef _SIMPLEMENU_ITEM_H
#define _SIMPLEMENU_ITEM_H

#include <JLBFont.h>
#include <JGui.h>

#define SCALE_SELECTED		1.2f
#define SCALE_NORMAL		1.0f


class SimpleMenuItem: public JGuiObject
{
 private:
  bool mHasFocus;
  JLBFont *mFont;
  string mText;
  int mX;
  int mY;

  float mScale;
  float mTargetScale;



 public:
  SimpleMenuItem(int id, JLBFont *font, const char* text, int x, int y, bool hasFocus = false);

  void RenderWithOffset(float yOffset);
  virtual void Render();
  virtual void Update(float dt);

  virtual void Entering();
  virtual bool Leaving(u32 key);
  virtual bool ButtonPressed();
};

#endif

