#include "../include/debug.h"
#include "../include/GameObserver.h"

#include "../include/GameOptions.h"
#include "../include/ConstraintResolver.h"
#include "../include/CardGui.h"
#include "../include/Damage.h"
#include "../include/DamageResolverLayer.h"

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
  reaction = 0;
  gameOver = NULL;
  phaseRing = NEW PhaseRing(_players,_nb_players);
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
  currentPlayerId = (currentPlayerId+1)%nbPlayers;
  currentPlayer = players[currentPlayerId];
  currentActionPlayer = currentPlayer;

}
void GameObserver::nextGamePhase(){
  phaseRing->forward();
  Phase * cPhase = phaseRing->getCurrentPhase();
  currentGamePhase = cPhase->id;
  if (currentPlayer != cPhase->player) nextPlayer();

  //init begin of turn
  if (currentGamePhase == MTG_PHASE_BEFORE_BEGIN){
    cleanupPhase();
    currentPlayer->canPutLandsIntoPlay = 1;
    mLayers->actionLayer()->Update(0);
    return nextGamePhase();
  }
  //manaBurn
  if (currentGamePhase == MTG_PHASE_UNTAP ||
      currentGamePhase == MTG_PHASE_FIRSTMAIN ||
      currentGamePhase == MTG_PHASE_COMBATBEGIN ||
      currentGamePhase == MTG_PHASE_SECONDMAIN ||
      currentGamePhase == MTG_PHASE_ENDOFTURN
      ){
    currentPlayer->manaBurn();
  }

  //After End of turn
  if (currentGamePhase == MTG_PHASE_AFTER_EOT){
    //Auto Hand cleaning, in case the player didn't do it himself
    while(currentPlayer->game->hand->nb_cards > 7){
      currentPlayer->game->putInGraveyard(currentPlayer->game->hand->cards[0]);
    }
    mLayers->stackLayer()->garbageCollect(); //clean stack history for this turn;
    mLayers->actionLayer()->Update(0);
    return nextGamePhase();
  }

  //Phase Specific actions
  switch(currentGamePhase){
  case MTG_PHASE_UNTAP:
    untapPhase();
    break;
  case MTG_PHASE_DRAW:
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
  if (mLayers->combatLayer()->remainingDamageSteps) return;
  //TODO CHECK POSSIBILITY
  if (opponent()->isAI() || GameOptions::GetInstance()->values[OPTIONS_INTERRUPTATENDOFPHASE_OFFSET+currentGamePhase]){
    mLayers->stackLayer()->AddNextGamePhase();
  }else{
    nextGamePhase();
  }
}



void GameObserver::startGame(int shuffle, int draw){
  int i;
  for (i=0; i<nbPlayers; i++){
    players[i]->game->initGame(shuffle, draw);
  }
  phaseRing->goToPhase(MTG_PHASE_FIRSTMAIN, players[0]);
  currentGamePhase = MTG_PHASE_FIRSTMAIN;
}

void GameObserver::addObserver(MTGAbility * observer){
  mLayers->actionLayer()->Add(observer);
}


void GameObserver::removeObserver(ActionElement * observer){
  if (observer){
    observer->destroy();
  }else{
    //TODO log error
  }
  mLayers->actionLayer()->Remove(observer);
}

GameObserver::~GameObserver(){
  LOG("==Destroying GameObserver==");
  SAFE_DELETE(targetChooser);
  SAFE_DELETE(mLayers);
  SAFE_DELETE(phaseRing);
  LOG("==GameObserver Destroyed==");

}

void GameObserver::Update(float dt){
  Player * player =  currentPlayer;
  if (currentGamePhase == MTG_PHASE_COMBATBLOCKERS){
    player = opponent();
  }else	if (currentGamePhase == MTG_PHASE_COMBATDAMAGE){
    DamageResolverLayer *  drl = mLayers->combatLayer();
    if (drl->currentChoosingPlayer && drl->mCount) player = drl->currentChoosingPlayer;
  }
  currentActionPlayer = player;
  if (isInterrupting) player = isInterrupting;
  mLayers->Update(dt,player);
  stateEffects();
  oldGamePhase = currentGamePhase;

}

//applies damage to creatures after updates
//Players life test
void GameObserver::stateEffects(){
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

}


void GameObserver::nextStep(){

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
  //if (id>= -6 && id <= -3){
  if (id== -5 || id == -3){ //TODO libraries ???
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
  LOG("==GameObserver::cardClick");
  if (card) 	{LOG(card->getName())};
  Player * clickedPlayer = NULL;
  if (!card) clickedPlayer = ((Player *)object);
  if (targetChooser != NULL){
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

  if (card){
    reaction = mLayers->actionLayer()->isReactingToClick(card);
    if (reaction == -1) mLayers->actionLayer()->reactToClick(card);
  }else{
    reaction = mLayers->actionLayer()->isReactingToTargetClick(object);
    if (reaction == -1) mLayers->actionLayer()->reactToTargetClick(object);
  }

  if (reaction != -1){
    if (!card) return;
    if (currentlyActing()->game->hand->hasCard(card)){
      //Current player's hand
      if (canPutInPlay(card)){
	putInPlay(card);
	if (card->hasType("land")){
	  currentPlayer->canPutLandsIntoPlay--;
	}
      }else if (currentPlayer->game->hand->hasCard(card)){ 		//Current player's hand
	if (currentGamePhase == MTG_PHASE_CLEANUP && currentPlayer->game->hand->nb_cards > 7){
	  currentPlayer->game->putInGraveyard(card);
	}
      }
    }else if (reaction){
      if (reaction == 1){
	mLayers->actionLayer()->reactToClick(card);
      }else{
	mLayers->actionLayer()->setMenuObject(object);
      }
    }else if (card->isTapped() && card->controller() == currentPlayer){
      int a = ConstraintResolver::untap(this, card);
    }
  }


}


TargetChooser * GameObserver::getCurrentTargetChooser(){
  TargetChooser * _tc = mLayers->actionLayer()->getCurrentTargetChooser();
  if (_tc) return _tc;
  return targetChooser;
}

//Check if it is possible to put a card into play
//TODO : improve according to spells in game...
int GameObserver::canPutInPlay(MTGCardInstance *  card){
  Player * player = currentlyActing();
  LOG("CANPUTINPLAY- check if card belongs to current player\n");
  if (!player->game->hand->hasCard(card)) return 0;
  LOG("CANPUTINPLAY- check if card is land or can be played\n");
  if (card->hasType("land")){
    LOG("CANPUTINPLAY- card is land - check if can be played\n");
    if (player == currentPlayer && currentPlayer->canPutLandsIntoPlay && (currentGamePhase == MTG_PHASE_FIRSTMAIN || currentGamePhase == MTG_PHASE_SECONDMAIN)){
      LOG("CANPUTINPLAY- Land, ok\n");
      return 1;
    }
  }else if ((card->hasType("instant")) || card->has(FLASH) || (player == currentPlayer && (currentGamePhase == MTG_PHASE_FIRSTMAIN || currentGamePhase == MTG_PHASE_SECONDMAIN))){
    LOG("CANPUTINPLAY- correct time to play\n");
    if (checkManaCost(card)){
      LOG("CANPUTINPLAY- ManaCost ok\n");
      if (targetListIsSet(card)){
#ifdef LOG
	LOG("CANPUTINPLAY- Targets chosen -> OK\n");
#endif
	return 1;
      }else{
#ifdef LOG
	LOG("CANPUTINPLAY- Targets not chosen yet\n");
#endif
	return 0;
      }
    }
  }
  return 0;
}


void GameObserver::putInPlay(MTGCardInstance *  card){
  Player * player = currentlyActing();
  ManaCost * previousManaPool = NEW ManaCost(player->getManaPool());
  player->getManaPool()->pay(card->getManaCost());
  ManaCost * spellCost = previousManaPool->Diff(player->getManaPool());
  delete previousManaPool;
  if (card->hasType("land")){
    Spell * spell = NEW Spell(card);
    player->game->putInZone(card,  player->game->hand, player->game->stack);
    spell->resolve();
    delete spellCost;
    delete spell;
  }else{
    if (targetChooser){
      mLayers->stackLayer()->addSpell(card,targetChooser->targets,targetChooser->cursor, spellCost);
      delete targetChooser;
      targetChooser = NULL;
    }else{
      mLayers->stackLayer()->addSpell(card,NULL,0, spellCost);
    }
    player->game->putInZone(card,  player->game->hand, player->game->stack);

  }


}

/* Returns true if the card is in one of the player's play zone */
int GameObserver::isInPlay(MTGCardInstance * card){
  for (int i = 0; i < 2; i++){
    if (players[i]->game->isInPlay(card)) return 1;
  }
  return 0;
}

void GameObserver::draw(){
  //TODO checks to allow multiple draw, or no draw, etc...
  currentPlayer->game->drawFromLibrary();
}

void GameObserver::cleanupPhase(){
  currentPlayer->cleanupPhase();
  opponent()->cleanupPhase();
}

void GameObserver::untapPhase(){
  currentPlayer->inPlay()->untapAll();
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
