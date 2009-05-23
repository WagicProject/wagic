#ifndef _TEXTSCROLLER_H_
#define _TEXTSCROLLER_H_

class JLBFont;
#include <JGui.h>
#include <string>
#include <vector>
using namespace std;

class TextScroller: public JGuiObject{
private:
  string mText;
  string tempText;
  JLBFont * mFont;
  float mWidth;
  float mSpeed;
  float mX;
  float mY;
  float start;
  int timer;
  vector<string> strings;
  unsigned int currentId;
  int mRandom;
public:
  void Add(string text);
  void Reset();
  void setRandom(int mode = 1);
  TextScroller(JLBFont * font, float x, float y, float width, float speed = 30);
  void Render();
  void Update(float dt);
  virtual ostream& toString(ostream& out) const;
};

#endif
