#include "../include/config.h"
#include "../include/MTGRules.h"

MTGPutInPlayRule::MTGPutInPlayRule(int _id):MTGAbility(_id, NULL){
  aType=MTGAbility::PUT_INTO_PLAY;
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
  }else if ((card->hasType("instant")) || card->has(Constants::FLASH) || (player == currentPlayer && !game->isInterrupting && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN))){
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
  aType=MTGAbility::MTG_ATTACK_RULE;
}

int MTGAttackRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana){
  if (currentPhase == Constants::MTG_PHASE_COMBATATTACKERS && card->controller() == game->currentPlayer && !card->isAttacker()){
    if (card->canAttack()) return 1;
  }
  return 0;
}

void MTGAttackRule::Update(float dt){
  if (currentPhase != newPhase && currentPhase == Constants::MTG_PHASE_COMBATATTACKERS){
    Player * p = game->currentPlayer;
    MTGGameZone * z = p->game->inPlay;
    for (int i= 0; i < z->nb_cards; i++){
      MTGCardInstance * card = z->cards[i];
      if (!card->isAttacker() && card->has(Constants::MUSTATTACK)) reactToClick(card);
    }
  }
  MTGAbility::Update(dt);
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
  aType=MTGAbility::MTG_BLOCK_RULE;
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


//
// * Momir
//

int MTGMomirRule::initialized = 0;
vector<int> MTGMomirRule::pool[20];

MTGMomirRule::MTGMomirRule(int _id, MTGAllCards * _collection):MTGAbility(_id, NULL){
  collection = _collection;
  if (!initialized){
    int total_cards = collection->totalCards();
    for (int i = 0; i < total_cards; i++){
      MTGCard * card = collection->collection[i];
      if (card->isACreature()){
         int convertedCost = card->getManaCost()->getConvertedCost();
         if (convertedCost>20) continue;
         pool[convertedCost].push_back(card->getMTGId());
      }
    }
    initialized =1;
  }
  alreadyplayed = 0;
  aType=MTGAbility::MOMIR;
  textAlpha = 0;
}

int MTGMomirRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana){
  if (alreadyplayed) return 0;
  Player * player = game->currentlyActing();
  Player * currentPlayer = game->currentPlayer;
  LOG("CANPUTINPLAY- check if card belongs to current player\n");
  if (!player->game->hand->hasCard(card)) return 0;
  LOG("CANPUTINPLAY- check if card is land or can be played\n");
  if (player == currentPlayer && !game->isInterrupting && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN)){
    LOG("CANPUTINPLAY- correct time to play\n");
	return 1;
  }
  return 0;
}

int MTGMomirRule::reactToClick(MTGCardInstance * card_to_discard){
  Player * player = game->currentlyActing();
  ManaCost * cost = player->getManaPool();
  int converted = cost->getConvertedCost();
  int id = genRandomCreatureId(converted);
  return reactToClick(card_to_discard, id);
}

int MTGMomirRule::reactToClick(MTGCardInstance * card_to_discard, int cardId){
  if (!isReactingToClick(card_to_discard)) return 0;
   Player * player = game->currentlyActing();
  ManaCost * cost = player->getManaPool(); 
  player->getManaPool()->pay(cost);
   MTGCardInstance * card = genCreature(cardId);
  player->game->putInZone(card_to_discard,  player->game->hand, player->game->graveyard);

  player->game->stack->addCard(card);
  Spell * spell = NEW Spell(card);
  spell->resolve();
  spell->source->isToken = 1;
  delete spell;
	alreadyplayed = 1;
  textAlpha = 255;
  text = card->name;
  return 1;
}

MTGCardInstance * MTGMomirRule::genCreature( int id){
  if (!id) return NULL;
  Player * p = game->currentlyActing();
  MTGCard * card = collection->getCardById(id);
  return NEW MTGCardInstance(card,p->game);
}

int MTGMomirRule::genRandomCreatureId(int convertedCost){
   if (convertedCost > 20) return 0;
   int total_cards = pool[convertedCost].size();
   if (!total_cards) return 0;
   int start = (rand() % total_cards);
   return pool[convertedCost][start];
}

//The Momir rule is never destroyed
int MTGMomirRule::testDestroy(){
  return 0;
}

void MTGMomirRule::Update(float dt){
  if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP){
    alreadyplayed = 0;
  }
  if (textAlpha){
    textAlpha -= (200*dt);
    if (textAlpha <0) textAlpha = 0;
  }
  MTGAbility::Update(dt);
}

void MTGMomirRule::Render(){
  if (!textAlpha) return;
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MENU_FONT);
  mFont->SetScale(2 - (float)textAlpha/130);
  mFont->SetColor(ARGB(textAlpha,255,255,255));
  mFont->DrawString(text.c_str(),SCREEN_WIDTH/2,SCREEN_HEIGHT/2,JGETEXT_CENTER);
}