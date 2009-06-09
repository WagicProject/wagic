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
#include "WEvent.h"

#include <JGui.h>
#include <hge/hgeparticle.h>


#include <map>
using std::map;

/*
  Generic classes
*/

//MayAbility: May do something when comes into play (should be extended)
class MayAbility:public MTGAbility{
public:
  int triggered;
  MTGAbility * ability;
  int deleteAbility;
  MayAbility(int _id, MTGAbility * _ability,  MTGCardInstance * _source):MTGAbility(_id,_source),ability(_ability){
    triggered = 0;
    ability->forceDestroy = 1;
    deleteAbility = 1;
  }

  void Update(float dt){
    MTGAbility::Update(dt);
    if (!triggered){
      triggered = 1;
      game->mLayers->actionLayer()->setMenuObject(source);
      OutputDebugString("SetMenuObject!\n");
    }
  }

   const char * getMenuText(){
     return ability->getMenuText();
  }

  int testDestroy(){
    if (triggered && !game->mLayers->actionLayer()->menuObject){
      OutputDebugString("Destroy!\n");
      return 1;
    }
    return 0;
  }

  int isReactingToTargetClick(Targetable * card){
    OutputDebugString("IsReacting ???\n");
    if (card == source) return 1;
    return 0;
  }

  int reactToTargetClick(Targetable * object){
    OutputDebugString("ReactToTargetClick!\n");
    deleteAbility = 0;
    game->addObserver(ability);
    return ability->reactToTargetClick(object);
  }

  ~MayAbility(){
    if (deleteAbility) SAFE_DELETE(ability);
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "MayAbility ::: triggered : " << triggered
	<< " ; ability : " << ability
	<< " ; deleteAbility : " << deleteAbility
	<< " (";
    return MTGAbility::toString(out) << ")";
  }

};


//MultiAbility : triggers several actions for a cost
class MultiAbility:public ActivatedAbility{
public:
  vector<MTGAbility *> abilities;
  vector<TriggeredEvent *> events;

  MultiAbility(int _id, MTGCardInstance * card,ManaCost * _cost, int _tap):ActivatedAbility(_id, card,_cost,0,_tap){
  }


  int Add(TriggeredEvent * event){
    events.push_back(event);
    return 1;
  }

  int Add(MTGAbility * ability){
    abilities.push_back(ability);
    return 1;
  }

  int resolve(){
    vector<int>::size_type sz = abilities.size();
    for (unsigned int i = 0; i < sz; i++){
      abilities[i]->resolve();
    }
    sz = events.size();
    for (unsigned int i = 0; i < sz; i++){
      events[i]->resolve();
    }
    return 1;
  }

  ~MultiAbility(){
    vector<int>::size_type sz = abilities.size();
    for (unsigned int i = 0; i < sz; i++){
      delete abilities[i];
    }
    sz = events.size();
    for (unsigned int i = 0; i < sz; i++){
      delete events[i];
    }
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "MultiAbility ::: abilities : ?" // << abilities
      	<< " ; events : ?" // << events
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }
};


//Drawer, allows to draw a card for a cost:

class ADrawer:public ActivatedAbility{
 public:
  int nbcards;
 ADrawer(int _id, MTGCardInstance * card,ManaCost * _cost, int _nbcards = 1, int _tap = 1):ActivatedAbility(_id, card,_cost,0,_tap),nbcards(_nbcards){
  }

  int resolve(){
    game->mLayers->stackLayer()->addDraw(source->controller(),nbcards);
    return 1;
  }



  const char * getMenuText(){
    return "Draw";
  }

  ~ADrawer(){
    OutputDebugString("Deleting ADrawer\n");
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADrawer ::: nbcards : " << nbcards
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }

};

// Gives/Takes Life to controller of source
class ALifeGiver:public ActivatedAbility{
 public:
  int life;
 ALifeGiver(int _id, MTGCardInstance * card,ManaCost * _cost, int _life, int _tap = 1):ActivatedAbility(_id, card,_cost,0,_tap),life(_life){
  }

  int resolve(){
    source->controller()->life+=life;
    return 1;
  }

  const char * getMenuText(){
    if (life < 0) return "Lose life";
    return "Gain life";
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ALifeGiver ::: life : " << life
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }
};

class ATokenCreator:public ActivatedAbility{
public:
  list<int>abilities;
  list<int>types;
  list<int>colors;
  int power, toughness;
  string name;
  ATokenCreator(int _id,MTGCardInstance * _source,ManaCost * _cost, string sname, string stypes,int _power,int _toughness, string sabilities, int _doTap):ActivatedAbility(_id,_source,_cost,0,_doTap){
    power = _power;
    toughness = _toughness;
    name = sname;

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
        int id = Subtypes::subtypesList->Add(s.substr(0,found));
        types.push_back(id);
        s = s.substr(found+1);
      }else{
        int id = Subtypes::subtypesList->Add(s);
        types.push_back(id);
        s = "";
      }
    }
  }

  int resolve(){
    Token * myToken = NEW Token(name,source,power,toughness);
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
    source->controller()->game->stack->addCard(myToken);
    Spell * spell = NEW Spell(myToken);


    spell->resolve();
    delete spell;
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

};

//Moves Cards from a zone to another
class AZoneMover:public TargetAbility{

public:
  string destinationZone;

   AZoneMover(int _id, MTGCardInstance * _source, TargetChooser * _tc,string destZone, ManaCost * _cost = NULL, int _tap=0):TargetAbility(_id,_source, _tc,_cost,0,_tap){
    destinationZone = destZone;
  }

   static int moveTarget(MTGCardInstance * _target, string destinationZone, MTGCardInstance * source){
     GameObserver * g = GameObserver::GetInstance();
     if(_target){
      Player* p = _target->controller();
      if (p){
        MTGGameZone * fromZone = _target->getCurrentZone();
        MTGGameZone * destZone = MTGGameZone::stringToZone(destinationZone, source,_target);

        //inplay is a special zone !
        for (int i=0; i < 2; i++){
          if (destZone == g->players[i]->game->inPlay){
              MTGCardInstance * copy = g->players[i]->game->putInZone(_target,  fromZone, g->players[i]->game->stack);
              Spell * spell = NEW Spell(copy);

              spell->resolve();
              delete spell;
              return 1;
          }
        }
        p->game->putInZone(_target,fromZone,destZone);
      }
      return 1;
    }
    return 0;
   }

  int resolve(){
    MTGCardInstance * _target = tc->getNextCardTarget();
    return moveTarget(_target,destinationZone, source);
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "AZoneMover ::: destinationZone : " << destinationZone
	<< " (";
    return TargetAbility::toString(out) << ")";
  }

};


//Moves Cards from a zone to another
//TODO: this looks awfully similar to the equivalent targetAbility...woudln't there be a way to merge them ?
class AZoneSelfMover:public ActivatedAbility{

public:
  string destinationZone;

   AZoneSelfMover(int _id, MTGCardInstance * _source,string destZone, ManaCost * _cost = NULL, int _tap=0):ActivatedAbility(_id,_source,_cost,0,_tap){
    destinationZone = destZone;
  }

  int resolve(){
    MTGCardInstance * _target = source;
    if(_target){
      Player* p = _target->controller();
      if (p){
        MTGGameZone * fromZone = _target->getCurrentZone();
        MTGGameZone * destZone = MTGGameZone::stringToZone(destinationZone, source,_target);
        //inplay is a special zone !
        for (int i=0; i < 2; i++){
          if (destZone == game->players[i]->game->inPlay){
              MTGCardInstance * copy = game->players[i]->game->putInZone(_target,  fromZone, game->players[i]->game->stack);
              Spell * spell = NEW Spell(copy);

              spell->resolve();
              delete spell;
              return 1;
          }
        }
        p->game->putInZone(_target,fromZone,destZone);
      }
      return 1;
    }
    return 0;
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "AZoneSelfMover ::: destinationZone : " << destinationZone
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }
};


