#include "../include/config.h"
#include "../include/GameObserver.h"

#include "../include/GameOptions.h"
#include "../include/ConstraintResolver.h"
#include "../include/CardGui.h"
#include "../include/Damage.h"
#include "../include/DamageResolverLayer.h"
#include "../include/ExtraCost.h"

#include <JRenderer.h>

GameObserver * GameObserver::mInstance = NULL;


GameObserver* GameObserver::GetInstance()
{

  return mInstance;
}

void GameObserver::EndInstance()
{

  SAFE_DELETE(mInstance);
}

void GameObserver::Init(Player * _players[], int _nbplayers){
  mInstance = NEW GameObserver(_players, _nbplayers);
  mInstance->mLayers = NEW DuelLayers();
  mInstance->mLayers->init();
}


GameObserver::GameObserver(Player * _players[], int _nb_players){
  int i;

  for (i =0; i < _nb_players;i ++){
    players[i] = _players[i];
  }
  currentPlayer = players[0];
  currentActionPlayer = currentPlayer;
  isInterrupting = NULL;
  currentPlayerId = 0;
  nbPlayers = _nb_players;
  currentRound  = 1;
  currentGamePhase = -1;
  targetChooser = NULL;
  cardWaitingForTargets = NULL;
  waitForExtraPayment = NULL;
  reaction = 0;
  gameOver = NULL;
  phaseRing = NEW PhaseRing(_players,_nb_players);
  replacementEffects = NEW ReplacementEffects();
  blockersSorted = false;
  blockersAssigned = 0;
}

void GameObserver::setGamePhaseManager(MTGGamePhase * _phases){
  gamePhaseManager = _phases;
}

int GameObserver::getCurrentGamePhase(){
  return currentGamePhase;
}


Player * GameObserver::opponent(){
  int index = (currentPlayerId+1)%nbPlayers;
  return players[index];
}

int GameObserver::enteringPhase(int phase){
  //TODO
  return 0;
}

void GameObserver::nextPlayer(){
  turn++;
  currentPlayerId = (currentPlayerId+1)%nbPlayers;
  currentPlayer = players[currentPlayerId];
  currentActionPlayer = currentPlayer;
  blockersSorted = false;
  blockersAssigned = 0;

}
void GameObserver::nextGamePhase(){
  Phase * cPhaseOld = phaseRing->getCurrentPhase();
  if (!blockersSorted && cPhaseOld->id == Constants::MTG_PHASE_COMBATBLOCKERS){
    blockersAssigned = 1;
    if (!mLayers->combatLayer()->autoOrderBlockers()){
      OutputDebugString("Player has To choose ordering!");
      return;
    }
  }

  phaseRing->forward();
  Phase * cPhase = phaseRing->getCurrentPhase();
  currentGamePhase = cPhase->id;
  if (currentPlayer != cPhase->player) nextPlayer();


  //init begin of turn
  if (currentGamePhase == Constants::MTG_PHASE_BEFORE_BEGIN){
    cleanupPhase();
    currentPlayer->canPutLandsIntoPlay = 1;
    mLayers->actionLayer()->Update(0);
    return nextGamePhase();
  }

  for (int i=0; i < 2; i++){
    players[i]->getManaPool()->init();
  }

  //After End of turn
  if (currentGamePhase == Constants::MTG_PHASE_AFTER_EOT){
    //Auto Hand cleaning, in case the player didn't do it himself
    while(currentPlayer->game->hand->nb_cards > 7){
      currentPlayer->game->putInGraveyard(currentPlayer->game->hand->cards[0]);
    }
    mLayers->stackLayer()->garbageCollect(); //clean stack history for this turn;
    mLayers->actionLayer()->Update(0);
    for (int i=0; i < 2; i++){
      delete (players[i]->game->garbage);
      players[i]->game->garbage = NEW MTGGameZone();
    }
    return nextGamePhase();
  }

  //Phase Specific actions
  switch(currentGamePhase){
  case Constants::MTG_PHASE_UNTAP:
    untapPhase();
    break;
  case Constants::MTG_PHASE_DRAW:
    mLayers->stackLayer()->addDraw(currentPlayer,1);
    break;
  default:
    break;
  }
}

