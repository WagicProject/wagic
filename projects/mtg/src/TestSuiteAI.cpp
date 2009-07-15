#include "../include/TestSuiteAI.h"
#include "../include/config.h"
#include "../include/MTGAbility.h"
#include "../include/MTGRules.h"
#include "../include/ActionLayer.h"
#include "../include/DamageResolverLayer.h"

#include <string>
using std::string;

TestSuiteAI::TestSuiteAI(TestSuite * _suite, int playerId):AIPlayer(_suite->buildDeck(playerId),"testsuite", "testsuite"){
  suite = _suite;
  timer= 0;
  mAvatarTex = JRenderer::GetInstance()->LoadTexture("ai/baka/avatar.jpg", TEX_TYPE_USE_VRAM);
  if (mAvatarTex){
    mAvatar = NEW JQuad(mAvatarTex, 0, 0, 35, 50);
  }
}

int TestSuite::getMTGId(string cardName){
  int cardnb = atoi(cardName.c_str());
  if (cardnb) return cardnb;
  if (cardName.compare("*") == 0) return -1; //Any card
  MTGCard * card = collection->getCardByName(cardName);
  if (card) return card->getMTGId();
  return 0;
}

MTGCardInstance * TestSuite::getCardByMTGId(int mtgid){
  GameObserver * g = GameObserver::GetInstance();
  for (int i = 0; i < 2; i++){
    Player * p = g->players[i];
    MTGGameZone * zones[] = {p->game->library,p->game->hand,  p->game->inPlay, p->game->graveyard};
    for (int j = 0; j < 4; j++){
      MTGGameZone * zone = zones[j];
      for (int k = 0; k < zone->nb_cards; k++){
	MTGCardInstance * card = zone->cards[k];
	if (!card) OutputDebugString ("wtf ?");
	if (card->getMTGId() == mtgid) return card;
      }
    }
  }
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


int TestSuiteAI::Act(float dt){
  GameObserver * g = GameObserver::GetInstance();
  g->gameOver = NULL; // Prevent draw rule from losing the game
  timer+= dt;
  if (AManaProducer::currentlyTapping || timer < suite->timerLimit) return 1;
  timer = 0;

  string action = suite->getNextAction();
  g->mLayers->stackLayer()->Dump();
  DamageResolverLayer * drl = g->mLayers->combatLayer();

  OutputDebugString(action.c_str());
  OutputDebugString("\n");


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
    return 1;
  }

  if (action.compare("eot")== 0){
    if (g->getCurrentGamePhase() != Constants::MTG_PHASE_CLEANUP) suite->currentAction--;
    g->userRequestNextGamePhase();
  }
  else if (action.compare("next")==0){
    if (drl->orderingIsNeeded){
      drl->blockersOrderingDone();
      g->userRequestNextGamePhase();
    }else if (drl->mCount){
      OutputDebugString("End of combat damage!\n");
      drl->nextPlayer();
      g->userRequestNextGamePhase();
    }else{
      g->userRequestNextGamePhase();
    }
  }else if (action.compare("yes")==0){
    g->mLayers->stackLayer()->setIsInterrupting(this);
  }else if (action.compare("endinterruption")==0){
    g->mLayers->stackLayer()->endOfInterruption();
  }else if(action.compare("no")==0){
    if (g->mLayers->stackLayer()->askIfWishesToInterrupt == this){
      g->mLayers->stackLayer()->cancelInterruptOffer();
    }
  }else if(action.find("choice ")!=string::npos){
    OutputDebugString("choice !!!\n");
    int choice = atoi(action.substr(action.find("choice ") + 7).c_str());
    g->mLayers->actionLayer()->doReactTo(choice);
  }else if(action.find(" -momir- ")!=string::npos){
    int start = action.find(" -momir- ");
    int cardId = suite->getMTGId(action.substr(start + 9).c_str());
    int cardIdHand = suite->getMTGId(action.substr(0,start).c_str());
    MTGMomirRule * a = ((MTGMomirRule *)g->mLayers->actionLayer()->getAbility(MTGAbility::MOMIR));
    a->reactToClick(suite->getCardByMTGId(cardIdHand), cardId);
    g->mLayers->actionLayer()->stuffHappened = 1;
  }else if(action.find("p1")!=string::npos || action.find("p2")!=string::npos){
    Player * p = g->players[1];
    int start = action.find("p1");
    if (start != string::npos) p = g->players[0];
    g->cardClick(NULL, p);
  }else{
    int mtgid = suite->getMTGId(action);
    if (mtgid){
      char buffe[512];
      sprintf(buffe, "CARD ID : %i\n", mtgid);
      OutputDebugString(buffe);
      Interruptible * toInterrupt = suite->getActionByMTGId(mtgid);
	    if (toInterrupt){
	      g->stackObjectClicked(toInterrupt);
	    }else{
        MTGCardInstance * card = suite->getCardByMTGId(mtgid);
        if (card) {
          OutputDebugString("Clicking ON: ");
          OutputDebugString(card->name.c_str());
          if (drl->mCount){
            if (drl->orderingIsNeeded){
              OutputDebugString(" Ordering Card\n");
              drl->clickReorderBlocker(card);
            }else{
              OutputDebugString(" Damaging Card\n");
              drl->clickDamage(card);
            }
          }else{
	          g->cardClick(card,card);
          }
        }
      }
    }else{
      return 0;
    }
  }
  return 1;
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

void TestSuiteState::parsePlayerState(int playerId, string s, TestSuite * suite){
  unsigned int limiter = s.find(":");
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
	      value = suite->getMTGId(s.substr(0,limiter));
	      s = s.substr(limiter+1);
      }else{
	      value = suite->getMTGId(s);
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
  MTGPlayerCards * deck = NEW MTGPlayerCards(collection, list, nbcards);
  return deck;
}

void TestSuite::initGame(){
  //The first test runs slowly, the other ones run faster.
  //This way a human can see what happens when testing a specific file,
  // or go faster when it comes to the whole test suite.
  //Warning, putting this value too low (< 0.25) will give unexpected results
  if (!timerLimit){
    timerLimit = 0.5;
  }else{
    timerLimit = 0.1;
  }
  //Put the GameObserver in the initial state
  GameObserver * g = GameObserver::GetInstance();
  OutputDebugString("Init Game\n");
  g->phaseRing->goToPhase(initState.phase, g->players[0]);
  g->currentGamePhase = initState.phase;
  for (int i = 0; i < 2; i++){
    Player * p = g->players[i];
    p->life = initState.playerData[i].life;
    p->getManaPool()->copy(initState.playerData[i].manapool);
    MTGGameZone * playerZones[] = {p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay};
    for (int j = 0; j < 4; j++){
      MTGGameZone * zone = playerZones[j];
      for (int k = 0; k < initState.playerData[i].zones[j].nbitems; k++){
	      MTGCardInstance * card = getCardByMTGId(initState.playerData[i].zones[j].cards[k]);
        char buf[4096];
        sprintf(buf, "QUAD : %p\n", card->getQuad());
        OutputDebugString(buf);
	      if (card && zone != p->game->library){
	        if (zone == p->game->inPlay){
	          MTGCardInstance * copy = p->game->putInZone(card,  p->game->library, p->game->stack);
	          Spell * spell = NEW Spell(copy);          
	          spell->resolve();
            if (!summoningSickness && p->game->inPlay->nb_cards>k) p->game->inPlay->cards[k]->summoningSickness = 0;
	          delete spell;
	        }else{
	          if (!p->game->library->hasCard(card)){
	            LOG ("ERROR, CARD NOT FOUND IN LIBRARY\n");
	          }
	          p->game->putInZone(card,p->game->library,zone);
	        }
        }else{
	if (!card) { LOG ("ERROR, card is NULL\n"); }
        }
      }
    }
  }
  OutputDebugString("Init Game Done !\n");
}

int TestSuite::Log(const char * text){
  ofstream file (RESPATH"/test/results.html",ios_base::app);
  if (file){
    file << text;
    file << "\n";
    file.close();
  }
#if defined (WIN32) || defined (LINUX)
  OutputDebugString(text);
  OutputDebugString("\n");
#endif
  return 1;

}
int TestSuite::assertGame(){
  //compare the game state with the results
  char result[4096];
  sprintf(result,"<h3>%s</h3>",files[currentfile-1].c_str());
  Log(result);

  int error = 0;
  GameObserver * g = GameObserver::GetInstance();
  if (g->currentGamePhase != endState.phase){
    sprintf(result, "<span class=\"error\">==phase problem. Expected %i, got %i==</span><br />",endState.phase, g->currentGamePhase);
    Log(result);
    error++;
  }
  for (int i = 0; i < 2; i++){
    Player * p = g->players[i];
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
        sprintf(result, "<span class=\"error\">==Card number not the same in %i==, expected %i, got %i</span><br />",j, endState.playerData[i].zones[j].nbitems, zone->nb_cards);
        Log(result);
	      error++;
	      return 0;
      }
      for (int k = 0; k < endState.playerData[i].zones[j].nbitems; k++){
	      int cardid = endState.playerData[i].zones[j].cards[k];
        if (cardid != -1){
	        MTGCardInstance * card = getCardByMTGId(cardid);
          if (!card || !zone->hasCard(card)){
	          sprintf(result, "<span class=\"error\">==Card ID not the same. Didn't find %i</span><br />", cardid);
	          Log(result);
	          error++;
	        }
        }
      }
    }
  }
  if (error) return 0;
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
  int comment = 0;
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

  ofstream file2 (RESPATH"/test/results.html");
  if (file2){
    file2 << "<html><head>";
    file2 << "<meta http-equiv=\"refresh\" content=\"10\" >";
    file2 << "<STYLE type='text/css'>";
    file2 << ".success {color:green}\n";
    file2 << ".error {color:red}\n";
    file2 << "</STYLE></head><body>\n";
    file2.close();
  }

}

