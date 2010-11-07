#include "PrecompiledHeader.h"

#include "DeckMenu.h"
#include "GameStateDuel.h"
#include "GameOptions.h"
#include "utils.h"
#include "AIPlayer.h"
#include "AIMomirPlayer.h"
#include "PlayerData.h"
#include "DeckStats.h"
#include "DeckManager.h"

#include "DeckMetaData.h"
#include "MTGRules.h"
#include "Credits.h"
#include "Translate.h"
#include "Rules.h"

#ifdef TESTSUITE
#include "TestSuiteAI.h"
#endif

#if defined (WIN32) || defined (LINUX)
#include <time.h>
#endif
  
enum ENUM_DUEL_STATE
  {
    DUEL_STATE_START,
    DUEL_STATE_END,
    DUEL_STATE_CHOOSE_DECK1,
    DUEL_STATE_CHOOSE_DECK1_TO_2,
    DUEL_STATE_CHOOSE_DECK2,
    DUEL_STATE_CHOOSE_DECK2_TO_PLAY,
    DUEL_STATE_ERROR_NO_DECK,
    DUEL_STATE_CANCEL,
    DUEL_STATE_PLAY,
    DUEL_STATE_BACK_TO_MAIN_MENU,
    DUEL_STATE_MENU,
    DUEL_STATE_ERROR
  };

enum ENUM_DUEL_MENUS
  {
    DUEL_MENU_GAME_MENU,
    DUEL_MENU_CHOOSE_DECK,
    DUEL_MENU_CHOOSE_OPPONENT
  };


GameStateDuel::GameStateDuel(GameApp* parent): GameState(parent) {
	for (int i = 0; i<2; i ++){
    deck[i]=NULL;
    mPlayers[i]=NULL;
  }
  premadeDeck = false;
  game = NULL;
  deckmenu = NULL;
  opponentMenu = NULL;
  menu = NULL;
#ifdef TESTSUITE
  testSuite = NULL;
#endif

  credits = NULL;
  rules = NULL;

}

GameStateDuel::~GameStateDuel() {
  End();

}

void GameStateDuel::Start()
{
  JRenderer * renderer = JRenderer::GetInstance();
  renderer->EnableVSync(true);
  OpponentsDeckid=0;


#ifdef TESTSUITE
  SAFE_DELETE(testSuite);
  testSuite = NEW TestSuite(JGE_GET_RES("test/_tests.txt").c_str(),mParent->collection);
#endif

  mGamePhase = DUEL_STATE_CHOOSE_DECK1;
  credits = NEW Credits();

  menu = NULL;

  int decksneeded = 0;
  for (int i = 0; i<2; i ++){
    if (mParent->players[i] ==  PLAYER_TYPE_HUMAN){
      decksneeded = 1;
      
      deckmenu = NEW DeckMenu(DUEL_MENU_CHOOSE_DECK, this, Fonts::MAGIC_FONT, "Choose a Deck");

      DeckManager *deckManager = DeckManager::GetInstance();
      vector<DeckMetaData *> playerDeckList = getValidDeckMetaData( options.profileFile() );
      int nbDecks = playerDeckList.size();

      if (nbDecks) 
      {
        decksneeded = 0;
				if (nbDecks > 1 )
					deckmenu->Add( MENUITEM_RANDOM_PLAYER, "Random", "Play with a random deck." );
      }

      renderDeckMenu( deckmenu, playerDeckList );
      // save the changes to the player deck list maintained in DeckManager
      deckManager->updateMetaDataList( &playerDeckList, false);
      playerDeckList.clear();

      break;
    }
  }

  if(deckmenu){
    if (decksneeded){
        //translate deck creating desc
        Translator * t = Translator::GetInstance();
        map<string,string>::iterator it = t->deckValues.find("Create your Deck!");
        if (it != t->deckValues.end())
          deckmenu->Add( MENUITEM_NEW_DECK, "Create your Deck!", it->second);
        else
          deckmenu->Add( MENUITEM_NEW_DECK, "Create your Deck!", "Highly recommended to get\nthe full Wagic experience!");
        premadeDeck = true;
        fillDeckMenu(deckmenu,JGE_GET_RES("player/premade"));
    }
    deckmenu->Add( MENUITEM_NEW_DECK, "New Deck...", "Create a new deck to play with.");
    deckmenu->Add( MENUITEM_CANCEL, "Main Menu", "Return to Main Menu");
  } 
  
  for (int i = 0; i < 2; ++i){
    mPlayers[i] = NULL;
  }
    
}

