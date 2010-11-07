#include "PrecompiledHeader.h"

#include "TestSuiteAI.h"
#include "MTGAbility.h"
#include "MTGRules.h"
#include "ActionLayer.h"
#include "GuiCombat.h"
#include "Rules.h"
#include "GameObserver.h"
#include "GameStateShop.h"

using std::string;

// NULL is sent in place of a MTGDeck since there is no way to create a MTGDeck without a proper deck file.
// TestSuiteAI will be responsible for managing its own deck state.
TestSuiteAI::TestSuiteAI(TestSuite * _suite, int playerId):AIPlayerBaka(NULL, "testsuite", "testsuite", "baka.jpg") {
  this->game = _suite->buildDeck(playerId);
  game->setOwner( this );
  suite = _suite;
  timer = 0;
  playMode = MODE_TEST_SUITE;
  this->deckName = "Test Suite AI";
}


MTGCardInstance * TestSuiteAI::getCard(string action){
  int mtgid = Rules::getMTGId(action);
  if (mtgid) return Rules::getCardByMTGId(mtgid);

  //This mostly handles tokens
  GameObserver * g = GameObserver::GetInstance();
  std::transform(action.begin(), action.end(), action.begin(),::tolower );
  for (int i = 0; i < 2; i++){
    Player * p = g->players[i];
    MTGGameZone * zones[] = {p->game->library,p->game->hand,  p->game->inPlay, p->game->graveyard};
    for (int j = 0; j < 4; j++){
      MTGGameZone * zone = zones[j];
      for (int k = 0; k < zone->nb_cards; k++){
	      MTGCardInstance * card = zone->cards[k];
	      if (!card) return NULL;
        string name = card->getLCName();
        if (name.compare(action) == 0) return card;
      }
    }
  }
  DebugTrace("TESTUISTEAI: Can't find card:" << action.c_str());
  return NULL;
}


Interruptible * TestSuite::getActionByMTGId(int mtgid){
  ActionStack * as= GameObserver::GetInstance()->mLayers->stackLayer();
  Interruptible * action = NULL;
  while ((action = as->getNext(action,0,0,1))){
    if (action->source && action->source->getMTGId() == mtgid){
      return action;
    }
  }
  return NULL;
}

int TestSuiteAI::displayStack(){
  if (playMode == MODE_AI) return 0;
  return 1;
}

