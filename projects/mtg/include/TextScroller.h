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
  float mWidth;	// width of the text scroller object
  float mScrollSpeed;
  float mX;
  float mY;
  float start;
  int timer;
 
  vector<string> strings;
  unsigned int currentId;
  int mRandom;
  int scrollDirection;

public:
  TextScroller(int fontId, float x, float y, float width, float speed = 100);
  void Add(string text);
  void Reset();
  void setRandom(int mode = 1);
  void Render();
  void Update(float dt);
  virtual ostream& toString(ostream& out) const;
};

class VerticalTextScroller:
	public TextScroller
{
private:
	size_t mNbItemsShown;
	bool mScrollerInitialized;
	float mHeight; // maximum height availble for display
	int marginX;
	int marginY; // margin used to allow text to scroll off screen without
				// affecting look and feel.  Should be enough
				// for at least one line of text ( marginY)

protected:
	string wordWrap(string sentence, float width);

public:
	VerticalTextScroller(int fontId, float x, float y, float width, float height, float scrollSpeed = 30, size_t _minimumItems = 1);
	void Render();
	void Update(float dt);
	
};
#endif
