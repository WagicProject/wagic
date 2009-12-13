#ifndef _CARDS_H_
#define _CARDS_H_

#include "MTGAbility.h"
#include "ManaCost.h"
#include "CardDescriptor.h"
#include "AIPlayer.h"
#include "CardDisplay.h"
#include "Subtypes.h"
#include "CardGui.h"
#include "GameOptions.h"
#include "Token.h"
#include "Counters.h"
#include "WEvent.h"
#include "GuiStatic.h"

#include <JGui.h>
#include <hge/hgeparticle.h>


#include <map>
using std::map;


//
// Misc classes
//
class WParsedInt{
public:
  int intValue;

 int computeX(Spell * spell, MTGCardInstance * card){
  if (spell) return spell->computeX(card);
  return 1; //this should only hapen when the ai calls the ability. This is to give it an idea of the "direction" of X (positive/negative)
 }
 WParsedInt(int value = 0){
   intValue = value;
 }

  WParsedInt(string s, Spell * spell, MTGCardInstance * card){
    MTGCardInstance * target = card->target;
    if (!target) target = card;
    int multiplier = 1;
    if (s[0] == '-'){
      s = s.substr(1);
      multiplier = -1;
    }
    if (s == "x" || s == "X"){
      intValue = computeX(spell,card);
    }else if (s == "manacost"){
      intValue = target->getManaCost()->getConvertedCost();
    }else if (s == "p"){
      intValue = target->power;
    }else if (s == "t"){
      intValue = target->toughness;
    }else{
      intValue = atoi(s.c_str());
    }
    intValue *= multiplier;
  }

  int getValue(){
    return intValue;
  }
};

class WParsedPT{
public:
  bool ok;
  WParsedInt power,toughness;

  WParsedPT(int p, int t){
    power.intValue = p;
    toughness.intValue = t;
    ok = true;
  }

  WParsedPT(string s, Spell * spell, MTGCardInstance * card){
    size_t found = s.find("/");
    ok = false;
    if (found != string::npos){
      size_t end = s.find(" ", found);
      if (end == string::npos) end = s.size();
      size_t start = s.find_last_of(" ",found);
      if (start == string::npos) start = 0;
      else start++;

      power = WParsedInt(s.substr(start,found - start), spell, card);
      toughness = WParsedInt(s.substr(found+1,end-found-1), spell, card);

      ok = true;
    }
  }
};

//
//Triggers
//

class TrCardAddedToZone:public TriggeredAbility{
public:
  TargetChooser * toTc;
  TargetZoneChooser * fromTc;
  TrCardAddedToZone(int id, MTGCardInstance * source, TargetChooser * toTc, TargetZoneChooser * fromTc = NULL):TriggeredAbility(id,source), toTc(toTc), fromTc(fromTc){}

  int resolve(){
    return 0; //This is a trigger, this function should not be called
  }

  int triggerOnEvent(WEvent * event){
    WEventZoneChange * e = dynamic_cast<WEventZoneChange*>(event);
    if (!e) return 0;
    if (!toTc->canTarget(e->card)) return 0;
    if (fromTc && !fromTc->targetsZone(e->from)) return 0;
    
    //Battlefield is a special case. We usually don't want to trigger when a card comes from battlefield to battlefield
    // http://code.google.com/p/wagic/issues/detail?id=179
    if ((e->from == game->players[0]->game->battlefield || e->from == game->players[1]->game->battlefield) && 
      (e->to == game->players[0]->game->battlefield || e->to == game->players[1]->game->battlefield)) {
        return 0;
    }
    return 1;
  }

  ~TrCardAddedToZone(){
    SAFE_DELETE(toTc);
    SAFE_DELETE(fromTc);
  }

  TrCardAddedToZone * clone() const{
    TrCardAddedToZone * a =  NEW TrCardAddedToZone(*this);
    a->isClone = 1;
    return a;
  }
};

class TrCardTapped:public TriggeredAbility{
public:
  TargetChooser * tc;
  bool tap;
  TrCardTapped(int id, MTGCardInstance * source, TargetChooser * tc, bool tap = true):TriggeredAbility(id,source), tc(tc),tap(tap){}

  int resolve(){
    return 0; //This is a trigger, this function should not be called
  }

  int triggerOnEvent(WEvent * event){
    WEventCardTap * e = dynamic_cast<WEventCardTap *>(event);
    if (!e) return 0;
    if (e->before == e->after) return 0;
    if (e->after != tap) return 0;
    if (!tc->canTarget(e->card)) return 0;
    return 1;
  }

  ~TrCardTapped(){
    SAFE_DELETE(tc);
  }

  TrCardTapped * clone() const{
    TrCardTapped * a =  NEW TrCardTapped(*this);
    a->isClone = 1;
    return a;
  }
};

class TrDamaged:public TriggeredAbility{
public:
  TargetChooser * tc;
  TargetChooser * fromTc;
  TrDamaged (int id, MTGCardInstance * source, TargetChooser * tc, TargetChooser * fromTc = NULL):TriggeredAbility(id,source), tc(tc), fromTc(fromTc){}

  int resolve(){
    return 0; //This is a trigger, this function should not be called
  }

  int triggerOnEvent(WEvent * event){
    WEventDamage * e = dynamic_cast<WEventDamage *>(event);
    if (!e) return 0;
	if(!tc->canTarget(e->damage->target)) return 0;
	if (fromTc && !fromTc->canTarget(e->damage->source)) return 0;
    return 1;
  }

  ~TrDamaged (){
    SAFE_DELETE(tc);
	  SAFE_DELETE(fromTc);
  }

  TrDamaged * clone() const{
    TrDamaged * a =  NEW TrDamaged (*this);
    a->isClone = 1;
    return a;
  }
};

//counters
class AACounter: public ActivatedAbility{
 public:
  int nb;
  int power;
  int toughness;
 AACounter(int id, MTGCardInstance * source, MTGCardInstance * target, int power, int toughness, int nb, ManaCost * cost = NULL, int doTap = 0) : ActivatedAbility(id, source, cost, 0, doTap), nb(nb), power(power), toughness(toughness) {
	  this->target = target;
 }


 int resolve(){
   if (target){
     MTGCardInstance * _target = (MTGCardInstance *)target;
     if (nb>0){
       for (int i=0; i < nb; i++){
        _target->counters->addCounter(power, toughness);
       }
     }else{
       for (int i=0; i < -nb; i++){
        _target->counters->removeCounter(power, toughness);
       }
     }
     return nb;
   }
   return 0;
 }

  AACounter * clone() const{
    AACounter * a =  NEW AACounter(*this);
    a->isClone = 1;
    return a;
  }
};


class AAFizzler:public ActivatedAbility{
 public:
 AAFizzler(int _id, MTGCardInstance * card, Spell * _target, ManaCost * _cost = NULL, int _tap = 0):ActivatedAbility(_id, card,_cost,0,_tap){
   target = _target;
  }

  int resolve(){
    Spell * _target = (Spell *) target;
    if(target && _target->source->has(Constants::NOFIZZLE))
      return 0;
    game->mLayers->stackLayer()->Fizzle(_target);
    return 1;
  }

  const char * getMenuText(){
    return "Fizzle";
  }

  AAFizzler* clone() const{
    AAFizzler * a =  NEW AAFizzler(*this);
    a->isClone = 1;
    return a;
  }

};


/*
  Generic classes
*/



//MayAbility: May do something when comes into play (should be extended)
class MayAbility:public MTGAbility{
public:
  int triggered;
  MTGAbility * ability;
  MTGAbility * mClone;
  MayAbility(int _id, MTGAbility * _ability,  MTGCardInstance * _source):MTGAbility(_id,_source),ability(_ability){
    triggered = 0;
    mClone = NULL;
  }


  void Update(float dt){
    MTGAbility::Update(dt);
    if (!triggered){
      triggered = 1;
      game->mLayers->actionLayer()->setMenuObject(source);
      game->mLayers->stackLayer()->setIsInterrupting(source->controller());
      OutputDebugString("ALLABILITIES SetMenuObject!\n");
    }
  }

   const char * getMenuText(){
     return ability->getMenuText();
  }

  int testDestroy(){
    if (!triggered) return 0;
    if (game->mLayers->actionLayer()->menuObject) return 0;
    if (game->mLayers->actionLayer()->getIndexOf(mClone) !=-1) return 0;
    //if (game->mLayers->actionLayer()->getIndexOf(this) !=-1) return 0;
    OutputDebugString("Destroy!\n");
    return 1;
  }

  int isReactingToTargetClick(Targetable * card){
    OutputDebugString("IsReacting ???\n");
    if (card == source) return 1;
    return 0;
  }

  int reactToTargetClick(Targetable * object){
    OutputDebugString("ReactToTargetClick!\n");
    mClone = ability->clone();
    mClone->addToGame();
    mClone->forceDestroy = 1;
    return mClone->reactToTargetClick(object);
  }

  ~MayAbility(){
    SAFE_DELETE(ability);
  }

  MayAbility * clone() const{
    MayAbility * a =  NEW MayAbility(*this);
    a->ability = ability->clone();
    a->isClone = 1;
    return a;
  }

};


//MultiAbility : triggers several actions for a cost
class MultiAbility:public ActivatedAbility{
public:
  vector<MTGAbility *> abilities;


  MultiAbility(int _id, MTGCardInstance * card,Targetable * _target, ManaCost * _cost, int _tap):ActivatedAbility(_id, card,_cost,0,_tap){
    if (_target) target = _target;
  }


  int Add(MTGAbility * ability){
    abilities.push_back(ability);
    return 1;
  }

  int resolve(){
    vector<int>::size_type sz = abilities.size();
    for (unsigned int i = 0; i < sz; i++){
      Targetable * backup =  abilities[i]->target;
      if (target && target!= source && abilities[i]->target == abilities[i]->source) abilities[i]->target = target;
      abilities[i]->resolve();
      abilities[i]->target = backup;
    }
    return 1;
  }

  ~MultiAbility(){
   if (!isClone){
      vector<int>::size_type sz = abilities.size();
      for (size_t i = 0; i < sz; i++){
        delete abilities[i];
      }
    }
    abilities.clear();
  }

   const char * getMenuText(){
     if (abilities.size()) return abilities[0]->getMenuText();
     return "";
  }

  MultiAbility * clone() const{
    MultiAbility * a =  NEW MultiAbility(*this);
    a->isClone = 1;
    return a;
  }
};


//Generic Activated Ability

class GenericActivatedAbility:public ActivatedAbility{
 public:
  MTGAbility * ability;
  int limitPerTurn;
  int counters;
  MTGGameZone * activeZone;
 GenericActivatedAbility(int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost, int _tap = 0, int limit = 0, int restrictions = 0, MTGGameZone * dest = NULL):ActivatedAbility(_id, card,_cost,restrictions,_tap),ability(a),limitPerTurn(limit),activeZone(dest){
   counters = 0;
   target = ability->target;
  }

  int resolve(){
    counters++;
    ability->target = target; //may have been updated...
    if (ability) return ability->resolve();
    return 0;
  }

  const char * getMenuText(){
    if (ability) return ability->getMenuText();
    return "Error";
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (limitPerTurn && counters >= limitPerTurn) return 0;
    return ActivatedAbility::isReactingToClick(card,mana);
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase ==Constants::MTG_PHASE_AFTER_EOT){
      counters = 0;
    }
    ActivatedAbility::Update(dt);
  }

  GenericActivatedAbility * clone() const{
    GenericActivatedAbility * a =  NEW GenericActivatedAbility(*this);
    a->cost = NEW ManaCost();
    a->cost->copy(cost);
    a->ability = ability->clone();
    return a;
  }

  ~GenericActivatedAbility(){
    SAFE_DELETE(ability);
  }

  int testDestroy(){
    if (!activeZone) return ActivatedAbility::testDestroy();
    if (activeZone->hasCard(source)) return 0;
    return 1;
    
  }

};

/* Generic TargetAbility */
class GenericTargetAbility:public TargetAbility{

public:
  int limitPerTurn;
  int counters;
  MTGGameZone * activeZone;
   GenericTargetAbility(int _id, MTGCardInstance * _source, TargetChooser * _tc,MTGAbility * a, ManaCost * _cost = NULL, int _tap=0, int limit = 0, int restrictions = 0, MTGGameZone * dest = NULL):TargetAbility(_id,_source, _tc,_cost,restrictions,_tap),limitPerTurn(limit), activeZone(dest){
    ability = a;
    counters = 0;
  }

   ~GenericTargetAbility(){
     if (isClone) SAFE_DELETE(ability);
   }

  GenericTargetAbility * clone() const{
    GenericTargetAbility * a =  NEW GenericTargetAbility(*this);
    a->ability = ability->clone();
    a->isClone = 1;
    return a;
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (limitPerTurn && counters >= limitPerTurn) return 0;
    return TargetAbility::isReactingToClick(card,mana);
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase ==Constants::MTG_PHASE_AFTER_EOT){
      counters = 0;
    }
    TargetAbility::Update(dt);
  }

  int testDestroy(){
    if (!activeZone) return TargetAbility::testDestroy();
    if (activeZone->hasCard(source)) return 0;
    return 1;
    
  }

};

//Cycling

class ACycle:public ActivatedAbility{
 public:
  ACycle(int _id, MTGCardInstance * card,Targetable * _target):ActivatedAbility(_id, card){
    target = _target;
  }

  int resolve(){
    source->controller()->game->putInGraveyard(source);
    source->controller()->game->drawFromLibrary();
    return 1;
  }

  const char * getMenuText(){
    return "Cycling";
  }

  ACycle * clone() const{
    ACycle * a =  NEW ACycle(*this);
    a->isClone = 1;
    return a;
  }


};


//Drawer, allows to draw a card for a cost:

