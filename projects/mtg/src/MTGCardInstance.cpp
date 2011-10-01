/*---------------------------------------------
 Card Instance
 Instance of a given MTGCard in the game
 Although there is only one MTGCard of each type, there can be as much Instances of it as needed in the game
 --------------------------------------------
 */

#include "PrecompiledHeader.h"

#include "MTGCardInstance.h"
#include "CardDescriptor.h"
#include "Counters.h"
#include "Subtypes.h"

using namespace std;

SUPPORT_OBJECT_ANALYTICS(MTGCardInstance)

MTGCardInstance MTGCardInstance::AnyCard = MTGCardInstance();
MTGCardInstance MTGCardInstance::NoCard = MTGCardInstance();

MTGCardInstance MTGCardInstance::ExtraRules[] = { MTGCardInstance(), MTGCardInstance() };

MTGCardInstance::MTGCardInstance() :
    CardPrimitive(), MTGCard(), Damageable(0, 0), view(NULL)
{
    initMTGCI();
}
MTGCardInstance::MTGCardInstance(MTGCard * card, MTGPlayerCards * arg_belongs_to) :
    CardPrimitive(card->data), MTGCard(card), Damageable(0, card->data->getToughness()), view(NULL)
{
    if(arg_belongs_to)
      if(arg_belongs_to->owner)
        observer = arg_belongs_to->owner->getObserver();

    initMTGCI();
    model = card;
    attacker = 0;
    lifeOrig = life;
    origpower = power;
    origtoughness = toughness;
    belongs_to = arg_belongs_to;
    owner = NULL;
    if (arg_belongs_to)
        owner = arg_belongs_to->library->owner;
    lastController = owner;
    defenser = NULL;
    banding = NULL;
    life = toughness;
    preventable = 0;
    thatmuch = 0;
    flanked = 0;
    castMethod = Constants::NOT_CAST;
}

void MTGCardInstance::copy(MTGCardInstance * card)
{
    MTGCard * source = card->model;
    CardPrimitive * data = source->data;

    basicAbilities = card->basicAbilities;
    for (size_t i = 0; i < data->types.size(); i++)
    {
        types.push_back(data->types[i]);
    }

    colors = data->colors;

    manaCost.copy(data->getManaCost());

    setText(""); //The text is retrieved from the data anyways
    setName(data->name);

    power = data->power;
    toughness = data->toughness;
    life = toughness;
    lifeOrig = life;
    magicText = data->magicText;
    spellTargetType = data->spellTargetType;
    alias = data->alias;

    //Now this is dirty...
    int backupid = mtgid;
    mtgid = source->getId();
    Spell * spell = NEW Spell(observer, this);
    observer = card->observer;
    AbilityFactory af(observer);
    af.addAbilities(observer->mLayers->actionLayer()->getMaxId(), spell);
    delete spell;
    mtgid = backupid;
}

MTGCardInstance::~MTGCardInstance()
{
    SAFE_DELETE(counters);
	if (previous != NULL)
	{
		//DebugTrace("MTGCardInstance::~MTGCardInstance():  deleting " << ToHex(previous));
	    SAFE_DELETE(previous);
	}
}

int MTGCardInstance::init()
{
    MTGCard::init();
    CardPrimitive::init();
    data = this;
    X = 0;
    return 1;
}