void GameStateDuel::loadPlayer(int playerId, int decknb, int isAI){
  if (decknb) {
    if (!isAI) { //Human Player
      char deckFile[255];
      if(premadeDeck)
        sprintf(deckFile, JGE_GET_RES("player/premade/deck%i.txt").c_str(),decknb);
      else
        sprintf(deckFile, "%s/deck%i.txt",options.profileFile().c_str(), decknb);
      char deckFileSmall[255];
      sprintf(deckFileSmall, "player_deck%i",decknb);
      MTGDeck * tempDeck = NEW MTGDeck(deckFile, mParent->collection);
      mPlayers[playerId] = NEW HumanPlayer(tempDeck, deckFile, deckFileSmall);

      deck[playerId] = mPlayers[playerId]->game;
      delete tempDeck;
    } 
    else { //AI Player, chooses deck
          AIPlayerFactory playerCreator;
          Player * opponent = NULL;
          if (playerId == 1) opponent = mPlayers[0];
          mPlayers[playerId] = playerCreator.createAIPlayer(mParent->collection,opponent,decknb);
          deck[playerId] = mPlayers[playerId]->game;
    }
  }
  else { //Random AI deck
    AIPlayerFactory playerCreator;
    Player * opponent = NULL;
    if (playerId == 1) opponent = mPlayers[0];
    mPlayers[playerId] = playerCreator.createAIPlayer(mParent->collection,opponent);
    deck[playerId] = mPlayers[playerId]->game;
  }
}

void GameStateDuel::initRand(unsigned int seed){
  if (!seed) seed = time(0);
  srand(seed);
}

#ifdef TESTSUITE
void GameStateDuel::loadTestSuitePlayers(){
  if (!testSuite) return;
  initRand(testSuite->seed);
  SAFE_DELETE(game);
  for (int i = 0; i < 2; i++){
    mPlayers[i] = NEW TestSuiteAI(testSuite, i);
    deck[i] = mPlayers[i]->game;
  }
  mParent->gameType = testSuite->gameType;

  GameObserver::Init(mPlayers, 2);
  game = GameObserver::GetInstance();
  game->startGame(rules);
  if (mParent->gameType == GAME_TYPE_MOMIR){
    game->addObserver(NEW MTGMomirRule(-1, mParent->collection));
  }
}
#endif

void GameStateDuel::End()
{
  DebugTrace("Ending GameStateDuel");

  JRenderer::GetInstance()->EnableVSync(false);
  if (mPlayers[0] && mPlayers[1]) mPlayers[0]->End();
  GameObserver::EndInstance();
  DeckManager::EndInstance();
  game = NULL;
  premadeDeck = false;

  for (int i = 0; i < 2; i++){
    mPlayers[i] = NULL;
    deck[i] = NULL;
  }

  SAFE_DELETE(credits);
  SAFE_DELETE(rules);

  SAFE_DELETE(menu);
  SAFE_DELETE(opponentMenu);
  SAFE_DELETE(deckmenu);
#ifdef TESTSUITE
  SAFE_DELETE(testSuite);
#endif
}


//TODO Move This to utils or ResourceManager. Don't we have more generic functions that can do that?
bool GameStateDuel::MusicExist(string FileName){
  string filepath = JGE_GET_RES(resources.musicFile(FileName));
  std::ifstream file(filepath.c_str());
  if (file) {
    file.close();
    return true;
  }
  else
    return false;
}