int TestSuiteAI::Act(float dt){
  GameObserver * g = GameObserver::GetInstance();
  g->gameOver = NULL; // Prevent draw rule from losing the game
  if (playMode == MODE_AI && suite->aiMaxCalls) {
    suite->aiMaxCalls--;
    suite->timerLimit = 40; //TODO Remove this limitation when AI is not using a stupid timer anymore...
    AIPlayerBaka::Act(dt);
  }
  if (playMode == MODE_HUMAN){
    g->mLayers->CheckUserInput(0);
    return 1;
  }

  
  timer+= 1;
  if (timer < suite->timerLimit) return 1;
  timer = 0;

  string action = suite->getNextAction();
  g->mLayers->stackLayer()->Dump();
  //  DamageResolverLayer * drl = g->mLayers->combatLayer();
  DebugTrace("TESTSUITE command: " << action); 

  if (g->mLayers->stackLayer()->askIfWishesToInterrupt == this){
    if(action.compare("no") != 0 && action.compare("yes") != 0){
      g->mLayers->stackLayer()->cancelInterruptOffer();
      suite->currentAction--;
      return 1;
    }
  }

  if (action == ""){
    //end of game
    suite->assertGame();
    g->gameOver = g->players[0];
    DebugTrace("================================    END OF TEST   =======================\n"); 
    return 1;
  }

  if (action.compare("eot")== 0){
    if (g->getCurrentGamePhase() != Constants::MTG_PHASE_CLEANUP) suite->currentAction--;
    g->userRequestNextGamePhase();
  }
  else if (action.compare("human")==0){
    DebugTrace("TESTSUITE You have control");
    playMode = MODE_HUMAN;
    return 1;
  }
  else if (action.compare("ai")==0){
    DebugTrace("TESTSUITE Switching to AI");
    playMode = MODE_AI;
    return 1;
  }
  else if (action.compare("next")==0){
    GuiCombat * gc = g->mLayers->combatLayer();
    if (ORDER == g->combatStep || DAMAGE == g->combatStep) 
      gc->clickOK();
    else g->userRequestNextGamePhase();
  }
  else if (action.compare("yes")==0)
    g->mLayers->stackLayer()->setIsInterrupting(this);
  else if (action.compare("endinterruption")==0)
    g->mLayers->stackLayer()->endOfInterruption();
  else if(action.compare("no")==0){
    if (g->mLayers->stackLayer()->askIfWishesToInterrupt == this)
      g->mLayers->stackLayer()->cancelInterruptOffer();
  }else if(action.find("choice ")!=string::npos){
    DebugTrace("TESTSUITE choice !!!");
    int choice = atoi(action.substr(action.find("choice ") + 7).c_str());
    g->mLayers->actionLayer()->doReactTo(choice);
  }else if(action.find(" -momir- ")!=string::npos){
    int start = action.find(" -momir- ");
    int cardId = Rules::getMTGId(action.substr(start + 9).c_str());
    int cardIdHand = Rules::getMTGId(action.substr(0,start).c_str());
    MTGMomirRule * a = ((MTGMomirRule *)g->mLayers->actionLayer()->getAbility(MTGAbility::MOMIR));
    a->reactToClick(Rules::getCardByMTGId(cardIdHand), cardId);
    g->mLayers->actionLayer()->stuffHappened = 1;
  }else if(action.find("p1")!=string::npos || action.find("p2")!=string::npos){
    Player * p = g->players[1];
    size_t start = action.find("p1");
    if (start != string::npos) p = g->players[0];
    g->cardClick(NULL, p);
  }else{
    int mtgid = Rules::getMTGId(action);
    Interruptible * toInterrupt = NULL;
    if (mtgid){
      DebugTrace("TESTSUITE CARD ID:" << mtgid);
      toInterrupt = suite->getActionByMTGId(mtgid);
    }

    if (toInterrupt) {
      g->stackObjectClicked(toInterrupt);
      return 1;
    }

    MTGCardInstance * card = getCard(action);
    if (card) {
      DebugTrace("TESTSUITE Clicking ON: " << card->name); 
      card->currentZone->needShuffle = true; //mimic library shuffle
      g->cardClick(card,card);
      g->forceShuffleLibraries(); //mimic library shuffle
      return 1;
    }
  }
  return 0;
}


TestSuiteActions::TestSuiteActions(){
  nbitems = 0;
}

void TestSuiteActions::add(string s){
  actions[nbitems] = s;
  nbitems++;
}

TestSuitePlayerData::TestSuitePlayerData(){
  life = 20;
  manapool = NEW ManaCost();
}

TestSuitePlayerData::~TestSuitePlayerData(){
  SAFE_DELETE(manapool);
}

TestSuitePlayerZone::TestSuitePlayerZone(){
  nbitems = 0;
}

void TestSuitePlayerZone::add(int cardId){
  cards[nbitems] = cardId;
  nbitems++;
}

TestSuiteState::TestSuiteState(){

}

void TestSuiteState::parsePlayerState(int playerId, string s){
  size_t limiter = s.find(":");
  string areaS;
  int area;
  if (limiter != string::npos){
    areaS = s.substr(0,limiter);
    if (areaS.compare("graveyard") == 0){
      area = 0;
    }else if(areaS.compare("library")  == 0){
      area = 1;
    }else if(areaS.compare("hand")  == 0){
      area = 2;
    }else if(areaS.compare("inplay")  == 0 || areaS.compare("battlefield")  == 0  ){
      area = 3;
    }else if(areaS.compare("life")  == 0){
      playerData[playerId].life = atoi((s.substr(limiter+1)).c_str());
      return;
    }else if(areaS.compare("manapool")  == 0){
      SAFE_DELETE(playerData[playerId].manapool);
      playerData[playerId].manapool = ManaCost::parseManaCost(s.substr(limiter+1));
      return;
    }else{
      return; // ERROR
    }
    s = s.substr(limiter+1);
    while (s.size()){
      unsigned int value;
      limiter = s.find(",");
      if (limiter != string::npos){
        value = Rules::getMTGId(trim(s.substr(0,limiter)));
	      s = s.substr(limiter+1);
      }else{
	      value = Rules::getMTGId(trim(s));
	      s = "";
      }
      if (value) playerData[playerId].zones[area].add(value);
    }
  }else{
    //ERROR
  }
}


