#include "../include/config.h"
#include "../include/GameStateDuel.h"
#include "../include/GameOptions.h"
#include "../include/utils.h"
#include "../include/AIPlayer.h"
#include "../include/AIMomirPlayer.h"
#include "../include/PlayerData.h"
#include "../include/DeckStats.h"
#include "../include/MTGRules.h"
#include "../include/Credits.h"
#include "../include/Translate.h"
#include "../include/Rules.h"

#ifdef TESTSUITE
#include "../include/TestSuiteAI.h"
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
  testSuite = NEW TestSuite(RESPATH"/test/_tests.txt",mParent->collection);
#endif

  mGamePhase = DUEL_STATE_CHOOSE_DECK1;
  credits = NEW Credits();

  menu = NEW SimpleMenu(DUEL_MENU_GAME_MENU, this, Constants::MENU_FONT, SCREEN_WIDTH/2-100, 25);
  menu->Add(12,"Back to main menu");
  menu->Add(13, "Cancel");

  int decksneeded = 0;


  for (int i = 0; i<2; i ++){
    if (mParent->players[i] ==  PLAYER_TYPE_HUMAN){
      decksneeded = 1;
      deckmenu = NEW SimpleMenu(DUEL_MENU_CHOOSE_DECK, this, Constants::MENU_FONT, 35, 25, "Choose a Deck");
      int nbDecks = fillDeckMenu(deckmenu,options.profileFile());
      if (nbDecks) decksneeded = 0;
      break;
    }
  }

  if (decksneeded){
    deckmenu->Add(-1,_("Create your Deck!").c_str(),"Highly recommended to get\nthe full Wagic experience!");
    premadeDeck = true;
    fillDeckMenu(deckmenu,RESPATH"/player/premade");
  }
  
  for (int i = 0; i < 2; ++i){
    mPlayers[i] = NULL;
  }
    
}

