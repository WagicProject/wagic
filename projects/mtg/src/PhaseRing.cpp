#include "../include/PhaseRing.h"
#include "../include/MTGDefinitions.h"
#include "../include/Player.h"
#include "../include/config.h"
#include "../include/WEvent.h"


/* Creates a new phase ring with the default rules */
PhaseRing::PhaseRing(Player* players[], int nbPlayers){
  for (int i = 0; i < nbPlayers; i++){
    for (int j = 0; j < Constants::NB_MTG_PHASES; j++){
      Phase * phase = NEW Phase(j,players[i]);
      addPhase(phase);
    }
  }
  current = ring.begin();
}

PhaseRing::~PhaseRing(){
  list<Phase *>::iterator it;
  for (it = ring.begin(); it != ring.end(); it++){
    Phase * currentPhase = *it;
    delete(currentPhase);
  }
}

Phase * PhaseRing::getCurrentPhase(){
  if (current == ring.end()){
    current = ring.begin();
  }
  return *current;
}

Phase * PhaseRing::forward(){
  Phase * cPhaseOld = *current;
  if (current != ring.end()) current++;
  if (current == ring.end()) current = ring.begin();

  //Warn the layers about the phase Change
  WEvent * e = NEW WEventPhaseChange(cPhaseOld, *current);
  GameObserver::GetInstance()->receiveEvent(e);
  //delete e;

  return *current;
}

Phase * PhaseRing::goToPhase(int id, Player * player){
  Phase * currentPhase = *current;
  while(currentPhase->id !=id || currentPhase->player != player){ //Dangerous, risk for inifinte loop !
#ifdef WIN32
    OutputDebugString("goto");
#endif
    currentPhase = forward();
  }
  return currentPhase;
}

int PhaseRing::addPhase(Phase * phase){
  ring.push_back(phase);
  return 1;
}

int PhaseRing::addPhaseBefore(int id, Player* player,int after_id, Player * after_player, int allOccurences){
  int result = 0;
  list<Phase *>::iterator it;
  for (it = ring.begin(); it != ring.end(); it++){
    Phase * currentPhase = *it;
    if (currentPhase->id == after_id && currentPhase->player == after_player){
      result++;
      ring.insert(it,NEW Phase(id,player));
      if (!allOccurences) return 1;
    }
  }
  return result;
}
int PhaseRing::removePhase (int id, Player * player, int allOccurences){
  int result = 0;
  list<Phase *>::iterator it = ring.begin();
  while (it != ring.end()){
    Phase * currentPhase = *it;
    if (currentPhase->id == id && currentPhase->player == player){
      if (current == it) current++; //Avoid our cursor to get invalidated
      it = ring.erase(it);
      delete(currentPhase);
      result++;
      if (!allOccurences) return 1;
    }else{
      it++;
    }
  }
  return result;
}