string TestSuite::getNextAction(){
  currentAction++;
  if (actions.nbitems && currentAction <= actions.nbitems){
    return actions.actions[currentAction-1];
  }
  return "";
}


MTGPlayerCards * TestSuite::buildDeck( int playerId){
  int list[100];
  int nbcards = 0;
  for (int j = 0; j < 4; j++){
    for (int k = 0; k < initState.playerData[playerId].zones[j].nbitems; k++){
      int cardid = initState.playerData[playerId].zones[j].cards[k];
      list[nbcards] = cardid;
      nbcards++;
    }
  }
  MTGPlayerCards * deck = NEW MTGPlayerCards(list, nbcards);
  return deck;
}

void TestSuite::initGame(){
  //The first test runs slowly, the other ones run faster.
  //This way a human can see what happens when testing a specific file,
  // or go faster when it comes to the whole test suite.
  //Warning, putting this value too low (< 3) will give unexpected results
  if (!timerLimit){
    timerLimit = 40;
  }else{
    timerLimit = 3;
  }
  //Put the GameObserver in the initial state
  GameObserver * g = GameObserver::GetInstance();
  DebugTrace("TESTSUITE Init Game");
  g->phaseRing->goToPhase(initState.phase, g->players[0]);
  g->currentGamePhase = initState.phase;
  for (int i = 0; i < 2; i++){
    AIPlayer * p = (AIPlayer *) (g->players[i]);
    p->forceBestAbilityUse = forceAbility;
    p->life = initState.playerData[i].life;
    p->getManaPool()->copy(initState.playerData[i].manapool);
    MTGGameZone * playerZones[] = {p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay};
    for (int j = 0; j < 4; j++){
      MTGGameZone * zone = playerZones[j];
      for (int k = 0; k < initState.playerData[i].zones[j].nbitems; k++){
        MTGCardInstance * card = Rules::getCardByMTGId(initState.playerData[i].zones[j].cards[k]);
	      if (card && zone != p->game->library){
	        if (zone == p->game->inPlay){
	          MTGCardInstance * copy = p->game->putInZone(card,  p->game->library, p->game->stack);
	          Spell * spell = NEW Spell(copy);
	          spell->resolve();
            if (!summoningSickness && p->game->inPlay->nb_cards>k) p->game->inPlay->cards[k]->summoningSickness = 0;
	          delete spell;
	        }else{
	          if (!p->game->library->hasCard(card)){
	            LOG ("TESTUITE ERROR, CARD NOT FOUND IN LIBRARY\n");
	          }
	          p->game->putInZone(card,p->game->library,zone);
	        }
        }else{
	if (!card) { LOG ("TESTUITE ERROR, card is NULL\n"); }
        }
      }
    }
  }
  DebugTrace("TESTUITE Init Game Done !");
}
int TestSuite::Log(const char * text){
  ofstream file (JGE_GET_RES("test/results.html").c_str(),ios_base::app);
  if (file){
    file << text;
    file << "\n";
    file.close();
  }

  DebugTrace(text);
  return 1;

}
int TestSuite::assertGame(){
  //compare the game state with the results
  char result[4096];
  sprintf(result,"<h3>%s</h3>",files[currentfile-1].c_str());
  Log(result);

  int error = 0;
  bool wasAI = false;

  GameObserver * g = GameObserver::GetInstance();
  if (g->currentGamePhase != endState.phase){
    sprintf(result, "<span class=\"error\">==phase problem. Expected %i, got %i==</span><br />",endState.phase, g->currentGamePhase);
    Log(result);
    error++;
  }
  for (int i = 0; i < 2; i++){
    TestSuiteAI * p = (TestSuiteAI *)(g->players[i]);
    if (p->playMode == Player::MODE_AI) wasAI = true;

    if (p->life != endState.playerData[i].life){
      sprintf(result, "<span class=\"error\">==life problem for player %i. Expected %i, got %i==</span><br />",i,endState.playerData[i].life, p->life);
      Log(result);
      error++;
    }
    if (! p->getManaPool()->canAfford(endState.playerData[i].manapool)){
      sprintf(result, "<span class=\"error\">==Mana problem. Was expecting %i but got %i for player %i==</span><br />",endState.playerData[i].manapool->getConvertedCost(),p->getManaPool()->getConvertedCost(),i);
      Log(result);
      error++;
    }
    if(! endState.playerData[i].manapool->canAfford(p->getManaPool())){
      sprintf(result, "<span class=\"error\">==Mana problem. Was expecting %i but got %i for player %i==</span><br />",endState.playerData[i].manapool->getConvertedCost(),p->getManaPool()->getConvertedCost(),i);
      Log(result);
      error++;

    }
    MTGGameZone * playerZones[] = {p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay};
    for (int j = 0; j < 4; j++){
      MTGGameZone * zone = playerZones[j];
      if (zone->nb_cards != endState.playerData[i].zones[j].nbitems){
        sprintf(result, "<span class=\"error\">==Card number not the same in player %i's %s==, expected %i, got %i</span><br />",i, zone->getName(), endState.playerData[i].zones[j].nbitems, zone->nb_cards);
        Log(result);
	      error++;
      }
      for (int k = 0; k < endState.playerData[i].zones[j].nbitems; k++){
	      int cardid = endState.playerData[i].zones[j].cards[k];
        if (cardid != -1){
          MTGCardInstance * card = Rules::getCardByMTGId(cardid);
          if (!card || !zone->hasCard(card)){
	          sprintf(result, "<span class=\"error\">==Card ID not the same. Didn't find %i</span><br />", cardid);
	          Log(result);
	          error++;
	        }
        }
      }
    }
  }
  if (wasAI) {
    nbAITests++;
    if (error) {
      nbAIFailed++;
      return 0;
    }
  } else {
    nbTests++;
    if (error) {
      nbFailed++;
      return 0;
    }
  }
  Log("<span class=\"success\">==Test Succesful !==</span>");
  return 1;
}

