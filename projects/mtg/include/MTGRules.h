/* Default observers/Abilities that are added to the game for a standard Magic Game
 */

#ifndef _MTGRULES_H_
#define _MTGRULES_H_

#include "../include/MTGAbility.h"
#include "../include/Counters.h"
#include "../include/WEvent.h"
#include "../include/CardSelector.h"

class OtherAbilitiesEventReceiver:public MTGAbility{
public:
  int testDestroy();
  int receiveEvent(WEvent * event);
  OtherAbilitiesEventReceiver(int _id);
  OtherAbilitiesEventReceiver * clone() const;
};

class MTGPutInPlayRule:public MTGAbility{
 public:
  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
  int reactToClick(MTGCardInstance * card);
  int testDestroy();
  virtual ostream& toString(ostream& out) const;
  MTGPutInPlayRule(int _id);
  const char * getMenuText(){return "Put into play";}
  virtual MTGPutInPlayRule * clone() const;
};

class MTGAttackRule:public MTGAbility, public Limitor{
 public:
  virtual bool select(Target*);
  virtual bool greyout(Target*);
  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
  int reactToClick(MTGCardInstance * card);
  int testDestroy();
  virtual ostream& toString(ostream& out) const;
  MTGAttackRule(int _id);
  const char * getMenuText(){return "Attacker";}
  int receiveEvent(WEvent * event);
  virtual MTGAttackRule * clone() const;
};

class MTGBlockRule:public MTGAbility{
 public:
  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
  int reactToClick(MTGCardInstance * card);
  int testDestroy();
  virtual ostream& toString(ostream& out) const;
  MTGBlockRule(int _id);
  const char * getMenuText(){return "Blocker";}
  virtual MTGBlockRule * clone() const;
};


/* Persist Rule */
class MTGPersistRule:public MTGAbility{
 public:
  MTGPersistRule(int _id);
  int receiveEvent(WEvent * event);
  virtual ostream& toString(ostream& out) const;
  int testDestroy();
  virtual MTGPersistRule * clone() const;
};

class MTGUnearthRule:public MTGAbility{
 public:
  MTGUnearthRule(int _id);
  int receiveEvent(WEvent * event);
  virtual ostream& toString(ostream& out) const;
  int testDestroy();
  virtual MTGUnearthRule * clone() const;
};

class MTGSneakAttackRule:public MTGAbility{
 public:
  MTGSneakAttackRule(int _id);
  int receiveEvent(WEvent * event);
  virtual ostream& toString(ostream& out) const;
  int testDestroy();
  virtual MTGSneakAttackRule * clone() const;
};

class MTGTokensCleanup:public MTGAbility{
 public:
  vector<MTGCardInstance *> list;
  MTGTokensCleanup(int _id);
  int receiveEvent(WEvent * event);
  int testDestroy();
  void Update(float dt);
  virtual MTGTokensCleanup * clone() const;
};

/*
 * Rule 420.5e (Legend Rule)
 * If two or more legendary permanents with the same name are in play, all are put into their
 * owners' graveyards. This is called the "legend rule." If only one of those permanents is
 * legendary, this rule doesn't apply.
 */
class MTGLegendRule:public ListMaintainerAbility{
 public:
  MTGLegendRule(int _id);
  int canBeInList(MTGCardInstance * card);
  int added(MTGCardInstance * card);
  int removed(MTGCardInstance * card);
  int testDestroy();
  virtual ostream& toString(ostream& out) const;
  virtual MTGLegendRule * clone() const;
};


class MTGMomirRule:public MTGAbility{
private:
  int genRandomCreatureId(int convertedCost);
  static vector<int> pool[20];
  static int initialized;

  int textAlpha;
  string text;
public:

  int alreadyplayed;
  MTGAllCards * collection;
  MTGCardInstance * genCreature(int id);
  int testDestroy();
  void Update(float dt);
  void Render();
  MTGMomirRule(int _id, MTGAllCards * _collection);
  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
  int reactToClick(MTGCardInstance * card);
  int reactToClick(MTGCardInstance * card, int id);
  const char * getMenuText(){return "Momir";}
  virtual ostream& toString(ostream& out) const;
  virtual MTGMomirRule * clone() const;
};


/* LifeLink */
class MTGLifelinkRule:public MTGAbility{
  public:
  MTGLifelinkRule(int _id);

  int receiveEvent(WEvent * event);

  int testDestroy();

  virtual ostream& toString(ostream& out) const;

  virtual MTGLifelinkRule * clone() const;
};

/* Deathtouch */
class MTGDeathtouchRule:public MTGAbility{
  public:
  MTGDeathtouchRule(int _id);

  int receiveEvent(WEvent * event);

  int testDestroy();
  const char * getMenuText(){return "Deathtouch";}

  virtual MTGDeathtouchRule * clone() const;
};

/* HUD Display */

class HUDString {
public:
  string value;
  int timestamp;
  int quantity;
  HUDString(string s, int ts):value(s),timestamp(ts){quantity = 1;};
};

class HUDDisplay:public MTGAbility{
private:
  list<HUDString *> events;
  float timestamp;
  float popdelay;
  WFont * f;
  float maxWidth;
  int addEvent(string s);
public:
  int testDestroy();
  int receiveEvent(WEvent * event);
  void Update(float dt);
  void Render();
  HUDDisplay(int _id);
  ~HUDDisplay();
  virtual HUDDisplay * clone() const;
};



#endif
