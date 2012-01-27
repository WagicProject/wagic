#include "PrecompiledHeader.h"

#include "DeckMenu.h"
#include "GameStateDuel.h"
#include "utils.h"
#include "AIPlayer.h"
#include "AIMomirPlayer.h"
#include "PlayerData.h"
#include "DeckStats.h"
#include "DeckManager.h"

#include "DeckMetaData.h"
#include "Credits.h"
#include "Translate.h"
#include "Rules.h"
#include "ModRules.h"

#ifdef TESTSUITE
#include "TestSuiteAI.h"
#endif

#ifdef NETWORK_SUPPORT
#include "NetworkPlayer.h"
#endif

#if defined (WIN32) || defined (LINUX) || defined(IOS)
#include <time.h>
#endif

const float MENU_FONT_SCALE = 1.0f;

enum ENUM_DUEL_STATE
{
    DUEL_STATE_UNSET = 0,
    DUEL_STATE_START,
    DUEL_STATE_END,
    DUEL_STATE_CHOOSE_DECK1,
    DUEL_STATE_DECK1_DETAILED_INFO,
    DUEL_STATE_DECK2_DETAILED_INFO,
    DUEL_STATE_CHOOSE_DECK1_TO_2,
    DUEL_STATE_CHOOSE_DECK2_TO_1,
    DUEL_STATE_CHOOSE_DECK2,
    DUEL_STATE_CHOOSE_DECK2_TO_PLAY,
    DUEL_STATE_ERROR_NO_DECK,
    DUEL_STATE_CANCEL,
    DUEL_STATE_PLAY,
    DUEL_STATE_BACK_TO_MAIN_MENU,
    DUEL_STATE_MENU,
    DUEL_STATE_OPPONENT_WAIT, // used For Network only
    DUEL_STATE_ERROR
};

const char * stateStrings[] = { "unset", "start", "end", "choose_deck1", "deck1_detailed_info", "deck2_detailed_info", "choose_deck1_to_2", "choose_deck2", "choose_deck2_to_play",
    "error_no_deck", "cancel", "play", "back_to_main_menu", "menu", "oponent_wait", "error" };

enum ENUM_DUEL_MENUS
{
    DUEL_MENU_GAME_MENU,
    DUEL_MENU_CHOOSE_DECK,
    DUEL_MENU_CHOOSE_OPPONENT,
    DUEL_MENU_DETAILED_DECK1_INFO,
    DUEL_MENU_DETAILED_DECK2_INFO
};

int GameStateDuel::selectedPlayerDeckId = 0;
int GameStateDuel::selectedAIDeckId = 0;

GameStateDuel::GameStateDuel(GameApp* parent) :
GameState(parent, "duel")
{
    premadeDeck = false;
    game = NULL;
    deckmenu = NULL;
    opponentMenu = NULL;
    menu = NULL;
    popupScreen = NULL;
    mGamePhase = DUEL_STATE_UNSET;

#ifdef TESTSUITE
    testSuite = NULL;
#endif

#ifdef AI_CHANGE_TESTING
    totalTestGames = 0;
    testPlayer2Victories = 0;
    totalAIDecks = 0;
    {
        int found = 1;
        while (found)
        {
            found = 0;
            char buffer[512];
            sprintf(buffer, "ai/baka/deck%i.txt", totalAIDecks + 1);
            if (FileExists(buffer))
            {
                found = 1;
                totalAIDecks++;
            }
        } 
        
    }
#endif

    credits = NULL;

#ifdef NETWORK_SUPPORT
    RegisterNetworkPlayers();
#endif //NETWORK_SUPPORT

}

GameStateDuel::~GameStateDuel()
{
    End();
}