TestSuite::TestSuite(const char * filename,MTGAllCards* _collection){
  collection=_collection;
  timerLimit = 0;
  std::ifstream file(filename);
  std::string s;
  nbfiles = 0;
  currentfile = 0;
  nbFailed = 0;
  nbTests = 0;
  nbAIFailed = 0;
  nbAITests = 0;
  int comment = 0;
  seed = 0;
  forceAbility = false;
  aiMaxCalls = -1;
  if(file){
    while(std::getline(file,s)){
      if (!s.size()) continue;
      if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
      if (s[0] == '/' && s[1] == '*') comment = 1;
      if (s[0] && s[0] != '#' && !comment){
	files[nbfiles] = s;
	nbfiles++;
      }
      if (s[0] == '*' && s[1] == '/') comment = 0;
    }
    file.close();
  }

  ofstream file2 (JGE_GET_RES("/test/results.html").c_str());
  if (file2){
    file2 << "<html><head>";
#ifdef WIN32
    file2 << "<meta http-equiv=\"refresh\" content=\"10\" >";
#endif
    file2 << "<STYLE type='text/css'>";
    file2 << ".success {color:green}\n";
    file2 << ".error {color:red}\n";
    file2 << "</STYLE></head><body>\n";
    file2.close();
  }

}

int TestSuite::loadNext(){
  summoningSickness = 0;
  seed = 0;
  aiMaxCalls = -1;
  if (!nbfiles) return 0;
  if (currentfile >= nbfiles) return 0;
  currentfile++;
  if (!load(files[currentfile-1].c_str())) return loadNext();
  else cout << "Starting test : " << files[currentfile-1] << endl;
  //load(files[currentfile].c_str());
  //currentfile++;
  return currentfile;
}


