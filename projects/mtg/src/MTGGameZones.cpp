#include "PrecompiledHeader.h"

#include "CardSelector.h"
#include "MTGGameZones.h"
#include "Player.h"
#include "WEvent.h"
#include "MTGDeck.h"
#include "Subtypes.h"

#include "Rules.h"
#include "Token.h"

#if defined (WIN32) || defined (LINUX)
#include <time.h>
#endif
//------------------------------
//Players Game
//------------------------------

MTGPlayerCards::MTGPlayerCards()
    : owner(0)
{
    init();
}

MTGPlayerCards::MTGPlayerCards(Player* player, int * idList, int idListSize)
    : owner(player)
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
    : owner(0)
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
    SAFE_DELETE(garbageLastTurn);
    garbageLastTurn = garbage = NEW MTGGameZone();
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
    garbageLastTurn->setOwner(player);
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
    GameObserver * game = who->getObserver();

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
        if (inPlay->hasAbility(Constants::CANTLOSE)
            || inPlay->hasAbility(Constants::CANTMILLLOSE)
            || owner->opponent()->game->inPlay->hasAbility(Constants::CANTWIN))
        {
            return;
        }

        library->owner->getObserver()->gameOver = library->owner;
        return;
    }
    MTGCardInstance * toMove = library->cards[library->nb_cards - 1];
    library->lastCardDrawn = toMove;

    // useability tweak - assume that the user is probably going to want to see the new card,
    // so prefetch it.

    // if we're not in text mode, always get the thumb
    if (library->owner->getObserver()->getCardSelector()->GetDrawMode() != DrawMode::kText
            && library->owner->getObserver()->getResourceManager())
    {
        DebugTrace("Prefetching AI card going into play: " << toMove->getImageName());
        library->owner->getObserver()->getResourceManager()->RetrieveCard(toMove, RETRIEVE_THUMB);

        // also cache the large image if we're using kNormal mode
        if (library->owner->getObserver()->getCardSelector()->GetDrawMode() == DrawMode::kNormal)
        {
            library->owner->getObserver()->getResourceManager()->RetrieveCard(toMove);
        }
    }

    if(putInZone(toMove, library, hand))
        toMove->currentZone = hand;
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
    garbageLastTurn = garbage;
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
    GameObserver *g = owner->getObserver();
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
            if (card->isCreature() && g->getResourceManager())
            {
                g->getResourceManager()->PlaySample("graveyard.wav");
            }
        }
    }

    MTGCardInstance * ret = copy;
    for(int i = 0; i < 2; ++i)
    {
        if(to == g->players[i]->game->library && from == g->players[i]->game->library)//if its going to the library from the library we intend to put it on top.
        {
            g->players[i]->game->temp->addCard(copy);
            g->players[i]->game->library->placeOnTop.push_back(copy);
            return ret;//don't send event
        }
    }
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
    int r = owner->getObserver()->getRandomGenerator()->random() % (from->nb_cards);
    WEvent * e = NEW WEventCardDiscard(from->cards[r]);
    GameObserver * game = owner->getObserver();
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
    for (size_t i = 0; i < cards.size(); i++)
    {
        cards[i]->stillNeeded = false;
        SAFE_DELETE(cards[i]->previous);
        SAFE_DELETE( cards[i] );
    }
    cards.clear();
    cardsMap.clear();
    owner = NULL;
}

void MTGGameZone::beforeBeginPhase()
{
    cardsSeenLastTurn.clear();
    for(size_t k = 0; k < cardsSeenThisTurn.size(); k++)
    {
        cardsSeenLastTurn.push_back(cardsSeenThisTurn[k]);
    }
    cardsSeenThisTurn.clear();
};

