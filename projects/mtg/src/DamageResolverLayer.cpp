#include "../include/config.h"
#include "../include/DamageResolverLayer.h"
#include "../include/GameObserver.h"
#include "../include/MTGCardInstance.h"
#include "../include/DamagerDamaged.h"
#include "../include/Damage.h"
#include "../include/Translate.h"

DamageResolverLayer::DamageResolverLayer(GameObserver * game) : game(game){
  currentPhase = -1;
  remainingDamageSteps = 0;
  damageStack = NULL;
  currentSource = NULL;
  buttonOk = 0;
  currentChoosingPlayer = NULL;
  orderingIsNeeded = 0;
}
void DamageResolverLayer::Update(float dt){
  int newPhase = game->getCurrentGamePhase();
  if (newPhase == Constants::MTG_PHASE_UNTAP){
    orderingIsNeeded = 0;
    game->blockersSorted = 0;
  }
  if (newPhase == Constants::MTG_PHASE_COMBATDAMAGE){
    if (!game->mLayers->stackLayer()->getNext(NULL,0,NOT_RESOLVED)){
      if (newPhase != currentPhase){
	      init();
      }
      if (remainingDamageSteps && empty()){
        OutputDebugString("Combat Damage STEP\n");
	      initResolve();
      }
    }
  }else{
    remainingDamageSteps = 0;
  }
  currentPhase = newPhase;
  PlayGuiObjectController::Update(dt);
}


int DamageResolverLayer::autoOrderBlockers(){
  resetObjects();
  MTGInPlay * attackers = game->currentPlayer->game->inPlay;
 MTGCardInstance * attacker = attackers->getNextAttacker(NULL);
  while (attacker != NULL){
    if (attacker->blockers.size() > 1){
      orderingIsNeeded = 1;
      Player * p = attacker->controller();
      addIfNotExists(attacker, p);
      list<MTGCardInstance *>::iterator it;
      for (it= attacker->blockers.begin(); it != attacker->blockers.end(); ++it){
        addIfNotExists(*it, p);
      }
    }
    attacker = attackers->getNextAttacker(attacker);
  }
  game->blockersSorted = 1;
  return 1 - orderingIsNeeded;
};

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


DamagerDamaged * DamageResolverLayer::addIfNotExists(MTGCardInstance * card, Player * selecter){
  for (int i = 0; i < mCount; i++){
    DamagerDamaged * item = (DamagerDamaged *)mObjects[i];
    if (item->card == card) return item;
  }
  //  CardGui * cardg = game->mLayers->playLayer()->getByCard(card);
  DamagerDamaged * item = NEW DamagerDamaged(card, selecter, mCount == 0);
  //  Add(NEW TransientCardView(card->gui));
  mCurr = 0;
  return item;
}

void DamageResolverLayer::updateAllCoordinates(){
  for (int i = 0; i < mCount; i++){
    DamagerDamaged * item = (DamagerDamaged *)mObjects[i];
    /*
    CardGui * cardg = game->mLayers->playLayer()->getByCard(item->card);
    item->x = cardg->x;
    item->y = cardg->y;
    */
  }
}

int DamageResolverLayer::updateCoordinates(MTGCardInstance * card){
  DamagerDamaged * item = NULL;
  for (int i = 0; i < mCount; i++){
    item = (DamagerDamaged *)mObjects[i];
    if (item->card != card) item = NULL ;
  }
  if (!item) return 0;
  /*
  CardGui * cardg = game->mLayers->playLayer()->getByCard(card);
  item->x = cardg->x;
  item->y = cardg->y;
  */
  return 1;
}