void GameStateDuel::ensureOpponentMenu(){
  if (!opponentMenu){
    opponentMenu = NEW DeckMenu(DUEL_MENU_CHOOSE_OPPONENT, this, Fonts::MAGIC_FONT, "Choose Your Opponent");
    opponentMenu->Add( MENUITEM_RANDOM_AI, "Random");
    if (options[Options::EVILTWIN_MODE_UNLOCKED].number)
      opponentMenu->Add( MENUITEM_EVIL_TWIN, "Evil Twin", _("Can you play against yourself?").c_str());
    DeckManager * deckManager = DeckManager::GetInstance();
    vector<DeckMetaData* > opponentDeckList = fillDeckMenu( opponentMenu, JGE_GET_RES("ai/baka"), "ai_baka", mPlayers[0]);
    deckManager->updateMetaDataList(&opponentDeckList, true);
    opponentMenu->Add( MENUITEM_CANCEL, "Cancel", _("Choose a different player deck").c_str());
    opponentDeckList.clear();
  }
}

void GameStateDuel::Update(float dt)
{
  switch (mGamePhase)
    {
    case DUEL_STATE_ERROR_NO_DECK:
      if (JGE_BTN_OK == mEngine->ReadButton())
	mParent->SetNextState(GAME_STATE_DECK_VIEWER);
      break;
    case DUEL_STATE_CHOOSE_DECK1:
      if (mParent->gameType == GAME_TYPE_MOMIR){
        rules = NEW Rules("momir.txt");
        mGamePhase = DUEL_STATE_PLAY;
      } else if (mParent->gameType == GAME_TYPE_RANDOM1){
        rules = NEW Rules ("random1.txt");
        mGamePhase = DUEL_STATE_PLAY;
      }else if (mParent->gameType == GAME_TYPE_RANDOM2) {
        rules = NEW Rules ("random2.txt");
        mGamePhase = DUEL_STATE_PLAY;
      }
#ifdef TESTSUITE
      else if (mParent->players[1] ==  PLAYER_TYPE_TESTSUITE){
	      if (testSuite && testSuite->loadNext()){
          rules = NEW Rules("testsuite.txt");
	        loadTestSuitePlayers();
	        mGamePhase = DUEL_STATE_PLAY;
          testSuite->pregameTests();
	        testSuite->initGame();
	      }else{
          if (!game){
            mGamePhase = DUEL_STATE_ERROR;
          }else{
	          mGamePhase = DUEL_STATE_END;
          }
	      }
      }
#endif
      else{
        if (!rules) rules = NEW Rules("mtg.txt");
        if (mParent->players[0] ==  PLAYER_TYPE_HUMAN)
	        deckmenu->Update(dt);
        else{
	        loadPlayer(0);
	        mGamePhase = DUEL_STATE_CHOOSE_DECK2;
        }
      }
      break;
    case DUEL_STATE_CHOOSE_DECK1_TO_2:
      if (deckmenu->closed) mGamePhase = DUEL_STATE_CHOOSE_DECK2;
      else deckmenu->Update(dt);
      break;
    case DUEL_STATE_CHOOSE_DECK2:
      if (mParent->players[1] ==  PLAYER_TYPE_HUMAN)
        deckmenu->Update(dt);
      else{
        if (mParent->players[0] ==  PLAYER_TYPE_HUMAN){
          ensureOpponentMenu();
          opponentMenu->Update(dt);
        }else{
          loadPlayer(1);
          mGamePhase = DUEL_STATE_PLAY;
        }
      }
      break;
    case DUEL_STATE_CHOOSE_DECK2_TO_PLAY:
      if (mParent->players[1] ==  PLAYER_TYPE_HUMAN){
	      if (deckmenu->closed) mGamePhase = DUEL_STATE_PLAY;
	      else deckmenu->Update(dt);
      }
      else{
          ensureOpponentMenu();
          if (opponentMenu->closed) mGamePhase = DUEL_STATE_PLAY;
          else opponentMenu->Update(dt);
	    }
      break;
    case DUEL_STATE_PLAY:
      if (!game){
	      GameObserver::Init(mPlayers, 2);
	      game = GameObserver::GetInstance();
	      game->startGame(rules);
        if (mParent->gameType == GAME_TYPE_MOMIR){
          game->addObserver(NEW MTGMomirRule(-1, mParent->collection));
        }

        //start of in game music code
	         musictrack = "";
	         //check opponent id and choose the music track based on it
	         if(OpponentsDeckid) {
	          char temp[4096];
	          sprintf(temp,"ai_baka_music%i.mp3",OpponentsDeckid);
	          musictrack.assign(temp);
	         }
	         else if(mParent->gameType == GAME_TYPE_CLASSIC)
	          musictrack = "ai_baka_music.mp3";
	         else if(mParent->gameType == GAME_TYPE_MOMIR)
	          musictrack = "ai_baka_music_momir.mp3";
	         else if(mParent->gameType == GAME_TYPE_RANDOM1 || mParent->gameType == GAME_TYPE_RANDOM2)
	          musictrack = "ai_baka_music_random.mp3";

          if(!MusicExist(musictrack))
	          musictrack = "ai_baka_music.mp3";

          GameApp::playMusic(musictrack);
      }
      game->Update(dt);
      if (game->gameOver){
        if (game->players[1]->playMode != Player::MODE_TEST_SUITE)
          credits->compute(game->players[0],game->players[1], mParent);
        mGamePhase = DUEL_STATE_END;
#ifdef TESTSUITE
        if (mParent->players[1] == PLAYER_TYPE_TESTSUITE){
          if (testSuite->loadNext()){
            loadTestSuitePlayers();
            mGamePhase = DUEL_STATE_PLAY;
            testSuite->initGame();
          }
          else
            mGamePhase = DUEL_STATE_END;
        }
        else
#endif
          if (mParent->players[0] == PLAYER_TYPE_CPU && mParent->players[1] == PLAYER_TYPE_CPU){
            End();
            Start();
          }
      }
      if (mEngine->GetButtonClick(JGE_BTN_MENU)) {
        if (!menu) {
          menu = NEW SimpleMenu(DUEL_MENU_GAME_MENU, this, Fonts::MENU_FONT, SCREEN_WIDTH/2-100, 25, game->players[1]->deckName.c_str());
          int cardsinhand = game->players[0]->game->hand->nb_cards;
    		  
          //almosthumane - mulligan
          if ((game->turn < 1) && (cardsinhand != 0) 
	          && game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN
	          && game->players[0]->game->inPlay->nb_cards == 0
	          && game->players[0]->game->graveyard->nb_cards == 0
	          && game->players[0]->game->exile->nb_cards == 0) //1st Play Check 
	          //IF there was no play at the moment automatically mulligan
          {
            menu->Add( MENUITEM_MULLIGAN, "Mulligan");
          }
          //END almosthumane - mulligan
          menu->Add(MENUITEM_MAIN_MENU, "Back to main menu");
          menu->Add(MENUITEM_CANCEL, "Cancel");
        }
        mGamePhase = DUEL_STATE_MENU;
      }
      break;
    case DUEL_STATE_MENU:
      menu->Update(dt);
      break;
    case DUEL_STATE_CANCEL:
      menu->Update(dt);
      if (menu->closed) {
	      mGamePhase = DUEL_STATE_PLAY;
        SAFE_DELETE(menu);
      }
      break;
    case DUEL_STATE_BACK_TO_MAIN_MENU:
      if(menu){
        menu->Update(dt);
        if (menu->closed) {
          PlayerData * playerdata = NEW PlayerData(mParent->collection);
          playerdata->taskList->passOneDay();
          playerdata->taskList->save();
          SAFE_DELETE(playerdata);
          SAFE_DELETE(menu);
        }
      }
      mParent->DoTransition(TRANSITION_FADE,GAME_STATE_MENU);
      
      break;
    default:
      if (JGE_BTN_OK == mEngine->ReadButton())
	mParent->SetNextState(GAME_STATE_MENU);
    }
}


