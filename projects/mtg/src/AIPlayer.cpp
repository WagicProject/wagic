#include "../include/debug.h"
#include "../include/AIPlayer.h"
#include "../include/CardDescriptor.h"
#include "../include/DamageResolverLayer.h"
#include "../include/DamagerDamaged.h"
#include "../include/AIStats.h"

const char * const MTG_LAND_TEXTS[] = {"artifact","forest","island","mountain","swamp","plains","other lands"};

AIPlayer::AIPlayer(MTGPlayerCards * _deck, string file): Player(_deck, file){
  potentialMana = NEW ManaCost();
  nextCardToPlay = NULL;
  stats = NULL;
}

AIPlayer::~AIPlayer(){
  if (potentialMana) delete potentialMana;
  SAFE_DELETE(stats);
}
MTGCardInstance * AIPlayer::chooseCard(TargetChooser * tc, MTGCardInstance * source, int random){
  for (int i = 0; i < game->hand->nb_cards; i++){
    MTGCardInstance * card = game->hand->cards[i];
    if (!tc->alreadyHasTarget(card) && tc->canTarget(card)){
      return card;
    }
  }
  return NULL;
}

int AIPlayer::Act(float dt){
  GameObserver * gameObs = GameObserver::GetInstance();
  if (gameObs->currentPlayer == this){
    gameObs->userRequestNextGamePhase();
    return 1;
  }else{
    return 1;
  }
}


void AIPlayer::tapLandsForMana(ManaCost * potentialMana, ManaCost * cost){
#if defined (WIN32) || defined (LINUX)
  OutputDebugString("tapping land for mana\n");
#endif

  ManaCost * diff = potentialMana->Diff(cost);
  int currentCost = 0;
  GameObserver * gameObs = GameObserver::GetInstance();
  CardDescriptor cd;
  cd.setColor(MTG_COLOR_LAND);
  cd.tapped = -1;

  MTGCardInstance * card = NULL;
  while((card = cd.nextmatch(game->inPlay, card))){
#if defined (WIN32) || defined (LINUX)
    OutputDebugString("Found mana card\n");
#endif
    int doTap = 1;
    for (int i=MTG_NB_COLORS-1; i>= 0; i--){
      if (diff->getCost(i) &&  card->hasSubtype(MTG_LAND_TEXTS[i]) ){
#if defined (WIN32) || defined (LINUX)
	OutputDebugString("Not Gonna Tap\n");
#endif
	diff->remove(i,1);
	doTap = 0;
	break;
      }
    }
    if (doTap){
      gameObs->cardClick(card);
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("Tapped\n");
#endif
    }
  }

  delete(diff);


#if defined (WIN32) || defined (LINUX)
  OutputDebugString("ok land tapped");
#endif
}
//TODO a better function that does not take into account only basic lands
ManaCost * AIPlayer::getPotentialMana(){
  SAFE_DELETE(potentialMana);
  potentialMana = NEW ManaCost();
  CardDescriptor cd;
  cd.setColor(MTG_COLOR_LAND);
  cd.tapped = -1;
  MTGCardInstance * card = NULL;
  while((card = cd.nextmatch(game->inPlay, card))){

    if (card->hasSubtype("plains")){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("Found Potential plain\n");
#endif
      potentialMana->add(MTG_COLOR_WHITE,1);
    }else if(card->hasSubtype("swamp")){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("Found Potential swamp\n");
#endif
      potentialMana->add(MTG_COLOR_BLACK,1);
    }else if(card->hasSubtype("forest")){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("Found Potential forestn\n");
#endif
      potentialMana->add(MTG_COLOR_GREEN,1);
    }else if(card->hasSubtype("mountain")){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("Found Potential Mountain\n");
#endif
      potentialMana->add(MTG_COLOR_RED,1);
    }else if(card->hasSubtype("island")){
      potentialMana->add(MTG_COLOR_BLUE,1);
    }else{
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("WTF ????\n");
#endif
    }
  }
  return potentialMana;
}


