#ifndef _TEXTSCROLLER_H_
#define _TEXTSCROLLER_H_

class JLBFont;
#include <JGui.h>
#include <string>
#include <vector>
using namespace std;

class TextScroller: public JGuiObject
{
protected:
    string mText;
    string tempText;
    int fontId;
    float mWidth; // width of the text scroller object
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
    TextScroller(int fontId, float x, float y, float width, float speed = 30);
    void Add(string text);
    void Reset();
    void setRandom(int mode = 1);
    void Render();
    void Update(float dt);
    virtual ostream& toString(ostream& out) const;
};

class VerticalTextScroller: public TextScroller
{
private:
    size_t mNbItemsShown;
    float mHeight; // maximum height availble for display
    float mMarginX;
    float mMarginY; // margin used to allow text to scroll off screen without
    // affecting look and feel.  Should be enough
    // for at least one line of text ( mY - line height of current font )
    float mOriginalY; // mY initially, used to restore scroller to original position after update

public:
    VerticalTextScroller(int fontId, float x, float y, float width, float height, float scrollSpeed = 30, size_t _minimumItems = 1);
    void Render();
    void Update(float dt);
    void Add(string text);
};
#endif
