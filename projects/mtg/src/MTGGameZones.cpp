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
    //commander zone init
    if(deck->CommandZone.size())
    {
        for(unsigned int j = 0; j < deck->CommandZone.size(); j++)
        {
            string cardID = deck->CommandZone[j];
            MTGCard * card = MTGCollection()->getCardById(atoi(cardID.c_str()));
            if(card)
            {
                MTGCardInstance * newCard = NEW MTGCardInstance(card, this);
                //the card is marked as commander ad added to library.
                newCard->isCommander = 1;
                library->addCard(newCard);
            }
        }
    }
    map<int, int>::iterator it;
    for (it = deck->cards.begin(); it != deck->cards.end(); it++)
    {
        MTGCard * card = deck->getCardById(it->first);
        if (card)
        {
            for (int j = 0; j < it->second; j++)
            {
                MTGCardInstance * newCard = NEW MTGCardInstance(card, this);
                if(!deck->CommandZone.size()){ //If no commander format there are no limitations for cards.
                    library->addCard(newCard);
                } else {
                    if(newCard->hasType("Land") && newCard->hasType("Basic")){ //There are no limitations for basic lands cards.
                        library->addCard(newCard);
                    } else{
                        bool colorFound = false; // All the cards have to share at least one color with commander identity color (any symbol in manacost or magic text).
                        bool colorless = false; // Colorless card can be always added to deck.
                        for(unsigned int i = 0; i < deck->CommandZone.size() && !colorFound; i++){
                            MTGCard * cmdcard = MTGCollection()->getCardById(atoi(deck->CommandZone[i].c_str()));
                            if(cmdcard){
                                MTGCardInstance * commander = NEW MTGCardInstance(cmdcard, this);
                                if((newCard->hasColor(Constants::MTG_COLOR_WHITE) && commander->hasColor(Constants::MTG_COLOR_WHITE)) ||
                                    (newCard->hasColor(Constants::MTG_COLOR_BLACK) && commander->hasColor(Constants::MTG_COLOR_BLACK)) ||
                                    (newCard->hasColor(Constants::MTG_COLOR_RED) && commander->hasColor(Constants::MTG_COLOR_RED)) ||
                                    (newCard->hasColor(Constants::MTG_COLOR_BLUE) && commander->hasColor(Constants::MTG_COLOR_BLUE)) ||
                                    (newCard->hasColor(Constants::MTG_COLOR_GREEN) && commander->hasColor(Constants::MTG_COLOR_GREEN)) ||
                                    (newCard->hasColor(Constants::MTG_COLOR_WHITE) && commander->magicText.find("{w}") != std::string::npos) ||
                                    (newCard->hasColor(Constants::MTG_COLOR_BLACK) && commander->magicText.find("{b}") != std::string::npos)||
                                    (newCard->hasColor(Constants::MTG_COLOR_RED) && commander->magicText.find("{r}") != std::string::npos) ||
                                    (newCard->hasColor(Constants::MTG_COLOR_BLUE) && commander->magicText.find("{u}") != std::string::npos) ||
                                    (newCard->hasColor(Constants::MTG_COLOR_GREEN) && commander->magicText.find("{g}") != std::string::npos) || 
                                    (newCard->magicText.find("{w}") != std::string::npos && commander->hasColor(Constants::MTG_COLOR_WHITE)) ||
                                    (newCard->magicText.find("{b}") != std::string::npos && commander->hasColor(Constants::MTG_COLOR_BLACK)) ||
                                    (newCard->magicText.find("{r}") != std::string::npos && commander->hasColor(Constants::MTG_COLOR_RED)) ||
                                    (newCard->magicText.find("{u}") != std::string::npos && commander->hasColor(Constants::MTG_COLOR_BLUE)) ||
                                    (newCard->magicText.find("{g}") != std::string::npos && commander->hasColor(Constants::MTG_COLOR_GREEN)) ||
                                    (newCard->magicText.find("{w}") != std::string::npos && commander->magicText.find("{w}") != std::string::npos) ||
                                    (newCard->magicText.find("{b}") != std::string::npos && commander->magicText.find("{b}") != std::string::npos)||
                                    (newCard->magicText.find("{r}") != std::string::npos && commander->magicText.find("{r}") != std::string::npos) ||
                                    (newCard->magicText.find("{u}") != std::string::npos && commander->magicText.find("{u}") != std::string::npos) ||
                                    (newCard->magicText.find("{g}") != std::string::npos && commander->magicText.find("{g}") != std::string::npos)){
                                    colorFound = true;
                                }
                            }
                        }
                        if(!colorFound)
                            colorless = (newCard->magicText.find("{g}") == std::string::npos && newCard->magicText.find("{w}") == std::string::npos && newCard->magicText.find("{b}") == std::string::npos && 
                            newCard->magicText.find("{r}") == std::string::npos && newCard->magicText.find("{u}") == std::string::npos && !newCard->hasColor(Constants::MTG_COLOR_BLUE) && 
                            !newCard->hasColor(Constants::MTG_COLOR_RED) &&  !newCard->hasColor(Constants::MTG_COLOR_WHITE) && !newCard->hasColor(Constants::MTG_COLOR_GREEN) && 
                            !newCard->hasColor(Constants::MTG_COLOR_BLACK));
                        if(colorFound || colorless){
                            bool onlyInstance = true; // In commander format only single cards are allowed if they are not basic lands.
                            for(unsigned int k = 0; k < library->cards.size() && onlyInstance; k++){
                                if(library->cards[k]->name == newCard->name)
                                    onlyInstance = false;
                            }
                            if(onlyInstance)
                                library->addCard(newCard);
                        }
                    }
                }
            }
        }
    }
    //sb init
    if(deck->Sideboard.size())
    {
        for(unsigned int j = 0; j < deck->Sideboard.size(); j++)
        {
            string cardID = deck->Sideboard[j];
            MTGCard * card = MTGCollection()->getCardById(atoi(cardID.c_str()));
            if(card)
            {
                MTGCardInstance * newCard = NEW MTGCardInstance(card, this);
                //sb zone
                sideboard->addCard(newCard);
            }
        }
    }
    //dungeon init
    if(deck->DungeonZone.size())
    {
        for(unsigned int j = 0; j < deck->DungeonZone.size(); j++)
        {
            string cardID = deck->DungeonZone[j];
            MTGCard * card = MTGCollection()->getCardById(atoi(cardID.c_str()));
            if(card)
            {
                MTGCardInstance * newCard = NEW MTGCardInstance(card, this);
                //Dungeons will be added to sideboard zone...
                sideboard->addCard(newCard);
            }
        }
    }
}

