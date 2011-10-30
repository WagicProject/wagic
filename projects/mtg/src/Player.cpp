#include "PrecompiledHeader.h"

#include "Player.h"
#include "GameObserver.h"
#include "DeckStats.h"
#include "ManaCost.h"

#ifdef TESTSUITE
#include "TestSuiteAI.h"
#endif

Player::Player(GameObserver *observer, string file, string fileSmall, MTGDeck * deck) :
    Damageable(observer, 20), mAvatarName(""), offerInterruptOnPhase(Constants::MTG_PHASE_DRAW)
{
    if(deck == NULL && file != "testsuite" && file != "remote" && file != "")
        deck = NEW MTGDeck(file.c_str(), MTGCollection());

    premade = false;
    game = NULL;
    deckFile = file;
    deckFileSmall = fileSmall;
    handsize = 0;
    manaPool = NEW ManaPool(this);
    nomaxhandsize = false;
    poisonCount = 0;
    damageCount = 0;
    preventable = 0;
    mAvatarTex = NULL;
    type_as_damageable = DAMAGEABLE_PLAYER;
    playMode = MODE_HUMAN;
    if (deck != NULL)
    {
        game = NEW MTGPlayerCards(deck);
        // This automatically sets the observer pointer on all the deck cards
        game->setOwner(this);
        deckName = deck->meta_name;
    }
    else
    {
        game = new MTGPlayerCards();
        game->setOwner(this);
    }
    mDeck = deck;
}

void Player::setObserver(GameObserver*g)
{
    observer = g;
    // fix card instances direct pointer
    game->setOwner(this);
}

/*Method to call at the end of a game, before all objects involved in the game are destroyed */
void Player::End()
{
    DeckStats::GetInstance()->saveStats(this, opponent(), observer);
}

Player::~Player()
{
    SAFE_DELETE(manaPool);
    SAFE_DELETE(game);
    WResourceManager::Instance()->Release(mAvatarTex);
    mAvatarTex = NULL;
    SAFE_DELETE(mDeck);
}

bool Player::loadAvatar(string file, string resName)
{
    if (mAvatarTex)
    {
        WResourceManager::Instance()->Release(mAvatarTex);
        mAvatarTex = NULL;
    }
    mAvatarTex = WResourceManager::Instance()->RetrieveTexture(file, RETRIEVE_LOCK, TEXTURE_SUB_AVATAR);
    if (mAvatarTex) {
        mAvatar = WResourceManager::Instance()->RetrieveQuad(file, 0, 0, 35, 50, resName, RETRIEVE_NORMAL, TEXTURE_SUB_AVATAR);
        return true;
    }

    return false;
}

const string Player::getDisplayName() const
{
    if (this == observer->players[0]) return "Player 1";
    return "Player 2";
}

MTGInPlay * Player::inPlay()
{
    return game->inPlay;
}

int Player::getId()
{
    for (int i = 0; i < 2; i++)
    {
        if (observer->players[i] == this) return i;
    }
    return -1;
}

JQuadPtr Player::getIcon()
{
    if(!mAvatarTex)
        loadAvatar(mAvatarName);

    return mAvatar;
}

Player * Player::opponent()
{
    if (!observer || (observer->players.size() < 2 )) return NULL;
    return this == observer->players[0] ? observer->players[1] : observer->players[0];
}

HumanPlayer::HumanPlayer(GameObserver *observer, string file, string fileSmall, bool isPremade, MTGDeck * deck) :
    Player(observer, file, fileSmall, deck)
{
    mAvatarName = "avatar.jpg";
    playMode = MODE_HUMAN;
    premade = isPremade;
}

ManaPool * Player::getManaPool()
{
    return manaPool;
}

int Player::gainOrLoseLife(int value)
{
    if (!value)
        return 0; //Don't do anything if there's no actual life change

    thatmuch = abs(value); //the value that much is a variable to be used with triggered abilities.
    //ie:when ever you gain life, draw that many cards. when used in a trigger draw:thatmuch, will return the value
    //that the triggered event stored in the card for "that much".
    life+=value;
    if (value<0)
        lifeLostThisTurn += abs(value);

    //Send life event to listeners
    WEvent * lifed = NEW WEventLife(this,value);
    observer->receiveEvent(lifed);

    return value;
}

