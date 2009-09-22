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
    STAGE_MENU = 7
  };


#define ALL_COLORS -1

#define ROTATE_LEFT 1;
#define ROTATE_RIGHT 0;

#define HIGH_SPEED 15.0
#define MED_SPEED 5.0
#define LOW_SPEED 1.5


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
  int colorFilter;
  JMusic * bgMusic;
  JQuad * backQuad;
  SimpleMenu * welcome_menu;
  JLBFont * mFont;
  bool showing_user_deck;
  JLBFont * menuFont;
  SimpleMenu * menu;
  SimpleMenu * sellMenu;
  PriceList* pricelist;
  PlayerData * playerdata;
  int price;
  DeckDataWrapper * displayed_deck;
  DeckDataWrapper * myDeck;
  DeckDataWrapper * myCollection;
  MTGCard * currentCard;
  MTGCard *  cardIndex[7];
  int hudAlpha;
  float scrollSpeed;
  int delSellMenu;
  string newDeckname;

public:
  GameStateDeckViewer(GameApp* parent);
  virtual ~GameStateDeckViewer();
  void updateDecks();
  void rotateCards(int direction);
  void loadIndexes(MTGCard * current = NULL);
  void switchDisplay();
  void Start();
  virtual void End();
  void addRemove(MTGCard * card);
  int Remove(MTGCard * card);
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

};


#endif