//Default AI does not interrupt
int AIPlayer::checkInterrupt(){
  GameObserver * gameObs = GameObserver::GetInstance();
  if (gameObs->mLayers->stackLayer()->askIfWishesToInterrupt == this){
    gameObs->mLayers->stackLayer()->cancelInterruptOffer();
    return 1;
  }
  return 0;
}

int AIPlayer::effectBadOrGood(MTGCardInstance * card){
  int id = card->getMTGId();
  switch (id){
  default:
    break;
  }
  AbilityFactory * af = NEW AbilityFactory();
  int autoGuess = af->magicText(id,NULL,card);
  delete af;
  if (autoGuess) return autoGuess;
  return BAKA_EFFECT_DONTKNOW;
}

int AIPlayer::chooseTarget(TargetChooser * tc){
  Targetable * potentialTargets[50];
  int nbtargets = 0;
  GameObserver * gameObs = GameObserver::GetInstance();
  int checkOnly = 0;
  if (tc){
    checkOnly = 1;
  }else{
    tc = gameObs->getCurrentTargetChooser();
  }
  if (!tc) return 0;
  if (!(gameObs->currentlyActing() == this)) return 0;
  Player * target = this;
  int cardEffect = effectBadOrGood(tc->source);
  if (cardEffect != BAKA_EFFECT_GOOD){
    target = this->opponent();
  }


  if (!tc->alreadyHasTarget(target) &&  tc->canTarget(target) && nbtargets < 50){
    for (int i = 0; i < 3; i++){ //Increase probability to target a player when this is possible
      potentialTargets[nbtargets] = target;
      nbtargets++;
    }
    if (checkOnly) return 1;
  }
  MTGPlayerCards * playerZones = target->game;
  MTGGameZone * zones[] = {playerZones->hand,playerZones->library,playerZones->inPlay, playerZones->graveyard};
  for (int j = 0; j < 4; j++){
    MTGGameZone * zone = zones[j];
    for (int k=0; k< zone->nb_cards; k++){
      MTGCardInstance * card = zone->cards[k];
      if (!tc->alreadyHasTarget(card) && tc->canTarget(card)  && nbtargets < 50){
	if (checkOnly) return 1;
	int multiplier = 1;
	if (getStats() && getStats()->isInTop(card,10)){
	  multiplier++;
	  if (getStats()->isInTop(card,5)){
	    multiplier++;
	    if (getStats()->isInTop(card,3)){
	      multiplier++;
	    }
	  }
	}
	for (int l=0; l < multiplier; l++){
	  potentialTargets[nbtargets] = card;
	  nbtargets++;
	}
      }
    }
  }
  if (nbtargets){
    int i = rand() % nbtargets;
    int type = potentialTargets[i]->typeAsTarget();
    switch(type){
    case TARGET_CARD:
      {
	MTGCardInstance * card = ((MTGCardInstance *) potentialTargets[i]);
	gameObs->cardClick(card);
	return 1;
	break;
      }
    case TARGET_PLAYER:
      {
	Player * player = ((Player *) potentialTargets[i]);
	gameObs->cardClick(NULL, player);
	return 1;
	break;
      }
    }
  }
  //BIG PROBLEM
  gameObs->cancelCurrentAction();
  return 0;
}

int AIPlayer::getCreaturesInfo(Player * player, int neededInfo , int untapMode, int canAttack){
  int result = 0;
  CardDescriptor cd;
  cd.init();
  cd.setType("Creature");
  cd.tapped = untapMode;
  MTGCardInstance * card = NULL;
  while((card = cd.nextmatch(player->game->inPlay, card))){
    if (!canAttack || card->canAttack()){
      if (neededInfo == INFO_NBCREATURES){
	result++;
      }else{
	result+=card->power;
      }
    }
  }
  return result;
}



int AIPlayer::chooseAttackers(){
  //Attack with all creatures
  //How much damage can the other player do during his next Attack ?
  int opponentForce = getCreaturesInfo(opponent(),INFO_CREATURESPOWER);
  int opponentCreatures = getCreaturesInfo(opponent(), INFO_NBCREATURES);
  int myForce = getCreaturesInfo(this,INFO_CREATURESPOWER,-1,1);
  int myCreatures = getCreaturesInfo(this, INFO_NBCREATURES, -1,1);
  bool attack = (myCreatures > opponentCreatures || myForce > opponentForce || myForce > 2*opponent()->life);
  if (attack){
    CardDescriptor cd;
    cd.init();
    cd.setType("Creature");
    MTGCardInstance * card = NULL;
    while((card = cd.nextmatch(game->inPlay, card))){
      GameObserver::GetInstance()->cardClick(card);
    }
  }
  return 1;
}