void MTGGameZone::setOwner(Player * player)
{
    for (int i = 0; i < nb_cards; i++)
    {
        cards[i]->owner = player;
        cards[i]->lastController = player;
        cards[i]->setObserver(player->getObserver());
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
            if(!card)
                return NULL;
            if (createCopy)
            {
                copy = card->clone();
                copy->previous = card;
                copy->view = card->view;
                copy->isToken = card->isToken;
                copy->X = card->X;
                copy->castX = card->castX;
                copy->kicked = card->kicked;

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

size_t MTGGameZone::getIndex(MTGCardInstance * card)
{
    size_t i;
    for(i = 0; i < cards.size(); i++)
    {
        if(cards[i] == card)
            return i;
    }
    return -1;
}


unsigned int MTGGameZone::countByType(const char * value)
{
    int result = 0;
    int subTypeId = MTGAllCards::findType(value);
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(subTypeId))
        {
            result++;
        }
    }
    return result;
}

unsigned int MTGGameZone::countByCanTarget(TargetChooser * tc)
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
        if (cards[i]->name == name || cards[i]->getLCName()/*tokens*/ == name)
        {
            return cards[i];
        }
    }
    return NULL;
}

bool MTGGameZone::hasType(const char * value)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(value))
        {
            return true;
        }
    }
    return false;
}

bool MTGGameZone::hasTypeSpecificInt(int value1,int value)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(value1) && cards[i]->hasType(value))
        {
            return true;
        }
    }
    return false;
}

bool MTGGameZone::hasPrimaryType(const char * value,const char * secondvalue)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(value) && cards[i]->hasType(secondvalue))
        {
            return true;
        }
    }
    return 0;
}

bool MTGGameZone::hasSpecificType(const char * value,const char * secondvalue)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(value) && cards[i]->hasSubtype(secondvalue))
        {
            return true;
        }
    }
    return false;
}

bool MTGGameZone::hasTypeButNotType(const char * value,const char * secondvalue)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(value) && cards[i]->hasSubtype(value) && !cards[i]->hasType(secondvalue) && !cards[i]->hasSubtype(secondvalue))
        {
            return true;
        }
    }
    return false;
}

bool MTGGameZone::hasName(string value)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->name == value)
        {
            return true;
        }
    }
    return false;
}

bool MTGGameZone::hasColor(int value)
{
    for (int i = 0; i < (nb_cards); i++)
    {
			if (cards[i]->getManaCost()->hasColor(value) && cards[i]->getManaCost()->getConvertedCost() > 0)
        {
            return true;
        }
    }
    return false;
}

bool MTGGameZone::hasX()
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

bool MTGGameZone::hasAbility(int ability)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->basicAbilities[ability])
        {
            return true;
        }
    }
    return false;
}

int MTGGameZone::seenThisTurn(TargetChooser * tc, int castMethod, bool lastTurn)
{
    //The following 2 lines modify the passed TargetChooser. Call this function with care :/
    tc->setAllZones(); // This is to allow targetting cards without caring about the actual zone
    tc->targetter = NULL;

    int count = 0;
    if (lastTurn)
    {
        for (vector<MTGCardInstance *>::iterator iter = cardsSeenLastTurn.begin(); iter != cardsSeenLastTurn.end(); ++iter)
        {
            MTGCardInstance * c = (*iter);
            if (c && c->matchesCastFilter(castMethod) &&  tc->canTarget(c))
                count++;
        }
    }
    else
    {
        for (vector<MTGCardInstance *>::iterator iter = cardsSeenThisTurn.begin(); iter != cardsSeenThisTurn.end(); ++iter)
        {
            MTGCardInstance * c = (*iter);
            if (c->matchesCastFilter(castMethod) &&  tc->canTarget(c))
                count++;
        }
    }
    return count;
}

int MTGGameZone::seenThisTurn(string targetChooserDefinition, int castMethod)
{
    TargetChooserFactory tcf(owner->getObserver());
    TargetChooser *tc = tcf.createTargetChooser(targetChooserDefinition, NULL);
    int result = seenThisTurn(tc, castMethod,false);
    SAFE_DELETE(tc);
    return result;
}

int MTGGameZone::seenLastTurn(string targetChooserDefinition, int castMethod)
{
    TargetChooserFactory tcf(owner->getObserver());
    TargetChooser *tc = tcf.createTargetChooser(targetChooserDefinition, NULL);
    int result = seenThisTurn(tc, castMethod,true);
    SAFE_DELETE(tc);
    return result;
}