MTGPlayerCards::~MTGPlayerCards()
{
    if(temp->cards.size() > 0){
        for(size_t i = 0; i < library->placeOnTop.size(); i++){
            if(temp->hasCard(library->placeOnTop[i])){
                temp->removeCard(library->placeOnTop[i]); // Fix crash when temp zone contains library place on top cards.
            }
        }
    }
    SAFE_DELETE(library);
    SAFE_DELETE(graveyard);
    SAFE_DELETE(hand);
    SAFE_DELETE(inPlay);
    SAFE_DELETE(stack);
    SAFE_DELETE(removedFromGame);
    SAFE_DELETE(garbage);
    SAFE_DELETE(reveal);
    SAFE_DELETE(sideboard);
    SAFE_DELETE(commandzone);
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
    reveal->beforeBeginPhase();
    sideboard->beforeBeginPhase();
    commandzone->beforeBeginPhase();
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
    reveal->setOwner(player);
    sideboard->setOwner(player);
    commandzone->setOwner(player);
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
        Player * p = dynamic_cast<Player*>(who);
        MTGCardInstance * card = NULL;
        MTGGameZone * z = p->game->library;
        z->shuffle();

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

        library->owner->getObserver()->setLoser(library->owner);
        return;
    }
    MTGCardInstance * toMove = library->cards[library->nb_cards - 1];
    if (!library->miracle)
    {
        library->miracle = true;
        toMove->miracle = true;
    }

    bool prefetch = options[Options::CARDPREFETCHING].number?true:false;
    if (prefetch && WResourceManager::Instance()->IsThreaded())
    {
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
    }

    MTGCardInstance * ret = putInZone(toMove, library, hand);
    if(ret)
    {
        toMove->currentZone = hand;
        ret->currentZone = hand;
        library->lastCardDrawn = ret;
    }
}

void MTGPlayerCards::resetLibrary()
{
    SAFE_DELETE(library);
    library = NEW MTGLibrary();
    library->miracle = false;
}

void MTGPlayerCards::init()
{
    library = NEW MTGLibrary();
    library->miracle = false;
    graveyard = NEW MTGGraveyard();
    hand = NEW MTGHand();
    inPlay = NEW MTGInPlay();
    battlefield = inPlay;

    stack = NEW MTGStack();
    removedFromGame = NEW MTGRemovedFromGame();
    exile = removedFromGame;
    garbage = NEW MTGGameZone();
    garbageLastTurn = garbage;
    reveal = NEW MTGGameZone();
    sideboard = NEW MTGGameZone();
    commandzone = NEW MTGGameZone();
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
    if (card->getCurrentZone() != card->controller()->game->hand && (card->basicAbilities[(int)Constants::EXILEDEATH] || card->basicAbilities[(int)Constants::GAINEDEXILEDEATH] || (card->basicAbilities[(int)Constants::HASDISTURB] && card->alternateCostPaid[ManaCost::MANA_PAID_WITH_RETRACE] == 1)))
    {
         MTGCardInstance* ret = putInZone(card, card->getCurrentZone(), card->owner->game->exile);
         ret->basicAbilities[(int)Constants::GAINEDEXILEDEATH] = 0;
         return ret;
    }
    else if (card->getCurrentZone() != card->controller()->game->hand && (card->basicAbilities[(int)Constants::HANDDEATH] || card->basicAbilities[(int)Constants::GAINEDHANDDEATH]))
    {
         MTGCardInstance* ret = putInZone(card, card->getCurrentZone(), card->owner->game->hand);
         ret->basicAbilities[(int)Constants::GAINEDHANDDEATH] = 0;
         return ret;
    }
    else if (card->getCurrentZone() != card->controller()->game->hand && (card->basicAbilities[(int)Constants::DOUBLEFACEDEATH] || card->basicAbilities[(int)Constants::GAINEDDOUBLEFACEDEATH]))
    {
         MTGCardInstance* ret = putInZone(card, card->getCurrentZone(), card->owner->game->temp);
         ret->basicAbilities[(int)Constants::GAINEDDOUBLEFACEDEATH] = 0;
         return ret;
    }
    else if (card->getCurrentZone() != card->controller()->game->hand && (card->basicAbilities[(int)Constants::INPLAYDEATH] || card->basicAbilities[(int)Constants::INPLAYTAPDEATH]))
    {
        bool toTap = card->basicAbilities[(int)Constants::INPLAYTAPDEATH];
        bool addCounter = card->basicAbilities[(int)Constants::COUNTERDEATH];
        MTGCardInstance* ret = putInZone(card, card->getCurrentZone(), card->owner->game->graveyard);
        ret = putInZone(ret, ret->getCurrentZone(), ret->owner->game->battlefield);
        if(toTap)
            ret->tap(true);
        if(addCounter)
            ret->counters->addCounter(1, 1, false);
        return ret;
    }
    return putInZone(card, card->currentZone, card->owner->game->graveyard);
}

