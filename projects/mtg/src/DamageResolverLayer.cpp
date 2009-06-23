#include "../include/config.h"
#include "../include/DamageResolverLayer.h"
#include "../include/GameObserver.h"
#include "../include/MTGCardInstance.h"
#include "../include/DamagerDamaged.h"
#include "../include/Damage.h"

DamageResolverLayer::DamageResolverLayer(int id, GameObserver * _game):PlayGuiObjectController(id,  _game){
  currentPhase = -1;
  remainingDamageSteps = 0;
  damageStack = NULL;
  currentSource = NULL;
  buttonOk = 0;
  currentChoosingPlayer = NULL;
}
void DamageResolverLayer::Update(float dt){
  int newPhase = game->getCurrentGamePhase();
  if (newPhase == Constants::MTG_PHASE_COMBATDAMAGE){
    if (!game->mLayers->stackLayer()->getNext(NULL,0,NOT_RESOLVED)){

      if (newPhase != currentPhase){
	init();
      }
      if (remainingDamageSteps && empty()){
	initResolve();
      }
    }
  }else{
    remainingDamageSteps = 0;
  }
  currentPhase = newPhase;
  PlayGuiObjectController::Update(dt);
}


Player * DamageResolverLayer::whoSelectsDamagesDealtBy(MTGCardInstance * card){
  if (card->controller() == game->currentPlayer){ //Attacker
    MTGInPlay * defensers = game->opponent()->game->inPlay;
    int nbdefensers = defensers->nbDefensers(card);
    if (nbdefensers == 0) return NULL;
    if(nbdefensers == 1 && !card->has(Constants::TRAMPLE)) return NULL;
    MTGCardInstance * defenser = defensers->getNextDefenser(NULL, card);
    while (defenser != NULL){
      if (defenser->has(Constants::BANDING)) return game->opponent();
      defenser = defensers->getNextDefenser(defenser, card);
    }
    return game->currentPlayer;
  }else{ //Defenser
    MTGInPlay * attackers = game->currentPlayer->game->inPlay;
    int nbattackers = attackers->nbPartners(card->isDefenser());
    if(nbattackers <= 1) return NULL;
    if (card->isDefenser()->banding) return game->currentPlayer;
    return game->opponent();
  }
}

int DamageResolverLayer::addAutoDamageToOpponents(MTGCardInstance * card){
  if (card->controller() == game->currentPlayer){ //Attacker
    MTGInPlay * defensers = game->opponent()->game->inPlay;
    int nbdefensers = defensers->nbDefensers(card);
    if (nbdefensers == 0){
      Damage * damage = NEW Damage (0, card, game->opponent());
      damageStack->Add(damage);
    }else if (nbdefensers == 1){
      Damage * damage = NEW Damage (0, card, defensers->getNextDefenser(NULL, card));
      damageStack->Add(damage);
    }else{
      //SHOULDN'T HAPPEN !
    }
  }else{ //Defenser
    Damage * damage = NEW Damage (mCount,card, card->isDefenser());
    damageStack->Add(damage);
  }
  return 1;
}


int DamageResolverLayer::addIfNotExists(MTGCardInstance * card, Player * selecter){
  for (int i = 0; i < mCount; i++){
    DamagerDamaged * item = (DamagerDamaged *)mObjects[i];
    if (item->card == card) return 0;
  }
  CardGui * cardg = game->mLayers->playLayer()->getByCard(card);
  DamagerDamaged * item = NEW DamagerDamaged(cardg, selecter, mCount == 0);
  Add(item);
  mCurr = 0;
  return 1;
}


//Adds a card and all its opponents to the Damagers' list
int DamageResolverLayer::addDamager(MTGCardInstance * card, Player * selecter){
  addIfNotExists(card, selecter);
  if (card->controller() == game->currentPlayer){ //Attacker
    MTGInPlay * defensers = game->opponent()->game->inPlay;
    MTGCardInstance * defenser = defensers->getNextDefenser(NULL, card);
    while (defenser != NULL){
      addIfNotExists(defenser, whoSelectsDamagesDealtBy(defenser));
      defenser = defensers->getNextDefenser(defenser, card);
    }
  }else{ //Defenser
    MTGInPlay * attackers = game->currentPlayer->game->inPlay;
    MTGCardInstance * attacker = card->isDefenser();
    addIfNotExists(attacker,whoSelectsDamagesDealtBy(attacker));
    MTGCardInstance * banding = attacker->banding;
    if (banding){
      attacker = attackers->getNextAttacker(NULL);
      while (attacker != NULL){
	if (attacker->banding == banding){
	  addIfNotExists(attacker,whoSelectsDamagesDealtBy(attacker));
	}
	attacker = attackers->getNextAttacker(attacker);
      }
    }
  }
  return 1;
}

