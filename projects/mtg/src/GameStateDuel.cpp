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
    DUEL_STATE_CHOOSE_DECK1_TO_PLAY,
    DUEL_STATE_CHOOSE_DECK2_TO_1,
    DUEL_STATE_CHOOSE_DECK2,
    DUEL_STATE_CHOOSE_DECK2_TO_2, // for tournament selection
    DUEL_STATE_CHOOSE_DECK2_TO_PLAY,
    DUEL_STATE_ERROR_NO_DECK,
    DUEL_STATE_CANCEL,
    DUEL_STATE_PLAY,
    DUEL_STATE_BACK_TO_MAIN_MENU,
    DUEL_STATE_MENU,
    DUEL_STATE_OPPONENT_WAIT, // used For Network only
    DUEL_STATE_PREPARE_CNOGMENU,
    DUEL_STATE_CHOOSE_NUMBER_OF_GAMES,
    DUEL_STATE_CNOGMENU_IS_CLOSING,
    DUEL_STATE_CANCEL_CNOGMENU,
    DUEL_STATE_CONTINUE_TOURNAMENT,
    DUEL_STATE_SHOW_SCORE,
    DUEL_STATE_MENU_TO_SCORE,
    DUEL_STATE_END_OF_GAME,
    DUEL_STATE_END_OF_MATCH,
    DUEL_STATE_END_OF_TOURNAMENT,
    DUEL_STATE_ERROR
};

const char * stateStrings[] = { "unset", "start", "end", "choose_deck1", "deck1_detailed_info", "deck2_detailed_info", "choose_deck1_to_2", "choose_deck1_to_play","choose_deck2_to_1", "choose_deck2",
                                "choose_deck2_to_2", "choose_deck2_to_play","error_no_deck", "cancel", "play", "back_to_main_menu", "menu", "oponent_wait","prepare_cnogmenu",
                                "choose_number_games","cnogmenu_closes","cancel_cnogmenu","continue_tournament","show_score","menu_to_score","end_of_game","end_of_match","end_of_tournament", "error"};


enum ENUM_DUEL_MENUS
{
    DUEL_MENU_GAME_MENU,
    DUEL_MENU_CHOOSE_DECK,
    DUEL_MENU_CHOOSE_OPPONENT,
    DUEL_MENU_DETAILED_DECK1_INFO,
    DUEL_MENU_CHOOSE_NUMBER_OF_GAMES,
    DUEL_MENU_DETAILED_DECK2_INFO
};

enum ENUM_CNOGMENU_ITEMS
{
  CNOGMENU_ITEM_SINGLE_GAME,
  CNOGMENU_ITEM_CONTINUE_TOURNAMENT,
  CNOGMENU_ITEM_BESTOF3,
  CNOGMENU_ITEM_BESTOF5,
  CNOGMENU_ITEM_BESTOF7,
  CNOGMENU_ITEM_BESTOF13,
  CNOGMENU_ITEM_DELTA2,
  CNOGMENU_ITEM_DELTA3,
  CNOGMENU_ITEM_CONSECUTIVE2,
  CNOGMENU_ITEM_CONSECUTIVE3,
  CNOGMENU_ITEM_3GAMES,
  CNOGMENU_ITEM_5GAMES,
  CNOGMENU_ITEM_10GAMES,
  CNOGMENU_ITEM_25GAMES,
  CNOGMENU_ITEM_50GAMES,
  CNOGMENU_ITEM_100GAMES,
  CNOGMENU_ITEM_BESTOF7PLAYOFF,
  CNOGMENU_ITEM_CANCEL
};

enum ENUM_MATCHMODES {
  MATCHMODE_FIXED = 1,
  MATCHMODE_BESTOF = 2,
  MATCHMODE_DELTA = 3,
  MATCHMODE_CONSECUTIVE = 4
};

enum ENUM_TOURNAMENTMODES {
  TOURNAMENTMODES_ENDLESS = 1,
  TOURNAMENTMODES_GAUNTLET = 2,
  TOURNAMENTMODES_KO = 3,
  TOURNAMENTMODES_DOUBLEKO = 4,
  TOURNAMENTMODES_ROUNDROBIN = 5
};


//enum ENUM_GAMEMENU_ITEMS
//{
//  GAMEMENU_ITEM_CONTINUE,
//  GAMEMENU_ITEM_SPEED_STEPWISE,
//  GAMEMENU_ITEM_CONCEDE_GAME,
//  GAMEMENU_ITEM_CONCEDE_MATCH
//};

enum ENUM_OPERATION_LEVELS
{
  OPLEVEL_NEW = 1,
  OPLEVEL_NEXT_MATCH = 2,
  OPLEVEL_NEXT_GAME = 3,
  OPLEVEL_GAME_ACTIV = 4
};


int GameStateDuel::selectedPlayerDeckId = 0;
int GameStateDuel::selectedAIDeckId = 0;

GameStateDuel::GameStateDuel(GameApp* parent) :
GameState(parent, "duel")
{

    cnogmenu = NULL;
    tournament = new Tournament();
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
}

GameStateDuel::~GameStateDuel()
{
    End();
    SAFE_DELETE(tournament);
}

void GameStateDuel::Start()
{
    JRenderer * renderer = JRenderer::GetInstance();
    renderer->EnableVSync(true);
    OpponentsDeckid = 0;
    bool createDeckMenu=true; // create only a deckmenu if not in tournament


#ifdef NETWORK_SUPPORT
    if(!mParent->mpNetwork) {
#endif //NETWORK_SUPPORT
        game = NEW GameObserver(WResourceManager::Instance(), JGE::GetInstance());
#ifdef NETWORK_SUPPORT
    } else {
        game = NEW NetworkGameObserver(mParent->mpNetwork, WResourceManager::Instance(), JGE::GetInstance());
    }
#endif //NETWORK_SUPPORT


#ifdef TESTSUITE
    SAFE_DELETE(testSuite);
    testSuite = NEW TestSuite("test/_tests.txt");
#endif

    setGamePhase(DUEL_STATE_CHOOSE_DECK1);
    credits = NEW Credits();

    // match mode is available in classic and demo mode.
    // in both modes player 1 is from type PLAYER_TYPE_CPU
    if (mParent->players[1] == PLAYER_TYPE_CPU && (mParent->gameType == GAME_TYPE_CLASSIC || mParent->gameType == GAME_TYPE_DEMO))
    {
        //initialize match
        // only reset "Played Games" and "Victories" info if we didn't come here from within a match
        tournament->Start();

        if (tournament->getOpLevel()< OPLEVEL_NEXT_GAME)
            setGamePhase(DUEL_STATE_PREPARE_CNOGMENU);
        else
        {
            createDeckMenu=false; //there is a tournament running and we do not need a deckmenu
            SAFE_DELETE(deckmenu);
        }
        tournamentSelection=false;
    }

    menu = NULL;
    int decksneeded = 0;
    for (int i = 0; i < 2; i++)
    {
        if (createDeckMenu && mParent->players[i] == PLAYER_TYPE_HUMAN)
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
                string desc =  _("Highly recommended to get\nthe full Wagic experience!").c_str();
                map<string, string>::iterator it = t->deckValues.find(_("Create your Deck!").c_str());
                if (it != t->deckValues.end())
                    desc = it->second;

                deckmenu->Add(MENUITEM_NEW_DECK, _("Create your Deck!").c_str(), desc);
            }
            premadeDeck = true;
            fillDeckMenu(deckmenu, _("player/premade").c_str());
        }
        else if (gModRules.general.hasDeckEditor())
        {
            deckmenu->Add(MENUITEM_NEW_DECK, _("New Deck...").c_str(), _("Create a new deck to play with.").c_str());
        }
        deckmenu->Add(MENUITEM_CANCEL, _("Main Menu").c_str(), _("Return to Main Menu").c_str());
    }
    else if(createDeckMenu && (mParent->players[0] == PLAYER_TYPE_CPU && mParent->players[1] == PLAYER_TYPE_CPU))
    {
        //DeckManager::EndInstance();
        deckmenu = NEW DeckMenu(DUEL_MENU_CHOOSE_DECK, this, Fonts::OPTION_FONT, "Choose a Deck", GameStateDuel::selectedPlayerDeckId, true);
        int nmbDecks = options[Options::CHEATMODEAIDECK].number ? 1000 : options[Options::AIDECKS_UNLOCKED].number;
        if (nmbDecks > 1)
        {
             deckmenu->Add(MENUITEM_ENDLESSDEMO,"Endless demo",_("Random AI decks plays\n again each other").c_str());
             deckmenu->Add(MENUITEM_KO_TOURNAMENT,"KO Tournament",_("Starts a Knockout Tournament  of all available AI-Decks.\n ").c_str());
             deckmenu->Add(MENUITEM_DOUBLEKO_TOURNAMENT,"Double KO Tournament",_("Starts a double Knockout Tournament  of all available AI-Decks.\n ").c_str());
             deckmenu->Add(MENUITEM_GAUNTLET,"Gauntlet",_("Prove your mettle against each and\nevery opponent, one at a time.").c_str());
             deckmenu->Add(MENUITEM_RANDOM_PLAYER, "Random", _("Selects a random deck.").c_str());
        }
        //deckmenu->enableDisplayDetailsOverride();
        DeckManager *deckManager = DeckManager::GetInstance();
        vector<DeckMetaData *> playerDeckList;

        playerDeckList = fillDeckMenu(deckmenu, "ai/baka","ai_baka",NULL,nmbDecks);
        //printf("nmbDecks %i\n",playerDeckList.size());
        renderDeckMenu(deckmenu, playerDeckList);
        // save the changes to the player deck list maintained in DeckManager
        deckManager->updateMetaDataList(&playerDeckList, true);
        //save the real number of available decks
        tournament->setAvailableDecks(playerDeckList.size());
        playerDeckList.clear();
        deckmenu->Add(MENUITEM_CANCEL, _("Main Menu").c_str(), _("Return to Main Menu").c_str());

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

    //reset player for next match (only if actual match is finished)
    tournament->End();

#ifdef TESTSUITE
    SAFE_DELETE(testSuite);
#endif
}

//TODO Move This to utils or ResourceManager. Don't we have more generic functions that can do that?
bool GameStateDuel::MusicExist(string FileName)
{
    string musicFilename = WResourceManager::Instance()->musicFile(FileName);
    if (musicFilename.length() < 1) return false;

    return FileExists(musicFilename);
}