int GameObserver::cancelCurrentAction(){
  SAFE_DELETE(targetChooser);
  return 1;
}

void GameObserver::userRequestNextGamePhase(){
  if (mLayers->stackLayer()->getNext(NULL,0,NOT_RESOLVED)) return;
  if (getCurrentTargetChooser()) return;
  if (mLayers->combatLayer()->isDisplayed()) return;
  Phase * cPhaseOld = phaseRing->getCurrentPhase();
  if (!blockersSorted && cPhaseOld->id == Constants::MTG_PHASE_COMBATBLOCKERS){
    blockersAssigned = 1;
    if (!mLayers->combatLayer()->autoOrderBlockers()){
      OutputDebugString("Player has To choose ordering!");
      return;
    }
  }
  if (cPhaseOld->id == Constants::MTG_PHASE_COMBATBLOCKERS || 
     opponent()->isAI() || 
     GameOptions::GetInstance()->values[GameOptions::phaseInterrupts[currentGamePhase]].getIntValue()){
    mLayers->stackLayer()->AddNextGamePhase();
  }else{
    nextGamePhase();
  }
}

int GameObserver::forceShuffleLibraries(){
  OutputDebugString("FORCING\n");
  int result = 0;
  for (int i=0; i<2; i++){
    if (forceShuffleLibrary[i]) {
      forceShuffleLibrary[i] = 0;
      players[i]->game->library->shuffle();
      result++;
      OutputDebugString("YAY\n");
    }
  }
  if (result) mLayers->playLayer()->forceUpdateCards();
  return result;
}

void GameObserver::startGame(int shuffle, int draw){
  int i;
  for (i=0; i<nbPlayers; i++){
    players[i]->game->initGame(shuffle, draw);
    forceShuffleLibrary[i] = 0;
  }

  //Preload images from hand
  if (!players[0]->isAI()){
    for (i=0; i< players[0]->game->hand->nb_cards; i++){
      players[0]->game->hand->cards[i]->getThumb();
      players[0]->game->hand->cards[i]->getQuad();
    }
  }
  turn = 0;
  phaseRing->goToPhase(Constants::MTG_PHASE_FIRSTMAIN, players[0]);
  currentGamePhase = Constants::MTG_PHASE_FIRSTMAIN;

  //Difficult mode special stuff
  if (!players[0]->isAI() && players[1]->isAI()){
    GameOptions * go = GameOptions::GetInstance(); 
    int difficulty = go->values[OPTIONS_DIFFICULTY].getIntValue();
    if (go->values[OPTIONS_DIFFICULTY_MODE_UNLOCKED].getIntValue() && difficulty) {
      Player * p = players[1];
      for (int level=0; level < difficulty; level ++){
        MTGCardInstance * card = NULL;
        MTGGameZone * z = p->game->library;
        for (int j = 0; j<z->nb_cards; j++){
          MTGCardInstance * _card = z->cards[j];
          if (_card->hasType("land")){
            card = _card;
            j = z->nb_cards;
          }
        }
        if (card){
          MTGCardInstance * copy = p->game->putInZone(card,  p->game->library, p->game->stack);
          Spell * spell = NEW Spell(copy);          
          spell->resolve();
          delete spell;
        }
      }
    }
  }
}

void GameObserver::addObserver(MTGAbility * observer){
  mLayers->actionLayer()->Add(observer);
}


void GameObserver::removeObserver(ActionElement * observer){
  if (observer){
    if (mLayers->actionLayer()->getIndexOf(observer) != -1){
      observer->destroy();
      mLayers->actionLayer()->Remove(observer);
    }
  }else{
    //TODO log error
  }
  
}

GameObserver::~GameObserver(){
  LOG("==Destroying GameObserver==");
  SAFE_DELETE(targetChooser);
  SAFE_DELETE(mLayers);
  SAFE_DELETE(phaseRing);
  SAFE_DELETE(replacementEffects);
  LOG("==GameObserver Destroyed==");

}