// Moves a card to its owner's sideboard
MTGCardInstance * MTGPlayerCards::putInSideboard(MTGCardInstance * card)
{
    return putInZone(card, card->currentZone, card->owner->game->sideboard);
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
MTGCardInstance * MTGPlayerCards::putInZone(MTGCardInstance * card, MTGGameZone * from, MTGGameZone * to,bool asCopy)
{
    MTGCardInstance * copy = NULL;
    Player * discarderOwner = NULL;
    GameObserver *g = owner->getObserver();
    if (!from || !to)
        return card; //Error check

    int doCopy = 1;
    bool shufflelibrary = false;
    bool bottomoflibrary = false;
    bool inplaytoinplay = false;
    bool ripToken = false;
    if (card->discarderOwner)
        discarderOwner = card->discarderOwner;
    if (g->players[0]->game->battlefield->hasName("Rest in Peace")||g->players[1]->game->battlefield->hasName("Rest in Peace"))
        ripToken = true;
    //Madness or Put in Play...
    for(int i = 0; i < 2; ++i)
    {
        if (card->discarded && (to == g->players[i]->game->graveyard) && (from == g->players[i]->game->hand))
        {
            if(card->basicAbilities[(int)Constants::MADNESS])
                to = g->players[i]->game->exile;
        }
    }
    //Darksteel Colossus, Legacy Weapon ... top priority since we replace destination directly automatically...
    for(int i = 0; i < 2; ++i)
    {
        if ((to == g->players[i]->game->graveyard) && (
        card->basicAbilities[(int)Constants::LIBRARYDEATH]||
        card->basicAbilities[(int)Constants::BOTTOMLIBRARYDEATH]||
        card->basicAbilities[(int)Constants::SHUFFLELIBRARYDEATH]))
        {
            to = g->players[i]->game->library;
            shufflelibrary = card->basicAbilities[(int)Constants::SHUFFLELIBRARYDEATH];
            bottomoflibrary = card->basicAbilities[(int)Constants::BOTTOMLIBRARYDEATH];
        } 
    }
    //Leyline of the Void, Yawgmoth's Agenda... effect...
    for(int i = 0; i < 2; ++i)
    {
        if ((to == g->players[i]->game->graveyard) && (
        (g->players[i]->game->battlefield->hasAbility(Constants::MYGCREATUREEXILER) && card->isCreature()) ||
        (g->players[i]->opponent()->game->battlefield->hasAbility(Constants::OPPGCREATUREEXILER) && card->isCreature())||
        g->players[i]->game->battlefield->hasAbility(Constants::MYGRAVEEXILER) ||
        g->players[i]->opponent()->game->battlefield->hasAbility(Constants::OPPGRAVEEXILER)))
        {
            if ((card->isToken && ripToken))
                to = g->players[i]->game->exile;
            if (!card->isToken)
                to = g->players[i]->game->exile;
        }

        //close the currently open MAIN display.
        if (from == g->players[i]->game->library || from == g->players[i]->game->graveyard || from == g->players[i]->game->exile || from == g->players[i]->game->commandzone || from == g->players[i]->game->sideboard)
        {
            if (g->guiOpenDisplay)
            {
                g->ButtonPressed(g->guiOpenDisplay);
            }
        }

    }
    //all cards that go from the hand to the graveyard is ALWAYS a discard.
    if ((to == g->players[0]->game->graveyard || to == g->players[1]->game->graveyard) && (from == g->players[0]->game->hand || from
        == g->players[1]->game->hand))
    {
        card->discarded = true;
    }

    //When a card is moved from inPlay to inPlay (controller change, for example), it is still the same object
    if ((to == g->players[0]->game->inPlay || to == g->players[1]->game->inPlay) && (from == g->players[0]->game->inPlay || from
                    == g->players[1]->game->inPlay))
    {
        doCopy = 0;
        asCopy = true;//don't send zone change event so it will not destroy the GUI when multiple switching of control...
        inplaytoinplay = true;//try sending different event...
    }

    //Increase the number of time this card has been casted from commandzone to recalculate cost.
    if(from != to && (from == g->players[0]->game->commandzone || from == g->players[1]->game->commandzone)){
        card->numofcastfromcommandzone++;
        card->controller()->numOfCommandCast++;
    }

    if (!(copy = from->removeCard(card, doCopy)))
        return NULL; //ERROR

    for(int i = 0; i < 2; ++i)
    {
        if ((to == g->players[i]->game->battlefield || to == g->players[i]->game->stack) && (
        card->basicAbilities[(int)Constants::LIBRARYDEATH]||
        card->basicAbilities[(int)Constants::BOTTOMLIBRARYDEATH]||
        card->basicAbilities[(int)Constants::SHUFFLELIBRARYDEATH]))
        {
            copy->basicAbilities[Constants::LIBRARYDEATH] = card->basicAbilities[Constants::LIBRARYDEATH];
            copy->basicAbilities[Constants::BOTTOMLIBRARYDEATH] = card->basicAbilities[Constants::BOTTOMLIBRARYDEATH];
            copy->basicAbilities[Constants::SHUFFLELIBRARYDEATH] = card->basicAbilities[Constants::SHUFFLELIBRARYDEATH];
        }
    }

    // Set the mana value used to cast this spell
    if((to == g->players[0]->game->stack || to == g->players[1]->game->stack) && card->getManaCost() && card->getManaCost()->getManaUsedToCast()){
        copy->getManaCost()->setManaUsedToCast(NEW ManaCost());
        copy->getManaCost()->getManaUsedToCast()->copy(card->getManaCost()->getManaUsedToCast());
    }

    if(card->name == copy->name && !card->hasType(Subtypes::TYPE_LEGENDARY) && copy->hasType(Subtypes::TYPE_LEGENDARY)) // This fix issue when cloning a card with nolegend option (e.g. Double Major)
        copy->removeType(Subtypes::TYPE_LEGENDARY);

    // This fix issue types problem when card change zone with different card types than its original version (e.g. double face cards or cards that gained new types before to change zone).
    std::vector<int> realTypes;
    string realName = copy->name;
    if(doCopy && !asCopy && !inplaytoinplay && !card->isMorphed && ((copy->types.size() != card->types.size()) || ((copy->types.size() == card->types.size()) && (!equal(copy->types.begin(), copy->types.end(), card->types.begin())))) ){
        realTypes = copy->types;
        copy->types = card->types;
        copy->mPropertiesChangedSinceLastUpdate = false;
    }

    // This fix issue when card changes zone with different name than its original version (e.g. double face cards).
    if(doCopy && !asCopy && !inplaytoinplay && copy->name != card->name)
        copy->name = card->name;

    // Copy all the counters of the original card... (solving the bug on comparison cards with counter before zone changing events)
    if(card->counters && doCopy && !asCopy && !inplaytoinplay){
        for (unsigned int i = 0; i < card->counters->counters.size(); i++){
            Counter * counter = card->counters->counters[i];
            for(int j = 0; j < counter->nb; j++)
                copy->counters->addCounter(counter->name.c_str(), counter->power, counter->toughness, true);
        }
    }

    // Save the haunted status... (solving the bug on comparison cards with haunted status before zone changing events)
    if(card->has(Constants::ISPREY) && doCopy && !asCopy && !inplaytoinplay)
        copy->basicAbilities[Constants::ISPREY] = 1;

    //Commander is going back to Command Zone, so we recalculate costs according to how many times it has been casted from there.
    if((to == g->players[0]->game->commandzone || to == g->players[1]->game->commandzone) && copy->numofcastfromcommandzone > 0){
        copy->getManaCost()->add(Constants::MTG_COLOR_ARTIFACT,2*copy->numofcastfromcommandzone);
        if(copy->getManaCost()->getAlternative())
            copy->getManaCost()->getAlternative()->add(Constants::MTG_COLOR_ARTIFACT,2*copy->numofcastfromcommandzone);
        if(copy->getManaCost()->getMorph())
            copy->getManaCost()->getMorph()->add(Constants::MTG_COLOR_ARTIFACT,2*copy->numofcastfromcommandzone);
    }

    if (card->miracle)
    {
        copy->miracle = true;
    }
    //reset discarder Owner
    if(to == g->players[0]->game->hand || to == g->players[0]->game->stack || to == g->players[0]->game->library ||
        to == g->players[1]->game->hand || to == g->players[1]->game->stack || to == g->players[1]->game->library)
    {
        card->discarderOwner = NULL;
        copy->discarderOwner = NULL;
    }
    //copy discarderowner
    if (discarderOwner)
    {
        copy->discarderOwner = discarderOwner;
        //change to
        if(to == g->players[0]->game->graveyard)
        {
            if(card->has(Constants::DISCARDTOPLAYBYOPPONENT) && discarderOwner == card->controller()->opponent())
            {
                to = g->players[0]->game->battlefield;
            }
        }
        else if(to == g->players[1]->game->graveyard)
        {
            if(card->has(Constants::DISCARDTOPLAYBYOPPONENT) && discarderOwner == card->controller()->opponent())
            {
                to = g->players[1]->game->battlefield;
            }
        }
    }
    if(from == g->players[0]->game->battlefield || from == g->players[1]->game->battlefield)
        if(to != g->players[0]->game->battlefield || to != g->players[1]->game->battlefield)
        {
            card->kicked = 0;
            copy->kicked = 0;//kicked reset everflowing chalice...
        }
    if (card->discarded)
    {//set discarded for madness...
        if(from == g->players[0]->game->hand || from == g->players[1]->game->hand)
            copy->discarded = true;
        else//turn off discarded if its previous zone is not in hand...
            copy->discarded = false;
    }
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
    //before adding card to zone, if its Melded, we break it apart
    if (from == g->players[0]->game->battlefield || from == g->players[1]->game->battlefield)
    {
        if(to != g->players[0]->game->battlefield || to != g->players[1]->game->battlefield)
        if (copy->previous && copy->previous->MeldedFrom.size() && !copy->isACopier && !copy->isToken)//!copier & !token fix kiki-jiki clones crash
        {
            vector<string> names = split(copy->previous->MeldedFrom, '|');
            MTGCard * cardone = MTGCollection()->getCardByName(names[0], copy->setId);
            MTGCardInstance * cardOne = NEW MTGCardInstance(cardone, copy->owner->game);
            to->addCard(cardOne);
            WEvent * e = NEW WEventZoneChange(cardOne, from, to);
            g->receiveEvent(e);
            MTGCard * cardtwo = MTGCollection()->getCardByName(names[1], copy->setId);
            MTGCardInstance * cardTwo = NEW MTGCardInstance(cardtwo, copy->owner->game);
            to->addCard(cardTwo);
            WEvent * e2 = NEW WEventZoneChange(cardTwo, from, to);
            g->receiveEvent(e2);

            if(from == g->players[0]->game->battlefield)
                g->players[0]->game->temp->addCard(copy);
            if (from == g->players[1]->game->battlefield)
                g->players[1]->game->temp->addCard(copy);
            WEvent * e3 = NEW WEventZoneChange(copy, from, to);
            g->receiveEvent(e3);
            return ret;
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

        if(to == g->players[0]->game->battlefield || to == g->players[1]->game->battlefield)
        {
            if(ret->alias == 109736 && discarderOwner)
            {
                if(discarderOwner == ret->controller()->opponent())
                {
                    AbilityFactory af(g);
                    MTGAbility * dodeCounter = af.parseMagicLine("counter(1/1,2)",-1,NULL,ret);
                    dodeCounter->oneShot = true;
                    dodeCounter->canBeInterrupted = false;
                    dodeCounter->resolve();
                    SAFE_DELETE(dodeCounter);
                }
            }
        }
        //remove exerted if changing controls 
        if((to == g->players[0]->game->battlefield && from == g->players[1]->game->battlefield)||
            (to == g->players[1]->game->battlefield && from == g->players[0]->game->battlefield))
        {
            if(ret->exerted)
                ret->exerted = false;
        }
    }
    if(!asCopy)
    {
        if(shufflelibrary)
            copy->owner->game->library->shuffle();

        if(bottomoflibrary){
            MTGLibrary * library = copy->owner->game->library;
            vector<MTGCardInstance *>oldOrder = library->cards;
            vector<MTGCardInstance *>newOrder;
            newOrder.push_back(copy);
            for(unsigned int k = 0;k < oldOrder.size();++k)
            {
                MTGCardInstance * rearranged = oldOrder[k];
                if(rearranged != copy)
                    newOrder.push_back(rearranged);
            }
            library->cards = newOrder;
        }

        if(copy->has(Constants::ADVENTURE) && copy->alternateCostPaid[ManaCost::MANA_PAID_WITH_ALTERNATIVE] == 1 && //Added to correctly set the adventure cards type on stack.
            (to == g->players[0]->game->stack || to == g->players[1]->game->stack)){
                copy->types.clear();
                if(copy->has(Constants::ASFLASH))
                    copy->types.push_back(Subtypes::TYPE_INSTANT);
                else
                    copy->types.push_back(Subtypes::TYPE_SORCERY);
        }

        if(copy->has(Constants::HASAFTERMATH) && copy->alternateCostPaid[ManaCost::MANA_PAID_WITH_FLASHBACK] == 1 && //Added to correctly set the aftermath cards type on stack.
            (to == g->players[0]->game->stack || to == g->players[1]->game->stack)){
                copy->types.clear();
                if(copy->has(Constants::ASFLASH))
                    copy->types.push_back(Subtypes::TYPE_INSTANT);
                else
                    copy->types.push_back(Subtypes::TYPE_SORCERY);
        }

        if(!copy->has(Constants::NOMOVETRIGGER)){//no trigger when playing these cards (e.g. fake ability cards such as Davriel Conditions, Davriel Offers, Annihilation Rooms)
            WEvent * e = NEW WEventZoneChange(copy, from, to);
            g->receiveEvent(e);
        }

        // Reset the haunted status... (if the creature is moving from battlefield is no longer a prey)
        if(doCopy && !inplaytoinplay && copy->has(Constants::ISPREY))
            copy->basicAbilities[Constants::ISPREY] = 0;

        // Reset original types when card changes zone with different card types than its original version (e.g. double face cards).
        if(doCopy && !inplaytoinplay && realTypes.size()){
            copy->types = realTypes;
            realTypes.clear();
            copy->mPropertiesChangedSinceLastUpdate = false;
        }

        // Reset the original name when card changes zone with different name than its original version (e.g. double face cards).
        if(doCopy && !inplaytoinplay && copy->name != realName)
            copy->name = realName;

        // Erasing counters from copy after the event has been triggered (no counter can survive to a zone changing except the perpetual ones)
        if(doCopy && !inplaytoinplay && copy->counters && copy->counters->mCount > 0){
            for (unsigned int i = 0; i < copy->counters->counters.size(); i++){
                Counter * counter = copy->counters->counters[i];
                for(int j = counter->nb; j > 0; j--){
                    if(counter->name.find("perpetual") == string::npos)
                        copy->counters->removeCounter(counter->name.c_str(), counter->power, counter->toughness, true);
                }
            }
        }
    }
    if(inplaytoinplay)
    {
        WEvent * ep = NEW WEventCardControllerChange(copy);
        g->receiveEvent(ep);
    }
    return ret;

}

void MTGPlayerCards::discardRandom(MTGGameZone * from, MTGCardInstance * _stored)
{
    if (!from->nb_cards)
        return;
    int r = owner->getObserver()->getRandomGenerator()->random() % (from->nb_cards);
    WEvent * e = NEW WEventCardDiscard(from->cards[r]);
    GameObserver * game = owner->getObserver();
    game->receiveEvent(e);
    if(_stored)
        from->cards[r]->discarderOwner = _stored->controller();
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
        //SAFE_DELETE(cards[i]->previous);
        //SAFE_DELETE( cards[i] );
        //cause crashes for generated cards using castcard named card...??? test fix for now
        if(cards[i]->previous)
        {
            delete cards[i]->previous;
            cards[i]->previous = NULL;
        }
        if(cards[i])
        {
            delete cards[i];
            cards[i] = NULL;
        }
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
                copy->storedCard = card->storedCard;
                copy->storedSourceCard = card->storedSourceCard;
                copy->lastController = card->controller();
                copy->previousController = card->controller();
                copy->isCommander = card->isCommander;
                copy->basicAbilities[Constants::GAINEDEXILEDEATH] = card->basicAbilities[Constants::GAINEDEXILEDEATH];
                copy->basicAbilities[Constants::GAINEDHANDDEATH] = card->basicAbilities[Constants::GAINEDHANDDEATH];
                copy->basicAbilities[Constants::GAINEDDOUBLEFACEDEATH] = card->basicAbilities[Constants::GAINEDDOUBLEFACEDEATH];
                copy->basicAbilities[Constants::DUNGEONCOMPLETED] = card->basicAbilities[Constants::DUNGEONCOMPLETED];
                copy->basicAbilities[Constants::PERPETUALDEATHTOUCH] = card->basicAbilities[Constants::PERPETUALDEATHTOUCH];
                copy->basicAbilities[Constants::PERPETUALLIFELINK] = card->basicAbilities[Constants::PERPETUALLIFELINK];
                copy->damageInflictedAsCommander = card->damageInflictedAsCommander;
                copy->numofcastfromcommandzone = card->numofcastfromcommandzone;
                for (int i = 0; i < ManaCost::MANA_PAID_WITH_BESTOW +1; i++)
                    copy->alternateCostPaid[i] = card->alternateCostPaid[i];

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

unsigned int MTGGameZone::countByAlias(int number)
{
    if(!number)
        return 0;
    int result = 0;
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->alias == number)
        {
            result++;
        }
    }
    return result;
}

unsigned int MTGGameZone::countByType(const string &value)
{
    int result = 0;
    int subTypeId = MTGAllCards::findType(value);
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->hasType(subTypeId))
        {
            result++;
        }
        else if(value == "token" && cards[i]->isToken)
            result++;
    }
    return result;
}

