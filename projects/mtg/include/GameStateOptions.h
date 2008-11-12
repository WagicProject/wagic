#ifndef _GAME_STATE_OPTIONS_H_
#define _GAME_STATE_OPTIONS_H_

#include <JGui.h>
#include "../include/GameState.h"

#define SHOW_OPTIONS 1
#define SHOW_OPTIONS_MENU 2

class GameApp;
class OptionsList;
class SimpleMenu;

class GameStateOptions: public GameState, public JGuiListener
{

 public:
  SimpleMenu * optionsMenu;
  int mState;
  OptionsList * optionsList;
  GameStateOptions(GameApp* parent);
  virtual ~GameStateOptions();

  virtual void Start();
  virtual void End();
  virtual void Update(float dt);
  virtual void Render();
  void ButtonPressed(int controllerId, int ControlId);

};


#endif