void GameStateDuel::ConstructOpponentMenu()
{
    if (opponentMenu == NULL)
    {
        opponentMenu = NEW DeckMenu(DUEL_MENU_CHOOSE_OPPONENT, this, Fonts::OPTION_FONT, "Choose Opponent",
            GameStateDuel::selectedAIDeckId, true);

        int nbUnlockedDecks = options[Options::CHEATMODEAIDECK].number ? 1000 : options[Options::AIDECKS_UNLOCKED].number;
        if ((mParent->gameType == GAME_TYPE_CLASSIC || mParent->gameType == GAME_TYPE_DEMO)&&mParent->players[1] == PLAYER_TYPE_CPU)
        {
            if (!tournamentSelection)
            {
                if (!tournament->isGauntlet())
                {
                    if (mParent->players[0] ==  PLAYER_TYPE_HUMAN){
                        opponentMenu->Add(MENUITEM_KO_TOURNAMENT,"KO Tournament",_("Starts a Knockout Tournament  against selected AI-Decks! You have to win all matches.\n 4 Decks -> 2 Matches\n7 Decks -> 3 Matches\n16 Decks -> 4 Matches\n32 Decks -> 5 Matches\n64 Decks -> 6 Matches\n128 Decks -> 7 Matches\n256 Decks -> 8 Matches").c_str());
                        opponentMenu->Add(MENUITEM_DOUBLEKO_TOURNAMENT,"Double KO",_("Starts a Double Knockout Tournament  against selected AI-Decks! You can loose once, but not more!\n 4 Decks -> 3 Matches\n7 Decks -> 4 Matches\n16 Decks -> 5 Matches\n32 Decks -> 6 Matches\n64 Decks -> 7 Matches\n128 Decks -> 8 Matches\n256 Decks -> 9 Matches").c_str());
                    }
                    opponentMenu->Add(MENUITEM_RR_TOURNAMENT,"Round Robin Tournament",_("Starts an all vs all tournament!\nFirstly, in a selection process participating AI-Decks can be chosen.\n The tournament state is saved after each game  and can be\n continued!").c_str());
                }
                if (mParent->players[0] ==  PLAYER_TYPE_CPU)
                    opponentMenu->Add(MENUITEM_GAUNTLET,"Gauntlet",_("Prove your mettle against each and\nevery opponent, one at a time.").c_str());
                opponentMenu->Add(MENUITEM_RANDOM_AI, "Random");
                if (mParent->players[0] ==  PLAYER_TYPE_HUMAN)
                   opponentMenu->Add(MENUITEM_RANDOM_AI_HARD, "Random (Not easy)",_("Selects a random  AI deck\nwith hard or normal difficulty.").c_str());
            }
            else
            {
                opponentMenu->Add(MENUITEM_START_TOURNAMENT,"Begin Tournament",(std::string("Stop selection and start ")+tournament->exportTournamentDescription()).c_str());
                if (tournament->getTournamentMode()==TOURNAMENTMODES_KO || tournament->getTournamentMode()==TOURNAMENTMODES_DOUBLEKO)
                {
                    opponentMenu->Add(MENUITEM_FILL_NEXT_STAGE_HARD,"Fill stage (Not easy)",_("Fill next stage with random hard or normal opponents.\n1 opponent -> 1 stage\n3 opponents -> 2 stages\n7 opponents -> 3 stages\n15 opponents -> 4 stages\n31 opponents -> 5 stages\n63 opponents -> 6 stages\n127 opponents -> 7 stages\n255 opponents -> 8 stages").c_str());
                    opponentMenu->Add(MENUITEM_FILL_NEXT_STAGE,"Fill stage",_("Fill next stage with random opponents.\n1 opponent -> 1 stage\n3 opponents -> 2 stages\n7 opponents -> 3 stages\n15 opponents -> 4 stages\n31 opponents -> 5 stages\n63 opponents -> 6 stages\n127 opponents -> 7 stages\n255 opponents -> 8 stages").c_str());
                }
                else
                {
                     if (mParent->players[0] ==  PLAYER_TYPE_HUMAN)
                        opponentMenu->Add(MENUITEM_RANDOM_AI_HARD, "Random (Not easy)",_("Selects a random  AI deck\nwith hard or normal difficulty.").c_str());
                     opponentMenu->Add(MENUITEM_RANDOM_AI, "Random");
                }

            }
        }
        if (options[Options::EVILTWIN_MODE_UNLOCKED].number && !tournamentSelection)
            opponentMenu->Add(MENUITEM_EVIL_TWIN, "Evil Twin", _("Can you defeat yourself?").c_str());
        DeckManager * deckManager = DeckManager::GetInstance();
        vector<DeckMetaData*> opponentDeckList;

        opponentDeckList = fillDeckMenu(opponentMenu, "ai/baka", "ai_baka", game->getPlayer(0), nbUnlockedDecks);
        deckManager->updateMetaDataList(&opponentDeckList, true);
        tournament->setAvailableDecks(opponentDeckList.size());
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
        observer.loadPlayer(0, PLAYER_TYPE_CPU_TEST);
        observer.loadPlayer(1, PLAYER_TYPE_CPU_TEST);
        observer.startGame(instance->mParent->gameType, instance->mParent->rules);
        while(!observer.didWin()) 
        {
            if(observer.turn == oldTurn && observer.getCurrentGamePhase() == oldPhase) 
            {
                stagnationCounter++;
            } 
            else 
            {
                stagnationCounter = 0;
                oldTurn = observer.turn;
                oldPhase = observer.getCurrentGamePhase();
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
      // The next state prepares the "Choose number of games" menu (PSY)
    case DUEL_STATE_PREPARE_CNOGMENU:
        SAFE_DELETE(cnogmenu);

        cnogmenu = NEW SimpleMenu(JGE::GetInstance(),DUEL_MENU_CHOOSE_NUMBER_OF_GAMES, this, Fonts::MENU_FONT,35,25,"How many games per match?");
        cnogmenu->Add(CNOGMENU_ITEM_SINGLE_GAME,"Single Game","");
        if (tournament->checkTournamentFile(mParent->players[0] ==  PLAYER_TYPE_CPU))
            cnogmenu->Add(CNOGMENU_ITEM_CONTINUE_TOURNAMENT,"Continue Tournament","");
        cnogmenu->Add(CNOGMENU_ITEM_BESTOF3,"Best of 3","");
        cnogmenu->Add(CNOGMENU_ITEM_BESTOF5,"Best of 5","");
        cnogmenu->Add(CNOGMENU_ITEM_BESTOF7,"Best of 7","");
        cnogmenu->Add(CNOGMENU_ITEM_BESTOF13,"Best of 13","");
        cnogmenu->Add(CNOGMENU_ITEM_DELTA2,"2 wins difference","");
        cnogmenu->Add(CNOGMENU_ITEM_DELTA3,"3 wins difference","");
        cnogmenu->Add(CNOGMENU_ITEM_CONSECUTIVE2,"2 in a row","");
        cnogmenu->Add(CNOGMENU_ITEM_CONSECUTIVE3,"3 in a row","");
        if (mParent->players[0] ==  PLAYER_TYPE_CPU && mParent->players[1] ==  PLAYER_TYPE_CPU)
        {
            //games with fixed game lenght are boring for human player
            cnogmenu->Add(CNOGMENU_ITEM_3GAMES,"3 Games","");
            cnogmenu->Add(CNOGMENU_ITEM_5GAMES,"5 Games","");
            cnogmenu->Add(CNOGMENU_ITEM_10GAMES,"10 Games","");

            // i think nobody will play 25 games against the ai
            cnogmenu->Add(CNOGMENU_ITEM_25GAMES,"25 Games","");
            // i think nobody will play 50 games against the ai
            cnogmenu->Add(CNOGMENU_ITEM_50GAMES,"50 Games","");
            // i think nobody will play 100 games against the ai
            cnogmenu->Add(CNOGMENU_ITEM_100GAMES,"100 Games","");
            cnogmenu->Add(CNOGMENU_ITEM_BESTOF7PLAYOFF,"Best of 7 Playoff","");
        }
        cnogmenu->Add(CNOGMENU_ITEM_CANCEL,"Cancel","");
        setGamePhase(DUEL_STATE_CHOOSE_NUMBER_OF_GAMES);
        break;

      /* The next state displays the "Choose number of games" menu
        and waits for input. (PSY) */
    case DUEL_STATE_CHOOSE_NUMBER_OF_GAMES:
            cnogmenu->Update(dt);
        break;

    // The next state waits until cnogmenu is fully closed
    case DUEL_STATE_CNOGMENU_IS_CLOSING:
            if (cnogmenu->isClosed())
            {   // Is the closing animation finished?
                 setGamePhase(DUEL_STATE_CHOOSE_DECK1);
                 SAFE_DELETE(cnogmenu);

            }
            else cnogmenu->Update(dt);
        break;
     // The next state waits until cnogmenu is fully closed
     case DUEL_STATE_CONTINUE_TOURNAMENT:
              if (cnogmenu && cnogmenu->isClosed())
              {   // Is the closing animation finished?
                         // contruct and close opponentmenu for loading decknames
                         ConstructOpponentMenu();
                         opponentMenu->Close();

                         //setGamePhase(DUEL_STATE_PLAY);
                         SAFE_DELETE(cnogmenu);

              } else if (opponentMenu && opponentMenu->isClosed())
              {
                        // Is the closing animation finished then we can start to play!
                        setGamePhase(DUEL_STATE_PLAY);
                        SAFE_DELETE(opponentMenu);
              }
              else if (cnogmenu) cnogmenu->Update(dt);
              else if (opponentMenu) opponentMenu->Update(dt);
               break;

      // The next state occurs when the player selected "Cancel" in the cnogmenu
    case DUEL_STATE_CANCEL_CNOGMENU:
        cnogmenu->Update(dt);
        if (cnogmenu->isClosed())
        {
            // wait until menu has closed, then go back to main menu
            setGamePhase(DUEL_STATE_BACK_TO_MAIN_MENU);
            SAFE_DELETE(cnogmenu);
        }
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
                if (mParent->players[1] == PLAYER_TYPE_CPU && (mParent->gameType == GAME_TYPE_CLASSIC || mParent->gameType == GAME_TYPE_DEMO))
                {
                    if (deckmenu && (!popupScreen || popupScreen->isClosed())) deckmenu->Update(dt);
                }
                else
                {
                    game->loadPlayer(0, mParent->players[0]);
                    setGamePhase(DUEL_STATE_CHOOSE_DECK2);
                }
            }
        }
        break;
    case DUEL_STATE_CHOOSE_DECK1_TO_PLAY:
        if (deckmenu->isClosed())
        {
            setGamePhase(DUEL_STATE_PLAY);
            SAFE_DELETE(deckmenu);
        }
        else
            deckmenu->Update(dt);

        break;
    case DUEL_STATE_CHOOSE_DECK1_TO_2:
        if (deckmenu->isClosed())
            setGamePhase(DUEL_STATE_CHOOSE_DECK2);
        else
            deckmenu->Update(dt);
        break;
    case DUEL_STATE_CHOOSE_DECK2_TO_1:
        if (opponentMenu->isClosed())
        {
            setGamePhase(DUEL_STATE_CHOOSE_DECK1);
            SAFE_DELETE(opponentMenu);
        }
        else
            opponentMenu->Update(dt);
        break;
    case DUEL_STATE_CHOOSE_DECK2_TO_2:
        if (opponentMenu->isClosed())
        {
            setGamePhase(DUEL_STATE_CHOOSE_DECK2);
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
                if (mParent->players[1] == PLAYER_TYPE_CPU && (mParent->gameType == GAME_TYPE_CLASSIC || mParent->gameType == GAME_TYPE_DEMO))
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
            if (opponentMenu->isClosed())
            {
                setGamePhase(DUEL_STATE_PLAY);
                SAFE_DELETE(opponentMenu);
            }
            else
                opponentMenu->Update(dt);
        }
        break;
        // this called after each match and tournament
    case DUEL_STATE_SHOW_SCORE:
         if ((JGE_BTN_OK == mEngine->ReadButton()) ||  ( mParent->players[0] ==  PLAYER_TYPE_CPU && mParent->players[1] ==  PLAYER_TYPE_CPU && !(game && !game->didWin()) && !tournament->isMatchFinished()))
         {
           //check if Match and tournament is over
           if (game->didWin() && tournament->isMatchFinished() && !tournament->isNextDeckFound())
           {
               tournament->setScoreDisplayed(true);
               tournament->setOpLevel(OPLEVEL_NEW);
               tournament->End();
               //Display credits for human player
               if (mParent->players[0] ==  PLAYER_TYPE_HUMAN)
               {
                   tournament->save(false);
                   setGamePhase(DUEL_STATE_END);
               }
               else
               {
                   setGamePhase(DUEL_STATE_END_OF_TOURNAMENT);
               }

           }
           else
           { // we have to play again
               if (game->didWin())
                  tournament->setScoreDisplayed(true);
               setGamePhase(DUEL_STATE_PLAY);
           }
         }
         break;
     // The next state occurs after the *match* has ended:
     case DUEL_STATE_END_OF_MATCH:
        break;

      // The next state occurs after the end of the last match of the tournament:
     case DUEL_STATE_END_OF_TOURNAMENT:
        if ( (JGE_BTN_OK == mEngine->ReadButton()) || (mParent->players[0] ==  PLAYER_TYPE_CPU && mParent->players[1] ==  PLAYER_TYPE_CPU) )
        {
          setGamePhase(DUEL_STATE_BACK_TO_MAIN_MENU);
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

            if (!MusicExist(musictrack)) 
                musictrack = "ai_baka_music.mp3";

            GameApp::playMusic(musictrack);
            // init Score table
            if ( (mParent->gameType == GAME_TYPE_CLASSIC || mParent->gameType == GAME_TYPE_DEMO)&& mParent->players[1] == PLAYER_TYPE_CPU)
            {
                tournament->updateScoreTable(game->players[0], game->players[1], mParent->gameType);
                tournament->setScoreDisplayed(false);
            }
        }
        game->Update(dt);
        //run a "post update" init call in the rules. This is for things such as Manapool, which gets emptied in the update
        // That's mostly because of a legacy bug, where we use the update sequence for some things when we should use events (such as phase changes)
        mParent->rules->postUpdateInit(game);

#ifdef NETWORK_SUPPORT
        if(mParent->mpNetwork) ((NetworkGameObserver*)game)->synchronize();
#endif

        if (game->didWin())
        {
            //the following section will be called only in a classic or demo gamemode and if a tournament or match with more then one game is activ
             if ( (mParent->gameType == GAME_TYPE_CLASSIC || mParent->gameType == GAME_TYPE_DEMO)&& mParent->players[1] == PLAYER_TYPE_CPU && (tournament->isTournament() || tournament->getGamesToPlay()>1 ))
             {
                 setGamePhase(DUEL_STATE_SHOW_SCORE);
                /* Determine the winner of this game.
                      This could be done more elegantly in credits.cpp (PSY) */
                    // (PSY!) check how the game handles draws, does it think it has a winner?
                 if (!tournament->wasScoreDisplayed() || mParent->players[0]==PLAYER_TYPE_CPU)
                 {
                    tournament->gameFinished(game->didWin(game->players[0]),game->didWin(game->players[1]));
                    tournament->updateScoreTable(game->players[0], game->players[1], mParent->gameType);
                    if (tournament->isMatchFinished())
                    {
                         // Hack for the competition: swap players back if swapped
                         tournament->revertSwapPlayer();
                         //save match results in ai_test.csv only for ai vs ai games
                         if (mParent->players[0]==PLAYER_TYPE_CPU)
                              tournament->saveMatchResults();

                         if (tournament->isTournament())
                         {  // We are in Tournament mode
                           tournament->updateTournament();
                           // if first player is human then other results are random
                           // It would be to boring for human player to watch all other ai decks to finish
                           if (mParent->players[0]==PLAYER_TYPE_HUMAN)
                               tournament->leaveOutAIvsAIMatches();
                           tournament->calculateRanking();
                           tournament->updateScoreforTournament();
                           if (tournament->isNextDeckFound())
                               // first player is either human or cpu
                               mParent->players[0] = tournament->getDeckType(0);
                         }
                     }

                     if (!tournament->isNextDeckFound() && tournament->isMatchFinished())
                     {
                         //save tournament status
                         tournament->save(mParent->players[0]==PLAYER_TYPE_CPU);
                         if (mParent->players[0]==PLAYER_TYPE_HUMAN)
                         {
                             credits->computeTournament(game, mParent,tournament->isTournament(),!tournament->isTournament(),tournament->didHumanWin(),tournament->gamesWonbyHuman(),tournament->gamesPlayedbyHuman(),tournament->matchesWonbyHuman(),tournament->matchesPlayedbyHuman());
                         }
                     }
                 }

                 if (tournament->wasScoreDisplayed() || mParent->players[0]==PLAYER_TYPE_CPU)
                 {
                     tournament->setScoreDisplayed(false);
                    // load next deck if actual match is not finished or a next match is available
                    if (tournament->isNextDeckFound() || !tournament->isMatchFinished())
                    {
                        tournament->setOpLevel(OPLEVEL_NEXT_GAME);
                        End();
                        Start();
                        tournament->setOpLevel(OPLEVEL_GAME_ACTIV);
                        // Hack for the competition: swap players (human must be on deck[0])
                        tournament->swapPlayer();
                        for (int i = 0; i < NMB_PLAYERS; i++)
                        {
                                game->loadPlayer(i, tournament->getDeckType(i), tournament->getDeckNumber(i), premadeDeck);
                        }
                        //saves actual tournament state
                        tournament->save(mParent->players[0]==PLAYER_TYPE_CPU);
                        setAISpeed();
                        setGamePhase(DUEL_STATE_PLAY);

                    }
                }


            }
            else
            {
                if (game->players[1]->playMode != Player::MODE_TEST_SUITE)   credits->compute(game, mParent);
                setGamePhase(DUEL_STATE_END);
            }
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

#else
                        thread_count = 4;
#endif
                        for(size_t i = 0; i < (thread_count); i++)
                            mWorkerThread.push_back(boost::thread(ThreadProc, this));
                    }
                }
            }
