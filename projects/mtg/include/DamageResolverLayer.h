#ifndef _DAMAGERESOLVERLAYER_H_
#define _DAMAGERESOLVERLAYER_H_
#include "../include/PlayGuiObjectController.h"

class MTGCardInstance;
class DamagerDamaged;
class DamageStack;


class DamageResolverLayer:public PlayGuiObjectController{
 protected:
  int trampleDamage();
 public:
  int buttonOk;
  int currentPhase;
  int remainingDamageSteps;
  Player *  currentChoosingPlayer;
  DamageStack * damageStack;
  DamagerDamaged * currentSource;

  DamageResolverLayer(int id, GameObserver* _game);
  int init();
  int initResolve();
  Player * whoSelectsDamagesDealtBy(MTGCardInstance * card);
  int addAutoDamageToOpponents(MTGCardInstance * card);
  int addIfNotExists(MTGCardInstance * card, Player * selecter);
  int addDamager(MTGCardInstance * card, Player * selecter);
  DamagerDamaged * findByCard(MTGCardInstance * card);
  int canStopDealDamages();
  int resolveDamages();
  int isOpponent(DamagerDamaged * a, DamagerDamaged * b);
  void nextPlayer();
  virtual void Update(float dt);
  virtual void CheckUserInput(float dt);
  virtual void Render();
};

#endif
