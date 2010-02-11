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


#define ALL_COLORS -1

#define ROTATE_LEFT 1;
#define ROTATE_RIGHT 0;

#define HIGH_SPEED 15.0
#define MED_SPEED 5.0
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
  int useFilter[2];
  JMusic * bgMusic;
  JQuad * backQuad;
  WGuiFilters * filterDeck;
  WGuiFilters * filterCollection;
  SimpleMenu * welcome_menu;
  SimpleMenu * menu;
  SimpleMenu * sellMenu;
  PriceList* pricelist;
  PlayerData * playerdata;
  int price;
  DeckDataWrapper * displayed_deck;
  DeckDataWrapper * myDeck;
  DeckDataWrapper * myCollection;
  MTGCard *  cardIndex[7];
  int hudAlpha;
  int delSellMenu;
  string newDeckname;
  StatsWrapper stw;
  bool mSwitching;

public:
  GameStateDeckViewer(GameApp* parent);
  virtual ~GameStateDeckViewer();
  void updateDecks();
  void rotateCards(int direction);
  void loadIndexes();
  void updateFilters();
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
