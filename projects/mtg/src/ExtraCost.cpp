#include "../include/DebugRoutines.h"
#include "../include/ExtraCost.h"
#include "../include/TargetChooser.h"
#include "../include/MTGCardInstance.h"
#include "../include/Translate.h"
#include "../include/config.h"
#include "../include/Player.h"
#include "../include/Counters.h"
#include <JGE.h>

ExtraCost::ExtraCost( TargetChooser *_tc):tc(_tc){

}

ExtraCost::~ExtraCost(){
  SAFE_DELETE(tc);
}

int ExtraCost::setSource(MTGCardInstance * _source){
  source=_source;
  if (tc){ tc->source = _source; tc->targetter = _source;}
  return 1;
}
//life cost
LifeCost *  LifeCost::clone() const{
  LifeCost * ec =  NEW LifeCost(*this);
  if (tc) ec->tc = tc->clone();
  return ec;
}
LifeCost::LifeCost(TargetChooser *_tc):ExtraCost(_tc){
  if (tc) tc->targetter = NULL; 
  target = NULL;
}

int LifeCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
  if (tc) tc->targetter = NULL;
  if (!tc) target = card;
  return 1;
}
int LifeCost::setPayment(MTGCardInstance * card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      target = card;
      return result;
    }
  }
  return 0;
}
int LifeCost::isPaymentSet(){
  if (target) return 1;
  return 0;
}

int LifeCost::canPay(){
  return 1;
}
int LifeCost::doPay(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(target){
      _target->controller()->life -= 1;
    target = NULL;
    if (tc) tc->initTargets();
    return 1;
  }
  return 0;
}


void LifeCost::Render(){
  //TODO : real stuff
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("Life").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}
//endlifecost
//discard a card at random as a cost
//DiscardRandom cost
DiscardRandomCost *  DiscardRandomCost::clone() const{
  DiscardRandomCost * ec =  NEW DiscardRandomCost(*this);
  if (tc) ec->tc = tc->clone();
  return ec;
}
DiscardRandomCost::DiscardRandomCost(TargetChooser *_tc):ExtraCost(_tc){
  if (tc) tc->targetter = NULL; 
  target = NULL;
}

int DiscardRandomCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
  if (tc) tc->targetter = NULL;
  if (!tc) target = card;
  return 1;
}
int DiscardRandomCost::setPayment(MTGCardInstance * card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      target = card;
      return result;
    }
  }
  return 0;
}
int DiscardRandomCost::isPaymentSet(){
  if (target) return 1;
  return 0;
}

int DiscardRandomCost::canPay(){
  MTGGameZone * z = target->controller()->game->hand;
  int nbcards = z->nb_cards;
  if(nbcards < 1) return 0;
  return 1;
}
int DiscardRandomCost::doPay(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(target){
	  _target->controller()->game->discardRandom(_target->controller()->game->hand);
    target = NULL;
    if (tc) tc->initTargets();
    return 1;
  }
  return 0;
}


void DiscardRandomCost::Render(){
  //TODO : real stuff
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("Discard Random").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}
//discardrandomcost

//put a card ontop of your library cost

//ToLibrarycost
ToLibraryCost *  ToLibraryCost::clone() const{
  ToLibraryCost * ec =  NEW ToLibraryCost(*this);
  if (tc) ec->tc = tc->clone();
  return ec;
}

ToLibraryCost::ToLibraryCost(TargetChooser *_tc):ExtraCost(_tc){
  if (tc) tc->targetter = NULL; //tapping targets is not targetting, protections do not apply
  target = NULL;
}

int ToLibraryCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
  if (tc) tc->targetter = NULL; //Tapping targets is not targetting, protections do not apply
  if (!tc) target = card;
  return 1;
}

int ToLibraryCost::setPayment(MTGCardInstance * card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      target = card;
      return result;
    }
  }
  return 0;
}

int ToLibraryCost::isPaymentSet(){
  if (target) return 1;
  return 0;
}

int ToLibraryCost::canPay(){
  //tap target does not have any additional restrictions.
  return 1;
}

int ToLibraryCost::doPay(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(target){
      _target->controller()->game->putInLibrary(target);
    target = NULL;
    if (tc) tc->initTargets();
    return 1;
  }
  return 0;
}
void ToLibraryCost::Render(){
  //TODO : real stuff
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("Put a card on top of Library").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}
//ToLibrarycost