void MTGCardInstance::initMTGCI()
{
    sample = "";
    model = NULL;
    isToken = false;
    lifeOrig = 0;
    doDamageTest = 1;
    belongs_to = NULL;
    tapped = 0;
    untapping = 0;
    frozen = 0;
    fresh = 0;
    isMultiColored = 0;
    isLeveler = 0;
    enchanted = false;
    CDenchanted = 0;
    CDdamaged = 0;
    blinked = false;
    isExtraCostTarget = false;
    morphed = false;
    turningOver = false;
    isMorphed = false;
    isPhased = false;
    phasedTurn = -1;
    didattacked = 0;
    didblocked = 0;
    notblocked = 0;
    sunburst = 0;
    equipment = 0;
    auras = 0;
    damageToOpponent = false;
    damageToController = false;
    wasDealtDamage = false;
    isDualWielding = false;
    suspended = false;
    castMethod = Constants::NOT_CAST;
    mPropertiesChangedSinceLastUpdate = false;
    kicked = 0;


    for (int i = 0; i < ManaCost::MANA_PAID_WITH_RETRACE +1; i++)
        alternateCostPaid[i] = 0;

    paymenttype = MTGAbility::PUT_INTO_PLAY;
    reduxamount = 0;
    summoningSickness = 1;
    preventable = 0;
    thatmuch = 0;
    flanked = 0;
    target = NULL;
    type_as_damageable = DAMAGEABLE_MTGCARDINSTANCE;
    banding = NULL;
    owner = NULL;
    counters = NEW Counters(this);
    previousZone = NULL;
    previous = NULL;
    next = NULL;
    lastController = NULL;
    regenerateTokens = 0;
    blocked = false;
    currentZone = NULL;
    data = this; //an MTGCardInstance point to itself for data, allows to update it without killing the underlying database item

    if (basicAbilities[(int)Constants::CHANGELING])
    {//if the card is a changeling, it gains all creature subtypes
        const vector<string> values = Subtypes::subtypesList->getValuesById();
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (!Subtypes::subtypesList->isSubtypeOfType(i,Subtypes::TYPE_CREATURE))
                continue;

            //Don' want to send any event to the gameObserver inside of initMCGI, so calling the parent addType method instead of mine
            CardPrimitive::addType(i);
        }
    }

    int colored = 0;

    for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
    {
        if (this->hasColor(i))
            ++colored;
    }
    if (colored > 1)
    {
        isMultiColored = 1;
    }

    if(previous && previous->morphed && !turningOver)
    {
        morphed = true;
        isMorphed = true;
    }
}

const string MTGCardInstance::getDisplayName() const
{
    return getName();
}

void MTGCardInstance::addType(int type)
{
    bool before = hasType(type);
    CardPrimitive::addType(type);

    if (!before)
        mPropertiesChangedSinceLastUpdate = true;

    // If the card name is not set, set it to the type.
    //This is a hack for "transform", used in combination with "losesubtypes" on Basic Lands
    //See removeType below
    if (!name.length())
        setName(Subtypes::subtypesList->find(type));

    WEvent * e = NEW WEventCardChangeType(this, type, before, true);
    if (observer)
        observer->receiveEvent(e);
    else
        SAFE_DELETE(e);
}

void MTGCardInstance::addType(char * type_text)
{
    setSubtype(type_text);
}

void MTGCardInstance::setType(const char * type_text)
{
    setSubtype(type_text);
}

void MTGCardInstance::setSubtype(string value)
{
    int id = Subtypes::subtypesList->find(value);
    addType(id);
}
int MTGCardInstance::removeType(string value, int removeAll)
{
    int id = Subtypes::subtypesList->find(value);
    return removeType(id, removeAll);
}

int MTGCardInstance::removeType(int id, int removeAll)
{
    bool before = hasType(id);
    int result = CardPrimitive::removeType(id, removeAll);
    bool after = hasType(id);
    if (before != after)
    {
        mPropertiesChangedSinceLastUpdate = true;
        // Basic lands have the same name as their subtypes, and TargetChoosers don't make a distinction between name and type,
        // so if we remove a subtype "Forest", we also need to remove its name.
        //This means the card might lose its name, but usually when we force remove a type, we add another one just after that.
        //see "AddType" above which force sets a name if necessary
        if (name.compare(Subtypes::subtypesList->find(id)) == 0)
            setName("");
    }
    WEvent * e = NEW WEventCardChangeType(this, id, before, after);
    if (observer)
        observer->receiveEvent(e);
    else
        SAFE_DELETE(e);
    return result;
}

int MTGCardInstance::isInPlay(GameObserver* game)
{
    for (int i = 0; i < 2; i++)
    {
        MTGGameZone * zone = game->players[i]->game->inPlay;
        if (zone->hasCard(this))
            return 1;
    }
    return 0;
}

int MTGCardInstance::afterDamage()
{
    if (!doDamageTest)
        return 0;
    doDamageTest = 0;
    if (!isCreature())
        return 0;
    if (life <= 0 && isInPlay(observer))
    {
        return destroy();
    }
    return 0;
}