int AIPlayer::chooseBlockers(){
  map<MTGCardInstance *, int> opponentsToughness;
  int opponentForce = getCreaturesInfo(opponent(),INFO_CREATURESPOWER);
  int opponentCreatures = getCreaturesInfo(opponent(), INFO_NBCREATURES, -1);
  int myForce = getCreaturesInfo(this,INFO_CREATURESPOWER);
  int myCreatures = getCreaturesInfo(this, INFO_NBCREATURES, -1);
  CardDescriptor cd;
  cd.init();
  cd.setType("Creature");
  cd.tapped = -1;
  MTGCardInstance * card = NULL;
  while((card = cd.nextmatch(game->inPlay, card))){
    GameObserver::GetInstance()->cardClick(card);
    int set = 0;
    while(!set){
      if (!card->defenser){
	set = 1;
      }else{
	MTGCardInstance * attacker = card->defenser;
	map<MTGCardInstance *,int>::iterator it = opponentsToughness.find(attacker);
	if ( it == opponentsToughness.end()){
	  opponentsToughness[attacker] = attacker->toughness;
	  it = opponentsToughness.find(attacker);
	}
	if (opponentsToughness[attacker] > 0 && getStats() && getStats()->isInTop(attacker,3,false)){
	  opponentsToughness[attacker]-= card->power;
	  set = 1;
	}else{
	  GameObserver::GetInstance()->cardClick(card);
	}
      }
    }
  }
  card = NULL;
  while((card = cd.nextmatch(game->inPlay, card))){
    if (card->defenser && opponentsToughness[card->defenser] > 0){
      while (card->defenser) GameObserver::GetInstance()->cardClick(card);
    }
  }
  card = NULL;
  while((card = cd.nextmatch(game->inPlay, card))){
    if(!card->defenser){
      GameObserver::GetInstance()->cardClick(card);
      int set = 0;
      while(!set){
	if (!card->defenser){
	  set = 1;
	}else{
	  MTGCardInstance * attacker = card->defenser;
	  if (opponentsToughness[attacker] <= 0 || (card->toughness <= card->defenser->power && opponentForce*2 <life)  || card->defenser->nbOpponents()>1){
	    GameObserver::GetInstance()->cardClick(card);
	  }else{
	    set = 1;
	  }
	}
      }
    }
  }
  return 1;
}

int AIPlayer::combatDamages(){
  int result = 0;
  GameObserver * gameObs = GameObserver::GetInstance();
  Player * currentPlayer = gameObs->currentPlayer;
  int currentGamePhase = gameObs->getCurrentGamePhase();
  if (currentGamePhase != MTG_PHASE_COMBATDAMAGE) return 0;
  DamageResolverLayer *  drl = gameObs->mLayers->combatLayer();
#if defined (WIN32) || defined (LINUX)
  OutputDebugString("AI Combat Phase START\n");
#endif
  if (drl->currentChoosingPlayer == this){
#if defined (WIN32) || defined (LINUX)
    OutputDebugString("This player chooses\n");
#endif
    for (int i = 0; i < drl->mCount; i++){
#if defined (WIN32) || defined (LINUX)
      OutputDebugString("AI Combat Phase\n");
#endif
      DamagerDamaged * current = (DamagerDamaged *) drl->mObjects[i];
      if (current->damageSelecter == this){
	result = 1;
	DamagerDamaged * canardEmissaire = NULL;
	for (int j = 0; j < drl->mCount; j++){
	  DamagerDamaged * opponent = (DamagerDamaged *) drl->mObjects[j];
	  if (drl->isOpponent(current, opponent)){
	    if (!canardEmissaire) canardEmissaire = opponent;
	    int over = opponent->hasLethalDamage();
	    while(!over){
	      if(!current->dealOneDamage(opponent)){
		over = 1;
	      }else{
		over =  opponent->hasLethalDamage();
	      }
#if defined (WIN32) || defined (LINUX)
	      char buf[4096];
	      sprintf(buf, "==========\n%s deals %i damages to %s\n=============\n", current->card->getName(), 1, opponent->card->getName());
	      OutputDebugString(buf);
#endif
	    }
	  }
	}
	if (canardEmissaire && !current->card->has(TRAMPLE)){
	  while(current->dealOneDamage(canardEmissaire)){
#if defined (WIN32) || defined (LINUX)
	    OutputDebugString("==========\nDealing damage to Canard Emissaire\n================\n");
#endif

	  }
	}
      }
    }
    if (result){
      drl->nextPlayer();
    }
  }
  return result;

}

