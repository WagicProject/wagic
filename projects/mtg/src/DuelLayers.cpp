#include "../include/config.h"
#include "../include/DuelLayers.h"
#include "../include/MTGRules.h"
#include "../include/DamageResolverLayer.h"
#include "../include/GuiPhaseBar.h"


void DuelLayers::init(){


  //0 Stack Layer
  ActionStack * mActionStack = NEW ActionStack(0, GameObserver::GetInstance());

  //Damage Resolver
  DamageResolverLayer * mDamageResolver = NEW DamageResolverLayer(1, GameObserver::GetInstance());

  //1 Action Layer
  GuiLayer * actionLayer = NEW ActionLayer(2, GameObserver::GetInstance());
  MTGGamePhase * phaseManager = NEW MTGGamePhase(actionLayer->getMaxId());
  actionLayer->Add(phaseManager);
  //Add Magic Specific Rules
  actionLayer->Add(NEW MTGPutInPlayRule(-1));
  actionLayer->Add(NEW MTGAttackRule(-1));
  actionLayer->Add(NEW MTGBlockRule(-1));
  actionLayer->Add(NEW MTGLegendRule(-1));
  actionLayer->Add(NEW MTGPersistRule(-1));
  actionLayer->Add(NEW MTGLifelinkRule(-1));
  //Other display elements
  actionLayer->Add(NEW HUDDisplay(-1));

  //2 Hand Layer
  MTGGuiHand * mGuiHand = NEW MTGGuiHand(3, GameObserver::GetInstance());

  //3 Game
  MTGGuiPlay * play = NEW MTGGuiPlay(4, GameObserver::GetInstance());

  //Add(NEW GuiPhaseBar(GameObserver::GetInstance()));
  Add(mActionStack);
  Add(mDamageResolver);
  Add(actionLayer);
  Add(mGuiHand);
  Add(play);
}


ActionStack * DuelLayers::stackLayer(){
  return ((ActionStack *) (objects[0]));
}

DamageResolverLayer * DuelLayers::combatLayer(){
  return ((DamageResolverLayer *) (objects[1]));
}

ActionLayer * DuelLayers::actionLayer(){
  return ((ActionLayer *) (objects[2]));
}

MTGGuiHand * DuelLayers::handLayer(){
  return ((MTGGuiHand *) (objects[3]));
}
MTGGuiPlay * DuelLayers::playLayer(){
  return ((MTGGuiPlay *) (objects[4]));
}