int MTGCardInstance::bury()
{
    Player * p = controller();
    if (basicAbilities[(int)Constants::EXILEDEATH])
    {
        p->game->putInZone(this, p->game->inPlay, owner->game->exile);
        return 1;
    }
    if (!basicAbilities[(int)Constants::INDESTRUCTIBLE])
    {
        p->game->putInZone(this, p->game->inPlay, owner->game->graveyard);
        return 1;
    }
    return 0;
}
int MTGCardInstance::destroy()
{
    if (!triggerRegenerate())
        return bury();
    return 0;
}

MTGGameZone * MTGCardInstance::getCurrentZone()
{
    return currentZone;
}

int MTGCardInstance::has(int basicAbility)
{
    return basicAbilities[basicAbility];
}

ManaCost* MTGCardInstance::getReducedManaCost()
{
    return &reducedCost;
}
ManaCost* MTGCardInstance::getIncreasedManaCost()
{
    return &increasedCost;
}

//sets card as attacked and sends events
void MTGCardInstance::eventattacked()
{
    didattacked = 1;
    WEvent * e = NEW WEventCardAttacked(this);
    observer->receiveEvent(e);
}

//sets card as attacked alone and sends events
void MTGCardInstance::eventattackedAlone()
{
    WEvent * e = NEW WEventCardAttackedAlone(this);
    observer->receiveEvent(e);
}

//sets card as attacked and sends events
void MTGCardInstance::eventattackednotblocked()
{
    didattacked = 1;
    WEvent * e = NEW WEventCardAttackedNotBlocked(this);
    observer->receiveEvent(e);
}

//sets card as attacked and sends events
void MTGCardInstance::eventattackedblocked(MTGCardInstance * opponent)
{
    didattacked = 1;
    WEvent * e = NEW WEventCardAttackedBlocked(this,opponent);
    observer->receiveEvent(e);
}

//sets card as blocking and sends events
void MTGCardInstance::eventblocked(MTGCardInstance * opponent)
{
    didblocked = 1;
    WEvent * e = NEW WEventCardBlocked(this,opponent);
    observer->receiveEvent(e);
}

//Taps the card
void MTGCardInstance::tap()
{
    if (tapped)
        return;
    tapped = 1;
    WEvent * e = NEW WEventCardTap(this, 0, 1);
    observer->receiveEvent(e);
}

void MTGCardInstance::untap()
{
    if (!tapped)
        return;
    tapped = 0;
    WEvent * e = NEW WEventCardTap(this, 1, 0);
    observer->receiveEvent(e);
}

void MTGCardInstance::setUntapping()
{
    untapping = 1;
}

int MTGCardInstance::isUntapping()
{
    return untapping;
}

//Tries to Untap the card
void MTGCardInstance::attemptUntap()
{
    if (untapping)
    {
        untap();
        untapping = 0;
    }
}

//Tells if the card is tapped or not
int MTGCardInstance::isTapped()
{
    return tapped;
}

int MTGCardInstance::regenerate()
{
    if (has(Constants::CANTREGEN))
        return 0;
    return ++regenerateTokens;
}

int MTGCardInstance::triggerRegenerate()
{
    if (!regenerateTokens)
        return 0;
    if (has(Constants::CANTREGEN))
        return 0;
    regenerateTokens--;
    tap();
    if(isCreature())
    {
        life = toughness;
        initAttackersDefensers();
        if (life < 1)
            return 0; //regeneration didn't work (wither ?)
    }
    return 1;
}

int MTGCardInstance::initAttackersDefensers()
{
    setAttacker(0);
    setDefenser(NULL);
    banding = NULL;
    blockers.clear();
    blocked = 0;
    didattacked = 0;
    didblocked = 0;
    return 1;
}

//Function to cleanup flags on a card (generally at the end of the turn)
int MTGCardInstance::cleanup()
{
    initAttackersDefensers();
    if (!observer || observer->currentPlayer == controller())
    {
        summoningSickness = 0;
    }
    if (previous && !previous->stillInUse())
    {
        //DebugTrace("MTGCardInstance::cleanup():  deleting " << ToHex(previous));
        SAFE_DELETE(previous);
    }
    regenerateTokens = 0;
    preventable = 0;
    thatmuch = 0;
    return 1;
}

int MTGCardInstance::stillInUse()
{
    if (observer->mLayers->actionLayer()->stillInUse(this))
        return 1;
    if (!previous)
        return 0;

    return previous->stillInUse();
}

