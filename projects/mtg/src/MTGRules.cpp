#include "../include/config.h"
#include "../include/MTGRules.h"

MTGPutInPlayRule::MTGPutInPlayRule(int _id):MTGAbility(_id, NULL){

}

int MTGPutInPlayRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana){
  Player * player = game->currentlyActing();
  Player * currentPlayer = game->currentPlayer;
  LOG("CANPUTINPLAY- check if card belongs to current player\n");
  if (!player->game->hand->hasCard(card)) return 0;
  LOG("CANPUTINPLAY- check if card is land or can be played\n");
  if (card->hasType("land")){
    LOG("CANPUTINPLAY- card is land - check if can be played\n");
    if (player == currentPlayer && currentPlayer->canPutLandsIntoPlay && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN)){
      LOG("CANPUTINPLAY- Land, ok\n");
      return 1;
    }
  }else if ((card->hasType("instant")) || card->has(Constants::FLASH) || (player == currentPlayer && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN))){
    LOG("CANPUTINPLAY- correct time to play\n");
    ManaCost * playerMana = player->getManaPool();
    ManaCost * cost = card->getManaCost();
#ifdef WIN32
  cost->Dump();
#endif
    if (playerMana->canAfford(cost)){
      LOG("CANPUTINPLAY- ManaCost ok\n");
      return 1;
    }
  }
  return 0;
}

int MTGPutInPlayRule::reactToClick(MTGCardInstance * card){
  if (!isReactingToClick(card)) return 0;
  Player * player = game->currentlyActing();
  ManaCost * cost = card->getManaCost();
  if (cost->isExtraPaymentSet()){
    if (!game->targetListIsSet(card)){
      LOG("CANPUTINPLAY- Targets not chosen yet\n");
      return 0;
    }
  }else{
    cost->setExtraCostsAction(this, card);
    game->waitForExtraPayment = cost->extraCosts;
    return 0;
  }
  ManaCost * previousManaPool = NEW ManaCost(player->getManaPool());
  player->getManaPool()->pay(card->getManaCost());
  card->getManaCost()->doPayExtra();
  ManaCost * spellCost = previousManaPool->Diff(player->getManaPool());
  delete previousManaPool;
  if (card->hasType("land")){   
    MTGCardInstance * copy = player->game->putInZone(card,  player->game->hand, player->game->stack);
    Spell * spell = NEW Spell(copy);
    spell->resolve();
    delete spellCost;
    delete spell;
    player->canPutLandsIntoPlay--;
  }else{
    MTGCardInstance * copy = player->game->putInZone(card,  player->game->hand, player->game->stack);
    if (game->targetChooser){
      game->mLayers->stackLayer()->addSpell(copy,game->targetChooser->targets,game->targetChooser->cursor, spellCost);
      SAFE_DELETE(game->targetChooser);
    }else{
      game->mLayers->stackLayer()->addSpell(copy,NULL,0, spellCost);
    }
    

  }
  return 1;
}

//The Put into play rule is never destroyed
int MTGPutInPlayRule::testDestroy(){
  return 0;
}

MTGAttackRule::MTGAttackRule(int _id):MTGAbility(_id,NULL){
}

int MTGAttackRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana){
  if (currentPhase == Constants::MTG_PHASE_COMBATATTACKERS && card->controller() == game->currentPlayer && !card->isAttacker()){
    if (card->canAttack()) return 1;
  }
  return 0;
}

int MTGAttackRule::reactToClick(MTGCardInstance * card){
  if (!isReactingToClick(card)) return 0;
  card->attacker = 1;
  if (!card->basicAbilities[Constants::VIGILANCE]) card->tapped = 1;
  return 1;
}

//The Attack rule is never destroyed
int MTGAttackRule::testDestroy(){
  return 0;
}



MTGBlockRule::MTGBlockRule(int _id):MTGAbility(_id,NULL){
}

int MTGBlockRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana){
  if (currentPhase == Constants::MTG_PHASE_COMBATBLOCKERS && !game->isInterrupting && card->controller() == game->opponent()){
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
