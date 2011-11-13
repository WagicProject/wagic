#ifndef _ICONBUTTON_H
#define _ICONBUTTON_H

#include <string>
#include <JGui.h>

using std::string;

class IconButtonsController: public JGuiController, public JGuiListener
{
public:
    float mX;
    float mY;
    IconButtonsController(JGE* jge, float x, float y);
    void SetColor(PIXEL_TYPE color);
};

class IconButton: public JGuiObject
{
private:
    PIXEL_TYPE mColor;
    IconButtonsController * mParent;
    bool mHasFocus;
    int mFontId;
    string mText;
    float mScale, mCurrentScale, mTargetScale;
    float mX, mY;
    float mTextRelativeX, mTextRelativeY;
    JTexture * mTex;
    JQuad * mQuad;

public:
    IconButton(int id, IconButtonsController * parent, string texture, float x, float y, float scale, int fontId, string text, float textRelativeX, float textRelativeY, bool hasFocus = false);
    IconButton(int id, IconButtonsController * parent, JQuad * quad, float x, float y, float scale, int fontId, string text, float textRelativeX, float textRelativeY, bool hasFocus = false);
    void init(IconButtonsController * parent, JQuad * quad, float x, float y, float scale, int fontId, string text, float textRelativeX, float textRelativeY, bool hasFocus);
    ~IconButton();
    ostream& toString(ostream& out) const;

    bool hasFocus();

    virtual void Render();
    virtual void Update(float dt);

    virtual void Entering();
    virtual bool Leaving(JButton key);
    virtual bool ButtonPressed();
    void SetColor(PIXEL_TYPE color);
};

#endif