void GameObserver::Update(float dt){
  Player * player =  currentPlayer;
  if (currentGamePhase == Constants::MTG_PHASE_COMBATBLOCKERS && !blockersSorted){
    player = opponent();
  }else	if (currentGamePhase == Constants::MTG_PHASE_COMBATDAMAGE){
    DamageResolverLayer *  drl = mLayers->combatLayer();
    if (drl->currentChoosingPlayer && drl->mCount) player = drl->currentChoosingPlayer;
  }
  currentActionPlayer = player;
  if (isInterrupting) player = isInterrupting;
  mLayers->Update(dt,player);
  while (mLayers->actionLayer()->stuffHappened){
    mLayers->actionLayer()->Update(0);
  }
  stateEffects();
  oldGamePhase = currentGamePhase;

}

//applies damage to creatures after updates
//Players life test
void GameObserver::stateEffects(){

  if (mLayers->stackLayer()->count(0,NOT_RESOLVED) != 0) return;
  if (mLayers->actionLayer()->menuObject) return;
  if (targetChooser || mLayers->actionLayer()->isWaitingForAnswer()) return;
  for (int i =0; i < 2; i++){
    MTGGameZone * zone = players[i]->game->inPlay;
    for (int j = zone->nb_cards-1 ; j>=0; j--){
      MTGCardInstance * card = zone->cards[j];
      card->afterDamage();
    }
  }

  for (int i =0; i < 2; i++){
    if (players[i]->life <= 0) gameOver = players[i];
  }

}


void GameObserver::Render(){
  mLayers->Render();
  if (targetChooser || mLayers->actionLayer()->isWaitingForAnswer()){
    JRenderer::GetInstance()->DrawRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(255,255,0,0));
  }
  if (waitForExtraPayment){
    waitForExtraPayment->Render();
  }

}




void GameObserver::ButtonPressed (int controllerId, PlayGuiObject * _object){
#if defined (WIN32) || defined (LINUX)
  OutputDebugString("Click\n");
#endif
  int id = _object->GetId();
  if (id >=0){
    MTGCardInstance * card = ((CardGui *)_object)->card;
    cardClick(card, card);
  }
  if (id== -6 || id == -4){ //libraries
    GuiGameZone * zone = (GuiGameZone *)_object;
    if (zone->showCards){
      zone->toggleDisplay();
      forceShuffleLibraries();
    } else {
      int pId = (-id - 4)/2;
      TargetChooser * _tc = this->getCurrentTargetChooser();
      if (_tc && _tc->targetsZone(players[pId]->game->library)){
        zone->toggleDisplay();
        forceShuffleLibrary[pId] = 1;
      }
    }
    
  }
  if (id== -5 || id == -3){ 
    GuiGameZone * zone = (GuiGameZone *)_object;
    zone->toggleDisplay();
  }
  if (id == -1 || id == -2){
#if defined (WIN32) || defined (LINUX)
    OutputDebugString("Click Player !\n");
#endif
    cardClick(NULL, ((GuiAvatar *)_object)->player);
  }
}

void GameObserver::stackObjectClicked(Interruptible * action){
  if (targetChooser != NULL){
#if defined (WIN32) || defined (LINUX)
    OutputDebugString("target chooser ok \n");
#endif
    int result = targetChooser->toggleTarget(action);
    if (result == TARGET_OK_FULL){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("target chooser Full \n");
#endif
      cardClick(cardWaitingForTargets);
    }else{
      return;
    }
  }else{
    reaction = mLayers->actionLayer()->isReactingToTargetClick(action);
    if (reaction == -1) mLayers->actionLayer()->reactToTargetClick(action);
  }
}

