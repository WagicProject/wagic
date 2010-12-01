#include "PrecompiledHeader.h"

#include "Player.h"
#include "GameObserver.h"
#include "DeckStats.h"
#include "ManaCost.h"

Player::Player(MTGDeck * deck, string file, string fileSmall) :
    Damageable(20)
{
    deckFile = file;
    deckFileSmall = fileSmall;
    manaPool = NEW ManaPool(this);
    canPutLandsIntoPlay = 1;
    nomaxhandsize = 0;
    castedspellsthisturn = 0;
    castrestrictedspell = 0;
    castrestrictedcreature = 0;
    onlyonecast = 0;
    castcount = 0;
    nocreatureinstant = 0;
    nospellinstant = 0;
    onlyoneinstant = 0;
    poisonCount = 0;
    damageCount = 0;
    preventable = 0;
    mAvatar = NULL;
    mAvatarTex = NULL;
    type_as_damageable = DAMAGEABLE_PLAYER;
    playMode = MODE_HUMAN;
    if (deck != NULL)
    {
        game = NEW MTGPlayerCards(deck);
        game->setOwner(this);
        deckName = deck->meta_name;
    }
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
    mAvatar = NULL;
    mAvatarTex = NULL;
}

void Player::loadAvatar(string file)
{
    if (mAvatarTex)
    {
        WResourceManager::Instance()->Release(mAvatarTex);
        mAvatar = NULL;
        mAvatarTex = NULL;
    }
    mAvatarTex = WResourceManager::Instance()->RetrieveTexture(file, RETRIEVE_LOCK, TEXTURE_SUB_AVATAR);
    if (mAvatarTex)
        mAvatar = WResourceManager::Instance()->RetrieveQuad(file, 0, 0, 35, 50, "playerAvatar", RETRIEVE_NORMAL, TEXTURE_SUB_AVATAR);
    else
        mAvatar = NULL;
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

JQuad * Player::getIcon()
{
    return mAvatar;
}

Player * Player::opponent()
{
    GameObserver * game = GameObserver::GetInstance();
    if (!game) return NULL;
    return this == game->players[0] ? game->players[1] : game->players[0];
}

HumanPlayer::HumanPlayer(MTGDeck * deck, string file, string fileSmall) :
    Player(deck, file, fileSmall)
{
    loadAvatar("avatar.jpg");
    playMode = MODE_HUMAN;
}

ManaPool * Player::getManaPool()
{
    return manaPool;
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
//Cleanup phase at the end of a turn
void Player::cleanupPhase()
{
    game->inPlay->cleanupPhase();
    game->graveyard->cleanupPhase();
}

ostream& operator<<(ostream& out, const Player& p)
{
    return out << p.getDisplayName();
}
