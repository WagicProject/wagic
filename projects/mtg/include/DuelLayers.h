#ifndef _DUELLAYERS_H_
#define _DUELLAYERS_H_


#include "GuiLayers.h"

class MTGGuiHand;
class MTGGuiPlay;
class ActionLayer;
class ActionStack;
class DamageResolverLayer;

class DuelLayers: public GuiLayers{

public : 
  ActionLayer * actionLayer();
  MTGGuiHand * handLayer();
  MTGGuiPlay * playLayer();
  ActionStack * stackLayer();
  DamageResolverLayer * combatLayer();
  void init();

};

#include "ActionLayer.h"
#include "GameObserver.h"
#include "MTGGamePhase.h"
#include "MTGGuiHand.h"
#include "MTGGuiPlay.h"
#include "ActionStack.h"
#include "Damage.h"


#endif
