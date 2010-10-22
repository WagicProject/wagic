#ifndef _DUELLAYERS_H_
#define _DUELLAYERS_H_

#include "GuiLayers.h"
#include "CardSelector.h"

class MTGGuiHand;
class MTGGuiPlay;
class ActionLayer;
class ActionStack;
class DamageResolverLayer;
class GuiHandSelf;
class GuiHandOpponent;
class GuiCombat;
class GuiAvatars;
struct Pos;

class DuelLayers {
 protected:
  int nbitems;
  vector<GuiLayer*> objects;
  vector<Pos*> waiters;
  GuiCombat* combat;
  ActionLayer* action;
  ActionStack* stack;
  GuiHandSelf *hand;
  GuiAvatars * avatars;

public:
  DuelLayers();
  ~DuelLayers();

  ActionLayer * actionLayer();
  ActionStack * stackLayer();
  GuiCombat * combatLayer();
  GuiAvatars * GetAvatars();
  void init();
  virtual void Update(float dt, Player * player);
  void CheckUserInput(int isAI);
  void Render();
  void Add(GuiLayer * layer);
  void Remove();
  int receiveEvent(WEvent * e);
  float RightBoundary();

  CardSelector* mCardSelector;
};

#include "ActionLayer.h"
#include "GameObserver.h"
#include "MTGGamePhase.h"
#include "ActionStack.h"
#include "Damage.h"


#endif
