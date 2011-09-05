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

#define NO_USER_ACTIVITY_HELP_DELAY 10
#define NO_USER_ACTIVITY_SHOWCARD_DELAY 0.1

enum
{
    STAGE_TRANSITION_RIGHT = 0,
    STAGE_TRANSITION_LEFT = 1,
    STAGE_WAITING = 2,
    STAGE_TRANSITION_UP = 3,
    STAGE_TRANSITION_DOWN = 4,
    STAGE_ONSCREEN_MENU = 5,
    STAGE_WELCOME = 6,
    STAGE_MENU = 7,
    STAGE_FILTERS = 8,
    STAGE_TRANSITION_SELECTED = 9
};

// TODO: need a better name for MENU_FIRST_MENU, this is reused for the 1st submenu of
// available options in the duel menu
enum
{
    MENU_CARD_PURCHASE = 2,
    MENU_DECK_SELECTION = 10,
    MENU_DECK_BUILDER = 11,
    MENU_FIRST_DUEL_SUBMENU = 102,
    MENU_LANGUAGE_SELECTION = 103,
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

#define ALL_COLORS -1

#define ROTATE_LEFT 1;
#define ROTATE_RIGHT 0;

#define HIGH_SPEED 15.0
#define MED_SPEED 5.0f
#define LOW_SPEED 1.5

#define MAX_SAVED_FILTERS 8
#define CARDS_DISPLAYED 7

class GameStateDeckViewer: public GameState, public JGuiListener
{
private:
    JQuadPtr mIcons[CARDS_DISPLAYED];
    JQuadPtr pspIcons[8];
    JTexture * pspIconsTexture;
    float last_user_activity;
    float onScreenTransition;
    float mRotation;
    float mSlide;
    int mAlpha;
    int mStage;
    int useFilter;
    JMusic * bgMusic;
    int lastPos;
    int lastTotal;
    int mSelected;

    WGuiFilters * filterMenu;
    WSrcDeckViewer * source;

    DeckEditorMenu * welcome_menu;
    SimpleMenu * subMenu;
    DeckEditorMenu * menu;
    PriceList* pricelist;
    PlayerData * playerdata;
    int price;
    DeckDataWrapper * displayed_deck;
    DeckDataWrapper * myDeck;
    DeckDataWrapper * myCollection;
    MTGCard * cardIndex[CARDS_DISPLAYED];
    StatsWrapper *stw;

    int hudAlpha;
    string newDeckname;
    bool isAIDeckSave;
    bool mSwitching;
    void saveDeck(); //Saves the deck and additional necessary information
    void saveAsAIDeck(string deckName); // saves deck as an AI Deck
    int getCurrentPos();
    pair<float, float> cardsCoordinates[CARDS_DISPLAYED];

public:
    GameStateDeckViewer(GameApp* parent);
    virtual ~GameStateDeckViewer();
    void updateDecks();
    void rotateCards(int direction);
    void loadIndexes();
    void updateFilters();
    void rebuildFilters();
    void switchDisplay();
    void Start();
    virtual void End();
    void addRemove(MTGCard * card);
    virtual void Update(float dt);
    void renderOnScreenBasicInfo();
    void renderSlideBar();
    void renderDeckBackground();
    void renderOnScreenMenu();
    virtual void renderCard(int id, float rotation);
    virtual void renderCard(int id);
    virtual void Render();
    int loadDeck(int deckid);
    void LoadDeckStatistics(int deckId);

    void OnScroll(int inXVelocity, int inYVelocity);

    void buildEditorMenu();
    virtual void ButtonPressed(int controllerId, int controlId);
};

#endif
