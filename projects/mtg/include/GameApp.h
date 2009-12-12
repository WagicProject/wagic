/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */





#ifndef _GAMEAPP_H_
#define _GAMEAPP_H_


#include "../include/Logger.h"


#include <JApp.h>
#include <JGE.h>
#include <JSprite.h>
#include <JLBFont.h>
#include <hge/hgeparticle.h>
#include "../include/WResourceManager.h"

#include "../include/GameState.h"
#include "../include/GameOptions.h"

#include "../include/MTGDeck.h"
#include "../include/MTGCard.h"
#include "../include/MTGGameZones.h"

#include "../include/CardEffect.h"

#define PLAYER_TYPE_CPU 0
#define PLAYER_TYPE_HUMAN 1
#define PLAYER_TYPE_TESTSUITE 2


#define GAME_TYPE_CLASSIC 0
#define GAME_TYPE_MOMIR 1
#define GAME_TYPE_RANDOM1 2
#define GAME_TYPE_RANDOM2 3

class MTGAllCards;

class GameApp:	public JApp
{

 private:
#ifdef DEBUG
   int nbUpdates;
   float totalFPS;
#endif
  bool mShowDebugInfo;
  int mScreenShotCount;

  GameState* mCurrentState;
  GameState* mNextState;
  GameState* mGameStates[GAME_STATE_MAX];

 public:

  
  int gameType;
  CardEffect *effect;


  GameApp();
  virtual ~GameApp();

  virtual void Create();
  virtual void Destroy();
  virtual void Update();
  virtual void Render();
  virtual void Pause();
  virtual void Resume();

  void LoadGameStates();
  void SetNextState(int state);
  static hgeParticleSystem * Particles[6];
  static int HasMusic;
  static string systemError;
  static JMusic* music;
  static MTGAllCards * collection;
    static int players[2];

};

extern JQuad* manaIcons[7];

#endif
