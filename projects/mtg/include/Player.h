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
    JQuad * mAvatar;
    int playMode;
    bool canPutLandsIntoPlay;
    int landsPlayerCanStillPlay;
    bool nomaxhandsize;
    int castedspellsthisturn;
    bool onlyonecast;
    int castcount;
    bool nocreatureinstant;
    bool nospellinstant;
    bool onlyoneinstant;
    bool castrestrictedcreature;
    bool castrestrictedspell;
    bool onlyoneboth;
    bool bothrestrictedspell;
    bool bothrestrictedcreature;
    bool isPoisoned;
    MTGPlayerCards * game;
    string deckFile;
    string deckFileSmall;
    string deckName;

    Player(MTGDeck * deck, string deckFile, string deckFileSmall);
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
    JQuad * getIcon();

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
    HumanPlayer(MTGDeck * deck, string deckFile, string deckFileSmall);
    HumanPlayer(string deckFile);

};

ostream& operator<<(ostream&, const Player&);

#endif