int TestSuite::loadNext(){
  summoningSickness = 0;
  if (!nbfiles) return 0;
  if (currentfile >= nbfiles) return 0;
  currentfile++;
  if (!load(files[currentfile-1].c_str())) return loadNext();
  
  //load(files[currentfile].c_str());
  //currentfile++;
  return currentfile;
}

//TODO PArses a string and gives phase numer
int TestSuite::phaseStrToInt(string s){
  if (s.compare("untap") == 0) return Constants::MTG_PHASE_UNTAP;
  if (s.compare("upkeep") == 0)return Constants::MTG_PHASE_UPKEEP;
  if (s.compare("draw") == 0)return Constants::MTG_PHASE_DRAW;
  if (s.compare("firstmain") == 0)return Constants::MTG_PHASE_FIRSTMAIN;
  if (s.compare("combatbegin") == 0)return Constants::MTG_PHASE_COMBATBEGIN;
  if (s.compare("combatattackers") == 0)return Constants::MTG_PHASE_COMBATATTACKERS;
  if (s.compare("combatblockers") == 0)return Constants::MTG_PHASE_COMBATBLOCKERS;
  if (s.compare("combatdamage") == 0)return Constants::MTG_PHASE_COMBATDAMAGE;
  if (s.compare("combatend") == 0)return Constants::MTG_PHASE_COMBATEND;
  if (s.compare("secondmain") == 0)return Constants::MTG_PHASE_SECONDMAIN;
  if (s.compare("endofturn") == 0)return Constants::MTG_PHASE_ENDOFTURN;
  if (s.compare("cleanup") == 0)return Constants::MTG_PHASE_CLEANUP;
  return -1;
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
}

int TestSuite::load(const char * _filename){
  summoningSickness = 0;
  gameType = GAME_TYPE_CLASSIC;
  char filename[4096];
  sprintf(filename, RESPATH"/test/%s", _filename);
  std::ifstream file(filename);
  std::string s;

  int state = -1;

  std::cout << std::endl << std::endl << "!!!" << file << std::endl << std::endl;
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
	  initState.phase = phaseStrToInt(s);
	}
	break;
      case 1:
	if (s.compare("[player2]") == 0){
	  state++;
	}else{
	  initState.parsePlayerState(0, s,this);
	}
	break;
      case 2:
	if (s.compare("[do]") == 0){
	  state++;
	}else{
	  initState.parsePlayerState(1, s,this);
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
	  endState.phase = phaseStrToInt(s);
	}
	break;
      case 5:
	if (s.compare("[player2]") == 0){
	  state++;
	}else{
	  endState.parsePlayerState(0, s,this);
	}
	break;
      case 6:
	if (s.compare("[end]") == 0){
	  state++;
	}else{
	  endState.parsePlayerState(1, s,this);
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

