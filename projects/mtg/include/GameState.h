#ifndef _GAME_STATE_H_
#define _GAME_STATE_H_

#define FADING_SPEED		350.0f

class JGE;

#include <JSoundSystem.h>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

enum ENUM_GAME_STATE
  {
    GAME_STATE_NONE = -1,
    GAME_STATE_MENU = 1,
    GAME_STATE_DUEL = 2,
    GAME_STATE_DECK_VIEWER = 3,
    GAME_STATE_SHOP = 4,
    GAME_STATE_OPTIONS = 5,
    GAME_STATE_AWARDS = 6,
    GAME_STATE_STORY = 7,
    GAME_STATE_TRANSITION = 8,
    GAME_STATE_MAX = 9,
  };

enum ENUM_GS_TRANSITION
  {
    TRANSITION_FADE = 0,
    TRANSITION_FADE_IN = 1,
    MAX_TRANSITION
  };

class GameApp;
class SimpleMenu;
class Player;

class GameState
{
 protected:
  GameApp* mParent;
  JGE* mEngine;

 public:
  GameState(GameApp* parent);
  virtual ~GameState() {}

  virtual void Create() {}
  virtual void Destroy() {}

  virtual void Start() {}
  virtual void End() {}

  virtual void Update(float dt) = 0;
  virtual void Render() = 0;
  static int fillDeckMenu(SimpleMenu * _menu, string path, string smallDeckPrefix = "", Player * statsPlayer = NULL);
  static int fillDeckMenu( vector<int> * deckIdList, SimpleMenu * _menu, string path, string smallDeckPrefix = "", Player * statsPlayer = NULL);

    string& trim(string &str);
    string& ltrim(string &str);
    string& rtrim(string &str);

    };


#endif