unsigned int MTGGameZone::countByCanTarget(TargetChooser * tc)
{
    if(!tc)
        return 0;
    // we don't care if cards have protection.
    bool withoutProtections = true;
    int result = 0;
    for (int i = 0; i < (nb_cards); i++)
    {
        if (tc->canTarget(cards[i], withoutProtections))
        {
            result++;
        }
    }
    return result;
}

unsigned int MTGGameZone::countTotalManaSymbols(TargetChooser * tc, int color)
{
    if (!tc) {
        return 0;
    }
    // we don't care if cards have protection.
    bool withoutProtections = true;
    int result = 0;
    for (int i = 0; i < nb_cards; i++)
    {
        if (tc->canTarget(cards[i], withoutProtections))
        {
            result += cards[i]->getManaCost()->getManaSymbols(color);
        }
    }
    return result;
}

unsigned int MTGGameZone::countDevotion(TargetChooser * tc, int color1, int color2)
{
    if (!tc) {
        return 0;
    }
    // we don't care if cards have protection.
    bool withoutProtections = true;
    int result = 0;
    for (int i = 0; i < nb_cards; i++)
    {
        if (tc->canTarget(cards[i], withoutProtections))
        {
            result += cards[i]->getManaCost()->getManaSymbolsHybridMerged(color1);
        }
        if (tc->canTarget(cards[i], withoutProtections))
        {
            result += cards[i]->getManaCost()->getManaSymbolsHybridMerged(color2);
        }
        result -= cards[i]->getManaCost()->countHybridsNoPhyrexian();
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

bool MTGGameZone::hasAlias(int alias)
{
    for (int i = 0; i < (nb_cards); i++)
    {
        if (cards[i]->alias == alias)
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
    if(this == owner->game->library){
        owner->lastShuffleTurn = owner->getObserver()->turn;
        WEvent * e = NEW WEventplayerShuffled(owner); //Added to trigger an event when a player shuffles his/her library.
        owner->getObserver()->receiveEvent(e);
    }
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

MTGCardInstance * MTGInPlay::getNextLurer(MTGCardInstance * previous)
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
        else if (foundprevious && current->isAttacker() && current->has(Constants::LURE))
        {
            return current;
        }
    }
    return NULL;
}

MTGCardInstance * MTGInPlay::getNextProvoker(MTGCardInstance * previous, MTGCardInstance * thiscard)
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
        else if (foundprevious && current->isAttacker() && thiscard->isProvoked && current->ProvokeTarget)
        {
            if(thiscard == current->ProvokeTarget)
                return current;
        }
    }
    return NULL;
}

