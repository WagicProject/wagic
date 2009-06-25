/*
  A Filter/Mask system for Card Instances to find cards matching specific settings such as color, type, etc...
*/

#ifndef _CARDDESCRIPTOR_H_
#define _CARDDESCRIPTOR_H_

#include "MTGCardInstance.h"
#include "MTGGameZones.h"

#define CD_OR 1
#define CD_AND 2

class CardDescriptor: public MTGCardInstance{
 protected:
  MTGCardInstance * match_or(MTGCardInstance * card);
  MTGCardInstance * match_and(MTGCardInstance * card);
 public:
  int mode;
  int init();
  CardDescriptor();
  void unsecureSetTapped(int i);
  void setNegativeSubtype( string value);
  MTGCardInstance * match(MTGCardInstance * card);
  MTGCardInstance * match(MTGGameZone * zone);
  MTGCardInstance * nextmatch(MTGGameZone * zone, MTGCardInstance * previous);
};

#endif