//put a card from top of library into graveyard

//Mill yourself as a cost
MillCost *  MillCost::clone() const{
  MillCost * ec =  NEW MillCost(*this);
  if (tc) ec->tc = tc->clone();
  return ec;
}

MillCost::MillCost(TargetChooser *_tc):ExtraCost(_tc){
  if (tc) tc->targetter = NULL; //tapping targets is not targetting, protections do not apply
  target = NULL;
}

int MillCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
  if (tc) tc->targetter = NULL; //Tapping targets is not targetting, protections do not apply
  if (!tc) target = card;
  return 1;
}

int MillCost::setPayment(MTGCardInstance * card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      target = card;
      return result;
    }
  }
  return 0;
}

int MillCost::isPaymentSet(){
  if (target) return 1;
  return 0;
}

int MillCost::canPay(){
  MTGGameZone * z = target->controller()->game->library;
  int nbcards = z->nb_cards;
  if(nbcards < 1) return 0;
  return 1;
}

int MillCost::doPay(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(target){
	    _target->controller()->game->putInZone(_target->controller()->game->library->cards[_target->controller()->game->library->nb_cards-1],_target->controller()->game->library, _target->controller()->game->graveyard);
    target = NULL;
    if (tc) tc->initTargets();
    return 1;
  }
  return 0;
}
void MillCost::Render(){
  //TODO : real stuff
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("Deplete").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}
//millyourselfcost

//put a card from top of library into exile

//Mill ot exile yourself as a cost
MillExileCost *  MillExileCost::clone() const{
  MillExileCost * ec =  NEW MillExileCost(*this);
  if (tc) ec->tc = tc->clone();
  return ec;
}

MillExileCost::MillExileCost(TargetChooser *_tc):ExtraCost(_tc){
  if (tc) tc->targetter = NULL; //tapping targets is not targetting, protections do not apply
  target = NULL;
}

int MillExileCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
  if (tc) tc->targetter = NULL; //Tapping targets is not targetting, protections do not apply
  if (!tc) target = card;
  return 1;
}

int MillExileCost::setPayment(MTGCardInstance * card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      target = card;
      return result;
    }
  }
  return 0;
}

int MillExileCost::isPaymentSet(){
  if (target) return 1;
  return 0;
}

int MillExileCost::canPay(){
 MTGGameZone * z = target->controller()->game->library;
  int nbcards = z->nb_cards;
  if(nbcards < 1) return 0;
  return 1;
}

int MillExileCost::doPay(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(target){
	    _target->controller()->game->putInZone(_target->controller()->game->library->cards[_target->controller()->game->library->nb_cards-1],_target->controller()->game->library, _target->controller()->game->exile);
    target = NULL;
    if (tc) tc->initTargets();
    return 1;
  }
  return 0;
}
void MillExileCost::Render(){
  //TODO : real stuff
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("Deplete To Exile").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}
//milltoExileyourselfcost

//Tap target cost
TapTargetCost *  TapTargetCost::clone() const{
  TapTargetCost * ec =  NEW TapTargetCost(*this);
  if (tc) ec->tc = tc->clone();
  return ec;
}


TapTargetCost::TapTargetCost(TargetChooser *_tc):ExtraCost(_tc){
  if (tc) tc->targetter = NULL; //tapping targets is not targetting, protections do not apply
  target = NULL;
}

int TapTargetCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
  if (tc) tc->targetter = NULL; //Tapping targets is not targetting, protections do not apply
  if (!tc) target = card;
  return 1;
}

int TapTargetCost::setPayment(MTGCardInstance * card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      target = card;
      return result;
    }
  }
  return 0;
}

int TapTargetCost::isPaymentSet(){
  if (target) return 1;
  return 0;
}

int TapTargetCost::canPay(){
  //tap target does not have any additional restrictions.
  return 1;
}

int TapTargetCost::doPay(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(target){
      _target->tap();
    target = NULL;
    if (tc) tc->initTargets();
    return 1;
  }
  return 0;
}

void TapTargetCost::Render(){
  //TODO : real stuff
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("Tap Target").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}
//endtaptargetcost

