#include "../include/config.h"
#include "../include/MTGGamePhase.h"


MTGGamePhase::MTGGamePhase(int id):ActionElement(id){
  animation = 0;
  currentState = -1;
  mFont= resources.GetJLBFont("simon");
  mFont->SetBase(0);	// using 2nd font
}


void MTGGamePhase::Update(float dt){

  int newState = GameObserver::GetInstance()->getCurrentGamePhase();
  if (newState != currentState){
    activeState = ACTIVE;
    animation = 4;
    currentState = newState;

    switch (currentState){

    default: break;
    }

  }


  if (animation > 0){
    //    fprintf(stderr, "animation = %f", animation);
    animation -- ;
  }else{
    activeState = INACTIVE;
    animation = 0;

  }

}

bool MTGGamePhase::CheckUserInput(u32 key){
  GameObserver * game = GameObserver::GetInstance();
  if (activeState == INACTIVE){
    if ((PSP_CTRL_RTRIGGER == key) && game->currentActionPlayer == game->currentlyActing())
      {
	activeState = ACTIVE;
	game->userRequestNextGamePhase();
	return true;
      }
  }
  return false;
}

  MTGGamePhase * MTGGamePhase::clone() const{
    MTGGamePhase * a =  NEW MTGGamePhase(*this);
    a->isClone = 1;
    return a;
  }

ostream& MTGGamePhase::toString(ostream& out) const
{
  return out << "MTGGamePhase ::: animation " << animation << " ; currentState : " << currentState;
}