void GameStateDuel::Start()
{
    JRenderer * renderer = JRenderer::GetInstance();
    renderer->EnableVSync(true);
    OpponentsDeckid = 0;

    game = NEW GameObserver(WResourceManager::Instance(), JGE::GetInstance());

#ifdef TESTSUITE
    SAFE_DELETE(testSuite);
    testSuite = NEW TestSuite("test/_tests.txt");
#endif

    setGamePhase(DUEL_STATE_CHOOSE_DECK1);
    credits = NEW Credits();

    menu = NULL;

    int decksneeded = 0;
    for (int i = 0; i < 2; i++)
    {
        if (mParent->players[i] == PLAYER_TYPE_HUMAN)
        {
            //DeckManager::EndInstance();
            decksneeded = 1;

            deckmenu = NEW DeckMenu(DUEL_MENU_CHOOSE_DECK, this, Fonts::OPTION_FONT, "Choose a Deck",
                GameStateDuel::selectedPlayerDeckId, true);
            deckmenu->enableDisplayDetailsOverride();
            DeckManager *deckManager = DeckManager::GetInstance();
            vector<DeckMetaData *> playerDeckList = BuildDeckList(options.profileFile());
            int nbDecks = playerDeckList.size();

            if (nbDecks)
                decksneeded = 0;
            
             if (nbDecks > 1)   
                deckmenu->Add(MENUITEM_RANDOM_PLAYER, "Random", "Play with a random deck.");

            renderDeckMenu(deckmenu, playerDeckList);
            // save the changes to the player deck list maintained in DeckManager
            deckManager->updateMetaDataList(&playerDeckList, false);
            playerDeckList.clear();

            break;
        }
    }

    if (deckmenu)
    {
        if (decksneeded)
        {
            if (gModRules.general.hasDeckEditor())
            {
                //translate deck creating desc
                Translator * t = Translator::GetInstance();
                string desc =  "Highly recommended to get\nthe full Wagic experience!";
                map<string, string>::iterator it = t->deckValues.find("Create your Deck!");
                if (it != t->deckValues.end())
                    desc = it->second;

                deckmenu->Add(MENUITEM_NEW_DECK, "Create your Deck!", desc);
            }
            premadeDeck = true;
            fillDeckMenu(deckmenu, "player/premade");
        }
        else if (gModRules.general.hasDeckEditor())
        {
            deckmenu->Add(MENUITEM_NEW_DECK, "New Deck...", "Create a new deck to play with.");
        }
        deckmenu->Add(MENUITEM_CANCEL, "Main Menu", "Return to Main Menu");
    }
    
    mEngine->ResetInput();
}

void GameStateDuel::initRand(unsigned int seed)
{
    if (!seed)
        seed = static_cast<unsigned int>(time(0));
    srand(seed);
}

#ifdef TESTSUITE
void GameStateDuel::loadTestSuitePlayers()
{
    if (!testSuite) return;
    initRand(testSuite->seed);
    SAFE_DELETE(game);
    game = new GameObserver(WResourceManager::Instance(), JGE::GetInstance());
    testSuite->setObserver(game);
    for (int i = 0; i < 2; i++)
    {
        game->loadTestSuitePlayer(i, testSuite);
    }
    mParent->gameType = testSuite->getGameType();

    game->startGame(mParent->gameType, mParent->rules);
}
#endif

void GameStateDuel::End()
{
    DebugTrace("Ending GameStateDuel");

#ifdef TRACK_OBJECT_USAGE
    ObjectAnalytics::DumpStatistics();
#endif

    JRenderer::GetInstance()->EnableVSync(false);

    SAFE_DELETE(game);
    premadeDeck = false;
    SAFE_DELETE(credits);

    SAFE_DELETE(menu);
    SAFE_DELETE(opponentMenu);
    SAFE_DELETE(deckmenu);
    SAFE_DELETE(popupScreen);

#ifdef TESTSUITE
    SAFE_DELETE(testSuite);
#endif
}

//TODO Move This to utils or ResourceManager. Don't we have more generic functions that can do that?
bool GameStateDuel::MusicExist(string FileName)
{
    return FileExists(WResourceManager::Instance()->musicFile(FileName));
}