//exile as cost
ExileTargetCost *  ExileTargetCost::clone() const{
  ExileTargetCost * ec =  NEW ExileTargetCost(*this);
  if (tc) ec->tc = tc->clone();
  return ec;
}


ExileTargetCost::ExileTargetCost(TargetChooser *_tc):ExtraCost(_tc){
  if (tc) tc->targetter = NULL; //tapping targets is not targetting, protections do not apply
  target = NULL;
}

int ExileTargetCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
  if (tc) tc->targetter = NULL; //Tapping targets is not targetting, protections do not apply
  if (!tc) target = card;
  return 1;
}

int ExileTargetCost::setPayment(MTGCardInstance * card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      target = card;
      return result;
    }
  }
  return 0;
}

int ExileTargetCost::isPaymentSet(){
  if (target) return 1;
  return 0;
}

int ExileTargetCost::canPay(){
  //tap target does not have any additional restrictions.
  return 1;
}

int ExileTargetCost::doPay(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(target){
    target->controller()->game->putInExile(target);
    target = NULL;
    if (tc) tc->initTargets();
    return 1;
  }
  return 0;
}

void ExileTargetCost::Render(){
  //TODO : real stuff
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("Exile Target").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}
//endExiletargetcost
//------------------------------------------------------------
//Bounce as cost
BounceTargetCost *  BounceTargetCost::clone() const{
  BounceTargetCost * ec =  NEW BounceTargetCost(*this);
  if (tc) ec->tc = tc->clone();
  return ec;
}


BounceTargetCost::BounceTargetCost(TargetChooser *_tc):ExtraCost(_tc){
  if (tc) tc->targetter = NULL; //tapping targets is not targetting, protections do not apply
  target = NULL;
}

int BounceTargetCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
  if (tc) tc->targetter = NULL; //Tapping targets is not targetting, protections do not apply
  if (!tc) target = card;
  return 1;
}

int BounceTargetCost::setPayment(MTGCardInstance * card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      target = card;
      return result;
    }
  }
  return 0;
}

int BounceTargetCost::isPaymentSet(){
  if (target) return 1;
  return 0;
}

int BounceTargetCost::canPay(){
  //tap target does not have any additional restrictions.
  return 1;
}

int BounceTargetCost::doPay(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  if(target){
    target->controller()->game->putInHand(target);
    target = NULL;
    if (tc) tc->initTargets();
    return 1;
  }
  return 0;
}

void BounceTargetCost::Render(){
  //TODO : real stuff
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("Return Target to Hand").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}
//endbouncetargetcost
//------------------------------------------------------------
SacrificeCost *  SacrificeCost::clone() const{
  SacrificeCost * ec =  NEW SacrificeCost(*this);
  if (tc) ec->tc = tc->clone();
  return ec;
}


SacrificeCost::SacrificeCost(TargetChooser *_tc):ExtraCost(_tc){
  if (tc) tc->targetter = NULL; //Sacrificing is not targetting, protections do not apply
  target = NULL;
}

int SacrificeCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
  if (tc) tc->targetter = NULL; //Sacrificing is not targetting, protections do not apply
  if (!tc) target = card;
  return 1;
}

int SacrificeCost::setPayment(MTGCardInstance * card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      target = card;
      return result;
	}
  }
  return 0;
}

int SacrificeCost::isPaymentSet(){
  if (target) return 1;
  return 0;
}

int SacrificeCost::canPay(){
  //Sacrifice does not have any additional restrictions.
  return 1;
}

int SacrificeCost::doPay(){
  if(target){
	  target->controller()->game->putInGraveyard(target);
    target = NULL;
    if (tc) tc->initTargets();
    return 1;
  }
  return 0;
}

void SacrificeCost::Render(){
  //TODO : real stuff
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("sacrifice").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}

//Counter costs

CounterCost * CounterCost::clone() const{
  CounterCost * ec =  NEW CounterCost(*this);
  if (tc) ec->tc = tc->clone();
  if (counter) ec->counter = NEW Counter(counter->target, counter->name.c_str(),counter->power, counter->toughness);
  return ec;
}

CounterCost::CounterCost(Counter * _counter,TargetChooser *_tc):ExtraCost(_tc) {
  if (tc) tc->targetter = NULL;
  target = NULL;
  counter = _counter;
  hasCounters = 0;
}

