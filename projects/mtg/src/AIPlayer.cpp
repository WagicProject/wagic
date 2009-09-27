#include "../include/config.h"
#include "../include/AIPlayer.h"
#include "../include/CardDescriptor.h"
#include "../include/AIStats.h"
#include "../include/AllAbilities.h"
#include "../include/ExtraCost.h"
#include "../include/GuiCombat.h"

const char * const MTG_LAND_TEXTS[] = {"artifact","forest","island","mountain","swamp","plains","other lands"};

int AIAction::Act(){
  GameObserver * g = GameObserver::GetInstance();
  if (player){
    g->cardClick(NULL, player);
    return 1;
  }
  if (ability){
    g->mLayers->actionLayer()->reactToClick(ability,click);
    if (target) g->cardClick(target);
    return 1;
  }else if (click){ //Shouldn't be used, really...
    g->cardClick(click);
    if (target) g->cardClick(target);
    return 1;
  }
  return 0;
}

AIPlayer::AIPlayer(MTGPlayerCards * deck, string file, string fileSmall) : Player(deck, file, fileSmall) {
  potentialMana = NEW ManaCost();
  nextCardToPlay = NULL;
  stats = NULL;
  agressivity = 50;
}

AIPlayer::~AIPlayer(){
  SAFE_DELETE(potentialMana);
  if (stats){
    stats->save();
    SAFE_DELETE(stats);
  }
  while(!clickstream.empty()){
    AIAction * action = clickstream.front();
    SAFE_DELETE(action);
    clickstream.pop();
  }
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
  if (gameObs->currentPlayer == this)
    gameObs->userRequestNextGamePhase();
  return 1;
}


void AIPlayer::tapLandsForMana(ManaCost * potentialMana, ManaCost * cost){
#if defined (WIN32) || defined (LINUX)
  OutputDebugString("tapping land for mana\n");
#endif
  if (!cost) return;
  ManaCost * diff = potentialMana->Diff(cost);
  GameObserver * g = GameObserver::GetInstance();

  map<MTGCardInstance *,bool>used;
  for (int i = 1; i < g->mLayers->actionLayer()->mCount; i++){ //0 is not a mtgability...hackish
    //Make sure we can use the ability
    MTGAbility * a = ((MTGAbility *)g->mLayers->actionLayer()->mObjects[i]);
    AManaProducer * amp = dynamic_cast<AManaProducer*>(a);
    if (amp && canHandleCost(amp)){
      MTGCardInstance * card = amp->source;
      if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost()==1){
        used[card] = true;
        int doTap = 1;
        for (int i=Constants::MTG_NB_COLORS-1; i>= 0; i--){
          if (diff->getCost(i) &&  amp->output->getCost(i) ){
	          diff->remove(i,1);
            doTap = 0;
            break;
          }
        }
        if (doTap){
          AIAction * action = NEW AIAction(amp,card);
          clickstream.push(action);
        }
      }
    }
  }
  delete(diff);

}

ManaCost * AIPlayer::getPotentialMana(){
  SAFE_DELETE(potentialMana);
  potentialMana = NEW ManaCost();
  GameObserver * g = GameObserver::GetInstance();
  map<MTGCardInstance *,bool>used;
  for (int i = 1; i < g->mLayers->actionLayer()->mCount; i++){ //0 is not a mtgability...hackish
    //Make sure we can use the ability
    MTGAbility * a = ((MTGAbility *)g->mLayers->actionLayer()->mObjects[i]);
    AManaProducer * amp = dynamic_cast<AManaProducer*>(a);
    if (amp && canHandleCost(amp)){
      MTGCardInstance * card = amp->source;
      if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost()==1){
        potentialMana->add(amp->output);
        used[card] = true;
      }
    }
  }

  return potentialMana;
}


int AIPlayer::getEfficiency(AIAction * action){
  return action->getEfficiency();
}

int AIPlayer::canHandleCost(MTGAbility * ability){
    //Can't handle sacrifice costs that require a target yet :(
  if (ability->cost){
    ExtraCosts * ec = ability->cost->extraCosts;
    if (ec){
      for (size_t i = 0; i < ec->costs.size(); i++){
        if (ec->costs[i]->tc) return 0;
      }
    }
  }
  return 1;
}