void MTGGameZone::cleanupPhase()
{
    for (int i = 0; i < (nb_cards); i++)
        (cards[i])->cleanup();
}

void MTGGameZone::shuffle()
{
    owner->getObserver()->getRandomGenerator()->random_shuffle(cards.begin(), cards.end());
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

MTGGameZone * MTGGameZone::intToZone(GameObserver *g, int zoneId, MTGCardInstance * source, MTGCardInstance * target)
{
    Player *p, *p2;

    if (!source)
        p = g->currentlyActing();
    else
        p = source->controller();
    if (!target)
    {
        //TODO source may be NULL, need to handle the case when it is NULL.  method declaration has NULL being default value of source and target. 
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

MTGGameZone * MTGGameZone::stringToZone(GameObserver *g, string zoneName, MTGCardInstance * source, MTGCardInstance * target)
{
    return intToZone(g, zoneStringToId(zoneName), source, target);
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
    for (int i = 0; i < z.nb_cards; i++)
    {
        out << z.cards[i]->getMTGId();
        if(i < z.nb_cards - 1)
            out << ",";
    }
    return out;

//    return z.toString(out);
}

bool MTGGameZone::parseLine(const string& ss)
{
    bool result = false;
    string s = ss;

    for (int i = 0; i < nb_cards; i++)
    {
        SAFE_DELETE( cards[i] );
    }
    cards.clear();
    cardsMap.clear();
    nb_cards = 0;

    while(s.size())
    {
        size_t limiter = s.find(",");
        MTGCard * card = 0;
        string toFind;
        if (limiter != string::npos)
        {
            toFind = trim(s.substr(0, limiter));
            s = s.substr(limiter + 1);
        }
        else
        {
            toFind = trim(s);
            s = "";
        }

        card = MTGCollection()->getCardByName(toFind);
        int id = Rules::getMTGId(toFind);

        if (card)
        {
            /* For the moment we add the card directly in the final zone.
                This is not the normal way and this prevents to resolve spells.
                We'll need a fusion operation afterward to cast relevant spells */
            MTGCardInstance * newCard = NEW MTGCardInstance(card, owner->game);
            addCard(newCard);
            result = true;
        }
        else
        {
            if(toFind == "*")
                nb_cards++;
            else if ( id < 0 )
            {
                // For the moment, we create a dummy Token to please the testsuite
                Token* myToken = new Token(id);
                addCard(myToken);
                result = true;
            }
            else
            {
                DebugTrace("Card unfound " << toFind << " " << id);
            }
        }
    }

    return result;
}

ostream& operator<<(ostream& out, const MTGPlayerCards& z)
{
    if(z.library->cards.size()) {
        out << "library=";
        out << *(z.library) << endl;
    }
    if(z.battlefield->cards.size()) {
        out << "inplay=";
        out << *(z.battlefield) << endl;
    }
    if(z.graveyard->cards.size()) {
        out << "graveyard=";
        out << *(z.graveyard) << endl;
    }
    if(z.hand->cards.size()) {
        out << "hand=";
        out << *(z.hand) << endl;
    }

    return out;
}

bool MTGPlayerCards::parseLine(const string& s)
{
    size_t limiter = s.find("=");
    if (limiter == string::npos) limiter = s.find(":");
    string areaS;
    if (limiter != string::npos)
    {
        areaS = s.substr(0, limiter);
        if (areaS.compare("graveyard") == 0)
        {
            graveyard->parseLine(s.substr(limiter+1));
            return true;
        }
        else if (areaS.compare("library") == 0)
        {
            library->parseLine(s.substr(limiter+1));
            return true;
        }
        else if (areaS.compare("hand") == 0)
        {
            hand->parseLine(s.substr(limiter+1));
            return true;
        }
        else if (areaS.compare("inplay") == 0 || areaS.compare("battlefield") == 0)
        {
            battlefield->parseLine(s.substr(limiter+1));
            return true;
        }
    }

    return false;
}

