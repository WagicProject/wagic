#ifndef _BLOCKER_H_
#define _BLOCKER_H_


#define MAX_BLOCKERS 64  // Currently needs to be equal to MAX_GUI_OBJECTS in JGE/JGui.h.


#include "MTGAbility.h"


class ManaCost;
class GameObserver;
class MTGAbility;

class Blocker : public MTGAbility {
 protected:
  ManaCost * manaCost;
  int currentPhase;
  void init(ManaCost * _cost);
 public:
  virtual ManaCost * untapManaCost(){return manaCost;};
  virtual int unblock(){return 1;};
  Blocker(int id, MTGCardInstance * card, ManaCost * _cost);
  Blocker(int id, MTGCardInstance * card);
  Blocker(int id, MTGCardInstance * card, MTGCardInstance *_target);
  Blocker(int id, MTGCardInstance * card, MTGCardInstance *_target, ManaCost * _cost);
  ~Blocker();
  virtual void Update(float dt);
  virtual int destroy();
};


class Blockers {
 protected:
  int cursor;
  int blockers[MAX_BLOCKERS];
  GameObserver * game;
 public:
  Blockers();
  ~Blockers();
  int Add (Blocker * ability);
  int Remove (Blocker * ability);
  int init();
  Blocker * next();
  int rewind();
  int isEmpty();
};


#include "ManaCost.h"
#include "GameObserver.h"

#endif

