#include "PrecompiledHeader.h"

#include "Player.h"
#include "GameObserver.h"
#include "DeckStats.h"
#include "ManaCost.h"
#include "DeckMetaData.h"
#include "DeckManager.h"

#ifdef TESTSUITE
#include "TestSuiteAI.h"
#endif

Player::Player(GameObserver *observer, string file, string fileSmall, MTGDeck * deck) :
    Damageable(observer, 20), mAvatarName(""), offerInterruptOnPhase(MTG_PHASE_DRAW)
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
    skippingTurn = 0;
    extraTurn = 0;
    drawCounter = 0;
    prowledTypes.clear();
    doesntEmpty = NEW ManaCost();
    poolDoesntEmpty = NEW ManaCost();
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
    SAFE_DELETE(doesntEmpty);
    SAFE_DELETE(poolDoesntEmpty);
    SAFE_DELETE(game);
    if(mAvatarTex && observer->getResourceManager())
        observer->getResourceManager()->Release(mAvatarTex);
    mAvatarTex = NULL;
    SAFE_DELETE(mDeck);
}

bool Player::loadAvatar(string file, string resName)
{
    WResourceManager * rm = observer->getResourceManager();
    if(!rm) return false;

    if (mAvatarTex)
    {
        rm->Release(mAvatarTex);
        mAvatarTex = NULL;
    }
    mAvatarTex = rm->RetrieveTexture(file, RETRIEVE_LOCK, TEXTURE_SUB_AVATAR);
    if (mAvatarTex) {
        mAvatar = rm->RetrieveQuad(file, 0, 0, 35, 50, resName, RETRIEVE_NORMAL, TEXTURE_SUB_AVATAR);
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

void Player::serumMulligan()
{
    MTGPlayerCards * currentPlayerZones = game;
    int cardsinhand = currentPlayerZones->hand->nb_cards;
    for (int i = 0; i < cardsinhand; i++) //Exile
        currentPlayerZones->putInZone(currentPlayerZones->hand->cards[0],
        currentPlayerZones->hand,
        currentPlayerZones->exile);

    currentPlayerZones->library->shuffle(); //Shuffle
    
    for (int i = 0; i < (cardsinhand); i++)
        game->drawFromLibrary();
         //Draw hand no penalty
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
        else if (areaS.compare("mode") == 0)
        {
			this->playMode = (Player::Mode)atoi(s.substr(limiter + 1).c_str());
            return true;
        }
        else if (areaS.compare("avatar") == 0)
        {
            mAvatarName = s.substr(limiter + 1);
            loadAvatar(mAvatarName, "bakaAvatar");
            return true;
        }
        else if (areaS.compare("customphasering") == 0)
        {
            phaseRing = s.substr(limiter + 1);
            return true;
        }
        else if (areaS.compare("premade") == 0)
        {
            premade = (atoi(s.substr(limiter + 1).c_str())==1);
            return true;
        }
        else if (areaS.compare("deckfile") == 0)
        {
            deckFile = s.substr(limiter + 1);
            if(playMode == Player::MODE_AI)
            {
                sscanf(deckFile.c_str(), "ai/baka/deck%i.txt", &deckId);

                int deckSetting = EASY;
                if ( opponent() )
                {
                    bool isOpponentAI = opponent()->isAI() == 1;
                    DeckMetaData *meta = observer->getDeckManager()->getDeckMetaDataByFilename( opponent()->deckFile, isOpponentAI);
                    if ( meta && meta->getVictoryPercentage() >= 65)
                        deckSetting = HARD;
                }

                SAFE_DELETE(mDeck);
                SAFE_DELETE(game);
                mDeck = NEW MTGDeck(deckFile.c_str(), MTGCollection(),0, deckSetting);
                game = NEW MTGPlayerCards(mDeck);
                // This automatically sets the observer pointer on all the deck cards
                game->setOwner(this);
                deckName = mDeck->meta_name;
            }
            return true;
        }
        else if (areaS.compare("deckfilesmall") == 0)
        {
            deckFileSmall = s.substr(limiter + 1);
            return true;
        }
        else if (areaS.compare("offerinterruptonphase") == 0)
        {
            for (int i = 0; i < NB_MTG_PHASES; i++)
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

void HumanPlayer::End()
{
    if(!premade && opponent() && (observer->gameType() == GAME_TYPE_CLASSIC))
        DeckStats::GetInstance()->saveStats(this, opponent(), observer);
}

ostream& operator<<(ostream& out, const Player& p)
{
    out << "mode=" << p.playMode << endl;
    out << *(Damageable*)&p;
	if(p.manaPool)
	{
		string manapoolstring = p.manaPool->toString();
		if(manapoolstring != "")
			out << "manapool=" << manapoolstring << endl;
	}
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

istream& operator>>(istream& in, Player& p)
{
    string s;

    while(std::getline(in, s))
    {
        if(!p.parseLine(s))
        {
            break;
        }
    }

    return in;
}


// Method comparing "this" to "aPlayer", each in their own gameObserver
bool Player::operator<(Player& aPlayer)
{
    // if this is dead and aPlayer is not dead then this < aPlayer
    if(isDead() && !aPlayer.isDead())
        return true;

    // heuristics for min-max

    // if this is more poisoined than aPlayer then this < aPlayer
    if(poisonCount > aPlayer.poisonCount)
        return true;

    // if this has less life than aPlayer then this < aPlayer
    if(life < aPlayer.life)
        return true;

    // if this has less parmanents in game that aPlayer then this < aPlayer
    if(game->battlefield->cards.size() < aPlayer.game->battlefield->cards.size())
        return true;

    return false;
}

bool Player::isDead() {
    if(observer)
        return observer->didWin(opponent());
    return false;
};
