//
//  SimpleButton.h
//  - base class for creating buttons/links inside of the game engine.  
//
//  Created by Michael Nguyen on 1/21/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef wagic_SimpleButton_h
#define wagic_SimpleButton_h

#include <string>
#include <JLBFont.h>
#include <JGui.h>

using std::string;

#define SCALE_SELECTED		1.2f
#define SCALE_NORMAL		1.0f
#define SCALE_SHRINK        0.75f

class SimpleButton: public JGuiObject
{
private:
    
    float mScale;
    float mTargetScale;
    bool mHasFocus;
    bool mIsValidSelection;
    float mX;
    float mY;
    
protected:
    float mYOffset;
    JGuiController* parent;
    int mFontId;
    string mText;
    float mXOffset;
    
public:

    SimpleButton(int id);
    SimpleButton(JGuiController* _parent, int id, int fontId, string text, float x, float y, bool hasFocus = false, bool autoTranslate = false);

    virtual float getScale() const;
    virtual float getTargetScale() const;
    virtual JGuiController* getParent() const;
    
    virtual int getFontId() const;
    virtual void setFontId( const int& fontId );
    virtual void setX( const float& x ) { mX = x; };
    virtual void setY( const float& y ) { mY = y; };
    
    virtual void setIsSelectionValid( bool validSelection );    
    virtual void setFocus(bool value);    
    virtual void setText( const string& text);
    
    virtual bool isSelectionValid() const;
    virtual bool hasFocus() const;
    virtual string getText() const;
    float getX() const;
    float getY() const;
    virtual void checkUserClick();
    
    virtual float GetWidth();
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
