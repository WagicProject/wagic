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
    bool mHasFocus;
    bool mIsValidSelection;
    float mX;
    float mY;
    
protected:
    int mFontId;
    string mText;
    static float mYOffset;
    float mXOffset;
    string mDescription;

public:
    SimpleMenuItem(int id);
    SimpleMenuItem(SimpleMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus = false, bool autoTranslate = false);

    virtual int getFontId() const;
    virtual void setFontId( const int& fontId );
    virtual void setX( const float& x ) { mX = x; };
    virtual void setY( const float& y ) { mY = y; };
    
    virtual void setIsSelectionValid( bool validSelection );    
    virtual void setFocus(bool value);    
    virtual void setDescription( const string& desc );
    virtual void setText( const string& text);

    virtual bool isSelectionValid() const;
    virtual bool hasFocus() const;
    virtual string getDescription() const;
    virtual string getText() const;
    float getX() const;
    float getY() const;
    virtual void checkUserClick();
    
    virtual float GetWidth() const;
    virtual void Relocate(float x, float y);

    virtual void RenderWithOffset(float yOffset);
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
