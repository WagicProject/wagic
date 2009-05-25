#ifndef _SIMPLEMENU_ITEM_H
#define _SIMPLEMENU_ITEM_H

#include <string>
#include <JLBFont.h>
#include <JGui.h>
#include <string>
#include "SimpleMenu.h"

using std::string;

#define SCALE_SELECTED		1.2f
#define SCALE_NORMAL		1.0f


class SimpleMenuItem: public JGuiObject
{
 private:
  bool mHasFocus;
  SimpleMenu* parent;
  JLBFont *mFont;
  string mText;
  float mScale;
  float mTargetScale;

 public:
  string desc;
  SimpleMenuItem(SimpleMenu* _parent, int id, JLBFont *font, string text, int x, int y, bool hasFocus = false);

  int mX;
  int mY;

  void Relocate(int x, int y);
  int GetWidth();
  bool hasFocus();

  void RenderWithOffset(float yOffset);
  virtual void Render();
  virtual void Update(float dt);

  virtual void Entering();
  virtual bool Leaving(u32 key);
  virtual bool ButtonPressed();
  virtual ostream& toString(ostream& out) const;
};

#endif
