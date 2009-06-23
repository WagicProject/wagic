#ifndef _REPLACEMENT_EFFECTS_H_
#define _REPLACEMENT_EFFECTS_H_

#include <list>
using namespace std;
#include "WEvent.h"

class TargetChooser;

class ReplacementEffect {
public:
  virtual WEvent * replace (WEvent * e) {return e;};
  virtual ~ReplacementEffect(){};
};

class REDamagePrevention: public ReplacementEffect {
protected:
  TargetChooser * tcSource;
  TargetChooser * tcTarget;
  int damage;
  bool oneShot;
public:
  REDamagePrevention(TargetChooser *_tcSource = NULL,TargetChooser *_tcTarget = NULL, int _damage = -1, bool _oneShot = true);
  WEvent * replace (WEvent *e);
  ~REDamagePrevention();
};

class ReplacementEffects {
protected:
  list<ReplacementEffect *>modifiers;
public:
  ReplacementEffects();
  WEvent * replace(WEvent *e);
  int add(ReplacementEffect * re);
  int remove (ReplacementEffect * re);
  ~ReplacementEffects();
};

#endif