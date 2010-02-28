#ifndef _GAME_STATE_OPTIONS_H_
#define _GAME_STATE_OPTIONS_H_

#include <JGE.h>
#include <JGui.h>
#include "../include/GameState.h"

class GameApp;
class WGuiTabMenu;
class SimpleMenu;
class SimplePad;

struct KeybGrabber {
  virtual void KeyPressed(LocalKeySym) = 0;
};

class GameStateOptions: public GameState, public JGuiListener {
private:
  enum {
    SHOW_OPTIONS,
    SHOW_OPTIONS_MENU,
    SAVE
  };
  float timer;
  bool mReload;
  KeybGrabber* grabber;

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
  virtual void GrabKeyboard(KeybGrabber*);
  virtual void UngrabKeyboard(const KeybGrabber*);
  void ButtonPressed(int controllerId, int ControlId);

  string newProfile;

};


#endif
