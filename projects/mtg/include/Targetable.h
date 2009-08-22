#ifndef _TARGETABLE_H_
#define _TARGETABLE_H_

#define TARGET_CARD 1
#define TARGET_PLAYER 2
#define TARGET_STACKACTION 3

class Targetable{
 public:
  virtual int typeAsTarget() = 0;
  virtual const string getDisplayName() = 0;
};

#endif
