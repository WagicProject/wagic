#ifndef _MTGGAMEPHASE_H_
#define _MTGGAMEPHASE_H_

#include "ActionElement.h"
#include "GameObserver.h"

#include <JGui.h>
#include <JLBFont.h>


class MTGGamePhase: public ActionElement {
 protected:
  float animation;
  int currentState;
  JLBFont * mFont;
 public:
  MTGGamePhase(int id);
  virtual void Render();
  virtual void Update(float dt);
  bool CheckUserInput(u32 key);
  virtual ostream& toString(ostream& out) const;
};


#endif