class AADrawer:public ActivatedAbilityTP{
 public:
  WParsedInt *nbcards;
  AADrawer(int _id, MTGCardInstance * card,Targetable * _target,ManaCost * _cost, WParsedInt * _nbcards, int _tap = 0, int who=TargetChooser::UNSET):ActivatedAbilityTP(_id, card,_target,_cost,_tap,who),nbcards(_nbcards){
  }

  int resolve(){
    Targetable * _target = getTarget();
    Player * player;
    if (_target){
      if (_target->typeAsTarget() == TARGET_CARD){
        player = ((MTGCardInstance *)_target)->controller();
      }else{
        player = (Player *) _target;
      }
      game->mLayers->stackLayer()->addDraw(player,nbcards->getValue());
      game->mLayers->stackLayer()->resolve();
    }
    return 1;
  }

  const char * getMenuText(){
    return "Draw";
  }

  AADrawer * clone() const{
    AADrawer * a =  NEW AADrawer(*this);
    a->nbcards = NEW WParsedInt(*(a->nbcards));
    a->isClone = 1;
    return a;
  }

  ~AADrawer(){
    SAFE_DELETE(nbcards);
  }

};

/*Gives life to target controller*/
class AALifer:public ActivatedAbilityTP{
 public:
  WParsedInt *life;
  AALifer(int _id, MTGCardInstance * card, Targetable * _target, WParsedInt * life, ManaCost * _cost = NULL, int _tap = 0, int who = TargetChooser::UNSET):ActivatedAbilityTP(_id, card,_target,_cost,_tap,who),life(life){
  }

  int resolve(){
    Damageable * _target = (Damageable *) getTarget();
    if (_target){
      if (_target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE){
        _target = ((MTGCardInstance *)_target)->controller();
      }
      _target->life+=life->getValue();
    }
    return 1;
  }

  const char * getMenuText(){
    return "Life";
  }

  AALifer * clone() const{
    AALifer * a =  NEW AALifer(*this);
    a->life = NEW WParsedInt(*(a->life));
    a->isClone = 1;
    return a;
  }

  ~AALifer(){
    SAFE_DELETE(life);
  }

};



class ATokenCreator:public ActivatedAbility{
public:
  list<int>abilities;
  list<int>types;
  list<int>colors;
  int power, toughness;
  int tokenId;
  string name;
  WParsedInt * multiplier;
  ATokenCreator(int _id,MTGCardInstance * _source,ManaCost * _cost, int tokenId, int _doTap, WParsedInt * multiplier = NULL):ActivatedAbility(_id,_source,_cost,0,_doTap), tokenId(tokenId), multiplier(multiplier){
    if(!multiplier) this->multiplier = NEW WParsedInt(1);
  }

  ATokenCreator(int _id,MTGCardInstance * _source,ManaCost * _cost, string sname, string stypes,int _power,int _toughness, string sabilities, int _doTap, WParsedInt * multiplier = NULL):ActivatedAbility(_id,_source,_cost,0,_doTap), multiplier(multiplier){
    power = _power;
    toughness = _toughness;
    name = sname;
    tokenId = 0;
    if(!multiplier) this->multiplier = NEW WParsedInt(1);

//TODO this is a copy/past of other code that's all around the place, everything should be in a dedicated parser class;

    for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++){
      unsigned int found = sabilities.find(Constants::MTGBasicAbilities[j]);
      if (found != string::npos){
        abilities.push_back(j);
      }
    }

    for (int j = 0; j < Constants::MTG_NB_COLORS; j++){
      unsigned int found = sabilities.find(Constants::MTGColorStrings[j]);
      if (found != string::npos){
        colors.push_back(j);
      }
    }

    string s = stypes;
    while (s.size()){
      unsigned int found = s.find(" ");
      if (found != string::npos){
        int id = Subtypes::subtypesList->find(s.substr(0,found));
        types.push_back(id);
        s = s.substr(found+1);
      }else{
        int id = Subtypes::subtypesList->find(s);
        types.push_back(id);
        s = "";
      }
    }
  }

  int resolve(){
    for (int i = 0; i < multiplier->getValue(); ++i){
      MTGCardInstance * myToken;
      if (tokenId){
        MTGCard * card = GameApp::collection->getCardById(tokenId);
          myToken = NEW MTGCardInstance(card,source->controller()->game);       
      } else {
        myToken = NEW Token(name,source,power,toughness);
        list<int>::iterator it;
        for ( it=types.begin() ; it != types.end(); it++ ){
          myToken->addType(*it);
        }
        for ( it=colors.begin() ; it != colors.end(); it++ ){
          myToken->setColor(*it);
        }
        for ( it=abilities.begin() ; it != abilities.end(); it++ ){
          myToken->basicAbilities[*it] = 1;
        }
      }
      source->controller()->game->temp->addCard(myToken);
      Spell * spell = NEW Spell(myToken);
      spell->resolve();
      spell->source->isToken = 1;
      delete spell;
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ATokenCreator ::: abilities : ?" // << abilities
	<< " ; types : ?" // << types
	<< " ; colors : ?" // << colors
	<< " ; power : " << power
	<< " ; toughness : " << toughness
	<< " ; name : " << name
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }

  ATokenCreator * clone() const{
    ATokenCreator * a =  NEW ATokenCreator(*this);
    a->isClone = 1;
    return a;
  }

  ~ATokenCreator(){
    if (!isClone){
      delete(multiplier);
    }
  }

};

class AAMover:public ActivatedAbility{
public:
  string destination;
  AAMover(int _id, MTGCardInstance * _source, MTGCardInstance * _target, string dest, ManaCost * _cost=NULL, int doTap=0):ActivatedAbility(_id,_source,_cost,0,doTap),destination(dest){
    if (_target) target = _target; 
 }

  MTGGameZone * destinationZone(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    return MTGGameZone::stringToZone(destination, source,_target);
  }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(target){
      Player* p = _target->controller();
      if (p){
        GameObserver * g = GameObserver::GetInstance();
        MTGGameZone * fromZone = _target->getCurrentZone();
        MTGGameZone * destZone = destinationZone();

        //inplay is a special zone !
        for (int i=0; i < 2; i++){
          if (destZone == g->players[i]->game->inPlay && fromZone != g->players[i]->game->inPlay && fromZone != g->players[i]->opponent()->game->inPlay){
              MTGCardInstance * copy = g->players[i]->game->putInZone(_target,  fromZone, g->players[i]->game->temp);
              Spell * spell = NEW Spell(copy);
              spell->resolve();
              delete spell;
              return 1;
          }
        }
        p->game->putInZone(_target,fromZone,destZone);
        return 1;
      }
    }
    return 0;
  }

  const char * getMenuText(){
    return "Move";
  }


  AAMover * clone() const{
    AAMover * a =  NEW AAMover(*this);
    a->isClone = 1;
    return a;
  }

};


//Copier. ActivatedAbility
class AACopier:public ActivatedAbility{
 public:
 AACopier(int _id, MTGCardInstance * _source, MTGCardInstance * _target = NULL, ManaCost * _cost=NULL):ActivatedAbility(_id,_source,_cost,0,0){
    target = _target;
  }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(_target){
      source->copy(_target);
      return 1;
    }
    return 0;
  }

  const char * getMenuText(){
    return "Copy";
  }


  AACopier * clone() const{
    AACopier * a =  NEW AACopier(*this);
    a->isClone = 1;
    return a;
  }
};


class AADestroyer:public ActivatedAbility{
public:
  int bury;
 AADestroyer(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _bury = 0, ManaCost * _cost=NULL):ActivatedAbility(_id,_source,_cost),bury(_bury){
    if (_target) target = _target; 
 }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(_target){
      if (bury) return _target->bury();
      else return _target->destroy();
    }
    return 0;
  }

  const char * getMenuText(){
    return "Destroy";
  }


  AADestroyer * clone() const{
    AADestroyer * a =  NEW AADestroyer(*this);
    a->isClone = 1;
    return a;
  }


};



/*Changes one of the basic abilities of target
  source : spell
  target : spell target (creature)
  modifier : 1 to add the ability, 0 to remove it
  _ability : Id of the ability, as described in mtgdefinitions
*/
class ABasicAbilityModifier:public MTGAbility{
 public:
  int modifier;
  int ability;
  int value_before_modification;
 ABasicAbilityModifier(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _ability, int _modifier = 1): MTGAbility(_id,_source,_target),modifier(_modifier),ability(_ability){

  }

 int addToGame(){
    value_before_modification = ((MTGCardInstance * )target)->basicAbilities[ability];
    ((MTGCardInstance * )target)->basicAbilities[ability]=modifier;
    return MTGAbility::addToGame();
 }

  int destroy(){
    if (((MTGCardInstance * )target)->basicAbilities[ability] == modifier){
      ((MTGCardInstance * )target)->basicAbilities[ability] = value_before_modification;
      return 1;
    }else{
      //BUG !!!
      return 0;
    }
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ABasicAbilityModifier ::: modifier : " << modifier
	<< " ; ability : " << ability
	<< " ; value_before_modification : " << value_before_modification
	<< " (";
    return MTGAbility::toString(out) << ")";
  }

  ABasicAbilityModifier * clone() const{
    ABasicAbilityModifier * a =  NEW ABasicAbilityModifier(*this);
    a->isClone = 1;
    return a;
  }

};

//Modifies an	ability until end of turn. Needs a target
class ABasicAbilityModifierUntilEOT:public TargetAbility{
 public:
  MTGCardInstance * mTargets[50];
  int nbTargets;
  int modifier;
  int stateBeforeActivation[50];
  int ability;
 ABasicAbilityModifierUntilEOT(int _id, MTGCardInstance * _source, int _ability, ManaCost * _cost, TargetChooser * _tc = NULL, int _modifier = 1,int _tap=1): TargetAbility(_id,_source,_cost,0,_tap),modifier(_modifier), ability(_ability){
    nbTargets = 0;
    tc = _tc;
    if (!tc) tc = NEW CreatureTargetChooser(_source);
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP){
      for (int i = 0; i < nbTargets; i++){
	MTGCardInstance * mTarget = mTargets[i];
	if(mTarget && mTarget->basicAbilities[ability]){
	  mTarget->basicAbilities[ability] = stateBeforeActivation[i];
	}
      }
      nbTargets = 0;
    }
    TargetAbility::Update(dt);
  }


  int resolve(){
    MTGCardInstance * mTarget = tc->getNextCardTarget();
    if (mTarget){
      mTargets[nbTargets] = mTarget;
      stateBeforeActivation[nbTargets] = mTarget->basicAbilities[ability];
      mTarget->basicAbilities[ability] = modifier;
      nbTargets++;
    }
    return 1;
  }

  const char * getMenuText(){
    return Constants::MTGBasicAbilities[ability];
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ABasicAbilityModifierUntilEOT ::: mTargets : " << mTargets
	<< " ; nbTargets : " << nbTargets
	<< " ; modifier : " << modifier
	<< " ; stateBeforeActivation : " << stateBeforeActivation
        << " ; ability : " << ability
	<< " (";
    return TargetAbility::toString(out) << ")";
  }

  ABasicAbilityModifierUntilEOT * clone() const{
    ABasicAbilityModifierUntilEOT * a =  NEW ABasicAbilityModifierUntilEOT(*this);
    a->isClone = 1;
    return a;
  }

};

/*Instants that modifies a basic ability until end of turn */
class  AInstantBasicAbilityModifierUntilEOT: public InstantAbility{
 public:
  int stateBeforeActivation;
  int ability;
  int value;
 AInstantBasicAbilityModifierUntilEOT(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _ability, int value):InstantAbility(_id, _source, _target),ability(_ability),value(value){

  }

 int addToGame(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    stateBeforeActivation = _target->basicAbilities[ability];
    _target->basicAbilities[ability] = value;
    return InstantAbility::addToGame();
 }

  const char * getMenuText(){
    return Constants::MTGBasicAbilities[ability];
  }

  int destroy(){
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if (_target) _target->basicAbilities[ability] = stateBeforeActivation;
    return 1;
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "ABasicAbilityModifierUntilEOT ::: stateBeforeActivation : " << stateBeforeActivation
	<< " ability : " << ability
	<< " (";
    return InstantAbility::toString(out) << ")";
  }

  AInstantBasicAbilityModifierUntilEOT * clone() const{
    AInstantBasicAbilityModifierUntilEOT * a =  NEW AInstantBasicAbilityModifierUntilEOT(*this);
    a->isClone = 1;
    return a;
  }

};

//Alteration of Ability until of turn (Aura)
class ABasicAbilityAuraModifierUntilEOT: public ActivatedAbility{
 public:
  AInstantBasicAbilityModifierUntilEOT * ability;
 ABasicAbilityAuraModifierUntilEOT(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, int _ability, int _value = 1):ActivatedAbility(_id,_source, _cost, 0,0){
    target = _target;
    ability = NEW AInstantBasicAbilityModifierUntilEOT(_id,_source,_target,_ability, _value);
  }

  int resolve(){
    MTGAbility * a = ability->clone();
    a->target = target;
    a->addToGame();
    return 1;
  }

  const char * getMenuText(){
    return ability->getMenuText(); 
  }

  ABasicAbilityAuraModifierUntilEOT * clone() const{
    ABasicAbilityAuraModifierUntilEOT * a =  NEW ABasicAbilityAuraModifierUntilEOT(*this);
    a->isClone = 1;
    return a;
  }

  ~ABasicAbilityAuraModifierUntilEOT(){
    if (!isClone) SAFE_DELETE(ability);
  }
};


class AEquip:public TargetAbility{
public:
  vector<MTGAbility *> currentAbilities; 
  AEquip(int _id, MTGCardInstance * _source, ManaCost * _cost=NULL, int doTap=0, int restrictions = ActivatedAbility::AS_SORCERY):TargetAbility(_id,_source,NULL,_cost,restrictions,doTap){
    
 }