AIStats * AIPlayer::getStats(){
  if (!stats){
    char statFile[512];
    sprintf(statFile, "Res/ai/baka/stats/%s.stats", opponent()->deckFile.c_str());
    stats = new AIStats(this, statFile);
  }
  return stats;
}

AIPlayer * AIPlayerFactory::createAIPlayer(MTGAllCards * collection, MTGPlayerCards * oponents_deck){
  int nbdecks = 0;
  int found = 1;
  while (found){
    found = 0;
    char buffer[512];
    sprintf(buffer, "Res/ai/baka/deck%i.txt",nbdecks+1);
    std::ifstream file(buffer);
    if(file){
      found = 1;
      file.close();
      nbdecks++;
    }
  }
  if (!nbdecks) return NULL;
  int deckid = 1 + rand() % (nbdecks);
  char deckFile[512];
  sprintf(deckFile, "Res/ai/baka/deck%i.txt",deckid);
  char deckFileSmall[512];
  sprintf(deckFileSmall, "ai_baka_deck%i",deckid);
#if defined (WIN32) || defined (LINUX)
  char debuf[4096];
  sprintf(debuf,"Deck File: %s", deckFile);
  OutputDebugString(debuf);
#endif
  int deck_cards_ids[100];
  int nb_elements = readfile_to_ints(deckFile, deck_cards_ids);
  MTGPlayerCards * deck = NEW MTGPlayerCards(collection,deck_cards_ids, nb_elements);
  AIPlayerBaka * baka = NEW AIPlayerBaka(deck,deckFileSmall);
  return baka;
}


MTGCardInstance * AIPlayerBaka::FindCardToPlay(ManaCost * potentialMana, const char * type){
  int maxCost = -1;
  MTGCardInstance * nextCardToPlay = NULL;
  MTGCardInstance * card = NULL;
  CardDescriptor cd;
  cd.init();
  cd.setType(type);
  card = NULL;
  while((card = cd.nextmatch(game->hand, card))){
    int currentCost = card->getManaCost()->getConvertedCost();
    if (currentCost > maxCost && potentialMana->canAfford(card->getManaCost())){
      TargetChooserFactory * tcf = NEW TargetChooserFactory();
      TargetChooser * tc = tcf->createTargetChooser(card);
      delete tcf;
      if (tc){
	      int hasTarget = (chooseTarget(tc));
	      delete tc;
	      if (!hasTarget)continue;
      }else{
        int shouldPlayPercentage = 10;
        int shouldPlay = effectBadOrGood(card);
        if (shouldPlay == BAKA_EFFECT_GOOD){
          shouldPlayPercentage = 90;
        }else if(BAKA_EFFECT_DONTKNOW == shouldPlay){
          shouldPlayPercentage = 70;
        }
        if (rand() % 100 > shouldPlayPercentage) continue;
      }
      nextCardToPlay = card;
      maxCost = currentCost;
    }
  }
  return nextCardToPlay;
}

AIPlayerBaka::AIPlayerBaka(MTGPlayerCards * _deck, char * file): AIPlayer(_deck,file){
  mAvatarTex = JRenderer::GetInstance()->LoadTexture("ai/baka/avatar.jpg", TEX_TYPE_USE_VRAM);
  if (mAvatarTex)
    mAvatar = NEW JQuad(mAvatarTex, 0, 0, 35, 50);
  initTimer();
}