void TestSuiteActions::cleanup(){
  nbitems = 0;
}

void TestSuitePlayerZone::cleanup(){
  nbitems = 0;
}

void TestSuitePlayerData::cleanup(){
  if (manapool) delete manapool;
  manapool = NULL;
  manapool = NEW ManaCost();
  for (int i = 0; i < 5; i++){
    zones[i].cleanup();
  }
  life=20;
}

void TestSuiteState::cleanup(){
  for (int i = 0; i < 2; i++){
    playerData[i].cleanup();
  }
}

void TestSuite::cleanup(){
  currentAction = 0;
  initState.cleanup();
  endState.cleanup();
  actions.cleanup();
  loadRandValues("");
}

int TestSuite::load(const char * _filename){
  summoningSickness = 0;
  forceAbility = false;
  gameType = GAME_TYPE_CLASSIC;
  char filename[4096];
  sprintf(filename, JGE_GET_RES("/test/%s").c_str(), _filename);
  std::ifstream file(filename);
  std::string s;
  loadRandValues("");

  int state = -1;

  //  std::cout << std::endl << std::endl << "!!!" << file << std::endl << std::endl;
  if(file){
    cleanup();
    while(std::getline(file,s)){
      if (!s.size()) continue;
      if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
      if (s[0] == '#') continue;
      std::transform( s.begin(), s.end(), s.begin(),::tolower );
      if (s.compare("summoningsickness") == 0) {
        summoningSickness = 1;
        continue;
      }
      if (s.compare("forceability") == 0) {
        forceAbility = true;
        continue;
      }
      if (s.find("seed ") == 0) {
        seed = atoi(s.substr(5).c_str());
        continue;
      }
      if (s.find("rvalues:") == 0) {
        loadRandValues(s.substr(8).c_str());
        continue;
      }
      if (s.find("aicalls ") == 0) {
        aiMaxCalls = atoi(s.substr(8).c_str());
        continue;
      }
      if (s.compare("momir") == 0) {
        gameType = GAME_TYPE_MOMIR;
        continue;
      }
      switch(state){
      case -1:
	if (s.compare("[init]") == 0) state++;
	break;
      case 0:
	if (s.compare("[player1]") == 0){
	  state++;
	}else{
    initState.phase = PhaseRing::phaseStrToInt(s);
	}
	break;
      case 1:
	if (s.compare("[player2]") == 0){
	  state++;
	}else{
	  initState.parsePlayerState(0, s);
	}
	break;
      case 2:
	if (s.compare("[do]") == 0){
	  state++;
	}else{
	  initState.parsePlayerState(1, s);
	}
	break;
      case 3:
	if (s.compare("[assert]") == 0){
	  state++;
	}else{
	  actions.add(s);
	}
	break;
      case 4:
	if (s.compare("[player1]") == 0){
	  state++;
	}else{
    endState.phase = PhaseRing::phaseStrToInt(s);
	}
	break;
      case 5:
	if (s.compare("[player2]") == 0){
	  state++;
	}else{
	  endState.parsePlayerState(0, s);
	}
	break;
      case 6:
	if (s.compare("[end]") == 0){
	  state++;
	}else{
	  endState.parsePlayerState(1, s);
	}
	break;
      }
    }
    file.close();
  }else{
    return 0;
  }
  return 1;
}


void TestSuite::pregameTests(){
    //Test Booster Generation
    srand(1024);
    char result[1024];
    ShopBooster sb;
    for(int i=0;i<5;i++){
        nbTests++;
        sprintf(result, "<h3>pregame/BoosterTest#%i</h3>", i);
        Log(result);
        if(!sb.unitTest())
            nbFailed++;
    }
}