#include "PrecompiledHeader.h"

#include "Player.h"
#include "GameObserver.h"
#include "DeckStats.h"
#include "ManaCost.h"

Player::Player(string file, string fileSmall, MTGDeck * deck) :
Damageable(20)
{
    if(deck == NULL && file != "testsuite" && file != "remote")
        deck = NEW MTGDeck(file.c_str(), MTGCollection());

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
        game->setOwner(this);
        deckName = deck->meta_name;
    }
    mDeck = deck;
}

/*Method to call at the end of a game, before all objects involved in the game are destroyed */
void Player::End()
{
    DeckStats::GetInstance()->saveStats(this, opponent(), GameObserver::GetInstance());
}

Player::~Player()
{
    SAFE_DELETE(manaPool);
    SAFE_DELETE(game);
    WResourceManager::Instance()->Release(mAvatarTex);
    mAvatarTex = NULL;
    SAFE_DELETE(mDeck);
}

void Player::loadAvatar(string file)
{
    if (mAvatarTex)
    {
        WResourceManager::Instance()->Release(mAvatarTex);
        mAvatarTex = NULL;
    }
    mAvatarTex = WResourceManager::Instance()->RetrieveTexture(file, RETRIEVE_LOCK, TEXTURE_SUB_AVATAR);
    if (mAvatarTex)
        mAvatar = WResourceManager::Instance()->RetrieveQuad(file, 0, 0, 35, 50, "playerAvatar", RETRIEVE_NORMAL, TEXTURE_SUB_AVATAR);
}

const string Player::getDisplayName() const
{
    GameObserver * g = GameObserver::GetInstance();
    if (this == g->players[0]) return "Player 1";
    return "Player 2";
}

MTGInPlay * Player::inPlay()
{
    return game->inPlay;
}

int Player::getId()
{
    GameObserver * game = GameObserver::GetInstance();
    for (int i = 0; i < 2; i++)
    {
        if (game->players[i] == this) return i;
    }
    return -1;
}

JQuadPtr Player::getIcon()
{
    return mAvatar;
}

Player * Player::opponent()
{
    GameObserver * game = GameObserver::GetInstance();
    if (!game) return NULL;
    return this == game->players[0] ? game->players[1] : game->players[0];
}

HumanPlayer::HumanPlayer(string file, string fileSmall, MTGDeck * deck) :
    Player(file, fileSmall, deck)
{
    loadAvatar("avatar.jpg");
    playMode = MODE_HUMAN;
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
    GameObserver * game = GameObserver::GetInstance();
    game->receiveEvent(lifed);

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


ostream& operator<<(ostream& out, const Player& p)
{
    return out << *(p.game);
}

istream& operator>>(istream& in, Player& p)
{
    if(!p.game)
    {
        p.game = new MTGPlayerCards();
    }

    in >> *(p.game);
    p.game->setOwner(&p);

    return in;
}