void GameStateDuel::Render()
{
  WFont * mFont = resources.GetWFont(Fonts::MAIN_FONT);
  JRenderer * r = JRenderer::GetInstance();
  r->ClearScreen(ARGB(0,0,0,0));

  if (game)
    game->Render();
    
  switch (mGamePhase)
    {
    case DUEL_STATE_END:
      {
        JRenderer * r = JRenderer::GetInstance();
        r->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(200,0,0,0));
	      credits->Render();
#ifdef TESTSUITE
	      if (mParent->players[1] == PLAYER_TYPE_TESTSUITE){
          r->ClearScreen(ARGB(255,0,0,0));
          char buf[4096];
          int nbFailed = testSuite->nbFailed;
          int nbTests = testSuite->nbTests;
          if (!nbFailed){
            sprintf(buf, "All %i tests successful!", nbTests);
          }else{
            sprintf(buf, "%i tests out of %i FAILED!", nbFailed, nbTests);
          }
          mFont->DrawString(buf,0,SCREEN_HEIGHT/2);
          nbFailed = testSuite->nbAIFailed;
          nbTests = testSuite->nbAITests;
          if (nbTests){
            if (!nbFailed){
              sprintf(buf, "AI Tests: All %i tests successful!", nbTests);
            }else{
              sprintf(buf, "AI Tests: %i tests out of %i FAILED!", nbFailed, nbTests);
            }
            mFont->DrawString(buf,0,SCREEN_HEIGHT/2+20);
          }
        }
#endif
	      break;
      }
    case DUEL_STATE_ERROR:
      {
	      r->ClearScreen(ARGB(200,0,0,0));
	      mFont->DrawString(_("AN ERROR OCCURRED, CHECK FILE NAMES").c_str(),0,SCREEN_HEIGHT/2);
	      break;
      }
    case DUEL_STATE_CHOOSE_DECK1:
    case DUEL_STATE_CHOOSE_DECK1_TO_2:
    case DUEL_STATE_CHOOSE_DECK2:
    case DUEL_STATE_CHOOSE_DECK2_TO_PLAY:
      if (mParent->gameType != GAME_TYPE_CLASSIC)
        mFont->DrawString(_("LOADING DECKS").c_str(),0,SCREEN_HEIGHT/2);
      else{
        if (opponentMenu)
	        opponentMenu->Render();
        else if (deckmenu)
	        deckmenu->Render();

      }
      break;
    case DUEL_STATE_ERROR_NO_DECK:
      mFont->DrawString(_("NO DECK AVAILABLE,").c_str(),0,SCREEN_HEIGHT/2);
      mFont->DrawString(_("PRESS CIRCLE TO GO TO THE DECK EDITOR!").c_str(),0,SCREEN_HEIGHT/2 + 20);
      break;
    case DUEL_STATE_MENU:
    case DUEL_STATE_CANCEL:
    case DUEL_STATE_BACK_TO_MAIN_MENU:
      if (game) {
        r->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(100,0,0,0));
        char buffer[4096];
        sprintf(buffer,_("Turn:%i").c_str(),game->turn);
        mFont->SetColor(ARGB(255,255,255,255));
        mFont->DrawString(buffer,SCREEN_WIDTH/2,0,JGETEXT_CENTER);
      }
      if(menu)
        menu->Render();
  }
}

