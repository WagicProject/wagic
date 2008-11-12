#ifndef _GAME_STATE_H_
#define _GAME_STATE_H_

#define FADING_SPEED		350.0f

class JGE;

#include <JSoundSystem.h>

enum _gameSates
  {
    GAME_STATE_MENU,
    GAME_STATE_DUEL,
    GAME_STATE_DECK_VIEWER,
    GAME_STATE_SHOP,
    GAME_STATE_OPTIONS
  };


class GameApp;

class GameState
{
 protected:
  GameApp* mParent;
  JGE* mEngine;

 public:

  static const char * const menuTexts[];
  GameState(GameApp* parent);
  virtual ~GameState() {}

  virtual void Create() {}
  virtual void Destroy() {}

  virtual void Start() {}
  virtual void End() {}

  virtual void Update(float dt) = 0;
  virtual void Render() = 0;
};


#endif

