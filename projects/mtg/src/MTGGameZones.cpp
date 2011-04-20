#include "PrecompiledHeader.h"

#include "MTGGameZones.h"
#include "Player.h"
#include "WEvent.h"
#include "MTGDeck.h"
#include "Subtypes.h"

#if defined (WIN32) || defined (LINUX)
#include <time.h>
#endif

//------------------------------
//Players Game
//------------------------------

MTGPlayerCards::MTGPlayerCards()
{
    init();
}

MTGPlayerCards::MTGPlayerCards(int * idList, int idListSize)
{
    init();
    int i;
    for (i = 0; i < idListSize; i++)
    {
        MTGCard * card = MTGCollection()->getCardById(idList[i]);
        if (card)
        {
            MTGCardInstance * newCard = NEW MTGCardInstance(card, this);
            library->addCard(newCard);
        }
    }
}

MTGPlayerCards::MTGPlayerCards(MTGDeck * deck)
{
    init();
    initDeck(deck);
}

void MTGPlayerCards::initDeck(MTGDeck * deck)
{
    resetLibrary();
    map<int, int>::iterator it;
    for (it = deck->cards.begin(); it != deck->cards.end(); it++)
    {
        MTGCard * card = deck->getCardById(it->first);
        if (card)
        {
            for (int i = 0; i < it->second; i++)
            {
                MTGCardInstance * newCard = NEW MTGCardInstance(card, this);
                library->addCard(newCard);
            }
        }
    }
}

MTGPlayerCards::~MTGPlayerCards()
{
    SAFE_DELETE(library);
    SAFE_DELETE(graveyard);
    SAFE_DELETE(hand);
    SAFE_DELETE(inPlay);
    SAFE_DELETE(stack);
    SAFE_DELETE(removedFromGame);
    SAFE_DELETE(garbage);
    SAFE_DELETE(temp);
    SAFE_DELETE(playRestrictions);
}

void MTGPlayerCards::beforeBeginPhase()
{
    delete garbage;
    garbage = NEW MTGGameZone();
    garbage->setOwner(this->owner);

    library->beforeBeginPhase();
    graveyard->beforeBeginPhase();
    hand->beforeBeginPhase();
    inPlay->beforeBeginPhase();
    stack->beforeBeginPhase();
    removedFromGame->beforeBeginPhase();
    garbage->beforeBeginPhase();
    temp->beforeBeginPhase();
}

void MTGPlayerCards::setOwner(Player * player)
{
    this->owner = player;
    library->setOwner(player);
    graveyard->setOwner(player);
    hand->setOwner(player);
    inPlay->setOwner(player);
    removedFromGame->setOwner(player);
    stack->setOwner(player);
    garbage->setOwner(player);
    temp->setOwner(player);
}

void MTGPlayerCards::initGame(int shuffle, int draw)
{
    if (shuffle)
        library->shuffle();
    if (draw)
    {
        for (int i = 0; i < 7; i++)
        {
            drawFromLibrary();
        }
    }
}