void GameObserver::cardClick (MTGCardInstance * card, Targetable * object){
  Player * clickedPlayer = NULL;
  if (!card) clickedPlayer = ((Player *)object);
  if (targetChooser){
    int result;
    if (card) {
      if (card == cardWaitingForTargets){
	      LOG("attempt to close targetting");
	      int _result = targetChooser->ForceTargetListReady();
	      if (_result){
	        result = TARGET_OK_FULL;
	      }else{

	        LOG("...but we cant!\n");
	        result = targetChooser->targetsReadyCheck();
	      }
      }else{
	      result = targetChooser->toggleTarget(card);
      }
    }else{
      result = targetChooser->toggleTarget(clickedPlayer);
    }
    if (result == TARGET_OK_FULL){
      card = cardWaitingForTargets;
    }else{
      return;
    }
  }

  if (waitForExtraPayment){
    if (card){
      waitForExtraPayment->tryToSetPayment(card);
    }
    if (waitForExtraPayment->isPaymentSet()){
      mLayers->actionLayer()->reactToClick(waitForExtraPayment->action, waitForExtraPayment->source);
      waitForExtraPayment = NULL;
    }
    return;
  }

  if (card){
    reaction = mLayers->actionLayer()->isReactingToClick(card);
    if (reaction == -1) mLayers->actionLayer()->reactToClick(card);
  }else{
    reaction = mLayers->actionLayer()->isReactingToTargetClick(object);
    if (reaction == -1) mLayers->actionLayer()->reactToTargetClick(object);
  }

  if (reaction != -1){
    if (!card) return;
		//Current player's hand
	  if (currentPlayer->game->hand->hasCard(card) && currentGamePhase == Constants::MTG_PHASE_CLEANUP && currentPlayer->game->hand->nb_cards > 7){
	    currentPlayer->game->putInGraveyard(card);
	  }else if (reaction){
      if (reaction == 1){
	      mLayers->actionLayer()->reactToClick(card);
      }else{
	      mLayers->actionLayer()->setMenuObject(object);
      }
    }else if (card->isTapped() && card->controller() == currentPlayer){
      ConstraintResolver::untap(this, card);
    }
  }


}


TargetChooser * GameObserver::getCurrentTargetChooser(){
  TargetChooser * _tc = mLayers->actionLayer()->getCurrentTargetChooser();
  if (_tc) return _tc;
  return targetChooser;
}



/* Returns true if the card is in one of the player's play zone */
int GameObserver::isInPlay(MTGCardInstance * card){
  for (int i = 0; i < 2; i++){
    if (players[i]->game->isInPlay(card)) return 1;
  }
  return 0;
}

void GameObserver::draw(){
  currentPlayer->game->drawFromLibrary();
}

void GameObserver::cleanupPhase(){
  currentPlayer->cleanupPhase();
  opponent()->cleanupPhase();
}

void GameObserver::untapPhase(){
  currentPlayer->inPlay()->untapAll();
}

int GameObserver::receiveEvent(WEvent * e){
  if (!e) return 0;
  return mLayers->receiveEvent(e);
}


int GameObserver::isACreature(MTGCardInstance * card){
  return card->isACreature();
}


Player * GameObserver::currentlyActing(){
  if (isInterrupting) return isInterrupting;
  return currentActionPlayer;
}

int GameObserver::tryToTapOrUntap(MTGCardInstance * card){

  int reaction = mLayers->actionLayer()->isReactingToClick(card);
  if (reaction){
    if (reaction == 1){
      mLayers->actionLayer()->reactToClick(card);
    }else{
      //TODO, what happens when several abilities react to the click ?
    }
    return reaction;
  }else{
    if (card->isTapped() && card->controller() == currentPlayer){
      int a = ConstraintResolver::untap(this, card);
      return a;
    }else{
      //TODO Check Spells
      //card->tap();
      return 0;
    }
    return 0;
  }
}

//TODO CORRECT THIS MESS
int GameObserver::targetListIsSet(MTGCardInstance * card){
  if (targetChooser == NULL){
    TargetChooserFactory tcf;
    targetChooser = tcf.createTargetChooser(card);
    cardWaitingForTargets = card;
    if (targetChooser == NULL){
      return 1;
    }
  }
  return (targetChooser->targetListSet());
}


int GameObserver::checkManaCost(MTGCardInstance * card){
  ManaCost * playerMana = currentlyActing()->getManaPool();
  ManaCost * cost = card->getManaCost();
  if (playerMana->canAfford(cost)){
    return 1;
  }
  return 0;
}
