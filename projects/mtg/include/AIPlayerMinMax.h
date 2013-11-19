/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/

 AIPlayerMinMax  is the MinMax implementation of the AIPlayer interface
 */

#ifndef _IAPLAYER_MINMAX_H
#define _IAPLAYER_MINMAX_H

#include "AIPlayer.h"
#include "config.h"

namespace AI {

class AIPlayerMinMax: public AIPlayer{

protected:
    void LookAround(); 

public:
    AIPlayerMinMax(GameObserver *observer, string deckFile, string deckFileSmall, string avatarFile, MTGDeck * deck = NULL);
    virtual ~AIPlayerMinMax();
    
    virtual int chooseTarget(TargetChooser * tc = NULL, Player * forceTarget = NULL, MTGCardInstance * Chosencard = NULL, bool checkonly = false) = 0;
    virtual int affectCombatDamages(CombatStep) = 0;
    virtual int Act(float dt) = 0;

};

};

#endif
