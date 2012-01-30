#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "JGE.h"
#include "MTGGameZones.h"
#include "Damage.h"
#include "Targetable.h"

class MTGDeck;
class MTGPlayerCards;
class MTGInPlay;
class ManaPool;

class Player: public Damageable
{
protected:
    ManaPool * manaPool;
    JTexture * mAvatarTex;
    JQuadPtr mAvatar;
    bool loadAvatar(string file, string resName = "playerAvatar");
    bool premade;

public:
    enum Mode
    {
        MODE_TEST_SUITE,
        MODE_HUMAN,
        MODE_AI
    };

    int deckId;
    string mAvatarName;
    Mode playMode;
    bool nomaxhandsize;
    MTGPlayerCards * game;
    MTGDeck * mDeck;
    string deckFile;
    string deckFileSmall;
    string deckName;
    string phaseRing;
    int offerInterruptOnPhase;
    Player(GameObserver *observer, string deckFile, string deckFileSmall, MTGDeck * deck = NULL);
    virtual ~Player();
    virtual void setObserver(GameObserver*g);
    virtual void End();
    virtual int displayStack()
    {
        return 1;
    }
    const string getDisplayName() const;

    int afterDamage();

    int gainLife(int value);
    int loseLife(int value);
    int gainOrLoseLife(int value);

    bool isPoisoned() {return (poisonCount > 0);}
    int poisoned();
    int damaged();
    int prevented();
    void unTapPhase();
    MTGInPlay * inPlay();
    ManaPool * getManaPool();
    void takeMulligan();
    
    void cleanupPhase();
    virtual int Act(float dt)
    {
        return 0;
    }

    virtual int isAI()
    {
        return 0;
    }

    bool isHuman()
    {
        return (playMode == MODE_HUMAN);
    }

    Player * opponent();
    int getId();
    JQuadPtr getIcon();

    virtual int receiveEvent(WEvent * event)
    {
        return 0;
    }

    virtual void Render()
    {
    }

    /**
    ** Returns the path to the stats file of currently selected deck. 
    */
    std::string GetCurrentDeckStatsFile();
    virtual bool parseLine(const string& s);
    friend ostream& operator<<(ostream&, const Player&);
};

class HumanPlayer: public Player
{
public:
    HumanPlayer(GameObserver *observer, string deckFile, string deckFileSmall, bool premade = false, MTGDeck * deck = NULL);
    void End(){
        if(!premade && opponent())
            DeckStats::GetInstance()->saveStats(this, opponent(), observer);
    };
    friend ostream& operator<<(ostream&, const HumanPlayer&);
};

#endif
