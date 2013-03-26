#ifndef _GAME_STATE_DUEL_H_
#define _GAME_STATE_DUEL_H_

#include "GameState.h"
#include "SimpleMenu.h"
#include "SimplePopup.h"
#include "DeckMenu.h"
#include "MTGDeck.h"
#include "GameObserver.h"
#ifdef AI_CHANGE_TESTING
#include "Threading.h"
#endif //AI_CHANGE_TESTING


#define CHOOSE_OPPONENT 7
#define NMB_PLAYERS 2  // Number of players in the game. Currently that's always 2.

#ifdef TESTSUITE
class TestSuite;
#endif
class Credits;
#ifdef NETWORK_SUPPORT
class JNetwork;
#endif

/////// Tournament Mod ///////////
#define PLAYER_TOURNAMENT "tournament.dat"
#define AI_TOURNAMENT "/ai/baka/tournament.dat"
#define AI_RESULTS_FILE "/ai/baka/ai_test.csv"
class TDeck{
public:
    TDeck(int deck,PlayerType deckType,int victories = 0,int lastWin = 0, int wonMatches=0, int playedMatches=0, int wonGames=0, int playedGames=0);
    TDeck();
    ~TDeck();
    int getDeckNumber(){return mDeckNumber;}
    void setDeckNumber(int deck){mDeckNumber=deck;}
    void setDeckType(PlayerType ptype){mDeckType=ptype;}
    PlayerType getDeckType(){return mDeckType;}
    bool isAI(){return mDeckType==PLAYER_TYPE_CPU;}
    void reset();
    void increaseDeckNumber();
    void winGame(){mVictories++;mLastWin++;mGamesPlayed++;mWonGames++;}
    void winMatch(){mWonMatches++;mMatchesPlayed++;}
    void setMatchesWon(int w){mWonMatches=w;}
    int getMatchesWon(){return mWonMatches;}
    void looseGame(){mLastWin = 0;mGamesPlayed++;}
    void looseMatch(){mMatchesPlayed++;}
    void drawMatch(){mMatchesPlayed++;}
    int getLastWin(){return mLastWin;}
    int getVictories(){return mVictories;}
    void newMatch(){mVictories=0;mLastWin=0;}
    void deleteStats(){mVictories=0;mLastWin=0;mWonMatches=0;mMatchesPlayed=0;mWonGames=0;mGamesPlayed=0;mRanking=0;}
    void resetTournament(){mWonMatches=0;}
    void setVictories(int v){mVictories=v;}
    void setLastWin(int lastWin){mLastWin=lastWin;}
    int getGamesPlayed(){return mGamesPlayed;}
    void setGamesPlayed(int games){mGamesPlayed=games;}
    int getGamesWon(){return mWonGames;}
    void setGamesWon(int games){mWonGames=games;}
    int getMatchesPlayed(){return mMatchesPlayed;}
    void setMatchesPlayed(int matches){mMatchesPlayed=matches;}
    void setRanking(int ranking){mRanking=ranking;}
    int getRanking(){return mRanking;}
    std::string getDeckName();
private:
    int mDeckNumber;
    PlayerType mDeckType;
    int mVictories;
    int mLastWin;
    int mWonMatches;
    int mMatchesPlayed;
    int mWonGames;
    int mGamesPlayed;
    int mRanking;
    int mDifficulty;

};




class Tournament{
public:
    Tournament();
    ~Tournament();
    void addDeck(int player,int deck,PlayerType deckType);
    void addDeck(int player,TDeck newDeck);
    int getDeckNumber(int player){return Deck[player].getDeckNumber();}
    PlayerType getDeckType(int player){return Deck[player].getDeckType();}
    unsigned int getAvailableDecks(){return nmbDecks;}
    void setAvailableDecks(unsigned int a){nmbDecks=a;}
    void setGamesToPlay(int games){mNbGames=games;}
    int getGamesToPlay(){return mNbGames;}
    void setMatchType(int gamestoplay,int matchMode,bool swapPlayer=false){mNbGames=gamestoplay;mMatchMode=matchMode;mCompetition=swapPlayer;}
    void setMatchMode(int m){mMatchMode=m;}
    int getMatchMode(){return mMatchMode;}
    int getTournamentMode(){return mTournamentMode;}
    bool isGauntlet();
    bool isEndlessDemo();
    bool isTournament(){return mTournamentMode>0;}
    void setTournamentMode(int b){mTournamentMode=b;}
    bool getFastTimerMode(){return mFastTimerMode;}
    void setFastTimerMode(bool timerMode){mFastTimerMode=timerMode;}
    void updateScoreTable(Player * _p0, Player * _p1, int gt = GAME_TYPE_CLASSIC, bool gameEnd=false);
    void renderScoreTable();
    bool gameFinished(bool,bool);
    bool isMatchFinished(){return endOfMatch;}
    bool isNextDeckFound(){return nextDeckFound;}
    void swapPlayer();
    void revertSwapPlayer();
    void Start(); //may change the gamephase
    void End();
    void saveMatchResults();
    bool updateTournament();
    void save(bool isAI);
    bool load(bool isAI, bool loadComplete);
    void setOpLevel(int o){mOpLevel=o;}
    int getOpLevel(){return mOpLevel;}
    void initTournament();
    void resetTournamentSelection(); //resets the choosen oppent decks
    std::string exportTournamentDescription();
    unsigned int getNumberofTournamentDecks(){return TournamentsDecks.size();}
    void enableTournamantMode(int tournamentMode, int player = 0);
    bool wasScoreDisplayed(){return scoreDisplayed;}
    void setScoreDisplayed(bool s){scoreDisplayed=s;}
    bool didHumanWin();
    int gamesPlayedbyHuman();
    int gamesWonbyHuman();
    int matchesPlayedbyHuman();
    int matchesWonbyHuman();
    void calculateRanking();
    void initTournamentResults();
    void setRankedDeck(int rank,TDeck deck){if (rank<11&&rank>0)sortedTournamentDecks[rank-1]=deck;}
    bool checkTournamentFile(bool isAI);
    void leaveOutAIvsAIMatches();
    void updateScoreforTournament();
    int getHardRandomDeck();
    int getRandomDeck(bool noEasyDecks);
    int remainingDecksToNextStage();
private:
    bool mCompetition;
    bool mPlayerSwaped;
    int mSpeed;
    int mNbGames;
    int mMatchMode;
    int mGamesPlayed;
    unsigned int TournamentsDecksID[NMB_PLAYERS];
    int gauntletLastDeckNumber[NMB_PLAYERS];
    TDeck Deck[NMB_PLAYERS];
    std::vector<TDeck>  TournamentsDecks;
    int mOpLevel;
    unsigned int nmbDecks; // number of decks in the last constructed deckmenu
    bool mFastTimerMode;
    int mTournamentMode;
    bool endOfMatch;
    bool nextDeckFound;
    bool looserDecks; //double KO
    // variables for scoretable
    bool scoreDisplayed;
    JQuadPtr  p0Quad;
    JQuadPtr p1Quad;
    int mgameType;
    TDeck  sortedTournamentDecks[10];
    int mVictories0;
    int mVictories1;
    bool p0IsAI;
    bool p1IsAI;
    bool scoreFinal;
    bool tournamentFinal;
    int scoreMatchesToPlay;
    int scoreMatchesPlayed;
};

