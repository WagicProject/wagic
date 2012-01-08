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
    
    virtual bool hasFocus();

    virtual void Relocate(float x, float y);
    virtual float GetWidth();
    virtual void Render();
    virtual void Update(float dt);
    
    virtual bool getTopLeft(float& top, float& left)
    {
        return SimpleMenuItem::getTopLeft(top, left);
    }
    
    string GetText()
    {
        return mText;
    }
    string GetDescription()
    {
        return desc;
    }

    DeckMenuItem(DeckMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus = false, bool autoTranslate = false, DeckMetaData *meta = NULL);
    ~DeckMenuItem();
    virtual void Entering();
    virtual bool Leaving(JButton key);
    virtual bool ButtonPressed();
    virtual ostream& toString(ostream& out) const;
    virtual void RenderWithOffset(float yOffset);

};

#endif
