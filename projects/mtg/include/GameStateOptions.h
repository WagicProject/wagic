#ifndef _GAME_STATE_OPTIONS_H_
#define _GAME_STATE_OPTIONS_H_

#include <JGui.h>
#include "../include/GameState.h"

#define SHOW_OPTIONS 1
#define SHOW_OPTIONS_MENU 2
#define SHOW_OPTIONS_PROFILE 3

class GameApp;
class OptionsMenu;
class SimpleMenu;
class SimplePad;

class GameStateOptions: public GameState, public JGuiListener
{
private:
  float timer;

 public:
  SimpleMenu * optionsMenu;
  SimpleMenu * confirmMenu;
  OptionsMenu * optionsTabs;

  int mState;
  GameStateOptions(GameApp* parent);
  virtual ~GameStateOptions();

  virtual void Start();
  virtual void End();
  virtual void Update(float dt);
  virtual void Render();
  void ButtonPressed(int controllerId, int ControlId);

};


#endif