void GameStateDuel::ButtonPressed(int controllerId, int controlId) {
  int deckNumber = controlId;
  DeckManager * deckManager = DeckManager::GetInstance();
  int aiDeckSize =  deckManager->getAIDeckOrderList()->size();
  switch (controllerId){
    case DUEL_MENU_CHOOSE_OPPONENT:
      {
        switch(controlId){
          case MENUITEM_RANDOM_AI:
              loadPlayer(1);
	          opponentMenu->Close();
	          mGamePhase = DUEL_STATE_CHOOSE_DECK2_TO_PLAY;
            break;
          default:
            // cancel option.  return to player deck selection

            if (controlId == MENUITEM_CANCEL) 
            {
                opponentMenu->Close();
                deckmenu->Close();
                mParent->SetNextState( DUEL_STATE_CHOOSE_DECK1 );
                mGamePhase = DUEL_MENU_GAME_MENU;
                break;
            }
            else if ( controlId != MENUITEM_EVIL_TWIN && aiDeckSize > 0) // evil twin
              deckNumber = deckManager->getAIDeckOrderList()->at( controlId - 1 )->getDeckId();

            loadPlayer(1,deckNumber,1);
			      OpponentsDeckid=deckNumber;
	          opponentMenu->Close();
	          mGamePhase = DUEL_STATE_CHOOSE_DECK2_TO_PLAY;
            break;
        }
        break;
      }
    case DUEL_MENU_CHOOSE_DECK:
      {
        if ( controlId == MENUITEM_RANDOM_PLAYER ) // Random Player Deck Selection
        {
          vector<DeckMetaData *> * playerDeckList = deckManager->getPlayerDeckOrderList();
          deckNumber = playerDeckList->at(WRand() * 1001 % (playerDeckList->size()) )->getDeckId();
          loadPlayer( 0, deckNumber );
          deckmenu->Close();
          mGamePhase = DUEL_STATE_CHOOSE_DECK2_TO_PLAY;
          break;
        }
        else if (controlId == MENUITEM_MAIN_MENU || controlId == MENUITEM_CANCEL ) // user clicked on "Cancel"
        {            
            if (deckmenu) 
                deckmenu->Close();
            mGamePhase = DUEL_STATE_BACK_TO_MAIN_MENU;
            break;
        }
        if (controlId < 0){
          mParent->SetNextState(GAME_STATE_DECK_VIEWER);
          return;
        }
        if (mGamePhase == DUEL_STATE_CHOOSE_DECK1){
          vector<DeckMetaData *> * playerDeck = deckManager->getPlayerDeckOrderList();
          if ( !premadeDeck && controlId > 0 )
            deckNumber = playerDeck->at( controlId - 1 )->getDeckId();
          loadPlayer(0,deckNumber);
          deckmenu->Close();
          mGamePhase = DUEL_STATE_CHOOSE_DECK1_TO_2;
        }else{
          loadPlayer(1,controlId);
          deckmenu->Close();
          mGamePhase = DUEL_STATE_CHOOSE_DECK2_TO_PLAY;
        }
        break;
      }
    default:
      {
      switch (controlId)
        {

          case MENUITEM_MAIN_MENU:
	          menu->Close();
	          mGamePhase = DUEL_STATE_BACK_TO_MAIN_MENU;
            break;
          case MENUITEM_CANCEL:
	          menu->Close();
            mGamePhase = DUEL_STATE_CANCEL;
            break;
          case MENUITEM_MULLIGAN:
          //almosthumane - mulligan
          {

		        int cardsinhand = game->players[0]->game->hand->nb_cards;

		        for (int i = 0 ; i < cardsinhand; i ++) //Discard hand
		          game->currentPlayer->game->putInZone(game->currentPlayer->game->hand->cards[0],game->currentPlayer->game->hand ,game->currentPlayer->game->library);
      		  
	            game->currentPlayer->game->library->shuffle(); //Shuffle

	            for (int i = 0; i < (cardsinhand-1); i ++)	game->draw(); //Draw hand with 1 less card penalty //almhum	  
            menu->Close();
            mGamePhase = DUEL_STATE_CANCEL;
            break;
          }
          //END almosthumane - mulligan
      }
    }
  }
}