MTGCardInstance * MTGInPlay::findAProvoker(MTGCardInstance * thiscard)
{
    for (int i = 0; i < nb_cards; i++)
    {
        MTGCardInstance * current = cards[i];
        if (current->isAttacker() && current->ProvokeTarget)
        {
            if(current->ProvokeTarget == thiscard)
                return current;
        }
    }
    return NULL;
}

MTGCardInstance * MTGInPlay::findALurer()
{
    for (int i = 0; i < nb_cards; i++)
    {
        MTGCardInstance * current = cards[i];
        if (current->isAttacker() && current->has(Constants::LURE))
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
        if (!card->basicAbilities[(int)Constants::DOESNOTUNTAP] && !card->basicAbilities[(int)Constants::SHACKLER])
        {
            if(card->exerted)
            {
                card->exerted = false;
                if (card->frozen >= 1)
                {
                    card->frozen = 0;
                    card->resetUntapping(); // Fix to avoid the untap on frozen card by clicking on them after the untap phase.
                }
            }
            else
            {
                card->exerted = false;
                if (card->frozen < 1)
                {
                    card->attemptUntap();
                }
                if (card->frozen >= 1)
                {
                    card->frozen = 0;
                    card->resetUntapping(); // Fix to avoid the untap on frozen card by clicking on them after the untap phase.
                }
            }
        }
    }
}


