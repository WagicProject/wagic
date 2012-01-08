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

    SimpleMenu* parent;
    float mScale;
    float mTargetScale;

protected:
    int fontId;
    string mText;
    bool mHasFocus;
    static float mYOffset;
    float mXOffset;
    
    bool mIsValidSelection;    
    void checkUserClick();


public:
    string desc;
    SimpleMenuItem(int id);
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
    virtual bool getTopLeft(float& top, float& left)
    {
        top = mY + mYOffset;
        left = mX;
        return true;
    }
    ;
};

#endif
