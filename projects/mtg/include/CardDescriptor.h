/*
A Filter/Mask system for Card Instances to find cards matching specific settings such as color, type, etc...
*/

#ifndef _CARDDESCRIPTOR_H_
#define _CARDDESCRIPTOR_H_

#include "MTGCardInstance.h"
#include "MTGGameZones.h"

class CardDescriptor: public MTGCardInstance{
protected:

public:
	int init();
	CardDescriptor();
	MTGCardInstance * match(MTGCardInstance * card);
	MTGCardInstance * match(MTGGameZone * zone);
	MTGCardInstance * nextmatch(MTGGameZone * zone, MTGCardInstance * previous);
};

#endif