  int unequip(){
    source->target = NULL;
    for (size_t i = 0; i < currentAbilities.size(); ++i){
      MTGAbility * a = currentAbilities[i];
      if(dynamic_cast<AEquip *>(a)){
        SAFE_DELETE(a);
        continue;
      }
      GameObserver::GetInstance()->removeObserver(currentAbilities[i]);
    }
    currentAbilities.clear();
    return 1;
  }


  int equip(MTGCardInstance * equipped){
    source->target = equipped;
    AbilityFactory af;
    af.getAbilities(&currentAbilities,NULL,source);
    for (size_t i = 0; i < currentAbilities.size(); ++i){
      MTGAbility * a = currentAbilities[i];
      if(dynamic_cast<AEquip *>(a)) continue;
      a->addToGame();
    }
    return 1;

  }


  int resolve(){
    MTGCardInstance * mTarget = tc->getNextCardTarget();
    if (!mTarget) return 0;
    if (mTarget == source) return 0;
    unequip();
    equip(mTarget);
 
    return 1;
  }

  const char * getMenuText(){
    return "Equip";
  }


  int testDestroy(){
    if (source->target && !game->isInPlay(source->target)) 
      unequip();
    return TargetAbility::testDestroy();
  }

  int destroy(){
    unequip();
    return TargetAbility::destroy();
  }

  AEquip * clone() const{
    AEquip * a =  NEW AEquip(*this);
    a->isClone = 1;
    return a;
  }

};


/*Gives life each time a spell matching CardDescriptor's criteria are match . Optionnal manacost*/
class ASpellCastLife:public MTGAbility{
 public:
  CardDescriptor trigger;
  ManaCost * cost;
  int life;
  MTGCardInstance * lastUsedOn;
  MTGCardInstance * lastChecked;
 ASpellCastLife(int id, MTGCardInstance * _source, CardDescriptor  _trigger, ManaCost * _cost, int _life): MTGAbility(id, _source), trigger(_trigger), cost(_cost), life(_life){
  }
 ASpellCastLife(int id, MTGCardInstance * _source, int color, ManaCost * _cost, int _life): MTGAbility(id, _source), cost(_cost), life(_life){
    trigger.setColor(color);
  }

  int isReactingToClick(MTGCardInstance *  _card, ManaCost * mana = NULL){
    if (_card == source && game->currentlyActing()->game->inPlay->hasCard(source)){
      if (game->currentlyActing()->getManaPool()->canAfford(cost)){
	Interruptible * laststackitem = game->mLayers->stackLayer()->getAt(-1);
	if (laststackitem && laststackitem->type == ACTION_SPELL){
	  Spell * spell = (Spell*)laststackitem;
	  if (spell->source != lastUsedOn && trigger.match(spell->source)){
	    lastChecked = spell->source;
	    return 1;
	  }
	}
      }
    }
    return 0;
  }

  int reactToClick(MTGCardInstance *  _card){
    if (!isReactingToClick( _card)) return 0;
    game->currentlyActing()->getManaPool()->pay(cost);
    game->currentlyActing()->life+=life;
    lastUsedOn = lastChecked;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ASpellCastLife ::: trigger : ? " // << trigger
	<< " ; cost : " << cost
	<< " ; life : " << life
	<< " ; lastUsedOn : " << lastUsedOn
	<< " ; lastChecked : " << lastChecked
	<< " (";
    return MTGAbility::toString(out) << ")";
  }

  ASpellCastLife * clone() const{
    ASpellCastLife * a =  NEW ASpellCastLife(*this);
    a->isClone = 1;
    return a;
  }

  ~ASpellCastLife(){
    SAFE_DELETE(cost);
  }

};

//Allows to untap at any moment for an amount of mana
class AUnBlocker:public MTGAbility{
 public:
  ManaCost * cost;
 AUnBlocker(int id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost):MTGAbility(id, _source, _target), cost(_cost){
  }


  int isReactingToClick(MTGCardInstance *  _card,ManaCost * mana = NULL){
    if (_card == target && game->currentlyActing()->game->inPlay->hasCard(source) && (MTGCardInstance *) _card->isTapped()){
      if (game->currentlyActing()->getManaPool()->canAfford(cost)){
	return 1;
      }
    }
    return 0;
  }

  int reactToClick(MTGCardInstance *  _card){
    if (!isReactingToClick( _card)) return 0;
    game->currentlyActing()->getManaPool()->pay(cost);
    _card->attemptUntap();
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AUnBlocker ::: cost : " << cost
	<< " (";
    return MTGAbility::toString(out) << ")";
  }

  AUnBlocker * clone() const{
    AUnBlocker * a =  NEW AUnBlocker(*this);
    a->isClone = 1;
    return a;
  }

};

//Protection From (creature/aura)
class AProtectionFrom: public MTGAbility{
 public:
  TargetChooser * fromTc;
  AProtectionFrom(int id, MTGCardInstance * _source, MTGCardInstance * _target, TargetChooser *fromTc):MTGAbility(id,_source,_target),fromTc(fromTc){

  }

 int addToGame(){
   MTGCardInstance * _target = (MTGCardInstance *)target;
   _target->addProtection(fromTc);
   return MTGAbility::addToGame();
 }

  int destroy(){
    ((MTGCardInstance *)target)->removeProtection(fromTc);
    return 1;
  }

  AProtectionFrom * clone() const{
    AProtectionFrom * a =  NEW AProtectionFrom(*this);
    a->isClone = 1;
    return a;
  }

  ~AProtectionFrom(){
    SAFE_DELETE(fromTc);
  }

};


//Alteration of Power and Toughness  (enchantments)
class APowerToughnessModifier: public MTGAbility{
 public:
  WParsedPT * wppt;
 APowerToughnessModifier(int id, MTGCardInstance * _source, MTGCardInstance * _target, WParsedPT * wppt):MTGAbility(id,_source,_target),wppt(wppt){

  }

 int addToGame(){
   MTGCardInstance * _target = (MTGCardInstance *)target;
   _target->power += wppt->power.getValue();
    _target->addToToughness(wppt->toughness.getValue());
    return MTGAbility::addToGame();
 }

  int destroy(){
    ((MTGCardInstance *)target)->power -= wppt->power.getValue();
    ((MTGCardInstance *)target)->addToToughness(- wppt->toughness.getValue());
    return 1;
  }

  APowerToughnessModifier * clone() const{
    APowerToughnessModifier * a =  NEW APowerToughnessModifier(*this);
    a->wppt = NEW WParsedPT(*(a->wppt));
    a->isClone = 1;
    return a;
  }

  ~APowerToughnessModifier(){
    delete(wppt);
  }

};


//Alteration of Power and Toughness until end of turn (TargetAbility)
// Gives +n/+m until end of turn to any card that's a target
class ATargetterPowerToughnessModifierUntilEOT: public TargetAbility{
 public:
  MTGCardInstance * mTargets[50];
  int nbTargets;
  WParsedPT * wppt;

 ATargetterPowerToughnessModifierUntilEOT(int _id, MTGCardInstance * _source, WParsedPT * wppt,  ManaCost * _cost, TargetChooser * _tc = NULL, int doTap=1):TargetAbility(_id,_source,_tc,_cost,0,doTap),wppt(wppt){
    if (!tc) tc = NEW CreatureTargetChooser(_source);
    nbTargets = 0;
  }


  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP){
      for (int i = 0; i < nbTargets; i++){
	MTGCardInstance * mTarget = mTargets[i];
	if(mTarget){
	  mTarget->power-=wppt->power.getValue();
	  mTarget->addToToughness(-wppt->toughness.getValue());
	}
      }
      nbTargets = 0;
    }
    TargetAbility::Update(dt);
  }


  int resolve(){
    MTGCardInstance * mTarget = tc->getNextCardTarget();
    if (mTarget){
      mTargets[nbTargets] = mTarget;
      mTarget->power+= wppt->power.getValue();
      mTarget->addToToughness(wppt->toughness.getValue());
      nbTargets++;
    }
    return 1;
  }

  ATargetterPowerToughnessModifierUntilEOT * clone() const{
    ATargetterPowerToughnessModifierUntilEOT * a =  NEW ATargetterPowerToughnessModifierUntilEOT(*this);
    a->wppt = NEW WParsedPT(*(a->wppt));
    a->isClone = 1;
    return a;
  }

  ~ATargetterPowerToughnessModifierUntilEOT(){
    delete(wppt);
  }
};

//Alteration of Power and toughness until end of turn (instant)
class  AInstantPowerToughnessModifierUntilEOT: public InstantAbility{
 public:
  WParsedPT * wppt;
 AInstantPowerToughnessModifierUntilEOT(int _id, MTGCardInstance * _source, MTGCardInstance * _target, WParsedPT * wppt): InstantAbility(_id, _source, _target), wppt(wppt){
  }

  int resolve(){
    ((MTGCardInstance *)target)->power +=wppt->power.getValue();
    ((MTGCardInstance *)target)->addToToughness(wppt->toughness.getValue());
    return 1;
  }

  int destroy(){
    ((MTGCardInstance *)target)->power -=wppt->power.getValue();
    ((MTGCardInstance *)target)->addToToughness(-wppt->toughness.getValue());
    return 1;
  }

  const char * getMenuText(){
    sprintf(menuText, "%i/%i",wppt->power.getValue(),wppt->toughness.getValue());
    return menuText;
  }


  AInstantPowerToughnessModifierUntilEOT * clone() const{
    AInstantPowerToughnessModifierUntilEOT * a =  NEW AInstantPowerToughnessModifierUntilEOT(*this);
    a->wppt = NEW WParsedPT(*(a->wppt));
    a->isClone = 1;
    return a;
  }

  ~AInstantPowerToughnessModifierUntilEOT(){
    delete wppt;
  }
};



//Alteration of Power and Toughness until end of turn (Aura)
class APowerToughnessModifierUntilEndOfTurn: public ActivatedAbility{
 public:
  AInstantPowerToughnessModifierUntilEOT * ability;
  int counters;
  int maxcounters;
 APowerToughnessModifierUntilEndOfTurn(int id, MTGCardInstance * _source, MTGCardInstance * _target, WParsedPT * wppt,  ManaCost * _cost = NULL, int _maxcounters = 0):ActivatedAbility(id,_source,_cost,0,0),maxcounters(_maxcounters){
    counters = 0;
    target=_target;
    ability = NEW AInstantPowerToughnessModifierUntilEOT(id,_source,_target,wppt);
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_AFTER_EOT){
      counters = 0;
    }
    ActivatedAbility::Update(dt);
  }

  int fireAbility(){
    return resolve();
  }

  const char * getMenuText(){
    return ability->getMenuText();
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (!ActivatedAbility::isReactingToClick(card,mana)) return 0;
    return (!maxcounters || (counters < maxcounters));
  }

  int resolve(){
    MTGAbility * a = ability->clone();
    a->target = target;
    a->addToGame();
    counters++;
    return 1;
  }

  APowerToughnessModifierUntilEndOfTurn * clone() const{
    APowerToughnessModifierUntilEndOfTurn * a =  NEW APowerToughnessModifierUntilEndOfTurn(*this);
    a->isClone = 1;
    return a;
  }

  ~APowerToughnessModifierUntilEndOfTurn(){
    if (!isClone) SAFE_DELETE(ability);
  }
};


class  GenericInstantAbility: public InstantAbility{
 public:
  MTGAbility * ability;
 GenericInstantAbility(int _id, MTGCardInstance * _source, Damageable * _target, MTGAbility * ability): InstantAbility(_id, _source, _target), ability(ability){
  }

 int addToGame(){
   ability->forceDestroy = -1;
   ability->addToGame();
   return InstantAbility::addToGame();
 }

 int destroy(){
   ability->forceDestroy = 0;
   return InstantAbility::destroy();
 }

  GenericInstantAbility * clone() const{
   GenericInstantAbility * a =  NEW GenericInstantAbility(*this);
    a->isClone = 1;
    return a;
  }

};


//Untap Blockers with simple Mana Mechanism
class AUntapManaBlocker: public UntapBlocker{
 public:
 AUntapManaBlocker(int id, MTGCardInstance * card, ManaCost * _cost):UntapBlocker(id, card, _cost){
  }

 AUntapManaBlocker(int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost):UntapBlocker(id, card,_target, _cost){
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "AUntapManaBlocker ::: (";
    return UntapBlocker::toString(out) << ")";
  }
  AUntapManaBlocker * clone() const{
    AUntapManaBlocker * a =  NEW AUntapManaBlocker(*this);
    a->isClone = 1;
    return a;
  }
};


//Circle of Protections
class ACircleOfProtection: public TargetAbility{
protected:
  map<ReplacementEffect*, int> current;
 public:
 ACircleOfProtection(int _id, MTGCardInstance * source, int _color):TargetAbility(_id,source,NEW SpellOrPermanentTargetChooser(source,_color),NEW ManaCost(),0,0){
    cost->add(Constants::MTG_COLOR_ARTIFACT,1);
    tc->targetter = NULL; //Circle of Protection doesn't use the word "source"
  }

  int resolve(){
    MTGCardInstance * _target = NULL;
    if (! (_target = tc->getNextCardTarget())){
      Spell * starget = tc->getNextSpellTarget();
      _target = starget->source;
    }
    if (!_target) return 0;
    REDamagePrevention * re = NEW REDamagePrevention (
      this,
      NEW CardTargetChooser(_target,NULL), 
      NEW PlayerTargetChooser(0,1,source->controller()));
    current[re] = 1;
    game->replacementEffects->add(re);
    return 1;
  }

  void clear(){
    for (map<ReplacementEffect*, int>::iterator it = current.begin(); it!=current.end(); it++){
      ReplacementEffect* re = (*it).first;
      game->replacementEffects->remove(re);
      delete re;
    }
    current.clear();
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP) clear();
    TargetAbility::Update(dt);
  }

  ~ACircleOfProtection(){
    clear();
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ACircleOfProtection ::: (";
    return TargetAbility::toString(out) << ")";
  }
  ACircleOfProtection * clone() const{
    ACircleOfProtection * a =  NEW ACircleOfProtection(*this);
    a->isClone = 1;
    return a;
  }
};