MTGGameZone * MTGGameZone::intToZone(int zoneId, Player * p, Player * p2)
{
    if (p2 != p && p2 && (p != p2->opponent()))
    {
        p = p2->opponent();
        //these cases are generally handled this is a edge case fix.
    }
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

    case MY_REVEAL:
        return p->game->reveal;
    case OPPONENT_REVEAL:
        return p->opponent()->game->reveal;
    case REVEAL:
        return p->game->reveal;

    case MY_SIDEBOARD:
        return p->game->sideboard;
    case OPPONENT_SIDEBOARD:
        return p->opponent()->game->sideboard;
    case SIDEBOARD:
        return p->game->sideboard;

    case MY_COMMANDZONE:
        return p->game->commandzone;
    case OPPONENT_COMMANDZONE:
        return p->opponent()->game->commandzone;
    case COMMANDZONE:
        return p->game->commandzone;

    case MY_TEMP:
        return p->game->temp;
    case OPPONENT_TEMP:
        return p->opponent()->game->temp;
    case TEMP:
        return p->game->temp;

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

    case TARGET_CONTROLLER_REVEAL:
        return p2->game->reveal;

    case TARGET_CONTROLLER_SIDEBOARD:
        return p2->game->sideboard;

    case TARGET_CONTROLLER_COMMANDZONE:
        return p2->game->commandzone;

    case TARGET_CONTROLLER_TEMP:
        return p2->game->temp;

    default:
        return NULL;
    }
}

