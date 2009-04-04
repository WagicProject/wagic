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

int ActionLayer::unstopableRenderInProgress(){

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
  if (menuObject){
    abilitiesMenu->Update(dt);
    return;
  }
  modal = 0;
  for (int i=mCount -1 ;i>=0;i--){
    if (mObjects[i]!= NULL){
      ActionElement * currentAction = (ActionElement *)mObjects[i];
      if (currentAction->testDestroy()){
	currentAction->destroy();
	Remove(currentAction);
      }
    }
  }
  int newPhase = GameObserver::GetInstance()->getCurrentGamePhase();
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

int ActionLayer::isWaitingForAnswer(){
  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    if(currentAction->waitingForAnswer) return 1;
  }
  return 0;
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
      return currentAction->reactToTargetClick(card);
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
      return currentAction->reactToClick(card);
    }
  }

  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    OutputDebugString(currentAction->getMenuText());
    result += currentAction->reactToClick(card);
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