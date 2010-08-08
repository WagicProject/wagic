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
#ifdef WIN32
  char buf[4096];
  OutputDebugString("=====\nDumping ExtraCosts=====\n");
  sprintf(buf, "NbElements : %i\n", costs.size());
  OutputDebugString(buf);
#endif
}