int Player::gainLife(int value)
{
    if (value <0)
    {
        DebugTrace("PLAYER.CPP: don't call gainLife on a negative value, use loseLife instead");
        return 0;
    }
    return gainOrLoseLife(value);
}

int Player::loseLife(int value)
{
    if (value <0)
    {
        DebugTrace("PLAYER.CPP: don't call loseLife on a negative value, use gainLife instead");
        return 0;
    }
    return gainOrLoseLife(-value);
}

int Player::afterDamage()
{
    return life;
}

int Player::poisoned()
{
    return poisonCount;
}

int Player::damaged()
{
    return damageCount;
}

int Player::prevented()
{
    return preventable;
}

void Player::takeMulligan()
{
    MTGPlayerCards * currentPlayerZones = game;
    int cardsinhand = currentPlayerZones->hand->nb_cards;
    for (int i = 0; i < cardsinhand; i++) //Discard hand
        currentPlayerZones->putInZone(currentPlayerZones->hand->cards[0],
        currentPlayerZones->hand,
        currentPlayerZones->library);

    currentPlayerZones->library->shuffle(); //Shuffle
    
    for (int i = 0; i < (cardsinhand - 1); i++)
        game->drawFromLibrary();
         //Draw hand with 1 less card penalty //almhum
}

//Cleanup phase at the end of a turn
void Player::cleanupPhase()
{
    game->inPlay->cleanupPhase();
    game->graveyard->cleanupPhase();
}

std::string Player::GetCurrentDeckStatsFile()
{
    std::ostringstream filename;
    filename << "stats/" << deckFileSmall << ".txt";
   return options.profileFile(filename.str());
}

bool Player::parseLine(const string& s)
{
    if(((Damageable*)this)->parseLine(s))
        return true;

    size_t limiter = s.find("=");
    if (limiter == string::npos) limiter = s.find(":");
    string areaS;
    if (limiter != string::npos)
    {
        areaS = s.substr(0, limiter);
        if (areaS.compare("manapool") == 0)
        {
            SAFE_DELETE(manaPool);
            manaPool = new ManaPool(this);
            ManaCost::parseManaCost(s.substr(limiter + 1), manaPool);
            return true;
        }
        else if (areaS.compare("avatar") == 0)
        {   // We don't load directly for now
            mAvatarName = s.substr(limiter + 1);
            return true;
        }
        else if (areaS.compare("customphasering") == 0)
        {
            phaseRing = s.substr(limiter + 1);
            return true;
        }
        else if (areaS.compare("premade") == 0)
        {
            premade = atoi(s.substr(limiter + 1).c_str());
            return true;
        }
        else if (areaS.compare("deckfile") == 0)
        {
            deckFile = s.substr(limiter + 1);
            return true;
        }
        else if (areaS.compare("deckfilesmall") == 0)
        {
            deckFileSmall = s.substr(limiter + 1);
            return true;
        }
        else if (areaS.compare("offerinterruptonphase") == 0)
        {
            for (int i = 0; i < Constants::NB_MTG_PHASES; i++)
            {
                string phaseStr = Constants::MTGPhaseCodeNames[i];
                if (s.find(phaseStr) != string::npos)
                {
                    offerInterruptOnPhase = PhaseRing::phaseStrToInt(phaseStr);
                    return true;
                }
            }
        }
    }

    if(!game)
    {
      game = new MTGPlayerCards();
      game->setOwner(this);
    }

    if(game->parseLine(s))
      return true;

    return false;
}

ostream& operator<<(ostream& out, const Player& p)
{
    out << *(Damageable*)&p;
    string manapoolstring = p.manaPool->toString();
    if(manapoolstring != "")
        out << "manapool=" << manapoolstring << endl;
    if(p.mAvatarName != "")
        out << "avatar=" << p.mAvatarName << endl;
    if(p.phaseRing != "")
        out << "customphasering=" << p.phaseRing << endl;
    out << "offerinterruptonphase=" << Constants::MTGPhaseCodeNames[p.offerInterruptOnPhase] << endl;
    out << "premade=" << p.premade << endl;
    if(p.deckFile != "")
        out << "deckfile=" << p.deckFile << endl;
    if(p.deckFileSmall != "")
        out << "deckfilesmall=" << p.deckFileSmall << endl;

    if(p.game)
    {
        out << *(p.game);
    }

    return out;
}