void MTGPlayerCards::OptimizedHand(Player * who,int amount, int lands, int creatures, int othercards)
{
    //give the Ai hand adventage to insure a challanging match.
    GameObserver * game = game->GetInstance();
    game->currentPlayerId = game->currentPlayerId;
    game->currentPlayer = game->currentPlayer;

    if (!game->players[0]->isAI() && game->players[1]->isAI())
    {
        Player * p = who;
        MTGCardInstance * card = NULL;
        MTGGameZone * z = p->game->library;

        int optimizedland = 0;
        int optimizedothercards = 0;
        int optimizedcreatures = 0;
        for (int j = 0; j < z->nb_cards; j++)
        {
            MTGCardInstance * _card = z->cards[j];
            //-------------
            if (_card->isLand() && optimizedland < lands)
            {
                card = _card;
                if (card)
                {
                    p->game->putInZone(card, p->game->library, p->game->hand);
                    optimizedland += 1;
                }
            }
            //----------------first try to optimize a few cards that cost 2 or less.
            if (_card->getManaCost()->getConvertedCost() <= 2 && optimizedothercards < othercards && !_card->isLand()
                            && !_card->isCreature())
            {
                card = _card;
                if (card)
                {
                    p->game->putInZone(card, p->game->library, p->game->hand);
                    optimizedothercards += 1;
                }
            }
            if (_card->getManaCost()->getConvertedCost() <= 2 && optimizedcreatures < creatures && _card->isCreature())
            {
                card = _card;
                if (card)
                {
                    p->game->putInZone(card, p->game->library, p->game->hand);
                    optimizedcreatures += 1;
                }
            }
        }
        //--------------incase none of them cost 2 or less(which makes for a really poorly crafted Ai deck), try for 3 or less at this point we're accepting anything but lands under 3 mana---
        for (int k = 0; k < z->nb_cards; k++)
        {
            MTGCardInstance * _card = z->cards[k];

            if (_card->getManaCost()->getConvertedCost() <= 3 && optimizedothercards < othercards && (!_card->isLand()
                            || _card->isCreature()))
            {
                card = _card;
                if (card)
                {
                    p->game->putInZone(card, p->game->library, p->game->hand);
                    optimizedothercards += 1;
                }
            }
            if (_card->getManaCost()->getConvertedCost() <= 3 && optimizedcreatures < creatures && (_card->isCreature()
                            || !_card->isLand()))
            {
                card = _card;
                if (card)
                {
                    p->game->putInZone(card, p->game->library, p->game->hand);
                    optimizedcreatures += 1;
                }
            }
        }
        //--------------add up remaining. only 7 cards are optimized, the remaining cards (if rules change amount) are just drawn.
        int leftover = 0;
        leftover = amount;
        leftover -= optimizedland;
        leftover -= optimizedcreatures;
        leftover -= optimizedothercards;
        for (int i = leftover; i > 0; i--)
        {
            p->game->drawFromLibrary();
        }

    }
    //----------------------------
}

void MTGPlayerCards::drawFromLibrary()
{
    if (!library->nb_cards)
    {
        int cantlosers = 0;
        MTGGameZone * z = library->owner->game->inPlay;
        int nbcards = z->nb_cards;
        for (int i = 0; i < nbcards; ++i)
        {
            MTGCardInstance * c = z->cards[i];
            if (c->has(Constants::CANTLOSE) || c->has(Constants::CANTMILLLOSE))
            {
                cantlosers++;
            }
        }
        MTGGameZone * k = library->owner->opponent()->game->inPlay;
        int onbcards = k->nb_cards;
        for (int m = 0; m < onbcards; ++m)
        {
            MTGCardInstance * e = k->cards[m];
            if (e->has(Constants::CANTWIN))
            {
                cantlosers++;
            }
        }
        if (cantlosers < 1)
        {
            GameObserver::GetInstance()->gameOver = library->owner;
        }
        return;
    }
    MTGCardInstance * toMove = library->cards[library->nb_cards - 1];
    library->lastCardDrawn = toMove;
    putInZone(toMove, library, hand);
}

void MTGPlayerCards::resetLibrary()
{
    SAFE_DELETE(library);
    library = NEW MTGLibrary();
}

void MTGPlayerCards::init()
{
    library = NEW MTGLibrary();
    graveyard = NEW MTGGraveyard();
    hand = NEW MTGHand();
    inPlay = NEW MTGInPlay();
    battlefield = inPlay;

    stack = NEW MTGStack();
    removedFromGame = NEW MTGRemovedFromGame();
    exile = removedFromGame;
    garbage = NEW MTGGameZone();
    temp = NEW MTGGameZone();

    playRestrictions = NEW PlayRestrictions();
}

void MTGPlayerCards::showHand()
{
    hand->debugPrint();
}

// Moves a card to its owner's graveyard
MTGCardInstance * MTGPlayerCards::putInGraveyard(MTGCardInstance * card)
{
    return putInZone(card, card->currentZone, card->owner->game->graveyard);
}