int AIAction::getEfficiency(){
  //TODO add multiplier according to what the player wants
  if (efficiency != -1) return efficiency;
  if (!ability) return 0;
  GameObserver * g = GameObserver::GetInstance();
  ActionStack * s = g->mLayers->stackLayer();
  Player * p = g->currentlyActing();
  if (s->has(ability)) return 0;

  MTGAbility * a = ability;
  GenericTargetAbility * gta = dynamic_cast<GenericTargetAbility*>(a);
  if (gta) a = gta->ability;

  GenericActivatedAbility * gaa = dynamic_cast<GenericActivatedAbility*>(a);
  if (gaa) a = gaa->ability;

  if (!a){
    OutputDebugString("FATAL: Ability is NULL in AIAction::getEfficiency()");
    return 0;
  }

  if (!((AIPlayer *)p)->canHandleCost(ability)) return 0;

  switch (a->aType){
    case MTGAbility::DAMAGER:
      {
        AADamager * aad = (AADamager *) a;
        if ( p == target->controller()){
          efficiency = 0;
        }else if (aad->damage >= target->toughness){
          efficiency = 100;
        }else if (target->toughness){
          efficiency = (50 * aad->damage) / target->toughness;
        }else{
          efficiency = 0;
        }
        break;
      }
    case MTGAbility::STANDARD_REGENERATE:
      {
        MTGCardInstance * _target = (MTGCardInstance *)(a->target);
        efficiency = 0;
        if (!_target->regenerateTokens && g->getCurrentGamePhase()< Constants::MTG_PHASE_COMBATDAMAGE && (_target->defenser || _target->blockers.size())){
          efficiency = 95;
        }
        //TODO If the card is the target of a damage spell
        break;
      }
    case MTGAbility::MANA_PRODUCER: //can't use mana producers right now :/
      efficiency = 0;
      break;
    default:
      if (target){
        AbilityFactory af;
        int suggestion = af.abilityEfficiency(a, p, MODE_ABILITY);
        if ((suggestion == BAKA_EFFECT_BAD && p==target->controller()) ||(suggestion == BAKA_EFFECT_GOOD && p!=target->controller())){
          efficiency =0;
        }else{
          efficiency = rand() % 5; //Small percentage of chance for unknown abilities
        }
      }else{
        efficiency = rand() % 10;
      }
      break;
  }
  return efficiency;
}




int AIPlayer::createAbilityTargets(MTGAbility * a, MTGCardInstance * c, map<AIAction *, int, CmpAbilities> * ranking){
  if (!a->tc){
    AIAction * as = NEW AIAction(a,c,NULL);
    (*ranking)[as] = 1;
    return 1;
  }
  GameObserver * g = GameObserver::GetInstance();
  for (int i = 0; i < 2; i++){
    Player * p = g->players[i];
    MTGGameZone * playerZones[] = {p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay};
    for (int j = 0; j < 4; j++){
      MTGGameZone * zone = playerZones[j];
      for (int k=0; k < zone->nb_cards; k++){
        MTGCardInstance * t = zone->cards[k];
        if (a->tc->canTarget(t)){

          AIAction * as = NEW AIAction(a,c,t);
          (*ranking)[as] = 1;
        }
      }
    }
  }
  return 1;
}

int AIPlayer::selectAbility(){
  map<AIAction *, int,CmpAbilities>ranking;
  list<int>::iterator it;
  ManaCost * pMana = getPotentialMana();
  GameObserver * g = GameObserver::GetInstance();
  for (int i = 1; i < g->mLayers->actionLayer()->mCount; i++){ //0 is not a mtgability...hackish
    //Make sure we can use the ability
    MTGAbility * a = ((MTGAbility *)g->mLayers->actionLayer()->mObjects[i]);
    for (int j=0; j < game->inPlay->nb_cards; j++){
      MTGCardInstance * card =  game->inPlay->cards[j];
      if (a->isReactingToClick(card,pMana)){
        createAbilityTargets(a, card, &ranking);
      }
    }
  }

  if (ranking.size()){
    AIAction * a = ranking.begin()->first;
    int chance = 1 + rand() % 100;
    if (getEfficiency(a) < chance){
      a = NULL;
    }else{
      OutputDebugString("AIPlayer:Using Activated ability\n");
      tapLandsForMana(pMana, a->ability->cost);
      clickstream.push(a);
    }
    map<AIAction *, int, CmpAbilities>::iterator it2;
    for (it2 = ranking.begin(); it2!=ranking.end(); it2++){
      if (a != it2->first) delete(it2->first);
    }
  }
  return 1;
}



