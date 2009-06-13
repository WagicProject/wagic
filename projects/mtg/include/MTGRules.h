/* Default observers/Abilities that are added to the game for a standard Magic Game
 */

#ifndef _MTGRULES_H_
#define _MTGRULES_H_

#include "../include/MTGAbility.h"
#include "../include/Counters.h"
#include "../include/WEvent.h"

class MTGPutInPlayRule:public MTGAbility{
 public:
  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
  int reactToClick(MTGCardInstance * card);
  int testDestroy();
  virtual ostream& toString(ostream& out) const;
  MTGPutInPlayRule(int _id);
  const char * getMenuText(){return "Put into play";}
};

class MTGAttackRule:public MTGAbility{
 public:
  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
  int reactToClick(MTGCardInstance * card);
  int testDestroy();
  virtual ostream& toString(ostream& out) const;
  MTGAttackRule(int _id);
  const char * getMenuText(){return "Attacker";}
  void Update(float dt);
};

class MTGBlockRule:public MTGAbility{
 public:
  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
  int reactToClick(MTGCardInstance * card);
  int testDestroy();
  virtual ostream& toString(ostream& out) const;
  MTGBlockRule(int _id);
  const char * getMenuText(){return "Blocker";}
};


/* Persist Rule */
class MTGPersistRule:public MTGAbility{
 public:
  MTGPersistRule(int _id):MTGAbility(_id,NULL){};

  int receiveEvent(WEvent * event){
    if (event->type == WEvent::CHANGE_ZONE){
      WEventZoneChange * e = (WEventZoneChange *) event;
      MTGCardInstance * card = e->card->previous;
      if (card && card->basicAbilities[Constants::PERSIST] && !card->counters->hasCounter(-1,-1)){
        int ok = 0;
        for (int i = 0; i < 2 ; i++){
          Player * p = game->players[i];
          if (e->from == p->game->inPlay) ok = 1;
        }
        if (!ok) return 0;
        for (int i = 0; i < 2 ; i++){
          Player * p = game->players[i];
          if (e->to == p->game->graveyard){
            //p->game->putInZone(card,  p->game->graveyard, card->owner->game->hand);
	          MTGCardInstance * copy = p->game->putInZone(e->card,  p->game->graveyard, e->card->owner->game->stack);
            Spell * spell = NEW Spell(copy);
	          spell->resolve();
            spell->source->counters->addCounter(-1,-1);
            game->mLayers->playLayer()->forceUpdateCards();
            delete spell;
            return 1;
          }
        }
      }
    }
    return 0;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "MTGPersistRule ::: (";
    return MTGAbility::toString(out) << ")";
  }
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
    if (card->basicAbilities[Constants::LEGENDARY] && game->isInPlay(card)){
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

  virtual ostream& toString(ostream& out) const
  {
    return out << "MTGLegendRule :::";
  }
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
};


/* LifeLink */
class MTGLifelinkRule:public MTGAbility{
  public:
  MTGLifelinkRule(int _id):MTGAbility(_id,NULL){};

  int receiveEvent(WEvent * event){
    if (event->type == WEvent::DAMAGE){
      WEventDamage * e = (WEventDamage *) event;
      Damage * d = e->damage;
      MTGCardInstance * card = d->source;
      if (d->damage>0 && card && card->basicAbilities[Constants::LIFELINK]){
        card->controller()->life+= d->damage;
        return 1;
      }
    }
    return 0;
  }

  int testDestroy(){return 0;}

  virtual ostream& toString(ostream& out) const
  {
    out << "MTGLifelinkRule ::: (";
    return MTGAbility::toString(out) << ")";
  }
};


#endif