void GameStateDuel::ConstructOpponentMenu()
{
    if (opponentMenu == NULL)
    {
        opponentMenu = NEW DeckMenu(DUEL_MENU_CHOOSE_OPPONENT, this, Fonts::OPTION_FONT, "Choose Opponent",
            GameStateDuel::selectedAIDeckId, true);
        opponentMenu->Add(MENUITEM_RANDOM_AI, "Random");
        if (options[Options::EVILTWIN_MODE_UNLOCKED].number) opponentMenu->Add(MENUITEM_EVIL_TWIN, "Evil Twin", 
            _("Can you defeat yourself?").c_str());
        DeckManager * deckManager = DeckManager::GetInstance();
        vector<DeckMetaData*> opponentDeckList;
        int nbUnlockedDecks = options[Options::CHEATMODEAIDECK].number ? 1000 : options[Options::AIDECKS_UNLOCKED].number;
        opponentDeckList = fillDeckMenu(opponentMenu, "ai/baka", "ai_baka", game->getPlayer(0), nbUnlockedDecks);
        deckManager->updateMetaDataList(&opponentDeckList, true);
        opponentMenu->Add(MENUITEM_CANCEL, "Cancel", _("Choose a different player deck").c_str());
        opponentDeckList.clear();
    }
}

void GameStateDuel::setGamePhase(int newGamePhase) {
    if (mGamePhase == newGamePhase)
        return;

    if (mGamePhase)
        JGE::GetInstance()->SendCommand("leaveduelphase:" + string(stateStrings[mGamePhase]));

    mGamePhase = newGamePhase;

    if (mGamePhase )
        JGE::GetInstance()->SendCommand("enterduelphase:" +  string(stateStrings[mGamePhase]));
}

#ifdef AI_CHANGE_TESTING
boost::mutex GameStateDuel::mMutex;

void GameStateDuel::ThreadProc(void* inParam)
{
    GameStateDuel* instance = reinterpret_cast<GameStateDuel*>(inParam);
    float counter = 1.0f;
    while(instance->mGamePhase != DUEL_STATE_BACK_TO_MAIN_MENU)
    {
        GameObserver observer;
        int oldTurn = -1;
        int oldPhase = -1;
        int stagnationCounter = -1;

        observer.loadPlayer(0, PLAYER_TYPE_TESTSUITE);
        observer.loadPlayer(1, PLAYER_TYPE_TESTSUITE);
        observer.startGame(instance->mParent->gameType, instance->mParent->rules);

        while(!observer.gameOver) {
            if(observer.turn == oldTurn && observer.currentGamePhase == oldPhase) {
                stagnationCounter++;
            } else {
                stagnationCounter = 0;
                oldTurn = observer.turn;
                oldPhase = observer.currentGamePhase;
            }
            if(stagnationCounter >= 1000)
            {
                observer.dumpAssert(false);
            }
            observer.Update(counter++);
        }

        instance->handleResults(&observer);
    }
}
#endif //AI_CHANGE_TESTING

