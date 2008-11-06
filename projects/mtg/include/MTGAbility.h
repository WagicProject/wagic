#ifndef _MTGABILITY_H_
#define _MTGABILITY_H_



class MTGCardInstance;
class GameObserver;
class Spell;
class Damageable;
class PlayGuiObject;
class TargetChooser;
class ManaCost;
class MTGGameZone;
class Player;

#include "ActionElement.h"
#include <string>
#include <map>
using std::string;
using std::map;


#define BAKA_EFFECT_GOOD 10
#define BAKA_EFFECT_BAD 11

class AbilityFactory{
private:
	int destroyAllFromTypeInPlay(const char * type, MTGCardInstance * source, int bury = 0);
	int destroyAllFromColorInPlay(int color, MTGCardInstance * source, int bury = 0);
	int putInPlayFromZone(MTGCardInstance * card, MTGGameZone * zone, Player * p);
 public:
	int magicText(int id, Spell * spell, MTGCardInstance * card = NULL);
  void addAbilities(int _id, Spell * spell);
}; 

class MTGAbility: public ActionElement{
 protected:
	char menuText[25];
  Damageable * target;
  GameObserver * game;
 public:
  MTGCardInstance * source;
  MTGAbility(int id, MTGCardInstance * card); 
  MTGAbility(int id, MTGCardInstance * _source, Damageable * _target);
  virtual int testDestroy();
  virtual ~MTGAbility();
  virtual void Render(){};
	virtual int isReactingToClick(MTGCardInstance * card){return 0;};
	virtual int reactToClick(MTGCardInstance * card){return 0;};
  virtual void Update(float dt){};
	virtual int fireAbility();
	virtual int resolve(){return 0;};

  
};


class TriggeredAbility:public MTGAbility{
	public:
	TriggeredAbility(int id, MTGCardInstance * card);
	TriggeredAbility(int id, MTGCardInstance * _source, Damageable * _target);
	virtual void Update(float dt);
	virtual void Render(){};
	virtual int trigger()=0;
	virtual int resolve() = 0;
};


class ActivatedAbility:public MTGAbility{
public:
	ManaCost * cost;
	int playerturnonly;
	int needsTapping;
	ActivatedAbility(int id, MTGCardInstance * card,ManaCost * _cost = NULL, int _playerturnonly = 0,int tap = 1);
	virtual int reactToClick(MTGCardInstance * card);
	virtual int isReactingToClick(MTGCardInstance * card);
	virtual int reactToTargetClick(Targetable * object);
	virtual int resolve() = 0;
	virtual ~ActivatedAbility();
};

class TargetAbility:public ActivatedAbility{
public:
	TargetAbility(int id, MTGCardInstance * card, TargetChooser * _tc,ManaCost * _cost = NULL, int _playerturnonly = 0,int tap = 1);
	TargetAbility(int id, MTGCardInstance * card,ManaCost * _cost = NULL, int _playerturnonly = 0,int tap = 1);
	virtual void Update(float dt);
	virtual int reactToClick(MTGCardInstance * card);
	virtual int reactToTargetClick(Targetable * object);
	virtual void Render();
};

class InstantAbility:public MTGAbility{
public:
	int init;
	virtual void Update(float dt);
	virtual int testDestroy();
	InstantAbility(int _id, MTGCardInstance * source);
	InstantAbility(int _id, MTGCardInstance * source,Damageable * _target);
	virtual int resolve(){return 0;};
};

/* State based effects. This class works ONLY for InPlay and needs to be extended for other areas of the game !!! */
class ListMaintainerAbility:public MTGAbility{
public:
	map<MTGCardInstance *,bool> cards;
	ListMaintainerAbility(int _id):MTGAbility(_id,NULL){};
	ListMaintainerAbility(int _id, MTGCardInstance *_source):MTGAbility(_id, _source){};
	ListMaintainerAbility(int _id, MTGCardInstance *_source,Damageable * _target):MTGAbility(_id, _source, _target){};
	virtual void Update(float dt);
	virtual int canBeInList(MTGCardInstance * card) = 0;
	virtual int added(MTGCardInstance * card) = 0;
	virtual int removed(MTGCardInstance * card) = 0;
	virtual int destroy();
};

#include "MTGCardInstance.h"

#endif
