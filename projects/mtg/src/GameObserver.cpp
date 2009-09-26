#include "../include/config.h"
#include "../include/GameObserver.h"
#include "../include/GameOptions.h"
#include "../include/ConstraintResolver.h"
#include "../include/CardGui.h"
#include "../include/Damage.h"
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
  currentGamePhase = -1;
  targetChooser = NULL;
  cardWaitingForTargets = NULL;
  waitForExtraPayment = NULL;
  reaction = 0;
  gameOver = NULL;
  phaseRing = NEW PhaseRing(_players,_nb_players);
  replacementEffects = NEW ReplacementEffects();
  combatStep = BLOCKERS;
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


void GameObserver::nextPlayer(){
  turn++;
  currentPlayerId = (currentPlayerId+1)%nbPlayers;
  currentPlayer = players[currentPlayerId];
  currentActionPlayer = currentPlayer;
  combatStep = BLOCKERS;
}
void GameObserver::nextGamePhase(){
  Phase * cPhaseOld = phaseRing->getCurrentPhase();
  if (cPhaseOld->id == Constants::MTG_PHASE_COMBATDAMAGE)
    if (FIRST_STRIKE == combatStep || END_FIRST_STRIKE == combatStep || DAMAGE == combatStep) { 
      nextCombatStep(); return; 
    }
  if (cPhaseOld->id == Constants::MTG_PHASE_COMBATBLOCKERS)
    if (BLOCKERS == combatStep) { nextCombatStep(); return; }

  phaseRing->forward();

  //Go directly to end of combat if no attackers
  if (cPhaseOld->id == Constants::MTG_PHASE_COMBATATTACKERS && !(currentPlayer->game->inPlay->getNextAttacker(NULL))){
    phaseRing->forward();
    phaseRing->forward();
  }
  
  Phase * cPhase = phaseRing->getCurrentPhase();
  currentGamePhase = cPhase->id;

  if (Constants::MTG_PHASE_COMBATDAMAGE == currentGamePhase)
    nextCombatStep();

  if (currentPlayer != cPhase->player) nextPlayer();


  //init begin of turn
  if (currentGamePhase == Constants::MTG_PHASE_BEFORE_BEGIN){
    cleanupPhase();
    currentPlayer->canPutLandsIntoPlay = 1;
    mLayers->actionLayer()->Update(0);
    combatStep = BLOCKERS;
    return nextGamePhase();
  }

  for (int i = 0; i < 2; ++i)
    players[i]->getManaPool()->init();

  //After End of turn
  if (currentGamePhase == Constants::MTG_PHASE_AFTER_EOT){
    //Auto Hand cleaning, in case the player didn't do it himself
    while(currentPlayer->game->hand->nb_cards > 7)
      currentPlayer->game->putInGraveyard(currentPlayer->game->hand->cards[0]);
    mLayers->actionLayer()->cleanGarbage(); //clean abilities history for this turn;
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
  mLayers->actionLayer()->cancelCurrentAction();
  return 1;
}

void GameObserver::nextCombatStep()
{
  switch (combatStep)
    {
    case BLOCKERS         : receiveEvent(NEW WEventBlockersChosen());
                            receiveEvent(NEW WEventCombatStepChange(combatStep = ORDER)); return;
    case ORDER            : receiveEvent(NEW WEventCombatStepChange(combatStep = FIRST_STRIKE)); return;
    case FIRST_STRIKE     : receiveEvent(NEW WEventCombatStepChange(combatStep = END_FIRST_STRIKE)); return;
    case END_FIRST_STRIKE : receiveEvent(NEW WEventCombatStepChange(combatStep = DAMAGE)); return;
    case DAMAGE           : receiveEvent(NEW WEventCombatStepChange(combatStep = END_DAMAGE)); return;
    case END_DAMAGE       : ; // Nothing : go to next phase
    }
}

void GameObserver::userRequestNextGamePhase(){
  if (mLayers->stackLayer()->getNext(NULL,0,NOT_RESOLVED)) return;
  if (getCurrentTargetChooser()) return;
  Phase * cPhaseOld = phaseRing->getCurrentPhase();


  if ((cPhaseOld->id == Constants::MTG_PHASE_COMBATBLOCKERS && combatStep == ORDER) ||
    cPhaseOld->id == Constants::MTG_PHASE_COMBATDAMAGE ||
     opponent()->isAI() ||
     options[Options::optionInterrupt(currentGamePhase)].number)
    mLayers->stackLayer()->AddNextGamePhase();
  else
    nextGamePhase();

}

int GameObserver::forceShuffleLibraries(){
  int result = 0;
  if (players[0]->game->library->needShuffle)
    {
      players[0]->game->library->shuffle();
      players[0]->game->library->needShuffle = false;
      ++result;
    }
  if (players[1]->game->library->needShuffle)
    {
      players[1]->game->library->shuffle();
      players[1]->game->library->needShuffle = false;
      ++result;
    }
  return result;
}

void GameObserver::startGame(int shuffle, int draw){
  int i;
  for (i=0; i<nbPlayers; i++)
    players[i]->game->initGame(shuffle, draw);

  //Preload images from hand
  if (!players[0]->isAI()){
    for (i=0; i< players[0]->game->hand->nb_cards; i++){
      resources.RetrieveCard(players[0]->game->hand->cards[i],CACHE_THUMB);
      resources.RetrieveCard(players[0]->game->hand->cards[i]);
    }
  }
  turn = 0;
  phaseRing->goToPhase(Constants::MTG_PHASE_FIRSTMAIN, players[0]);
  currentGamePhase = Constants::MTG_PHASE_FIRSTMAIN;

  //Difficult mode special stuff
  if (!players[0]->isAI() && players[1]->isAI()){
    int difficulty = options[Options::DIFFICULTY].number;
    if (options[Options::DIFFICULTY_MODE_UNLOCKED].number && difficulty) {
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
  if (observer)mLayers->actionLayer()->moveToGarbage(observer);

  else
    {} //TODO log error
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
  Player * player = currentPlayer;
  if (Constants::MTG_PHASE_COMBATBLOCKERS == currentGamePhase && BLOCKERS == combatStep)
    player = player->opponent();
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
void GameObserver::stateEffects()
{
  if (mLayers->stackLayer()->count(0,NOT_RESOLVED) != 0) return;
  if (mLayers->actionLayer()->menuObject) return;
  if (targetChooser || mLayers->actionLayer()->isWaitingForAnswer()) return;
  for (int i =0; i < 2; i++){
    MTGGameZone * zone = players[i]->game->inPlay;
    for (int j = zone->nb_cards-1 ; j>=0; j--){
      MTGCardInstance * card = zone->cards[j];
      card->afterDamage();

      //Remove auras that don't have a valid target anymore
      if (card->target && !isInPlay(card->target)){
        players[i]->game->putInGraveyard(card);
      }
    }
  }
  for (int i =0; i < 2; i++)
    if (players[i]->life <= 0) gameOver = players[i];
}


void GameObserver::Render()
{
  mLayers->Render();
  if (targetChooser || mLayers->actionLayer()->isWaitingForAnswer())
    JRenderer::GetInstance()->DrawRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(255,255,0,0));
  if (waitForExtraPayment)
    waitForExtraPayment->Render();
}




void GameObserver::ButtonPressed(PlayGuiObject * target){
#if defined (WIN32) || defined (LINUX)
  OutputDebugString("GAMEOBSERVER Click\n");
#endif
  if (CardView* cardview = dynamic_cast<CardView*>(target)){
    MTGCardInstance * card = cardview->getCard();
    cardClick(card, card);
  }
  else if (GuiLibrary* library = dynamic_cast<GuiLibrary*>(target)){
    if (library->showCards){
      library->toggleDisplay();
      forceShuffleLibraries();
    } else {
      TargetChooser * _tc = this->getCurrentTargetChooser();
      if (_tc && _tc->targetsZone(library->zone)){
        library->toggleDisplay();
        library->zone->needShuffle = true;
      }
    }
  }
  else if (GuiGraveyard* graveyard = dynamic_cast<GuiGraveyard*>(target))
    graveyard->toggleDisplay();
  else if (GuiAvatar* avatar = dynamic_cast<GuiAvatar*>(target)){
    cardClick(NULL, avatar->player);
  }
}

void GameObserver::stackObjectClicked(Interruptible * action){
  if (targetChooser != NULL){
    int result = targetChooser->toggleTarget(action);
    if (result == TARGET_OK_FULL){
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
    if (result == TARGET_OK_FULL)
      card = cardWaitingForTargets;
    else
      return;
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

  if (ORDER == combatStep)
    {
      card->defenser->raiseBlockerRankOrder(card);
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
  eventsQueue.push(e);
  if (eventsQueue.size() > 1) return -1;
  int result = 0;
  while(eventsQueue.size()){
    WEvent * ev = eventsQueue.front();
    result +=  mLayers->receiveEvent(ev);
    SAFE_DELETE(ev);
    eventsQueue.pop();
  }
  return result;
}


bool GameObserver::isCreature(MTGCardInstance * card){
  return card->isCreature();
}


Player * GameObserver::currentlyActing(){
  if (isInterrupting) return isInterrupting;
  return currentActionPlayer;
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