void GameStateDuel::Update(float dt)
{
    switch (mGamePhase)
    {
    case DUEL_STATE_ERROR_NO_DECK:
        if (JGE_BTN_OK == mEngine->ReadButton()) mParent->SetNextState(GAME_STATE_DECK_VIEWER);
        break;

    case DUEL_STATE_DECK1_DETAILED_INFO:
    case DUEL_STATE_DECK2_DETAILED_INFO:
        popupScreen->Update(dt);
        break;

    case DUEL_STATE_CHOOSE_DECK1:
        if (!mParent->rules->canChooseDeck())
        {
            setGamePhase(DUEL_STATE_PLAY);
        }
#ifdef TESTSUITE
        else if (mParent->players[1] == PLAYER_TYPE_TESTSUITE)
        {
            testSuite->setRules(mParent->rules);

            if (testSuite && testSuite->loadNext())
            {
                loadTestSuitePlayers();
                setGamePhase(DUEL_STATE_PLAY);
                testSuite->initGame(game);
            }
            else
            {
                if (!game)
                {
                    setGamePhase(DUEL_STATE_ERROR);
                }
                else
                {
                    setGamePhase(DUEL_STATE_END);
                }
            }
        }
#endif
        else
        {
            if (mParent->players[0] == PLAYER_TYPE_HUMAN)
            {
                if (!popupScreen || popupScreen->isClosed()) deckmenu->Update(dt);
            }
            else
            {
                game->loadPlayer(0, mParent->players[0]);
                setGamePhase(DUEL_STATE_CHOOSE_DECK2);
            }
        }
        break;
    case DUEL_STATE_CHOOSE_DECK1_TO_2:
        if (deckmenu->isClosed())
            setGamePhase(DUEL_STATE_CHOOSE_DECK2);
        else
            deckmenu->Update(dt);
        break;
    case DUEL_STATE_CHOOSE_DECK2_TO_1:
        if (opponentMenu->isClosed()) {
            setGamePhase(DUEL_STATE_CHOOSE_DECK1);
            SAFE_DELETE(opponentMenu);
        } else
            opponentMenu->Update(dt);
        break;
    case DUEL_STATE_CHOOSE_DECK2:
        if (mParent->players[1] == PLAYER_TYPE_HUMAN)
            deckmenu->Update(dt);
        else
        {
            if (mParent->players[0] == PLAYER_TYPE_HUMAN)
            {
                ConstructOpponentMenu();
                opponentMenu->Update(dt);
            }
            else
            {
                game->loadPlayer(1, mParent->players[1]);
                setGamePhase(DUEL_STATE_PLAY);
            }
        }
        break;
    case DUEL_STATE_CHOOSE_DECK2_TO_PLAY:
        if (mParent->players[1] == PLAYER_TYPE_HUMAN)
        {
            if (deckmenu->isClosed())
                setGamePhase(DUEL_STATE_PLAY);
            else
                deckmenu->Update(dt);
        }
        else
        {
            ConstructOpponentMenu();
            if (opponentMenu->isClosed())
                setGamePhase(DUEL_STATE_PLAY);
            else
                opponentMenu->Update(dt);
        }
        break;
    case DUEL_STATE_PLAY:
        if (!game->isStarted())
        {
            game->startGame(mParent->gameType, mParent->rules);

            //start of in game music code
            musictrack = "";
            //check opponent id and choose the music track based on it
            if (OpponentsDeckid)
            {
                char temp[4096];
                sprintf(temp, "ai_baka_music%i.mp3", OpponentsDeckid);
                musictrack.assign(temp);
            }
            else if (mParent->gameType == GAME_TYPE_CLASSIC)
                musictrack = "ai_baka_music.mp3";
            else if (mParent->gameType == GAME_TYPE_MOMIR)
                musictrack = "ai_baka_music_momir.mp3";
            else if (mParent->gameType == GAME_TYPE_RANDOM1 || mParent->gameType == GAME_TYPE_RANDOM2) musictrack
                = "ai_baka_music_random.mp3";

            if (!MusicExist(musictrack)) musictrack = "ai_baka_music.mp3";

            GameApp::playMusic(musictrack);
        }
        game->Update(dt);
        //run a "post update" init call in the rules. This is for things such as Manapool, which gets emptied in the update
        // That's mostly because of a legacy bug, where we use the update sequence for some things when we should use events (such as phase changes)
        mParent->rules->postUpdateInit(game);

        if (game->gameOver)
        {
            if (game->players[1]->playMode != Player::MODE_TEST_SUITE) credits->compute(game, mParent);
            setGamePhase(DUEL_STATE_END);
#ifdef TESTSUITE
            if (mParent->players[1] == PLAYER_TYPE_TESTSUITE)
            {
                if (testSuite->loadNext())
                {
                    loadTestSuitePlayers();
                    setGamePhase(DUEL_STATE_PLAY);
                    testSuite->initGame(game);
                }
                else {
                    setGamePhase(DUEL_STATE_END);
                }
            }
            else
#endif
#ifdef AI_CHANGE_TESTING
            {
                if (mParent->players[0] == PLAYER_TYPE_CPU_TEST && mParent->players[1] == PLAYER_TYPE_CPU_TEST)
                {
                    handleResults(game);
                    End();
                    Start();

                    if(mWorkerThread.empty())
                    {   // "I don't like to wait" mode
                        size_t thread_count = 1;
                        startTime = JGEGetTime();

                #ifdef QT_CONFIG
                        thread_count = QThread::idealThreadCount();
                #endif
                        for(size_t i = 0; i < (thread_count-1); i++)
                            mWorkerThread.push_back(boost::thread(ThreadProc, this));
                    }
                }
            }
#endif
            if (mParent->players[0] == PLAYER_TYPE_CPU && mParent->players[1] == PLAYER_TYPE_CPU)
            {
                End();
                Start();
            }
        }
        if (mEngine->GetButtonClick(JGE_BTN_MENU))
        {
            if (!menu)
            {
                menu = NEW SimpleMenu(JGE::GetInstance(), DUEL_MENU_GAME_MENU, this, Fonts::MENU_FONT, SCREEN_WIDTH / 2 - 100, 25,
                    game->players[1]->deckName.c_str());
                int cardsinhand = game->currentPlayer->game->hand->nb_cards;

                //almosthumane - mulligan
                if ((game->turn < 1) && (cardsinhand != 0) && game->currentGamePhase == MTG_PHASE_FIRSTMAIN
                    && game->currentPlayer->game->inPlay->nb_cards == 0 && game->currentPlayer->game->graveyard->nb_cards == 0
					&& game->currentPlayer->game->exile->nb_cards == 0 && game->currentlyActing() == (Player*)game->currentPlayer) //1st Play Check
                    //IF there was no play at the moment automatically mulligan
                {
                    menu->Add(MENUITEM_MULLIGAN, "Mulligan");
                }
                //END almosthumane - mulligan
                menu->Add(MENUITEM_MAIN_MENU, "Back to main menu");
                //menu->Add(MENUITEM_UNDO, "Undo");
#ifdef TESTSUITE
                menu->Add(MENUITEM_LOAD, "Load");
#endif
                menu->Add(MENUITEM_CANCEL, "Cancel");
            }
            setGamePhase(DUEL_STATE_MENU);
        }
        break;
#ifdef NETWORK_SUPPORT
    case DUEL_STATE_OPPONENT_WAIT:
      {
        if(mPlayers[1] && mPlayers[1]->game)
        { // Player loaded
          menu->Close();
          SAFE_DELETE(menu);
          setGamePhase(DUEL_STATE_PLAY);
        } else if(menu == NULL)
        {
          loadPlayer(1, 42/* 0 not good*/, false, true);
          menu = NEW SimpleMenu(DUEL_STATE_OPPONENT_WAIT, this, Fonts::MENU_FONT, 150, 60);
          if (menu)
          {
              menu->Add(MENUITEM_MAIN_MENU, "Back to main menu");
          }
        } else
        {
          menu->Update(dt);
        }
      }
      break;
#endif //NETWORK_SUPPORT
    case DUEL_STATE_MENU:
        menu->Update(dt);
        break;
    case DUEL_STATE_CANCEL:
        menu->Update(dt);
        if (menu->isClosed())
        {
            setGamePhase(DUEL_STATE_PLAY);
            SAFE_DELETE(menu);
        }
        break;
    case DUEL_STATE_BACK_TO_MAIN_MENU:
        if (menu)
        {
#ifdef AI_CHANGE_TESTING
            while(mWorkerThread.size())
            {
              mWorkerThread.back().join();
              mWorkerThread.pop_back();
            }
#endif //AI_CHANGE_TESTING

            menu->Update(dt);
            if (menu->isClosed())
            {
                PlayerData * playerdata = NEW PlayerData(MTGCollection());
                playerdata->taskList->passOneDay();
                playerdata->taskList->save();
                SAFE_DELETE(playerdata);
                SAFE_DELETE(menu);
            }
        }
        mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);

        break;
    default:
        if (JGE_BTN_OK == mEngine->ReadButton()) mParent->SetNextState(GAME_STATE_MENU);
    }
}

