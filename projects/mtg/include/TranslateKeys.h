#ifndef _TRANSLATEKEYS_H_
#define _TRANSLATEKEYS_H_

#include <string>
#include "JGE.h"

struct KeyRep
{
  KeyRep(JQuad*, std::string);
  JQuad* icon;
  std::string text;
};


const KeyRep& translateKey(LocalKeySym);
const KeyRep& translateKey(JButton);

#endif
