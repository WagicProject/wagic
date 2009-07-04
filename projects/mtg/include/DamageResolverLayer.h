#ifndef _DAMAGERESOLVERLAYER_H_
#define _DAMAGERESOLVERLAYER_H_
#include "../include/PlayGuiObjectController.h"

class MTGCardInstance;
class DamagerDamaged;
class DamageStack;


class DamageResolverLayer:public PlayGuiObjectController{
 protected:
  int trampleDamage();
  void updateAllCoordinates();
 public:
  int buttonOk;
  int orderingIsNeeded;

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
  DamagerDamaged * addIfNotExists(MTGCardInstance * card, Player * selecter);
  int addDamager(MTGCardInstance * card, Player * selecter);
  int updateCoordinates(MTGCardInstance * card);
  DamagerDamaged * findByCard(MTGCardInstance * card);
  int canStopDealDamages();
  int resolveDamages();
  int isOpponent(DamagerDamaged * a, DamagerDamaged * b);
  int nextPlayer();
  virtual void Update(float dt);
  virtual bool CheckUserInput(u32 key);
  virtual void Render();
  int isDisplayed(){return mCount;};
  int autoOrderBlockers();
  bool blockersOrderingDone();

  bool clickDamage(DamagerDamaged * current);
  bool clickDamage(MTGCardInstance * c);
  bool clickReorderBlocker(MTGCardInstance * blocker);

  bool checkUserInputOrderBlockers(u32 key);
};

#endif