/* Summoning Sickness
 * 212.3f A creature's activated ability with the tap symbol or the untap symbol in its activation cost
 * can't be played unless the creature has been under its controller's control since the start of his or
 * her most recent turn. A creature can't attack unless it has been under its controller's control
 * since the start of his or her most recent turn. This rule is informally called the "summoning
 * sickness" rule. Ignore this rule for creatures with haste (see rule 502.5).
 */
int MTGCardInstance::hasSummoningSickness()
{
    if (!summoningSickness)
        return 0;
    if (basicAbilities[(int)Constants::HASTE])
        return 0;
    if (!isCreature())
        return 0;
    return 1;
}

MTGCardInstance * MTGCardInstance::changeController(Player * newController)
{
    Player * originalOwner = controller();
    if (originalOwner == newController)
        return this;
    MTGCardInstance * copy = originalOwner->game->putInZone(this, originalOwner->game->inPlay, newController->game->inPlay);
    copy->summoningSickness = 1;
    return copy;
}

Player * MTGCardInstance::controller()
{
    return lastController;
}

int MTGCardInstance::canAttack()
{
    if (tapped)
        return 0;
    if (hasSummoningSickness())
        return 0;
    if ((basicAbilities[(int)Constants::DEFENSER] || basicAbilities[(int)Constants::CANTATTACK]) && !basicAbilities[(int)Constants::CANATTACK])
        return 0;
    if (!isCreature())
        return 0;
    if (!isInPlay(observer))
        return 0;
    return 1;
}

int MTGCardInstance::addToToughness(int value)
{
    toughness += value;
    life += value;
    doDamageTest = 1;
    return 1;
}

int MTGCardInstance::setToughness(int value)
{
    toughness = value;
    life = value;
    doDamageTest = 1;
    return 1;
}

int MTGCardInstance::canBlock()
{
    if (tapped)
        return 0;
    if (basicAbilities[(int)Constants::CANTBLOCK])
        return 0;
    if (!isCreature())
        return 0;
    if (!isInPlay(observer))
        return 0;
    return 1;
}

int MTGCardInstance::canBlock(MTGCardInstance * opponent)
{
    if (!canBlock())
        return 0;
    if (!opponent)
        return 1;
    if (!opponent->isAttacker())
        return 0;
    // Comprehensive rule 502.7f : If a creature with protection attacks, it can't be blocked by creatures that have the stated quality.
    if (opponent->protectedAgainst(this))
        return 0;
    if (opponent->cantBeBlockedBy(this))
        return 0;
    if (opponent->basicAbilities[(int)Constants::UNBLOCKABLE])
        return 0;
    if (opponent->basicAbilities[(int)Constants::ONEBLOCKER] && opponent->blocked)
        return 0;
    if(opponent->basicAbilities[(int)Constants::STRONG] && power < opponent->power)
        return 0;
    if(this->basicAbilities[(int)Constants::WEAK] && power < opponent->power)
        return 0;
    if (opponent->basicAbilities[(int)Constants::FEAR] && !(this->hasType(Subtypes::TYPE_ARTIFACT) || this->hasColor(Constants::MTG_COLOR_BLACK)))
        return 0;

    //intimidate
    if (opponent->basicAbilities[(int)Constants::INTIMIDATE] && !(this->hasType(Subtypes::TYPE_ARTIFACT)))
    {
        int canblock = 0;
        for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
        {
            if (this->hasColor(i) && opponent->hasColor(i))
            {
                canblock = 1;
                break;
            }
        }
        if (!canblock)
            return 0;
    }

    if (opponent->basicAbilities[(int)Constants::FLYING] && !(basicAbilities[(int)Constants::FLYING] || basicAbilities[(int)Constants::REACH]))
        return 0;
    //Can block only creatures with flying if has cloud
    if (basicAbilities[(int)Constants::CLOUD] && !(opponent->basicAbilities[(int)Constants::FLYING]))
        return 0;
    // If opponent has shadow and a creature does not have either shadow or reachshadow it cannot be blocked
    if (opponent->basicAbilities[(int)Constants::SHADOW] && !(basicAbilities[(int)Constants::SHADOW]
                    || basicAbilities[(int)Constants::REACHSHADOW]))
        return 0;
    // If opponent does not have shadow and a creature has shadow it cannot be blocked
    if (!opponent->basicAbilities[(int)Constants::SHADOW] && basicAbilities[(int)Constants::SHADOW])
        return 0;
    if (opponent->basicAbilities[(int)Constants::HORSEMANSHIP] && !basicAbilities[(int)Constants::HORSEMANSHIP])
        return 0;
    if (opponent->basicAbilities[(int)Constants::SWAMPWALK] && controller()->game->inPlay->hasType("swamp"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::FORESTWALK] && controller()->game->inPlay->hasType("forest"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::ISLANDWALK] && controller()->game->inPlay->hasType("island"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::MOUNTAINWALK] && controller()->game->inPlay->hasType("mountain"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::PLAINSWALK] && controller()->game->inPlay->hasType("plains"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::LEGENDARYWALK] && controller()->game->inPlay->hasPrimaryType("legendary","land"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::DESERTWALK] && controller()->game->inPlay->hasSpecificType("land","desert"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::SNOWSWAMPWALK] && controller()->game->inPlay->hasSpecificType("snow","swamp"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::SNOWFORESTWALK] && controller()->game->inPlay->hasSpecificType("snow","forest"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::SNOWISLANDWALK] && controller()->game->inPlay->hasSpecificType("snow","island"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::SNOWMOUNTAINWALK] && controller()->game->inPlay->hasSpecificType("snow","mountain"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::SNOWPLAINSWALK] && controller()->game->inPlay->hasSpecificType("snow","plains"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::SNOWWALK] && controller()->game->inPlay->hasPrimaryType("snow","land"))
        return 0;
    if (opponent->basicAbilities[(int)Constants::NONBASICWALK] && controller()->game->inPlay->hasTypeButNotType("land","basic"))
        return 0;
    return 1;
}

