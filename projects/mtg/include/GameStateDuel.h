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

#ifdef TESTSUITE
class TestSuite;
#endif
class Credits;
#ifdef NETWORK_SUPPORT
class JNetwork;
#endif

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
        MENUITEM_MORE_INFO = kInfoMenuID
    };

};

#endif

