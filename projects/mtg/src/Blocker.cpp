#include "../include/config.h"
#include "../include/Blocker.h"

Blocker::Blocker(int id, MTGCardInstance * card):MTGAbility(id, card){
  init ( NEW ManaCost());
}

Blocker::Blocker(int id, MTGCardInstance * card, ManaCost * _cost):MTGAbility(id, card){
  init(_cost);
}
Blocker::Blocker(int id, MTGCardInstance * card, MTGCardInstance *_target):MTGAbility(id, card,_target){
  init ( NEW ManaCost());
}
Blocker::Blocker(int id, MTGCardInstance * card, MTGCardInstance *_target, ManaCost * _cost):MTGAbility(id, card,_target){
  init(_cost);
}

Blocker::~Blocker(){
  SAFE_DELETE(manaCost);
}

void Blocker::init(ManaCost * _cost){
  currentPhase = -1;
  manaCost = _cost;
}


//Default behaviour for blockers : they block the card they're attached to
void Blocker::Update(float dt){
  game = GameObserver::GetInstance();
  int newPhase = game->getCurrentGamePhase();
  if (newPhase != currentPhase){
    MTGCardInstance * _target;
    if (target){
      _target = (MTGCardInstance *) target;
    }else{
      _target = source;
    }
    _target->getBlockers()->Add(this);
#if defined (WIN32) || defined (LINUX)
    char buf[4096];
    sprintf(buf, "Adding Blocker to %s \n", _target->model->getName());
    OutputDebugString(buf);
#endif
  }
  currentPhase = newPhase;
}

int Blocker::destroy(){
  MTGCardInstance * _target;
  if (target){
    _target = (MTGCardInstance *) target;
  }else{
    _target = source;
  }
  _target->getBlockers()->Remove(this);
  return 1;
}

Blockers::Blockers(){
  init();
}



int Blockers::init(){
  cursor = -1;

  for (int i=0; i< MAX_BLOCKERS ; i++){
    blockers[i] = 0;
  }

  return 1;
}

int Blockers::Add (Blocker * ability){
  game = GameObserver::GetInstance();
  int index = game->mLayers->actionLayer()->getIndexOf(ability);
  blockers[index] = 1;
  return index;
}
int Blockers::Remove (Blocker * ability){
  game = GameObserver::GetInstance();
  int index = game->mLayers->actionLayer()->getIndexOf(ability);
  blockers[index] = 0;
  return index;
}

int Blockers::rewind(){
  cursor = -1;
  return 1;
}

Blocker * Blockers::next(){
  cursor++;
  game = GameObserver::GetInstance();
  while (blockers[cursor] == 0){
    cursor ++;
    if (cursor == MAX_BLOCKERS){
      cursor = -1;
      return NULL;
    }
  }
  return (Blocker *) (game->mLayers->actionLayer()->getByIndex(cursor));
}



int Blockers::isEmpty(){
  for (int i=0; i< MAX_BLOCKERS ; i++){
    if (blockers[i])
      return 0;
  }
  return 1;
}

Blockers::~Blockers(){

}
