#include "PrecompiledHeader.h"

#include "ActionLayer.h"
#include "GameObserver.h"
#include "Targetable.h"
#include "WEvent.h"

MTGAbility* ActionLayer::getAbility(int type){
  for (int i = 1; i < mCount; i++){
     MTGAbility * a = ((MTGAbility *)mObjects[i]);
     if (a->aType == type){
       return a;
     }
   }
  return NULL;
}

int ActionLayer::moveToGarbage(ActionElement * e){
  int i = getIndexOf(e);
  if (i != -1){
    if (isWaitingForAnswer() == e) setCurrentWaitingAction(NULL);
    e->destroy();
    mObjects.erase(mObjects.begin()+i);
    mCount--;
    garbage.push_back(e);
    return 1;
  }
  return 0;
        
}

int ActionLayer::cleanGarbage(){
  for (size_t i = 0; i < garbage.size(); ++i){
    delete(garbage[i]);
  }
  garbage.clear();
  return 1;
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

bool ActionLayer::CheckUserInput(JButton key){
  GameObserver * g = GameObserver::GetInstance();
  if (g->waitForExtraPayment && key == JGE_BTN_SEC){
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
  GameObserver* game = GameObserver::GetInstance();
  for (int i=mCount -1 ;i>=0;i--){
    if (mObjects[i]!= NULL){
      ActionElement * currentAction = (ActionElement *)mObjects[i];
      if (currentAction->testDestroy())
	      game->removeObserver(currentAction);
    }
  }
  int newPhase = game->getCurrentGamePhase();
  for (int i=0;i<mCount;i++){
    if (mObjects[i]!=NULL){
      ActionElement * currentAction = (ActionElement *)mObjects[i];
      currentAction->newPhase = newPhase;
      currentAction->Update(dt);
      currentAction->currentPhase = newPhase;
    }
  }

  if (cantCancel){
    ActionElement * ae = isWaitingForAnswer();
    if (ae && !ae->tc->validTargetsExist()) {
      cantCancel = 0;
      cancelCurrentAction();
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
      currentAction->Render();
    }
  }
}


void ActionLayer::setCurrentWaitingAction(ActionElement * ae){
  assert(!ae || !currentWaitingAction);//this assert causes crashes when may abilities overlap each other on ai. this conidiation is preexsiting.
  currentWaitingAction = ae;
  if (!ae) cantCancel = 0;
}

TargetChooser * ActionLayer::getCurrentTargetChooser(){
  if (currentWaitingAction && currentWaitingAction->waitingForAnswer)
     return currentWaitingAction->tc;
  return NULL;
}

int ActionLayer::cancelCurrentAction(){
  ActionElement * ae = isWaitingForAnswer();
  if (!ae) return 0;
  if (cantCancel && ae->tc->validTargetsExist()) return 0;
  ae->waitingForAnswer = 0; //TODO MOVE THIS IN ActionElement
  setCurrentWaitingAction(NULL);
  return 1;
}

ActionElement * ActionLayer::isWaitingForAnswer(){
  if (currentWaitingAction && currentWaitingAction->waitingForAnswer)
     return currentWaitingAction;
  return NULL;
}

int ActionLayer::stillInUse(MTGCardInstance * card){
  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    if(currentAction->stillInUse(card)) return 1;
  }
  return 0;
}

int ActionLayer::receiveEventPlus(WEvent * event){
  int result = 0;
  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    result += currentAction->receiveEvent(event);
  }
  return 0;
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

  ActionElement * ae = isWaitingForAnswer();
  if (ae) return reactToTargetClick(ae,card);

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

  ActionElement * ae = isWaitingForAnswer();
  if (ae) return reactToClick(ae,card);

  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    result += reactToClick(currentAction,card);
    if (result) return result;
  }
  return result;
}


void ActionLayer::setMenuObject(Targetable * object, bool must){
  if (!object){
    DebugTrace("FATAL: ActionLayer::setMenuObject");
    return;
  }
  menuObject = object;

  SAFE_DELETE(abilitiesMenu);

  abilitiesMenu = NEW SimpleMenu(10, this, Fonts::MAIN_FONT, 100, 100,object->getDisplayName().c_str());

  for (int i=0;i<mCount;i++){
    ActionElement * currentAction = (ActionElement *)mObjects[i];
    if (currentAction->isReactingToTargetClick(object)){
      abilitiesMenu->Add(i,currentAction->getMenuText());
    }
  }
  if (!must) abilitiesMenu->Add(kCancelMenuID, "Cancel");
  else cantCancel = 1;
  modal = 1;
}

void ActionLayer::doReactTo(int menuIndex){

  if (menuObject){
    int controlid = abilitiesMenu->mObjects[menuIndex]->GetId();
    DebugTrace("ActionLayer::doReactTo " << controlid);
    ButtonPressed(0,controlid);
  }
}

void ActionLayer::ButtonPressed(int controllerid, int controlid){
  if (controlid != -1){
    ActionElement * currentAction = (ActionElement *)mObjects[controlid];
    currentAction->reactToTargetClick(menuObject);
  }else{
    GameObserver::GetInstance()->mLayers->stackLayer()->endOfInterruption();
  }
  menuObject = 0;

}

ActionLayer::ActionLayer(){
  menuObject = NULL; 
  abilitiesMenu = NULL; 
  stuffHappened = 0;
  currentWaitingAction = NULL;
  cantCancel = 0;
}

ActionLayer::~ActionLayer(){
  for (int i=mCount-1;i>=0;i--){
    moveToGarbage((ActionElement *)mObjects[i]);
  }
  SAFE_DELETE(abilitiesMenu);
  cleanGarbage();
}
