#ifndef _CREDITS_H_
#define _CREDITS_H_


#include <vector>
#include <string>
#include <JGE.h>
#include <JLBFont.h>
#include "../include/Player.h"
class GameApp;

using namespace std;


class CreditBonus{
public:
  int value;
  string text;
  CreditBonus(int _value, string _text);
  void Render(float x, float y, JLBFont * font);
};

class Credits{
private:
  int isDifficultyUnlocked();
  int isMomirUnlocked();
  int isEvilTwinUnlocked();
  int isRandomDeckUnlocked();
public:
  int value;
  Player * p1, *p2;
  GameApp * app;
  int showMsg;
  int unlocked;
  JQuad * unlockedQuad;
  JTexture * unlockedTex;
  vector<CreditBonus *> bonus;
  Credits();
  ~Credits();
  void compute(Player * _p1, Player * _p2, GameApp * _app);
  void Render();
};

#endif