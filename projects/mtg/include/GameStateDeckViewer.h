#ifndef _GAME_STATE_DECK_VIEWER_H_
#define _GAME_STATE_DECK_VIEWER_H_

#include <math.h>
#include <iostream>

#include <JGE.h>

#include "../include/GameState.h"
#include "../include/SimpleMenu.h"
#include "../include/WResourceManager.h"
#include "../include/CardGui.h"
#include "../include/GameOptions.h"
#include "../include/PriceList.h"
#include "../include/PlayerData.h"
#include "../include/DeckDataWrapper.h"
#include "../include/DeckStats.h"
#include "../include/WDataSrc.h"
#include "../include/WGui.h"

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
    STAGE_FILTERS = 8
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
    MENU_ITEM_CANCEL = -1,
    MENU_ITEM_SAVE_RETURN_MAIN_MENU = 0,
    MENU_ITEM_SAVE_RENAME = 1,
    MENU_ITEM_SWITCH_DECKS_NO_SAVE = 2,
    MENU_ITEM_MAIN_MENU = 3,
    MENU_ITEM_EDITOR_CANCEL = 4,
    MENU_ITEM_SAVE_AS_AI_DECK = 5,
    MENU_ITEM_YES = 20,
    MENU_ITEM_NO = 21,
    MENU_ITEM_FILTER_BY = 22


  };

#define ALL_COLORS -1

#define ROTATE_LEFT 1;
#define ROTATE_RIGHT 0;

#define HIGH_SPEED 15.0
#define MED_SPEED 5.0f
#define LOW_SPEED 1.5

#define MAX_SAVED_FILTERS 8

static const int STATS_FOR_TURNS = 8;
static const int STATS_MAX_MANA_COST = 9; 

// Struct to hold statistics-related stuff
struct StatsWrapper {
  // Stats parameters and status 
  int currentPage;
  int pageCount;
  bool needUpdate;

  // Actual stats
  int percentVictories;
  int gamesPlayed;
  int cardCount;
  int countLands;
  int totalPrice;
  int totalManaCost;
  float avgManaCost;
  int totalCreatureCost;
  float avgCreatureCost;
  int totalSpellCost;
  float avgSpellCost;
  int countManaProducers;

  int countCreatures, countSpells, countInstants, countEnchantments, countSorceries, countArtifacts;

  float noLandsProbInTurn[STATS_FOR_TURNS];
  float noCreaturesProbInTurn[STATS_FOR_TURNS];

  int countCardsPerCost[STATS_MAX_MANA_COST+1];
  int countCardsPerCostAndColor[STATS_MAX_MANA_COST+1][Constants::MTG_NB_COLORS+1];
  int countCreaturesPerCost[STATS_MAX_MANA_COST+1];
  int countCreaturesPerCostAndColor[STATS_MAX_MANA_COST+1][Constants::MTG_NB_COLORS+1];
  int countSpellsPerCost[STATS_MAX_MANA_COST+1];
  int countSpellsPerCostAndColor[STATS_MAX_MANA_COST+1][Constants::MTG_NB_COLORS+1];
  int countLandsPerColor[Constants::MTG_NB_COLORS+1];
  int countBasicLandsPerColor[Constants::MTG_NB_COLORS+1];
  int countNonLandProducersPerColor[Constants::MTG_NB_COLORS+1];
  int totalCostPerColor[Constants::MTG_NB_COLORS+1];
  int totalColoredSymbols;

  vector<string> aiDeckNames;
  vector<DeckStat*> aiDeckStats;
};

class GameStateDeckViewer: public GameState, public JGuiListener
{
private:
  JQuad * mIcons[7];
  JQuad * pspIcons[8];
  JTexture * pspIconsTexture;
  float last_user_activity;
  float onScreenTransition;
  float mRotation;
  float mSlide;
  int mAlpha;
  int mStage;
  int nbDecks;
  int deckNum;
  int useFilter;
  JMusic * bgMusic;
  JQuad * backQuad;
  int lastPos;
  int lastTotal;
  
  WGuiFilters * filterMenu;
  WSrcDeckViewer * source;

  SimpleMenu * welcome_menu;
  SimpleMenu * subMenu;
  SimpleMenu * menu;
  PriceList* pricelist;
  PlayerData * playerdata;
  int price;
  DeckDataWrapper * displayed_deck;
  DeckDataWrapper * myDeck;
  DeckDataWrapper * myCollection;
  MTGCard *  cardIndex[7];
  int hudAlpha;
  string newDeckname;
  bool isAIDeckSave;
  StatsWrapper stw;
  bool mSwitching;
  void saveDeck(); //Saves the deck and additional necessary information
  void saveAsAIDeck(string deckName); // saves deck as an AI Deck
  int getCurrentPos();

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
  virtual void renderCard (int id);
  virtual void Render();
  int loadDeck(int deckid);
  virtual void ButtonPressed(int controllerId, int controlId);
  void updateStats();
  int countCardsByType(const char * _type);
};

// n cards total, a of them are desired, x drawn 
// returns probability of no A's
float noLuck(int n, int a, int x);

#endif