int DamageResolverLayer::initResolve(){
#if defined (WIN32) || defined (LINUX)
  char buf[4096];
  sprintf(buf, "starting resolve, remainingDamagesStep = %i\n",  remainingDamageSteps);
  OutputDebugString(buf);
#endif
  if (damageStack) return 0;

#if defined (WIN32) || defined (LINUX)
  sprintf(buf, "damageStack is NULL, we can resolve \n");
  OutputDebugString(buf);
#endif
  currentSource = NULL;
  currentChoosingPlayer = game->currentPlayer;
  damageStack = NEW DamageStack(mCount,game);
  int strike = 0;
  if (remainingDamageSteps == 2) strike = 1;

  MTGInPlay * attackers = game->currentPlayer->game->inPlay;
  MTGInPlay * defensers = game->opponent()->game->inPlay;

  MTGCardInstance * attacker = attackers->getNextAttacker(NULL);
  while (attacker != NULL){
#if defined (WIN32) || defined (LINUX)
    sprintf(buf, "attacker : %s \n", attacker->getName());
    OutputDebugString(buf);
#endif
    if ((!strike && !attacker->has(Constants::FIRSTSTRIKE)) || (strike && attacker->has(Constants::FIRSTSTRIKE)) || attacker->has(Constants::DOUBLESTRIKE)){
      Player * selecter = whoSelectsDamagesDealtBy(attacker);
      if (!selecter){
	addAutoDamageToOpponents(attacker);
      }else{
	addDamager(attacker, selecter);
      }
    }
    MTGCardInstance * defenser = defensers->getNextDefenser(NULL, attacker);
    while (defenser != NULL){
      if ((!strike && !defenser->has(Constants::FIRSTSTRIKE)) || (strike && defenser->has(Constants::FIRSTSTRIKE)) || defenser->has(Constants::DOUBLESTRIKE)){
	Player * selecterb = whoSelectsDamagesDealtBy(defenser);
	if (!selecterb){
	  addAutoDamageToOpponents(defenser);
	}else{
	  addDamager(defenser, selecterb);
	}
      }
      defenser = defensers->getNextDefenser(defenser, attacker);
    }
    attacker = attackers->getNextAttacker(attacker);
  }

  if (empty()){
    if (!damageStack->empty()){
      game->mLayers->stackLayer()->addAction(damageStack);
      game->mLayers->stackLayer()->resolve(); //Wagic 2010
    }else{
      SAFE_DELETE(damageStack);
    }
    remainingDamageSteps--;
    damageStack = NULL;
    modal = remainingDamageSteps;
  }else{
    if (canStopDealDamages()) currentChoosingPlayer = game->opponent();
  }
  return 1;
}
int DamageResolverLayer::init(){
  modal = 1;
  remainingDamageSteps = 2;
  return 1;
}

DamagerDamaged * DamageResolverLayer::findByCard(MTGCardInstance * card){
  for (int i =0; i < mCount; i++){
    DamagerDamaged * current = (DamagerDamaged *) mObjects[i];
    if (current->card == card) return current;
  }
  return NULL;
}

//Returns 1 if all "compulsory" damages have been assigned for current player
int DamageResolverLayer::canStopDealDamages(){
  for (int i = 0; i < mCount ; i ++){
    DamagerDamaged * current = (DamagerDamaged *) mObjects[i];
    if (current->damageSelecter==currentChoosingPlayer && current->damageToDeal > 0){
      MTGCardInstance * card = current->card;
      if (card->controller() == game->currentPlayer){ //Attacker
	if (card->has(Constants::TRAMPLE)){
	  MTGInPlay * defensers = game->opponent()->game->inPlay;
	  MTGCardInstance * defenser = defensers->getNextDefenser(NULL, card);
	  while (defenser != NULL){
	    DamagerDamaged * _defenser = findByCard(defenser);
	    if (!_defenser->hasLethalDamage()) return 0;
	    defenser = defensers->getNextDefenser(defenser, card);
	  }
	}else{
	  return 0;
	}
      }else{ //Defenser
	return 0;
      }
    }
  }
  return 1;
}

int DamageResolverLayer::trampleDamage(){
  for (int i = 0; i < mCount ; i ++){
    DamagerDamaged * current = (DamagerDamaged *) mObjects[i];
    if (current->damageToDeal > 0){
      MTGCardInstance * card = current->card;
      if (card->controller() == game->currentPlayer){ //Attacker
	if (card->has(Constants::TRAMPLE)){
	  Damage * damage = NEW Damage(0, card, game->opponent(), current->damageToDeal);
	  damageStack->Add(damage);
	}
      }
    }
  }
  return 1;
}