JQuadPtr MTGCardInstance::getIcon()
{
    return WResourceManager::Instance()->RetrieveCard(this, CACHE_THUMB);
}

MTGCardInstance * MTGCardInstance::getNextPartner()
{
    MTGInPlay * inplay = controller()->game->inPlay;
    MTGCardInstance * bandingPartner = inplay->getNextAttacker(banding);
    while (bandingPartner)
    {
        if (basicAbilities[(int)Constants::BANDING] || bandingPartner->basicAbilities[(int)Constants::BANDING])
            return bandingPartner;
        bandingPartner = inplay->getNextAttacker(bandingPartner);
    }
    return NULL;
}

int MTGCardInstance::DangerRanking()
{
    int danger;
    int result;
    danger = 0;
    result = 0;
    result += power;
    result += toughness;
    result += getManaCost()->getConvertedCost();
    for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++)
    {
        if (basicAbilities[j])
        {
            result += 1;
        }
    }
    if (result > 1)
        danger += 1;
    if (result > 2)
        danger += 1;
    if (result > 4)
        danger += 1;
    if (result > 6)
        danger += 1;
    if (result > 10)
        danger += 1;
    return danger;
}

int MTGCardInstance::setAttacker(int value)
{
    Targetable * previousTarget = NULL;
    Targetable * target = NULL;
    Player * p = controller()->opponent();
    if (value)
        target = p;
    if (attacker)
        previousTarget = p;
    attacker = value;
    WEvent * e = NEW WEventCreatureAttacker(this, previousTarget, target);
    if (observer)
        observer->receiveEvent(e);
    else
        SAFE_DELETE(e);
    return 1;
}

int MTGCardInstance::toggleAttacker()
{
    if (!attacker)
    {
        //if (!basicAbilities[Constants::VIGILANCE]) tap();
        setAttacker(1);
        return 1;
    }
    else
    {
        //untap();
        setAttacker(0);
        return 1;
    }
    return 0;
}

int MTGCardInstance::isAttacker()
{
    return attacker;
}

MTGCardInstance * MTGCardInstance::isDefenser()
{
    return defenser;
}

int MTGCardInstance::nbOpponents()
{
    int result = 0;
    MTGCardInstance* opponent = getNextOpponent();
    while (opponent)
    {
        result++;
        opponent = getNextOpponent(opponent);
    }
    return result;
}

int MTGCardInstance::raiseBlockerRankOrder(MTGCardInstance * blocker)
{
    list<MTGCardInstance *>::iterator it1 = find(blockers.begin(), blockers.end(), blocker);
    list<MTGCardInstance *>::iterator it2 = it1;
    if (blockers.begin() == it2)
        ++it2;
    else
        --it2;

    std::iter_swap(it1, it2);
    WEvent* e = NEW WEventCreatureBlockerRank(*it1, *it2, this);
    if (observer)
        observer->receiveEvent(e);
    else
        SAFE_DELETE(e);
    //delete(e);
    return 1;
}