#endif
//            if (mParent->players[0] == PLAYER_TYPE_CPU && mParent->players[1] == PLAYER_TYPE_CPU)
//            {
/////// Tournament Mod ///////////
//                End();
//                Start();
/////// END Tournament Mod ///////////
//            }
            //this is handled elsewhere.
        }
        if (mEngine->GetButtonClick(JGE_BTN_MENU))
        {
            if (!menu)
            {
                menu = NEW SimpleMenu(JGE::GetInstance(), DUEL_MENU_GAME_MENU, this, Fonts::MENU_FONT, SCREEN_WIDTH / 2 - 100, 25);
                int cardsinhand = game->currentPlayer->game->hand->nb_cards;

                //almosthumane - mulligan
                if ((game->turn < 1) && (cardsinhand != 0) && game->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN
                    && game->currentPlayer->game->inPlay->nb_cards == 0 && game->currentPlayer->game->graveyard->nb_cards == 0
                    && game->currentPlayer->game->exile->nb_cards == 0 && game->currentlyActing() == (Player*)game->currentPlayer) //1st Play Check
                    //IF there was no play at the moment automatically mulligan
                {
                    menu->Add(MENUITEM_MULLIGAN, "Mulligan");
                }
                //END almosthumane - mulligan
                menu->Add(MENUITEM_MAIN_MENU, "Back to main menu");
#ifdef TESTSUITE
                menu->Add(MENUITEM_UNDO, "Undo");
#endif
#ifdef TESTSUITE
                menu->Add(MENUITEM_LOAD, "Load");
#endif

                if (mParent->players[1] == PLAYER_TYPE_CPU && (mParent->gameType == GAME_TYPE_CLASSIC || mParent->gameType == GAME_TYPE_DEMO))
                {
                    menu->Add(MENUITEM_SHOW_SCORE, "Show current score");
                    if (mParent->players[0] == PLAYER_TYPE_CPU)
                    {
                        if (tournament->getFastTimerMode())
                            menu->Add(MENUITEM_SPEED_NORMAL, "set Speed to NORMAL");
                        else
                            menu->Add(MENUITEM_SPEED_FAST, "set Speed to FAST");

                    }
                }
                menu->Add(MENUITEM_CANCEL, "Cancel");
            }
            setGamePhase(DUEL_STATE_MENU);
        }
        break;
#ifdef NETWORK_SUPPORT
    case DUEL_STATE_OPPONENT_WAIT:
        {
            if(game->players.size() > 1)
            { // Player loaded
                menu->Close();
                SAFE_DELETE(menu);
                setGamePhase(DUEL_STATE_PLAY);
            } else if(menu == NULL)
            {
                menu = NEW SimpleMenu(JGE::GetInstance(), DUEL_STATE_OPPONENT_WAIT, this, Fonts::MENU_FONT, 150, 60);
                if (menu)
                {
                    menu->Add(MENUITEM_MAIN_MENU, "Back to main menu");
                }
            } else
            {
                menu->Update(dt);
                game->Update(dt);
            }
        }
        break;
#endif //NETWORK_SUPPORT
    case DUEL_STATE_MENU:
        menu->Update(dt);
        break;
    case DUEL_STATE_MENU_TO_SCORE:
        menu->Update(dt);
        if (menu->isClosed())
        {
            setGamePhase(DUEL_STATE_SHOW_SCORE);
            SAFE_DELETE(menu);
        }
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
            tournament->setOpLevel(OPLEVEL_NEW);
            tournament->End();
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

    //render the game until someone did win the game (otherwise it crashes sometimes under linux)
    if (game && !game->didWin())
        game->Render();

#ifdef AI_CHANGE_TESTING
    if (game && totalTestGames)
    {
        char buf[4096];
        int currentTime = JGEGetTime();

        if (totalTestGames < 2.5 * totalAIDecks)
        {
            mFont->SetColor(ARGB(255,255,255,0));
            sprintf(buf, "           Results are not significant, you should let at least %i more games run", (int)(totalAIDecks * 2.5) - totalTestGames);
            mFont->DrawString(buf,0,SCREEN_HEIGHT/2 - 20);
        }

        mFont->SetColor(ARGB(255,255,255,255));
        float ratio = float(testPlayer2Victories) / float(totalTestGames);
        if (ratio < 0.48)
            mFont->SetColor(ARGB(255,255,0,0));
        if (ratio > 0.52)
            mFont->SetColor(ARGB(255,0,255,0));
        sprintf(buf, "              Victories Player 2/total Games: %i/%i - Game Turn: %i",
            testPlayer2Victories, totalTestGames, /*(float)(1000*totalTestGames)/(currentTime - startTime)*/game->turn);
        mFont->DrawString(buf,0,SCREEN_HEIGHT/2);
    }
#endif

    switch (mGamePhase)
    {
   case DUEL_STATE_START:
   break;

    case DUEL_STATE_SHOW_SCORE:
    {
         r->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(200,0,0,0));
         tournament->renderScoreTable();
         break;
    }
    case DUEL_STATE_END_OF_GAME:
        break;
    case DUEL_STATE_END_OF_MATCH:
        break;
    case DUEL_STATE_END_OF_TOURNAMENT:
        break;
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
     case DUEL_STATE_PREPARE_CNOGMENU:
     case DUEL_STATE_CHOOSE_NUMBER_OF_GAMES:
     case DUEL_STATE_CNOGMENU_IS_CLOSING:
     case DUEL_STATE_CANCEL_CNOGMENU:
        if (cnogmenu)
            cnogmenu->Render();
        break;

    case DUEL_STATE_CHOOSE_DECK1:
    case DUEL_STATE_CHOOSE_DECK1_TO_2:
    case DUEL_STATE_CHOOSE_DECK1_TO_PLAY:
    case DUEL_STATE_CHOOSE_DECK2:
    case DUEL_STATE_CHOOSE_DECK2_TO_2:
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
            {
                opponentMenu->Render();
                // display the selected player deck name too
                string selectedPlayerDeckName = _("Player Deck: ").c_str() + game->players[0]->deckName;
                mFont->DrawString( selectedPlayerDeckName.c_str(), 30, 40);
            }
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
    case DUEL_STATE_MENU_TO_SCORE:
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
        {
            menu->Render();

            // display the player deck names in their respective corners
            string playerDeckName =  game->players[0]->deckName;
            float playerDeckNamePixelLength = mFont->GetStringWidth(playerDeckName.c_str());
            mFont->DrawString( playerDeckName, SCREEN_WIDTH_F - playerDeckNamePixelLength, SCREEN_HEIGHT_F - 50);
            if(game->players.size()>1)
            {
                string opponentDeckName = game->players[1]->deckName;
                mFont->DrawString( opponentDeckName, 0, 50);
            }
        }
    }
}

