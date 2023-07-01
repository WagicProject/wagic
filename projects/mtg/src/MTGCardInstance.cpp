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


#include "AIPlayerBaka.h"

using namespace std;

SUPPORT_OBJECT_ANALYTICS(MTGCardInstance)

MTGCardInstance MTGCardInstance::AnyCard = MTGCardInstance();
MTGCardInstance MTGCardInstance::NoCard = MTGCardInstance();

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
    willattackplayer = 0;
    willattackpw = 0;
    lifeOrig = life;
    origpower = power;
    basepower = origpower;
    pbonus = 0;
    origtoughness = toughness;
    basetoughness = origtoughness;
    tbonus = 0;
    belongs_to = arg_belongs_to;
    owner = NULL;
    if (arg_belongs_to)
        owner = arg_belongs_to->library->owner;
    lastController = owner;
    previousController = owner;
    defenser = NULL;
    banding = NULL;
    life = toughness;
    preventable = 0;
    thatmuch = 0;
    flanked = 0;
    castMethod = Constants::NOT_CAST;
    isSettingBase = 0;
    isCDA = false;
    isSwitchedPT = false;
    isACopier = false;
    bypassTC = false;
    discarded = false;
    copiedID = getId();
    copiedSetID = 0;
    LKIpower = power;
    LKItoughness = toughness;
    forcedBorderA = 0;
    forcedBorderB = 0;
    myconvertedcost = getManaCost()->getConvertedCost();
    revealedLast = NULL;
    MadnessPlay = false;
    backSide = card->data->backSide;
    partner = card->data->partner;
}

  MTGCardInstance * MTGCardInstance::createSnapShot()
    {
        //the below section of code was changed without all possible side effects checked
        //the reason was becuase while NEW MTGCardInstance(*this); does indeed return an exact copy
        //the lower layer cardprimitive data is pointed to from the original source.
        //this would cause cards like lotus bloom, which contain a restriction, to already has deleted the restriction
        //which belonged to the original card before getting to the safe_delete, 
        //it was leaving a dangling pointer which leads to
        //a total crash on "cleanup()" calls from garbage zone.
        //snapshots are created for extra cost, they are used for abilities contained after the cost through storecard variable.
        //TODO:fix this correctly. I want this to use an exact copy of the card in its current state for stored.
        //making it safe_delete these "copies" leads to the same crash, as they are still pointing to the original data.
        MTGCardInstance * snapShot = this;
        //below is how we used to handle this.
       // MTGCardInstance * snapShot = NEW MTGCardInstance(*this);
        //snapShot->previous = NULL;
       // snapShot->counters = NEW Counters(snapShot);
        //controller()->game->garbage->addCard(snapShot);
        return snapShot;
    }

void MTGCardInstance::copy(MTGCardInstance * card, bool nolegend)
{
    MTGCard * source = NULL;
    if(card->isACopier && card->copiedID)
    {
        source = MTGCollection()->getCardById(card->copiedID);
    }
    else if(card->isToken || card->hasCopiedToken)
    {
        if(card->getMTGId() > 0)//not generated token
            source = MTGCollection()->getCardById(card->getMTGId());
        else
        {
            if(card->tokCard){  // Fix a possible crash when tokCard is null...
                source = card->tokCard;
                source->data = card->tokCard; //?wtf
            } else {
                source = card;
                source->data = card;
            }
        }
    }
    else
         source = MTGCollection()->getCardById(card->copiedID);

    if(!source)
        source = card;

    CardPrimitive * data = source->data;
    //basicAbilities = data->basicAbilities;
    for(unsigned int j = 0; j < data->basicAbilities.size(); j++)
    {
        if(data->basicAbilities[j])
            basicAbilities[j] = data->basicAbilities[j];
    }
    types.clear();//reset types.. fix copying man lands... the copier becomes an unanimated land...
    for (size_t i = 0; i < data->types.size(); i++)
    {
        if(!(nolegend && data->types[i] == Subtypes::TYPE_LEGENDARY)) // Check if the copy has to be legendary or not. (e.g. Echoing Equation)
            types.push_back(data->types[i]);
    }

    colors = data->colors;

    manaCost.copy(data->getManaCost());

    setText(data->text); //The text is retrieved from the data anyways
    setName(data->name);
    backSide = data->backSide;
    partner = data->partner;
    power = data->power;//layer 7a
    toughness = data->toughness;//layer 7a
    power += pbonus;//layer 7b
    toughness += tbonus;//layer 7b
    life = toughness;
    lifeOrig = life;
    magicText = data->magicText;
    spellTargetType = data->spellTargetType;
    alias = data->alias;
    copiedID = card->copiedID;
    copiedSetID = card->setId;
    doubleFaced = data->doubleFaced;
    AICustomCode = data->AICustomCode;
    CrewAbility = data->CrewAbility;
    ModularValue = data->ModularValue;
    PhasedOutAbility = data->PhasedOutAbility;
    origpower = card->origpower;//for flip
    origtoughness = card->origtoughness;//for flip
    TokenAndAbility = card->TokenAndAbility;//token andAbility
    tokCard = card->tokCard;

    //Now this is dirty...
    int backupid = mtgid;
    int castMethodBackUP = this->castMethod;
    mtgid = source->getId();
    MTGCardInstance * oldStored = this->storedSourceCard;

    mtgid = backupid; // found a way :)

    castMethod = castMethodBackUP;
    backupTargets = this->backupTargets;
    storedCard = oldStored;
    miracle = false;

    
    //add event here copied a card...
    WEvent * e = NEW WEventCardCopiedACard(this);
    getObserver()->receiveEvent(e);
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
    castX = 0;
    setX = -1;
    return 1;
}

