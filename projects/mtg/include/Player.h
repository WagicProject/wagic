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

public:
    enum ENUM_PLAY_MODE
    {
        MODE_TEST_SUITE,
        MODE_HUMAN,
        MODE_AI
    };

    JTexture * mAvatarTex;
    JQuadPtr mAvatar;
    int playMode;
    bool nomaxhandsize;
    bool isPoisoned;
    MTGPlayerCards * game;
    string deckFile;
    string deckFileSmall;
    string deckName;
    string phaseRing;
    Player(string deckFile, string deckFileSmall, MTGDeck * deck = NULL);
    virtual ~Player();

    virtual void End();
    virtual int displayStack()
    {
        return 1;
    }
    const string getDisplayName() const;
    int typeAsTarget()
    {
        return TARGET_PLAYER;
    }

    int afterDamage();

    int gainLife(int value);
    int loseLife(int value);
    int gainOrLoseLife(int value);

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

    void loadAvatar(string file);

    /**
    ** Returns the path to the stats file of currently selected deck. 
    */
    std::string GetCurrentDeckStatsFile();
};

class HumanPlayer: public Player
{
public:
    HumanPlayer(string deckFile, string deckFileSmall, MTGDeck * deck = NULL);
    HumanPlayer(string deckFile);

};

ostream& operator<<(ostream&, const Player&);
istream& operator>>(istream& in, Player& p);

#endif
