#include "../include/config.h"
#include "../include/MTGRules.h"

MTGAttackRule::MTGAttackRule(int _id):MTGAbility(_id,NULL){
}

int MTGAttackRule::isReactingToClick(MTGCardInstance * card){
  if (currentPhase == MTG_PHASE_COMBATATTACKERS && card->controller() == game->currentPlayer && !card->isAttacker()){
    if (card->canAttack()) return 1;
  }
  return 0;
}

int MTGAttackRule::reactToClick(MTGCardInstance * card){
  if (!isReactingToClick(card)) return 0;
  card->attacker = 1;
  if (!card->basicAbilities[VIGILANCE]) card->tapped = 1;
  return 1;
}

//The Attack rule is never destroyed
int MTGAttackRule::testDestroy(){
  return 0;
}



MTGBlockRule::MTGBlockRule(int _id):MTGAbility(_id,NULL){
}

int MTGBlockRule::isReactingToClick(MTGCardInstance * card){
  if (currentPhase == MTG_PHASE_COMBATBLOCKERS && !game->isInterrupting && card->controller() == game->opponent()){
    if (card->canBlock()) return 1;
  }
  return 0;
}

int MTGBlockRule::reactToClick(MTGCardInstance * card){
  if (!isReactingToClick(card)) return 0;
  MTGCardInstance * currentOpponent = card->isDefenser();

  bool result = false;
  int candefend = 0;
  while (!result){
    currentOpponent = game->currentPlayer->game->inPlay->getNextAttacker(currentOpponent);
#if defined (WIN32) || defined (LINUX)
    char buf[4096];
    sprintf(buf,"Defenser Toggle %s \n" ,card->model->getName());
    OutputDebugString(buf);
#endif
    candefend =  card->toggleDefenser(currentOpponent);
    result = (candefend || currentOpponent == NULL);
  }
  return 1;
}

//The Block rule is never destroyed
int MTGBlockRule::testDestroy(){
  return 0;
}
