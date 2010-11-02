#ifndef _TEXTSCROLLER_H_
#define _TEXTSCROLLER_H_

class JLBFont;
#include <JGui.h>
#include <string>
#include <vector>
using namespace std;

class TextScroller: public JGuiObject{
protected:
  string mText;
  string tempText;
  int fontId;
  float mWidth;
  float mSpeed;
  float mX;
  float mY;
  float start;
  int timer;
  vector<string> strings;
  unsigned int currentId;
  int mRandom;
  int scrollDirection;

public:
  void Add(string text);
  void Reset();
  void setRandom(int mode = 1);
  TextScroller(int fontId, float x, float y, float width, float speed = 30, int scrollerType = 0);
  void Render();
  void Update(float dt);
  virtual ostream& toString(ostream& out) const;
};

#endif