void GameStateDuel::ButtonPressed(int controllerId, int controlId)
{
    int deckNumber = controlId;
    DeckManager * deckManager = DeckManager::GetInstance();
    int aiDeckSize = deckManager->getAIDeckOrderList()->size();
    switch (controllerId)
    {

        case DUEL_MENU_CHOOSE_NUMBER_OF_GAMES:  // We're in the "Choose number of Games" menu
        {
          switch (controlId)
          {
              case CNOGMENU_ITEM_SINGLE_GAME:
                tournament->setMatchType(1,MATCHMODE_FIXED);break;
              case CNOGMENU_ITEM_CONTINUE_TOURNAMENT:
                  if (cnogmenu) cnogmenu->Close();
                  if (!tournament->load(mParent->players[0] ==  PLAYER_TYPE_CPU,true))
                  {
                      setGamePhase(DUEL_STATE_CANCEL_CNOGMENU);
                      return;
                  }
                  tournament->calculateRanking();
                  tournament->updateScoreforTournament();

                  if (tournament->isNextDeckFound() || !tournament->isMatchFinished())
                  {
                          tournament->setOpLevel(OPLEVEL_GAME_ACTIV);


                          mParent->players[0] = tournament->getDeckType(0);
                          mParent->players[1] = tournament->getDeckType(1);
                          for (int i = 0; i < NMB_PLAYERS; i++)
                          {
                             game->loadPlayer(i, tournament->getDeckType(i),tournament->getDeckNumber(i) , premadeDeck);
                          }
                          setAISpeed();
                          setGamePhase(DUEL_STATE_CONTINUE_TOURNAMENT);
                          return;

                  }
                  else
                  { //end of match or end of tournament was already reached, the tournament cannot be continued
                          setGamePhase(DUEL_STATE_CANCEL_CNOGMENU);
                          return;

                   }
                   break;
              case CNOGMENU_ITEM_BESTOF3:
                tournament->setMatchType(3,MATCHMODE_BESTOF); break;
              case CNOGMENU_ITEM_BESTOF5:
                tournament->setMatchType(5,MATCHMODE_BESTOF); break;
              case CNOGMENU_ITEM_BESTOF7:
                tournament->setMatchType(7,MATCHMODE_BESTOF); break;
              case CNOGMENU_ITEM_BESTOF13:
                tournament->setMatchType(13,MATCHMODE_BESTOF); break;
              case CNOGMENU_ITEM_DELTA2:
                tournament->setMatchType(2,MATCHMODE_DELTA); break;
              case CNOGMENU_ITEM_DELTA3:
                tournament->setMatchType(3,MATCHMODE_DELTA); break;
              case CNOGMENU_ITEM_CONSECUTIVE2:
                tournament->setMatchType(2,MATCHMODE_CONSECUTIVE); break;
              case CNOGMENU_ITEM_CONSECUTIVE3:
                tournament->setMatchType(3,MATCHMODE_CONSECUTIVE); break;
              case CNOGMENU_ITEM_3GAMES:
                tournament->setMatchType(3,MATCHMODE_FIXED); break;
              case CNOGMENU_ITEM_5GAMES:
                tournament->setMatchType(5,MATCHMODE_FIXED); break;
              case CNOGMENU_ITEM_10GAMES:
                tournament->setMatchType(10,MATCHMODE_FIXED); break;
              case CNOGMENU_ITEM_25GAMES:
                tournament->setMatchType(25,MATCHMODE_FIXED); break;
              case CNOGMENU_ITEM_50GAMES:
                tournament->setMatchType(50,MATCHMODE_FIXED); break;
              case CNOGMENU_ITEM_100GAMES:
                tournament->setMatchType(100,MATCHMODE_FIXED); break;
              case CNOGMENU_ITEM_BESTOF7PLAYOFF:
                tournament->setMatchType(7,MATCHMODE_BESTOF,true); break;
              case CNOGMENU_ITEM_CANCEL:
                cnogmenu->Close();
                setGamePhase(DUEL_STATE_CANCEL_CNOGMENU);

                return;
          }
        }
        cnogmenu->Close();
        setGamePhase(DUEL_STATE_CNOGMENU_IS_CLOSING);
        break;
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
            tournament->addDeck(1,game->players.at(1)->deckId,mParent->players[1]);
            setAISpeed();
            if (opponentMenu) opponentMenu->Close();
            if (tournamentSelection)
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_2);
            else
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);
            break;
        case MENUITEM_RANDOM_AI_HARD:
            {
                int deck=tournament->getRandomDeck(true);
                if (deck>0)
                {
                    game->loadPlayer(1, mParent->players[1], deck, premadeDeck);
                    tournament->addDeck(1,game->players.at(1)->deckId,mParent->players[1]);
                }
            }
            setAISpeed();
            if (opponentMenu) opponentMenu->Close();
            if (tournamentSelection)
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_2);
            else
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);

            break;
         case MENUITEM_FILL_NEXT_STAGE:
         case MENUITEM_FILL_NEXT_STAGE_HARD:
             {
                int deck=0;
                do
                {
                    deck=tournament->getRandomDeck(controlId==MENUITEM_FILL_NEXT_STAGE_HARD);
                    if (deck>0)
                    {
                        game->loadPlayer(1, mParent->players[1], deck, premadeDeck);
                        tournament->addDeck(1,game->players.at(1)->deckId,mParent->players[1]);
                    }
                }
                while(tournament->remainingDecksToNextStage()>0 && deck>0);
             }
             setAISpeed();
             if (opponentMenu) opponentMenu->Close();
             if (tournamentSelection)
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_2);
             else
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);

            break;
        default:
            // cancel option.  return to player deck selection

            if (controlId == MENUITEM_CANCEL)
            {
                if (opponentMenu) opponentMenu->Close();
                if (deckmenu) deckmenu->Close();
                //reset tournament
                tournamentSelection=false;
                tournament->resetTournamentSelection();
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
            else if (controlId == MENUITEM_GAUNTLET)
            {
                tournament->enableTournamantMode(TOURNAMENTMODES_GAUNTLET,1);
                deckNumber = 1;// start Gauntlet with first deck
            }
            else if (controlId == MENUITEM_KO_TOURNAMENT)
            {
                tournamentSelection=true;
                tournament->enableTournamantMode(TOURNAMENTMODES_KO,1);
                if (opponentMenu) opponentMenu->Close();
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_2);
                break;

            }
            else if (controlId == MENUITEM_DOUBLEKO_TOURNAMENT)
            {
                tournamentSelection=true;
                tournament->enableTournamantMode(TOURNAMENTMODES_DOUBLEKO,1);
                if (opponentMenu) opponentMenu->Close();
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_2);
                break;

            }
            else if (controlId == MENUITEM_RR_TOURNAMENT)
            {
                tournamentSelection=true;
                tournament->enableTournamantMode(TOURNAMENTMODES_ROUNDROBIN);

                if (opponentMenu) opponentMenu->Close();
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_2);
                break;
            }
            else if (controlId == MENUITEM_START_TOURNAMENT)
            {
                if (tournament->getNumberofTournamentDecks()>2)
                {
                    tournamentSelection=false;
                    tournament->initTournament();
                    game->loadPlayer(1, mParent->players[1], tournament->getDeckNumber(1), premadeDeck);
                    setAISpeed();
                    if (opponentMenu) opponentMenu->Close();
                    setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);
                }
                else
                {
                    if (opponentMenu) opponentMenu->Close();
                    setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_2);
                }
                break;

            }
            else if (tournamentSelection)
            {
                deckNumber = deckManager->getAIDeckOrderList()->at(controlId - 1)->getDeckId();
                OpponentsDeckid = deckNumber;
                tournament->addDeck(1,deckNumber,mParent->players[1]);
                if (opponentMenu) opponentMenu->Close();
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_2);
                break;
            }
            else if (controlId != MENUITEM_EVIL_TWIN && aiDeckSize > 0) // evil twin
                deckNumber = deckManager->getAIDeckOrderList()->at(controlId - 1)->getDeckId();
            game->loadPlayer(1, mParent->players[1], deckNumber, premadeDeck);
            tournament->addDeck(1,deckNumber,mParent->players[1]);
            setAISpeed();
            OpponentsDeckid = deckNumber;
            if (opponentMenu) opponentMenu->Close();
            setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);
            break;
        }
        break;

    case DUEL_MENU_CHOOSE_DECK:

        if (controlId == MENUITEM_RANDOM_PLAYER) // Random Player Deck Selection
        {
            if (mParent->players[0] ==  PLAYER_TYPE_HUMAN)
                deckmenu->selectRandomDeck(false);
            else
                deckmenu->selectRandomDeck(true);
            tournament->addDeck(0,deckmenu->getSelectedDeckId(),mParent->players[0]);
            game->loadPlayer(0, mParent->players[0], deckmenu->getSelectedDeckId(), premadeDeck);
            if (deckmenu) deckmenu->Close();
            ConstructOpponentMenu();
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
        else if (controlId == MENUITEM_GAUNTLET)
        {
            tournament->enableTournamantMode(TOURNAMENTMODES_GAUNTLET,0);
            controlId = 1;
        }
        else if (controlId == MENUITEM_ENDLESSDEMO)
        {
            tournament->enableTournamantMode(TOURNAMENTMODES_ENDLESS);

            game->loadPlayer(0, mParent->players[0], tournament->getDeckNumber(0), premadeDeck);
            setAISpeed();
            setGamePhase(DUEL_STATE_CHOOSE_DECK1_TO_PLAY);
            controlId = tournament->getDeckNumber(1);
        }
        else if (controlId == MENUITEM_KO_TOURNAMENT)
        {
            tournament->enableTournamantMode(TOURNAMENTMODES_KO);

            game->loadPlayer(0, mParent->players[0], tournament->getDeckNumber(0), premadeDeck);
            setAISpeed();
            setGamePhase(DUEL_STATE_CHOOSE_DECK1_TO_PLAY);
            controlId = tournament->getDeckNumber(1);
        }
        else if (controlId == MENUITEM_DOUBLEKO_TOURNAMENT)
        {
            tournament->enableTournamantMode(TOURNAMENTMODES_DOUBLEKO);

            game->loadPlayer(0, mParent->players[0], tournament->getDeckNumber(0), premadeDeck);
            setAISpeed();
            setGamePhase(DUEL_STATE_CHOOSE_DECK1_TO_PLAY);
            controlId = tournament->getDeckNumber(1);
        }
        if (controlId < 0)
        {
            mParent->SetNextState(GAME_STATE_DECK_VIEWER);
            return;
        }
        if (mGamePhase == DUEL_STATE_CHOOSE_DECK1)
        {
            if (mParent->players[0] ==  PLAYER_TYPE_HUMAN)
            {
                vector<DeckMetaData *> * playerDeck = deckManager->getPlayerDeckOrderList();
                if (!premadeDeck && controlId > 0)
                    deckNumber = playerDeck->at(controlId - 1)->getDeckId();
                game->loadPlayer(0, mParent->players[0], deckNumber, premadeDeck);
                tournament->addDeck(0,deckNumber,mParent->players[0]);
                playerDeck = NULL;
            }
            else
            { //demo first player is CPU
                if (tournament->isGauntlet() || tournament->getTournamentMode() == TOURNAMENTMODES_KO || tournament->getTournamentMode() == TOURNAMENTMODES_DOUBLEKO)
                    // start Gauntlet with first deck
                    deckNumber = tournament->getDeckNumber(0);
                else
                {
                    deckNumber = deckManager->getAIDeckOrderList()->at(controlId - 1)->getDeckId();
                    tournament->addDeck(0,deckNumber,mParent->players[0]);
                }
                game->loadPlayer(0, mParent->players[0], deckNumber, premadeDeck);

             }
            if (deckmenu) deckmenu->Close();
#ifdef NETWORK_SUPPORT
            if(mParent->mpNetwork)
            {   // no need to choose an opponent deck in network mode
                setGamePhase(DUEL_STATE_OPPONENT_WAIT);
            }
            else
#endif //NETWORK_SUPPORT
            {
                setGamePhase(DUEL_STATE_CHOOSE_DECK1_TO_2);
            }
        }
        else
        {
            game->loadPlayer(1, mParent->players[1], controlId, premadeDeck);
            if (deckmenu) deckmenu->Close();
            if (!tournament->isEndlessDemo() && tournament->getTournamentMode()!=TOURNAMENTMODES_KO && tournament->getTournamentMode()!=TOURNAMENTMODES_DOUBLEKO) //  no oppenent has to be selected for these modes
                setGamePhase(DUEL_STATE_CHOOSE_DECK2_TO_PLAY);
        }
        break;

    default:
        switch (controlId)
        {
//        case GAMEMENU_ITEM_CONCEDE_MATCH:
//          // This is a hack for 1-Player mode, it won't work in 2-player mode:
//          if (mParent->players[0] ==  PLAYER_TYPE_HUMAN) {
//            mVictories[0] = 0;
//            mVictories[1] = mNbGames;
//            mGamesPlayed = mNbGames;
//          }
//          // falling through
//        case GAMEMENU_ITEM_CONCEDE_GAME:
//          // This is a hack for 1-Player mode, it won't work in 2-player mode:
//          if (mParent->players[0] ==  PLAYER_TYPE_HUMAN) {
//            game->players[0]->life = -99;
//          }
//          // falling through
//        case GAMEMENU_ITEM_CONTINUE:
//          menu->Close();
//          mGamePhase = DUEL_STATE_CONTINUE;
//          break;
        case MENUITEM_SPEED_FAST:
            tournament->setFastTimerMode(true);
            setAISpeed();
            menu->Close();
            setGamePhase(DUEL_STATE_CANCEL);
            break;
        case MENUITEM_SPEED_NORMAL:
            tournament->setFastTimerMode(false);
            setAISpeed();
            menu->Close();
            setGamePhase(DUEL_STATE_CANCEL);
            break;
        case MENUITEM_SHOW_SCORE:
            menu->Close();
            //delete menu and display score
            setGamePhase(DUEL_STATE_MENU_TO_SCORE);
            break;
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
        /*bool flickUpwards = (inYVelocity < 0);*/
        mEngine->HoldKey_NoRepeat(/*flickUpwards ? JGE_BTN_PREV : */JGE_BTN_SEC);
        //removing the activation of "left trigger" or "advance phase" because on high end tablets this gesture
        //is picked up by simply looking at the cards in your hand if the resolution of tablet exceed 1800 X ANY.
    }
    if (abs(inXVelocity) > 300)
    {
        bool flickLeft = (inYVelocity > 0);
        if(flickLeft)
        {
            JButton trigger = (options[Options::REVERSETRIGGERS].number ? JGE_BTN_PREV : JGE_BTN_NEXT);
            mEngine->HoldKey_NoRepeat(trigger);
        }
    }
}


void GameStateDuel::setAISpeed()
{
  for (int i = 0; i<NMB_PLAYERS; i ++)
  {
    if (mParent->players[i] ==  PLAYER_TYPE_CPU)
    {
      if(game)
        ((AIPlayer *)game->players[i])->setFastTimerMode(tournament->getFastTimerMode());
    }
  }
}



TDeck::TDeck(int deck,PlayerType deckType, int victories, int lastWin, int wonMatches, int playedMatches,int wonGames,int playedGames)
    :mDeckNumber(deck),mDeckType(deckType),mVictories(victories),mLastWin(lastWin),mWonMatches(wonMatches),mMatchesPlayed(playedMatches),mWonGames(wonGames),mGamesPlayed(playedGames)
{
    mRanking=-1;
    mDifficulty=0;
}

TDeck::TDeck()
{
    reset();
}

void TDeck::reset()
{
    mDeckNumber=0;
    mDeckType=PLAYER_TYPE_CPU;
    mVictories=0;
    mLastWin=0;
    mWonMatches=0;
    mGamesPlayed=0;
    mMatchesPlayed=0;
    mRanking=-1;
    mWonGames=0;
    mDifficulty=0;
}


TDeck::~TDeck()
{


}

void TDeck::increaseDeckNumber()
{
    mDeckNumber++;
}

std::string TDeck::getDeckName()
{
    DeckManager * deckManager = DeckManager::GetInstance();
    return deckManager->getDeckMetaDataById(getDeckNumber(), isAI())->getName();
}

Tournament::Tournament()
{

    for (int i = 0; i<NMB_PLAYERS; i ++)
    {
      Deck[i].reset();
      gauntletLastDeckNumber[i]=0;
      TournamentsDecksID[i]=0;
    }

    mTournamentMode=0;
    mCompetition = false;
    mPlayerSwaped = false;
    mNbGames = 0;
    mMatchMode = MATCHMODE_FIXED;
    mGamesPlayed = 0;
    mOpLevel = OPLEVEL_NEW;
    nmbDecks = 0;
    //fastTimermode is set to false at startup
    mFastTimerMode=false;
    endOfMatch=false;
    nextDeckFound=false;
    scoreDisplayed=false;
    //for scoretable
    mVictories0=0;
    mVictories1=0;
    p0IsAI=false;
    p1IsAI=false;
    scoreFinal=false;
    tournamentFinal=false;

    for (int j=0;j<10;j++)
        sortedTournamentDecks[j].reset();
}

Tournament::~Tournament()
{
    End();
}

void Tournament::initTournamentResults()
{
    for (int j=0;j<10;j++)
        sortedTournamentDecks[j].reset();
    for (int j=0;j<10;j++)
        sortedTournamentDecks[j].setMatchesWon(-1);

}



int Tournament::getRandomDeck(bool noEasyDecks)
{
    DeckManager *deckManager = DeckManager::GetInstance();
    vector<DeckMetaData *> *deckList =  deckManager->getAIDeckOrderList();

    int deckNumber;
    unsigned int random=0;
    int k=0;
    bool isDouble=true;
    vector<unsigned int> decks;
    for (unsigned int i=0;i<deckList->size();i++)
        if (noEasyDecks && (deckList->at(i)->getDifficulty()==HARD || deckList->at(i)->getDifficulty()==NORMAL))
        {
            decks.push_back(i);
            //printf("hard deck%i/%i\n",i,deckList->size());
        } else
            decks.push_back(i);


    while(isDouble && decks.size()>0)
    {
        isDouble=false;
        k = rand() % (int)decks.size();
        random = decks.at(k);

        deckNumber = deckList->at(random)->getDeckId();
        if (  mTournamentMode==TOURNAMENTMODES_ROUNDROBIN || mTournamentMode==TOURNAMENTMODES_KO||mTournamentMode==TOURNAMENTMODES_DOUBLEKO)
            for (unsigned int i=0;i<TournamentsDecks.size();i++)
                if (TournamentsDecks.at(i).getDeckNumber()==deckNumber && TournamentsDecks.at(i).getDeckType()==0)
                {
                    isDouble=true;
                    for (unsigned int j=k;j<decks.size()-1;j++)
                        decks[j]=decks[j+1];
                    decks.pop_back();
                }
    };

    if (decks.size()==0)
        return -1;
    else
        return deckNumber;
}

void Tournament::addDeck(int player,int deckNumber,PlayerType deckType)
{

    if (mTournamentMode==0 || mTournamentMode==TOURNAMENTMODES_GAUNTLET)
    {
        Deck[player].setDeckNumber(deckNumber);
        Deck[player].setDeckType(deckType);
    }
    else if (  mTournamentMode==TOURNAMENTMODES_ROUNDROBIN || mTournamentMode==TOURNAMENTMODES_KO||mTournamentMode==TOURNAMENTMODES_DOUBLEKO)
    {
        //check for double deck entries
        bool isDouble=false;
        for (unsigned int i=0;i<TournamentsDecks.size();i++)
            if (TournamentsDecks.at(i).getDeckNumber()==deckNumber && TournamentsDecks.at(i).getDeckType()==deckType)
                isDouble=true;
        if (!isDouble)
            TournamentsDecks.push_back(TDeck(deckNumber,deckType));

        updateScoreforTournament();

    }

}

void Tournament::addDeck(int player,TDeck newDeck)
{

    if (mTournamentMode==0 || mTournamentMode==TOURNAMENTMODES_GAUNTLET)
    {
        Deck[player]=newDeck;
    }
    else if (mTournamentMode==TOURNAMENTMODES_KO || mTournamentMode==TOURNAMENTMODES_DOUBLEKO || mTournamentMode==TOURNAMENTMODES_ROUNDROBIN)
    {
        //check for double deck entries
        bool isDouble=false;
        for (unsigned int i=0;i<TournamentsDecks.size();i++)
            if (TournamentsDecks.at(i).getDeckNumber()==newDeck.getDeckNumber() && TournamentsDecks.at(i).getDeckType()==newDeck.getDeckType())
                isDouble=true;
        if (!isDouble)
            TournamentsDecks.push_back(newDeck);

        updateScoreforTournament();
    }
}