void GameStateDuel::Render()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    JRenderer * r = JRenderer::GetInstance();
    r->ClearScreen(ARGB(0,0,0,0));

    if (game) game->Render();

#ifdef AI_CHANGE_TESTING
      if (game && totalTestGames)
      {
          char buf[4096];
          int currentTime = JGEGetTime();

          if (totalTestGames < 2.5 * totalAIDecks)
          {
               mFont->SetColor(ARGB(255,255,255,0));
               sprintf(buf, "Results are not significant, you should let at least %i more games run", (int)(totalAIDecks * 2.5) - totalTestGames);
               mFont->DrawString(buf,0,SCREEN_HEIGHT/2 - 20);
          }

          mFont->SetColor(ARGB(255,255,255,255));
          float ratio = float(testPlayer2Victories) / float(totalTestGames);
          if (ratio < 0.48)
              mFont->SetColor(ARGB(255,255,0,0));
          if (ratio > 0.52)
              mFont->SetColor(ARGB(255,0,255,0));
          sprintf(buf, "Victories Player 2/total Games: %i/%i - Games/second: %f",
                  testPlayer2Victories, totalTestGames, (float)(1000*totalTestGames)/(currentTime - startTime));
          mFont->DrawString(buf,0,SCREEN_HEIGHT/2);
      }