// Moves a card to its owner's exile
MTGCardInstance * MTGPlayerCards::putInExile(MTGCardInstance * card)
{
    return putInZone(card, card->currentZone, card->owner->game->exile);
}

// Moves a card to its owner's library
MTGCardInstance * MTGPlayerCards::putInLibrary(MTGCardInstance * card)
{
    return putInZone(card, card->currentZone, card->owner->game->library);
}

// Moves a card to its *owner's* (not controller!) hand
MTGCardInstance * MTGPlayerCards::putInHand(MTGCardInstance * card)
{
    return putInZone(card, card->currentZone, card->owner->game->hand);
}


// Moves a card from one zone to another
// If the card is not actually in the expected "from" zone, does nothing and returns null 
MTGCardInstance * MTGPlayerCards::putInZone(MTGCardInstance * card, MTGGameZone * from, MTGGameZone * to)
{
    MTGCardInstance * copy = NULL;
    GameObserver *g = GameObserver::GetInstance();
    if (!from || !to)
        return card; //Error check

    int doCopy = 1;
    //When a card is moved from inPlay to inPlay (controller change, for example), it is still the same object
    if ((to == g->players[0]->game->inPlay || to == g->players[1]->game->inPlay) && (from == g->players[0]->game->inPlay || from
                    == g->players[1]->game->inPlay))
    {
        doCopy = 0;
    }

    if (!(copy = from->removeCard(card, doCopy)))
        return NULL; //ERROR

    if (options[Options::SFXVOLUME].number > 0)
    {
        if (to == g->players[0]->game->graveyard || to == g->players[1]->game->graveyard)
        {
            if (card->isCreature())
            {
                JSample * sample = WResourceManager::Instance()->RetrieveSample("graveyard.wav");
                if (sample)
                    JSoundSystem::GetInstance()->PlaySample(sample);
            }
        }
    }

    MTGCardInstance * ret = copy;

    to->addCard(copy);

    //The "Temp" zone are purely for code purposes, and we don't want the abilities engine to
    //Trigger when cards move in this zone
    // Additionally, when they move "from" this zone,
    // we trick the engine into believing that they moved from the zone the card was previously in
    // See http://code.google.com/p/wagic/issues/detail?id=335
    {
        if (to == g->players[0]->game->temp || to == g->players[1]->game->temp)
        {
            //don't send event when moving to temp
            return ret;
        }

        if (from == g->players[0]->game->temp || from == g->players[1]->game->temp)
        {
            //remove temporary stuff
            MTGCardInstance * previous = copy->previous;
            MTGCardInstance * previous2 = previous->previous;
            from = previous->previousZone;
            copy->previous = previous2;
            if (previous2)
                previous2->next = copy;
            previous->previous = NULL;
            previous->next = NULL;
            SAFE_DELETE(previous);
        }
    }

    WEvent * e = NEW WEventZoneChange(copy, from, to);
    g->receiveEvent(e);

    return ret;

}

void MTGPlayerCards::discardRandom(MTGGameZone * from, MTGCardInstance * source)
{
    if (!from->nb_cards)
        return;
    int r = WRand() % (from->nb_cards);
    WEvent * e = NEW WEventCardDiscard(from->cards[r]);
    GameObserver * game = GameObserver::GetInstance();
    game->receiveEvent(e);
    putInZone(from->cards[r], from, graveyard);
}

int MTGPlayerCards::isInPlay(MTGCardInstance * card)
{
    if (inPlay->hasCard(card))
    {
        return 1;
    }
    return 0;
}
int MTGPlayerCards::isInZone(MTGCardInstance * card,MTGGameZone * zone)
{
    if (zone->hasCard(card))
    {
        return 1;
    }
    return 0;
}
//--------------------------------------
// Zones specific code
//--------------------------------------

MTGGameZone::MTGGameZone() :
    nb_cards(0), lastCardDrawn(NULL), needShuffle(false)
{
}

MTGGameZone::~MTGGameZone()
{
    for (int i = 0; i < nb_cards; i++)
    {
        SAFE_DELETE( cards[i] );
    }
    cards.clear();
    cardsMap.clear();
    owner = NULL;
}

