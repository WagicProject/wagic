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
    
    DeckMenu* deckController;
    float mTitleResetWidth;
    
protected:
    virtual void checkUserClick();

public:
    DeckMenuItem(DeckMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus = false, bool autoTranslate = false, DeckMetaData *meta = NULL);
    ~DeckMenuItem();

    string imageFilename;
    float mScrollerOffset;
    DeckMetaData *meta;
    
    virtual void Relocate(float x, float y);
    virtual float GetWidth();
    virtual void Render();
    virtual void Update(float dt);
    
    virtual bool getTopLeft(float& top, float& left)
    {
        return SimpleMenuItem::getTopLeft(top, left);
    }
  
    virtual void Entering();
    virtual bool Leaving(JButton key);
    virtual bool ButtonPressed();
    virtual ostream& toString(ostream& out) const;
    virtual JGuiController* getParent() const;
    virtual void RenderWithOffset(float yOffset);

};

#endif
