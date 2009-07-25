#include "../include/config.h"
#include "../include/ActionLayer.h"
#include "../include/GameObserver.h"
#include "../include/Targetable.h"
#include "../include/WEvent.h"

MTGAbility* ActionLayer::getAbility(int type){
  for (int i = 1; i < mCount; i++){
     MTGAbility * a = ((MTGAbility *)mObjects[i]);
     if (a->aType == type){
       return a;
     }
   }
  return NULL;
}

int ActionLayer::reactToClick(ActionElement * ability, MTGCardInstance * card){
  int result = ability->reactToClick(card);
  if (result) stuffHappened = 1;
  return result;
}


int ActionLayer::reactToTargetClick(ActionElement* ability, Targetable * card){
  int result = ability->reactToTargetClick(card);
  if (result) stuffHappened = 1;
  return result;
}

int ActionLayer::unstoppableRenderInProgress(){

  for (int i=0;i<mCount;i++){
    if (mObjects[i]!=NULL){
      ActionElement * currentAction = (ActionElement *)mObjects[i];
      if (currentAction->getActivity() > 0){
	return 1;
      }
    }
  }
  return 0;
}


bool ActionLayer::CheckUserInput(u32 key){
  GameObserver * g = GameObserver::GetInstance();
  if (g->waitForExtraPayment && key == PSP_CTRL_CROSS){
    g->waitForExtraPayment = NULL;
    return 1;
  }
  if (menuObject){
    return false;
  }
  for (int i=0;i<mCount;i++){
    if (mObjects[i]!=NULL){
      ActionElement * currentAction = (ActionElement *)mObjects[i];
      if (currentAction->CheckUserInput(key)) return true;
    }
  }
  return false;
}


void ActionLayer::Update(float dt){
  stuffHappened = 0;
  if (menuObject){
    abilitiesMenu->Update(dt);
    return;
  }
  modal = 0;
  GameObserver * g = GameObserver::GetInstance();
  for (int i=mCount -1 ;i>=0;i--){
    if (mObjects[i]!= NULL){
      ActionElement * currentAction = (ActionElement *)mObjects[i];
      if (currentAction->testDestroy()){
        g->removeObserver(currentAction);
      }
    }
  }
  int newPhase = g->getCurrentGamePhase();
  for (int i=0;i<mCount;i++){
    if (mObjects[i]!=NULL){
      ActionElement * currentAction = (ActionElement *)mObjects[i];
      currentAction->newPhase = newPhase;
      currentAction->Update(dt);
      currentAction->currentPhase = newPhase;
    }
  }

}

void ActionLayer::Render (){
  if (menuObject){
    abilitiesMenu->Render();
    return;
  }
  for (int i=0;i<mCount;i++){
    if (mObjects[i]!=NULL){
      ActionElement * currentAction = (ActionElement *)mObjects[i];
      //if (currentAction->getActivity() > 0){
      currentAction->Render();
      //return;
      //}
    }
  }
}




TargetChooser * ActionLayer::getCurrentTargetChooser(){
  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    if(currentAction->waitingForAnswer) return currentAction->tc;
  }
  return NULL;
}

int ActionLayer::cancelCurrentAction(){
  ActionElement * ae = isWaitingForAnswer();
  if (!ae) return 0;
  ae->waitingForAnswer = 0; //TODO MOVE THIS IS ActionElement
  return 1;
}

ActionElement * ActionLayer::isWaitingForAnswer(){
  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    if(currentAction->waitingForAnswer) return currentAction;
  }
  return NULL;
}

int ActionLayer::stillInUse(MTGCardInstance * card){
  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    if(currentAction->stillInUse(card)) return 1;
  }
  return 0;
}

int ActionLayer::receiveEvent(WEvent * event){
  int result = 0;
  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    result += currentAction->receiveEvent(event);
  }
  return result;
}

int ActionLayer::isReactingToTargetClick(Targetable * card){
  int result = 0;

  if (isWaitingForAnswer()) return -1;

  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    result += currentAction->isReactingToTargetClick(card);
  }
  return result;
}

int ActionLayer::reactToTargetClick(Targetable * card){
  int result = 0;

  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    if(currentAction->waitingForAnswer){
      return reactToTargetClick(currentAction,card);
    }
  }

  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    result += currentAction->reactToTargetClick(card);
  }
  return result;
}

//TODO Simplify with only object !!!
int ActionLayer::isReactingToClick(MTGCardInstance * card){
  int result = 0;

  if (isWaitingForAnswer()) return -1; 


  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    result += currentAction->isReactingToClick(card);
  }

  return result;
}

int ActionLayer::reactToClick(MTGCardInstance * card){
  int result = 0;

  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    if(currentAction->waitingForAnswer){
      return reactToClick(currentAction,card);
    }
  }

  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    OutputDebugString(currentAction->getMenuText());
    result += reactToClick(currentAction,card);
    if (result) return result;
  }
  return result;
}


void ActionLayer::setMenuObject(Targetable * object){
  menuObject = object;

  SAFE_DELETE(abilitiesMenu);

  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  abilitiesMenu = NEW SimpleMenu(10, this, mFont, 100, 100);

  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    if (currentAction->isReactingToTargetClick(object)){
      abilitiesMenu->Add(i,currentAction->getMenuText());
    }
  }
  abilitiesMenu->Add(-1, "Cancel");
  modal = 1;
}

void ActionLayer::doReactTo(int menuIndex){

  if (menuObject){
    int controlid = abilitiesMenu->mObjects[menuIndex]->GetId();
    char buf[4096];
    sprintf(buf, "doReact To %i\n",controlid);
    OutputDebugString(buf);
    ActionElement * currentAction = (ActionElement *)mObjects[controlid];
    currentAction->reactToTargetClick(menuObject);
    menuObject = 0;
  }
}

void ActionLayer::ButtonPressed(int controllerid, int controlid){
  if (controlid == -1){
    
  }else{
    ActionElement * currentAction = (ActionElement *)mObjects[controlid];
    currentAction->reactToTargetClick(menuObject);
  }
  menuObject = 0;

}

ActionLayer::~ActionLayer(){
  SAFE_DELETE(abilitiesMenu);
}