#endif

    switch (mGamePhase)
    {
    case DUEL_STATE_END:
        {
            r->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(200,0,0,0));
            credits->Render();
#ifdef TESTSUITE
            if (mParent->players[1] == PLAYER_TYPE_TESTSUITE)
            {
                r->ClearScreen(ARGB(255,0,0,0));
                char buf[4096];
                mFont->SetColor(ARGB(255,255,255,255));

                int elapsedTime = (testSuite->endTime - testSuite->startTime);
                sprintf(buf, "Time to run the tests: %is", elapsedTime/1000);
                mFont->DrawString(buf,0,SCREEN_HEIGHT/2 - 20);

                int nbFailed = testSuite->nbFailed;
                int nbTests = testSuite->nbTests;
                if (!nbFailed)
                {
                    sprintf(buf, "All %i tests successful!", nbTests);
                }
                else
                {
                    sprintf(buf, "%i tests out of %i FAILED!", nbFailed, nbTests);
                }
  
                mFont->DrawString(buf,0,SCREEN_HEIGHT/2);
                nbFailed = testSuite->nbAIFailed;
                nbTests = testSuite->nbAITests;
                if (nbTests)
                {
                    if (!nbFailed)
                    {
                        sprintf(buf, "AI Tests: All %i tests successful!", nbTests);
                    }
                    else
                    {
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
            mFont->DrawString(_("AN ERROR OCCURRED, CHECK FILE NAMES").c_str(), 0, SCREEN_HEIGHT / 2);
            break;
        }
    case DUEL_STATE_CHOOSE_DECK1:
    case DUEL_STATE_CHOOSE_DECK1_TO_2:
    case DUEL_STATE_CHOOSE_DECK2:
    case DUEL_STATE_CHOOSE_DECK2_TO_PLAY:
    case DUEL_STATE_DECK1_DETAILED_INFO:
    case DUEL_STATE_DECK2_DETAILED_INFO:
        if (mParent->gameType != GAME_TYPE_CLASSIC
#ifdef NETWORK_SUPPORT
                && mParent->gameType != GAME_TYPE_SLAVE
#endif //NETWORK_SUPPORT
				&& mParent->gameType != GAME_TYPE_STONEHEWER
				&& mParent->gameType != GAME_TYPE_HERMIT)
            mFont->DrawString(_("LOADING DECKS").c_str(), 0, SCREEN_HEIGHT / 2);
        else
        {
            if (opponentMenu && !opponentMenu->isClosed())
                opponentMenu->Render();
            else if (deckmenu && !deckmenu->isClosed()) deckmenu->Render();

            if (menu) menu->Render();

            if (popupScreen && !popupScreen->isClosed()) popupScreen->Render();
        }
        break;
    case DUEL_STATE_ERROR_NO_DECK:
        mFont->DrawString(_("NO DECK AVAILABLE,").c_str(), 0, SCREEN_HEIGHT / 2);
        mFont->DrawString(_("PRESS CIRCLE TO GO TO THE DECK EDITOR!").c_str(), 0, SCREEN_HEIGHT / 2 + 20);
        break;
#ifdef NETWORK_SUPPORT
    case DUEL_STATE_OPPONENT_WAIT:
        if (menu) menu->Render();
        break;
#endif //NETWORK_SUPPORT
    case DUEL_STATE_MENU:
    case DUEL_STATE_CANCEL:
    case DUEL_STATE_BACK_TO_MAIN_MENU:
        if (game)
        {
            r->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(100,0,0,0));
            char buffer[4096];
            sprintf(buffer, _("Turn:%i").c_str(), game->turn);
            mFont->SetColor(ARGB(255,255,255,255));
            mFont->DrawString(buffer, SCREEN_WIDTH / 2, 0, JGETEXT_CENTER);
        }
        if (menu) 
            menu->Render();
    }
}

