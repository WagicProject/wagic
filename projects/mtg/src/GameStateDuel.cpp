#include "../include/debug.h"
#include "../include/GameStateDuel.h"
#include "../include/utils.h"
#include "../include/AIPlayer.h"
#include "../include/PlayerData.h"

#ifdef TESTSUITE
  #include "../include/TestSuiteAI.h"
#endif

GameStateDuel::GameStateDuel(GameApp* parent): GameState(parent) {
	for (int i = 0; i<2; i ++){
		deck[i]=NULL;
		mPlayers[i]=NULL;
	}

	game = NULL;
	deckmenu = NULL;
	menu = NULL;
#ifdef TESTSUITE
	testSuite = NULL;
#endif
}

GameStateDuel::~GameStateDuel() {
	End();
}

void GameStateDuel::Start()
{
	JRenderer * renderer = JRenderer::GetInstance();
  renderer->ResetPrivateVRAM();
  renderer->EnableVSync(true);


#ifdef TESTSUITE
if (testSuite) delete testSuite;
testSuite = NEW TestSuite("Res/test/_tests.txt");
#endif


  mGamePhase = DUEL_CHOOSE_DECK1;

	mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  mFont->SetBase(0);	// using 2nd font


	menu = NEW SimpleMenu(11,this,mFont,SCREEN_WIDTH/2-100,20,200);
	menu->Add(12,"Back to main menu");
	menu->Add(13, "Cancel");

	int decksneeded = 0;


	for (int i = 0; i<2; i ++){
		if (mParent->players[i] ==  PLAYER_TYPE_HUMAN){
			if (!deckmenu){
				decksneeded = 1;
				deckmenu = NEW SimpleMenu(1,this,mFont, 10 , 10, 100, "Choose a Deck");
				char buffer[100];
				for (int j=1; j<6; j++){
					sprintf(buffer, "Res/player/deck%i.txt",j);
					std::ifstream file(buffer);
					if(file){
						deckmenu->Add(j, GameState::menuTexts[j]);
						file.close();
						decksneeded = 0; 
					}
				}
			}
		}
	}

	if (decksneeded)
		mGamePhase = ERROR_NO_DECK;



}


void GameStateDuel::loadPlayer(int playerId, int decknb){
	if (decknb){ //Human Player
		char deckFile[255];
 		sprintf(deckFile, "Res/player/deck%i.txt",decknb);
		char deckFileSmall[255];
 		sprintf(deckFileSmall, "player_deck%i",decknb);
		int deck_cards_ids[100];
		int nb_elements = readfile_to_ints(deckFile, deck_cards_ids);
		deck[playerId] = NEW MTGPlayerCards(mParent->collection,deck_cards_ids, nb_elements);
		mPlayers[playerId] = NEW HumanPlayer(deck[playerId],deckFileSmall);
	}else{
			AIPlayerFactory playerCreator;
			mPlayers[playerId] = playerCreator.createAIPlayer(mParent->collection,NULL);
			deck[playerId] = mPlayers[playerId]->game;
	}
}

#ifdef TESTSUITE
void GameStateDuel::loadTestSuitePlayers(){
OutputDebugString ("loading suite 1\n"); 
	if (!testSuite) return;
	for (int i = 0; i < 2; i++){
		if (mPlayers[i]){
			delete mPlayers[i];
		}
		mPlayers[i] = NEW TestSuiteAI(mParent->collection,testSuite, i);
OutputDebugString ("loading suite 2\n");
		deck[i] = mPlayers[i]->game;
	}

	if (game) delete game;
	game = NULL;
	if (!game){
			GameObserver::Init(mPlayers, 2);
OutputDebugString ("loading suite 3\n");
			game = GameObserver::GetInstance();
OutputDebugString ("loading suite 4\n");
			game->startGame(0,0);
OutputDebugString ("loading suite 5\n");
	}
}
#endif

void GameStateDuel::End()
{
#if defined (WIN32) || defined (LINUX)
	OutputDebugString("Ending GamestateDuel\n");
#endif
	GameObserver::EndInstance();
	game = NULL;
	SAFE_DELETE(deckmenu);
	JRenderer::GetInstance()->EnableVSync(false);
	for (int i = 0; i < 2; i++){
		SAFE_DELETE(mPlayers[i]);
		SAFE_DELETE(deck[i]);
	}
	SAFE_DELETE(menu);
#ifdef TESTSUITE
	SAFE_DELETE(testSuite);
#endif
}


