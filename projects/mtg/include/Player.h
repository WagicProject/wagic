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
    int skippingTurn;
    int extraTurn;
    int drawCounter;
    int energyCount;
    int yidaroCount;
    int dungeonCompleted;
    int numOfCommandCast;
    int monarch;
    int surveilOffset;
    int devotionOffset;
    int epic;
    int forcefield;
    int dealsdamagebycombat;
    int initLife;
    int raidcount;
    int handmodifier;
    int snowManaG;
    int snowManaR;
    int snowManaB;
    int snowManaU;
    int snowManaW;
    int snowManaC;
    string lastChosenName;
    vector<string> prowledTypes;
    vector<MTGCardInstance*>curses;
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

    // Added source of life gain/loss in order to check later a possible exception.
    int gainLife(int value, MTGCardInstance* source);
    int loseLife(int value, MTGCardInstance* source);
    int gainOrLoseLife(int value, MTGCardInstance* source);

    bool isPoisoned() {return (poisonCount > 0);}
    int poisoned();
    int damaged();
    int prevented();
    void unTapPhase();
    MTGInPlay * inPlay();
    ManaPool * getManaPool();
    void takeMulligan();
    void serumMulligan();
    bool hasPossibleAttackers();
    bool noPossibleAttackers();
    bool DeadLifeState(bool check = false);
    ManaCost * doesntEmpty;
    ManaCost * poolDoesntEmpty;
    ManaCost * AuraIncreased;
    ManaCost * AuraReduced;
    void cleanupPhase();
    virtual int Act(float)
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

    virtual int receiveEvent(WEvent *)
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
    friend istream& operator>>(istream&, Player&);
    bool operator<(Player& aPlayer);
    bool isDead();
};

class HumanPlayer: public Player
{
public:
    HumanPlayer(GameObserver *observer, string deckFile, string deckFileSmall, bool premade = false, MTGDeck * deck = NULL);
    void End();
    friend ostream& operator<<(ostream&, const HumanPlayer&);
};

#endif