void AIPlayerBaka::initTimer(){
  timer = 20;
}

int AIPlayerBaka::Act(float dt){
  GameObserver * gameObs = GameObserver::GetInstance();
  int currentGamePhase = gameObs->getCurrentGamePhase();

  if (currentGamePhase == MTG_PHASE_CLEANUP && currentGamePhase != oldGamePhase){
#if defined (WIN32) || defined (LINUX)
    OutputDebugString("updating stats\n");
#endif
    if (getStats()) getStats()->updateStats();
  }


  oldGamePhase = currentGamePhase;

  if (checkInterrupt()) return 0;

  timer--;
  if (timer>0){
    return 0;
  }
  initTimer();
#if defined (WIN32) || defined (LINUX)
  OutputDebugString("==========\nNew Act CALL\n================\n");
#endif




#if defined (WIN32) || defined (LINUX)
  OutputDebugString("==========\nCombat Damages ?\n================\n");
#endif
  if (combatDamages()) return 0;

#if defined (WIN32) || defined (LINUX)
  OutputDebugString("==========\nChoose Target ?\n================\n");
#endif
  if (chooseTarget()) return 0;


  Player * currentPlayer = gameObs->currentPlayer;




  CardDescriptor cd;


  if (currentPlayer == this){
    MTGCardInstance * card = NULL;
    switch(currentGamePhase){
    case MTG_PHASE_FIRSTMAIN:
    case MTG_PHASE_SECONDMAIN:
      if (canPutLandsIntoPlay){

	//Attempt to put land into play
	cd.init();
	cd.setColor(MTG_COLOR_LAND);
	card = cd.match(game->hand);
	if (card){
	  gameObs->cardClick(card);
	}
      }
      if(NULL == card){

	//Attempt to put creature into play
	if (manaPool->getConvertedCost()==0){

	  //No mana, try to get some
	  getPotentialMana();
#if defined (WIN32) || defined (LINUX)
	  char buffe[4096];

	  sprintf(buffe,"potentail mana %i\n",potentialMana->getConvertedCost() );
	  OutputDebugString(buffe);
#endif
	  if (potentialMana->getConvertedCost() > 0){


	    //look for the most expensive creature we can afford
	    nextCardToPlay = FindCardToPlay(potentialMana, "creature");
	    //Let's Try an enchantment maybe ?
	    if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(potentialMana, "enchantment");
	    if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(potentialMana, "artifact");
	    if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(potentialMana, "instant");
	    if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(potentialMana, "sorcery");
	    if (nextCardToPlay){
#if defined (WIN32) || defined (LINUX)
	      sprintf(buffe, "Putting Card Into Play: %s", nextCardToPlay->getName());
	      OutputDebugString(buffe);
#endif

	      tapLandsForMana(potentialMana,nextCardToPlay->getManaCost());
	    }
	  }
	  SAFE_DELETE(potentialMana);
	}else{
	  //We have mana, we can try to put the card into play
#if defined (WIN32) || defined (LINUX)
	  OutputDebugString("Mana paid, ready to put card into play\n");
#endif
	  if (nextCardToPlay){
	    gameObs->cardClick(nextCardToPlay);
	    nextCardToPlay = NULL;
	  }else{
	    //ERROR, WE PAID MANA WITHOUT ANY WILL TO PLAY
	  }
	}
      }
      if (NULL == card && NULL == nextCardToPlay){
#if defined (WIN32) || defined (LINUX)
	OutputDebugString("Switching to next phase\n");
#endif
	gameObs->userRequestNextGamePhase();
      }
      break;
    case MTG_PHASE_COMBATATTACKERS:
      chooseAttackers();
      gameObs->userRequestNextGamePhase();
      break;
    default:
      gameObs->userRequestNextGamePhase();
      break;
    }
  }else{
    switch(currentGamePhase){
    case MTG_PHASE_COMBATBLOCKERS:
      chooseBlockers();
      gameObs->userRequestNextGamePhase();
      break;
    default:
      break;
    }
    return 1;
  }
  return 1;
}