int AIPlayer::interruptIfICan(){
  GameObserver * g = GameObserver::GetInstance();

  if (g->mLayers->stackLayer()->askIfWishesToInterrupt == this){
      if (!clickstream.empty()) g->mLayers->stackLayer()->cancelInterruptOffer();
      else g->mLayers->stackLayer()->setIsInterrupting(this);
      return 1;
  }
  return 0;
}

int AIPlayer::effectBadOrGood(MTGCardInstance * card){
  int id = card->getMTGId();
  AbilityFactory * af = NEW AbilityFactory();
  int autoGuess = af->magicText(id,NULL,card);
  delete af;
  if (autoGuess) return autoGuess;
  return BAKA_EFFECT_DONTKNOW;
}



int AIPlayer::chooseTarget(TargetChooser * tc){
  vector<Targetable *>potentialTargets;
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
      potentialTargets.push_back(target);
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
	  potentialTargets.push_back(card);
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
	clickstream.push(NEW AIAction(card));
	return 1;
	break;
      }
    case TARGET_PLAYER:
      {
	Player * player = ((Player *) potentialTargets[i]);
	clickstream.push(NEW AIAction(player));
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
  cd.unsecureSetTapped(untapMode);
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
  bool attack = ((myCreatures > opponentCreatures) || (myForce > opponentForce) || (myForce > 2*opponent()->life));
  if (agressivity > 80 && !attack && life > opponentForce) {
    opponentCreatures = getCreaturesInfo(opponent(), INFO_NBCREATURES,-1);
    opponentForce = getCreaturesInfo(opponent(),INFO_CREATURESPOWER,-1);
    attack = (myCreatures >= opponentCreatures && myForce > opponentForce) || (myForce > opponentForce) || (myForce > opponent()->life);
  }
  printf("Choose attackers : %i %i %i %i -> %i\n", opponentForce, opponentCreatures, myForce, myCreatures, attack);
  if (attack){
    CardDescriptor cd;
    cd.init();
    cd.setType("creature");
    MTGCardInstance * card = NULL;
    GameObserver * g = GameObserver::GetInstance();
    MTGAbility * a =  g->mLayers->actionLayer()->getAbility(MTGAbility::MTG_ATTACK_RULE);
    while((card = cd.nextmatch(game->inPlay, card))){
      g->mLayers->actionLayer()->reactToClick(a,card);
    }
  }
  return 1;
}

/* Can I first strike my oponent and get away with murder ? */
int AIPlayer::canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy){
  if (ennemy->has(Constants::FIRSTSTRIKE) || ennemy->has(Constants::DOUBLESTRIKE)) return 0;
  if (!(card->has(Constants::FIRSTSTRIKE) || card->has(Constants::DOUBLESTRIKE))) return 0;
  if (!(card->power >= ennemy->toughness)) return 0;
  return 1;
}

int AIPlayer::chooseBlockers(){
  map<MTGCardInstance *, int> opponentsToughness;
  int opponentForce = getCreaturesInfo(opponent(),INFO_CREATURESPOWER);
  //int opponentCreatures = getCreaturesInfo(opponent(), INFO_NBCREATURES, -1);
  //int myForce = getCreaturesInfo(this,INFO_CREATURESPOWER);
  //int myCreatures = getCreaturesInfo(this, INFO_NBCREATURES, -1);
  CardDescriptor cd;
  cd.init();
  cd.setType("Creature");
  cd.unsecureSetTapped(-1);
  MTGCardInstance * card = NULL;
  GameObserver * g = GameObserver::GetInstance();
  MTGAbility * a =  g->mLayers->actionLayer()->getAbility(MTGAbility::MTG_BLOCK_RULE);

  while((card = cd.nextmatch(game->inPlay, card))){
    g->mLayers->actionLayer()->reactToClick(a,card);
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
      g->mLayers->actionLayer()->reactToClick(a,card);
	}
      }
    }
  }
  card = NULL;
  while((card = cd.nextmatch(game->inPlay, card))){
    if (card->defenser && opponentsToughness[card->defenser] > 0){
      while (card->defenser){

        g->mLayers->actionLayer()->reactToClick(a,card);
      }
    }
  }
  card = NULL;
  while((card = cd.nextmatch(game->inPlay, card))){
    if(!card->defenser){
      g->mLayers->actionLayer()->reactToClick(a,card);
      int set = 0;
      while(!set){
	if (!card->defenser){
	  set = 1;
	}else{
	  MTGCardInstance * attacker = card->defenser;
	  if (opponentsToughness[attacker] <= 0 ||
    (card->toughness <= attacker->power && opponentForce*2 <life  && !canFirstStrikeKill(card,attacker))  ||
    attacker->nbOpponents()>1){
      g->mLayers->actionLayer()->reactToClick(a,card);
	  }else{
	    set = 1;
	  }
	}
      }
    }
  }
  return 1;
}

