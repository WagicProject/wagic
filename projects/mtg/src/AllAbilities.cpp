#include "PrecompiledHeader.h"
#include "AllAbilities.h"

// BanishCard implementations

AABanishCard::AABanishCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost=NULL, int _banishmentType = -1):ActivatedAbility(_id, _source,_cost),banishmentType(_banishmentType) {
  if (_target) target = _target; 
}

const char * AABanishCard::getMenuText() 
{
  return "Send to graveyard";
}

int AABanishCard::resolve()
{
  DebugTrace("This is not implemented!");
  return 0;
}

AABanishCard * AABanishCard::clone() const{
  AABanishCard * a =  NEW AABanishCard(*this);
  a->isClone = 1;
  return a;
}

// Bury

AABuryCard::AABuryCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL , int _banishmentType = 0):AABanishCard(_id, _source, _target, _cost, AABanishCard::BURY) 
{}

int AABuryCard::resolve(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(_target){
    return _target->bury();
  }
  return 0;
}

const char * AABuryCard::getMenuText(){
  return "Bury";
}

AABuryCard * AABuryCard::clone() const{
  AABuryCard * a =  NEW AABuryCard(*this);
  a->isClone = 1;
  return a;
}


// Destroy

AADestroyCard::AADestroyCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL, int _banishmentType  = 0):AABanishCard(_id, _source, _target, _cost, AABanishCard::DESTROY) 
{}

int AADestroyCard::resolve(){
    MTGCardInstance * _target = (MTGCardInstance *) target;
  if(_target){
    return _target->destroy();
  }
  return 0;
}

const char * AADestroyCard::getMenuText(){
  return "Destroy";
}

AADestroyCard * AADestroyCard::clone() const{
  AADestroyCard * a =  NEW AADestroyCard(*this);
  a->isClone = 1;
  return a;
}

// Sacrifice
AASacrificeCard::AASacrificeCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL, int _banishmentType  = 0):AABanishCard(_id, _source, _target, _cost, AABanishCard::SACRIFICE) {
}

int AASacrificeCard::resolve(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(_target){
    Player * p = _target->controller();
    WEvent * e = NEW WEventCardSacrifice(_target);
    GameObserver * game = GameObserver::GetInstance();
    game->receiveEvent(e);
    p->game->putInGraveyard(_target);
    return 1;
  }
  return 0;
}

const char * AASacrificeCard::getMenuText(){
  return "Sacrifice";
}

AASacrificeCard * AASacrificeCard::clone() const{
  AASacrificeCard * a =  NEW AASacrificeCard(*this);
  a->isClone = 1;
  return a;
}

// Discard 

AADiscardCard::AADiscardCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL, int _banishmentType  = 0):AABanishCard(_id, _source, _target, _cost, AABanishCard::DISCARD) {
}

int AADiscardCard::resolve(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(_target){
    Player * p = _target->controller();
    WEvent * e = NEW WEventCardDiscard(_target);
    GameObserver * game = GameObserver::GetInstance();
    game->receiveEvent(e);
    p->game->putInGraveyard(_target);
    return 1;
  }
  return 0;
}

const char * AADiscardCard::getMenuText(){
  return "Discard";
}

AADiscardCard * AADiscardCard::clone() const{
  AADiscardCard * a =  NEW AADiscardCard(*this);
  a->isClone = 1;
  return a;
}


//Mana Redux
AManaRedux::AManaRedux(int id, MTGCardInstance * source, MTGCardInstance * target,int amount,int type):MTGAbility(id,source,target),amount(amount),type(type) {
  MTGCardInstance * _target = (MTGCardInstance *)target;
}

int AManaRedux::addToGame(){
  MTGCardInstance * _target = (MTGCardInstance *)target;
  if(amount < 0){
    amount = abs(amount);
    if(_target->getManaCost()->hasColor(type)){
      if(_target->getManaCost()->getConvertedCost() >= 1){
        _target->getManaCost()->remove(type,amount);
        if(_target->getManaCost()->alternative > 0){
          _target->getManaCost()->alternative->remove(type,amount);}
        if(_target->getManaCost()->BuyBack > 0){
          _target->getManaCost()->BuyBack->remove(type,amount);}
      }
    }
  }else{
    _target->getManaCost()->add(type,amount);
    if(_target->getManaCost()->alternative > 0){
      _target->getManaCost()->alternative->add(type,amount);}
    if(_target->getManaCost()->BuyBack > 0){
      _target->getManaCost()->BuyBack->add(type,amount);}
  }
  return MTGAbility::addToGame();
}

AManaRedux * AManaRedux::clone() const {
  AManaRedux * a =  NEW AManaRedux(*this);
  a->isClone = 1;
  return a;
}

AManaRedux::~AManaRedux(){}