void MTGGameZone::beforeBeginPhase()
{
    cardsSeenThisTurn.clear();
};

void MTGGameZone::setOwner(Player * player)
{
    for (int i = 0; i < nb_cards; i++)
    {
        cards[i]->owner = player;
        cards[i]->lastController = player;
    }
    owner = player;
}

MTGCardInstance * MTGGameZone::removeCard(MTGCardInstance * card, int createCopy)
{
    assert(nb_cards < 10000);
    int i;
    cardsMap.erase(card);
    for (i = 0; i < (nb_cards); i++)
    {
        if (cards[i] == card)
        {
            card->currentZone = NULL;
            nb_cards--;
            cards.erase(cards.begin() + i);
            MTGCardInstance * copy = card;
            //if (card->isToken) //TODO better than this ?
            //  return card;
            //card->lastController = card->controller();
            if (createCopy)
            {
                copy = NEW MTGCardInstance(card->model, card->owner->game);
                copy->previous = card;
                copy->view = card->view;
                copy->isToken = card->isToken;
                copy->X = card->X;
                copy->XX = card->X/2;

                //stupid bug with tokens...
                if (card->model == card)
                    copy->model = copy;
                if (card->data == card)
                    copy->data = copy;

                card->next = copy;
            }
            copy->previousZone = this;
            return copy;
        }
    }
    return NULL;

}

MTGCardInstance * MTGGameZone::hasCard(MTGCardInstance * card)
{
    if (card->currentZone == this)
        return card;
    return NULL;

}

int MTGGameZone::countByType(const char * value)
{
    int result = 0;
    int subTypeId = Subtypes::subtypesList->find(value);
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(subTypeId))
        {
            result++;
        }
    }
    return result;
}

int MTGGameZone::countByCanTarget(TargetChooser * tc)
{
if(!tc)
return 0;
    int result = 0;
    for (int i = 0; i < (nb_cards); i++)
    {
        if (tc->canTarget(cards[i]))
        {
            result++;
        }
    }
    return result;
}
MTGCardInstance * MTGGameZone::findByName(string name)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->name == name)
        {
            return cards[i];
        }
    }
    return NULL;
}

int MTGGameZone::hasType(const char * value)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(value))
        {
            return 1;
        }
    }
    return 0;
}

int MTGGameZone::hasPrimaryType(const char * value,const char * secondvalue)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(value) && cards[i]->hasType(secondvalue))
        {
            return 1;
        }
    }
    return 0;
}

int MTGGameZone::hasSpecificType(const char * value,const char * secondvalue)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(value) && cards[i]->hasSubtype(secondvalue))
        {
            return 1;
        }
    }
    return 0;
}

int MTGGameZone::hasTypeButNotType(const char * value,const char * secondvalue)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(value) && cards[i]->hasSubtype(value) && !cards[i]->hasType(secondvalue) && !cards[i]->hasSubtype(secondvalue))
        {
            return 1;
        }
    }
    return 0;
}

int MTGGameZone::hasName(string value)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->name == value)
        {
            return 1;
        }
    }
    return 0;
}

int MTGGameZone::hasColor(int value)
{
    for (int i = 0; i < (nb_cards); i++)
    {
			if (cards[i]->getManaCost()->hasColor(value) && cards[i]->getManaCost()->getConvertedCost() > 0)
        {
            return 1;
        }
    }
    return 0;
}

int MTGGameZone::hasX()
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->getManaCost()->hasX())
        {
            return true;
        }
    }
    return false;
}

int MTGGameZone::hasAbility(int ability)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->basicAbilities[ability])
        {
            return 1;
        }
    }
    return 0;
}