void Tournament::Start()
{
    if (mOpLevel < OPLEVEL_NEXT_GAME )
    {  // only reset "Played Games" and "Victories" info if we didn't come here from within a match
      mGamesPlayed = 0;
      //mPlayerSwaped = false;
      for (int i = 0; i<NMB_PLAYERS; i ++)
      {
          Deck[i].newMatch();
      }
      scoreDisplayed=false;
    }

    if (mOpLevel == OPLEVEL_NEW)
    {  // only reset "Gauntlet" and "number of players" info if we didn't come here from within a Gauntlet tournament
      mPlayerSwaped = false;
      mNbGames = 0;
      mMatchMode = 0;
      mTournamentMode=0;
      scoreDisplayed=false;
      mGamesPlayed = 0;
      for (int i = 0; i<NMB_PLAYERS; i ++)
      {
        Deck[i].reset();
        gauntletLastDeckNumber[i]=0;
      }
    }
}

void Tournament::End()
{

    if (mOpLevel < OPLEVEL_NEXT_GAME)
    {  // only reset Players if we didn't come here from within a Match
        mGamesPlayed = 0;
        //mPlayerSwaped=false;
        for (int i = 0; i < NMB_PLAYERS; i++)
        {
          Deck[i].newMatch();
        }
      }

      if (mOpLevel == OPLEVEL_NEW)
      {  // only reset tournament info if we didn't come here from within a  tournament
        mCompetition = false;
        mPlayerSwaped=false;
        mTournamentMode=0;
        mNbGames = 0;
        mMatchMode = 0;
        mGamesPlayed = 0;
        for (int i = 0; i < NMB_PLAYERS; i++)
        {
          Deck[i].reset();
          gauntletLastDeckNumber[i]=0;
        }
        TournamentsDecks.clear();
        mGamesPlayed = 0;
        scoreMatchesToPlay=0;
        scoreMatchesPlayed=0;
      }

}

bool Tournament::gameFinished(bool player0didWin,bool player1didWin)
{
    if (player0didWin)
    {
      Deck[0].winGame();
      Deck[1].looseGame();
      mGamesPlayed++;
    }
    else if (player1didWin)
    {
      Deck[1].winGame();
      Deck[0].looseGame();
      mGamesPlayed++;
    }

    endOfMatch = false;
    switch (mMatchMode)
    {
      case MATCHMODE_FIXED:
        if (mGamesPlayed >= mNbGames)
          endOfMatch = true;
        break;
      case MATCHMODE_BESTOF:
        if ((Deck[0].getVictories() > mNbGames/2 || Deck[1].getVictories() > mNbGames/2) && (!mCompetition || abs(Deck[0].getVictories() - Deck[1].getVictories()) >1)) endOfMatch = true;
        break;
      case MATCHMODE_DELTA:
        if ((Deck[0].getVictories() - Deck[1].getVictories() >= mNbGames) || (Deck[1].getVictories() - Deck[0].getVictories() >= mNbGames)) endOfMatch = true;
        break;
      case MATCHMODE_CONSECUTIVE:
        if (Deck[0].getLastWin() >= mNbGames || Deck[1].getLastWin() >= mNbGames) endOfMatch = true;
        break;
      default:
        endOfMatch = true;
    }


    return endOfMatch;
}

void Tournament::saveMatchResults()
{
    std::ofstream file;
     if (JFileSystem::GetInstance()->openForWrite(file,AI_RESULTS_FILE,ios_base::app))
     {
         time_t rawtime;
         struct tm * timeinfo;
         char timestamp [80];
         time ( &rawtime );
         timeinfo = localtime ( &rawtime );
         strftime (timestamp,20,"%m/%d/%y, %H:%M:%S", timeinfo);
         int deck0 = 0;
         if (Deck[0].isAI())
             deck0 = Deck[0].getDeckNumber();
         else
             deck0 = -Deck[0].getDeckNumber();

         int deck1 = 0;
         if (Deck[1].isAI())
             deck1 = Deck[1].getDeckNumber();
         else
             deck1 = -Deck[1].getDeckNumber();

         int temp = 0; if (mCompetition) temp = 1;
         char buf[256];
         sprintf(buf, "%s,% 6i,% 6i,% 5i,% 5i,% 5i,% 1i,% 5i,% 1i\n", timestamp, deck0, deck1, Deck[0].getVictories(), Deck[1].getVictories(), mGamesPlayed, mMatchMode, mNbGames, temp);
         file << buf;
         file.close();
    }
}
void Tournament::resetTournamentSelection()
{
    mTournamentMode=0;
    TournamentsDecks.clear();
    TournamentsDecks.push_back(Deck[0]);
    TournamentsDecksID[0]=0;
    TournamentsDecksID[1]=1;

}

std::string Tournament::exportTournamentDescription()
{
    std::string describtion;
    char buf[1024];
    switch(mTournamentMode)
    {
        case TOURNAMENTMODES_KO:
        sprintf(buf, "knockout tournament!:\n");
        describtion.append(buf);
            break;
        case TOURNAMENTMODES_DOUBLEKO:
            sprintf(buf, "double knockout tournament!:\n");
            describtion.append(buf);
            break;
        case TOURNAMENTMODES_ROUNDROBIN:
            sprintf(buf, "round robin tournament!:\n");
            describtion.append(buf);
            break;
    }

    sprintf(buf, "%i Decks selected:\n",(int)TournamentsDecks.size()-1);
    describtion.append(buf);
    for (unsigned int i=1;i<TournamentsDecks.size();i++)
    {
        sprintf(buf, "%s, ",TournamentsDecks.at(i).getDeckName().c_str());
        describtion.append(buf);

    }
    return describtion;

}

void Tournament::enableTournamantMode(int tournamentMode,int player)
{
    if (mTournamentMode!=tournamentMode && tournamentMode>0)
    {

        mTournamentMode=tournamentMode;
        // the first two decks are the first playing decks
        TournamentsDecksID[0]=0;
        TournamentsDecksID[1]=1;
        switch(mTournamentMode)
        {
            case TOURNAMENTMODES_ENDLESS:
                if (nmbDecks>1)
                {
                    Deck[0].setDeckNumber((rand() % nmbDecks) + 1);
                    Deck[1].setDeckNumber((rand() % nmbDecks) + 1);
                }
                else
                {
                    Deck[0].setDeckNumber(1);
                    Deck[1].setDeckNumber(1);
                }
                Deck[0].setDeckType(PLAYER_TYPE_CPU);
                Deck[1].setDeckType(PLAYER_TYPE_CPU);
                nextDeckFound = true;
                scoreMatchesPlayed=0;
                break;
            case TOURNAMENTMODES_GAUNTLET:
                Deck[player].setDeckNumber(1);
                Deck[player].setDeckType(PLAYER_TYPE_CPU);
                gauntletLastDeckNumber[player]=nmbDecks;
                break;
            case TOURNAMENTMODES_KO:
                TournamentsDecks.clear();
                if (player>0)
                {
                    TournamentsDecks.push_back(Deck[0]);
                }
                else
                {
                    for (unsigned int i=0;i<nmbDecks;i++)
                        TournamentsDecks.push_back(TDeck(i+1,PLAYER_TYPE_CPU));
                    //shuffle remaining decks
                    // human player is always on first place
                    if (player>0)
                        std::random_shuffle ( TournamentsDecks.begin()+1, TournamentsDecks.end() );
                    else
                         std::random_shuffle ( TournamentsDecks.begin(), TournamentsDecks.end() );
                    Deck[0]=TournamentsDecks[0];
                    Deck[1]=TournamentsDecks[1];
                }
                 break;
        case TOURNAMENTMODES_DOUBLEKO:
                looserDecks=false;
                TournamentsDecks.clear();
                if (player>0)
                {
                    TournamentsDecks.push_back(Deck[0]);
                }
                else
                {
                    //only AI-vs-AI
                    for (unsigned int i=0;i<nmbDecks;i++)
                        TournamentsDecks.push_back(TDeck(i+1,PLAYER_TYPE_CPU));
                    //shuffle remaining decks
                    std::random_shuffle ( TournamentsDecks.begin(), TournamentsDecks.end() );
                    Deck[0]=TournamentsDecks[0];
                    Deck[1]=TournamentsDecks[1];
                }
                 break;
           case TOURNAMENTMODES_ROUNDROBIN:
                TournamentsDecks.clear();
                TournamentsDecks.push_back(Deck[0]);
                break;
        }
        updateScoreforTournament();

    }
    else if (tournamentMode==TOURNAMENTMODES_GAUNTLET)
    { //Gauntlet mode can be activated twice
        Deck[player].setDeckNumber(1);
        Deck[player].setDeckType(PLAYER_TYPE_CPU);
        gauntletLastDeckNumber[player]=nmbDecks;
        updateScoreforTournament();
    }



}

bool Tournament::checkTournamentFile(bool isAI)
{
    bool tournamentCanBeContinued=false;
    if (load(isAI,false))
    {
        if (!endOfMatch || nextDeckFound)
            tournamentCanBeContinued=true;
        //reset variables
        endOfMatch=false;
        nextDeckFound=false;
        mTournamentMode=0;
    }

    return tournamentCanBeContinued;
}


bool Tournament::load(bool isAI, bool loadComplete)
{
    std::string contents;
    bool readTournament;
    if (!isAI)
        readTournament = JFileSystem::GetInstance()->readIntoString(options.profileFile(PLAYER_TOURNAMENT), contents);
    else
        readTournament = JFileSystem::GetInstance()->readIntoString(AI_TOURNAMENT, contents);

    if (readTournament)
    {

        TournamentsDecks.clear();
        std::stringstream stream(contents);
        std::string s;
         endOfMatch=false;
         nextDeckFound=false;
         TDeck newDeck;
        while (std::getline(stream, s))
        {

            if (!s.size()) continue;
            if (s[s.size() - 1] == '\r') s.erase(s.size() - 1); //Handle DOS files
            if (s.size() && s[0] == '#') continue;
            size_t i = s.find_first_of("=");
            if (i == string::npos) continue;
            //printf("string %s\n",s.c_str());
            string key = s.substr(0, i);
            string value = s.substr(i + 1);
            //printf("key %s, value %s\n",key.c_str(),value.c_str());

            if (key.find("TournamentMode")!=string::npos)
                mTournamentMode=atoi(value.c_str());
            if (key.find("endOfMatch")!=string::npos)
                endOfMatch=atoi(value.c_str())>0;
            if (key.find("nextDeckFound")!=string::npos)
                nextDeckFound=atoi(value.c_str())>0;
            if (loadComplete){
                if (key.find("Competition")!=string::npos)
                    mCompetition=atoi(value.c_str())>0;
                if (key.find("PlayerSwaped")!=string::npos)
                    mPlayerSwaped=atoi(value.c_str())>0;
                if (key.find("NbGames")!=string::npos)
                    mNbGames=atoi(value.c_str());
                if (key.find("MatchMode")!=string::npos)
                    mMatchMode=atoi(value.c_str());
                if (key.find("GamesPlayed")!=string::npos)
                    mGamesPlayed=atoi(value.c_str());
                if (key.find("nmbDecks")!=string::npos)
                    nmbDecks=atoi(value.c_str());
                if (key.find("FastTimerMode")!=string::npos)
                    mFastTimerMode=atoi(value.c_str())>0;
                if (key.find("looserDecks")!=string::npos)
                    looserDecks=atoi(value.c_str())>0;
                if (s.find_first_of("[") != string::npos)
                {
                    size_t p1 = s.find_first_of("[");
                    key = s.substr(0, p1);
                    string player = s.substr(p1 + 1,1);

                   // printf("player %i\n",atoi(player.c_str()));
                    if (key.find("PlayDeck")!=string::npos)
                    {
                         size_t t1 = value.find_first_of(",");
                         Deck[atoi(player.c_str())].setDeckNumber(atoi(value.substr(0,t1).c_str()));
                         size_t t2 = value.find(",",t1+1);
                          Deck[atoi(player.c_str())].setDeckType((PlayerType)atoi(value.substr(t1+1,t2).c_str()));
                          size_t t3 = value.find(",",t2+1);
                         Deck[atoi(player.c_str())].setVictories(atoi(value.substr(t2+1,t3).c_str()));
                         size_t t4 = value.find(",",t3+1);
                        Deck[atoi(player.c_str())].setLastWin(atoi(value.substr(t3+1,t4).c_str()));
                        size_t t5 = value.find(",",t4+1);
                        gauntletLastDeckNumber[atoi(player.c_str())]=atoi(value.substr(t4+1,t5).c_str());
                        TournamentsDecksID[atoi(player.c_str())]=atoi(value.substr(t5+1).c_str());
                    }
                    if (key.find("TDeck")!=string::npos)
                    {
                         newDeck.reset();
                         size_t t1 = value.find_first_of(",");
                         newDeck.setDeckNumber(atoi(value.substr(0,t1).c_str()));
                         size_t t2 = value.find(",",t1+1);
                         newDeck.setDeckType((PlayerType)atoi(value.substr(t1+1,t2).c_str()));
                         size_t t3 = value.find(",",t2+1);
                         newDeck.setMatchesWon(atoi(value.substr(t2+1,t3).c_str()));
                         size_t t4 = value.find(",",t3+1);
                         newDeck.setMatchesPlayed(atoi(value.substr(t3+1,t4).c_str()));
                         size_t t5 = value.find(",",t4+1);
                         newDeck.setGamesWon(atoi(value.substr(t4+1,t5).c_str()));
                         newDeck.setGamesPlayed(atoi(value.substr(t5+1).c_str()));
                         addDeck(0,newDeck);
                    }
                }
            }
        }
        // copy played games and matches back to Deck (only for tournaments)
        if (TournamentsDecks.size()>1)
        {
            if (TournamentsDecksID[0]<TournamentsDecks.size()){
                Deck[0].setGamesPlayed(TournamentsDecks[TournamentsDecksID[0]].getGamesPlayed());
                Deck[0].setGamesWon(TournamentsDecks[TournamentsDecksID[0]].getGamesWon());
                Deck[0].setMatchesPlayed(TournamentsDecks[TournamentsDecksID[0]].getMatchesPlayed());
                Deck[0].setMatchesWon(TournamentsDecks[TournamentsDecksID[0]].getMatchesWon());
            }
            if (TournamentsDecksID[1]<TournamentsDecks.size()){
                Deck[1].setGamesPlayed(TournamentsDecks[TournamentsDecksID[1]].getGamesPlayed());
                Deck[1].setGamesWon(TournamentsDecks[TournamentsDecksID[1]].getGamesWon());
                Deck[1].setMatchesPlayed(TournamentsDecks[TournamentsDecksID[1]].getMatchesPlayed());
                Deck[1].setMatchesWon(TournamentsDecks[TournamentsDecksID[1]].getMatchesWon());
            }
        }

        return true;
    }
    else
        return false;

}