//Basic regeneration mechanism for a Mana cost
class AStandardRegenerate:public ActivatedAbility{
 public:
 AStandardRegenerate(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL):ActivatedAbility(_id,_source,_cost,0,0){
    target = _target;
    aType = MTGAbility::STANDARD_REGENERATE;
  }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *)target;
    _target->regenerate();
    return 1;
  }

  const char * getMenuText(){
    return "Regenerate";
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AStandardRegenerate ::: (";
    return ActivatedAbility::toString(out) << ")";
  }
  AStandardRegenerate * clone() const{
    AStandardRegenerate * a =  NEW AStandardRegenerate(*this);
    a->isClone = 1;
    return a;
  }
};


//Aura Enchantments that provide controller of target life or damages at a given phase of their turn
class ARegularLifeModifierAura:public MTGAbility{
 public:
  int life;
  int phase;
  int onlyIfTargetTapped;
 ARegularLifeModifierAura(int id, MTGCardInstance * _source, MTGCardInstance * _target, int _phase, int _life, int _onlyIfTargetTapped=0):MTGAbility(id,_source,_target),life(_life), phase(_phase),onlyIfTargetTapped(_onlyIfTargetTapped){
  }

  void Update(float dt){
    if (newPhase !=currentPhase && newPhase==phase && game->currentPlayer==((MTGCardInstance *)target)->controller()){
      if (!onlyIfTargetTapped || ((MTGCardInstance *)target)->isTapped()){
	if (life > 0){
	  game->currentPlayer->life+=life;
	}else{
	  game->mLayers->stackLayer()->addDamage(source, game->currentPlayer, -life);
	}
      }
    }
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "ARegularLifeModifierAura ::: life : " << life
	<< " ; phase : " << phase
	<< " ; onlyIfTargetTapped : " << onlyIfTargetTapped
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  ARegularLifeModifierAura * clone() const{
    ARegularLifeModifierAura * a =  NEW ARegularLifeModifierAura(*this);
    a->isClone = 1;
    return a;
  }
};


//ExaltedAbility (Shards of Alara)
class AExalted:public ListMaintainerAbility{
 public:
  int power, toughness;
  MTGCardInstance * luckyWinner;
 AExalted(int _id, MTGCardInstance * _source, int _power = 1, int _toughness = 1):ListMaintainerAbility(_id, _source),power(_power),toughness(_toughness){
    luckyWinner = NULL;
  }