int MTGCardInstance::getDefenserRank(MTGCardInstance * blocker)
{
    int result = 0;
    for (list<MTGCardInstance *>::iterator it1 = blockers.begin(); it1 != blockers.end(); ++it1)
    {
        result++;
        if ((*it1) == blocker)
            return result;
    }
    return 0;
}
;

int MTGCardInstance::removeBlocker(MTGCardInstance * blocker)
{
    blockers.remove(blocker);
    if (!blockers.size())
    {
        blocked = false;
    }
    return 1;
}

int MTGCardInstance::addBlocker(MTGCardInstance * blocker)
{
    blockers.push_back(blocker);
    blocked = true;
    return 1;
}

//Returns opponents to this card for this turn. This * should * take into account banding
MTGCardInstance * MTGCardInstance::getNextOpponent(MTGCardInstance * previous)
{
    int foundprevious = 0;
    if (!previous)
        foundprevious = 1;
    if (attacker)
    {
        MTGInPlay * inPlay = observer->opponent()->game->inPlay;
        for (int i = 0; i < inPlay->nb_cards; i++)
        {
            MTGCardInstance * current = inPlay->cards[i];
            if (current == previous)
            {
                foundprevious = 1;
            }
            else if (foundprevious)
            {
                MTGCardInstance * defensersOpponent = current->isDefenser();
                if (defensersOpponent && (defensersOpponent == this || (banding && defensersOpponent->banding == banding)))
                {
                    return current;
                }
            }
        }
    }
    else if (defenser)
    {
        MTGInPlay * inPlay = observer->currentPlayer->game->inPlay;
        for (int i = 0; i < inPlay->nb_cards; i++)
        {
            MTGCardInstance * current = inPlay->cards[i];
            if (current == previous)
            {
                foundprevious = 1;
            }
            else if (foundprevious)
            {
                if (defenser == current || (current->banding && defenser->banding == current->banding))
                {
                    return current;
                }
            }
        }
    }
    return NULL;
}

int MTGCardInstance::setDefenser(MTGCardInstance * opponent)
{
    if (defenser)
    {
        if (observer->players[0]->game->battlefield->hasCard(defenser) || observer->players[1]->game->battlefield->hasCard(defenser))
        {
            defenser->removeBlocker(this);
        }
    }
    WEvent * e = NULL;
    if (defenser != opponent)
        e = NEW WEventCreatureBlocker(this, defenser, opponent);
    defenser = opponent;
    if (defenser)
        defenser->addBlocker(this);
    if (e)
        observer->receiveEvent(e);
    return 1;
}

int MTGCardInstance::toggleDefenser(MTGCardInstance * opponent)
{
    if (canBlock())
    {
        if (canBlock(opponent))
        {
            setDefenser(opponent);
            didblocked = 1;
            if (opponent && opponent->controller()->isAI() && opponent->controller()->playMode != Player::MODE_TEST_SUITE)
            {
                if(opponent->view != NULL)
                {
                //todo: quote wololo "change this into a cool blinking effects when opposing creature has cursor focus."
                    opponent->view->actZ += .8f;
                    opponent->view->actT -= .2f;
                }
            }
            if (!opponent)
                didblocked = 0;
            return 1;
        }
    }
    return 0;
}

bool MTGCardInstance::matchesCastFilter(int castFilter) {
    if(castFilter == Constants::CAST_DONT_CARE)
        return true; //everything
    if(castFilter == Constants::CAST_ALL)
        return (castMethod != Constants::NOT_CAST); //everything except "not cast"
    if (castFilter == Constants::CAST_ALTERNATE && castMethod > Constants::CAST_NORMALLY)
        return true; //all alternate casts
    return (castFilter == castMethod);
};

int MTGCardInstance::addProtection(TargetChooser * tc)
{
    tc->targetter = NULL;
    protections.push_back(tc);
    return protections.size();
}

int MTGCardInstance::removeProtection(TargetChooser * tc, int erase)
{
    for (size_t i = 0; i < protections.size(); i++)
    {
        if (protections[i] == tc)
        {
            if (erase)
                delete (protections[i]);
            protections.erase(protections.begin() + i);
            return 1;
        }
    }
    return 0;
}