void Tournament::save(bool isAI)
{
    std::ofstream Tfile;
    bool saveTournament=false;
    if (!isAI)
        saveTournament = JFileSystem::GetInstance()->openForWrite(Tfile,options.profileFile(PLAYER_TOURNAMENT),ios_base::out);
    else
        saveTournament = JFileSystem::GetInstance()->openForWrite(Tfile,AI_TOURNAMENT,ios_base::out);

    if (saveTournament)
    {
        char buf[256];
        sprintf(buf, "TournamentMode=%i\n",mTournamentMode);
        Tfile << buf;
        sprintf(buf, "Competition=%i\n",mCompetition);
        Tfile << buf;
        sprintf(buf, "PlayerSwaped=%i\n",mPlayerSwaped);
        Tfile << buf;
        sprintf(buf, "NbGames=%i\n",mNbGames);
        Tfile << buf;
        sprintf(buf, "MatchMode=%i\n",mMatchMode);
        Tfile << buf;
        sprintf(buf, "GamesPlayed=%i\n",mGamesPlayed);
        Tfile << buf;
        sprintf(buf, "nmbDecks=%i\n",nmbDecks);
        Tfile << buf;
        sprintf(buf, "FastTimerMode=%i\n",mFastTimerMode);
        Tfile << buf;
        sprintf(buf, "endOfMatch=%i\n",endOfMatch);
        Tfile << buf;
        sprintf(buf, "nextDeckFound=%i\n",nextDeckFound);
        Tfile << buf;
        if (mTournamentMode==TOURNAMENTMODES_DOUBLEKO)
        {
            sprintf(buf, "looserDecks=%i\n",looserDecks);
            Tfile << buf;
        }
        sprintf(buf, "#PlayDeck[number]=decknumber,deckType,victories,lastWin,LastDeckNumber,TournamentsID\n");
        Tfile << buf;
        for (int i = 0; i < NMB_PLAYERS; i++)
        {
                sprintf(buf, "PlayDeck[%i]=%i,%i,%i,%i,%i,%i\n",i,Deck[i].getDeckNumber(),Deck[i].getDeckType(),Deck[i].getVictories(),Deck[i].getLastWin(),gauntletLastDeckNumber[i],TournamentsDecksID[i]);
                Tfile << buf;
        }
        if (mTournamentMode>2)
        {
            sprintf(buf, "#TournamentDeck[number]=decknumber,deckType,matchWins,playedMatches,gameWins,playedGames\n");
            Tfile << buf;
            for (unsigned int i = 0; i < TournamentsDecks.size(); i++)
            {
                   sprintf(buf, "TDeck[%i]=%i,%i,%i,%i,%i,%i\n",i,TournamentsDecks.at(i).getDeckNumber(),TournamentsDecks.at(i).getDeckType(),TournamentsDecks.at(i).getMatchesWon(),TournamentsDecks.at(i).getMatchesPlayed(),TournamentsDecks.at(i).getGamesWon(),TournamentsDecks.at(i).getGamesPlayed());
                   Tfile << buf;
            }
        }
        Tfile.close();
    }
}

void Tournament::swapPlayer()
{
    if ((mCompetition && Deck[0].isAI()) || !Deck[1].isAI())
    {
      TDeck tmpDeck;
      tmpDeck = Deck[0]; Deck[0] = Deck[1]; Deck[1] = tmpDeck;
      mPlayerSwaped = !mPlayerSwaped;
    }
}

void Tournament::revertSwapPlayer()
{
    if (mPlayerSwaped)
    {
        TDeck tmpDeck;
        tmpDeck = Deck[0]; Deck[0] = Deck[1]; Deck[1] = tmpDeck;
        mPlayerSwaped = !mPlayerSwaped;
    }
}

void Tournament::updateScoreTable(Player * _p0, Player * _p1, int gt, bool gameEnd)
{
    if (_p0) p0Quad = _p0->getIcon();
    if (_p1) p1Quad = _p1->getIcon();
    mgameType = gt;
    mVictories0=Deck[0].getVictories();
    mVictories1=Deck[1].getVictories();
    p0IsAI=Deck[0].isAI();
    p1IsAI=Deck[1].isAI();
    scoreFinal=isMatchFinished() || gameEnd;
    tournamentFinal=!isNextDeckFound() && isMatchFinished() && mTournamentMode>0;


}

void Tournament::updateScoreforTournament()
{
    tournamentFinal=!isNextDeckFound() && isMatchFinished() && mTournamentMode>0;
    scoreMatchesPlayed++;
    switch(mTournamentMode)
    {
      case 0: //no tournament
        scoreMatchesPlayed=0;
      case TOURNAMENTMODES_GAUNTLET:
        if ((gauntletLastDeckNumber[0])*(gauntletLastDeckNumber[1])>0)
            scoreMatchesToPlay=(gauntletLastDeckNumber[0]*gauntletLastDeckNumber[1])-(Deck[0].getDeckNumber()-1)*gauntletLastDeckNumber[0]-Deck[1].getDeckNumber()-1;
        else if ((gauntletLastDeckNumber[0])>0)
            scoreMatchesToPlay=(gauntletLastDeckNumber[0])-Deck[0].getDeckNumber();
        else
            scoreMatchesToPlay=(gauntletLastDeckNumber[1])-Deck[1].getDeckNumber();
        break;
    case TOURNAMENTMODES_KO:
        scoreMatchesToPlay=TournamentsDecks.size()-1;
        break;
    case TOURNAMENTMODES_DOUBLEKO:
        scoreMatchesToPlay=(TournamentsDecks.size());
        break;
    case TOURNAMENTMODES_ROUNDROBIN:
        //player 0 is always the player (RR is only available for human)
        scoreMatchesToPlay=(TournamentsDecks.size()-1)-TournamentsDecks.at(0).getMatchesPlayed();
        break;
    }
}

void Tournament::renderScoreTable()
{

    JRenderer * r = JRenderer::GetInstance();
    WFont * f2 = WResourceManager::Instance()->GetWFont(Fonts::MENU_FONT);
    char buffer[256];

    f2->SetColor(ARGB(255,255,255,255));
    float y = 25;
    float x_score = 150;
    f2->SetScale(2);
    if (tournamentFinal && mTournamentMode>0)
    {
        f2->SetScale(1);
        switch(mTournamentMode)
        {
           case TOURNAMENTMODES_ENDLESS:
            break;
          case TOURNAMENTMODES_GAUNTLET:
            //f2->DrawString(_("Gauntletmode").c_str() ,380, y2);
            break;
        case TOURNAMENTMODES_KO:
            f2->DrawString(_("KO Tournament is over. Results:").c_str() ,80, y);
            f2->SetScale(1);
            break;
        case TOURNAMENTMODES_DOUBLEKO:
            f2->DrawString(_("Double KO Tournament is over. Results:").c_str() ,80, y);
            f2->SetScale(1);
            break;
        case TOURNAMENTMODES_ROUNDROBIN:
              f2->DrawString(_("Round robin Tournament is over. Results:").c_str() ,80, y);
              f2->SetScale(0.8f);
            break;
        }

        y+=30;
        f2->SetScale(0.8f);
        f2->DrawString(_("Deck").c_str() ,80, y);
        f2->DrawString(_("Matches").c_str() ,300, y);
        f2->DrawString(_("Games").c_str() ,400, y);
        y+=30;
        for (unsigned int i=0;i<10;i++ )
        {
            if (sortedTournamentDecks[i].getDeckNumber()>0 && i<10)
            {
                if (sortedTournamentDecks[i].getDeckType()==PLAYER_TYPE_CPU)
                  sprintf(buffer, _("%i.: %s").c_str(),sortedTournamentDecks[i].getRanking(), sortedTournamentDecks[i].getDeckName().c_str());
                else
                    sprintf(buffer, _("%i.: Player Deck: %s").c_str(),sortedTournamentDecks[i].getRanking(), sortedTournamentDecks[i].getDeckName().c_str());
                f2->DrawString(buffer, 30, y);
                sprintf(buffer, _("%i/%i").c_str(), sortedTournamentDecks[i].getMatchesWon(), sortedTournamentDecks[i].getMatchesPlayed());
                f2->DrawString(buffer, 300, y);
                sprintf(buffer, _("%i/%i").c_str(), sortedTournamentDecks[i].getGamesWon(), sortedTournamentDecks[i].getGamesPlayed());
                f2->DrawString(buffer, 400, y);
                y+=20;
            }
        }

    }
    else
    {
        int gamesPlayed=0;
        if (scoreFinal)
        {
          f2->SetScale(1.5f);
          f2->DrawString(_("Final Score:").c_str() ,80, y);
          gamesPlayed=mVictories0+mVictories1;
        }
        else if (mCompetition)
        {
          f2->SetScale(1.5f);
          gamesPlayed=mVictories0+mVictories1+1;
          sprintf(buffer, _("Starting Game %i ...").c_str(), gamesPlayed);
          f2->DrawString(buffer, 80, y);
        }
        else
        {
          f2->SetScale(1.5f);
          gamesPlayed=mVictories0+mVictories1+1;
          f2->DrawString(_("Current Score:").c_str() ,80, y);
        }


        f2->SetScale(1);
        switch(mMatchMode)
        {
            case MATCHMODE_FIXED:
                sprintf(buffer, _("Game %i of %i games. ").c_str(),gamesPlayed,mNbGames);
                f2->DrawString(buffer ,30, 250);
                break;
            case MATCHMODE_BESTOF:
                //sprintf(buffer, _("Game %i,  Best of %i mode, win at least %i games. ").c_str(),gamesPlayed,mNbGames,(int)ceil((double)mNbGames/2.));
                sprintf(buffer, _("Game %i,  Best of %i mode, win at least %i games. ").c_str(),gamesPlayed,mNbGames,(int)(((float)mNbGames/2.0f)+0.5f));
                f2->DrawString(buffer ,30, 250);
                break;
            case MATCHMODE_DELTA:
                    sprintf(buffer, _("Game %i, Win the match with %i wins difference. ").c_str(),gamesPlayed,mNbGames);
                    f2->DrawString(buffer ,30, 250);
                    break;
            case MATCHMODE_CONSECUTIVE:
                    sprintf(buffer, _("Game %i, Win the match with  %i wins in a row. ").c_str(), gamesPlayed,mNbGames);
                    f2->DrawString(buffer ,30, 250);
                    break;

        }
        f2->SetScale(0.8f);
        float y2=30;
        switch(mTournamentMode)
        {

            case TOURNAMENTMODES_ENDLESS:
                x_score=80;
                f2->DrawString(_("Endless demo mode").c_str() ,330, y2);
                sprintf(buffer, _("%i matches played.").c_str(),scoreMatchesPlayed-1);
                y2+=20;
                f2->DrawString(buffer ,330, y2);
                break;
            case TOURNAMENTMODES_GAUNTLET:
                x_score=80;
                //f2->DrawString(_("Gauntletmode").c_str() ,380, y2);
                f2->DrawString(_("Tournament:").c_str() ,330, y2);
                y2+=20;
                sprintf(buffer,_("Gauntlet \n(%i matches left)").c_str(),scoreMatchesToPlay);
                f2->DrawString(buffer ,330, y2);
                break;
            case TOURNAMENTMODES_KO:
                x_score=80;
                f2->DrawString(_("Tournament:").c_str() ,300, y2);
                y2+=20;
                sprintf(buffer,_("Knockout \n(%i decks left)").c_str(),scoreMatchesToPlay);
                f2->DrawString(buffer ,300, y2);
                y2+=20;
                f2->SetScale(0.7f);
                for (int i=0;i<8;i++)
                {
                    if (sortedTournamentDecks[i].getDeckNumber()>0)
                    {
                        y2+=20;
                        if (sortedTournamentDecks[i].getDeckType()==PLAYER_TYPE_HUMAN)
                            sprintf(buffer, _("Player: %s (%i/%i)").c_str(), sortedTournamentDecks[i].getDeckName().c_str(),sortedTournamentDecks[i].getMatchesWon(),sortedTournamentDecks[i].getMatchesPlayed());
                        else
                            sprintf(buffer, _("%s (%i/%i)").c_str(), sortedTournamentDecks[i].getDeckName().c_str(),sortedTournamentDecks[i].getMatchesWon(),sortedTournamentDecks[i].getMatchesPlayed());
                        f2->DrawString(buffer ,300, y2);
                    }

                }
                break;
            case TOURNAMENTMODES_DOUBLEKO:
                x_score=80;
                f2->DrawString(_("Tournament:").c_str() ,300, y2);
                y2+=20;
                sprintf(buffer,_("Double Knockout \n%i decks left:").c_str(),scoreMatchesToPlay);
                f2->DrawString(buffer ,300, y2);
                y2+=20;
                f2->SetScale(0.7f);
                for (int i=0;i<8;i++)
                {
                    if (sortedTournamentDecks[i].getDeckNumber()>0)
                    {
                        y2+=20;
                        if (sortedTournamentDecks[i].getDeckType()==PLAYER_TYPE_HUMAN)
                            sprintf(buffer, _("Player: %s (%i/%i)").c_str(), sortedTournamentDecks[i].getDeckName().c_str(),sortedTournamentDecks[i].getMatchesWon(),sortedTournamentDecks[i].getMatchesPlayed());
                        else
                            sprintf(buffer, _("%s (%i/%i)").c_str(), sortedTournamentDecks[i].getDeckName().c_str(),sortedTournamentDecks[i].getMatchesWon(),sortedTournamentDecks[i].getMatchesPlayed());
                        f2->DrawString(buffer ,300, y2);
                    }

                }
                break;
            case TOURNAMENTMODES_ROUNDROBIN:
                  // move results to the left
                  x_score=80;
                  f2->DrawString(_("Tournament:").c_str() ,300, y2);
                  y2+=20;
                  sprintf(buffer,_("Round robin (%i left)").c_str(),scoreMatchesToPlay);
                  f2->DrawString(buffer ,300, y2);
                  //displays the first 6 decks
                  f2->SetScale(0.7f);
                  for (int i=0;i<6;i++)
                  {
                      if (sortedTournamentDecks[i].getDeckNumber()>0)
                      {
                          y2+=20;
                          if (sortedTournamentDecks[i].getDeckType()==PLAYER_TYPE_HUMAN)
                              sprintf(buffer, _("%i. Player: %s (%i/%i)").c_str(), i+1,sortedTournamentDecks[i].getDeckName().c_str(),sortedTournamentDecks[i].getMatchesWon(),sortedTournamentDecks[i].getMatchesPlayed());
                          else
                              //sprintf(buffer, _("%i. deck%i (%i/%i)").c_str(), i+1,sortedTournamentDecks[i].getDeckNumber());
                              sprintf(buffer, _("%i. %s (%i/%i)").c_str(), i+1,sortedTournamentDecks[i].getDeckName().c_str(),sortedTournamentDecks[i].getMatchesWon(),sortedTournamentDecks[i].getMatchesPlayed());
                          f2->DrawString(buffer ,300, y2);
                      }

                  }
                break;

        }
        y+=50;
        f2->SetScale(3);
        sprintf(buffer, _("%i").c_str(), mVictories1);
        f2->DrawString(buffer, x_score+80, y);
        y+=50;
        f2->DrawString(_("-").c_str() ,x_score+85, y);
        y+=50;
        sprintf(buffer, _("%i").c_str(), mVictories0);
        f2->DrawString(buffer, x_score+80, y);
        // For some reason, there's currently no player avatar prepared in-game:
        if (p1IsAI)
        {
            if (mgameType == GAME_TYPE_CLASSIC || mgameType == GAME_TYPE_DEMO)
                r->RenderQuad(p1Quad.get(), x_score, 70, 0,1,1);
        }
        else
        {
          f2->SetScale(1);
          f2->DrawString(_("Player").c_str() ,x_score, 80);
        }
        if (p0IsAI)
        {
            if (mgameType == GAME_TYPE_CLASSIC || mgameType == GAME_TYPE_DEMO)
                r->RenderQuad(p0Quad.get(), x_score+190, 215, 0,1,1);
        }
        else
        {
          f2->SetScale(1);
          f2->DrawString(_("Player").c_str() ,x_score+160, y+25);
        }
        f2->SetScale(1);
    }

}


