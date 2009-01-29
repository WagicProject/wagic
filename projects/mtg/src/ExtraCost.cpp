#include "../include/ExtraCost.h"
#include "../include/TargetChooser.h"
#include "../include/MTGCardInstance.h"
#include <JGE.h>

ExtraCost::ExtraCost( TargetChooser *_tc):tc(_tc){

}


int ExtraCost::setSource(MTGCardInstance * _source){
  source=_source; 
  if (tc){ tc->source = _source;} 
  return 1;
}

SacrificeCost::SacrificeCost(TargetChooser *_tc):ExtraCost(_tc){
  target = NULL;
}

int SacrificeCost::setSource(MTGCardInstance * card){
  ExtraCost::setSource(card);
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
    return 1;
  }
  return 0;
}

void SacrificeCost::Render(){
  //TODO : real stuff
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  char buffer[200];
  sprintf(buffer, "sacrifice");
  mFont->DrawString(buffer, 20 ,20, JGETEXT_LEFT);
}

//
//Container
//
ExtraCosts::ExtraCosts(){
  action = NULL;
  source = NULL;
}

void ExtraCosts::Render(){
  //TODO cool window and stuff...
  for (int i=0; i < costs.size(); i++){
    costs[i]->Render();
  }
}

int ExtraCosts::setAction(MTGAbility * _action, MTGCardInstance * _card){
  action = _action;
  source = _card;
  for (int i=0; i < costs.size(); i++){
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
  for (int i=0; i < costs.size(); i++){
    if (int result = costs[i]->setPayment(card)) return result;
  }
  return 0;
}

int ExtraCosts::isPaymentSet(){
  for (int i=0; i < costs.size(); i++){
    if (!costs[i]->isPaymentSet()) return 0;
  }
  return 1; 
}

int ExtraCosts::doPay(){
  int result = 0;
  for (int i=0; i < costs.size(); i++){
    result+=costs[i]->doPay();
  }
  return result; 
}
void ExtraCosts::Dump(){
#ifdef WIN32
  char buf[4096];
  OutputDebugString("=====\nDumping ExtraCosts=====\n");
  sprintf(buf, "NbElements : %i\n", costs.size());
  OutputDebugString(buf);
#endif
}