//Copier. TargetAbility
class ACopier:public TargetAbility{
 public:
 ACopier(int _id, MTGCardInstance * _source, TargetChooser * _tc = NULL, ManaCost * _cost=NULL):TargetAbility(_id,_source, _tc,_cost,0,0){
    if (!tc) tc = NEW CreatureTargetChooser();
  }

  int resolve(){
    MTGCardInstance * _target = tc->getNextCardTarget();
    if(_target){
      source->copy(_target);
      return 1;
    }
    return 0;
  }

  const char * getMenuText(){
    return "Copy";
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "ACopier ::: (";
    return TargetAbility::toString(out) << ")";
  }

};


//All Destroyer. TargetAbility
class AAllDestroyer:public ActivatedAbility{
 public:
  int bury;
  AAllDestroyer(int _id, MTGCardInstance * _source, TargetChooser * _tc, int _bury = 0, ManaCost * _cost=NULL,int doTap =1):ActivatedAbility(_id,_source,_cost,0,doTap),bury(_bury){
    tc = _tc;

  }

  int resolve(){
    AbilityFactory af;
    af.destroyAllInPlay(tc,bury);
    return 1;
  }

  const char * getMenuText(){
    return "Destroy All...";
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AAllDestroyer ::: bury : " << bury
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }

};

//Destroyer. TargetAbility
class ADestroyer:public TargetAbility{
 public:
  int bury;
 ADestroyer(int _id, MTGCardInstance * _source, TargetChooser * _tc = NULL, int _bury = 0, ManaCost * _cost=NULL):TargetAbility(_id,_source, _tc,_cost),bury(_bury){
    if (!tc) tc = NEW CreatureTargetChooser();
  }

  int resolve(){
    MTGCardInstance * _target = tc->getNextCardTarget();
    if(_target){
      if (bury){
	_target->controller()->game->putInGraveyard(_target);
      }else{
	game->mLayers->stackLayer()->addPutInGraveyard(_target);
      }
      return 1;
    }
    return 0;
  }

  const char * getMenuText(){
    return "Destroy";
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADestroyer ::: bury : " << bury
	<< " (";
    return TargetAbility::toString(out) << ")";
  }
};

//Destroyer. TargetAbility
class ABurier:public ADestroyer{
 public:
 ABurier(int _id, MTGCardInstance * _source, TargetChooser * _tc = NULL,ManaCost * _cost=NULL):ADestroyer(_id,_source, _tc,1,_cost){
  }

  const char * getMenuText(){
    return "Bury";
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ABurier ::: (";
    return ADestroyer::toString(out) << ")";
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
    value_before_modification = ((MTGCardInstance * )target)->basicAbilities[ability];
    ((MTGCardInstance * )target)->basicAbilities[ability]=modifier;
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

};

/*Instants that modifies a basic ability until end of turn */
class  AInstantBasicAbilityModifierUntilEOT: public InstantAbility{
 public:
  int stateBeforeActivation;
  int ability;
 AInstantBasicAbilityModifierUntilEOT(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _ability, int value):InstantAbility(_id, _source, _target),ability(_ability){
    stateBeforeActivation = _target->basicAbilities[ability];
    _target->basicAbilities[ability] = value;
  }

  int destroy(){
    ((MTGCardInstance *)target)->basicAbilities[ability] = stateBeforeActivation;
    return 1;
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "ABasicAbilityModifierUntilEOT ::: stateBeforeActivation : " << stateBeforeActivation
	<< " ability : " << ability
	<< " (";
    return InstantAbility::toString(out) << ")";
  }

};

//Alteration of Ability until of turn (Aura)
class ABasicAbilityAuraModifierUntilEOT: public ActivatedAbility{
 public:
  int stateBeforeActivation;
  int ability;
  int value;
 ABasicAbilityAuraModifierUntilEOT(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, int _ability, int _value = 1):ActivatedAbility(_id,_source, _cost, 0,0), ability(_ability), value(_value){
    target = _target;
    stateBeforeActivation = _target->basicAbilities[ability];
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP){
      MTGCardInstance * _target = (MTGCardInstance *) target;
      _target->basicAbilities[ability] = stateBeforeActivation;
    }
    ActivatedAbility::Update(dt);
  }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    stateBeforeActivation = _target->basicAbilities[ability];
    _target->basicAbilities[ability] = value;
    return 1;
  }

  const char * getMenuText(){
    return Constants::MTGBasicAbilities[ability];
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ABasicAbilityAuraModifierUntilEOT ::: stateBeforeActivation : " << stateBeforeActivation
	<< " ; ability : " << ability
	<< " ; value : " << value
	<< " (";
    return ActivatedAbility::toString(out) << ")";
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
    _card->untap();
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AUnBlocker ::: cost : " << cost
	<< " (";
    return MTGAbility::toString(out) << ")";
  }

};

//Allows to untap target card once per turn for a manaCost
class AUntaperOnceDuringTurn:public AUnBlocker{
 public:
  int untappedThisTurn;
  int onlyPlayerTurn;
 AUntaperOnceDuringTurn(int id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, int _onlyPlayerTurn = 1):AUnBlocker(id, _source, _target, _cost){
    onlyPlayerTurn = _onlyPlayerTurn;
    untappedThisTurn = 0;
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP) untappedThisTurn = 0;
    AUnBlocker::Update(dt);
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (onlyPlayerTurn && game->currentPlayer!=source->controller()) return 0;
    if (untappedThisTurn) return 0;
    return AUnBlocker::isReactingToClick(card,mana);
  }

  int reactToClick(MTGCardInstance * card){
    untappedThisTurn = 1;
    return AUnBlocker::reactToClick(card);
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AUntaperOnceDuringTurn ::: untappedThisTurn : " << untappedThisTurn
	<< " ; onlyPlayerTurn : " << onlyPlayerTurn
	<< " (";
    return AUnBlocker::toString(out) << ")";
  }

};

//Alteration of Power and Toughness  (enchantments)
class APowerToughnessModifier: public MTGAbility{
 public:
  int power, toughness;
 APowerToughnessModifier(int id, MTGCardInstance * _source, MTGCardInstance * _target, int _power, int _toughness):MTGAbility(id,_source,_target),power(_power),toughness(_toughness){
    _target->power += power;
    _target->addToToughness(toughness);
  }

  int destroy(){
    ((MTGCardInstance *)target)->power -= power;
    ((MTGCardInstance *)target)->addToToughness(-toughness);
    return 1;
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "APowerToughnessModifier ::: power : " << power
	<< " ; toughness : " << toughness
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
};

// Permanent life alteration evry turn of the target's controller. Useful only for unstable mutation currently
class APowerToughnessModifierRegularCounter:public MTGAbility{
 public:
  int power, toughness;
  int phase;
 APowerToughnessModifierRegularCounter(int id, MTGCardInstance * _source, MTGCardInstance * _target, int _phase, int _power, int _toughness):MTGAbility(id,_source,_target),power(_power),toughness(_toughness), phase(_phase){
  }

  void Update(float dt){
    if (newPhase !=currentPhase && newPhase==phase && game->currentPlayer==((MTGCardInstance *)target)->controller()){
      ((MTGCardInstance *)target)->power += power;
      ((MTGCardInstance *)target)->addToToughness(toughness);
    }
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "APowerToughnessModifierRegularCounter ::: power : " << power
	<< " ; toughness : " << toughness
	<< " ; phase : " << phase
	<< " (";
    return MTGAbility::toString(out) << ")";
  }

};


//Alteration of Power and Toughness until end of turn (TargetAbility)
// Gives +n/+m until end of turn to any card that's a target
class ATargetterPowerToughnessModifierUntilEOT: public TargetAbility{
 public:
  MTGCardInstance * mTargets[50];
  int nbTargets;
  int power, toughness;