void Tournament::initTournament()
{
    scoreMatchesToPlay=0;
    scoreMatchesPlayed=0;

     if (mTournamentMode==TOURNAMENTMODES_KO || mTournamentMode==TOURNAMENTMODES_DOUBLEKO || mTournamentMode==TOURNAMENTMODES_ROUNDROBIN)
     {
         if (TournamentsDecksID[0]<TournamentsDecks.size())
            Deck[0]=TournamentsDecks.at(TournamentsDecksID[0]);
         if (TournamentsDecksID[1]<TournamentsDecks.size())
            Deck[1]=TournamentsDecks.at(TournamentsDecksID[1]);
         //for (unsigned int i=0;i<TournamentsDecks.size();i++)
             //printf("Deck Nr %i deck%i is AI= %i (%i)\n",i,TournamentsDecks.at(i).getDeckNumber(),TournamentsDecks.at(i).isAI(),TournamentsDecks.size());
         nextDeckFound = true;
         initTournamentResults();
         calculateRanking();
         updateScoreforTournament();
     }


}
bool Tournament::isGauntlet(){return mTournamentMode==TOURNAMENTMODES_GAUNTLET;}
bool Tournament::isEndlessDemo(){return mTournamentMode==TOURNAMENTMODES_ENDLESS;}


void Tournament::leaveOutAIvsAIMatches()
{
    if (isTournament() && Deck[0].isAI() && Deck[1].isAI())
    {
        while (Deck[0].isAI() && Deck[1].isAI() && isNextDeckFound())
        {
            endOfMatch = false;
            while(!endOfMatch)
            {
              bool p0Won=  ((rand() % 100 + 1)<51);
              gameFinished(p0Won,!p0Won);

            }
            updateTournament();
            //printf("Deck %i vs %i\n",Deck[0].getDeckNumber(),Deck[1].getDeckNumber());

        }
    }
}

int Tournament::remainingDecksToNextStage()
{
    int n = TournamentsDecks.size();
    int r = (int)ceil(log(double(n))/log(2.));
    return (int)pow(2.,double(r)) -n;

}

bool Tournament::updateTournament()
{
    nextDeckFound = false;
    mGamesPlayed = 0;
    if (isGauntlet())
    {  // We are in Gauntlet mode
     for (int i = NMB_PLAYERS-1; i >=0; i--)
     {
       if (nmbDecks>1)
       {
           Deck[i].increaseDeckNumber();
           //second player must not be a human player
           if (i==1 && Deck[i].getDeckNumber() <= gauntletLastDeckNumber[i] && !Deck[i].isAI())
              Deck[i].increaseDeckNumber();
           if (Deck[i].getDeckNumber() > gauntletLastDeckNumber[i])
           {
              Deck[i].setDeckNumber(1);

           }
           else
           {
              nextDeckFound = true;
              break;
           }
       }
     }
     Deck[0].deleteStats();
     Deck[1].deleteStats();
    }
    else if (mTournamentMode==TOURNAMENTMODES_KO)
    {
        assert(TournamentsDecksID[0]<TournamentsDecks.size());
        assert(TournamentsDecksID[1]<TournamentsDecks.size());
        //update MatchScore
        if (Deck[0].getVictories()>Deck[1].getVictories())
        {
            TournamentsDecks[TournamentsDecksID[0]].winMatch();
            TournamentsDecks[TournamentsDecksID[1]].looseMatch();
        }
        else
        {
            TournamentsDecks[TournamentsDecksID[1]].winMatch();
            TournamentsDecks[TournamentsDecksID[0]].looseMatch();
        }
        //update played games and won games stats
        TournamentsDecks[TournamentsDecksID[1]].setGamesPlayed(Deck[1].getGamesPlayed());
        TournamentsDecks[TournamentsDecksID[1]].setGamesWon(Deck[1].getGamesWon());
        TournamentsDecks[TournamentsDecksID[0]].setGamesPlayed(Deck[0].getGamesPlayed());
        TournamentsDecks[TournamentsDecksID[0]].setGamesWon(Deck[0].getGamesWon());

        int n = TournamentsDecks.size();
        int r = (int)ceil(log(double(n))/log(2.));
        int Nbye=(int)pow(2.,double(r)) -n;
        //printf("n:%i r:%i nbye:%i\n",n,r,Nbye);

        if (TournamentsDecks.size()>0)
        {

            TournamentsDecksID[0]=TournamentsDecksID[0]+2;
            TournamentsDecksID[1]=TournamentsDecksID[1]+2;

            //is Deck available?
            if (TournamentsDecks.size()>2 && (TournamentsDecksID[1] >= TournamentsDecks.size()-Nbye || TournamentsDecksID[0] >= TournamentsDecks.size()-Nbye))
            {
              // next Stage
                //remove looser Decks
                unsigned int k=0;
                while(k<TournamentsDecks.size())
                {
                    if (TournamentsDecks.at(k).getMatchesWon()<TournamentsDecks.at(k).getMatchesPlayed())
                    {
                            // add Deck to sortedDecks
                            if (TournamentsDecks.size()<=8)
                            {
                                if (TournamentsDecks.size()<=4)
                                    TournamentsDecks[k].setRanking(3);
                                else
                                    TournamentsDecks[k].setRanking(4);
                                sortedTournamentDecks[TournamentsDecks.size()-1]=TDeck(TournamentsDecks[k]);

                            }
                            //remove deck
                            for (unsigned int i=k;i<TournamentsDecks.size()-1;i++)
                                TournamentsDecks[i]=TournamentsDecks[i+1];

                            TournamentsDecks.pop_back();
                    } else
                        k++;

                }
                TournamentsDecksID[0]=0;
                TournamentsDecksID[1]=1;
              //decks ok?
              if (TournamentsDecks.size()>1)
                  nextDeckFound = true;

            }
            else if (TournamentsDecks.size()==2)
            {
                TournamentsDecksID[0]=0;
                TournamentsDecksID[1]=1;
                // final match if both last decks win all matches until now.
                if (TournamentsDecks.at(0).getMatchesWon()==TournamentsDecks.at(0).getMatchesPlayed() && TournamentsDecks.at(1).getMatchesWon()==TournamentsDecks.at(1).getMatchesPlayed())
                {
                    nextDeckFound = true;
                }
            }
            else
            {
              nextDeckFound = true;
            }
          }


        if (nextDeckFound)
        {
            Deck[0]=TournamentsDecks.at(TournamentsDecksID[0]);
            Deck[1]=TournamentsDecks.at(TournamentsDecksID[1]);
            //printf("Deckfound %i vs %i (%i)\n",TournamentsDecksID[0],TournamentsDecksID[1],TournamentsDecks.size());
        }


    }
    else if (mTournamentMode==TOURNAMENTMODES_DOUBLEKO)
    {
        assert(TournamentsDecksID[0]<TournamentsDecks.size());
        assert(TournamentsDecksID[1]<TournamentsDecks.size());
        //update MatchScore
        if (Deck[0].getVictories()>Deck[1].getVictories())
        {
            TournamentsDecks[TournamentsDecksID[0]].winMatch();
            TournamentsDecks[TournamentsDecksID[1]].looseMatch();
        }
        else
        {
            TournamentsDecks[TournamentsDecksID[1]].winMatch();
            TournamentsDecks[TournamentsDecksID[0]].looseMatch();
        }
        //update played games and won games stats
        TournamentsDecks[TournamentsDecksID[1]].setGamesPlayed(Deck[1].getGamesPlayed());
        TournamentsDecks[TournamentsDecksID[1]].setGamesWon(Deck[1].getGamesWon());
        TournamentsDecks[TournamentsDecksID[0]].setGamesPlayed(Deck[0].getGamesPlayed());
        TournamentsDecks[TournamentsDecksID[0]].setGamesWon(Deck[0].getGamesWon());


        int n = TournamentsDecks.size();
        int r = (int)ceil(log(double(n))/log(2.));
        int Nbye=(int)pow(2.,double(r)) -n;
        if (TournamentsDecks.size()>0)
        {

            if (looserDecks)
            {
                // find next to teams who had loose one time wonMatches != playedMatches
                TournamentsDecksID[0]=TournamentsDecksID[1]+1;

                while ((TournamentsDecksID[0] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[0]].getMatchesPlayed()!=(TournamentsDecks[TournamentsDecksID[0]].getMatchesWon()+1))
                    TournamentsDecksID[0]++;
                TournamentsDecksID[1]=TournamentsDecksID[0]+1;
                while ((TournamentsDecksID[1] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[1]].getMatchesPlayed()!=(TournamentsDecks[TournamentsDecksID[1]].getMatchesWon()+1))
                    TournamentsDecksID[1]++;
            }
            else
            {
               // find next to teams who had never loose one time wonMatches == playedMatches
                TournamentsDecksID[0]=TournamentsDecksID[1]+1;

                while ((TournamentsDecksID[0] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[0]].getMatchesPlayed()!=TournamentsDecks[TournamentsDecksID[0]].getMatchesWon())
                    TournamentsDecksID[0]++;
                TournamentsDecksID[1]=TournamentsDecksID[0]+1;
                while ((TournamentsDecksID[1] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[1]].getMatchesPlayed()!=TournamentsDecks[TournamentsDecksID[1]].getMatchesWon())
                    TournamentsDecksID[1]++;


                //byes are only allowed in the first round
                for ( unsigned int i=0;i<TournamentsDecks.size();i++)
                    if (TournamentsDecks[i].getMatchesPlayed()>1)
                    {
                        Nbye=0;
                        break;
                    }

                if (TournamentsDecks.size()>2 && (TournamentsDecksID[1] >= TournamentsDecks.size()-Nbye || TournamentsDecksID[0] >= TournamentsDecks.size()-Nbye))
                {
                    looserDecks=true;                    
                    TournamentsDecksID[0]=0;
                    TournamentsDecksID[1]=1;
                    while ((TournamentsDecksID[0] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[0]].getMatchesPlayed()!=(TournamentsDecks[TournamentsDecksID[0]].getMatchesWon()+1))
                        TournamentsDecksID[0]++;
                    TournamentsDecksID[1]=TournamentsDecksID[0]+1;
                    while ((TournamentsDecksID[1] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[1]].getMatchesPlayed()!=(TournamentsDecks[TournamentsDecksID[1]].getMatchesWon()+1))
                        TournamentsDecksID[1]++;

                }

             }




            //is Deck available?
            if (looserDecks && TournamentsDecks.size()>2 && (TournamentsDecksID[1] >= TournamentsDecks.size() || TournamentsDecksID[0] >= TournamentsDecks.size()))
            {
                printf("next stage %i,%i\n",TournamentsDecksID[0],TournamentsDecksID[1]);
              // next Stage
                //remove double looser Decks
                unsigned int k=0;
                while(k<TournamentsDecks.size())
                {
                    if ((TournamentsDecks.at(k).getMatchesWon()+1)<TournamentsDecks.at(k).getMatchesPlayed())
                    {
                            // add Deck to sortedDecks
                            if (TournamentsDecks.size()<=8)
                            {
                                if (TournamentsDecks.size()<=4)
                                    TournamentsDecks[k].setRanking(3);
                                else
                                    TournamentsDecks[k].setRanking(4);
                                sortedTournamentDecks[TournamentsDecks.size()-1]=TDeck(TournamentsDecks[k]);

                            }
                            //remove deck
                            for (unsigned int i=k;i<TournamentsDecks.size()-1;i++)
                                TournamentsDecks[i]=TournamentsDecks[i+1];

                            TournamentsDecks.pop_back();
                    } else
                        k++;

                }
                TournamentsDecksID[0]=0;
                TournamentsDecksID[1]=1;
                if (TournamentsDecks.size()>2)
                {
                    //check if only one winner deck is available
                    int winnerDecks=0;
                    for (unsigned int i=0;i<TournamentsDecks.size();i++)
                        if (TournamentsDecks[i].getMatchesPlayed()==TournamentsDecks[i].getMatchesWon())
                            winnerDecks++;

                    if (winnerDecks>1)
                        looserDecks=false;
                    else
                        looserDecks=true;
                    if (!looserDecks)
                    {
                    // find next to teams who had never loose one time wonMatches == playedMatches
                     while ((TournamentsDecksID[0] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[0]].getMatchesPlayed()!=TournamentsDecks[TournamentsDecksID[0]].getMatchesWon())
                         TournamentsDecksID[0]++;
                     TournamentsDecksID[1]=TournamentsDecksID[0]+1;
                     while ((TournamentsDecksID[1] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[1]].getMatchesPlayed()!=TournamentsDecks[TournamentsDecksID[1]].getMatchesWon())
                         TournamentsDecksID[1]++;
                    }
                    else
                    {
                        while ((TournamentsDecksID[0] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[0]].getMatchesPlayed()!=TournamentsDecks[TournamentsDecksID[0]].getMatchesWon()+1)
                            TournamentsDecksID[0]++;
                        TournamentsDecksID[1]=TournamentsDecksID[0]+1;
                        while ((TournamentsDecksID[1] < TournamentsDecks.size()) && TournamentsDecks[TournamentsDecksID[1]].getMatchesPlayed()!=TournamentsDecks[TournamentsDecksID[1]].getMatchesWon()+1)
                            TournamentsDecksID[1]++;

                    }
                }
                else
                    looserDecks=true;
              //decks ok?
              if (TournamentsDecks.size()>1)
                  nextDeckFound = true;

            }
            else if (TournamentsDecks.size()==2)
            {
                TournamentsDecksID[0]=0;
                TournamentsDecksID[1]=1;
                // final match if both last decks win all matches until now.
                if (!looserDecks)
                {
                    nextDeckFound = true;
                    looserDecks=true;
                }
            }
            else
            {
              nextDeckFound = true;
            }
          }


        if (nextDeckFound)
        {
            Deck[0]=TournamentsDecks.at(TournamentsDecksID[0]);
            Deck[1]=TournamentsDecks.at(TournamentsDecksID[1]);
            //printf("Deckfound %i vs %i (%i)\n",TournamentsDecksID[0],TournamentsDecksID[1],TournamentsDecks.size());
        }


    }
    else if (mTournamentMode==TOURNAMENTMODES_ROUNDROBIN)
    {
        assert(TournamentsDecksID[0]<TournamentsDecks.size());
        assert(TournamentsDecksID[1]<TournamentsDecks.size());
        //update MatchScore
        if (Deck[0].getVictories()>Deck[1].getVictories())
        {
            TournamentsDecks[TournamentsDecksID[0]].winMatch();
            TournamentsDecks[TournamentsDecksID[1]].looseMatch();
        }
        else if (Deck[1].getVictories()>Deck[0].getVictories())
        {
            TournamentsDecks[TournamentsDecksID[1]].winMatch();
            TournamentsDecks[TournamentsDecksID[0]].looseMatch();
        }
        else
        { //draw
            TournamentsDecks[TournamentsDecksID[1]].drawMatch();
            TournamentsDecks[TournamentsDecksID[0]].drawMatch();
        }
        //update played games and won games stats
        TournamentsDecks[TournamentsDecksID[0]].setGamesPlayed(Deck[0].getGamesPlayed());
        TournamentsDecks[TournamentsDecksID[0]].setGamesWon(Deck[0].getGamesWon());
        TournamentsDecks[TournamentsDecksID[1]].setGamesPlayed(Deck[1].getGamesPlayed());
        TournamentsDecks[TournamentsDecksID[1]].setGamesWon(Deck[1].getGamesWon());

        if (TournamentsDecks.size()>0)
        {
            // decks are chosen in following way: 1-2,1-3,2-3,1-4,2-4,3-4, ... until 1-n,...,(n-1)-n
              TournamentsDecksID[0]++;
            //is Deck available?
            if (TournamentsDecksID[0] == TournamentsDecksID[1])
            {
              TournamentsDecksID[0]=0;
              TournamentsDecksID[1]++;
              //decks ok?
              if (TournamentsDecksID[0] < TournamentsDecksID[1] && TournamentsDecksID[1] < TournamentsDecks.size())
                  nextDeckFound = true;
            }
            else
            {
              nextDeckFound = true;
            }
        }


        if (nextDeckFound)
        {
            Deck[0]=TournamentsDecks.at(TournamentsDecksID[0]);
            Deck[1]=TournamentsDecks.at(TournamentsDecksID[1]);
            //printf("Deckfound %i vs %i (%i)\n",TournamentsDecksID[0],TournamentsDecksID[1],TournamentsDecks.size());
        }
    }//endif round robin
    else if (isEndlessDemo())
    {
        Deck[0].deleteStats();
        Deck[1].deleteStats();
        if (nmbDecks>1)
        {
            Deck[0].setDeckNumber(rand() % nmbDecks + 1);
            Deck[1].setDeckNumber(rand() % nmbDecks + 1);
        }
        else
        {
            Deck[0].setDeckNumber(1);
            Deck[1].setDeckNumber(1);
        }
        nextDeckFound = true;
    }

    return nextDeckFound;
}