int MTGCardInstance::protectedAgainst(MTGCardInstance * card)
{
    //Basic protections
    for (int i = Constants::PROTECTIONGREEN; i <= Constants::PROTECTIONWHITE; i++)
    {
        if (basicAbilities[i] && card->hasColor(i - Constants::PROTECTIONGREEN + Constants::MTG_COLOR_GREEN))
            return 1;
    }

    //General protections
    for (size_t i = 0; i < protections.size(); i++)
    {
        if (protections[i]->canTarget(card))
            return 1;
    }
    return 0;
}

int MTGCardInstance::addCantBeTarget(TargetChooser * tc)
{
    tc->targetter = NULL;
    canttarget.push_back(tc);
    return canttarget.size();
}

int MTGCardInstance::removeCantBeTarget(TargetChooser * tc, int erase)
{
    for (size_t i = 0; i < canttarget.size(); i++)
    {
        if (canttarget[i] == tc)
        {
            if (erase)
                delete (canttarget[i]);
            canttarget.erase(canttarget.begin() + i);
            return 1;
        }
    }
    return 0;
}

int MTGCardInstance::CantBeTargetby(MTGCardInstance * card)
{
    for (size_t i = 0; i < canttarget.size(); i++)
    {
        if (canttarget[i]->canTarget(card))
            return 1;
    }
    return 0;
}

int MTGCardInstance::addCantBeBlockedBy(TargetChooser * tc)
{
    cantBeBlockedBys.push_back(tc);
    return cantBeBlockedBys.size();
}

int MTGCardInstance::removeCantBeBlockedBy(TargetChooser * tc, int erase)
{
    for (size_t i = 0; i < cantBeBlockedBys.size(); i++)
    {
        if (cantBeBlockedBys[i] == tc)
        {
            if (erase)
                delete (cantBeBlockedBys[i]);
            cantBeBlockedBys.erase(cantBeBlockedBys.begin() + i);
            return 1;
        }
    }
    return 0;
}

int MTGCardInstance::cantBeBlockedBy(MTGCardInstance * card)
{
    for (size_t i = 0; i < cantBeBlockedBys.size(); i++)
    {
        if (cantBeBlockedBys[i]->canTarget(card))
            return 1;
    }
    return 0;
}

/* Choose a sound sample to associate to that card */
JSample * MTGCardInstance::getSample()
{
    JSample * js;

    if (sample.size())
        return WResourceManager::Instance()->RetrieveSample(sample);

    for (int i = types.size() - 1; i > 0; i--)
    {
        string type = Subtypes::subtypesList->find(types[i]);
        std::transform(type.begin(), type.end(), type.begin(), ::tolower);
        type = type + ".wav";
        js = WResourceManager::Instance()->RetrieveSample(type);
        if (js)
        {
            sample = string(type);
            return js;
        }
    }

    if (basicAbilities.any())
    {
        for (size_t x = 0; x < basicAbilities.size(); ++x)
        {
            if (!basicAbilities.test(x))
                continue;
            string type = Constants::MTGBasicAbilities[x];
            type = type + ".wav";
            js = WResourceManager::Instance()->RetrieveSample(type);
            if (js)
            {
                sample = string(type);
                return js;
            }
        }
    }

    string type = "";
    if(!types.size())
        return NULL;   
    type = Subtypes::subtypesList->find(types[0]);
    type.append(".wav");
    js = WResourceManager::Instance()->RetrieveSample(type);
    if (js)
    {
        sample = string(type);
        return js;
    }

    return NULL;
}

int MTGCardInstance::stepPower(CombatStep step)
{
    switch (step)
    {
    case FIRST_STRIKE:
    case END_FIRST_STRIKE:
        if (has(Constants::FIRSTSTRIKE) || has(Constants::DOUBLESTRIKE))
            return MAX(0, power);
        else
            return 0;
    case DAMAGE:
    case END_DAMAGE:
    default:
        if (has(Constants::FIRSTSTRIKE) && !has(Constants::DOUBLESTRIKE))
            return 0;
        else
            return MAX(0, power);
    }
}

std::ostream& MTGCardInstance::toString(std::ostream& out) const
{
    return out << name;
}

std::ostream& operator<<(std::ostream& out, const MTGCardInstance& c)
{
    return c.toString(out);
}