int AIPlayer::orderBlockers(){

  GameObserver * g = GameObserver::GetInstance();
  if (ORDER == g->combatStep && g->currentPlayer==this)
    {
      OutputDebugString("AIPLAYER: order blockers\n");
      g->userRequestNextGamePhase(); //TODO clever rank of blockers
      return 1;
    }

  return 0;
}

int AIPlayer::affectCombatDamages(CombatStep step){
  GameObserver * g = GameObserver::GetInstance();
  GuiCombat *  gc = g->mLayers->combatLayer();
  for (vector<AttackerDamaged*>::iterator attacker = gc->attackers.begin(); attacker != gc->attackers.end(); ++attacker)
          gc->autoaffectDamage(*attacker, step);
  return 1;
}

//TODO: Deprecate combatDamages
int AIPlayer::combatDamages(){
  //int result = 0;
  GameObserver * gameObs = GameObserver::GetInstance();
  int currentGamePhase = gameObs->getCurrentGamePhase();

  if (currentGamePhase == Constants::MTG_PHASE_COMBATBLOCKERS) return orderBlockers();

  if (currentGamePhase != Constants::MTG_PHASE_COMBATDAMAGE) return 0;

  return 0;

}


AIStats * AIPlayer::getStats(){
  if (!stats){
    char statFile[512];
    sprintf(statFile, RESPATH"/ai/baka/stats/%s.stats", opponent()->deckFileSmall.c_str());
    stats = NEW AIStats(this, statFile);
  }
  return stats;
}

AIPlayer * AIPlayerFactory::createAIPlayer(MTGAllCards * collection, Player * opponent, int deckid){
  char deckFile[512];
  char avatarFile[512];
  char deckFileSmall[512];

  if (deckid == -1){ //Evil twin
    sprintf(deckFile, opponent->deckFile.c_str());
    OutputDebugString(opponent->deckFile.c_str());  
    sprintf(avatarFile, "baka.jpg");
    sprintf(deckFileSmall, "ai_baka_eviltwin");
  }else{
    if (!deckid){
      int nbdecks = 0;
      int found = 1;
      while (found){
        found = 0;
        char buffer[512];
        sprintf(buffer, RESPATH"/ai/baka/deck%i.txt",nbdecks+1);
        std::ifstream file(buffer);
        if(file){
          found = 1;
          file.close();
          nbdecks++;
        }
      }
      if (!nbdecks) return NULL;
      deckid = 1 + rand() % (nbdecks);
    }
    sprintf(deckFile, RESPATH"/ai/baka/deck%i.txt",deckid);
    sprintf(avatarFile, "avatar%i.jpg",deckid);
    sprintf(deckFileSmall, "ai_baka_deck%i",deckid);
  }

  MTGDeck * tempDeck = NEW MTGDeck(deckFile, collection);
  MTGPlayerCards * deck = NEW MTGPlayerCards(collection,tempDeck);
  delete tempDeck;
  AIPlayerBaka * baka = NEW AIPlayerBaka(deck,deckFile, deckFileSmall, avatarFile);
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
    if (card->hasType("land") && !this->canPutLandsIntoPlay) continue;
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
          shouldPlayPercentage = 80;
        }
        if (rand() % 100 > shouldPlayPercentage) continue;
      }
      nextCardToPlay = card;
      maxCost = currentCost;
    }
  }
  return nextCardToPlay;
}

AIPlayerBaka::AIPlayerBaka(MTGPlayerCards * deck, string file, string fileSmall, string avatarFile) : AIPlayer(deck, file, fileSmall) {
  mAvatarTex = resources.RetrieveTexture(avatarFile,RETRIEVE_VRAM,TEXTURE_SUB_AVATAR);
  
  if(!mAvatarTex){
    avatarFile = "baka.jpg";
    mAvatarTex = resources.RetrieveTexture(avatarFile,RETRIEVE_VRAM,TEXTURE_SUB_AVATAR);
  }

  if(mAvatarTex)
    mAvatar = resources.RetrieveQuad(avatarFile, 0, 0, 35, 50,"bakaAvatar",RETRIEVE_VRAM,TEXTURE_SUB_AVATAR);
  else 
    mAvatar = NULL;

  initTimer();
}

