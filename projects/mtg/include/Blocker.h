#ifndef _BLOCKER_H_
#define _BLOCKER_H_


#define MAX_BLOCKERS 64  // Currently needs to be equal to MAX_GUI_OBJECTS in JGE/JGui.h.


#include "MTGAbility.h"


class ManaCost;
class GameObserver;
class MTGAbility;

class UntapBlocker : public MTGAbility {
 protected:
  ManaCost * manaCost;
  int currentPhase;
  void init(ManaCost * _cost);
 public:
  virtual ManaCost * untapManaCost(){return manaCost;};
  virtual int unblock(){return 1;};
  UntapBlocker(int id, MTGCardInstance * card, ManaCost * _cost);
  UntapBlocker(int id, MTGCardInstance * card);
  UntapBlocker(int id, MTGCardInstance * card, MTGCardInstance *_target);
  UntapBlocker(int id, MTGCardInstance * card, MTGCardInstance *_target, ManaCost * _cost);
  ~UntapBlocker();
  virtual void Update(float dt);
  virtual int destroy();
};


class UntapBlockers {
 protected:
  int cursor;
  int blockers[MAX_BLOCKERS];
  GameObserver * game;
 public:
  UntapBlockers();
  ~UntapBlockers();
  int Add (UntapBlocker * ability);
  int Remove (UntapBlocker * ability);
  int init();
  UntapBlocker * next();
  int rewind();
  int isEmpty();
};


#include "ManaCost.h"
#include "GameObserver.h"

#endif