void GameStateDuel::loadPlayer(int playerId, int decknb, int isAI){
  if (decknb){
    if (!isAI){ //Human Player
      char deckFile[255];
      if(premadeDeck)
        sprintf(deckFile, RESPATH"/player/premade/deck%i.txt",decknb);
      else
        sprintf(deckFile, "%s/deck%i.txt",options.profileFile().c_str(), decknb);
      char deckFileSmall[255];
      sprintf(deckFileSmall, "player_deck%i",decknb);
      MTGDeck * tempDeck = NEW MTGDeck(deckFile, mParent->collection);
      deck[playerId] = NEW MTGPlayerCards(tempDeck);
      delete tempDeck;
      mPlayers[playerId] = NEW HumanPlayer(deck[playerId],deckFile, deckFileSmall);
    }else{ //AI Player, chose deck
          AIPlayerFactory playerCreator;
          Player * opponent = NULL;
          if (playerId == 1) opponent = mPlayers[0];
          mPlayers[playerId] = playerCreator.createAIPlayer(mParent->collection,opponent,decknb);
          deck[playerId] = mPlayers[playerId]->game;
    }
  }else{ //Random AI deck
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
#if defined (WIN32) || defined (LINUX)
  OutputDebugString("Ending GamestateDuel\n");
#endif
  SAFE_DELETE(deckmenu);
  //stop game music
  if (GameApp::music){
	      JSoundSystem::GetInstance()->StopMusic(GameApp::music);
	      SAFE_DELETE(GameApp::music);
      }
  JRenderer::GetInstance()->EnableVSync(false);
  if (mPlayers[0] && mPlayers[1]) mPlayers[0]->End();
  GameObserver::EndInstance();
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
#ifdef TESTSUITE
  SAFE_DELETE(testSuite);
#endif
}


//TODO Move This to utils or ResourceManager. Don't we have more generic functions that can do that?
bool GameStateDuel::MusicExist(string FileName){
  string filepath = RESPATH;
  filepath = filepath + "/" + resources.musicFile(FileName);
  std::ifstream file(filepath.c_str());
  if (file) {
    file.close();
    return true;
  }
  else
    return false;
}

void GameStateDuel::Update(float dt)
{
  switch (mGamePhase)
    {
    case DUEL_STATE_ERROR_NO_DECK:
      if (PSP_CTRL_CIRCLE == mEngine->ReadButton())
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
	        testSuite->initGame();
	        char buf[4096];
	        sprintf(buf, "nb cards in player2's graveyard : %i\n",mPlayers[1]->game->graveyard->nb_cards);
	        LOG(buf);
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
	  if (!opponentMenu){
	    opponentMenu = NEW SimpleMenu(DUEL_MENU_CHOOSE_OPPONENT, this, Constants::MENU_FONT, 35, 25, "Choose Opponent");
	    opponentMenu->Add(0,"Random");
	    if (options[Options::EVILTWIN_MODE_UNLOCKED].number)
	      opponentMenu->Add(-1,"Evil Twin", "Can you play against yourself?");
	    fillDeckMenu(opponentMenu,RESPATH"/ai/baka", "ai_baka", mPlayers[0]);
	  }
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
      else
	{
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
        //stop menu music
        if (GameApp::music){
          JSoundSystem::GetInstance()->StopMusic(GameApp::music);
          SAFE_DELETE(GameApp::music);
        }
        //start of in game music code
        if (GameApp::HasMusic && options[Options::MUSICVOLUME].number > 0){
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

	        if (MusicExist(musictrack)){
	          GameApp::music = resources.ssLoadMusic(musictrack.c_str());
            JSoundSystem::GetInstance()->PlayMusic(GameApp::music, true);
          }
        }
        //end of music code
      }
      //      mParent->effect->UpdateSmall(dt);
      game->Update(dt);
      if (game->gameOver){
        credits->compute(game->players[0],game->players[1], mParent);
	      mGamePhase = DUEL_STATE_END;
#ifdef TESTSUITE
	if (mParent->players[1] == PLAYER_TYPE_TESTSUITE){
	  if (testSuite->loadNext()){
	    loadTestSuitePlayers();
	    mGamePhase = DUEL_STATE_PLAY;
	    testSuite->initGame();
	  }else{
	    mGamePhase = DUEL_STATE_END;
	  }
	}else
#endif
	  if (mParent->players[0] == PLAYER_TYPE_CPU && mParent->players[1] == PLAYER_TYPE_CPU){
	    End();
	    Start();
	  }
      }
      if (mEngine->GetButtonClick(PSP_CTRL_START)){
	mGamePhase = DUEL_STATE_MENU;
      }
      break;
    case DUEL_STATE_MENU:
      //      mParent->effect->UpdateSmall(dt);
      menu->Update(dt);
      break;
    case DUEL_STATE_CANCEL:
      //      mParent->effect->UpdateSmall(dt);
      menu->Update(dt);
      if (menu->closed)
	mGamePhase = DUEL_STATE_PLAY;
      break;
    case DUEL_STATE_BACK_TO_MAIN_MENU:
      //       mParent->effect->UpdateSmall(dt);
     menu->Update(dt);
      if (menu->closed) {
        PlayerData * playerdata = NEW PlayerData(mParent->collection);
        playerdata->taskList->passOneDay();
        playerdata->taskList->save();
        SAFE_DELETE(playerdata);
        mParent->SetNextState(GAME_STATE_MENU);
      }
      break;
    default:
      if (PSP_CTRL_CIRCLE == mEngine->ReadButton()){
	mParent->SetNextState(GAME_STATE_MENU);
      }
    }
}


void GameStateDuel::Render()
{
  JLBFont * mFont = resources.GetJLBFont(Constants::MAIN_FONT);
  //Erase
  LOG("Start Render\n");
  JRenderer::GetInstance()->ClearScreen(ARGB(0,0,0,0));

  if (game)
    game->Render();
  switch (mGamePhase)
    {
    case DUEL_STATE_END:
      {
        JRenderer * r = JRenderer::GetInstance();
	      r->ClearScreen(ARGB(200,0,0,0));
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
        JRenderer * r = JRenderer::GetInstance();
	      r->ClearScreen(ARGB(200,0,0,0));
	      mFont->DrawString(_("AN ERROR OCCURRED, CHECK FILE NAMES").c_str(),0,SCREEN_HEIGHT/2);
	      break;
      }
    case DUEL_STATE_CHOOSE_DECK1:
    case DUEL_STATE_CHOOSE_DECK1_TO_2:
    case DUEL_STATE_CHOOSE_DECK2:
    case DUEL_STATE_CHOOSE_DECK2_TO_PLAY:
      if (mParent->gameType != GAME_TYPE_CLASSIC){
        mFont->DrawString(_("LOADING DECKS").c_str(),0,SCREEN_HEIGHT/2);
      }else{
        if (opponentMenu){
	        opponentMenu->Render();
        }else if (deckmenu){
	        deckmenu->Render();
        }
      }
      break;
    case DUEL_STATE_ERROR_NO_DECK:
      mFont->DrawString(_("NO DECK AVAILABLE,").c_str(),0,SCREEN_HEIGHT/2);
      mFont->DrawString(_("PRESS CIRCLE TO GO TO THE DECK EDITOR!").c_str(),0,SCREEN_HEIGHT/2 + 20);
      break;
    case DUEL_STATE_MENU:
    case DUEL_STATE_CANCEL:
    case DUEL_STATE_BACK_TO_MAIN_MENU:
      menu->Render();
  }
  LOG("End Render\n");
}

void GameStateDuel::ButtonPressed(int controllerId, int controlId)
{
  switch (controllerId){
    case DUEL_MENU_CHOOSE_OPPONENT:
      {
        switch(controlId){
          case 0:
            loadPlayer(1);
	          opponentMenu->Close();
	          mGamePhase = DUEL_STATE_CHOOSE_DECK2_TO_PLAY;
            break;
          default:
            loadPlayer(1,controlId,1);
			OpponentsDeckid=controlId;
	          opponentMenu->Close();
	          mGamePhase = DUEL_STATE_CHOOSE_DECK2_TO_PLAY;
            break;

        }
        break;
      }
    case DUEL_MENU_CHOOSE_DECK:
      {
        if (controlId < 0){
          mParent->SetNextState(GAME_STATE_DECK_VIEWER);
          return;
        }
        if (mGamePhase == DUEL_STATE_CHOOSE_DECK1){
          loadPlayer(0,controlId);
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

          case 12:
	    menu->Close();
	    mGamePhase = DUEL_STATE_BACK_TO_MAIN_MENU;
            break;
          case 13:
	    menu->Close();
            mGamePhase = DUEL_STATE_CANCEL;
            break;
        }
      }
  }
}
