#ifndef _GAME_STATE_OPTIONS_H_
#define _GAME_STATE_OPTIONS_H_

#include <JGui.h>
#include "../include/GameState.h"

#define SHOW_OPTIONS 1
#define SHOW_OPTIONS_MENU 2

class GameApp;
class WGuiTabMenu;
class SimpleMenu;
class SimplePad;

class GameStateOptions: public GameState, public JGuiListener
{
private:
  float timer;
  bool mReload;

 public:
  SimpleMenu * optionsMenu;
  WGuiTabMenu * optionsTabs;
  int mState;
  
  GameStateOptions(GameApp* parent);
  virtual ~GameStateOptions();

  virtual void Start();
  virtual void End();
  virtual void Update(float dt);
  virtual void Render();
  void ButtonPressed(int controllerId, int ControlId);

  string newProfile;

};


#endif
