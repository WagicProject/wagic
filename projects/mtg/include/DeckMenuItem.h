#ifndef _DECKMENU_ITEM_H
#define _DECKMENU_ITEM_H

#include <string>
#include <JLBFont.h>
#include <JGui.h>
#include "DeckMenu.h"

using std::string;

class DeckMenuItem: public JGuiObject
{
 private:
  bool mHasFocus;
  DeckMenu* parent;
  int fontId;
  string mText;

 public:
  string imageFilename;
  string desc;
  DeckMetaData *meta;

  float mX;
  float mY;

  void Relocate(float x, float y);
  float GetWidth();
  bool hasFocus();

  DeckMenuItem(DeckMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus = false, bool autoTranslate = false, DeckMetaData *meta = NULL);
  ~DeckMenuItem();

  void RenderWithOffset(float yOffset);
  virtual void Render();

  virtual void Entering();
  virtual bool Leaving(JButton key);
  virtual bool ButtonPressed();
  virtual ostream& toString(ostream& out) const;
  virtual bool getTopLeft(float& top, float& left) {top = mY; left = mX; return true;};
};

#endif