  int canBeInList(MTGCardInstance * card){
    if (card->isAttacker() && game->currentPlayer == source->controller() && game->isInPlay(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
    if(cards.size() == 1){
      luckyWinner = cards.begin()->first;
      luckyWinner->addToToughness(toughness);
      luckyWinner->power+=power;
    }else if (cards.size() == 2){
      luckyWinner->addToToughness(-toughness);
      luckyWinner->power-=power;
    }
    return 1;
  }

  int removed(MTGCardInstance * card){
    if(cards.size() == 1){
      luckyWinner = cards.begin()->first;
      luckyWinner->addToToughness(toughness);
      luckyWinner->power+=power;
    }else if (cards.size() == 0){
      luckyWinner->addToToughness(-toughness);
      luckyWinner->power-=power;
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AExalted ::: power : " << power
	<< " ; toughness : " << toughness
	<< " (";
    return ListMaintainerAbility::toString(out) << ")";
  }
  AExalted * clone() const{
    AExalted * a =  NEW AExalted(*this);
    a->isClone = 1;
    return a;
  }
};


//ExaltedAbility for basic abilities (Shards of Alara)
class AExaltedAbility:public ListMaintainerAbility{
 public:
  int ability;
  MTGCardInstance * luckyWinner;
 AExaltedAbility(int _id, MTGCardInstance * _source, int _ability):ListMaintainerAbility(_id, _source),ability(_ability){
    luckyWinner = NULL;
  }


  int canBeInList(MTGCardInstance * card){
    if (card->isAttacker() && game->currentPlayer == source->controller() && game->isInPlay(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
    luckyWinner = cards.begin()->first;
    if(cards.size() == 1){
      luckyWinner->basicAbilities[ability]+=1;
    }else if (cards.size() == 2){
      luckyWinner->basicAbilities[ability]-=1;
    }
    return 1;
  }

  int removed(MTGCardInstance * card){
    if(cards.size() == 1){
      luckyWinner->basicAbilities[ability]+=1;
    }else if (cards.size() == 0){
      luckyWinner->basicAbilities[ability]-=1;
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AExaltedAbility ::: ability : " << ability
	<< " ; luckyWinner : " << luckyWinner
	<< " (";
    return ListMaintainerAbility::toString(out) << ")";
  }
  AExaltedAbility * clone() const{
    AExaltedAbility * a =  NEW AExaltedAbility(*this);
    a->isClone = 1;
    return a;
  }
};


//Converts lands to creatures (Kormus bell, Living lands)
class AConvertLandToCreatures:public ListMaintainerAbility{
 public:
  int type;
  int power, toughness;
 AConvertLandToCreatures(int _id, MTGCardInstance * _source, const char * _type, int _power = 1, int _toughness = 1):ListMaintainerAbility(_id, _source),power(_power),toughness(_toughness){
    type = Subtypes::subtypesList->find(_type);
  }


  int canBeInList(MTGCardInstance * card){
    if (card->hasType(type) && game->isInPlay(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
    card->power = 1;
    card->setToughness(1);
    card->setSubtype("creature");
    return 1;
  }

  int removed(MTGCardInstance * card){
    card->removeType("creature");
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AConvertLandToCreatures ::: power : " << power
	<< " ; toughness : " << toughness
	<< " ; type : " << type
	<< " (";
    return ListMaintainerAbility::toString(out) << ")";
  }
  AConvertLandToCreatures * clone() const{
    AConvertLandToCreatures * a =  NEW AConvertLandToCreatures(*this);
    a->isClone = 1;
    return a;
  }
};

//Generic Kird Ape
class AAsLongAs:public ListMaintainerAbility{
 public:
   MTGAbility * ability;
   MTGAbility * a;
  int includeSelf;
  int mini,maxi;
 AAsLongAs(int _id, MTGCardInstance * _source, Damageable * _target, TargetChooser * _tc, int _includeSelf, MTGAbility * ability,int mini = 0, int maxi = 0):ListMaintainerAbility(_id, _source,_target),ability(ability),mini(mini),maxi(maxi){
    tc = _tc;
    includeSelf = _includeSelf;
    tc->targetter  = NULL;
    ability->source = source;
    ability->target = target;
    a = NULL;
 }

 int canBeInList(MTGCardInstance * card){
   if ((includeSelf || card!=source) && tc->canTarget(card)) return 1;
   return 0;
 }

  int resolve(){
    //TODO check if ability is oneShot ?
    updateTargets();
    cards.clear();
    players.clear();
    return 1;
  }

  int addAbilityToGame(){
    if (a) return 0;
    a = ability->clone();
    if (a->oneShot){
      a->resolve();
      SAFE_DELETE(a);
    }else{
      a->addToGame();
    }
    return 1;    
  }

  int removeAbilityFromGame(){
     if (!a) return 0;
     game->removeObserver(a);
     a = NULL;
     return 1;
  }

  int _added(Damageable * d){
    size_t size = cards.size();
    if (mini && (int)size <= mini) return 0;
    if (maxi && (int)size >= maxi) return removeAbilityFromGame();
    if (!mini && !maxi && size !=1) return 0;
    return addAbilityToGame();
  }

  int added(MTGCardInstance * card){
   return  _added(card);
  }

  int added(Player * p){
    return _added(p);
  }


 int removed(MTGCardInstance * card){
   size_t size = cards.size();
  if (mini && (int)size <= mini) return removeAbilityFromGame();
  if (maxi && (int)size == maxi-1) return addAbilityToGame();
  if (!mini && !maxi && size !=0) return 0;
  return removeAbilityFromGame();
 }

  ~AAsLongAs(){
    if (!isClone) SAFE_DELETE(ability);
  }


  AAsLongAs * clone() const{
    AAsLongAs * a =  NEW AAsLongAs(*this);
    a->isClone = 1;
    return a;
  }
};

//Lords (Merfolk lord...) give power and toughness to OTHER creatures of their type, they can give them special abilities, regeneration
class ALord:public ListMaintainerAbility{
 public:
   MTGAbility * ability;
  int includeSelf;
  map<Damageable *, MTGAbility *> abilities;

 ALord(int _id, MTGCardInstance * card, TargetChooser * _tc, int _includeSelf, MTGAbility * a):ListMaintainerAbility(_id,card), ability(a){
    tc = _tc;
    tc->targetter = NULL;
    includeSelf = _includeSelf;
  }

  int canBeInList(Player *p){
    if (tc->canTarget(p)) return 1;
    return 0;
  }

  int canBeInList(MTGCardInstance * card){
    if ( (includeSelf || card!=source) && tc->canTarget(card)) return 1;
    return 0;
  }

  int resolve(){
    //TODO check if ability is oneShot ?
    updateTargets();
    cards.clear();
    players.clear();
    return 1;
  }
  
  int _added(Damageable * d){
      MTGAbility * a = ability->clone();
      a->target = d;
      if (a->oneShot){
        a->resolve();
        delete(a);
      }else{
        if (d->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE){
          a->source = (MTGCardInstance *)d;
        }
        if (oneShot){
          MTGAbility * wrapper = NEW GenericInstantAbility(1,source,d,a);
          wrapper->addToGame();
        }else{
          a->addToGame();
          abilities[d] = a;
        }
      }
      return 1;
  }

  int added(MTGCardInstance * card){
   return  _added(card);
  }

  int added(Player * p){
    return _added(p);
  }

  int removed(MTGCardInstance * card){
    if(abilities.find(card) != abilities.end()){
      game->removeObserver(abilities[card]);
      abilities.erase(card);
    }
    return 1;
  }

  ~ALord(){
    if (!isClone) SAFE_DELETE(ability);
  }

  ALord * clone() const{
    ALord * a =  NEW ALord(*this);
    a->isClone = 1;
    return a;
  }
};


//Foreach (plague rats...)
class AForeach:public ListMaintainerAbility{
 public:
   MTGAbility * ability;
  int includeSelf;
  int mini;
  int maxi;
  map<Damageable *, MTGAbility *> abilities;
 AForeach(int _id, MTGCardInstance * card,Damageable  * _target, TargetChooser * _tc, int _includeSelf, MTGAbility * a, int mini = 0, int maxi = 0):ListMaintainerAbility(_id,card,_target), ability(a),mini(mini),maxi(maxi){
    tc = _tc;
    tc->targetter = NULL;
    includeSelf = _includeSelf;
    ability->target = _target;
  }

  int canBeInList(MTGCardInstance * card){
    if ( (includeSelf || card!=source) && tc->canTarget(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
      if (mini && cards.size() <= (size_t)mini) return 0;
      if (maxi && cards.size() >= (size_t)maxi) return 0;

      MTGAbility * a = ability->clone();
      a->target = target;
      if (a->oneShot){
        a->resolve();
        delete(a);
      }else{
        a->addToGame();
        abilities[card] = a;
      }
      return 1;
  }

  int removed(MTGCardInstance * card){
    if(abilities.find(card) != abilities.end()){
      game->removeObserver(abilities[card]);
      abilities.erase(card);
      return 1;
    }
    return 0;
  }

  AForeach * clone() const{
    AForeach * a =  NEW AForeach(*this);
    a->isClone = 1;
    return a;
  }

  int resolve(){
    //TODO check if ability is oneShot ?
    updateTargets();
    cards.clear();
    players.clear();
    return 1;
  }
  

  ~AForeach(){
    if (!isClone) SAFE_DELETE(ability);
  }

};




class AADamager:public ActivatedAbilityTP{
public:
  WParsedInt * damage;
AADamager(int _id, MTGCardInstance * _source, Targetable * _target, WParsedInt * damage, ManaCost * _cost=NULL, int doTap = 0, int who = TargetChooser::UNSET):ActivatedAbilityTP(_id,_source,_target,_cost,doTap,who),damage(damage){
    aType = MTGAbility::DAMAGER;
 }

  int resolve(){
    if(target){
      Damageable * _target = (Damageable *) getTarget();     
      game->mLayers->stackLayer()->addDamage(source,_target, damage->getValue());
      game->mLayers->stackLayer()->resolve();
      return 1;
    }
    return 0;
  }

  const char * getMenuText(){
    return "Damage";
  }

  AADamager * clone() const{
    AADamager * a =  NEW AADamager(*this);
    a->damage = NEW WParsedInt(*(a->damage));
    a->isClone = 1;
    return a;
  }

  ~AADamager(){
    SAFE_DELETE(damage);
  }


};

/* Standard Damager, can choose a NEW target each time the price is paid */
class TADamager:public TargetAbility{
 public:

 TADamager(int id, MTGCardInstance * card, ManaCost * _cost, WParsedInt * damage, TargetChooser * _tc = NULL, int _tap = 0):TargetAbility(id,card, _tc, _cost,0,_tap){
    if (!tc) tc = NEW DamageableTargetChooser(card);
    ability = NEW AADamager(id,card,NULL,damage);
  }

  TADamager * clone() const{
    TADamager * a =  NEW TADamager(*this);
    a->isClone = 1;
    return a;
  }
};

/* Can tap a target for a cost */
class AATapper:public ActivatedAbility{
 public:
 AATapper(int id, MTGCardInstance * card, MTGCardInstance * _target,ManaCost * _cost = NULL, int doTap = 0):ActivatedAbility(id,card, _cost,0,doTap){
   target = _target;
  }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target){
      while (_target->next) _target=_target->next; //This is for cards such as rampant growth
      _target->tap();
    }
    return 1;
  }

  const char * getMenuText(){
    return "Tap";
  }

  AATapper * clone() const{
    AATapper * a =  NEW AATapper(*this);
    a->isClone = 1;
    return a;
  }
};


/* Can untap a target for a cost */
class AAUntapper:public ActivatedAbility{
 public:
 AAUntapper(int id, MTGCardInstance * card, MTGCardInstance * _target,ManaCost * _cost = NULL, int doTap = 0):ActivatedAbility(id,card, _cost,0,doTap){
   target = _target;
  }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target){
      while (_target->next) _target=_target->next; //This is for cards such as rampant growth
      _target->untap();
    }
    return 1;
  }

  const char * getMenuText(){
    return "Untap";
  }

  AAUntapper * clone() const{
    AAUntapper * a =  NEW AAUntapper(*this);
    a->isClone = 1;
    return a;
  }
};


// Add life of gives damage if a given zone has more or less than [condition] cards at the beginning of [phase]
//Ex : the rack, ivory tower...
class ALifeZoneLink:public MTGAbility{
 public:
  int phase;
  int condition;
  int life;
  int controller;
  int nbcards;
  MTGGameZone * zone;
 ALifeZoneLink(int _id ,MTGCardInstance * card, int _phase, int _condition, int _life = -1, int _controller = 0, MTGGameZone * _zone = NULL):MTGAbility(_id, card){
    phase = _phase;
    condition = _condition;
    controller = _controller;
    life = _life;
    zone = _zone;
    if (zone == NULL){
      if (controller){
	zone = game->currentPlayer->game->hand;
      }else{
	zone = game->opponent()->game->hand;
      }
    }
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == phase){
      if ((controller && game->currentPlayer == source->controller()) ||(!controller && game->currentPlayer != source->controller()) ){
	if ((condition < 0 && zone->nb_cards < - condition) ||(condition >0 && zone->nb_cards > condition)){
	  int diff = zone->nb_cards - condition;
	  if (condition < 0) diff = - condition - zone->nb_cards;
	  if (life > 0){
	    game->currentPlayer->life+=life*diff;
	  }else{
	    game->mLayers->stackLayer()->addDamage(source,game->currentPlayer,-life*diff);
	  }
	}
      }
    }
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "ALifeZoneLink ::: phase : " << phase
	<< " ; condition : " << condition
	<< " ; life : " << life
	<< " ; controller : " << controller
	<< " ; nbcards : " << nbcards
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  ALifeZoneLink * clone() const{
    ALifeZoneLink * a =  NEW ALifeZoneLink(*this);
    a->isClone = 1;
    return a;
  }
};

//Creatures that cannot attack if opponent has not a given type of land, and die if controller has not this type of land
//Ex : pirate ship...
class AStrongLandLinkCreature: public MTGAbility{
 public:
  char land[20];
 AStrongLandLinkCreature(int _id, MTGCardInstance * _source, const char * _land):MTGAbility(_id, _source){
    sprintf(land,"%s",_land);
  }

  void Update(float dt){
    if (source->isAttacker()){
      if (!game->opponent()->game->inPlay->hasType(land)){
        source->toggleAttacker();
	//TODO Improve, there can be race conditions here
      }
    }
    Player * player = source->controller();
    if(!player->game->inPlay->hasType(land)){
      player->game->putInGraveyard(source);
    }
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "AStrongLandLinkCreature ::: land : " << land
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  AStrongLandLinkCreature * clone() const{
    AStrongLandLinkCreature * a =  NEW AStrongLandLinkCreature(*this);
    a->isClone = 1;
    return a;
  }
};

//Steal control of a target
class AControlStealAura: public MTGAbility{
 public:
  Player  * originalController;
 AControlStealAura(int _id , MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source, _target){
    originalController = _target->controller();
    MTGCardInstance * copy = _target->changeController(game->currentlyActing());
    target = copy;
    source->target = copy;
  }

  int destroy(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    Player * p = _target->controller();
    if (p && p->game->inPlay->hasCard(_target)){ //if the target is still in game -> spell was destroyed
      _target->changeController(originalController);
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AControlStealAura ::: originalController : " << originalController
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  AControlStealAura * clone() const{
    AControlStealAura * a =  NEW AControlStealAura(*this);
    a->isClone = 1;
    return a;
  }
};

//Creatures that kill their blockers
//Ex : Cockatrice
class AOldSchoolDeathtouch:public MTGAbility{
 public:
  MTGCardInstance * opponents[20];
  int nbOpponents;
 AOldSchoolDeathtouch(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
    nbOpponents = 0;
  }

  void Update(float dt){
    if (newPhase != currentPhase){
      if( newPhase == Constants::MTG_PHASE_COMBATDAMAGE){
	nbOpponents = 0;
	MTGCardInstance * opponent = source->getNextOpponent();
	while (opponent && !opponent->hasSubtype("wall")){
	  opponents[nbOpponents] = opponent;
	  nbOpponents ++;
	  opponent = source->getNextOpponent(opponent);
	}
      }else if (newPhase == Constants::MTG_PHASE_COMBATEND){
	for (int i = 0; i < nbOpponents ; i++){
	  game->mLayers->stackLayer()->addPutInGraveyard(opponents[i]);
	}
      }
    }
  }

  int testDestroy(){
    if(!game->isInPlay(source) && currentPhase != Constants::MTG_PHASE_UNTAP){
      return 0;
    }else{
      return MTGAbility::testDestroy();
    }
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AOldSchoolDeathtouch ::: opponents : " << opponents
	<< " ; nbOpponents : " << nbOpponents
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  AOldSchoolDeathtouch * clone() const{
    AOldSchoolDeathtouch * a =  NEW AOldSchoolDeathtouch(*this);
    a->isClone = 1;
    return a;
  }
};

//Adds types/abilities/P/T to a card (aura)
class ABecomes:public MTGAbility{
public:
  list<int>abilities;
  list<int>types;
  list<int>colors;
  WParsedPT * wppt;
  ABecomes(int id, MTGCardInstance * source, MTGCardInstance * target, string stypes, WParsedPT * wppt, string sabilities):MTGAbility(id,source,target),wppt(wppt){
    //TODO this is a copy/past of other code that's all around the place, everything should be in a dedicated parser class;

    for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++){
      unsigned int found = sabilities.find(Constants::MTGBasicAbilities[j]);
      if (found != string::npos){
        abilities.push_back(j);
      }
    }

    for (int j = 0; j < Constants::MTG_NB_COLORS; j++){
      unsigned int found = sabilities.find(Constants::MTGColorStrings[j]);
      if (found != string::npos){
        colors.push_back(j);
      }
    }

    string s = stypes;
    while (s.size()){
      unsigned int found = s.find(" ");
      if (found != string::npos){
        int id = Subtypes::subtypesList->find(s.substr(0,found));
        types.push_back(id);
        s = s.substr(found+1);
      }else{
        int id = Subtypes::subtypesList->find(s);
        types.push_back(id);
        s = "";
      }
    }
  }

 int addToGame(){
   MTGCardInstance * _target = (MTGCardInstance *)target;
      list<int>::iterator it;
    for ( it=types.begin() ; it != types.end(); it++ ){
      _target->addType(*it);
    }
    for ( it=colors.begin() ; it != colors.end(); it++ ){
      _target->setColor(*it);
    }
    for ( it=abilities.begin() ; it != abilities.end(); it++ ){
      _target->basicAbilities[*it]++;
    }

    if (wppt){
      _target->power = wppt->power.getValue();
      _target->toughness = wppt->toughness.getValue();
      _target->life = _target->toughness;
    }
    return MTGAbility::addToGame();
 }

  int destroy(){
   MTGCardInstance * _target = (MTGCardInstance *)target;
      list<int>::iterator it;
    for ( it=types.begin() ; it != types.end(); it++ ){
      _target->removeType(*it);
    }
    for ( it=colors.begin() ; it != colors.end(); it++ ){
      _target->removeColor(*it);
    }
    for ( it=abilities.begin() ; it != abilities.end(); it++ ){
      _target->basicAbilities[*it]--;
    }
    return 1;
  }

  ABecomes * clone() const{
    ABecomes * a =  NEW ABecomes(*this);
    a->wppt = NEW WParsedPT(*(a->wppt));
    a->isClone = 1;
    return a;
  }

  ~ABecomes(){
    delete(wppt);
  }

};


//Adds types/abilities/P/T to a card (until end of turn)
class  ABecomesUEOT: public InstantAbility{
public:
  ABecomes * ability;
   ABecomesUEOT(int id, MTGCardInstance * source, MTGCardInstance * target, string types, WParsedPT * wpt, string abilities):InstantAbility(id,source,target){
     ability = NEW ABecomes(id,source,target,types,wpt,abilities);
   }
 
  int resolve(){
    ability->addToGame();
    return 1;
  }

  int destroy(){
    ability->destroy();
    return 1;
  }

  const char * getMenuText(){
    return ability->getMenuText();
  }


  ABecomesUEOT * clone() const{
    ABecomesUEOT * a =  NEW ABecomesUEOT(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
  }

  ~ABecomesUEOT(){
    delete ability;
  }

};


class APreventAllCombatDamage:public MTGAbility{
  public:
  string to, from;
  REDamagePrevention * re;

  APreventAllCombatDamage(int id,MTGCardInstance *  source,string to,string from):MTGAbility(id,source),to(to),from(from){
     re = NULL;
  }

  int addToGame(){  
    if (re) {
      OutputDebugString("FATAL:re shouldn't be already set in APreventAllCombatDAMAGE\n");
      return 0;
    }
    TargetChooserFactory tcf;
    TargetChooser *toTc = tcf.createTargetChooser(to,source,this);
    if (toTc) toTc->targetter = NULL;
    TargetChooser *fromTc = tcf.createTargetChooser(from,source,this);
    if (fromTc) fromTc->targetter = NULL;
    re = NEW REDamagePrevention (this, fromTc, toTc, -1, false, DAMAGE_COMBAT);
    game->replacementEffects->add(re);
    return MTGAbility::addToGame();
  }

  int destroy(){
    game->replacementEffects->remove(re);
    SAFE_DELETE(re);
    return 1;
  }

  APreventAllCombatDamage * clone() const{
    APreventAllCombatDamage * a =  NEW APreventAllCombatDamage(*this);
    a->isClone = 1;
    return a;
  }

};

//Adds types/abilities/P/T to a card (until end of turn)
class  APreventAllCombatDamageUEOT: public InstantAbility{
public:
  APreventAllCombatDamage * ability;
  vector<APreventAllCombatDamage *> clones;
   APreventAllCombatDamageUEOT(int id,MTGCardInstance *  source,string to, string from):InstantAbility(id,source){
     ability = NEW APreventAllCombatDamage(id,source,to, from);
   }
 
  int resolve(){
    APreventAllCombatDamage * a = ability->clone();
    a->target = this->target;
    a->forceDestroy = -1; //Prevent the effect from getting destroyed because its source is not inplay
    a->addToGame();
    clones.push_back(a);
    return 1;
  }

  int destroy(){
    for (size_t i = 0; i < clones.size(); ++i){
      clones[i]->forceDestroy = 0;
    }
    clones.clear();
    return 1;
  }

  const char * getMenuText(){
    return ability->getMenuText();
  }


  APreventAllCombatDamageUEOT * clone() const{
    APreventAllCombatDamageUEOT * a =  NEW APreventAllCombatDamageUEOT(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
  }

  ~APreventAllCombatDamageUEOT(){
    delete ability;
  }

};


/*
  Specific Classes
*/

// 1092 Specific to Aladdin's Lamp
class AAladdinsLamp: public TargetAbility{
 public:
  CardDisplay cd;
  int nbcards;
  int init;

 AAladdinsLamp(int id, MTGCardInstance * card) : TargetAbility(id,card) {
    cost = NEW ManaCost();
    cost->x();
    cd = CardDisplay(1, game, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, NULL);
    int zones[] = {MTGGameZone::MY_LIBRARY};
    tc = NEW TargetZoneChooser(zones,1,source);
    nbcards = 0;
    init = 0;
  }

  void Update(float dt){
    if (waitingForAnswer){
      if (!init){
	cd.resetObjects();
	int wished = game->currentlyActing()->getManaPool()->getConvertedCost();
	game->currentlyActing()->getManaPool()->pay(cost);
	nbcards = 0;
	MTGGameZone * library = game->currentlyActing()->game->library;
	while (nbcards < wished && nbcards < library->nb_cards){
	  cd.AddCard(library->cards[library->nb_cards - 1 - nbcards]);
	  nbcards++;
	}
	init = 1;
      }
      cd.Update(dt);
      //      cd.CheckUserInput(dt);
    }
  }

  void Render(float dt){
    if (waitingForAnswer){
      cd.Render();
    }
  }


  int fireAbility(){
    source->tap();
    MTGLibrary * library = game->currentlyActing()->game->library;
    MTGCardInstance * card = library->removeCard(tc->getNextCardTarget());
    library->shuffleTopToBottom(nbcards - 1 );
    library->addCard(card);
    init = 0;
    return 1;
  }

  int resolve(){return 1;};

  virtual ostream& toString(ostream& out) const
  {
    out << "AAladdinsLamp ::: cd : " << cd
	<< " ; nbcards  : " << nbcards
	<< " ; init : " << init
	<< " (";
    return TargetAbility::toString(out) << ")";
  }
  AAladdinsLamp * clone() const{
    AAladdinsLamp * a =  NEW AAladdinsLamp(*this);
    a->isClone = 1;
    return a;
  }
};


// Armageddon Clock
class AArmageddonClock:public MTGAbility{
 public:
  int counters;
  ManaCost cost;
 AArmageddonClock(int id, MTGCardInstance * _source):MTGAbility(id, _source){
    counters = 0;
    int _cost[] = {Constants::MTG_COLOR_ARTIFACT, 4};
    cost = ManaCost(_cost,1);
  }

  void Update(float dt){
    if (newPhase != currentPhase){
      if (newPhase == Constants::MTG_PHASE_UPKEEP && game->currentPlayer->game->inPlay->hasCard(source)){
	counters ++;
      }else if (newPhase == Constants::MTG_PHASE_DRAW && counters > 0 && game->currentPlayer->game->inPlay->hasCard(source)){ //End of upkeep = beginning of draw
	GameObserver::GetInstance()->mLayers->stackLayer()->addDamage(source,GameObserver::GetInstance()->players[0], counters);
	GameObserver::GetInstance()->mLayers->stackLayer()->addDamage(source,GameObserver::GetInstance()->players[1], counters);
      }
    }
  }
  int isReactingToClick(MTGCardInstance *   _card, ManaCost * mana = NULL){
    if (counters > 0 && _card == source && currentPhase == Constants::MTG_PHASE_UPKEEP){
      if (game->currentlyActing()->getManaPool()->canAfford( & cost)){
	return 1;
      }
    }
    return 0;
  }

  int reactToClick(MTGCardInstance * _card){
    if (!isReactingToClick( _card)) return 0;
    game->currentlyActing()->getManaPool()->pay(& cost);
    counters --;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AArmageddonClock ::: counters : " << counters
	<< " ; cost : " << cost
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  AArmageddonClock * clone() const{
    AArmageddonClock * a =  NEW AArmageddonClock(*this);
    a->isClone = 1;
    return a;
  }
};

//Channel
class AChannel:public ActivatedAbility{
 public:

 AChannel(int _id, MTGCardInstance * card):ActivatedAbility(_id, card,0,0,0){
  }

  int isReactingToClick(PlayGuiObject * object){
    if (object->type == GUI_AVATAR){
      Player * player = ((GuiAvatar *)object)->player;
      if (player == source->controller()) return 1;
    }
    return 0;
  }

  int resolve(){
    source->controller()->life--;
    source->controller()->getManaPool()->add(Constants::MTG_COLOR_ARTIFACT, 1);
    return 1;
  }

  int testDestroy(){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP) return 1;
    currentPhase = newPhase;
    return 0;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AChannel ::: (";
    return ActivatedAbility::toString(out) << ")";
  }
  AChannel * clone() const{
    AChannel * a =  NEW AChannel(*this);
    a->isClone = 1;
    return a;
  }
};


// Clockwork Beast
class AClockworkBeast:public MTGAbility{
 public:
  int counters;
  ManaCost cost;
 AClockworkBeast(int id, MTGCardInstance * _source):MTGAbility(id, _source){
    counters = 7;
    ((MTGCardInstance *)target)->power+=7;
    int _cost[] = {Constants::MTG_COLOR_ARTIFACT, 1};
    cost = ManaCost(_cost,1);
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_COMBATEND){
      if (((MTGCardInstance *)source)->isAttacker() || ((MTGCardInstance *)source)->isDefenser()){
	counters--;
	((MTGCardInstance *)target)->power-=1;
      }
    }
  }
  int isReactingToClick(MTGCardInstance *  _card, ManaCost * mana = NULL){
    if (counters < 7  && _card == source && currentPhase == Constants::MTG_PHASE_UPKEEP && game->currentPlayer->game->inPlay->hasCard(source)){
      if (game->currentlyActing()->getManaPool()->canAfford( & cost)){
	return 1;
      }
    }
    return 0;
  }

  int reactToClick(MTGCardInstance * _card){
    if (!isReactingToClick( _card)) return 0;
    game->currentlyActing()->getManaPool()->pay(& cost);
    counters ++;
    ((MTGCardInstance *)target)->power++;
    ((MTGCardInstance *)target)->tap();
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AClockworkBeast ::: counters : " << counters
	<< " ; cost : " << cost
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  AClockworkBeast * clone() const{
    AClockworkBeast * a =  NEW AClockworkBeast(*this);
    a->isClone = 1;
    return a;
  }
};

//1102: Conservator
class AConservator: public MTGAbility{
 public:
  int canprevent;
  ManaCost cost;
 AConservator(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
    canprevent = 0;
    int _cost[] = {Constants::MTG_COLOR_ARTIFACT, 2};
    cost = ManaCost(_cost, 1);
  }

  int alterDamage(Damage * damage){
    if (canprevent && damage->target == source->controller()){
      if (damage->damage >= canprevent){
	damage->damage-=canprevent;
	canprevent = 0;
      }else{
	canprevent-=damage->damage;
	damage->damage = 0;
      }
    }
    return 1;
  }
  int alterDamage(){
    if (canprevent){
      ActionStack * stack = game->mLayers->stackLayer();
      for (int i = stack->mCount-1; i>=0; i--){
	if (!canprevent) return 1;
	Interruptible * current = ((Interruptible *)stack->mObjects[i]);
	if (current->type == ACTION_DAMAGE && current->state==NOT_RESOLVED){
	  Damage * damage = (Damage *)current;
	  alterDamage(damage);
	}else if (current->type == ACTION_DAMAGES && current->state == NOT_RESOLVED){
	  DamageStack * damages = (DamageStack *)current;
	  for (int j = damages->mCount-1;j >=0; j--){
	    alterDamage(((Damage *)damages->mObjects[j]));
	  }
	}
      }
    }
    return 1;
  }

  void Update(float dt){
    alterDamage();
  }

  int isReactingToClick(MTGCardInstance *  _card, ManaCost * mana = NULL){
    if ( _card == source && game->currentlyActing()->game->inPlay->hasCard(source) && !_card->isTapped()){
      if (game->currentlyActing()->getManaPool()->canAfford( & cost)){
	return 1;
      }
    }
    return 0;
  }

  int reactToClick(MTGCardInstance * _card){
    if (!isReactingToClick( _card)) return 0;
    game->currentlyActing()->getManaPool()->pay(& cost);
    source->tap();
    canprevent = 2;
    alterDamage();
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AConservator ::: canprevent : " << canprevent
	<< " ; cost : " << cost
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  AConservator * clone() const{
    AConservator * a =  NEW AConservator(*this);
    a->isClone = 1;
    return a;
  }
};


//1345 Farmstead
class AFarmstead:public ActivatedAbility{
 public:
 int usedThisTurn;
 AFarmstead(int _id, MTGCardInstance * source, MTGCardInstance * _target):ActivatedAbility(_id, source,0,1,0){
    int _cost[] = {Constants::MTG_COLOR_WHITE, 2};
    cost = NEW ManaCost(_cost,1);
    target = _target;
    usedThisTurn = 0;
  }

 void Update(float dt){
   if (newPhase != currentPhase && newPhase != Constants::MTG_PHASE_UPKEEP){
     usedThisTurn = 0;
   }
   ActivatedAbility::Update(dt);
 }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (!ActivatedAbility::isReactingToClick(card,mana)) return 0;
    if (currentPhase != Constants::MTG_PHASE_UPKEEP) return 0;
    if (usedThisTurn) return 0;
    return 1;
  }

  int resolve(){
    source->controller()->life++;
    usedThisTurn = 1;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AFarmstead ::: usedThisTurn : " << usedThisTurn
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }
  AFarmstead * clone() const{
    AFarmstead * a =  NEW AFarmstead(*this);
    a->isClone = 1;
    return a;
  }
};


//1112 Howling Mine
class AHowlingMine:public MTGAbility{
 public:
 AHowlingMine(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){}

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_DRAW && !source->isTapped()){
      game->mLayers->stackLayer()->addDraw(game->currentPlayer);
    }
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AHowlingMine ::: (";
    return MTGAbility::toString(out) << ")";
  }
  AHowlingMine * clone() const{
    AHowlingMine * a =  NEW AHowlingMine(*this);
    a->isClone = 1;
    return a;
  }
};


//Living Artifact
class ALivingArtifact:public MTGAbility{
 public:
  int usedThisTurn;
  int counters;
  Damage * latest;
 ALivingArtifact(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id,_source,_target){
    usedThisTurn = 0;
    counters = 0;
    latest = NULL;
  }

  int receiveEvent(WEvent * event){
    WEventDamage * e = dynamic_cast<WEventDamage *>(event);
    if (!e) return 0;
    Player * p = dynamic_cast<Player *>(e->damage->target);
    if (!p) return 0;
    if (p != source->controller()) return 0;
    counters+=e->damage->damage;
    return 1; //is this meant to return 0 or 1?
  }

  int isReactingtoclick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (currentPhase == Constants::MTG_PHASE_UPKEEP && card == source && game->currentPlayer == source->controller() && counters && !usedThisTurn){
      return 1;
    }
    return 0;
  }

  int reactToClick(MTGCardInstance * card){
    source->controller()->life+=1;
    counters--;
    usedThisTurn = 1;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ALivingArtifact ::: usedThisTurn : " << usedThisTurn
	<< " ; counters : " << counters
	<< " ; latest : " << latest
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  ALivingArtifact * clone() const{
    ALivingArtifact * a =  NEW ALivingArtifact(*this);
    a->isClone = 1;
    return a;
  }
};

//Lord of the Pit
class ALordOfThePit: public TargetAbility{
 public:
  int paidThisTurn;
 ALordOfThePit(int _id, MTGCardInstance * source):TargetAbility(_id, source, NEW CreatureTargetChooser(),0,1,0){
    paidThisTurn = 1;
  }

  void Update(float dt){
    if (newPhase != currentPhase && source->controller() == game->currentPlayer){
      if (newPhase == Constants::MTG_PHASE_UNTAP){
	paidThisTurn = 0;
      }else if( newPhase == Constants::MTG_PHASE_UPKEEP + 1 && !paidThisTurn){
	game->mLayers->stackLayer()->addDamage(source,source->controller(), 7);
      }
    }
    TargetAbility::Update(dt);
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (currentPhase != Constants::MTG_PHASE_UPKEEP || paidThisTurn) return 0;
    return TargetAbility::isReactingToClick(card,mana);
  }

  int resolve(){
    MTGCardInstance * card = tc->getNextCardTarget();
    if (card && card != source && card->controller() == source->controller()){
      card->controller()->game->putInGraveyard(card);
      paidThisTurn = 1;
      return 1;
    }
    return 0;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ALordOfThePit ::: paidThisTurn : " << paidThisTurn
	<< " (";
    return TargetAbility::toString(out) << ")";
  }
  ALordOfThePit * clone() const{
    ALordOfThePit * a =  NEW ALordOfThePit(*this);
    a->isClone = 1;
    return a;
  }
};
//1143 Animate Dead
class AAnimateDead:public MTGAbility{
 public:
 AAnimateDead(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source, _target){
    MTGCardInstance * card =  _target;

    //Put the card in play again, with all its abilities !
    //AbilityFactory af;
    MTGCardInstance * copy = source->controller()->game->putInZone(card,  _target->controller()->game->graveyard, source->controller()->game->temp);
    Spell * spell = NEW Spell(copy);
    spell->resolve();
    target = spell->source;
    card = spell->source;
    card->power--;
    card->life = card->toughness;
    delete spell;
  }

  int destroy(){
    MTGCardInstance * card = (MTGCardInstance *) target;
    card->power++;
    card->controller()->game->putInZone(card, card->controller()->game->inPlay,card->owner->game->graveyard);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AAnimateDead ::: (";
    return MTGAbility::toString(out) << ")";
  }
    AAnimateDead * clone() const{
    AAnimateDead * a =  NEW AAnimateDead(*this);
    a->isClone = 1;
    return a;
  }
};


//1159 Erg Raiders
class AErgRaiders:public MTGAbility{
 public:
  int attackedThisTurn;
 AErgRaiders(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
    attackedThisTurn = 1;
  }

  void Update(float dt){
    if (newPhase != currentPhase){
      Player * controller =  source->controller();
      if (newPhase == Constants::MTG_PHASE_COMBATDAMAGE && game->currentPlayer == controller){
	      if (source->isAttacker()){
	        attackedThisTurn = 1;
	      }
      }else if (newPhase == Constants::MTG_PHASE_UNTAP){
        if (game->currentPlayer != controller && !attackedThisTurn){
	        game->mLayers->stackLayer()->addDamage(source, controller,2);
        }else if (game->currentPlayer == controller) {
          attackedThisTurn = 0;
	      }
      }
    }
  }

  AErgRaiders * clone() const{
    AErgRaiders * a =  NEW AErgRaiders(*this);
    a->isClone = 1;
    return a;
  }
};

//Fastbond
class AFastbond:public TriggeredAbility{
 public:
  int alreadyPlayedALand;
  int previous;
 AFastbond(int _id, MTGCardInstance * card):TriggeredAbility(_id, card){
    alreadyPlayedALand = 0;
    if (source->controller()->canPutLandsIntoPlay == 0){
      alreadyPlayedALand = 1;
      source->controller()->canPutLandsIntoPlay = 1;
    }
    previous = source->controller()->canPutLandsIntoPlay;
  }

  void Update(float dt){
    if (newPhase!=currentPhase && newPhase == Constants::MTG_PHASE_UNTAP){
      alreadyPlayedALand = 0;
    }
    TriggeredAbility::Update(dt);
  }

  int trigger(){
    if(source->controller()->canPutLandsIntoPlay==0 && previous == 1){
      previous = 0;
      source->controller()->canPutLandsIntoPlay = 1;
      if (alreadyPlayedALand) return 1;
      alreadyPlayedALand = 1;
      return 0;
    }
    previous = source->controller()->canPutLandsIntoPlay;
    return 0;
  }

  int resolve(){
    game->mLayers->stackLayer()->addDamage(source, source->controller(), 1);
    game->mLayers->stackLayer()->resolve();
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AFastbond ::: alreadyPlayedALand : " << alreadyPlayedALand
	<< " ; previous : " << previous
	<< " (";
    return TriggeredAbility::toString(out) << ")";
  }
  AFastbond * clone() const{
    AFastbond * a =  NEW AFastbond(*this);
    a->isClone = 1;
    return a;
  }
};



//1165 Hypnotic Specter
class AHypnoticSpecter:public MTGAbility{
 public:

 AHypnoticSpecter(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
 }

 int receiveEvent(WEvent * event){
  WEventDamage * e = dynamic_cast<WEventDamage *>(event);
  if (!e) return 0;
  if (e->damage->source != source) return 0;
  Player * p = dynamic_cast<Player *>(e->damage->target);
  if (!p) return 0;
	p->game->discardRandom(p->game->hand);
  return 1; //is this meant to return 0 or 1?
 }
  
  AHypnoticSpecter * clone() const{
    AHypnoticSpecter * a =  NEW AHypnoticSpecter(*this);
    a->isClone = 1;
    return a;
  }
};

//1117 Jandor's Ring
class AJandorsRing:public ActivatedAbility{
 public:
 AJandorsRing(int _id, MTGCardInstance * _source):ActivatedAbility(_id,_source, NEW ManaCost()){
    cost->add(Constants::MTG_COLOR_ARTIFACT, 2);
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (!source->controller()->game->hand->hasCard(source->controller()->game->library->lastCardDrawn)) return 0;
    return ActivatedAbility::isReactingToClick(card,mana);
  }

  int resolve(){
    source->controller()->game->putInGraveyard(source->controller()->game->library->lastCardDrawn);
    game->mLayers->stackLayer()->addDraw(source->controller());
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AJandorsRing ::: (";
    return ActivatedAbility::toString(out) << ")";
  }
  AJandorsRing * clone() const{
    AJandorsRing * a =  NEW AJandorsRing(*this);
    a->isClone = 1;
    return a;
  }
};


//Kudzu.
//What happens when there are no targets ???
class AKudzu: public TargetAbility{
 public:
 AKudzu(int _id, MTGCardInstance * card, MTGCardInstance * _target):TargetAbility(_id,card, NEW TypeTargetChooser("land",card)){
    tc->toggleTarget(_target);
    target = _target;
  }

 int receiveEvent(WEvent * event){
   if (WEventCardTap* wect =  dynamic_cast<WEventCardTap*>(event)) {
     if (wect->before == false && wect->after == true){
       MTGCardInstance * _target = (MTGCardInstance *)target;
       if (!_target->isInPlay()) return 0;
       target = _target->controller()->game->putInGraveyard(_target);
       reactToClick(source);
       return 1;
     }
   }
   return 0;

 }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if (card == source && (!_target || !_target->isInPlay())){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("Kudzu Reacts to click !\n");
#endif
      return 1;
    }
    return 0;
  }


  int resolve(){
    target = tc->getNextCardTarget();
    source->target = (MTGCardInstance *) target;
    return 1;
  }

  int testDestroy(){
    int stillLandsInPlay = 0;
    for (int i = 0; i < 2; i++){
      if (game->players[i]->game->inPlay->hasType("Land")) stillLandsInPlay = 1;
    }
    if (!stillLandsInPlay){
      source->controller()->game->putInGraveyard(source);
      return 1;
    }

    if (!game->isInPlay(source)){
      return 1;
    }

    return 0;
  }

  AKudzu * clone() const{
    AKudzu * a =  NEW AKudzu(*this);
    a->isClone = 1;
    return a;
  }
};

//1172 Pestilence
class APestilence: public ActivatedAbility{
 public:
 APestilence(int _id, MTGCardInstance * card):ActivatedAbility(_id, card, NEW ManaCost(), 0,0){
    cost->add(Constants::MTG_COLOR_BLACK, 1);
  }

  void Update(float dt){
    if (newPhase !=currentPhase && newPhase == Constants::MTG_PHASE_EOT){
      if (!game->players[0]->game->inPlay->hasType("creature") && !game->players[1]->game->inPlay->hasType("creature")){
	source->controller()->game->putInGraveyard(source);
      }
    }
  }

  int resolve(){
    for (int i = 0; i < 2 ; i++){
      MTGInPlay * inplay = game->players[i]->game->inPlay;
      for (int j = inplay->nb_cards - 1 ; j >=0; j--){
	if (inplay->cards[j]->isCreature()) game->mLayers->stackLayer()->addDamage(source,inplay->cards[j],1);
      }
      game->mLayers->stackLayer()->addDamage(source,game->players[i],1);
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "APestilence ::: (";
    return ActivatedAbility::toString(out) << ")";
  }
  APestilence * clone() const{
    APestilence * a =  NEW APestilence(*this);
    a->isClone = 1;
    return a;
  }
};


//Power Leak
class APowerLeak:public TriggeredAbility{
 public:
  int damagesToDealThisTurn;
  ManaCost cost;
 APowerLeak(int _id, MTGCardInstance * _source, MTGCardInstance * _target):TriggeredAbility(_id, _source, _target){
    cost.add(Constants::MTG_COLOR_ARTIFACT, 1);
    damagesToDealThisTurn = 0;
  }

  void Update(float dt){
    MTGCardInstance * _target  = (MTGCardInstance *) target;
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UPKEEP && _target->controller() == game->currentPlayer){
      damagesToDealThisTurn = 2;
    }
    TriggeredAbility::Update(dt);
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (damagesToDealThisTurn && currentPhase == Constants::MTG_PHASE_UPKEEP && card==source && _target->controller() == game->currentPlayer){
      if (game->currentPlayer->getManaPool()->canAfford(& cost)) return 1;
    }
    return 0;
  }

  int reactToclick(MTGCardInstance * card){
    game->currentPlayer->getManaPool()->pay( & cost);
    damagesToDealThisTurn--;
    return 1;
  }

  int trigger(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_DRAW && _target->controller() == game->currentPlayer){
      if (damagesToDealThisTurn) return 1;
    }
    return 0;
  }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    game->mLayers->stackLayer()->addDamage(source,_target->controller(), damagesToDealThisTurn);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "APowerLeak ::: damagesToDealThisTurn : " << damagesToDealThisTurn
      	<< " ; cost : " << cost
	<< " (";
    return TriggeredAbility::toString(out) << ")";
  }
  APowerLeak * clone() const{
    APowerLeak * a =  NEW APowerLeak(*this);
    a->isClone = 1;
    return a;
  }
};

//1176 Sacrifice
class ASacrifice:public InstantAbility{
 public:
 ASacrifice(int _id, MTGCardInstance * _source, MTGCardInstance * _target):InstantAbility(_id, _source){
    target = _target;
  }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target->isInPlay()){
      game->currentlyActing()->game->putInGraveyard(_target);
      int x = _target->getManaCost()->getConvertedCost();
      game->currentlyActing()->getManaPool()->add(Constants::MTG_COLOR_BLACK, x);
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ASacrifice ::: (";
    return InstantAbility::toString(out) << ")";
  }
  ASacrifice * clone() const{
    ASacrifice * a =  NEW ASacrifice(*this);
    a->isClone = 1;
    return a;
  }
};

//1178 Scavenging Ghoul
class AScavengingGhoul:public MTGAbility{
 public:
  int counters;
 AScavengingGhoul(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source, _target){
    counters = 0;
  }


  void Update(float dt){
    //TODO
  }

  int isReactingToClick(MTGCardInstance *  _card, ManaCost * mana = NULL){
    if (counters > 0 && _card == source && game->currentlyActing()->game->inPlay->hasCard(source)){
      return 1;
    }
    return 0;
  }

  int reactToClick(MTGCardInstance *  _card){
    if (!isReactingToClick( _card)) return 0;
    counters--;
    source->regenerate();
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AScavengingGhoul ::: counters : " << counters
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  AScavengingGhoul * clone() const{
    AScavengingGhoul * a =  NEW AScavengingGhoul(*this);
    a->isClone = 1;
    return a;
  }
};


//1235 Aspect of Wolf
class AAspectOfWolf:public ListMaintainerAbility{
 public:
  int color;
 AAspectOfWolf(int _id, MTGCardInstance * _source, MTGCardInstance * _target):ListMaintainerAbility(_id, _source, _target){
  }

  int canBeInList(MTGCardInstance * card){

    if (card->controller() == source->controller() &&  card->hasType("forest") && game->isInPlay(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    int size = cards.size();
    if (size % 2 == 0){
      _target->power += 1;
    }else{
      _target->addToToughness(1);
    }
    return 1;
  }

  int removed(MTGCardInstance * card){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    int size = cards.size();
    if (size % 2 == 1){
      _target->power -= 1;
    }else{
      _target->addToToughness(-1);
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AAspectOfWolf ::: color : " << color
	<< " (";
    return ListMaintainerAbility::toString(out) << ")";
  }
  AAspectOfWolf * clone() const{
    AAspectOfWolf * a =  NEW AAspectOfWolf(*this);
    a->isClone = 1;
    return a;
  }
};

//1284 Dragon Whelp
class ADragonWhelp: public APowerToughnessModifierUntilEndOfTurn{
 public:
 ADragonWhelp(int id, MTGCardInstance * card):APowerToughnessModifierUntilEndOfTurn(id, card, card, NEW WParsedPT(1, 0), NEW ManaCost()){
    cost->add(Constants::MTG_COLOR_RED, 1);
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_AFTER_EOT && counters > 3){
      source->controller()->game->putInGraveyard(source);
    }
    APowerToughnessModifierUntilEndOfTurn::Update(dt);
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADragonWhelp ::: (";
    return APowerToughnessModifierUntilEndOfTurn::toString(out) << ")";
  }
  ADragonWhelp * clone() const{
    ADragonWhelp * a =  NEW ADragonWhelp(*this);
    a->isClone = 1;
    return a;
  }
};

//1288 EarthBind
class AEarthbind:public ABasicAbilityModifier{
 public:
 AEarthbind(int _id, MTGCardInstance * _source, MTGCardInstance * _target):ABasicAbilityModifier(_id,_source,_target,Constants::FLYING,0){
   if (value_before_modification){
     Damageable * _target = (Damageable *)target;
     game->mLayers->stackLayer()->addDamage(source,_target,2);
   }
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AEarthbind ::: (";
    return ABasicAbilityModifier::toString(out) << ")";
  }
  AEarthbind * clone() const{
    AEarthbind * a =  NEW AEarthbind(*this);
    a->isClone = 1;
    return a;
  }
};

//1291 Fireball
class AFireball:public InstantAbility{
 public:
 AFireball(int _id, MTGCardInstance * card, Spell * spell, int x):InstantAbility(_id, card){
    int nbtargets = spell->getNbTargets();
    int totaldamage = x+1-nbtargets;
    int individualdamage = 0;
    if (nbtargets) individualdamage  = totaldamage / nbtargets;
    Damageable * _target = spell->getNextDamageableTarget();
    while(_target){
      game->mLayers->stackLayer()->addDamage(source,_target,individualdamage);
      _target = spell->getNextDamageableTarget(_target);
    }
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AFireball ::: (";
    return InstantAbility::toString(out) << ")";
  }
  AFireball * clone() const{
    AFireball * a =  NEW AFireball(*this);
    a->isClone = 1;
    return a;
  }
};

//1245 ForceOfNature
class AForceOfNature:public ActivatedAbility{
 public:
  int dealDamageThisTurn;
 AForceOfNature(int _id, MTGCardInstance * card):ActivatedAbility(_id,card, NEW ManaCost(),1,0){
    dealDamageThisTurn = 0;
    cost->add(Constants::MTG_COLOR_GREEN,4);
  }

  void Update(float dt){
    if (newPhase !=currentPhase){
      if (newPhase == Constants::MTG_PHASE_UNTAP){
	dealDamageThisTurn = 1;
      }else if (newPhase == Constants::MTG_PHASE_DRAW && dealDamageThisTurn && game->currentPlayer==source->controller() ){
	game->mLayers->stackLayer()->addDamage(source,source->controller(),8);
      }
    }
    ActivatedAbility::Update(dt);
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    return (dealDamageThisTurn && currentPhase == Constants::MTG_PHASE_UPKEEP && ActivatedAbility::isReactingToClick(card,mana));
  }

  int resolve(){
    dealDamageThisTurn = 0;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AForceOfNature ::: dealDamageThisTurn : " << dealDamageThisTurn
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }
  AForceOfNature * clone() const{
    AForceOfNature * a =  NEW AForceOfNature(*this);
    a->isClone = 1;
    return a;
  }
};


//1351 Island Sanctuary
class AIslandSanctuary:public MTGAbility{
 public:
  int initThisTurn;
 AIslandSanctuary(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
    initThisTurn = 0;
  }

  void Update(float dt){
    if (currentPhase == Constants::MTG_PHASE_UNTAP && game->currentPlayer == source->controller()) initThisTurn = 0;

    if (initThisTurn && currentPhase == Constants::MTG_PHASE_COMBATATTACKERS && game->currentPlayer != source->controller()){
      MTGGameZone *  zone = game->currentPlayer->game->inPlay;
      for (int i = 0; i < zone->nb_cards; i++){
	MTGCardInstance * card =  zone->cards[i];
  if (card->isAttacker() && !card->basicAbilities[Constants::FLYING] && !card->basicAbilities[Constants::ISLANDWALK]) source->toggleAttacker();
      }
    }
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (card==source && game->currentPlayer == card->controller() && currentPhase == Constants::MTG_PHASE_DRAW){
      Interruptible * action = game->mLayers->stackLayer()->getAt(-1);
      if (action->type == ACTION_DRAW) return 1;
    }
    return 0;
  }


  int reactToClick(MTGCardInstance * card){
    if (!isReactingToClick(card)) return 0;
    game->mLayers->stackLayer()->Remove(game->mLayers->stackLayer()->getAt(-1));
    initThisTurn = 1;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AIslandSanctuary ::: initThisTurn : " << initThisTurn
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  AIslandSanctuary * clone() const{
    AIslandSanctuary * a =  NEW AIslandSanctuary(*this);
    a->isClone = 1;
    return a;
  }
};


//Stasis
class AStasis:public ActivatedAbility{
 public:
  int paidThisTurn;
 AStasis(int _id, MTGCardInstance * card):ActivatedAbility(_id,card, NEW ManaCost(),1,0){
    paidThisTurn = 1;
    cost->add(Constants::MTG_COLOR_BLUE,1);
  }

  void Update(float dt){
    //Upkeep Cost
    if (newPhase !=currentPhase){
      if (newPhase == Constants::MTG_PHASE_UPKEEP){
	paidThisTurn = 0;
      }else if (!paidThisTurn && newPhase > Constants::MTG_PHASE_UPKEEP &&  game->currentPlayer==source->controller() ){
	game->currentPlayer->game->putInGraveyard(source);
	paidThisTurn = 1;
      }
    }
    //Stasis Effect
    for (int i = 0; i < 2; i++){
      game->phaseRing->removePhase(Constants::MTG_PHASE_UNTAP,game->players[i]);
    }

    //Parent Class Method Call
    ActivatedAbility::Update(dt);
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    return (!paidThisTurn && currentPhase == Constants::MTG_PHASE_UPKEEP && ActivatedAbility::isReactingToClick(card,mana));
  }

  int resolve(){
    paidThisTurn = 1;
    return 1;
  }

  int destroy(){
    for (int i = 0; i < 2; i++){
      game->phaseRing->addPhaseBefore(Constants::MTG_PHASE_UNTAP,game->players[i],Constants::MTG_PHASE_UPKEEP,game->players[i]);
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AStasis ::: paidThisTurn : " << paidThisTurn
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }
  AStasis * clone() const{
    AStasis * a =  NEW AStasis(*this);
    a->isClone = 1;
    return a;
  }
};



//--------------Addon Abra------------------

//Basilik --> needs to be made more generic to avoid duplicate (also something like if opponent=type then ...)
class ABasilik:public MTGAbility{
 public:
  MTGCardInstance * opponents[20];
  int nbOpponents;
 ABasilik (int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
    nbOpponents = 0;
  }

  void Update(float dt){
    if (newPhase != currentPhase){
      if( newPhase == Constants::MTG_PHASE_COMBATDAMAGE){
	nbOpponents = 0;
	MTGCardInstance * opponent = source->getNextOpponent();
	while (opponent){
	  opponents[nbOpponents] = opponent;
	  nbOpponents ++;
	  opponent = source->getNextOpponent(opponent);
	}
      }else if (newPhase == Constants::MTG_PHASE_COMBATEND){
	for (int i = 0; i < nbOpponents ; i++){
	  game->mLayers->stackLayer()->addPutInGraveyard(opponents[i]);
	}
      }
    }
  }
  
  virtual ostream& toString(ostream& out) const
  {
    out << "ABasilik ::: opponents : " << opponents
	<< " ; nbOpponents : " << nbOpponents
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
  ABasilik * clone() const{
    ABasilik * a =  NEW ABasilik(*this);
    a->isClone = 1;
    return a;
  }
};


//Lavaborn - quick and very dirty ;) copy of ALifezonelink but without the multiplier.
class ALavaborn:public MTGAbility{
 public:
  int phase;
  int condition;
  int life;
  int controller;
  int nbcards;
  MTGGameZone * zone;
 ALavaborn(int _id ,MTGCardInstance * card, int _phase, int _condition, int _life, int _controller = 0, MTGGameZone * _zone = NULL):MTGAbility(_id, card){
    phase = _phase;
    condition = _condition;
    controller = _controller;
    life = _life;
    zone = _zone;
    if (zone == NULL){
      if (controller){
	zone = game->currentPlayer->game->hand;
      }else{
	zone = game->opponent()->game->hand;
      }
    }
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == phase){
      if ((controller && game->currentPlayer == source->controller()) ||(!controller && game->currentPlayer != source->controller()) ){
	if ((condition < 0 && zone->nb_cards < - condition) ||(condition >0 && zone->nb_cards > condition)){
	  int diff = zone->nb_cards - condition;
	  if (condition < 0) diff = - condition - zone->nb_cards;
	  if (life > 0){
	    game->currentPlayer->life+=life;
	  }else{
	    game->mLayers->stackLayer()->addDamage(source,game->currentPlayer,-life);
	  }
	}
      }
    }
  }
virtual ostream& toString(ostream& out) const
  {
    out << "ALavaborn ::: phase : " << phase
	<< " ; condition : " << condition
	<< " ; life : " << life
	<< " ; controller : " << controller
	<< " ; nbcards : " << nbcards
	<< " (";
    return MTGAbility::toString(out) << ")";
  }  

ALavaborn * clone() const{
    ALavaborn * a =  NEW ALavaborn(*this);
    a->isClone = 1;
    return a;
  }

};


//Generic Millstone
class AADepleter:public ActivatedAbilityTP{
 public:
	 int nbcards;
   AADepleter(int _id, MTGCardInstance * card, Targetable * _target, int nbcards = 1, ManaCost * _cost=NULL, int _tap = 0, int who = TargetChooser::UNSET):ActivatedAbilityTP(_id,card, _target,_cost,_tap,who),nbcards(nbcards){
	  }
  int resolve(){
    Targetable * _target = getTarget();
    Player * player;
    if (_target){
      if (_target->typeAsTarget() == TARGET_CARD){
        player = ((MTGCardInstance *)_target)->controller();
      }else{
        player = (Player *) _target;
      }
      MTGLibrary * library = player->game->library;
      for (int i = 0; i < nbcards; i++){
        if (library->nb_cards)
	        player->game->putInZone(library->cards[library->nb_cards-1],library, player->game->graveyard);
      }
    }
    return 1;
  }

  const char * getMenuText(){
  return "Deplete";
  }

  AADepleter * clone() const{
    AADepleter * a =  NEW AADepleter(*this);
    a->isClone = 1;
    return a;
  }
};

//Shuffle
class AAShuffle:public ActivatedAbilityTP{
 public:
   AAShuffle(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost=NULL, int _tap = 0, int who = TargetChooser::UNSET):ActivatedAbilityTP(_id,card, _target,_cost,_tap,who){
	  }
  int resolve(){
    Targetable * _target = getTarget();
    Player * player;
    if (_target){
      if (_target->typeAsTarget() == TARGET_CARD){
        player = ((MTGCardInstance *)_target)->controller();
      }else{
        player = (Player *) _target;
      }
      MTGLibrary * library = player->game->library;
	  library->shuffle();
    }
    return 1;
  }

  const char * getMenuText(){
  return "Shuffle";
  }

  AAShuffle * clone() const{
    AAShuffle * a =  NEW AAShuffle(*this);
    a->isClone = 1;
    return a;
  }
};

//Random Discard
class AARandomDiscarder:public ActivatedAbilityTP{
 public:
	 int nbcards;
   AARandomDiscarder(int _id, MTGCardInstance * card, Targetable * _target, int nbcards = 1, ManaCost * _cost=NULL, int _tap = 0,int who=TargetChooser::UNSET):ActivatedAbilityTP(_id,card, _target,_cost,_tap,who),nbcards(nbcards){
	  }
  int resolve(){
    Targetable * _target = getTarget();
    Player * player;
    if (_target){
      if (_target->typeAsTarget() == TARGET_CARD){
        player = ((MTGCardInstance *)_target)->controller();
      }else{
        player = (Player *) _target;
      }
      for (int i = 0; i < nbcards; i++){
	      player->game->discardRandom(player->game->hand);
	    }
    }
    return 1;
  }

  const char * getMenuText(){
  return "Discard Random";
  }

  AARandomDiscarder * clone() const{
    AARandomDiscarder * a =  NEW AARandomDiscarder(*this);
    a->isClone = 1;
    return a;
  }
};



//ShieldOfTheAge
class AShieldOfTheAge: public TargetAbility{
 public:
 AShieldOfTheAge(int _id, MTGCardInstance * card):TargetAbility(_id,card,NEW DamageTargetChooser(card,_id),NEW ManaCost(),0,0){
    cost->add(Constants::MTG_COLOR_ARTIFACT,2);
  }

  int resolve(){
    Damage * damage = tc->getNextDamageTarget();
    if (!damage) return 0;
    game->mLayers->stackLayer()->Fizzle(damage);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AShieldOfTheAge ::: (";
    return TargetAbility::toString(out) << ")";
  }

    AShieldOfTheAge * clone() const{
    AShieldOfTheAge * a =  NEW AShieldOfTheAge(*this);
    a->isClone = 1;
    return a;
  }
};


//Minion of Leshrac
class AMinionofLeshrac: public TargetAbility{
 public:
  int paidThisTurn;
 AMinionofLeshrac(int _id, MTGCardInstance * source):TargetAbility(_id, source, NEW CreatureTargetChooser(),0,1,0){
    paidThisTurn = 1;
  }

  void Update(float dt){
    if (newPhase != currentPhase && source->controller() == game->currentPlayer){
      if (newPhase == Constants::MTG_PHASE_UNTAP){
	paidThisTurn = 0;
      }else if( newPhase == Constants::MTG_PHASE_UPKEEP + 1 && !paidThisTurn){
	game->mLayers->stackLayer()->addDamage(source,source->controller(), 5);
	source->tap();
      }
    }
    TargetAbility::Update(dt);
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (currentPhase != Constants::MTG_PHASE_UPKEEP || paidThisTurn) return 0;
    return TargetAbility::isReactingToClick(card,mana);
  }

  int resolve(){
    MTGCardInstance * card = tc->getNextCardTarget();
    if (card && card != source && card->controller() == source->controller()){
      card->controller()->game->putInGraveyard(card);
      paidThisTurn = 1;
      return 1;
    }
    return 0;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AMinionofLeshrac ::: paidThisTurn : " << paidThisTurn
	<< " (";
    return TargetAbility::toString(out) << ")";
  }

  AMinionofLeshrac * clone() const{
    AMinionofLeshrac * a =  NEW AMinionofLeshrac(*this);
    a->isClone = 1;
    return a;
  }
};


//Rampage ability
class ARampageAbility:public MTGAbility{
 public:
  int nbOpponents;
  int PowerModifier;
  int ToughnessModifier;
  int MaxOpponent;

 ARampageAbility(int _id, MTGCardInstance * _source,int _PowerModifier, int _ToughnessModifier, int _MaxOpponent):MTGAbility(_id, _source){
    PowerModifier = _PowerModifier;
    ToughnessModifier = _ToughnessModifier;
    MaxOpponent = _MaxOpponent;
    nbOpponents = 0;
  }
  int receiveEvent(WEvent * event) {
    if (dynamic_cast<WEventBlockersChosen*>(event)) {
      nbOpponents = source->blockers.size();
      if (nbOpponents <= MaxOpponent) return 0;
      source->power += PowerModifier * (nbOpponents - MaxOpponent);
      source->addToToughness(ToughnessModifier * (nbOpponents - MaxOpponent));
    }
    else if (WEventPhaseChange* pe = dynamic_cast<WEventPhaseChange*>(event)) {
      if (Constants::MTG_PHASE_AFTER_EOT == pe->to->id && nbOpponents > MaxOpponent)
        {
          source->power -= PowerModifier * (nbOpponents - MaxOpponent);
          source->addToToughness(-ToughnessModifier * (nbOpponents - MaxOpponent));
          nbOpponents = 0;
        }
    }
    return 1;
  }

  ARampageAbility * clone() const{
    ARampageAbility * a =  NEW ARampageAbility(*this);
    a->isClone = 1;
    return a;
  }
};

//Instant Steal control of a target
class AInstantControlSteal: public InstantAbility{
 public:
  Player  * TrueController;
  Player  * TheftController;
 AInstantControlSteal(int _id , MTGCardInstance * _source, MTGCardInstance * _target):InstantAbility(_id, _source, _target){
    TrueController = _target->controller();
 	  TheftController = source->controller();
	  MTGCardInstance * copy = _target->changeController(TheftController);
    target = copy;
    source->target = copy;
    copy->summoningSickness = 0; 
 }

 int destroy(){
	MTGCardInstance * _target = (MTGCardInstance *) target;
    if (TheftController && TheftController->game->inPlay->hasCard(_target)){ //if the target is still in game -> spell was destroyed
      _target->changeController(TrueController);
	  
    }
	 return 1;
  }


  virtual ostream& toString(ostream& out) const
  {
    out << "AInstantControlSteal ::: TrueController : " << TrueController
	<< " ; TheftController : " << TheftController
	<< " (";
    return InstantAbility::toString(out) << ")";
  }

  AInstantControlSteal * clone() const{
    AInstantControlSteal * a =  NEW AInstantControlSteal(*this);
    a->isClone = 1;
    return a;
  }
};

//Angelic Chorus (10E)
class AAngelicChorus: public ListMaintainerAbility{
 public:
  int init;
 AAngelicChorus(int id, MTGCardInstance * _source):ListMaintainerAbility(id, _source){
    init = 0;
  }

  void Update(float dt){
    ListMaintainerAbility::Update(dt);
    init = 1;
  }

  int canBeInList(MTGCardInstance * card){
    if (card->hasType(Subtypes::TYPE_CREATURE) && game->isInPlay(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
   if (!init) return 0;
   	  if (source->controller() == game->currentlyActing()){
      card->controller()->life+= card->toughness;
	  }
    return 1;
  }

  int removed(MTGCardInstance * card){
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AAngelicChorus ::: init : " << init
	<< " (";
    return ListMaintainerAbility::toString(out) << ")";
  }

  AAngelicChorus * clone() const{
    AAngelicChorus * a =  NEW AAngelicChorus(*this);
    a->isClone = 1;
    return a;
  }
};


#endif