void Tournament::calculateRanking()
{
    int rankingPos=1;
    unsigned int numberOfDecks=0;
    switch(mTournamentMode)
    {
        case 0://no tournament
            if (Deck[0].getVictories()>Deck[1].getVictories())
            {
                Deck[0].setRanking(1);
                Deck[1].setRanking(2);
            }
            else
            {
                Deck[1].setRanking(1);
                Deck[0].setRanking(2);
            }
            break;
        case  TOURNAMENTMODES_ROUNDROBIN:

             rankingPos=1;
             // starting from the maximal possible number of matchWins
             for (int maximalWin=TournamentsDecks.size();maximalWin>-1;maximalWin--)
             {
                 // goes trough each deck and search for the deck which has matchwins equal to maximalwins
                 for (unsigned int j=0;j<TournamentsDecks.size();j++)
                 {
                         if (TournamentsDecks[j].getMatchesWon()==maximalWin)
                         {
                             TournamentsDecks[j].setRanking(rankingPos);
                             rankingPos++;
                         }
                 }
            }
             //update scoretable

                 initTournamentResults();

                 for (int k=0;k<10;k++)
                 {
                     for (unsigned int i=0;i<TournamentsDecks.size();i++)
                     {
                        //printf("Deck %i won %i times and has rank of %i\n",i,TournamentsDecks.at(i).getMatchesWon(),TournamentsDecks.at(i).getRanking());
                        if (TournamentsDecks.at(i).getRanking()==(k+1))
                        {
                            //printf("Deck %i won %i times and has rank of %i\n",i,TournamentsDecks.at(i).getMatchesWon(),TournamentsDecks.at(i).getRanking());
                            sortedTournamentDecks[k]=TDeck(TournamentsDecks.at(i));
                        }
                     }
                 }

            break;
         case  TOURNAMENTMODES_DOUBLEKO:
            initTournamentResults();

            if (TournamentsDecks.size()==2)
            {
                if (TournamentsDecks[0].getMatchesWon()==TournamentsDecks[0].getMatchesPlayed())
                {
                    TournamentsDecks[0].setRanking(1);
                    TournamentsDecks[1].setRanking(2);
                    sortedTournamentDecks[0]=TDeck(TournamentsDecks.at(0));
                    sortedTournamentDecks[1]=TDeck(TournamentsDecks.at(1));
                }
                else
                {
                    TournamentsDecks[1].setRanking(1);
                    TournamentsDecks[0].setRanking(2);
                    sortedTournamentDecks[0]=TDeck(TournamentsDecks.at(1));
                    sortedTournamentDecks[1]=TDeck(TournamentsDecks.at(0));
                }
            }
            else
            {
                numberOfDecks=TournamentsDecks.size();
                if (numberOfDecks>8)
                    numberOfDecks=8;
                for (unsigned int i=0;i<numberOfDecks;i++)
                    sortedTournamentDecks[i]=TDeck(TournamentsDecks.at(i));
            }
           break;
         case  TOURNAMENTMODES_KO:
            initTournamentResults();

            if (TournamentsDecks.size()==2)
            {
                if (TournamentsDecks[0].getMatchesWon()>TournamentsDecks[1].getMatchesWon())
                {
                    TournamentsDecks[0].setRanking(1);
                    TournamentsDecks[1].setRanking(2);
                    sortedTournamentDecks[0]=TDeck(TournamentsDecks.at(0));
                    sortedTournamentDecks[1]=TDeck(TournamentsDecks.at(1));
                }
                else
                {
                    TournamentsDecks[1].setRanking(1);
                    TournamentsDecks[0].setRanking(2);
                    sortedTournamentDecks[0]=TDeck(TournamentsDecks.at(1));
                    sortedTournamentDecks[1]=TDeck(TournamentsDecks.at(0));
                }
            }
            else
            {
                numberOfDecks=TournamentsDecks.size();
                if (numberOfDecks>8)
                    numberOfDecks=8;
                for (unsigned int i=0;i<numberOfDecks;i++)
                    sortedTournamentDecks[i]=TDeck(TournamentsDecks.at(i));
            }
           break;

     }

}


bool Tournament::didHumanWin()
{
    bool didWin=false;
    switch(mTournamentMode)
    {
        case 0://no tournament
            if (!Deck[0].isAI())
                didWin=Deck[0].getVictories()>Deck[1].getVictories();
            else
                didWin=Deck[1].getVictories()>Deck[0].getVictories();
            break;
        case  TOURNAMENTMODES_KO:
            for (unsigned int player=0;player<TournamentsDecks.size();player++)
            {
                if (!TournamentsDecks.at(player).isAI())
                {
                  didWin=TournamentsDecks.at(player).getRanking()==1;
                  break;
                }
            }
            break;
        case  TOURNAMENTMODES_DOUBLEKO:
            for (unsigned int player=0;player<TournamentsDecks.size();player++)
            {
                if (!TournamentsDecks.at(player).isAI())
                {
                  didWin=TournamentsDecks.at(player).getRanking()<=2;
                  break;
                }
            }
            break;
        case  TOURNAMENTMODES_ROUNDROBIN:
              for (unsigned int player=0;player<TournamentsDecks.size();player++)
              {
                  if (!TournamentsDecks.at(player).isAI())
                  {
                    didWin=TournamentsDecks.at(player).getRanking()==1;
                    break;
                  }
              }
              break;
    }
    return didWin;
}

int Tournament::gamesPlayedbyHuman()
{
    int gamesplayed=0;
    switch(mTournamentMode)
    {
        case 0://no tournament
            if (!Deck[0].isAI())
                gamesplayed=Deck[0].getGamesPlayed();
            else
                gamesplayed=Deck[1].getGamesPlayed();
            break;
        case  TOURNAMENTMODES_KO:
        case  TOURNAMENTMODES_DOUBLEKO:
        case  TOURNAMENTMODES_ROUNDROBIN:
            for (unsigned int player=0;player<TournamentsDecks.size();player++)
            {
                if (!TournamentsDecks.at(player).isAI())
                {
                  gamesplayed=TournamentsDecks.at(player).getGamesPlayed();
                  break;
                }
            }
            break;
    }
    return gamesplayed;
}

int Tournament::gamesWonbyHuman()
{
    int gamesWon=0;
    switch(mTournamentMode)
    {
        case 0://no tournament
            if (!Deck[0].isAI())
                gamesWon=Deck[0].getVictories();
            else
                gamesWon=Deck[1].getVictories();
            break;
        case  TOURNAMENTMODES_KO:
        case  TOURNAMENTMODES_DOUBLEKO:
        case  TOURNAMENTMODES_ROUNDROBIN:
            for (unsigned int player=0;player<TournamentsDecks.size();player++)
            {
                if (!TournamentsDecks.at(player).isAI())
                {
                    gamesWon=TournamentsDecks.at(player).getGamesWon();
                    break;
                  }
              }
            break;
    }
    return gamesWon;
}

int Tournament::matchesPlayedbyHuman()
{
    int matchesplayed=0;
    switch(mTournamentMode)
    {
        case 0://no tournament
            break;
    case  TOURNAMENTMODES_KO:
    case  TOURNAMENTMODES_DOUBLEKO:
    case  TOURNAMENTMODES_ROUNDROBIN:
            for (unsigned int player=0;player<TournamentsDecks.size();player++)
            {
                if (!TournamentsDecks.at(player).isAI())
                {
                  matchesplayed=TournamentsDecks.at(player).getMatchesPlayed();
                  break;
                }
            }
            break;
    }
    return matchesplayed;
}

int Tournament::matchesWonbyHuman()
{
    int matcheswon=0;
    switch(mTournamentMode)
    {
        case 0://no tournament
            break;
        case  TOURNAMENTMODES_KO:
        case  TOURNAMENTMODES_DOUBLEKO:
        case  TOURNAMENTMODES_ROUNDROBIN:
            for (unsigned int player=0;player<TournamentsDecks.size();player++)
            {
                if (!TournamentsDecks.at(player).isAI())
                {
                  matcheswon=TournamentsDecks.at(player).getMatchesWon();
                  break;
                }
            }
            break;
    }
    return matcheswon;
}