int MTGGameZone::seenThisTurn(TargetChooser * tc, int castMethod)
{
    //The following 2 lines modify the passed TargetChooser. Call this function with care :/
    tc->setAllZones(); // This is to allow targetting cards without caring about the actual zone
    tc->targetter = NULL;

    int count = 0;
    for (vector<MTGCardInstance *>::iterator iter = cardsSeenThisTurn.begin(); iter != cardsSeenThisTurn.end(); ++iter)
    {
        MTGCardInstance * c = (*iter);
        if (c->matchesCastFilter(castMethod) &&  tc->canTarget(c))
            count++;
    }
    return count;
}

int MTGGameZone::seenThisTurn(string targetChooserDefinition, int castMethod)
{
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(targetChooserDefinition, NULL);
    int result = seenThisTurn(tc, castMethod);
    delete(tc);
    return result;
}


void MTGGameZone::cleanupPhase()
{
    for (int i = 0; i < (nb_cards); i++)
        (cards[i])->cleanup();
}

void MTGGameZone::shuffle()
{
    std::random_shuffle(cards.begin(), cards.end());
}

void MTGGameZone::addCard(MTGCardInstance * card)
{
    if (!card)
        return;
    cards.push_back(card);
    cardsSeenThisTurn.push_back(card);
    nb_cards++;
    cardsMap[card] = 1;
    card->lastController = this->owner;
    card->currentZone = this;

}

void MTGGameZone::debugPrint()
{
    for (int i = 0; i < nb_cards; i++)
        std::cerr << cards[i]->getName() << endl;
}

//------------------------------
MTGCardInstance * MTGInPlay::getNextAttacker(MTGCardInstance * previous)
{
    int foundprevious = 0;
    if (previous == NULL)
    {
        foundprevious = 1;
    }
    for (int i = 0; i < nb_cards; i++)
    {
        MTGCardInstance * current = cards[i];
        if (current == previous)
        {
            foundprevious = 1;
        }
        else if (foundprevious && current->isAttacker())
        {
            return current;
        }
    }
    return NULL;
}

void MTGInPlay::untapAll()
{
    int i;
    for (i = 0; i < nb_cards; i++)
    {
        MTGCardInstance * card = cards[i];
        card->setUntapping();
        if (!card->basicAbilities[(int)Constants::DOESNOTUNTAP])
        {
            if (card->frozen < 1)
            {
                card->attemptUntap();
            }
            if (card->frozen >= 1)
            {
                card->frozen = 0;
            }

        }
    }
}

//--------------------------
void MTGLibrary::shuffleTopToBottom(int nbcards)
{
    if (nbcards > nb_cards)
        nbcards = nb_cards;
    if (nbcards < 0)
        return;
    MTGCardInstance * _cards[MTG_MAX_PLAYER_CARDS];
    for (int i = nb_cards - nbcards; i < (nb_cards); i++)
    {
        int r = i + (WRand() % (nbcards - i)); // Random remaining position.
        MTGCardInstance * temp = cards[i];
        cards[i] = cards[r];
        cards[r] = temp;
    }
    for (int i = 0; i < nbcards; i++)
    {
        _cards[i] = cards[nb_cards - 1 - i];
    }
    for (int i = nbcards; i < nb_cards; i++)
    {
        _cards[i] = cards[i - nb_cards];
    }
    for (int i = 0; i < nb_cards; i++)
    {
        cards[i] = _cards[i];
    }
}

