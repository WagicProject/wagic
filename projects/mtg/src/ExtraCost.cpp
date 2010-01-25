#include "../include/ExtraCost.h"
#include "../include/TargetChooser.h"
#include "../include/MTGCardInstance.h"
#include "../include/Translate.h"
#include "../include/config.h"
#include "../include/Player.h"
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
  JLBFont * mFont = resources.GetJLBFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->SetColor(ARGB(255,255,255,255));
  char buffer[200];
  sprintf(buffer, "%s", _("sacrifice").c_str());
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
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