/////// End Tournament Mod ///////////

class GameStateDuel: public GameState, public JGuiListener
{
private:
#ifdef TESTSUITE
    TestSuite * testSuite;
#endif

    Credits * credits;
    int mGamePhase;
    GameObserver * game;
    DeckMenu * deckmenu;
    DeckMenu * opponentMenu;
    SimpleMenu * menu;
    SimplePopup * popupScreen; // used for informational screens, modal
    static int selectedPlayerDeckId;
    static int selectedAIDeckId;
/////// Tournament Mod ///////////
     //std::ofstream ai_file;
     SimpleMenu * cnogmenu;     // to choose the number of games to play in the match
     void setAISpeed();
     Tournament* tournament;
     bool tournamentSelection;
/////// End Tournament Mod ///////////

    bool premadeDeck;
    int OpponentsDeckid;
    string musictrack;

    bool MusicExist(string FileName);
    void ConstructOpponentMenu(); //loads the opponentMenu if it doesn't exist
    void initScroller();
    void setGamePhase(int newGamePhase);


public:
    GameStateDuel(GameApp* parent);
    virtual ~GameStateDuel();
#ifdef TESTSUITE
    void loadTestSuitePlayers();
    void setupTestSuite();
#endif

#ifdef AI_CHANGE_TESTING
    int startTime;
    int totalTestGames;
    int testPlayer2Victories;
    int totalAIDecks;
    static boost::mutex mMutex;
    vector<boost::thread> mWorkerThread;
    static void ThreadProc(void* inParam);
    void handleResults(GameObserver* aGame){
        mMutex.lock();
        if (aGame->didWin(aGame->players[1]))
        {
            testPlayer2Victories++;
            totalTestGames++;
        }
        else if( aGame->didWin(aGame->players[0]))
        {
            totalTestGames++;
        }
        mMutex.unlock();
    };
#endif

    virtual void ButtonPressed(int ControllerId, int ControlId);
    virtual void Start();
    virtual void End();
    virtual void Update(float dt);
    virtual void Render();
    void initRand(unsigned seed = 0);

    void OnScroll(int inXVelocity, int inYVelocity);

    enum ENUM_DUEL_STATE_MENU_ITEM
    {
        MENUITEM_CANCEL = kCancelMenuID,
        MENUITEM_NEW_DECK = -10,
        MENUITEM_RANDOM_PLAYER = kRandomPlayerMenuID,
        MENUITEM_RANDOM_AI = kRandomAIPlayerMenuID,
        MENUITEM_MAIN_MENU = -13,
        MENUITEM_EVIL_TWIN = kEvilTwinMenuID,
        MENUITEM_MULLIGAN = -15,
        MENUITEM_UNDO = -16,
#ifdef TESTSUITE
        MENUITEM_LOAD = -17,
#endif
#ifdef NETWORK_SUPPORT
        MENUITEM_REMOTE_CLIENT = -18,
        MENUITEM_REMOTE_SERVER = -19,
#endif
/////// Tournament Mod ///////////
        MENUITEM_SHOW_SCORE = -20,
        MENUITEM_SPEED_FAST = -21,
        MENUITEM_SPEED_NORMAL = -22,
        MENUITEM_GAUNTLET = -23,
        MENUITEM_KO_TOURNAMENT = -24,
        MENUITEM_RR_TOURNAMENT = -25,
        MENUITEM_START_TOURNAMENT = -26,
        MENUITEM_ENDLESSDEMO = -27,
        MENUITEM_RANDOM_AI_HARD = -28,
        MENUITEM_DOUBLEKO_TOURNAMENT = -29,
        MENUITEM_FILL_NEXT_STAGE_HARD = -30,
        MENUITEM_FILL_NEXT_STAGE = -31,
/////// End Tournament Mod ///////////
        MENUITEM_MORE_INFO = kInfoMenuID
    };

};

#endif

