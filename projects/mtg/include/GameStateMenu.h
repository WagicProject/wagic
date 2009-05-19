#ifndef _GAME_STATE_MENU_H_
#define _GAME_STATE_MENU_H_

#include <JGui.h>
#include <dirent.h>
#include "../include/GameState.h"
#include "../include/SimpleMenu.h"
#include "../include/TextScroller.h"

class GameStateMenu: public GameState, public JGuiListener
{
 private:
 TextScroller * scroller;
 int scrollerSet;
  JGuiController* mGuiController;
  SimpleMenu* subMenuController;
  SimpleMenu* gameTypeMenu;
  int hasChosenGameType;
  JQuad * mIcons[10];
  JTexture * mIconsTexture;
  JTexture * bgTexture;
  JTexture * movingWTexture;
  JQuad * mBg;
  JQuad * mMovingW;
  JTexture * splashTex;
  JQuad * splashQuad;
  float mCreditsYPos;
  int currentState;
  //JMusic * bgMusic;
  int mVolume;
  char nbcardsStr[400];

  DIR *mDip;
  struct dirent *mDit;
  char mCurrentSetName[10];
  char mCurrentSetFileName[512];

  int mReadConf;
  float timeIndex;
  float angleMultiplier;
  float angleW;
  float yW;
  void fillScroller();
 public:
  GameStateMenu(GameApp* parent);
  virtual ~GameStateMenu();
  virtual void Create();
  virtual void Destroy();
  virtual void Start();
  virtual void End();
  virtual void Update(float dt);
  virtual void Render();
  virtual void ButtonPressed(int controllerId, int controlId);

  int nextCardSet(); // Retrieves the next set subfolder automatically
  void createUsersFirstDeck(int setId);
};

#endif