MTGGameZone * MTGGameZone::intToZone(int zoneId, Player * p, Player * p2)
{

    switch (zoneId)
    {
    case MY_GRAVEYARD:
        return p->game->graveyard;
    case OPPONENT_GRAVEYARD:
        return p->opponent()->game->graveyard;

    case MY_BATTLEFIELD:
        return p->game->inPlay;
    case OPPONENT_BATTLEFIELD:
        return p->opponent()->game->inPlay;
    case BATTLEFIELD:
        return p->game->inPlay;

    case MY_HAND:
        return p->game->hand;
    case OPPONENT_HAND:
        return p->opponent()->game->hand;

    case MY_EXILE:
        return p->game->removedFromGame;
    case OPPONENT_EXILE:
        return p->opponent()->game->removedFromGame;


    case MY_LIBRARY:
        return p->game->library;
    case OPPONENT_LIBRARY:
        return p->opponent()->game->library;
    case LIBRARY:
        return p->game->library;

    case MY_STACK:
        return p->game->stack;
    case OPPONENT_STACK:
        return p->opponent()->game->stack;
    case STACK:
        return p->game->stack;
    }

    if (!p2) return NULL;
    switch (zoneId)
    {    
    case TARGET_CONTROLLER_GRAVEYARD:
        return p2->game->graveyard;

    case TARGET_CONTROLLER_BATTLEFIELD:
        return p2->game->inPlay;

    case TARGET_CONTROLLER_HAND:
        return p2->game->hand;

    case TARGET_CONTROLLER_EXILE:
        return p2->game->removedFromGame;

    case TARGET_CONTROLLER_LIBRARY:
        return p2->game->library;

    case TARGET_CONTROLLER_STACK:
        return p2->game->stack;

    default:
        return NULL;
    }
}

MTGGameZone * MTGGameZone::intToZone(int zoneId, MTGCardInstance * source, MTGCardInstance * target)
{
    Player *p, *p2;
    GameObserver * g = GameObserver::GetInstance();
    if (!source)
        p = g->currentlyActing();
    else
        p = source->controller();
    if (!target)
    {
        if(source->target)
        {
        //bug case, this is a patchwork fix for now
        //we need to find the root cause of why the 2nd variable is not returning the target.
            p2 = source->target->controller();
            target = source->target;
        }
        else
        {
        //bug or bug case default to 
            p2 = source->controller();
                target = source;
        }
    }
    else
        p2 = target->controller();


    MTGGameZone * result = intToZone(zoneId, p, p2);
    if (result) return result;

    switch (zoneId)
    {
    case TARGET_OWNER_GRAVEYARD:
        return target->owner->game->graveyard;
    case GRAVEYARD:
        return target->owner->game->graveyard;
    case OWNER_GRAVEYARD:
        return target->owner->game->graveyard;

    case TARGET_OWNER_BATTLEFIELD:
        return target->owner->game->inPlay;
    case OWNER_BATTLEFIELD:
        return target->owner->game->inPlay;

    case TARGET_OWNER_HAND:
        return target->owner->game->hand;
    case HAND:
        return target->owner->game->hand;
    case OWNER_HAND:
        return target->owner->game->hand;

    case TARGET_OWNER_EXILE:
        return target->owner->game->removedFromGame;
    case EXILE:
        return target->owner->game->removedFromGame;
    case OWNER_EXILE:
        return target->owner->game->removedFromGame;

    case TARGET_OWNER_LIBRARY:
        return target->owner->game->library;
    case OWNER_LIBRARY:
        return target->owner->game->library;

    case TARGET_OWNER_STACK:
        return target->owner->game->stack;
    case OWNER_STACK:
        return target->owner->game->stack;
    default:
        return NULL;
    }
 
}

