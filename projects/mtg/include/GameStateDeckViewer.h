#ifndef _GAME_STATE_DECK_VIEWER_H_
#define _GAME_STATE_DECK_VIEWER_H_

#include <math.h>
#include <iostream>

#include <JGE.h>

#include "GameState.h"
#include "DeckEditorMenu.h"
#include "SimpleMenu.h"
#include "WResourceManager.h"
#include "CardGui.h"
#include "PriceList.h"
#include "PlayerData.h"
#include "DeckDataWrapper.h"
#include "DeckStats.h"
#include "WDataSrc.h"
#include "WGui.h"
#include "InteractiveButton.h"

class DeckView;

// TODO: need a better name for MENU_FIRST_MENU, this is reused for the 1st submenu of
// available options in the duel menu
enum
{
    MENU_CARD_PURCHASE = 2,
    MENU_DECK_SELECTION = 10,
    MENU_DECK_BUILDER = 11,
    MENU_FIRST_DUEL_SUBMENU = 102,
    MENU_LANGUAGE_SELECTION = 103
};

// enums for menu options
// TODO: make these enums a little more descriptive. (ie should reflect what menu they are attached to )
enum DECK_VIEWER_MENU_ITEMS
{
    MENU_ITEM_NEW_DECK = -30,
    MENU_ITEM_CHEAT_MODE = -12,
    MENU_ITEM_CANCEL = kCancelMenuID,
    MENU_ITEM_SAVE_RETURN_MAIN_MENU = 0,
    MENU_ITEM_SAVE_RENAME = 1,
    MENU_ITEM_SWITCH_DECKS_NO_SAVE = 2,
    MENU_ITEM_MAIN_MENU = 3,
    MENU_ITEM_EDITOR_CANCEL = kCancelMenuID,
    MENU_ITEM_SAVE_AS_AI_DECK = 5,
    MENU_ITEM_YES = 20,
    MENU_ITEM_NO = 21,
    MENU_ITEM_FILTER_BY = 22,
    MENUITEM_MORE_INFO = kInfoMenuID
};

class GameStateDeckViewer: public GameState, public JGuiListener
{
private:
    enum DeckViewerStages
    {
        STAGE_WAITING = 0,
        STAGE_ONSCREEN_MENU,
        STAGE_WELCOME,
        STAGE_MENU,
        STAGE_FILTERS
    };

    vector<JQuadPtr> mIcons;
    JQuadPtr pspIcons[8];
    JTexture * pspIconsTexture;
    float last_user_activity;
    float onScreenTransition;
    DeckViewerStages mStage;
    JMusic * bgMusic;
    
    InteractiveButton *toggleDeckButton, *sellCardButton, *statsPrevButton, *filterButton, *toggleViewButton, *toggleUpButton, *toggleDownButton, *toggleLeftButton, *toggleRightButton;

    WGuiFilters * filterMenu;
    WSrcDeckViewer * source;

    DeckEditorMenu * welcome_menu;
    SimpleMenu * subMenu;
    DeckEditorMenu * deckMenu;
    PriceList* pricelist;
    PlayerData * playerdata;
    DeckDataWrapper * myDeck;
    DeckDataWrapper * myCollection;
    StatsWrapper * mStatsWrapper;

    int hudAlpha;
    string newDeckname;
    bool isAIDeckSave;
    bool mSwitching;

    enum AvailableView{
        CAROUSEL_VIEW,
        GRID_VIEW
    };
    DeckView* mView;
    AvailableView mCurrentView;

    void saveDeck(); //Saves the deck and additional necessary information
    void saveAsAIDeck(string deckName); // saves deck as an AI Deck
    void sellCard();
    void setButtonState(bool state);
    bool userPressedButton();
    void RenderButtons();
    void setupView(AvailableView view, DeckDataWrapper *deck);
    void toggleView();
public:
    GameStateDeckViewer(GameApp* parent);
    virtual ~GameStateDeckViewer();
    void updateDecks();
    void updateFilters();
    void rebuildFilters();
    void toggleCollection();
    void Start();
    virtual void End();
    void addRemove(MTGCard * card);
    virtual void Update(float dt);
    void renderOnScreenBasicInfo();
    void renderSlideBar();
    void renderDeckBackground();
    void renderOnScreenMenu();
    virtual void Render();
    int loadDeck(int deckid);

    void OnScroll(int inXVelocity, int inYVelocity);

    void buildEditorMenu();
    virtual void ButtonPressed(int controllerId, int controlId);
};

#endif