//Adds a card and all its opponents to the Damagers' list
int DamageResolverLayer::addDamager(MTGCardInstance * card, Player * selecter){
   DamagerDamaged * me = addIfNotExists(card, selecter);
  if (card->controller() == game->currentPlayer){ //Attacker
    MTGInPlay * defensers = game->opponent()->game->inPlay;
    MTGCardInstance * defenser = defensers->getNextDefenser(NULL, card);
    while (defenser != NULL){
      DamagerDamaged * item = addIfNotExists(defenser, whoSelectsDamagesDealtBy(defenser));
      while (!item->hasLethalDamage() && me->dealOneDamage(item)){} //Add default damage to the card...
      defenser = defensers->getNextDefenser(defenser, card);
    }
  }else{ //Defenser
    MTGInPlay * attackers = game->currentPlayer->game->inPlay;
    MTGCardInstance * attacker = card->isDefenser();
    DamagerDamaged * item = addIfNotExists(attacker,whoSelectsDamagesDealtBy(attacker));
    while (!item->hasLethalDamage() && me->dealOneDamage(item)){} //Add default damage to the card...
    MTGCardInstance * banding = attacker->banding;
    if (banding){
      attacker = attackers->getNextAttacker(NULL);
      while (attacker != NULL){
	    if (attacker->banding == banding){
	      item = addIfNotExists(attacker,whoSelectsDamagesDealtBy(attacker));
        while (!item->hasLethalDamage() && me->dealOneDamage(item)){} //Add default damage to the card...
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
  damageStack = NEW DamageStack(game);
  int strike = 0;
  if (remainingDamageSteps == 2) strike = 1;

  MTGInPlay * attackers = game->currentPlayer->game->inPlay;
  MTGInPlay * defensers = game->opponent()->game->inPlay;

  MTGCardInstance * attacker = attackers->getNextAttacker(NULL);
  while (attacker != NULL){
    if ((!strike && !attacker->has(Constants::FIRSTSTRIKE)) || (strike && attacker->has(Constants::FIRSTSTRIKE)) || attacker->has(Constants::DOUBLESTRIKE)){  
      OutputDebugString("Attacker Damaging!\n"); 
      if (Player * selecter = whoSelectsDamagesDealtBy(attacker)){
	      addDamager(attacker, selecter);
      }else{
        addAutoDamageToOpponents(attacker);
      }
    }
    MTGCardInstance * defenser = defensers->getNextDefenser(NULL, attacker);
    while (defenser != NULL){
      if ((!strike && !defenser->has(Constants::FIRSTSTRIKE)) || (strike && defenser->has(Constants::FIRSTSTRIKE)) || defenser->has(Constants::DOUBLESTRIKE)){       
        OutputDebugString("Blocker Damaging!\n");  
        if (Player * selecterb = whoSelectsDamagesDealtBy(defenser)){
	        addDamager(defenser, selecterb);
        }else{
          addAutoDamageToOpponents(defenser);
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
    //nextPlayer();
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
    MTGCardInstance * card = current->card;
    if (current->damageSelecter==currentChoosingPlayer){
      if (current->damageToDeal > 0){
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
      if (card->controller() == game->currentPlayer){ //Attacker
        //check that blockers have lethal damage
        list<MTGCardInstance *>::iterator it;
        int found_non_lethal = 0;
        for (it= card->blockers.begin(); it != card->blockers.end(); ++it){
          MTGCardInstance * c = *it;
          DamagerDamaged * defenser = findByCard(c);
          if (found_non_lethal && defenser->sumDamages()) return 0;
          if (!defenser->hasLethalDamage()) found_non_lethal = 1;
        }
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

int DamageResolverLayer::nextPlayer(){
  if (!canStopDealDamages()) return 0;
  if (currentChoosingPlayer == game->currentPlayer){
    currentChoosingPlayer = game->opponent();
  }

  resolveDamages();
  return 1;

}

bool DamageResolverLayer::blockersOrderingDone(){
    orderingIsNeeded = 0;
    game->blockersSorted = 1;
    resetObjects();
    return true;
}

bool DamageResolverLayer::clickReorderBlocker(MTGCardInstance * blocker){
  if (!blocker->defenser) return false;
  MTGCardInstance * attacker = blocker->defenser;
  attacker->moveBlockerInRow(blocker);
  list<MTGCardInstance *>::iterator it;
  return true;
}

bool DamageResolverLayer::checkUserInputOrderBlockers(u32 key){
  if (PSP_CTRL_CIRCLE == key) {
    if (mObjects[mCurr] && mObjects[mCurr]->ButtonPressed()){
      DamagerDamaged * current = (DamagerDamaged *) mObjects[mCurr];
      MTGCardInstance * blocker = current->card;
      return clickReorderBlocker(blocker);
    } 
    return false;
  }else if (PSP_CTRL_SQUARE == key){
    return blockersOrderingDone();
  }else{
    return PlayGuiObjectController::CheckUserInput(key);
  }
}

bool DamageResolverLayer::clickDamage(MTGCardInstance *c){
  DamagerDamaged * current = findByCard(c);
  return clickDamage(current);
}

bool DamageResolverLayer::clickDamage(DamagerDamaged * current){
  if (!current) return false;
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
        MTGCardInstance * card = currentSource->card;
        list<MTGCardInstance *>::iterator it = card->blockers.begin();
        while (it!= card->blockers.end() && *it!=current->card){
          DamagerDamaged * item = findByCard(*it);
          while (!item->hasLethalDamage() && currentSource->dealOneDamage(item)){} //Add default damage to the card...
          it++;
        }
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
  return true;
}

bool DamageResolverLayer::CheckUserInput(u32 key){
  if (!mCount) return false;
  if (orderingIsNeeded) return checkUserInputOrderBlockers(key);
  if (PSP_CTRL_CIRCLE == key){
    if (mObjects[mCurr] && mObjects[mCurr]->ButtonPressed()){
      DamagerDamaged * current = (DamagerDamaged *) mObjects[mCurr];
      return clickDamage(current);
      //buttonOk = 0;
      //if (canStopDealDamages()) buttonOk = 1;
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
    return nextPlayer();
  }else{
    if (!mCount)
      return false;
    if (game != NULL){
      if (mActionButton == key){
	if (mObjects[mCurr] != NULL && mObjects[mCurr]->ButtonPressed()){
	  game->ButtonPressed((PlayGuiObject *)mObjects[mCurr]);
	  return true;
	}
      }
      if (PSP_CTRL_CROSS == key){
	game->cancelCurrentAction();
	return true;
      }
    }

    last_user_move = 0;
    switch (key)
      {
      case PSP_CTRL_LEFT:
	{
	  int n = getClosestItem(DIR_LEFT);
	  if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_LEFT))
	    {
	      mCurr = n;
	      mObjects[mCurr]->Entering();
	    }
	  return true;
	}
      case PSP_CTRL_RIGHT:
	{
	  int n = getClosestItem(DIR_RIGHT);
	  if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_RIGHT))
	    {
	      mCurr = n;
	      mObjects[mCurr]->Entering();
	    }
	  return true;
	}
      case PSP_CTRL_UP:
	{
	  int n = getClosestItem(DIR_UP);
	  if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_UP))
	    {
	      mCurr = n;
	      mObjects[mCurr]->Entering();
	    }
	  return true;
	}
      case PSP_CTRL_DOWN:
	{
	  int n = getClosestItem(DIR_DOWN);
	  if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_DOWN))
	    {
	      mCurr = n;
	      mObjects[mCurr]->Entering();
	    }
	  return true;
	}
      case PSP_CTRL_TRIANGLE:
	showBigCards = (showBigCards + 1) % 3;
	return true;
      }
    return false;
  }
  return false;
}

void DamageResolverLayer::Render(){
  if (!mCount) return;
  updateAllCoordinates(); //this is dirty :(
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);

  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRect(0 ,0 , SCREEN_WIDTH , SCREEN_HEIGHT , ARGB(200,0,0,0));
  if (currentChoosingPlayer == game->currentPlayer){
    mFont->DrawString(_("Attacking Player").c_str(), 0,0);
  }else{
    mFont->DrawString(_("Blocking Player").c_str(), 0,0);
  }
  if (currentSource){
    //    currentSource->RenderBig(10, 20);
    mFont->DrawString(_("Current Damager:").c_str(), 10, 5);
  }
  for (int i = 0; i < mCount; i++){
    ((DamagerDamaged *)mObjects[i])->Render(currentChoosingPlayer);
  }
  if (mObjects[mCurr]){
    ((DamagerDamaged *)mObjects[mCurr])->Render(currentChoosingPlayer);
  }


  if (currentPhase == Constants::MTG_PHASE_COMBATDAMAGE && canStopDealDamages()){
    mFont->DrawString(_("Damages Assigned, Click Square to Continue").c_str(), 250, 5);
  }
  if (orderingIsNeeded) mFont->DrawString(_("Order blockers, then Click Square to Continue").c_str(), 200, 5);
}