void GameStateDuel::Update(float dt)
{
	if (mGamePhase == ERROR_NO_DECK){
		if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)){
			mParent->SetNextState(GAME_STATE_DECK_VIEWER);
		}
	}else if (mGamePhase == DUEL_CHOOSE_DECK1){
		if (mParent->players[0] ==  PLAYER_TYPE_HUMAN){
			deckmenu->Update(dt);	
		}
#ifdef TESTSUITE
		else if (mParent->players[1] ==  PLAYER_TYPE_TESTSUITE){
			if (testSuite && testSuite->loadNext()){
				loadTestSuitePlayers();
				mGamePhase = DUEL_PLAY;
				testSuite->initGame();
				char buf[4096];
				sprintf(buf, "nb cards in player2's graveyard : %i\n",mPlayers[1]->game->graveyard->nb_cards);
				LOG(buf);
			}else{
				mGamePhase = DUEL_END;
			}
		}
#endif		
		else{
			loadPlayer(0);
			mGamePhase = DUEL_CHOOSE_DECK2;
		}
	}else if(mGamePhase == DUEL_CHOOSE_DECK2){
		if (mParent->players[1] ==  PLAYER_TYPE_HUMAN){
			deckmenu->Update(dt);	
		}

		else{
			loadPlayer(1);
			mGamePhase = DUEL_PLAY;
		}

	}else if (mGamePhase == DUEL_PLAY){
		if (!game){
			GameObserver::Init(mPlayers, 2);
			game = GameObserver::GetInstance();
			game->startGame();
		}
		game->Update(dt);
		if (game->gameOver){
			if (!mPlayers[0]->isAI() && mPlayers[1]->isAI() && mPlayers[0]!= game->gameOver){
#if defined (WIN32) || defined (LINUX)
				char buf[4096];
				sprintf(buf, "%p - %p", mPlayers[0], game->gameOver);
				OutputDebugString(buf);
#endif
				PlayerData * playerdata = NEW PlayerData(mParent->collection);
				playerdata->credits+= 500;
				playerdata->save();
				delete playerdata;
			}
			mGamePhase = DUEL_END;
#ifdef TESTSUITE
			if (mParent->players[1] == PLAYER_TYPE_TESTSUITE){
				if (testSuite->loadNext()){ 
					loadTestSuitePlayers();
					mGamePhase = DUEL_PLAY;
					testSuite->initGame();
				}else{
					mGamePhase = DUEL_END;
				}
			}else if (mParent->players[0] == PLAYER_TYPE_CPU && mParent->players[1] == PLAYER_TYPE_CPU){
				End();
				Start();
			}
#endif
			mFont->SetColor(ARGB(255,255,255,255));
		}
		if (mEngine->GetButtonClick(PSP_CTRL_START)){
			mGamePhase = DUEL_MENU;
		}
	}else if (mGamePhase == DUEL_MENU){
		menu->Update(dt);
	}else{
		if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)){
			mParent->SetNextState(GAME_STATE_MENU);
		}
	}


}


void GameStateDuel::Render() 
{
  //Erase
  JRenderer::GetInstance()->ClearScreen(ARGB(0,0,0,0));


	if (game)
		game->Render();
	if (mGamePhase == DUEL_END){
		JRenderer::GetInstance()->ClearScreen(ARGB(200,0,0,0));
			char buffer[50];
			int p0life = mPlayers[0]->life;
			if (!mPlayers[0]->isAI() && mPlayers[1]->isAI() ){
				if (game->gameOver !=mPlayers[0]){
					sprintf (buffer, "Victory! Congratulations, You earn 500 credits");
				}else{
					sprintf (buffer, "You have been defeated");
				}
			}else{
				int winner = 2;
				if (game->gameOver !=mPlayers[0]){
					winner = 1;
				}
				sprintf(buffer, "Player %i wins (%i)", winner, p0life );
			}
				mFont->DrawString(buffer, 10, 150);
	}else if (mGamePhase == DUEL_CHOOSE_DECK1 || mGamePhase == DUEL_CHOOSE_DECK2){
		if (deckmenu)
			deckmenu->Render();
	}else if (mGamePhase == ERROR_NO_DECK){
		mFont->DrawString("NO DECK AVAILABLE,",0,SCREEN_HEIGHT/2);
		 mFont->DrawString("PRESS CIRCLE TO GO TO THE DECK EDITOR!",0,SCREEN_HEIGHT/2 + 20);
	}else if (mGamePhase == DUEL_MENU){
		menu->Render();
	}
 
}

void GameStateDuel::ButtonPressed(int controllerId, int controlId)
	{
		switch (controlId)
		{
		case 1:
		case 2:	
		case 3:
		case 4:	
		case 5:
			{
				if (mGamePhase == DUEL_CHOOSE_DECK1){
					loadPlayer(0,controlId);
					mGamePhase = DUEL_CHOOSE_DECK2;
				}else{
					loadPlayer(1,controlId);
					mGamePhase = DUEL_PLAY;
				}
				break;
			}
		case 12:
			mParent->SetNextState(GAME_STATE_MENU);
			break;
		case 13:
			mGamePhase = DUEL_PLAY;
			break;
		}
}







