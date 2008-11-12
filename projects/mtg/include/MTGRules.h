/* Default observers/Abilities that are added to the game for a standard Magic Game
 */

#ifndef _MTGRULES_H_
#define _MTGRULES_H_

#include "../include/MTGAbility.h"
#include "../include/Counters.h"


class MTGAttackRule:public MTGAbility{
 public:
  int isReactingToClick(MTGCardInstance * card);
  int reactToClick(MTGCardInstance * card);
  int testDestroy();
  MTGAttackRule(int _id);
  const char * getMenuText(){return "Attacker";}

};

class MTGBlockRule:public MTGAbility{
 public:
  int isReactingToClick(MTGCardInstance * card);
  int reactToClick(MTGCardInstance * card);
  int testDestroy();
  MTGBlockRule(int _id);
  const char * getMenuText(){return "Blocker";}
};


/* Persist Rule */
class MTGPersistRule:public ListMaintainerAbility{
 public:
 MTGPersistRule(int _id):ListMaintainerAbility(_id){};

  virtual void Update(float dt){
    map<MTGCardInstance *,bool>::iterator it;

    for ( it=cards.begin() ; it != cards.end(); it++ ){
      MTGCardInstance * card = ((*it).first);
      Player * p = card->controller();
      if (p->game->graveyard->hasCard(card)){
#if defined (WIN32) || defined (LINUX)
	OutputDebugString("persist passed test 1 !\n");
#endif
	p->game->putInZone(card,  p->game->graveyard, p->game->hand);
	Spell * spell = NEW Spell(card);
	p->game->putInZone(card,  p->game->hand, p->game->stack);
	spell->resolve();
	delete spell;
#if defined (WIN32) || defined (LINUX)
	OutputDebugString("persist passed test 2 !\n");
#endif
	card->counters->addCounter(-1,-1);
#if defined (WIN32) || defined (LINUX)
	OutputDebugString("persist passed test 3 !\n");
#endif
      }
    }
    ListMaintainerAbility::Update(dt);
  }

  int canBeInList(MTGCardInstance * card){
    if (card->basicAbilities[PERSIST] && !card->counters->hasCounter(-1,-1) ){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("yay, persist !\n");
#endif
      return 1;
    }
    return 0;
  }

  int added(MTGCardInstance * card){return 1;}

  int removed(MTGCardInstance * card){return 0;}

  int testDestroy(){return 0;}
};



/*
 * Rule 420.5e (Legend Rule)
 * If two or more legendary permanents with the same name are in play, all are put into their
 * owners' graveyards. This is called the "legend rule." If only one of those permanents is
 * legendary, this rule doesn't apply.
 */
class MTGLegendRule:public ListMaintainerAbility{
 public:
 MTGLegendRule(int _id):ListMaintainerAbility(_id){};

  int canBeInList(MTGCardInstance * card){
    if (card->basicAbilities[LEGENDARY]){
      return 1;
    }
    return 0;
  }

  int added(MTGCardInstance * card){
    map<MTGCardInstance *,bool>::iterator it;
    int destroy = 0;
    for ( it=cards.begin() ; it != cards.end(); it++ ){
      MTGCardInstance * comparison = (*it).first;
      if (comparison!= card && !strcmp(comparison->getName(), card->getName())){
	comparison->owner->game->putInGraveyard(comparison);
	destroy = 1;
      }
    }
    if (destroy){
      card->owner->game->putInGraveyard(card);
    }
    return 1;
  }

  int removed(MTGCardInstance * card){return 0;}

  int testDestroy(){return 0;}
};

#endif