 ATargetterPowerToughnessModifierUntilEOT(int _id, MTGCardInstance * _source, int _power, int _toughness,  ManaCost * _cost, TargetChooser * _tc = NULL, int doTap=1):TargetAbility(_id,_source,_tc,_cost,0,doTap),power(_power),toughness(_toughness){
    if (!tc) tc = NEW CreatureTargetChooser(_source);
    nbTargets = 0;
  }


  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP){
      for (int i = 0; i < nbTargets; i++){
	MTGCardInstance * mTarget = mTargets[i];
	if(mTarget){
	  mTarget->power-=power;
	  mTarget->addToToughness(-toughness);
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
      mTarget->power+= power;
      mTarget->addToToughness(toughness);
      nbTargets++;
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ATargetterPowerToughnessModifierUntilEOT ::: mTargets : " << mTargets
	<< " ; nbTargets : " << nbTargets
	<< " ; power : " << power
	<< " ; toughness : " << toughness
	<< " (";
    return TargetAbility::toString(out) << ")";
  }

};



//Alteration of Power and Toughness until end of turn (Aura)
class APowerToughnessModifierUntilEndOfTurn: public ActivatedAbility{
 public:
  int power, toughness;
  int counters;
  int maxcounters;
 APowerToughnessModifierUntilEndOfTurn(int id, MTGCardInstance * _source, MTGCardInstance * _target, int _power, int _toughness,  ManaCost * _cost, int _maxcounters = 0):ActivatedAbility(id,_source,_cost,0,0),power(_power),toughness(_toughness),maxcounters(_maxcounters){
    counters = 0;
    target=_target;
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP){
      while(counters){
	((MTGCardInstance *)target)->power -= power;
	((MTGCardInstance *)target)->addToToughness(-toughness);
	counters--;
      }
    }
    ActivatedAbility::Update(dt);
  }

  int fireAbility(){
    return resolve();
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    if (!ActivatedAbility::isReactingToClick(card,mana)) return 0;
    return (!maxcounters || (counters < maxcounters));
  }

  int resolve(){
    ((MTGCardInstance *)target)->power += power;
    ((MTGCardInstance *)target)->addToToughness(toughness);
    counters++;
    return 1;
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "APowerToughnessModifierUntilEndOfTurn ::: power : " << power
	<< " ; toughness : " << toughness
	<< " ; counters : " << counters
	<< " ; maxcounters : " << maxcounters
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }
};


//Alteration of Power and toughness until end of turn (instant)
class  AInstantPowerToughnessModifierUntilEOT: public InstantAbility{
 public:
  int power, toughness;
 AInstantPowerToughnessModifierUntilEOT(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _power, int _toughness): InstantAbility(_id, _source, _target), power(_power), toughness(_toughness){
  }

  int resolve(){
    ((MTGCardInstance *)target)->power +=power;
    ((MTGCardInstance *)target)->addToToughness(toughness);
    return 1;
  }

  int destroy(){
    ((MTGCardInstance *)target)->power -=power;
    ((MTGCardInstance *)target)->addToToughness(-toughness);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "APowerToughnessModifierUntilEndOfTurn ::: power : " << power
	<< " ; toughness : " << toughness
	<< " (";
    return InstantAbility::toString(out) << ")";
  }
};


//Untap Blockers with simple Mana Mechanism
class AUntapManaBlocker: public Blocker{
 public:
 AUntapManaBlocker(int id, MTGCardInstance * card, ManaCost * _cost):Blocker(id, card, _cost){
  }

 AUntapManaBlocker(int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost):Blocker(id, card,_target, _cost){
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "AUntapManaBlocker ::: (";
    return Blocker::toString(out) << ")";
  }
};

/* Spell Counters (Enchantment) for a mana cost */
//LifeForce
class ASpellCounterEnchantment:public TargetAbility{
 public:

 ASpellCounterEnchantment(int _id, MTGCardInstance * _source, ManaCost * _cost,int color = -1, int _tap = 0):TargetAbility(_id,_source,NEW SpellTargetChooser(_source,color),_cost,0,_tap){
  }

  int resolve(){
    Spell * _target = tc->getNextSpellTarget();
    if(_target){
      game->mLayers->stackLayer()->Fizzle(_target);
      return 1;
    }
    return 0;
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "ASpellCounterEnchantment ::: (";
    return TargetAbility::toString(out) << ")";
  }

};


//Circle of Protections
class ACircleOfProtection: public TargetAbility{
 public:
 ACircleOfProtection(int _id, MTGCardInstance * source, int _color):TargetAbility(_id,source,NEW DamageTargetChooser(source,_color),NEW ManaCost(),0,0){
    cost->add(Constants::MTG_COLOR_ARTIFACT,1);
  }

  int resolve(){
    Damage * damage = tc->getNextDamageTarget();
    if (!damage) return 0;
    game->mLayers->stackLayer()->Fizzle(damage);
    return 1;
  }
  virtual ostream& toString(ostream& out) const
  {
    out << "ACircleOfProtection ::: (";
    return TargetAbility::toString(out) << ")";
  }
};

//Basic regeneration mechanism for a Mana cost
class AStandardRegenerate:public ActivatedAbility{
 public:
 AStandardRegenerate(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost):ActivatedAbility(_id,_source,_cost,0,0){
    target = _target;
    aType = MTGAbility::STANDARD_REGENERATE;
  }

  int resolve(){
    MTGCardInstance * _target = (MTGCardInstance *)target;
    _target->regenerate();
    PutInGraveyard * action = ((PutInGraveyard *) game->mLayers->stackLayer()->getNext(NULL,ACTION_PUTINGRAVEYARD,NOT_RESOLVED));
    while(action){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("Fizzling due to regenerate! \n");
#endif
      if (action->card == _target){
	game->mLayers->stackLayer()->Fizzle(action);
      }
      action = ((PutInGraveyard *) game->mLayers->stackLayer()->getNext(action,ACTION_PUTINGRAVEYARD,NOT_RESOLVED));
    }
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
};

/*Gives protection to a target */
class AProtectionFrom:public MTGAbility{
 public:
  CardDescriptor * cd;
  void initProtection(){
    ((MTGCardInstance *)target)->addProtection(cd);
  }

 AProtectionFrom(int _id, MTGCardInstance * _source, MTGCardInstance * _target, CardDescriptor * _cd):MTGAbility(_id, _source, _target),cd(_cd){
    initProtection();
  }
 AProtectionFrom(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int color):MTGAbility(_id, _source, _target){
    cd = NEW CardDescriptor();
    cd->colors[color] = 1;
    initProtection();
  }

  int destroy(){
    ((MTGCardInstance *)target)->removeProtection(cd);
    return 1;
  }