void AIPlayerBaka::initTimer(){
  timer = 0.1;
}

int AIPlayerBaka::computeActions(){
  GameObserver * g = GameObserver::GetInstance();
  Player * p = g->currentPlayer;
  if (!(g->currentlyActing() == this)) return 0;
  if (g->mLayers->actionLayer()->menuObject){
    g->mLayers->actionLayer()->doReactTo(0);
    return 1;
  }
  if (chooseTarget()) return 1;
  int currentGamePhase = g->getCurrentGamePhase();
  if (g->isInterrupting == this){ // interrupting
    selectAbility();
    return 1;
  }else if (p == this && g->mLayers->stackLayer()->count(0,NOT_RESOLVED) == 0){ //standard actions
    CardDescriptor cd;
    MTGCardInstance * card = NULL;
    switch(currentGamePhase){
    case Constants::MTG_PHASE_FIRSTMAIN:
    case Constants::MTG_PHASE_SECONDMAIN:
    {
      if (canPutLandsIntoPlay){
	      //Attempt to put land into play
	      cd.init();
	      cd.setColor(Constants::MTG_COLOR_LAND);
	      card = cd.match(game->hand);
	      if (card){
          AIAction * a = NEW AIAction(card);
	        clickstream.push(a);
          return 1;
	      }
      }

	    //No mana, try to get some
	    SAFE_DELETE(potentialMana);
      ManaCost * currentMana = manaPool;
      if (!currentMana->getConvertedCost()){
        currentMana = getPotentialMana();
      }
	    if (currentMana->getConvertedCost() > 0){


	      //look for the most expensive creature we can afford
	      nextCardToPlay = FindCardToPlay(currentMana, "creature");
	      //Let's Try an enchantment maybe ?
	      if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(currentMana, "enchantment");
	      if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(currentMana, "artifact");
	      if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(currentMana, "instant");
	      if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(currentMana, "sorcery");
	      if (nextCardToPlay){
#if defined (WIN32) || defined (LINUX)
          char buffe[4096];
	  sprintf(buffe, "Putting Card Into Play: %s", nextCardToPlay->getName().c_str());
	  OutputDebugString(buffe);
#endif

	        if (currentMana == potentialMana) tapLandsForMana(currentMana,nextCardToPlay->getManaCost());
          AIAction * a = NEW AIAction(nextCardToPlay);
	        clickstream.push(a);
          return 1;
        }else{
          selectAbility();
        }
      }else{
        selectAbility();
      }
      break;
    }
    case Constants::MTG_PHASE_COMBATATTACKERS:
      chooseAttackers();
      break;
    default:
      selectAbility();
      break;
    }
  }else{
    cout << "my turn" << endl;
    switch(currentGamePhase){
    case Constants::MTG_PHASE_COMBATBLOCKERS:
      chooseBlockers();
      break;
    default:
      break;
    }
    return 1;
  }
  return 1;
};

int AIPlayerBaka::Act(float dt){
  GameObserver * g = GameObserver::GetInstance();

  if (!(g->currentlyActing() == this)){
    return 0;
  }

  int currentGamePhase = g->getCurrentGamePhase();

  if (currentGamePhase == Constants::MTG_PHASE_CLEANUP && currentGamePhase != oldGamePhase){
    if (getStats()) getStats()->updateStats();
  }
  oldGamePhase = currentGamePhase;

  timer-= dt;
  if (timer>0){
    return 0;
  }
  initTimer();
  if (combatDamages()){
    return 0;
  }
  interruptIfICan();
  if (!(g->currentlyActing() == this)){
    OutputDebugString("Cannot interrupt\n");
    return 0;
  }
  if (clickstream.empty()) computeActions();
  if (clickstream.empty()){
    if (g->isInterrupting == this){
      g->mLayers->stackLayer()->cancelInterruptOffer(); //endOfInterruption();
    }else{
      g->userRequestNextGamePhase();
    }
  } else {
    AIAction * action = clickstream.front();
    action->Act();
    SAFE_DELETE(action);
    clickstream.pop();
  }
  return 1;
};

