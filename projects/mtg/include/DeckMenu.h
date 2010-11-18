/*
 A class for menus with a fixed layout
 */
#ifndef _DeckMenu_H_
#define _DeckMenu_H_

#include <string>
#include "WFont.h"
#include "hge/hgeparticle.h"
#include "DeckMetaData.h"
#include "TextScroller.h"

class DeckMenu: public JGuiController
{
protected:

    float mHeight, mWidth, mX, mY;
    float titleX, titleY, titleWidth;
    float descX, descY, descHeight, descWidth;
    float statsX, statsY, statsHeight, statsWidth;
    float avatarX, avatarY;
    float detailedInfoBoxX, detailedInfoBoxY;
    float starsOffsetX;

    bool menuInitialized;
    string backgroundName;

    int fontId;
    string title;
    string displayTitle;
    WFont * mFont;
    float menuFontScale;
    float titleFontScale;

    int maxItems, startId;

    float selectionT, selectionY;
    float timeOpen;

    static hgeParticleSystem* stars;

    void initMenuItems();
    string getDescription();
    string getMetaInformation();
    DeckMetaData *selectedDeck;

public:
    TextScroller * scroller;
    bool autoTranslate;

    //used for detailed info button
    JQuad * pspIcons[8];
    JTexture * pspIconsTexture;
    DeckMetaData * getSelectedDeck();
    bool selectedDeckHasDetails();
    int selectedDeckId;
    bool showDetailsScreen;
    bool enableDetails;
    float selectionTargetY;
    bool closed;

    DeckMenu(int id, JGuiListener* listener, int fontId, const string _title = "", const int& startIndex = 0, const float& mFontScale = 1.0f);
    ~DeckMenu();

    void Render();
    void Update(float dt);
    void Add(int id, const char * Text, string desc = "", bool forceFocus = false, DeckMetaData *deckMetaData = NULL);
    void Close();
    void updateScroller();
    void RenderBackground();

    static void destroy();
};

#endif