void GameStateDuel::ButtonPressed(int controllerId, int controlId)
{
    int deckNumber = controlId;
    DeckManager * deckManager = DeckManager::GetInstance();
    int aiDeckSize = deckManager->getAIDeckOrderList()->size();
    switch (controllerId)
    {

    case DUEL_MENU_DETAILED_DECK1_INFO:
        if ((popupScreen || deckmenu->showDetailsScreen()))
        {
            DeckMetaData* selectedDeck = deckmenu->getSelectedDeck();
            if (!popupScreen->isClosed())
            {
                popupScreen->Close();
                setGamePhase(DUEL_STATE_CHOOSE_DECK1);
                SAFE_DELETE( popupScreen );
            }
            else
            {
                popupScreen->Update(selectedDeck);
                popupScreen->Render();
            }
        }
        break;
    case DUEL_MENU_DETAILED_DECK2_INFO:
        if ((popupScreen || opponentMenu->showDetailsScreen()))
        {
            DeckMetaData* selectedDeck = opponentMenu->getSelectedDeck();
            if (!popupScreen->isClosed())
            {
                popupScreen->Close();
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);
                SAFE_DELETE( popupScreen );
            }
            else
            {
                popupScreen->Update(selectedDeck);
                popupScreen->Render();
            }
        }
        break;

    case DUEL_MENU_CHOOSE_OPPONENT:

        switch (controlId)
        {
        case MENUITEM_RANDOM_AI:
            game->loadPlayer(1, mParent->players[1]);
            opponentMenu->Close();
            setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);
            break;
        default:
            // cancel option.  return to player deck selection

            if (controlId == MENUITEM_CANCEL)
            {
                opponentMenu->Close();
                deckmenu->Close();
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_1);
                break;
            }

            else if (controlId == MENUITEM_MORE_INFO && opponentMenu->showDetailsScreen())
            {
                DeckMetaData* selectedDeck = opponentMenu->getSelectedDeck();
                if (!popupScreen)
                {
                    popupScreen = NEW SimplePopup(DUEL_MENU_DETAILED_DECK2_INFO, this, Fonts::MAIN_FONT, "Detailed Information",
                        selectedDeck, MTGCollection());
                    popupScreen->Render();
                    selectedAIDeckId = selectedDeck->getDeckId();
                }
                else
                {
                    popupScreen->Update(selectedDeck);
                }
                setGamePhase(DUEL_STATE_DECK2_DETAILED_INFO);
                break;
            }
            else if (controlId == MENUITEM_MORE_INFO && !opponentMenu->showDetailsScreen())
            {
                // do nothing, ignore all key requests until popup is dismissed.
                break;
            }
            else if (controlId != MENUITEM_EVIL_TWIN && aiDeckSize > 0) // evil twin
                deckNumber = deckManager->getAIDeckOrderList()->at(controlId - 1)->getDeckId();
            game->loadPlayer(1, mParent->players[1], deckNumber, premadeDeck);
            OpponentsDeckid = deckNumber;
            opponentMenu->Close();
            setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);
            break;
        }
        break;

    case DUEL_MENU_CHOOSE_DECK:

        if (controlId == MENUITEM_RANDOM_PLAYER) // Random Player Deck Selection
        {
            vector<DeckMetaData *> * playerDeckList = deckManager->getPlayerDeckOrderList();
            deckNumber = playerDeckList->at(WRand() % (playerDeckList->size()))->getDeckId();
            game->loadPlayer(0, mParent->players[0], deckNumber, premadeDeck);
            deckmenu->Close();
            setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);
            break;
        }
        else if (controlId == MENUITEM_MAIN_MENU || controlId == MENUITEM_CANCEL) // user clicked on "Cancel"
        {
            if (deckmenu) deckmenu->Close();
            setGamePhase(DUEL_STATE_BACK_TO_MAIN_MENU);
            break;
        }
        else if (controlId == MENUITEM_MORE_INFO && deckmenu->showDetailsScreen())
        {
            DeckMetaData* selectedDeck = deckmenu->getSelectedDeck();
            if (!popupScreen)
            {
                popupScreen = NEW SimplePopup(DUEL_MENU_DETAILED_DECK1_INFO, this, Fonts::MAIN_FONT, "Detailed Information",
                    selectedDeck, MTGCollection());
                popupScreen->Render();
                selectedPlayerDeckId = deckmenu->getSelectedDeckId();
            }
            else
            {
                popupScreen->Update(selectedDeck);
            }
            setGamePhase(DUEL_STATE_DECK1_DETAILED_INFO);
            break;
        }
        else if (controlId == MENUITEM_MORE_INFO)
        {
            // do nothing
            break;
        }
        if (controlId < 0)
        {
            mParent->SetNextState(GAME_STATE_DECK_VIEWER);
            return;
        }
        if (mGamePhase == DUEL_STATE_CHOOSE_DECK1)
        {
            vector<DeckMetaData *> * playerDeck = deckManager->getPlayerDeckOrderList();
            if (!premadeDeck && controlId > 0) 
                deckNumber = playerDeck->at(controlId - 1)->getDeckId();
            game->loadPlayer(0, mParent->players[0], deckNumber, premadeDeck);
            deckmenu->Close();
#ifdef NETWORK_SUPPORT
            if(mParent->players[1] == PLAYER_TYPE_REMOTE)
            {   // no need to choose an opponent deck in network mode
                setGamePhase(DUEL_STATE_OPPONENT_WAIT);
            }
            else
#endif //NETWORK_SUPPORT
            {
                setGamePhase(DUEL_STATE_CHOOSE_DECK1_TO_2);
            }
            playerDeck = NULL;
        }
        else
        {
            game->loadPlayer(1, mParent->players[1], controlId, premadeDeck);
            deckmenu->Close();
            setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);
        }
        break;

    default:

        switch (controlId)
        {

        case MENUITEM_MAIN_MENU:
            menu->Close();
            setGamePhase(DUEL_STATE_BACK_TO_MAIN_MENU);
            break;
        case MENUITEM_CANCEL:
            menu->Close();
            setGamePhase(DUEL_STATE_CANCEL);
            break;
        case MENUITEM_MULLIGAN:
            //almosthumane - mulligan
            game->Mulligan();
            
            menu->Close();
            setGamePhase(DUEL_STATE_CANCEL);
            break;
        case MENUITEM_UNDO:
            {
            game->undo();
            menu->Close();
            setGamePhase(DUEL_STATE_CANCEL);
            break;
            }
#ifdef TESTSUITE
        case MENUITEM_LOAD:
            {
            std::string theGame;
            if (JFileSystem::GetInstance()->readIntoString("test/game/timetwister.txt", theGame))
            {
                game->load(theGame);
            }
            menu->Close();
            setGamePhase(DUEL_STATE_CANCEL);
            break;
            }
#endif
        }
            mEngine->ReleaseKey( JGE_BTN_MENU );

    }
}

void GameStateDuel::OnScroll(int inXVelocity, int inYVelocity)
{
    // ignore magnitude for now, since no action requires scrolling
    if (abs(inYVelocity) > 300)
    {
        bool flickUpwards = (inYVelocity < 0);
        mEngine->HoldKey_NoRepeat(flickUpwards ? JGE_BTN_PREV : JGE_BTN_SEC);
    }
}