int MTGGameZone::zoneStringToId(string zoneName)
{
    const char * strings[] = { "mygraveyard", "opponentgraveyard", "targetownergraveyard", "targetcontrollergraveyard",
                    "ownergraveyard", "graveyard",

                    "myinplay", "opponentinplay", "targetownerinplay", "targetcontrollerinplay", "ownerinplay", "inplay",

                    "mybattlefield", "opponentbattlefield", "targetownerbattlefield", "targetcontrollerbattlefield",
                    "ownerbattlefield", "battlefield",

                    "myhand", "opponenthand", "targetownerhand", "targetcontrollerhand", "ownerhand", "hand",

                    "mylibrary", "opponentlibrary", "targetownerlibrary", "targetcontrollerlibrary", "ownerlibrary", "library",

                    "myremovedfromgame", "opponentremovedfromgame", "targetownerremovedfromgame",
                    "targetcontrollerremovedfromgame", "ownerremovedfromgame", "removedfromgame",

                    "myexile", "opponentexile", "targetownerexile", "targetcontrollerexile", "ownerexile", "exile",

                    "mystack", "opponentstack", "targetownerstack", "targetcontrollerstack", "ownerstack", "stack",

    };

    int values[] = { MY_GRAVEYARD, OPPONENT_GRAVEYARD, TARGET_OWNER_GRAVEYARD, TARGET_CONTROLLER_GRAVEYARD, OWNER_GRAVEYARD,
                    GRAVEYARD,

                    MY_BATTLEFIELD, OPPONENT_BATTLEFIELD, TARGET_OWNER_BATTLEFIELD, TARGET_CONTROLLER_BATTLEFIELD,
                    OWNER_BATTLEFIELD, BATTLEFIELD,

                    MY_BATTLEFIELD, OPPONENT_BATTLEFIELD, TARGET_OWNER_BATTLEFIELD, TARGET_CONTROLLER_BATTLEFIELD,
                    OWNER_BATTLEFIELD, BATTLEFIELD,

                    MY_HAND, OPPONENT_HAND, TARGET_OWNER_HAND, TARGET_CONTROLLER_HAND, OWNER_HAND, HAND,

                    MY_LIBRARY, OPPONENT_LIBRARY, TARGET_OWNER_LIBRARY, TARGET_CONTROLLER_LIBRARY, OWNER_LIBRARY, LIBRARY,

                    MY_EXILE, OPPONENT_EXILE, TARGET_OWNER_EXILE, TARGET_CONTROLLER_EXILE, OWNER_EXILE, EXILE,

                    MY_EXILE, OPPONENT_EXILE, TARGET_OWNER_EXILE, TARGET_CONTROLLER_EXILE, OWNER_EXILE, EXILE,

                    MY_STACK, OPPONENT_STACK, TARGET_OWNER_STACK, TARGET_CONTROLLER_STACK, OWNER_STACK, STACK, };

    int max = sizeof(values) / sizeof *(values);

    for (int i = 0; i < max; ++i)
    {
        if (zoneName.compare(strings[i]) == 0)
        {
            return values[i];
        }
    }
    return 0;
}

MTGGameZone * MTGGameZone::stringToZone(string zoneName, MTGCardInstance * source, MTGCardInstance * target)
{
    return intToZone(zoneStringToId(zoneName), source, target);
}

ostream& MTGGameZone::toString(ostream& out) const
{
    return out << "Unknown zone";
}
ostream& MTGLibrary::toString(ostream& out) const
{
    return out << "Library " << owner->getDisplayName();
}
ostream& MTGGraveyard::toString(ostream& out) const
{
    return out << "Graveyard " << owner->getDisplayName();
}
ostream& MTGHand::toString(ostream& out) const
{
    return out << "Hand " << owner->getDisplayName();
}
ostream& MTGRemovedFromGame::toString(ostream& out) const
{
    return out << "RemovedFromGame " << owner->getDisplayName();
}
ostream& MTGStack::toString(ostream& out) const
{
    return out << "Stack " << owner->getDisplayName();
}
ostream& MTGInPlay::toString(ostream& out) const
{
    return out << "InPlay " << owner->getDisplayName();
}
ostream& operator<<(ostream& out, const MTGGameZone& z)
{
    return z.toString(out);
}
ostream& operator<<(ostream& out, const MTGPlayerCards& z)
{
    out << z.library->nb_cards << " ";
    for (int i = 0; i < z.library->nb_cards; i++)
        out << z.library->cards[i]->getMTGId() << " ";

    return out;
}

istream& operator>>(istream& in, MTGPlayerCards& z)
{
    int nb, mtgid;
    in >> nb;

    for (int i = 0; i < z.library->nb_cards; i++)
    {
        SAFE_DELETE( z.library->cards[i] );
    }
    z.library->cards.clear();
    z.library->cardsMap.clear();

    for (int i = 0; i < nb; i++)
    {
        in >> mtgid;
        MTGCard * card = MTGCollection()->getCardById(mtgid);
        if (card)
        {
            MTGCardInstance * newCard = NEW MTGCardInstance(card, &z);
            z.library->addCard(newCard);
        }
    }

    return in;
}
