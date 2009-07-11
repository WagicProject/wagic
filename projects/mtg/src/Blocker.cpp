#include "../include/config.h"
#include "../include/Blocker.h"

UntapBlocker::UntapBlocker(int id, MTGCardInstance * card):MTGAbility(id, card){
  init ( NEW ManaCost());
}

UntapBlocker::UntapBlocker(int id, MTGCardInstance * card, ManaCost * _cost):MTGAbility(id, card){
  init(_cost);
}
UntapBlocker::UntapBlocker(int id, MTGCardInstance * card, MTGCardInstance *_target):MTGAbility(id, card,_target){
  init ( NEW ManaCost());
}
UntapBlocker::UntapBlocker(int id, MTGCardInstance * card, MTGCardInstance *_target, ManaCost * _cost):MTGAbility(id, card,_target){
  init(_cost);
}

UntapBlocker::~UntapBlocker(){
  SAFE_DELETE(manaCost);
}

void UntapBlocker::init(ManaCost * _cost){
  currentPhase = -1;
  manaCost = _cost;
}

  UntapBlocker * UntapBlocker::clone() const{
    UntapBlocker * a =  NEW UntapBlocker(*this);
    a->isClone = 1;
    return a;
  }


//Default behaviour for blockers : they block the card they're attached to
void UntapBlocker::Update(float dt){
  game = GameObserver::GetInstance();
  int newPhase = game->getCurrentGamePhase();
  if (newPhase != currentPhase){
    MTGCardInstance * _target;
    if (target){
      _target = (MTGCardInstance *) target;
    }else{
      _target = source;
    }
    _target->getUntapBlockers()->Add(this);
#if defined (WIN32) || defined (LINUX)
    char buf[4096];
    sprintf(buf, "Adding Blocker to %s \n", _target->model->getName());
    OutputDebugString(buf);
#endif
  }
  currentPhase = newPhase;
}

int UntapBlocker::destroy(){
  MTGCardInstance * _target;
  if (target){
    _target = (MTGCardInstance *) target;
  }else{
    _target = source;
  }
  _target->getUntapBlockers()->Remove(this);
  return 1;
}

UntapBlockers::UntapBlockers(){
  init();
}



int UntapBlockers::init(){
  cursor = -1;

  for (int i=0; i< MAX_BLOCKERS ; i++){
    blockers[i] = 0;
  }

  return 1;
}

int UntapBlockers::Add (UntapBlocker * ability){
  game = GameObserver::GetInstance();
  int index = game->mLayers->actionLayer()->getIndexOf(ability);
  blockers[index] = 1;
  return index;
}
int UntapBlockers::Remove (UntapBlocker * ability){
  game = GameObserver::GetInstance();
  int index = game->mLayers->actionLayer()->getIndexOf(ability);
  blockers[index] = 0;
  return index;
}

int UntapBlockers::rewind(){
  cursor = -1;
  return 1;
}

UntapBlocker * UntapBlockers::next(){
  cursor++;
  game = GameObserver::GetInstance();
  while (blockers[cursor] == 0){
    cursor ++;
    if (cursor == MAX_BLOCKERS){
      cursor = -1;
      return NULL;
    }
  }
  return (UntapBlocker *) (game->mLayers->actionLayer()->getByIndex(cursor));
}



int UntapBlockers::isEmpty(){
  for (int i=0; i< MAX_BLOCKERS ; i++){
    if (blockers[i])
      return 0;
  }
  return 1;
}

UntapBlockers::~UntapBlockers(){

}
