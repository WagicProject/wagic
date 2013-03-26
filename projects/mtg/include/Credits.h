#ifndef _CREDITS_H_
#define _CREDITS_H_

#include <vector>
#include <string>
#include <JGE.h>
#include "WFont.h"
#include <time.h>
#include "Player.h"

class GameApp;
class DeckStats;

using namespace std;

class Unlockable
{
private:
    map <string, string>mValues;
public:
    Unlockable();
    void setValue(string, string);
    string getValue(string);
    bool isUnlocked();
    bool tryToUnlock(GameObserver * game);
    static void load();
    static map <string, Unlockable *> unlockables;
    static void Destroy();
};


class CreditBonus
{
public:
    int value;
    string text;
    CreditBonus(int _value, string _text);
    void Render(float x, float y, WFont * font);
};

class Credits
{
private:
    time_t gameLength;
    int isDifficultyUnlocked(DeckStats * stats);
    int isEvilTwinUnlocked();
    int isRandomDeckUnlocked();
    int IsMoreAIDecksUnlocked(DeckStats * stats);
    string unlockedTextureName;
    JQuadPtr GetUnlockedQuad(string texturename);
    bool mTournament;
    bool mMatch;
    bool mPlayerWin;
    int mGamesWon;
    int mGamesPlayed;
    int mMatchesWon;
    int mMatchesPlayed;
public:
    int value;
    Player * p1, *p2;
    GameObserver* observer;
    GameApp * app;
    int showMsg;
    int unlocked;
    string unlockedString;
    vector<CreditBonus *> bonus;
    Credits();
    ~Credits();
    void compute(GameObserver* observer, GameApp * _app);
    void computeTournament(GameObserver* g, GameApp * _app,bool tournament,bool match, bool playerWin,int gamesWon,int gamesPlayed,int matchesWon,int matchesPlayed);
    void Render();
    static int unlockRandomSet(bool force = false);
    static int unlockSetByName(string name);
    static int addCreditBonus(int value);
    static int addCardToCollection(int cardId, MTGDeck * collection);
    static int addCardToCollection(int cardId);
};

#endif