int CounterCost::setSource(MTGCardInstance * _source){
  ExtraCost::setSource(_source);
  if (tc) tc->targetter = NULL;
  if (!tc) target = _source;
  return 1;
}

int CounterCost::setPayment(MTGCardInstance *card){
  if (tc) {
    int result = tc->addTarget(card);
    if (result) {
      if (counter->nb >= 0) return 1; //add counters always possible 
      target = card;
      Counter * targetCounter = target->counters->hasCounter(counter->name.c_str(),counter->power,counter->toughness);
      if (targetCounter && targetCounter->nb >= - counter->nb) {
        hasCounters = 1;
        return result;
      }
    }
  }
  return 0;  
}

int CounterCost::isPaymentSet(){
  if (!target) return 0;
  if (counter->nb >=0) return 1; //add counters always possible
  Counter * targetCounter = target->counters->hasCounter(counter->name.c_str(),counter->power,counter->toughness);
    if (targetCounter && targetCounter->nb >= - counter->nb) {
      hasCounters = 1;  
    }
  if (target && hasCounters) return 1;
  return 0;
}

int CounterCost::canPay(){
  // if target needs to be chosen, then move on.
  if (tc) return 1;
  if (counter->nb >=0) return 1; //add counters always possible
  // otherwise, move on only if target has enough counters
  Counter * targetCounter = target->counters->hasCounter(counter->name.c_str(),counter->power,counter->toughness);
  if (targetCounter && targetCounter->nb >= - counter->nb) return 1;
  return 0;
}

int CounterCost::doPay(){
  if (!target) return 0;

  if (counter->nb >=0) { //Add counters as a cost
    for (int i = 0; i < counter->nb; i++) {
      target->counters->addCounter(counter->name.c_str(),counter->power,counter->toughness);
    }
    return 1;
  }

  //remove counters as a cost
  if (hasCounters) {
    for (int i = 0; i < - counter->nb; i++) {
      target->counters->removeCounter(counter->name.c_str(),counter->power,counter->toughness);
    }
    hasCounters = 0;
    return 1;
  }
  return 0;
}

void CounterCost::Render(){
  WFont * mFont = resources.GetWFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("counters").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}

CounterCost::~CounterCost(){
  SAFE_DELETE(counter);
}

//
//Container
//
ExtraCosts::ExtraCosts(){
  action = NULL;
  source = NULL;
}

ExtraCosts * ExtraCosts::clone() const{
  ExtraCosts * ec =  NEW ExtraCosts(*this);
  ec->costs.clear();
  for (size_t i = 0; i < costs.size(); i++){
    ec->costs.push_back(costs[i]->clone());
  }
  return ec;
}

void ExtraCosts::Render(){
  //TODO cool window and stuff...
  for (size_t i = 0; i < costs.size(); i++){
    costs[i]->Render();
  }
}

int ExtraCosts::setAction(MTGAbility * _action, MTGCardInstance * _card){
  action = _action;
  source = _card;
  for (size_t i = 0; i < costs.size(); i++){
    costs[i]->setSource(_card);
  }
  return 1;
}

int ExtraCosts::reset(){
  action = NULL;
  source = NULL;
  //TODO set all payments to "unset"
  return 1;
}

int ExtraCosts::tryToSetPayment(MTGCardInstance * card){
  for (size_t i = 0; i < costs.size(); i++){
    if (int result = costs[i]->setPayment(card)) return result;
  }
  return 0;
}

int ExtraCosts::isPaymentSet(){
  for (size_t i = 0; i < costs.size(); i++){
    if (!costs[i]->isPaymentSet()) return 0;
  }
  return 1;
}

int ExtraCosts::canPay(){
  for (size_t i = 0; i < costs.size(); i++){
    if (!costs[i]->canPay()) return 0;
  }
  return 1;
}

int ExtraCosts::doPay(){
  int result = 0;
  for (size_t i = 0; i < costs.size(); i++){
    result+=costs[i]->doPay();
  }
  return result;
}

ExtraCosts::~ExtraCosts(){
  for (size_t i = 0; i < costs.size(); i++){
    SAFE_DELETE(costs[i]);
  }
}

void ExtraCosts::Dump(){
  DebugTrace("=====\nDumping ExtraCosts=====\n");
  DebugTrace("NbElements: " << costs.size());
}