MTGGameZone * MTGGameZone::intToZone(GameObserver *g, int zoneId, MTGCardInstance * source, MTGCardInstance * target)
{
    Player *p = NULL;
    Player *p2 = NULL;

    if (!source && g) //patchwork fix when g is NULL.
        p = g->currentlyActing();
    else if (source)
        p = source->controller();
    if (!target && source) //patchwork fix when source is NULL.
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
    else if (target)
        p2 = target->controller();

    if(!p) { //patchwork fix when p is NULL.
        if(!p2)
            return NULL;
        else 
            p = p2;
    }
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
    case TARGETED_PLAYER_GRAVEYARD:
        if(source->playerTarget)
        return source->playerTarget->game->graveyard;
        else return source->controller()->game->graveyard;

    case TARGET_OWNER_BATTLEFIELD:
        return target->owner->game->inPlay;
    case OWNER_BATTLEFIELD:
        return target->owner->game->inPlay;
    case TARGETED_PLAYER_BATTLEFIELD:
        if(source->playerTarget)
            return source->playerTarget->game->inPlay;
        else return source->controller()->game->inPlay;

    case TARGET_OWNER_HAND:
        return target->owner->game->hand;
    case HAND:
        return target->owner->game->hand;
    case OWNER_HAND:
        return target->owner->game->hand;
    case TARGETED_PLAYER_HAND:
        if(source->playerTarget)
            return source->playerTarget->game->hand;
        else return source->controller()->game->hand;

    case TARGET_OWNER_EXILE:
        return target->owner->game->removedFromGame;
    case EXILE:
        return target->owner->game->removedFromGame;
    case OWNER_EXILE:
        return target->owner->game->removedFromGame;
    case TARGETED_PLAYER_EXILE:
        if(source->playerTarget)
            return source->playerTarget->game->removedFromGame;
        else return source->controller()->game->removedFromGame;

    case TARGET_OWNER_LIBRARY:
        return target->owner->game->library;
    case OWNER_LIBRARY:
        return target->owner->game->library;
    case TARGETED_PLAYER_LIBRARY:
        if(source->playerTarget)
            return source->playerTarget->game->library;
        else return source->controller()->game->library;

    case TARGET_OWNER_STACK:
        return target->owner->game->stack;
    case OWNER_STACK:
        return target->owner->game->stack;
    case TARGETED_PLAYER_STACK:
        if(source->playerTarget)
            return source->playerTarget->game->stack;
        else return source->controller()->game->stack;

    case TARGET_OWNER_REVEAL:
        return target->owner->game->reveal;
    case REVEAL:
        return target->owner->game->reveal;
    case OWNER_REVEAL:
        return target->owner->game->reveal;
    case TARGETED_PLAYER_REVEAL:
        if (source->playerTarget)
            return source->playerTarget->game->reveal;
        else return source->controller()->game->reveal;

    case TARGET_OWNER_SIDEBOARD:
        return target->owner->game->sideboard;
    case SIDEBOARD:
        return target->owner->game->sideboard;
    case OWNER_SIDEBOARD:
        return target->owner->game->sideboard;
    case TARGETED_PLAYER_SIDEBOARD:
        if (source->playerTarget)
            return source->playerTarget->game->sideboard;
        else return source->controller()->game->sideboard;

    case TARGET_OWNER_COMMANDZONE:
        return target->owner->game->commandzone;
    case COMMANDZONE:
        return target->owner->game->commandzone;
    case OWNER_COMMANDZONE:
        return target->owner->game->commandzone;
    case TARGETED_PLAYER_COMMANDZONE:
        if (source->playerTarget)
            return source->playerTarget->game->commandzone;
        else return source->controller()->game->commandzone;

    case TARGET_OWNER_TEMP:
        return target->owner->game->temp;
    case TEMP:
        return target->owner->game->temp;
    case OWNER_TEMP:
        return target->owner->game->temp;
    case TARGETED_PLAYER_TEMP:
        if (source->playerTarget)
            return source->playerTarget->game->temp;
        else return source->controller()->game->temp;

    default:
        return NULL;
    }
 
}

