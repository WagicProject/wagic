#ifndef _DECKMENU_ITEM_H
#define _DECKMENU_ITEM_H

#include <string>
#include <JLBFont.h>
#include <JGui.h>
#include "DeckMenu.h"
#include "SimpleMenuItem.h"

using std::string;

class DeckMenuItem: public SimpleMenuItem
{
private:
    bool mScrollEnabled;
    bool mDisplayInitialized;
    
    DeckMenu* parent;
    float mTitleResetWidth;

public:
    string imageFilename;
    string desc;
    float mScrollerOffset;
    DeckMetaData *meta;

    void Relocate(float x, float y);
    float GetWidth();
    
    string GetText()
    {
        return mText;
    }
    string GetDescription()
    {
        return desc;
    }
    bool hasFocus();

    DeckMenuItem(DeckMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus = false, bool autoTranslate = false, DeckMetaData *meta = NULL);
    ~DeckMenuItem();

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
