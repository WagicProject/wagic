#ifndef _SIMPLEMENU_ITEM_H
#define _SIMPLEMENU_ITEM_H

#include <string>
#include <JLBFont.h>
#include <JGui.h>
#include "SimpleMenu.h"

using std::string;

#define SCALE_SELECTED		1.2f
#define SCALE_NORMAL		1.0f


class SimpleMenuItem: public JGuiObject
{
 private:
  bool mHasFocus;
  SimpleMenu* parent;
  int fontId;
  string mText;
  float mScale;
  float mTargetScale;

 public:
  string desc;
  SimpleMenuItem(SimpleMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus = false, bool autoTranslate = false);

  float mX;
  float mY;

  void Relocate(float x, float y);
  float GetWidth();
  bool hasFocus();

  void RenderWithOffset(float yOffset);
  virtual void Render();
  virtual void Update(float dt);

  virtual void Entering();
  virtual bool Leaving(JButton key);
  virtual bool ButtonPressed();
  virtual ostream& toString(ostream& out) const;
  virtual bool getTopLeft(int& top, int& left) {top = mY; left = mX; return true;};
};

#endif