int MTGGameZone::zoneStringToId(string zoneName)
{
    const char * strings[] = { "mygraveyard", "opponentgraveyard", "targetownergraveyard", "targetcontrollergraveyard",
                    "ownergraveyard", "graveyard","targetedpersonsgraveyard",

                    "myinplay", "opponentinplay", "targetownerinplay", "targetcontrollerinplay", "ownerinplay", "inplay","targetedpersonsinplay",

                    "mybattlefield", "opponentbattlefield", "targetownerbattlefield", "targetcontrollerbattlefield",
                    "ownerbattlefield", "battlefield","targetedpersonsbattlefield",

                    "myhand", "opponenthand", "targetownerhand", "targetcontrollerhand", "ownerhand", "hand","targetedpersonshand",

                    "mylibrary", "opponentlibrary", "targetownerlibrary", "targetcontrollerlibrary", "ownerlibrary", "library","targetedpersonslibrary",

                    "myremovedfromgame", "opponentremovedfromgame", "targetownerremovedfromgame",
                    "targetcontrollerremovedfromgame", "ownerremovedfromgame", "removedfromgame","targetedpersonsremovefromgame",

                    "myexile", "opponentexile", "targetownerexile", "targetcontrollerexile", "ownerexile", "exile","targetedpersonsexile",

                    "mystack", "opponentstack", "targetownerstack", "targetcontrollerstack", "ownerstack", "stack","targetedpersonsstack",

                    "myreveal", "opponentreveal", "targetownerreveal", "targetcontrollerreveal", "ownerreveal", "reveal","targetedpersonsreveal",

                    "mysideboard", "opponentsideboard", "targetownersideboard", "targetcontrollersideboard", "ownersideboard", "sideboard","targetedpersonssideboard",

                    "mycommandzone", "opponentcommandzone", "targetownercommandzone", "targetcontrollercommandzone", "ownercommandzone", "commandzone","targetedpersonscommandzone",

                    "mytemp", "opponenttemp", "targetownertemp", "targetcontrollertemp", "ownertemp", "temp","targetedpersonstemp",

    };

    int values[] = { MY_GRAVEYARD, OPPONENT_GRAVEYARD, TARGET_OWNER_GRAVEYARD, TARGET_CONTROLLER_GRAVEYARD, OWNER_GRAVEYARD,
                    GRAVEYARD,TARGETED_PLAYER_GRAVEYARD,

                    MY_BATTLEFIELD, OPPONENT_BATTLEFIELD, TARGET_OWNER_BATTLEFIELD, TARGET_CONTROLLER_BATTLEFIELD,
                    OWNER_BATTLEFIELD, BATTLEFIELD,TARGETED_PLAYER_BATTLEFIELD,

                    MY_BATTLEFIELD, OPPONENT_BATTLEFIELD, TARGET_OWNER_BATTLEFIELD, TARGET_CONTROLLER_BATTLEFIELD,
                    OWNER_BATTLEFIELD, BATTLEFIELD,TARGETED_PLAYER_BATTLEFIELD,

                    MY_HAND, OPPONENT_HAND, TARGET_OWNER_HAND, TARGET_CONTROLLER_HAND, OWNER_HAND, HAND,TARGETED_PLAYER_HAND,

                    MY_LIBRARY, OPPONENT_LIBRARY, TARGET_OWNER_LIBRARY, TARGET_CONTROLLER_LIBRARY, OWNER_LIBRARY, LIBRARY,TARGETED_PLAYER_LIBRARY,

                    MY_EXILE, OPPONENT_EXILE, TARGET_OWNER_EXILE, TARGET_CONTROLLER_EXILE, OWNER_EXILE, EXILE,TARGETED_PLAYER_EXILE,

                    MY_EXILE, OPPONENT_EXILE, TARGET_OWNER_EXILE, TARGET_CONTROLLER_EXILE, OWNER_EXILE, EXILE,TARGETED_PLAYER_EXILE,

                    MY_STACK, OPPONENT_STACK, TARGET_OWNER_STACK, TARGET_CONTROLLER_STACK, OWNER_STACK, STACK,TARGETED_PLAYER_STACK,

                    MY_REVEAL, OPPONENT_REVEAL, TARGET_OWNER_REVEAL, TARGET_CONTROLLER_REVEAL, OWNER_REVEAL, REVEAL,TARGETED_PLAYER_REVEAL,

                    MY_SIDEBOARD, OPPONENT_SIDEBOARD, TARGET_OWNER_SIDEBOARD, TARGET_CONTROLLER_SIDEBOARD, OWNER_SIDEBOARD, SIDEBOARD,TARGETED_PLAYER_SIDEBOARD,

                    MY_COMMANDZONE, OPPONENT_COMMANDZONE, TARGET_OWNER_COMMANDZONE, TARGET_CONTROLLER_COMMANDZONE, OWNER_COMMANDZONE, COMMANDZONE,TARGETED_PLAYER_COMMANDZONE,

                    MY_TEMP, OPPONENT_TEMP, TARGET_OWNER_TEMP, TARGET_CONTROLLER_TEMP, OWNER_TEMP, TEMP,TARGETED_PLAYER_TEMP };

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
    if(z.removedFromGame->cards.size()) {
        out << "exile=";
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
        else if (areaS.compare("removedfromgame") == 0 || areaS.compare("exile") == 0)
        {
            removedFromGame->parseLine(s.substr(limiter+1));
            return true;
        }
    }

    return false;
}