int DamageResolverLayer::resolveDamages(){
  trampleDamage();
  for (int i = 0; i < mCount ; i++){
    DamagerDamaged * current = (DamagerDamaged *) mObjects[i];
    for (int j =0; j < current->mCount ; j++){
      Damage * damage = NEW Damage(0, current->damages[j]->source, current->damages[j]->target, current->damages[j]->damage);
      damageStack->Add(damage);
    }
  }
  game->mLayers->stackLayer()->addAction(damageStack);
  game->mLayers->stackLayer()->resolve(); //Wagic 2010
  remainingDamageSteps--;
  resetObjects();
  damageStack = NULL;
  modal = remainingDamageSteps;
  return 1;
}

//a and b are opponents if b is blocking a band in which a belongs or blocking directly a
int DamageResolverLayer::isOpponent(DamagerDamaged * a, DamagerDamaged * b){
  MTGCardInstance * carda = a->card;
  MTGCardInstance * cardb = b->card;
  if (cardb->controller() == game->currentPlayer) {//if b is the attacker switch the cards
    carda = cardb;
    cardb = a->card;
  }
  if (cardb->controller() == game->currentPlayer || carda->controller() == game->opponent()) return 0; //Same team, idiot !

  if (!carda->banding){
    if (cardb->isDefenser() == carda) return 1;
    return 0;
  }

  if (cardb->isDefenser() && cardb->isDefenser()->banding == carda->banding) return 1;
  return 0;
}

void DamageResolverLayer::nextPlayer(){
  if (currentChoosingPlayer == game->currentPlayer){
    currentChoosingPlayer = game->opponent();
    if (canStopDealDamages()) resolveDamages();
  }else{
    resolveDamages();
  }

}
bool DamageResolverLayer::CheckUserInput(u32 key){
  if (!mCount) return false;
  if (PSP_CTRL_CIRCLE == key){
    if (mObjects[mCurr] && mObjects[mCurr]->ButtonPressed()){
      DamagerDamaged * current = (DamagerDamaged *) mObjects[mCurr];
      if (!currentSource || !isOpponent(current,currentSource)){
	for (int i = 0; i < mCount; i++){
	  DamagerDamaged * _current = (DamagerDamaged *) mObjects[i];
	  if (isOpponent(current,_current)){
	    currentSource = _current;
	    break;
	  }
	}
      }
      if (currentSource){
	if (currentSource->damageSelecter == currentChoosingPlayer){
	  if (isOpponent(current,currentSource)){
	    if (!currentSource->dealOneDamage(current)){
	      currentSource->removeDamagesTo(current);
	    }
	  }
	}
      }else{
	if (current->damageSelecter == currentChoosingPlayer){
	  currentSource = current;
	}
      }
      buttonOk = 0;
      if (canStopDealDamages()) buttonOk = 1;
    }
    return true;
  }else if (PSP_CTRL_CROSS == key){
    if (mObjects[mCurr] && mObjects[mCurr]->ButtonPressed()){
      DamagerDamaged * current = (DamagerDamaged *) mObjects[mCurr];
      if (current->damageSelecter == currentChoosingPlayer){
	currentSource = current;
      }
      return true;
    }
  }else if (PSP_CTRL_SQUARE == key){
    if (canStopDealDamages()){
      nextPlayer();
      //switch to next player or end of selection
    }
    return true;
  }else{
    return PlayGuiObjectController::CheckUserInput(key);
  }
  return false;
}

void DamageResolverLayer::Render(){
  if (!mCount) return;
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);

  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRect(0 ,0 , SCREEN_WIDTH , SCREEN_HEIGHT , ARGB(200,0,0,0));
  if (currentChoosingPlayer == game->currentPlayer){
    mFont->DrawString("Player 1", 0,0);
  }else{
    mFont->DrawString("Player 2", 0,0);
  }
  if (currentSource){
    currentSource->RenderBig(10, 20);
    mFont->DrawString("Current Damager:", 10, 5);
  }
  for (int i = 0; i < mCount; i++){
    ((DamagerDamaged *)mObjects[i])->Render(currentChoosingPlayer);
  }
  if (mObjects[mCurr]){
    ((DamagerDamaged *)mObjects[mCurr])->Render(currentChoosingPlayer);
  }


  if (buttonOk){
    mFont->DrawString("Damages Assigned, Click Square to Continue", 250, 5);
  }
}