void MTGCardInstance::initMTGCI()
{
    X = 0;
    setX = -1;
    sample = "";
    model = NULL;
    isToken = false;
    lifeOrig = 0;
    doDamageTest = 1;
    skipDamageTestOnce = false;
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
    exerted = false;
    turningOver = false;
    isMorphed = false;
    MeldedFrom = "";
    isFlipped = 0;
    isCommander = 0;
    isRingBearer = 0;
    isDefeated = false;
    isPhased = false;
    isCascaded = false;
    phasedTurn = -1;
    didattacked = 0;
    didblocked = 0;
    notblocked = 0;
    sunburst = 0;
    equipment = 0;
    mutation = 0;
    damageInflictedAsCommander = 0;
    numofcastfromcommandzone = 0;
    auras = 0;
    combatdamageToOpponent = 0;
    damageToOpponent = 0;
    damageToController = 0;
    damageToCreature = 0;
    isProvoked = false;
    ProvokeTarget = NULL;
    Provoker = NULL;
    wasDealtDamage = 0;
    isDualWielding = false;
    suspended = false;
    isBestowed = false;
    isFacedown = false;
    castMethod = Constants::NOT_CAST;
    mPropertiesChangedSinceLastUpdate = false;
    stillNeeded = true;
    kicked = 0;
    dredge = 0;
    zpos = 0;
    chooseacolor = -1;
    chooseasubtype = "";
    chooseaname = "";
    coinSide = -1;
    lastFlipResult = -1;
    dieSide = 0;
    lastRollResult = 0;
    dieNumFaces = 0;
    scryedCards = 0;
    isAttacking = NULL;
    storedCard = NULL;
    storedSourceCard = NULL;
    myPair = NULL;
    shackled = NULL;
    seized = NULL;
    miracle = false;
    hasCopiedToken = false;
    countTrini = 0;
    anymanareplacement = false;
    imprintedCards.clear();
    hauntedCard = NULL;
    attackCost = 0;
    attackCostBackup = 0;
    attackPlaneswalkerCost = 0;
    attackPlaneswalkerCostBackup = 0;
    blockCost = 0;
    blockCostBackup = 0;
    imprintG = 0;
    imprintU = 0;
    imprintR = 0;
    imprintB = 0;
    imprintW = 0;
    foretellTurn = -1;
    bushidoPoints = 0;
    modularPoints = 0;
    entersBattlefield = 0;
    currentimprintName = "";
    imprintedNames.clear();
    CountedObjects = 0;
    CountedObjectsB = 0;

    for (int i = 0; i < ManaCost::MANA_PAID_WITH_SUSPEND +1; i++)
        alternateCostPaid[i] = 0;

    paymenttype = MTGAbility::PUT_INTO_PLAY;
    reduxamount = 0;
    summoningSickness = 1;
    preventable = 0;
    thatmuch = 0;
    flanked = 0;
    target = NULL;
    playerTarget = NULL;
    type_as_damageable = DAMAGEABLE_MTGCARDINSTANCE;
    banding = NULL;
    owner = NULL;
    counters = NEW Counters(this);
    previousZone = NULL;
    tokCard = NULL;
    previous = NULL;
    next = NULL;
    auraParent = NULL;
    TokenAndAbility = NULL;
    GrantedAndAbility = NULL;
    discarderOwner = NULL;
    lastController = NULL;
    regenerateTokens = 0;
    blocked = false;
    graveEffects = false;
    exileEffects = false;
    commandZoneEffects = false;
    currentZone = NULL;
    cardsAbilities = vector<MTGAbility *>();
    //cardsAbilitiesFilter = vector<MTGAbility *>();
    data = this; //an MTGCardInstance point to itself for data, allows to update it without killing the underlying database item

    if (observer && basicAbilities[(int)Constants::CHANGELING])
    {//if the card is a changeling, it gains all creature subtypes
        vector<string> values = MTGAllCards::getCreatureValuesById();
        for (size_t i = 0; i < values.size(); ++i)
        {
            //Don' want to send any event to the gameObserver inside of initMCGI, so calling the parent setSubtype method instead of mine
            CardPrimitive::setSubtype(values[i].c_str());
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
        setName(MTGAllCards::findType(type));

    WEvent * e = NEW WEventCardChangeType(this, type, before, true);
    if (observer)
        observer->receiveEvent(e);
    else
        SAFE_DELETE(e);
}

void MTGCardInstance::addType(const string& type_text)
{
    setSubtype(type_text);
}

void MTGCardInstance::setType(const string& type_text)
{
    setSubtype(type_text);
}

void MTGCardInstance::setSubtype(const string& value)
{
    int id = MTGAllCards::findType(value);
    addType(id);
}
int MTGCardInstance::removeType(const string& value, int removeAll)
{
    int id = MTGAllCards::findType(value);
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
        if (name.compare(MTGAllCards::findType(id)) == 0)
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
        if (zone->hasCard(this) && !isPhased)
            return 1;
    }
    return 0;
}

int MTGCardInstance::afterDamage()
{
    if(skipDamageTestOnce)
    {
        skipDamageTestOnce = false;
        return 0;
    }
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

int MTGCardInstance::totem(bool noregen)
{
    int testToughness = toughness;
    testToughness += tbonus;
    if(testToughness < 1)
    {
        if(noregen)
            return toGrave();
        else if (!triggerRegenerate())
            return toGrave();
        return 0;
    }
    bool canregen = (regenerateTokens && !has(Constants::CANTREGEN) && !noregen);
    vector<MTGAbility*>selection;
    TargetChooserFactory tf(getObserver());
    TargetChooser * tcb = tf.createTargetChooser("mytotem",this);
    tcb->targetter = NULL;
    tcb->maxtargets = 1;
    MTGAbility * destroyTotem = NEW ATriggerTotem(getObserver(), getObserver()->mLayers->actionLayer()->getMaxId(),this,NULL);
    destroyTotem->oneShot = true;
    destroyTotem->canBeInterrupted = false;
    MTGAbility * dtTarget = NEW GenericTargetAbility(getObserver(), "","",getObserver()->mLayers->actionLayer()->getMaxId(), this,tcb->clone(), destroyTotem->clone());
    SAFE_DELETE(destroyTotem);
    dtTarget->oneShot = true;
    dtTarget->canBeInterrupted = false;
    MTGAbility * addTotemtoGame = NEW GenericAddToGame(getObserver(), getObserver()->mLayers->actionLayer()->getMaxId(), this,NULL,dtTarget->clone());
    SAFE_DELETE(dtTarget);
    addTotemtoGame->oneShot = true;
    addTotemtoGame->canBeInterrupted = false;
    selection.push_back(addTotemtoGame->clone());
    SAFE_DELETE(addTotemtoGame);
    SAFE_DELETE(tcb);
    if(canregen)
    {
        MTGAbility * triggerRegen = NEW ATriggerRegen(getObserver(), getObserver()->mLayers->actionLayer()->getMaxId(), this, this);
        triggerRegen->oneShot = true;
        triggerRegen->canBeInterrupted = false;
        selection.push_back(triggerRegen->clone());
        SAFE_DELETE(triggerRegen);
    }
    MTGAbility * menuChoice = NEW MenuAbility(getObserver(), getObserver()->mLayers->actionLayer()->getMaxId(), NULL, this,true,selection,this->controller(),"");
    menuChoice->addToGame();
    return 1;
}
int MTGCardInstance::toGrave( bool forced )
{
    if(basicAbilities[(int)Constants::INDESTRUCTIBLE] && !forced)
        return 0; // Fixed bug for indestructible creatures that have to go different zone after death.

    Player * p = controller();
    if (basicAbilities[(int)Constants::EXILEDEATH] || basicAbilities[(int)Constants::GAINEDEXILEDEATH] || (basicAbilities[(int)Constants::HASDISTURB] && alternateCostPaid[ManaCost::MANA_PAID_WITH_RETRACE] == 1))
    {
        p->game->putInZone(this, p->game->inPlay, owner->game->exile);
        basicAbilities[(int)Constants::GAINEDEXILEDEATH] = 0;
        return 1;
    }
    if (basicAbilities[(int)Constants::DOUBLEFACEDEATH] || basicAbilities[(int)Constants::GAINEDDOUBLEFACEDEATH])
    {
        p->game->putInZone(this, p->game->inPlay, owner->game->temp);
        basicAbilities[(int)Constants::GAINEDDOUBLEFACEDEATH] = 0;
        return 1;
    }
    if (basicAbilities[(int)Constants::HANDDEATH] || basicAbilities[(int)Constants::GAINEDHANDDEATH])
    {
        p->game->putInZone(this, p->game->inPlay, owner->game->hand);
        basicAbilities[(int)Constants::GAINEDHANDDEATH] = 0;
        return 1;
    }
    if (basicAbilities[(int)Constants::INPLAYDEATH] || basicAbilities[(int)Constants::INPLAYTAPDEATH])
    {
        bool toTap = basicAbilities[(int)Constants::INPLAYTAPDEATH];
        bool addCounter = basicAbilities[(int)Constants::COUNTERDEATH];
        MTGCardInstance* ret = p->game->putInZone(this, p->game->inPlay, owner->game->graveyard);
        ret = p->game->putInZone(ret, owner->game->graveyard, owner->game->battlefield);
        if(toTap)
            ret->tap(true);
        if(addCounter)
            ret->counters->addCounter(1, 1, false);
        return 1;
    }
    // Let's put the creature in the default zone after death (graveyard).
    p->game->putInZone(this, p->game->inPlay, owner->game->graveyard);
    return 1;
}
int MTGCardInstance::destroy()
{
    if (hasTotemArmor())
        return totem();
    else if (!triggerRegenerate())
        return toGrave();
    return 0;
}
int MTGCardInstance::destroyNoRegen()
{
    if (hasTotemArmor())
        return totem(true);
    else
        return toGrave();
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

//Return the toxicity of card
int MTGCardInstance::getToxicity()
{
    int toxicity = 0;
    if(has(Constants::POISONTOXIC))
        toxicity = 1;
    else if(has(Constants::POISONTWOTOXIC))
        toxicity = 2;
    else if(has(Constants::POISONTHREETOXIC))
        toxicity = 3;
    else if(has(Constants::POISONFOURTOXIC))
        toxicity = 4;
    else if(has(Constants::POISONFIVETOXIC))
        toxicity = 5;
    else if(has(Constants::POISONSIXTOXIC))
        toxicity = 6;
    else if(has(Constants::POISONSEVENTOXIC))
        toxicity = 7;
    else if(has(Constants::POISONEIGHTTOXIC))
        toxicity = 8;
    else if(has(Constants::POISONNINETOXIC))
        toxicity = 9;
    else if(has(Constants::POISONTENTOXIC))
        toxicity = 10;
    return toxicity;
}

ManaCost* MTGCardInstance::getReducedManaCost()
{
    return &reducedCost;
}
ManaCost* MTGCardInstance::getIncreasedManaCost()
{
    return &increasedCost;
}
ManaCost* MTGCardInstance::getProducedMana()
{
    return &producedMana;
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
void MTGCardInstance::tap(bool sendNoEvent)
{
    if (tapped)
        return;
    tapped = 1;
    WEvent * e = NEW WEventCardTap(this, 0, 1);
    if (sendNoEvent)
        dynamic_cast<WEventCardTap*>(e)->noTrigger = true;
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

void MTGCardInstance::resetUntapping()
{
    untapping = 0; // Fix to avoid the untap on frozen card by clicking on them after the untap phase.
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
    if (previous->stillNeeded)
    {
        previous->stillNeeded = false;
        return 1;
    }
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
    if (has(Constants::HASTE))
        return 0;
    if (!isCreature())
        return 0;
    return 1;
}

MTGCardInstance * MTGCardInstance::changeController(Player * newController,bool notZone)
{
    if(notZone)
    {
        lastController = newController;
        return this;
    }
    Player * originalOwner = controller();
    MTGCardInstance * copy = originalOwner->game->putInZone(this, this->currentZone, newController->game->inPlay);
    //copy->summoningSickness = 1;
    return copy;
}

Player * MTGCardInstance::controller()
{
    return lastController;
}

int MTGCardInstance::canAttack( bool pwcheck )
{
    if(!pwcheck)
    {
        if (has(Constants::CANTATTACK))
            return 0;
        if (has(Constants::FLYERSONLY) && !has(Constants::FLYING))
            return 0;
    }
    else
    {
        if (has(Constants::CANTPWATTACK))
            return 0;
    }
    if (tapped)
        return 0;
    if (hasSummoningSickness())
        return 0;
    if (has(Constants::DEFENSER) && !has(Constants::CANATTACK))
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

void MTGCardInstance::stripPTbonus()
{
    power -= pbonus;
    addToToughness(-tbonus);
}

void MTGCardInstance::plusPTbonus(int p, int t)
{
    pbonus += p;
    tbonus += t;
}

void MTGCardInstance::minusPTbonus(int p, int t)
{
    pbonus -= p;
    tbonus -= t;
}

void MTGCardInstance::applyPTbonus()
{
    power += pbonus;
    addToToughness(tbonus);
}

void MTGCardInstance::addcounter(int p, int t)
{
    stripPTbonus();
    plusPTbonus(p,t);
    applyPTbonus();
}

void MTGCardInstance::addptbonus(int p, int t)
{
    stripPTbonus();
    plusPTbonus(p,t);
    applyPTbonus();
}

void MTGCardInstance::removecounter(int p, int t)
{
    stripPTbonus();
    minusPTbonus(p,t);
    applyPTbonus();
}

void MTGCardInstance::removeptbonus(int p, int t)
{
    stripPTbonus();
    minusPTbonus(p,t);
    applyPTbonus();
}

void MTGCardInstance::addbaseP(int p)
{
    basepower = p;
    power -= pbonus;
    power = p;
    power += pbonus;
}

void MTGCardInstance::addbaseT(int t)
{
    basetoughness = t;
    addToToughness(-tbonus);
    addToToughness(t - toughness);
    addToToughness(tbonus);
}

void MTGCardInstance::revertbaseP()
{
    power -= pbonus;
    power += origpower;
    power -= basepower;
    power += pbonus;
    basepower = origpower;
}

void MTGCardInstance::revertbaseT()
{
    addToToughness(-tbonus);
    addToToughness(origtoughness);
    addToToughness(-basetoughness);
    addToToughness(tbonus);
    basetoughness = origtoughness;
}

void MTGCardInstance::cdaPT(int p, int t)
{
    origpower = p;
    origtoughness = t;
    setPower(p);
    setToughness(t);
    applyPTbonus();
}

void MTGCardInstance::switchPT(bool apply)
{
    stripPTbonus();
    swapP = power;
    swapT = toughness;
    if(!origpower && !origtoughness){ // Fix when a non-creature card is firstly transformed into a creature (e.g. "Wandering Fumarole").
        origpower = power;
        origtoughness = toughness;
    }
    power += origpower;
    power -= swapP;
    addToToughness(origtoughness);
    addToToughness(-swapT);
    applyPTbonus();
    if(apply)
    {
        swapP = toughness;
        swapT = power;
        addToToughness(swapT);
        addToToughness(-swapP);
        setPower(swapP);
    }
}

int MTGCardInstance::getCurrentPower()
{
    if(observer && !isCreature())
        return 0;
    if(observer && !isInPlay(observer))
        return LKIpower;
    return power;
}

int MTGCardInstance::getCurrentToughness()
{
    if(observer && !isCreature())
        return 0;
    if(observer && !isInPlay(observer))
        return LKItoughness;
    return toughness;
}

int MTGCardInstance::countDuplicateCardNames()
{
    int count = 0;

    if(observer)
    {
        int nb_cards = controller()->game->battlefield->nb_cards;
        for(int x = 0; x < nb_cards; x++)
        {
            if(controller()->game->battlefield->cards[x]->name == this->name && !(controller()->game->battlefield->cards[x]->mutation && controller()->game->battlefield->cards[x]->parentCards.size() > 0)) // Don't count Mutated down card
                if(!(this->hasType(Subtypes::TYPE_LEGENDARY) && !(controller()->game->battlefield->cards[x]->hasType(Subtypes::TYPE_LEGENDARY)))) // This fix issue when cloning a card with nolegend option (e.g. Double Major)
                    count+=1;
        }
    }
    return count;
}

int MTGCardInstance::countDuplicateCardTypes()
{
    int count = 0;

    if(observer)
    {
        int nb_cards = controller()->game->battlefield->nb_cards;
        for(int x = 0; x < nb_cards; x++)
        {
            if(controller()->game->battlefield->cards[x] != this && controller()->game->battlefield->cards[x]->types == this->types)
                count+=1;
        }
    }
    return count;
}

//check can produce mana
int MTGCardInstance::canproduceMana(int color)
{
    int count = 0;

    if(cardsAbilities.size())
    {
        for(unsigned int j = 0; j < cardsAbilities.size(); j++)
        {
            AbilityFactory af(observer);
            MTGAbility * toCheck = af.getCoreAbility(cardsAbilities[j]);
            if(dynamic_cast<AManaProducer*> (toCheck) && dynamic_cast<AManaProducer*> (toCheck)->output->hasColor(color))
                count++;
        }
    }

    if(count)
        return 1;

    return 0;
}

//check can be played from library top
bool MTGCardInstance::canPlayFromLibrary()
{
    int found = 0;
    if(has(Constants::CANPLAYFROMLIBRARYTOP) 
        || (controller()->game->inPlay->nb_cards && controller()->game->inPlay->hasAbility(Constants::CANPLAYFROMLIBRARYTOP)))
        found++;
    if(isLand() && (has(Constants::CANPLAYLANDTOPLIBRARY)
        || (controller()->game->inPlay->nb_cards && controller()->game->inPlay->hasAbility(Constants::CANPLAYLANDTOPLIBRARY))))
        found++;
    if(hasSubtype(Subtypes::TYPE_ARTIFACT) && (has(Constants::CANPLAYARTIFACTTOPLIBRARY)
        || (controller()->game->inPlay->nb_cards && controller()->game->inPlay->hasAbility(Constants::CANPLAYARTIFACTTOPLIBRARY))))
        found++;
    if(isCreature() && (has(Constants::CANPLAYCREATURETOPLIBRARY)
        || (controller()->game->inPlay->nb_cards && controller()->game->inPlay->hasAbility(Constants::CANPLAYCREATURETOPLIBRARY))))
        found++;
    if(isSorceryorInstant() && (has(Constants::CANPLAYINSTANTSORCERYTOPLIBRARY)
        || (controller()->game->inPlay->nb_cards && controller()->game->inPlay->hasAbility(Constants::CANPLAYINSTANTSORCERYTOPLIBRARY))))
        found++;
    if((hasSubtype(Subtypes::TYPE_EQUIPMENT) || hasSubtype(Subtypes::TYPE_AURA)) && (has(Constants::CANPLAYAURAEQUIPTOPLIBRARY)
        || (controller()->game->inPlay->nb_cards && controller()->game->inPlay->hasAbility(Constants::CANPLAYAURAEQUIPTOPLIBRARY))))
        found++;

    if(found > 0)
        return true;

    return false;
}

//check stack
bool MTGCardInstance::StackIsEmptyandSorcerySpeed()
{
    Player * whoInterupts = getObserver()->isInterrupting;//leave this so we can actually debug who is interupting/current.
    Player * whoCurrent = getObserver()->currentPlayer;
    if((getObserver()->mLayers->stackLayer()->count(0, NOT_RESOLVED) == 0) &&
        (getObserver()->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN ||
        getObserver()->getCurrentGamePhase() == MTG_PHASE_SECONDMAIN) &&
        controller() == whoCurrent &&
        (!whoInterupts || whoInterupts == whoCurrent))
    {
        return true;
    }
    return false;
}

int MTGCardInstance::canBlock()
{
    if (tapped && !has(Constants::CANBLOCKTAPPED))
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
    if (this->cantBeBlockerOf(opponent))
        return 0;
    if (this->cantBeBlockerOfCard(opponent))
        return 0;
    if (opponent->basicAbilities[(int)Constants::UNBLOCKABLE])
        return 0;
    if (opponent->basicAbilities[(int)Constants::ONEBLOCKER] && opponent->blocked)
        return 0;
    if((opponent->basicAbilities[(int)Constants::EVADEBIGGER]|| opponent->basicAbilities[(int)Constants::SKULK]) && power > opponent->power)
        return 0;
    if(opponent->basicAbilities[(int)Constants::STRONG] && power < opponent->power)
        return 0;
    if(this->basicAbilities[(int)Constants::WEAK] && power < opponent->power)
        return 0;
    if (opponent->basicAbilities[(int)Constants::FEAR] && !(this->hasType(Subtypes::TYPE_ARTIFACT) || this->hasColor(Constants::MTG_COLOR_BLACK)))
        return 0;
    if (opponent->controller()->game->battlefield->hasAbility(Constants::LURE) && !opponent->has(Constants::LURE))
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

ManaCost * MTGCardInstance::computeNewCost(MTGCardInstance * card,ManaCost * Cost, ManaCost * Data, bool noTrinisphere, bool bestow)
{
    int color = 0;
    string type = "";
    ManaCost * original = NEW ManaCost();
    ManaCost * excess = NEW ManaCost();
    original->copy(Data);
    Cost->copy(original);
    if (Cost->extraCosts)
    {
        for (unsigned int i = 0; i < Cost->extraCosts->costs.size(); i++)
        {
            Cost->extraCosts->costs[i]->setSource(card);
        }
    }
    if (card->getIncreasedManaCost()->getConvertedCost() || card->getReducedManaCost()->getConvertedCost()
        || card->controller()->AuraReduced->getConvertedCost() || card->controller()->AuraIncreased->getConvertedCost())
    {//start1
        if (card->getIncreasedManaCost()->getConvertedCost())
            original->add(card->getIncreasedManaCost());
        if(bestow && card->controller()->AuraIncreased->getConvertedCost())
            original->add(card->controller()->AuraIncreased);
        //before removing get the diff for excess
        if(card->getReducedManaCost()->getConvertedCost())
        {
            for(int xc = 0; xc < 7;xc++)
            {//if the diff is more than 0
                if(card->getReducedManaCost()->getCost(xc) > original->getCost(xc))
                {
                    int count = card->getReducedManaCost()->getCost(xc) - original->getCost(xc);
                    excess->add(xc,count);
                }
            }
        }
        //apply reduced
        if (card->getReducedManaCost()->getConvertedCost())
            original->remove(card->getReducedManaCost());
        if(bestow && card->controller()->AuraReduced->getConvertedCost())
            original->remove(card->controller()->AuraReduced);
        //try to reduce hybrid
        if (excess->getConvertedCost())
        {
            original->removeHybrid(excess);
        }
        Cost->copy(original);
        if (Cost->extraCosts)
        {
            for (unsigned int i = 0; i < Cost->extraCosts->costs.size(); i++)
            {
                Cost->extraCosts->costs[i]->setSource(card);
            }
        }
    }//end1
    int reducem = 0;
    bool resetCost = false;
    for (unsigned int na = 0; na < card->cardsAbilities.size(); na++)
    {//start2
        if (!card->cardsAbilities[na])
            break;
        ANewAffinity * newAff = dynamic_cast<ANewAffinity*>(card->cardsAbilities[na]);
        if (newAff)
        {
            if (!resetCost)
            {
                resetCost = true;
                Cost->copy(original);
                if (Cost->extraCosts)
                {
                    for (unsigned int i = 0; i < Cost->extraCosts->costs.size(); i++)
                    {
                        Cost->extraCosts->costs[i]->setSource(card);
                    }
                }
            }
            if(newAff->tcString.find("variable{") != string::npos)
            {
                vector<string> eval = parseBetween(newAff->tcString, "variable{", "}");
                if(eval.size())
                {
                    WParsedInt* value = NEW WParsedInt(eval[1], NULL, card);
                    if(value)
                    {
                        reducem += value->getValue();
                        SAFE_DELETE(value);
                    }
                }
            }
            else 
            {
                TargetChooserFactory tf(getObserver());
                TargetChooser * tcn = tf.createTargetChooser(newAff->tcString, card, NULL);

                for (int w = 0; w < 2; ++w)
                {
                    Player *p = getObserver()->players[w];
                    MTGGameZone * zones[] = { p->game->inPlay, p->game->graveyard, p->game->hand, p->game->library, p->game->stack, p->game->exile, p->game->commandzone, p->game->sideboard, p->game->reveal };
                    for (int k = 0; k < 9; k++)
                    {
                        MTGGameZone * z = zones[k];
                        if (tcn->targetsZone(z))
                        {
                            reducem += z->countByCanTarget(tcn);
                        }
                    }
                }
                SAFE_DELETE(tcn);
            }
            ManaCost * removingCost = ManaCost::parseManaCost(newAff->manaString);
            for (int j = 0; j < reducem; j++)
                Cost->remove(removingCost);
            SAFE_DELETE(removingCost);
        }
    }//end2
    if (card->has(Constants::AFFINITYARTIFACTS) ||
        card->has(Constants::AFFINITYENCHANTMENTS) ||
        card->has(Constants::AFFINITYFOREST) ||
        card->has(Constants::AFFINITYGREENCREATURES) ||
        card->has(Constants::AFFINITYISLAND) ||
        card->has(Constants::AFFINITYMOUNTAIN) ||
        card->has(Constants::AFFINITYPLAINS) ||
        card->has(Constants::AFFINITYSWAMP) ||
        card->has(Constants::AFFINITYALLCREATURES) ||
        card->has(Constants::AFFINITYCONTROLLERCREATURES) ||
        card->has(Constants::AFFINITYOPPONENTCREATURES) ||
        card->has(Constants::AFFINITYALLDEADCREATURES) ||
        card->has(Constants::AFFINITYPARTY) ||
        card->has(Constants::AFFINITYBASICLANDTYPES) ||
        card->has(Constants::AFFINITYTWOBASICLANDTYPES) ||
        card->has(Constants::AFFINITYGRAVECREATURES) ||
        card->has(Constants::AFFINITYATTACKINGCREATURES) ||
        card->has(Constants::AFFINITYGRAVEINSTSORC) ||
        card->has(Constants::CONDUITED))
    {//start3
        if (card->has(Constants::AFFINITYARTIFACTS))
        {
            type = "artifact";
        }
        if (card->has(Constants::AFFINITYENCHANTMENTS))
        {
            type = "enchantment";
        }
        else if (card->has(Constants::AFFINITYSWAMP))
        {
            type = "swamp";
        }
        else if (card->has(Constants::AFFINITYMOUNTAIN))
        {
            type = "mountain";
        }
        else if (card->has(Constants::AFFINITYPLAINS))
        {
            type = "plains";
        }
        else if (card->has(Constants::AFFINITYISLAND))
        {
            type = "island";
        }
        else if (card->has(Constants::AFFINITYFOREST))
        {
            type = "forest";
        }
        else if (card->has(Constants::AFFINITYGREENCREATURES))
        {
            color = 1;
            type = "creature";
        }
        else if (card->has(Constants::AFFINITYALLCREATURES) || card->has(Constants::AFFINITYCONTROLLERCREATURES) || card->has(Constants::AFFINITYOPPONENTCREATURES) || card->has(Constants::AFFINITYALLDEADCREATURES))
        {
            type = "creature";
        }
        else if (card->has(Constants::AFFINITYPARTY) || card->has(Constants::AFFINITYGRAVECREATURES) || card->has(Constants::AFFINITYATTACKINGCREATURES))
        {
            type = "creature";
        }
        else if (card->has(Constants::AFFINITYBASICLANDTYPES) || card->has(Constants::AFFINITYTWOBASICLANDTYPES))
        {
            type = "land";
        }
        else if (card->has(Constants::AFFINITYGRAVEINSTSORC))
        {
            type = "instant,sorcery";
        }

        Cost->copy(original);
        if (Cost->extraCosts)
        {
            for (unsigned int i = 0; i < Cost->extraCosts->costs.size(); i++)
            {
                Cost->extraCosts->costs[i]->setSource(card);
            }
        }
        if (Cost->extraCosts)
        {
            for (unsigned int i = 0; i < Cost->extraCosts->costs.size(); i++)
            {
                Cost->extraCosts->costs[i]->setSource(card);
            }
        }
        int reduce = 0;
        if (card->has(Constants::AFFINITYALLCREATURES))
        {
            TargetChooserFactory tf(getObserver());
            TargetChooser * tc = tf.createTargetChooser("creature", NULL);
            reduce = card->controller()->game->battlefield->countByCanTarget(tc) + card->controller()->opponent()->game->battlefield->countByCanTarget(tc);
            SAFE_DELETE(tc);
        }
        else if (card->has(Constants::AFFINITYCONTROLLERCREATURES))
        {
            TargetChooserFactory tf(getObserver());
            TargetChooser * tc = tf.createTargetChooser("creature", NULL);
            reduce = card->controller()->game->battlefield->countByCanTarget(tc);
            SAFE_DELETE(tc);
        }
        else if (card->has(Constants::AFFINITYATTACKINGCREATURES))
        {
            TargetChooserFactory tf(getObserver());
            TargetChooser * tc = tf.createTargetChooser("creature[attacking]", NULL);
            reduce = card->controller()->game->battlefield->countByCanTarget(tc);
            SAFE_DELETE(tc);
        }
        else if (card->has(Constants::AFFINITYOPPONENTCREATURES))
        {
            TargetChooserFactory tf(getObserver());
            TargetChooser * tc = tf.createTargetChooser("creature", NULL);
            reduce = card->controller()->opponent()->game->battlefield->countByCanTarget(tc);
            SAFE_DELETE(tc);
        }
        else if (card->has(Constants::AFFINITYGRAVECREATURES))
        {
            WParsedInt* value = NEW WParsedInt("type:creature:mygraveyard", NULL, card);
            if(value)
                reduce = value->getValue();
            SAFE_DELETE(value);
        }
        else if (card->has(Constants::AFFINITYALLDEADCREATURES))
        {
            WParsedInt* value = NEW WParsedInt("bothalldeadcreature", NULL, card);
            if(value)
                reduce = value->getValue();
            SAFE_DELETE(value);
        }
        else if (card->has(Constants::AFFINITYPARTY))
        {
            WParsedInt* value = NEW WParsedInt("calculateparty", NULL, card);
            if(value)
                reduce = value->getValue();
            SAFE_DELETE(value);
        }
        else if (card->has(Constants::AFFINITYBASICLANDTYPES))
        {
            WParsedInt* value = NEW WParsedInt("pbasiclandtypes", NULL, card);
            if(value)
                reduce = value->getValue();
            SAFE_DELETE(value);
        }
        else if (card->has(Constants::AFFINITYTWOBASICLANDTYPES))
        {
            WParsedInt* value = NEW WParsedInt("pbasiclandtypes", NULL, card);
            if(value)
                reduce = value->getValue() * 2;
            SAFE_DELETE(value);
        }
        else if (card->has(Constants::AFFINITYGRAVEINSTSORC))
        {
            WParsedInt* value = NEW WParsedInt("pginstantsorcery", NULL, card);
            if(value)
                reduce = value->getValue();
            SAFE_DELETE(value);
        }
        else if (card->has(Constants::AFFINITYGREENCREATURES))
        {
            TargetChooserFactory tf(getObserver());
            TargetChooser * tc = tf.createTargetChooser("creature[green]", NULL);
            reduce = card->controller()->game->battlefield->countByCanTarget(tc);
            SAFE_DELETE(tc);
        }
        else if (card->has(Constants::CONDUITED))
        {//I had to hardcode this since it doesn't update with auto=this(creaturespells<1) lord(creature|mycastingzone) altercost(colorless,-2)
            color = 0;
            reduce = card->controller()->inPlay()->countByAlias(401847);
            reduce *= 2;
            if(card->controller()->game->stack->seenThisTurn("creature", Constants::CAST_ALL) > 0)
                reduce = 0;
        }
        else
        {
            reduce = card->controller()->game->battlefield->countByType(type);
        }
        for (int i = 0; i < reduce; i++)
        {
            if (Cost->getCost(color) > 0)
                Cost->remove(color, 1);
        }
    }//end3
     //trinisphere... now how to implement kicker recomputation

    if (card->has(Constants::TRINISPHERE))
    {
        for (int jj = Cost->getConvertedCost(); jj < 3; jj++)
        {
            Cost->add(Constants::MTG_COLOR_ARTIFACT, 1);
            card->countTrini++;
        }
    }
    else
    {
        if (card->countTrini)
        {
            Cost->remove(Constants::MTG_COLOR_ARTIFACT, card->countTrini);
            card->countTrini = 0;
        }
    }
    SAFE_DELETE(original);
    SAFE_DELETE(excess);
    return Cost;
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
    // Even at 60(danger=3) the AI is hasty to play removal on a simple creature
    // a vanilla 2 mana, 2/2 used to be eff = 60
    if (result > 2)
        danger += 1;
    if (result > 4)
        danger += 1;
    if (result > 6)
        danger += 1;
    if (result > 8)
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

int MTGCardInstance::toggleAttacker(bool pw)
{
    if (!attacker)
    {
        //if (!basicAbilities[Constants::VIGILANCE]) tap();
        if(pw)
        {
            willattackpw = 1;
        }
        else
        {
            willattackplayer = 1;
        }
        setAttacker(1);
        return 1;
    }
    else
    {
        //untap();
        willattackpw = 0;
        willattackplayer = 0;
        setAttacker(0);
        isAttacking = NULL;
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
    // Blockers can be removed "manually" (by the blocking player) at the Blockers step,
    // Or "automatically" in the damage phase, when they die and regenerate (see http://code.google.com/p/wagic/issues/detail?id=563 )
    // In the second case, we still want the card to be marked as "blocked" this turn
    if (!blockers.size() && observer->getCurrentGamePhase() == MTG_PHASE_COMBATBLOCKERS)
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
        {//remove blocker "this" from the attackers list of blockers.
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

bool MTGCardInstance::hasTotemArmor()
{
    int count = 0;
    if(observer)
    {
        for (int i = 0; i < 2; i++)
        {
            int nb_cards = observer->players[i]->game->battlefield->nb_cards;
            for(int x = 0; x < nb_cards; x++)
            {
                if(observer->players[i]->game->battlefield->cards[x]->auraParent)
                {
                    if(observer->players[i]->game->battlefield->cards[x]->auraParent == this && 
                        observer->players[i]->game->battlefield->cards[x]->has(Constants::TOTEMARMOR))
                        count+=1;
                }
            }
        }
    }

    if(count)
        return true;

    return false;
}

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
        if (cantBeBlockedBys[i]->canTarget(card, true))
            return 1;
    }
    return 0;
}
//cant be the block of
int MTGCardInstance::addCantBeBlockerOf(TargetChooser * tc)
{
    cantBeBlockerOfs.push_back(tc);
    return cantBeBlockerOfs.size();
}

int MTGCardInstance::removeCantBeBlockerOf(TargetChooser * tc, int erase)
{
    for (size_t i = 0; i < cantBeBlockerOfs.size(); i++)
    {
        if (cantBeBlockerOfs[i] == tc)
        {
            if (erase)
                delete (cantBeBlockerOfs[i]);
            cantBeBlockerOfs.erase(cantBeBlockerOfs.begin() + i);
            return 1;
        }
    }
    return 0;
}

int MTGCardInstance::cantBeBlockerOf(MTGCardInstance * card)
{
    for (size_t i = 0; i < cantBeBlockerOfs.size(); i++)
    {
        if (cantBeBlockerOfs[i]->canTarget(card, true))
            return 1;
    }
    return 0;
}

//cant be the block of
int MTGCardInstance::addCantBeBlockerOfCard(MTGCardInstance * card)
{
    cantBeBlockerOfCards.push_back(card);
    return cantBeBlockerOfCards.size();
}

int MTGCardInstance::removeCantBeBlockerOfCard(MTGCardInstance * card, int erase)
{
    for (size_t i = 0; i < cantBeBlockerOfCards.size(); i++)
    {
        if (cantBeBlockerOfCards[i] == card)
        {
            if (erase)
                delete (cantBeBlockerOfCards[i]);
            cantBeBlockerOfCards.erase(cantBeBlockerOfCards.begin() + i);
            return 1;
        }
    }
    return 0;
}

int MTGCardInstance::cantBeBlockerOfCard(MTGCardInstance * card)
{
    for (size_t i = 0; i < cantBeBlockerOfCards.size(); i++)
    {
        if (cantBeBlockerOfCards[i] == card)
            return 1;
    }
    return 0;
}
/* Choose a sound sample to associate to that card */
const string& MTGCardInstance::getSample()
{
    if (sample.size())
        return sample;

    for (int i = types.size() - 1; i > 0; i--)
    {
        string type = MTGAllCards::findType(types[i]);
        std::transform(type.begin(), type.end(), type.begin(), ::tolower);
        type = type + ".wav";
        if(getObserver() && getObserver()->getResourceManager())
        {
            if (getObserver()->getResourceManager()->RetrieveSample(type))
            {
                sample = string(type);
                return sample;
            }
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
            if(getObserver() && getObserver()->getResourceManager())
            {
                if (getObserver()->getResourceManager()->RetrieveSample(type))
                {
                    sample = string(type);
                    return sample;
                }
            }
        }
    }

    string type = "";
    if(!types.size())
        return sample;   
    type = MTGAllCards::findType(types[0]);
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    type.append(".wav");
    if(getObserver() && getObserver()->getResourceManager())
    {
        if (getObserver()->getResourceManager()->RetrieveSample(type))
        {
            sample = string(type);
            return sample;
        }
    }

    return sample;
}

int MTGCardInstance::stepPower(CombatStep step)
{
    int damage = has(Constants::COMBATTOUGHNESS) ? toughness : power;
    switch (step)
    {
    case FIRST_STRIKE:
    case END_FIRST_STRIKE:
        if (has(Constants::FIRSTSTRIKE) || has(Constants::DOUBLESTRIKE))
            return MAX(0, damage);
        else
            return 0;
    case DAMAGE:
    case END_DAMAGE:
    default:
        if (has(Constants::FIRSTSTRIKE) && !has(Constants::DOUBLESTRIKE))
            return 0;
        else
            return MAX(0, damage);
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

MTGCardInstance* MTGCardInstance::clone()
{
    return NEW MTGCardInstance(model, owner->game);
}