  ~AProtectionFrom(){
    delete(cd);
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AProtectionFrom ::: cd : " << cd
	<< " (";
    return MTGAbility::toString(out) << ")";
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
      if (!onlyIfTargetTapped || ((MTGCardInstance *)target)->tapped){
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
};


//Converts lands to creatures (Kormus bell, Living lands)
class AConvertLandToCreatures:public ListMaintainerAbility{
 public:
  int type;
  int power, toughness;
 AConvertLandToCreatures(int _id, MTGCardInstance * _source, const char * _type, int _power = 1, int _toughness = 1):ListMaintainerAbility(_id, _source),power(_power),toughness(_toughness){
    type = Subtypes::subtypesList->Add(_type);
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
};

//Lords (Merfolk lord...) give power and toughness to OTHER creatures of their type, they can give them special abilities, regeneration
class ALord:public ListMaintainerAbility{
 public:
  int power, toughness;
  int ability;
  int modifier;
  ManaCost * regenCost;
  int includeSelf;
  map<MTGCardInstance *, MTGAbility *> regenerations;
 ALord(int _id, MTGCardInstance * card, TargetChooser * _tc, int _includeSelf, int _power = 0 , int _toughness = 0, int _ability = -1, ManaCost * _regenCost = NULL, int _modifier = 1):ListMaintainerAbility(_id,card){
    tc = _tc;
    tc->source = NULL;
    includeSelf = _includeSelf;
    power = _power;
    toughness = _toughness;
    ability = _ability;
    regenCost = _regenCost;
    modifier = _modifier;
    if (!modifier) modifier = -1;
  }

  int canBeInList(MTGCardInstance * card){
    if ( (includeSelf || card!=source) && tc->canTarget(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
    card->power += power;
    card->addToToughness(toughness);
    if (ability != -1) card->basicAbilities[ability] +=modifier;
    if (regenCost){
      ManaCost * _regenCost = NEW ManaCost(regenCost);
      AStandardRegenerate * regen = NEW AStandardRegenerate(0, card, card, _regenCost);
      regenerations[card] = regen;
      game->addObserver(regen);
    }
    return 1;
  }

  int removed(MTGCardInstance * card){
    card->power -= power;
    card->addToToughness(-toughness);
    if (ability != -1 && ((card->basicAbilities[ability]>0 && (modifier >0)) || (card->basicAbilities[ability]<=0 && (modifier <0)) )) card->basicAbilities[ability] -=modifier;
    if (regenCost){
      if(regenerations.find(card) != regenerations.end()){
	      if (game->isInPlay(card)) game->removeObserver(regenerations[card]);
	      regenerations.erase(card);
      }
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ALord ::: power : " << power
	<< " ; toughness : " << toughness
	<< " ; ability : " << ability
	<< " ; modifier : " << modifier
	<< " ; regenCost : " << regenCost
	<< " ; includeSelf : " << includeSelf
	<< " (";
    return ListMaintainerAbility::toString(out) << ")";
  }
};


class ALordUEOT:public ALord{
public:
  ALordUEOT(int _id, MTGCardInstance * card, TargetChooser * _tc, int _includeSelf, int _power = 0 , int _toughness = 0, int _ability = -1, ManaCost * _regenCost = NULL, int _modifier = 1): ALord(_id, card,  _tc, _includeSelf,  _power  ,  _toughness , _ability , _regenCost,  _modifier){
  }


   int testDestroy(){
    if (newPhase == Constants::MTG_PHASE_AFTER_EOT) return 1;
    return 0;
  }
   virtual ostream& toString(ostream& out) const
   {
     out << "ALordUEOT ::: (";
     return ALord::toString(out) << ")";
   }
};

//Foreach (plague rats...)
class AForeach:public ListMaintainerAbility{
 public:
  int power, toughness;
  int includeSelf;
 AForeach(int _id, MTGCardInstance * card,MTGCardInstance * _target, TargetChooser * _tc, int _includeSelf, int _power = 0 , int _toughness = 0):ListMaintainerAbility(_id,card,_target){
    tc = _tc;
    tc->source = NULL;
    includeSelf = _includeSelf;
    power = _power;
    toughness = _toughness;
    if (!target) target = source; //Is this needed ?
  }

  int canBeInList(MTGCardInstance * card){
    if ( (includeSelf || card!=source) && tc->canTarget(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
    MTGCardInstance * _target = (MTGCardInstance *)target;
    _target->power += power;
    _target->addToToughness(toughness);
    return 1;
  }

  int removed(MTGCardInstance * card){
    MTGCardInstance * _target = (MTGCardInstance *)target;
    _target->power -= power;
    _target->addToToughness(-toughness);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AForeach ::: power : " << power
	<< " ; toughness : " << toughness
	<< " ; includeSelf : " << includeSelf
	<< " (";
    return ListMaintainerAbility::toString(out) << ")";
  }

};

//Damage all.... ActivatedAbility
class AAllDamager:public ActivatedAbility{
 public:
  int damage;
  AAllDamager(int _id, MTGCardInstance * _source, ManaCost * _cost, int _damage, TargetChooser * _tc ,int doTap =1):ActivatedAbility(_id,_source,_cost,0,doTap),damage(_damage){
    tc = _tc;

  }

  int resolve(){
    AbilityFactory af;
    af.damageAll(tc,damage);
    return 1;
  }

  const char * getMenuText(){
    return "Damage All...";
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AAllDamager ::: damage : " << damage
	<< " (";
    return ActivatedAbility::toString(out) << ")";
  }

};


/* Standard Damager, can choose a NEW target each time the price is paid */
class ADamager:public TargetAbility{
 public:
  int damage;
 ADamager(int id, MTGCardInstance * card, ManaCost * _cost, int _damage, TargetChooser * _tc = NULL, int _tap = 1):TargetAbility(id,card, _tc, _cost,0,_tap),damage(_damage){
    if (!tc) tc = NEW DamageableTargetChooser(card);
    aType = MTGAbility::DAMAGER;
  }
  int resolve(){
    Damageable * _target = tc->getNextDamageableTarget();
    GameObserver::GetInstance()->mLayers->stackLayer()->addDamage(source,_target, damage);
    return 1;
  }

  const char * getMenuText(){
    return "Damage target";
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADamager ::: damage : " << damage
	<< " (";
    return TargetAbility::toString(out) << ")";
  }
};

/* Can tap a target for a cost */
class ATapper:public TargetAbility{
 public:
  int damage;
 ATapper(int id, MTGCardInstance * card, ManaCost * _cost, TargetChooser * _chooser):TargetAbility(id,card, _chooser, _cost){
  }

  int resolve(){
    MTGCardInstance * _target = tc->getNextCardTarget();
    if (_target){
      _target->tapped = true;
    }
    return 1;
  }

  const char * getMenuText(){
    return "Tap target";
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ATapper ::: damage : " << damage
	<< " (";
    return TargetAbility::toString(out) << ")";
  }
};

//Ability to untap a target
class AUntaper:public TargetAbility{
 public:
 AUntaper(int _id, MTGCardInstance * card, ManaCost * _manacost, TargetChooser * _tc):TargetAbility(_id,card,_tc,_manacost){
  }

  int resolve(){
    MTGCardInstance * _target = tc->getNextCardTarget();
    if (_target){
      _target->tapped = 0;
    }
    return 1;
  }

  const char * getMenuText(){
    return "Untap target";
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AUntaper ::: (";
    return TargetAbility::toString(out) << ")";
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
	source->attacker=0;
	source->tapped = 0;
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
};

//Steal control of a target
class AControlStealAura: public MTGAbility{
 public:
  Player  * originalController;
 AControlStealAura(int _id , MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source, _target){
    originalController = _target->controller();
    _target->changeController(game->currentlyActing());
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
};




//Same as StealControl Aura ???? Obsolete ?
class ATakeControlAura:public MTGAbility{
 public:
  Player * previousController;
 ATakeControlAura(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source,_target){
    previousController = _target->controller();
    previousController->game->putInZone(_target, previousController->game->inPlay, source->controller()->game->inPlay);

  }

  int destroy(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    Player * p = _target->controller();
    if (p && p->game->inPlay->hasCard(_target)){
      p->game->putInZone(_target, p->game->inPlay, previousController->game->inPlay);
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ATakeControlAura ::: previousController : " << previousController
	<< " (";
    return MTGAbility::toString(out) << ")";
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
};

//Converts a card to a creature (Aura)
class AConvertToCreatureAura:public MTGAbility{
 public:
 AConvertToCreatureAura(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _power, int _toughness):MTGAbility(_id, _source, _target){
    _target->setSubtype("creature");
    _target->power = _power;
    _target->toughness = _toughness;
    _target->life = _toughness;
    //_target->afterDamage();
    _target->doDamageTest = 1;
  }

  int destroy(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    _target->removeType("creature");
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AConvertToCreatureAura ::: (";
    return MTGAbility::toString(out) << ")";
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

 AAladdinsLamp(int _id, MTGCardInstance * card):TargetAbility(_id,card){
    cost = NEW ManaCost();
    cost->x();
    cd = CardDisplay(1,game,SCREEN_WIDTH/2, SCREEN_HEIGHT/2,NULL);
    MTGGameZone * zones[] = {game->currentPlayer->game->library};
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
    source->tapped = 1;
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
};




//Ankh of Mishra
class AAnkhOfMishra: public ListMaintainerAbility{
 public:
  int init;
 AAnkhOfMishra(int id, MTGCardInstance * _source):ListMaintainerAbility(id, _source){
    init = 0;
  }

  void Update(float dt){
    ListMaintainerAbility::Update(dt);
    init = 1;
  }

  int canBeInList(MTGCardInstance * card){
    if (card->hasType("land") && game->isInPlay(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
    if (!init) return 0;
    game->mLayers->stackLayer()->addDamage(source,card->controller(), 2);
    return 1;
  }

  int removed(MTGCardInstance * card){
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AAnkhOfMishra ::: init : " << init
	<< " (";
    return ListMaintainerAbility::toString(out) << ")";
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
};


//Black Vise
class ABlackVise: public MTGAbility{
 public:
  int nbcards;
 ABlackVise(int id, MTGCardInstance * _source):MTGAbility(id, _source){
    nbcards = game->opponent()->game->hand->nb_cards;
  }

  void Update(float dt){
    if (newPhase == Constants::MTG_PHASE_UPKEEP && GameObserver::GetInstance()->opponent()->game->inPlay->hasCard(source)){
      nbcards = game->currentPlayer->game->hand->nb_cards;
    }
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_DRAW && GameObserver::GetInstance()->opponent()->game->inPlay->hasCard(source)){
      if ( nbcards > 4) game->mLayers->stackLayer()->addDamage(source,game->currentPlayer, nbcards - 4);
    }
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ABlackVise ::: nbcards : " << nbcards
	<< " (";
    return MTGAbility::toString(out) << ")";
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
    ((MTGCardInstance *)target)->tapped = 1;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AClockworkBeast ::: counters : " << counters
	<< " ; cost : " << cost
	<< " (";
    return MTGAbility::toString(out) << ")";
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
    source->tapped = 1;
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
};


//Creature bond
class ACreatureBond:public MTGAbility{
 public:
  int resolved;
 ACreatureBond(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id,_source,_target){
    resolved = 0;
  }

 int receiveEvent(WEvent * event){
   MTGCardInstance * _target = (MTGCardInstance *) target;
   if (event->type == WEvent::CHANGE_ZONE){
      WEventZoneChange * e = (WEventZoneChange *) event;
      MTGCardInstance * card = e->card->previous;
      if (card == _target){
        for (int i = 0; i < 2 ; i++){
          Player * p = game->players[i];
          if (e->to == p->game->graveyard){
            game->mLayers->stackLayer()->addDamage(source,_target->controller(),_target->toughness);
            return 1;
          }
        }
      }
   }
    return 0;
 }

 virtual ostream& toString(ostream& out) const
 {
   out << "ACreatureBond ::: resolved : " << resolved
       << " (";
   return MTGAbility::toString(out) << ")";
 }
};

//1105: Dingus Egg
class ADingusEgg: public ListMaintainerAbility{
 public:
 ADingusEgg(int id, MTGCardInstance * _source):ListMaintainerAbility(id, _source){
  }

  int canBeInList(MTGCardInstance * card){
    if (card->hasType("land") && game->isInPlay(card)) return 1;
    return 0;
  }

  int added(MTGCardInstance * card){
    return 1;
  }

  int removed(MTGCardInstance * card){
    game->mLayers->stackLayer()->addDamage(source,card->controller(), 2);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADingusEgg ::: (";
    return ListMaintainerAbility::toString(out) << ")";
  }
};



//1106 DisruptingScepter
class ADisruptingScepter:public TargetAbility{
 public:
 ADisruptingScepter(int id, MTGCardInstance * _source):TargetAbility(id,_source){
    MTGGameZone * zones[] = {GameObserver::GetInstance()->opponent()->game->hand};
    tc = NEW TargetZoneChooser(zones,1,_source);
    int _cost[] = {Constants::MTG_COLOR_ARTIFACT, 3};
    cost = NEW ManaCost(_cost,1);
  }

  void Update(float dt){
    if (game->opponent()->isAI()){
      if(waitingForAnswer){
	MTGCardInstance * card = ((AIPlayer *)game->opponent())->chooseCard(tc, source);
	if (card) tc->toggleTarget(card);
	if (!card || tc->targetsReadyCheck() == TARGET_OK) waitingForAnswer = 0;
      }
      TargetAbility::Update(dt);
    }else{
      TargetAbility::Update(dt);
    }
  }

  int resolve(){
    game->opponent()->game->putInGraveyard(tc->getNextCardTarget());
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADisruptingScepter ::: (";
    return TargetAbility::toString(out) << ")";
  }
};


//1108 Ebony Horse
class AEbonyHorse:public TargetAbility{
 public:

 AEbonyHorse(int _id, MTGCardInstance * _source):TargetAbility(_id,_source, NEW CreatureTargetChooser()){
    int _cost[] = {Constants::MTG_COLOR_ARTIFACT, 2};
    cost = NEW ManaCost(_cost,1);
  }

  int resolve(){
    tc->getNextCardTarget()->attacker =  0;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AEbonyHorse ::: (";
    return TargetAbility::toString(out) << ")";
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
};

//1110 Glasses of Urza
class AGlassesOfUrza:public MTGAbility{
 public:
  CardDisplay * display;
  bool isActive;
 AGlassesOfUrza(int _id, MTGCardInstance * _source):MTGAbility(_id, _source),isActive(false){
    display = NEW CardDisplay(0, game,SCREEN_WIDTH/2, SCREEN_HEIGHT/2,NULL);
  }

  void Update(float dt){
    if(isActive){
      display->Update(dt);
    }
  }

  bool CheckUserInput(u32 key){
    if (isActive){
      if (display->CheckUserInput(key)) return true;
      if (PSP_CTRL_CROSS == key){
	isActive = false;
	return true;
      }
    }
    return false;
  }

  void Render(float dt){
    if (isActive){
      display->Render();
    }

  }
  int isReactingToClick(MTGCardInstance *  card, ManaCost * mana = NULL){
    if ( card == source){
      if (game->currentlyActing()->game->isInPlay(card) && !source->isTapped()){
	return 1;
      }
    }
    return 0;
  }

  int reactToClick(MTGCardInstance * card){
    if (!isReactingToClick(card)) return 0;
    source->tapped = 1;
    isActive = true;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AGlassesOfUrza ::: display : " << display
	<< " ; isActive : " << isActive
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
};

//1112 Howling Mine
class AHowlingMine:public MTGAbility{
 public:
 AHowlingMine(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){}

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_DRAW && !source->tapped){
      game->mLayers->stackLayer()->addDraw(game->currentPlayer);
    }
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AHowlingMine ::: (";
    return MTGAbility::toString(out) << ")";
  }
};

//1119 Jayemdae Tome
class AJayemdaeTome:public ActivatedAbility{
 public:
 AJayemdaeTome(int _id, MTGCardInstance * card):ActivatedAbility(_id, card){
    int _cost[] = {Constants::MTG_COLOR_ARTIFACT, 4};
    cost = NEW ManaCost(_cost,1);
  }

  int resolve(){
    game->mLayers->stackLayer()->addDraw(source->controller());
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AJayemdaeTome ::: (";
    return ActivatedAbility::toString(out) << ")";
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

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP) usedThisTurn = 0;
    Damage * damage = ((Damage *)game->mLayers->stackLayer()->getNext(latest,ACTION_DAMAGE,RESOLVED_OK));
    while (damage){
      if (damage->target == source->controller()){
	counters += damage->damage;
      }
      latest = damage;
      damage = ((Damage *)game->mLayers->stackLayer()->getNext(damage,ACTION_DAMAGE,RESOLVED_OK));
    }
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
};
//1143 Animate Dead
class AAnimateDead:public MTGAbility{
 public:
 AAnimateDead(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source, _target){
    MTGCardInstance * card =  _target;

    //Put the card in play again, with all its abilities !
    //AbilityFactory af;
    MTGCardInstance * copy = source->controller()->game->putInZone(card,  _target->controller()->game->graveyard, source->controller()->game->stack);
    Spell * spell = NEW Spell(copy);
    //af.addAbilities(game->mLayers->actionLayer()->getMaxId(), spell);

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
};


//1159 Erg Raiders
class AErgRaiders:public MTGAbility{
 public:
  int init;
  int dealDamage;
 AErgRaiders(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
    init = 0;
    dealDamage = 0;
  }

  void Update(float dt){
    if (newPhase != currentPhase){
      Player * controller =  source->controller();
      if (newPhase == Constants::MTG_PHASE_COMBATDAMAGE && game->currentPlayer == controller){
	if (!source->isAttacker() && init){
	  dealDamage = 1;
	}
      }else if (newPhase == Constants::MTG_PHASE_UNTAP && game->currentPlayer != controller){
	if (dealDamage){
	  game->mLayers->stackLayer()->addDamage(source, controller,2);
	}
	init = 1;
	dealDamage = 0;
      }
    }

  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AErgRaiders ::: init : " << init
	<< " ; dealDamage : " << dealDamage
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
};

//Fastbond
class AFastbond:public TriggeredAbility{
 public:
  int alreadyPlayedALand;
  int previous;
 AFastbond(int _id, MTGCardInstance * card):TriggeredAbility(_id, card){
    alreadyPlayedALand = 0;
  }

  void Update(float dt){
    if (newPhase!=currentPhase && newPhase == Constants::MTG_PHASE_UNTAP){
      alreadyPlayedALand = 0;
    }
    TriggeredAbility::Update(dt);
  }

  int trigger(){
    if(source->controller()->canPutLandsIntoPlay==0 && previous ==1){
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
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AFastbond ::: alreadyPlayedALand : " << alreadyPlayedALand
	<< " ; previous : " << previous
	<< " (";
    return TriggeredAbility::toString(out) << ")";
  }
};



//1165 Hypnotic Specter
class AHypnoticSpecter:public MTGAbility{
 public:
  int nbdamagesthisturn[2];
 AHypnoticSpecter(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
    currentPhase = -1;
    for (int i = 0; i < 2; i++){
      nbdamagesthisturn[i] = 0;
    }
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP){
      for (int i = 0; i < 2; i++){
	nbdamagesthisturn[i] = 0;
      }
    }

    ActionStack * as = game->mLayers->stackLayer();
    int nbdamages[2];
    for (int i = 0; i < 2; i++){
      nbdamages[i] = 0;
    }

    Damage * current = ((Damage *)as->getNext(NULL,ACTION_DAMAGE,RESOLVED_OK));
    while(current){
      if (current->source == source){
	for (int j=0; j < 2; j++){
	  if(current->target == game->players[j]) nbdamages[j]++;
	}
      }
      current = ((Damage *)as->getNext(current,ACTION_DAMAGE,RESOLVED_OK));

    }

    for (int i = 0; i < 2; i++){
      while(nbdamages[i] > nbdamagesthisturn[i]){
	nbdamagesthisturn[i]++;
	game->players[i]->game->discardRandom(game->players[i]->game->hand);
      }
    }


  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AHypnoticSpecter ::: nbdamagesthisturn : " << nbdamagesthisturn
	<< " (";
    return MTGAbility::toString(out) << ")";
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
};


//Kudzu.
//What happens when there are no targets ???
class AKudzu: public TargetAbility{
 public:
  int previouslyTapped;
 AKudzu(int _id, MTGCardInstance * card, MTGCardInstance * _target):TargetAbility(_id,card, NEW TypeTargetChooser("land",card)){
    tc->toggleTarget(_target);
    target = _target;
    previouslyTapped = 0;
    if (_target->tapped) previouslyTapped = 1;
  }


  void Update(float dt){
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if (_target && !_target->tapped){
      previouslyTapped = 0;
    }else if (!previouslyTapped){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("Kudzu Strikes !\n");
#endif
      MTGCardInstance * _target = (MTGCardInstance *)target;
      target = _target->controller()->game->putInGraveyard(_target);
      reactToClick(source); // ????
    }
    TargetAbility::Update(dt);
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
    previouslyTapped = 0;
    if (source->target && source->target->tapped) previouslyTapped = 1;
    return 1;
  }

  int testDestroy(){
    int stillLandsInPlay = 0;
    for (int i = 0; i < 2; i++){
      if (game->players[i]->game->inPlay->hasType("land")) stillLandsInPlay = 1;
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

  virtual ostream& toString(ostream& out) const
  {
    out << "AKudzu ::: previouslyTapped : " << previouslyTapped
	<< " (";
    return TargetAbility::toString(out) << ")";
  }
};

//Millstone
class AMillstone:public TargetAbility{
 public:
 AMillstone(int _id, MTGCardInstance * card):TargetAbility(_id,card, NEW PlayerTargetChooser(), NEW ManaCost()){
    cost->add(Constants::MTG_COLOR_ARTIFACT, 2);
  }

  int resolve(){
    Player * player = tc->getNextPlayerTarget();
    if (!player) return 0;
    MTGLibrary * library = player->game->library;
    for (int i = 0; i < 2; i++){
      if (library->nb_cards)
	player->game->putInZone(library->cards[library->nb_cards-1],library, player->game->graveyard);
    }
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AMillstone ::: (";
    return TargetAbility::toString(out) << ")";
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
	if (inplay->cards[j]->isACreature()) game->mLayers->stackLayer()->addDamage(source,inplay->cards[j],1);
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
};

//Power Surge
class APowerSurge:public TriggeredAbility{
 public:
  int totalLands;
 APowerSurge(int _id, MTGCardInstance * _source):TriggeredAbility(_id,_source){
    totalLands = 0;
  }

  int trigger(){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_EOT){
      //That's ugly but untapped land at the beginning of the turn are opponent's untapped lands at the end of the turn
      totalLands = 0;
      MTGInPlay * inPlay = game->opponent()->game->inPlay;
      for (int i = 0; i < inPlay->nb_cards; i++){
	MTGCardInstance * card = inPlay->cards[i];
	if (!card->tapped && card->hasType("land")){
	  totalLands++;
	}
      }
    }
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UPKEEP && totalLands){
      return 1;
    }
    return 0;
  }

  int resolve(){
    if (totalLands) game->mLayers->stackLayer()->addDamage(source,game->currentPlayer,totalLands);
    totalLands = 0;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "APowerSurge ::: totalLands : " << totalLands
	<< " (";
    return TriggeredAbility::toString(out) << ")";
  }
};

//1175 Royal Assassin
class ARoyalAssassin:public TargetAbility{
 public:

 ARoyalAssassin(int _id, MTGCardInstance * _source):TargetAbility(_id,_source, NEW CreatureTargetChooser()){
  }

  int resolve(){
    MTGCardInstance * _target = tc->getNextCardTarget();
    if(_target && _target->tapped){
      _target->controller()->game->putInGraveyard(_target);
      return 1;
    }
    return 0;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ARoyalAssassin ::: (";
    return TargetAbility::toString(out) << ")";
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
};

//1218 Psychic Venom
class APsychicVenom:public MTGAbility{
 public:
  int tapped;
 APsychicVenom(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source,_target){
    tapped = _target->tapped;
  }

  void Update(float dt){
    MTGCardInstance*  _target = (MTGCardInstance* )target;
    int newState = _target->isTapped();
    if (newState != tapped && newState == 1){
      game->mLayers->stackLayer()->addDamage(source,_target->controller(),2);
    }
    tapped = newState;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "APsychicVenom ::: tapped : " << tapped
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
};


//1221 Serendib Efreet
class ASerendibEfreet:public MTGAbility{
 public:
 ASerendibEfreet(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
  }

  void Update(float dt){
    if (newPhase == Constants::MTG_PHASE_UPKEEP && newPhase != currentPhase && game->currentPlayer == source->controller()){
      game->mLayers->stackLayer()->addDamage(source,game->currentPlayer,1);
    }
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ASerendibEfreet ::: (";
    return MTGAbility::toString(out) << ")";
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
};

//1276 Wanderlust, 1148 Cursed Lands
class AWanderlust:public TriggeredAbility{
 public:
 AWanderlust(int _id, MTGCardInstance * _source, MTGCardInstance * _target):TriggeredAbility(_id,_source, _target){}

  int trigger(){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UPKEEP && ((MTGCardInstance *) target)->controller()==game->currentPlayer){
      return 1;
    }
    return 0;
  }

  int resolve(){
    game->mLayers->stackLayer()->addDamage(source,((MTGCardInstance *) target)->controller(),1);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AWanderlust ::: (";
    return TriggeredAbility::toString(out) << ")";
  }
};


//1284 Dragon Whelp
class ADragonWhelp: public APowerToughnessModifierUntilEndOfTurn{
 public:
 ADragonWhelp(int id, MTGCardInstance * card):APowerToughnessModifierUntilEndOfTurn(id, card, card, 1, 0, NEW ManaCost()){
    cost->add(Constants::MTG_COLOR_RED, 1);
  }

  void Update(float dt){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP && counters > 3){
      source->controller()->game->putInGraveyard(source);
    }
    APowerToughnessModifierUntilEndOfTurn::Update(dt);
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADragonWhelp ::: (";
    return APowerToughnessModifierUntilEndOfTurn::toString(out) << ")";
  }
};

//1288 EarthBind
class AEarthbind:public ABasicAbilityModifier{
 public:
 AEarthbind(int _id, MTGCardInstance * _source, MTGCardInstance * _target):ABasicAbilityModifier(_id,_source,_target,Constants::FLYING,0){
    if (value_before_modification) game->mLayers->stackLayer()->addDamage(source,target,2);
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AEarthbind ::: (";
    return ABasicAbilityModifier::toString(out) << ")";
  }
};

//1291 Fireball
class AFireball:public InstantAbility{
 public:
 AFireball(int _id, MTGCardInstance * card, Spell * spell, int x):InstantAbility(_id, card){
    int nbtargets = spell->cursor;
    int totaldamage = x+1-nbtargets;
    int individualdamage  = totaldamage / nbtargets;
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
};




//1309 Orcish Artilery
class AOrcishArtillery: public ADamager{
 public:
 AOrcishArtillery(int _id,MTGCardInstance * card): ADamager(_id, card, NEW ManaCost(), 2){
  }

  int resolve(){
    ADamager::resolve();
    game->mLayers->stackLayer()->addDamage(source,source->controller(), 3);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AOrcishArtillery ::: (";
    return ADamager::toString(out) << ")";
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
	if (card->isAttacker() && !card->basicAbilities[Constants::FLYING] && !card->basicAbilities[Constants::ISLANDWALK]) card->attacker=0;
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
};

//1352 Karma
class AKarma: public TriggeredAbility{
 public:
 AKarma(int _id, MTGCardInstance * _source):TriggeredAbility(_id, _source){
  }

  int trigger(){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UPKEEP) return 1;
    return 0;
  }

  int resolve(){
    int totaldamage = 0;
    MTGGameZone *  zone = game->currentPlayer->game->inPlay;
    for (int i = 0; i < zone->nb_cards; i++){
      if (zone->cards[i]->hasType("swamp")) totaldamage++;;
    }
    if (totaldamage) game->mLayers->stackLayer()->addDamage(source,game->currentPlayer, totaldamage);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AKarma ::: (";
    return TriggeredAbility::toString(out) << ")";
  }
};


//Soul Net
class ASoulNet:public ActivatedAbility{
 public:
  PutInGraveyard * latest;
  PutInGraveyard * newDead;
 ASoulNet(int _id, MTGCardInstance * card):ActivatedAbility(_id, card,0,0,0){
    int _cost[] = {Constants::MTG_COLOR_ARTIFACT, 1};
    cost = NEW ManaCost(_cost,1);
    latest = ((PutInGraveyard *) GameObserver::GetInstance()->mLayers->stackLayer()->getPrevious(NULL,ACTION_PUTINGRAVEYARD,RESOLVED_OK));
    newDead = latest;
  }

  int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){
    newDead = ((PutInGraveyard *) GameObserver::GetInstance()->mLayers->stackLayer()->getPrevious(NULL,ACTION_PUTINGRAVEYARD,RESOLVED_OK));
    if (newDead && newDead != latest && newDead->card->isACreature())
      return ActivatedAbility::isReactingToClick(card,mana);
    return 0;
  }
  int resolve(){
    latest = newDead;
    source->controller()->life++;
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ASoulNet ::: latest : " << latest
	<< " ; newDead : " << newDead
	<< " (";
    return ActivatedAbility::toString(out) << ")";
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
};



//--------------Addon Abra------------------

//Basilik
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
};


//GenericMillstone
class ADeplete:public TargetAbility{
 public:
	 int nbcards;
	  ADeplete(int _id, MTGCardInstance * card, ManaCost * _cost, int _nbcards = 1,TargetChooser * _tc = NULL, int _tap = 1):TargetAbility(_id,card, _tc, _cost,0,_tap){
      if (!tc) tc= NEW PlayerTargetChooser(card);
	  nbcards = _nbcards;
	  }
  int resolve(){
    Player * player = tc->getNextPlayerTarget();
    if (!player) return 0;
    MTGLibrary * library = player->game->library;
    for (int i = 0; i < nbcards; i++){
      if (library->nb_cards)
	player->game->putInZone(library->cards[library->nb_cards-1],library, player->game->graveyard);
    }
    return 1;
  }
    const char * getMenuText(){
    return "Deplete";
	  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADeplete ::: nbcards : " << nbcards
	<< " (";
    return TargetAbility::toString(out) << ")";
  }
};


class ADiscard:public TargetAbility{
 public:
	 int nbcards;
	  ADiscard(int _id, MTGCardInstance * card, ManaCost * _cost, int _nbcards = 1,TargetChooser * _tc = NULL, int _tap = 1):TargetAbility(_id,card, _tc, _cost,0,_tap){
      if (!tc) tc= NEW PlayerTargetChooser(card);
	  nbcards = _nbcards;
	  }
  int resolve(){
    Player * player = tc->getNextPlayerTarget();
    if (!player) return 0;
	for (int i = 0; i < 2; i++){
	game->players[i]->game->discardRandom(game->players[i]->game->hand);
	}
    return 1;
  }
    const char * getMenuText(){
    return "Discard";
  }

    virtual ostream& toString(ostream& out) const
    {
      out << "ADiscard ::: nbcards : " << nbcards
	  << " (";
      return TargetAbility::toString(out) << ")";
    }
};

// Generic Karma
class ADamageForTypeControlled: public TriggeredAbility{
 public:
	char type[20];
 ADamageForTypeControlled(int _id, MTGCardInstance * _source,const char * _type):TriggeredAbility(_id, _source){
  sprintf(type,"%s",_type);
    }

  int trigger(){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UPKEEP) return 1;
    return 0;
  }

  int resolve(){
    int totaldamage = 0;
    MTGGameZone *  zone = game->currentPlayer->game->inPlay;
    for (int i = 0; i < zone->nb_cards; i++){
      if (zone->cards[i]->hasType(type)) totaldamage++;;
    }
    if (totaldamage) game->mLayers->stackLayer()->addDamage(source,game->currentPlayer, totaldamage);
    return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADamageForTypeControlled ::: type : " << type
	<< " (";
    return TriggeredAbility::toString(out) << ")";
  }
};

// Dreamborn Muse
class ADreambornMuse: public TriggeredAbility{
 public:
	 int nbcards;
 ADreambornMuse(int _id, MTGCardInstance * _source):TriggeredAbility(_id, _source){
 }

  int trigger(){
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UPKEEP) return 1;
    return 0;
  }

  int resolve(){
    int nbcards = game->currentPlayer->game->hand->nb_cards;
    MTGLibrary * library = game->currentPlayer->game->library;
    for (int i = 0; i < nbcards; i++){
      if (library->nb_cards)
	game->currentPlayer->game->putInZone(library->cards[library->nb_cards-1],library,game->currentPlayer->game->graveyard);
	}
	return 1;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ADreambornMuse ::: nbcards : " << nbcards
	<< " (";
    return TriggeredAbility::toString(out) << ")";
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
};

//Abomination Kill blocking creature if white or green
class AAbomination :public MTGAbility{
 public:
  MTGCardInstance * opponents[20];
  int nbOpponents;
  AAbomination (int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
    nbOpponents = 0;
  }

  void Update(float dt){
    if (newPhase != currentPhase){
      if( newPhase == Constants::MTG_PHASE_COMBATDAMAGE){
	nbOpponents = 0;
	MTGCardInstance * opponent = source->getNextOpponent();
	while ((opponent && opponent->hasColor(Constants::MTG_COLOR_GREEN)) || opponent->hasColor(Constants::MTG_COLOR_WHITE)){
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
    out << "AAbomination ::: opponents : " << opponents
	<< " ; nbOpponents : " << nbOpponents
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
};

// GiveLifeForTappedType
class AGiveLifeForTappedType:public MTGAbility{
 public:
  char type[20];
  int nbtypestapped;

  int counttypesTapped(){
    int result = 0;
    MTGInPlay * inplay = source->controller()->opponent()->game->inPlay;
    for (int i = 0; i < inplay->nb_cards; i++){
      MTGCardInstance * card = inplay->cards[i];
      if (card->tapped && card->hasType(type)) result++;
    }
    return result;
  }

 AGiveLifeForTappedType(int _id, MTGCardInstance * source, const char * _type):MTGAbility(_id, source){
    sprintf(type,"%s",_type);{
      nbtypestapped = counttypesTapped();
    }
  }

  void Update(float dt){
    int newcount = counttypesTapped();
    for (int i=0; i < newcount - nbtypestapped; i++){
      source->controller()->life++;
    }
    nbtypestapped = newcount;
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "AGiveLifeForTappedType ::: type : " << type
	<< " ; nbtypestapped : " << nbtypestapped
	<< " (";
    return MTGAbility::toString(out) << ")";
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
	source->tapped = 1;
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
};





//Generic Kird Ape
class AKirdApe:public ListMaintainerAbility{
 public:
  int power;
  int toughness;
  int ability;
  int modifier;
  int includeSelf;
 AKirdApe(int _id, MTGCardInstance * _source, TargetChooser * _tc, int _includeSelf,int _power = 0, int _toughness = 0, int _ability=-1, int _abilityModifier = 1):ListMaintainerAbility(_id, _source){
    power = _power;
    toughness = _toughness;
    tc = _tc;
    includeSelf = _includeSelf;
    ability=_ability;
    modifier = _abilityModifier;
    if (!modifier) modifier = -1;
 }

 int canBeInList(MTGCardInstance * card){
   if ((includeSelf || card!=source) && tc->canTarget(card)) return 1;
   return 0;
 }

 int added(MTGCardInstance * card){
   if (cards.size()== 1){
      source->power+=power;
      source->addToToughness(toughness);
      if (ability != -1) source->basicAbilities[ability] +=modifier;
      return 1;
   }
   return 0;
 }

 int removed(MTGCardInstance * card){

   if (cards.size()== 0){
      source->power-=power;
      source->addToToughness(-toughness);
      if ((ability != -1) && source->basicAbilities[ability] >0 ) source->basicAbilities[ability] -=modifier;
      return 1;
   }
   return 0;
 }


 virtual ostream& toString(ostream& out) const
 {
   out << "AKirdApe ::: power : " << power
       << " ; toughness : " << toughness
       << " ; ability : " << ability
       << " ; modifier : " << modifier
       << " ; includeSelf : " << includeSelf
       << " (";
   return ListMaintainerAbility::toString(out) << ")";
 }
};

//Rampage ability
class ARampageAbility:public MTGAbility{
 public:
  MTGCardInstance * opponents[20];
  int nbOpponents;
  int PowerModifier;
  int ToughnessModifier;
  int MaxOpponent;

 ARampageAbility(int _id, MTGCardInstance * _source,int _PowerModifier, int _ToughnessModifier, int _MaxOpponent):MTGAbility(_id, _source){
    PowerModifier = _PowerModifier;
    ToughnessModifier = _ToughnessModifier;
	MaxOpponent = _MaxOpponent;
  }
  void Update(float dt){
    if (source->isAttacker()){
      if (newPhase != currentPhase){
	if( newPhase == Constants::MTG_PHASE_COMBATDAMAGE){
	  nbOpponents = 0;
	  MTGCardInstance * opponent = source->getNextOpponent();
	  while (opponent){
	    opponents[nbOpponents] = opponent;
	    nbOpponents ++;
		opponent = source->getNextOpponent(opponent);
		if (nbOpponents > MaxOpponent){
			source->power+= PowerModifier;
			source->addToToughness(ToughnessModifier);

		}
	  }
	}
	if( newPhase == Constants::MTG_PHASE_AFTER_EOT ){
		for (int i = 0; i < nbOpponents; i++){
			source->power-= PowerModifier;
			source->addToToughness(-ToughnessModifier);
		}
	}
	  }
	}
  }

  virtual ostream& toString(ostream& out) const
  {
    out << "ARampageAbility ::: opponents : " << opponents
	<< " ; nbOpponents : " << nbOpponents
	<< " ; PowerModifier : " << PowerModifier
	<< " ; ToughnessModifier : " << ToughnessModifier
	<< " ; MaxOpponent : " << MaxOpponent
	<< " (";
    return MTGAbility::toString(out) << ")";
  }
};

#endif
