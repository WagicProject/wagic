#ifndef _GAME_STATE_SHOP_H_
#define _GAME_STATE_SHOP_H_

#include <JGE.h>
#include "../include/GameState.h"
#include "../include/SimpleMenu.h"
#include "../include/ShopItem.h"


#define STATE_BUY 1
#define STATE_SELL 2
#define STAGE_SHOP_MENU 3
#define STAGE_SHOP_SHOP 4



class GameStateShop: public GameState, public JGuiListener
{
 private:

  ShopItems * shop;
  JTexture * altThumb[8];
  JQuad * mBack;
  JQuad * mBg;
  JTexture * mBgTex;

  SimpleMenu * menu;
  int mStage;
  char starterBuffer[128], boosterBuffer[128];
  char setNames[SHOP_BOOSTERS][128];
  int setIds[SHOP_BOOSTERS];
  void load();
 public:
  GameStateShop(GameApp* parent);
  virtual ~GameStateShop();

  virtual void Start();
  virtual void End();
  virtual void Create();
  virtual void Destroy();
  virtual void Update(float dt);
  virtual void Render();
  virtual void ButtonPressed(int controllerId, int controlId);

  
  

};


#endif

