#include "PrecompiledHeader.h"

#include "GameObserver.h"
#include "CardGui.h"
#include "Damage.h"
#include "Rules.h"
#include "ExtraCost.h"
#include "Subtypes.h"
#include <JLogger.h>
#include <JRenderer.h>
#include "MTGGamePhase.h"
#include "GuiPhaseBar.h"
#include "AIPlayerBaka.h"
#include "MTGRules.h"
#include "Trash.h"
#include "DeckManager.h"
#include "GuiCombat.h"
#include <algorithm>
#ifdef TESTSUITE
#include "TestSuiteAI.h"
#endif
#ifdef NETWORK_SUPPORT
#include "NetworkPlayer.h"
#endif


void GameObserver::cleanup()
{
    SAFE_DELETE(targetChooser);
    SAFE_DELETE(mLayers);
    SAFE_DELETE(phaseRing);
    SAFE_DELETE(replacementEffects);
    for (size_t i = 0; i < players.size(); ++i)
    {
        if(players[i])
            SAFE_DELETE(players[i]);
    }
    players.clear();

    currentPlayer = NULL;
    currentActionPlayer = NULL;
    isInterrupting = NULL;
    currentPlayerId = 0;
    mCurrentGamePhase = MTG_PHASE_INVALID;
    targetChooser = NULL;
    cardWaitingForTargets = NULL;
    mExtraPayment = NULL;
    gameOver = NULL;
    phaseRing = NULL;
    replacementEffects = NEW ReplacementEffects();
    combatStep = BLOCKERS;
    connectRule = false;
    actionsList.clear();
    gameTurn.clear();
    OpenedDisplay = NULL;
    AffinityNeedsUpdate = false;
}

GameObserver::~GameObserver()
{
    LOG("==Destroying GameObserver==");

    for (size_t i = 0; i < players.size(); ++i)
    {
        if(players[i])
            players[i]->End();
    }
    SAFE_DELETE(targetChooser);
    SAFE_DELETE(mLayers);
    SAFE_DELETE(phaseRing);
    SAFE_DELETE(replacementEffects);
    for (size_t i = 0; i < players.size(); ++i)
    {
        if(players[i])
            SAFE_DELETE(players[i]);
    }
    players.clear();
    delete[] ExtraRules;
    ExtraRules = 0;
    LOG("==GameObserver Destroyed==");
    SAFE_DELETE(mTrash);
    SAFE_DELETE(mDeckManager);

}

GameObserver::GameObserver(WResourceManager *output, JGE* input)
    : mSeed((unsigned int)time(0)), randomGenerator(mSeed, true), mResourceManager(output), mJGE(input)

{
    ExtraRules = new MTGCardInstance[2]();

    mGameType = GAME_TYPE_CLASSIC;
    currentPlayer = NULL;
    currentActionPlayer = NULL;
    isInterrupting = NULL;
    currentPlayerId = 0;
    mCurrentGamePhase = MTG_PHASE_INVALID;
    targetChooser = NULL;
    cardWaitingForTargets = NULL;
    mExtraPayment = NULL;
    OpenedDisplay = NULL;
    guiOpenDisplay = NULL;
    gameOver = NULL;
    phaseRing = NULL;
    replacementEffects = NEW ReplacementEffects();
    combatStep = BLOCKERS;
    mRules = NULL;
    connectRule = false;
    mLoading = false;
    mLayers = NULL;
    mTrash = new Trash();
    mDeckManager = new DeckManager();
}

GamePhase GameObserver::getCurrentGamePhase()
{
    return mCurrentGamePhase;
}

const string& GameObserver::getCurrentGamePhaseName()
{
    return phaseRing->phaseName(mCurrentGamePhase);
}

const string& GameObserver::getNextGamePhaseName()
{
    return phaseRing->phaseName((mCurrentGamePhase + 1) % MTG_PHASE_CLEANUP);
}

Player * GameObserver::opponent()
{
    int index = (currentPlayerId + 1) % players.size();
    return players[index];
}

Player * GameObserver::nextTurnsPlayer()
{
    int nextTurnsId = 0;
    if(!players[currentPlayerId]->extraTurn)
        nextTurnsId = (currentPlayerId + 1) % players.size();
    else
    {
        nextTurnsId = currentPlayerId;
    }
    if(players[currentPlayerId]->skippingTurn)
    {
        nextTurnsId = (currentPlayerId + 1) % players.size();
    }
    return players[nextTurnsId];
}

void GameObserver::nextPlayer()
{
    turn++;
    if(!players[currentPlayerId]->extraTurn)
        currentPlayerId = (currentPlayerId + 1) % players.size();
    else
    {
        players[currentPlayerId]->extraTurn--;
    }
    if(players[currentPlayerId]->skippingTurn)
    {
        players[currentPlayerId]->skippingTurn--;
        currentPlayerId = (currentPlayerId + 1) % players.size();
    }
    currentPlayer = players[currentPlayerId];
    currentActionPlayer = currentPlayer;
    combatStep = BLOCKERS;
}

void GameObserver::nextGamePhase()
{
    Phase * cPhaseOld = phaseRing->getCurrentPhase();
    if (cPhaseOld->id == MTG_PHASE_COMBATDAMAGE)
        if ((FIRST_STRIKE == combatStep) || (END_FIRST_STRIKE == combatStep) || (DAMAGE == combatStep))
        {
            nextCombatStep();
            return;
        }

    if (cPhaseOld->id == MTG_PHASE_COMBATBLOCKERS)
        if (BLOCKERS == combatStep || TRIGGERS == combatStep)
        {
            nextCombatStep();
            return;
        }

    phaseRing->forward();

    //Go directly to end of combat if no attackers
    if (cPhaseOld->id == MTG_PHASE_COMBATATTACKERS && !(currentPlayer->game->inPlay->getNextAttacker(NULL)))
    {
        phaseRing->forward();
        phaseRing->forward();
    }

    Phase * cPhase = phaseRing->getCurrentPhase();
    mCurrentGamePhase = cPhase->id;

    if (MTG_PHASE_COMBATDAMAGE == mCurrentGamePhase)
        nextCombatStep();
    if (MTG_PHASE_COMBATEND == mCurrentGamePhase)
        combatStep = BLOCKERS;

    //if (currentPlayer != cPhase->player)
    //    nextPlayer();//depreciated; we call this at EOT step now. unsure what the purpose of this was originally.fix for a bug?

    //init begin of turn
    if (mCurrentGamePhase == MTG_PHASE_BEFORE_BEGIN)
    {
        cleanupPhase();
        currentPlayer->damageCount = 0;
        currentPlayer->nonCombatDamage = 0;
        currentPlayer->drawCounter = 0;
        currentPlayer->raidcount = 0;
        currentPlayer->dealsdamagebycombat = 0; //clear check for restriction
        currentPlayer->opponent()->raidcount = 0;
        currentPlayer->prowledTypes.clear();
        currentPlayer->opponent()->damageCount = 0; //added to clear odcount
        currentPlayer->opponent()->nonCombatDamage = 0;
        currentPlayer->preventable = 0;
        mLayers->actionLayer()->cleanGarbage(); //clean abilities history for this turn;
        mLayers->stackLayer()->garbageCollect(); //clean stack history for this turn;
        mLayers->actionLayer()->Update(0);
        currentPlayer->game->library->miracle = false;
        currentPlayer->opponent()->game->library->miracle = false;
        for (int i = 0; i < 2; i++)
        {
            //Cleanup of each player's gamezones
            players[i]->game->beforeBeginPhase();
        }
        combatStep = BLOCKERS;
        return nextGamePhase();
    }

    if (mCurrentGamePhase == MTG_PHASE_AFTER_EOT)
    {
        int handmodified = 0;
        handmodified = currentPlayer->handsize+currentPlayer->handmodifier;
        //Auto Hand cleaning, in case the player didn't do it himself
        if(handmodified < 0)
            handmodified = 0;
        while (currentPlayer->game->hand->nb_cards > handmodified && currentPlayer->nomaxhandsize == false)
        {
            WEvent * e = NEW WEventCardDiscard(currentPlayer->game->hand->cards[0]);
            receiveEvent(e);
            currentPlayer->game->putInGraveyard(currentPlayer->game->hand->cards[0]);
        }
        mLayers->actionLayer()->Update(0);
        currentPlayer->drawCounter = 0;
        currentPlayer->prowledTypes.clear();
        currentPlayer->lifeLostThisTurn = 0;
        currentPlayer->opponent()->lifeLostThisTurn = 0;
        currentPlayer->lifeGainedThisTurn = 0;
        currentPlayer->opponent()->lifeGainedThisTurn = 0;
        currentPlayer->doesntEmpty->remove(currentPlayer->doesntEmpty);
        currentPlayer->opponent()->doesntEmpty->remove(currentPlayer->opponent()->doesntEmpty);
        nextPlayer();
        return nextGamePhase();
    }

    //Phase Specific actions
    switch (mCurrentGamePhase)
    {
    case MTG_PHASE_UNTAP:
        DebugTrace("Untap Phase -------------   Turn " << turn );
        untapPhase();
        break;
    case MTG_PHASE_COMBATBLOCKERS:
        receiveEvent(NEW WEventAttackersChosen());
        break;
    default:
        break;
    }
}

int GameObserver::cancelCurrentAction()
{
    SAFE_DELETE(targetChooser);
    return mLayers->actionLayer()->cancelCurrentAction();
}

void GameObserver::nextCombatStep()
{
    switch (combatStep)
    {
    case BLOCKERS:
        receiveEvent(NEW WEventBlockersChosen());
        receiveEvent(NEW WEventCombatStepChange(combatStep = TRIGGERS));
        return;

    case TRIGGERS:
        receiveEvent(NEW WEventCombatStepChange(combatStep = ORDER));
        return;
    case ORDER:
        receiveEvent(NEW WEventCombatStepChange(combatStep = FIRST_STRIKE));
        return;
    case FIRST_STRIKE:
        receiveEvent(NEW WEventCombatStepChange(combatStep = END_FIRST_STRIKE));
        return;
    case END_FIRST_STRIKE:
        receiveEvent(NEW WEventCombatStepChange(combatStep = DAMAGE));
        return;
    case DAMAGE:
        receiveEvent(NEW WEventCombatStepChange(combatStep = END_DAMAGE));
        return;
    case END_DAMAGE:
        ; // Nothing : go to next phase
    }
}

void GameObserver::userRequestNextGamePhase(bool allowInterrupt, bool log)
{
    if(log) {
        stringstream stream;
        stream << "next " << allowInterrupt << " " <<mCurrentGamePhase;
        logAction(currentPlayer, stream.str());
    }

    if(getCurrentTargetChooser() && getCurrentTargetChooser()->maxtargets == 1000)
    {
        getCurrentTargetChooser()->done = true;
        if(getCurrentTargetChooser()->source)
            cardClick(getCurrentTargetChooser()->source, 0, false);
    }
    if (allowInterrupt && mLayers->stackLayer()->getNext(NULL, 0, NOT_RESOLVED))
        return;
    if (getCurrentTargetChooser())
        return;
    //if (mLayers->actionLayer()->isWaitingForAnswer())
    //    return;
    // Wil 12/5/10: additional check, not quite understanding why TargetChooser doesn't seem active at this point.
    // If we deem that an extra cost payment needs to be made, don't allow the next game phase to proceed.
    // Here's what I find weird - if the extra cost is something like a sacrifice, doesn't that imply a TargetChooser?
    if (WaitForExtraPayment(NULL)) 
        return;
    /*if (OpenedDisplay)//dont let us fly through all the phases with grave and library box still open.
    {
        return;//I want this here, but it locks up on opponents turn, we need to come up with a clever way to close opened
        //displays, it makes no sense that you travel through 4 or 5 phases with library or grave still open.
    }*/
    Phase * cPhaseOld = phaseRing->getCurrentPhase();
    if (allowInterrupt && ((cPhaseOld->id == MTG_PHASE_COMBATBLOCKERS && combatStep == ORDER)
        || (cPhaseOld->id == MTG_PHASE_COMBATBLOCKERS && combatStep == TRIGGERS)
        || (cPhaseOld->id == MTG_PHASE_COMBATDAMAGE)
        || opponent()->isAI() 
        || options[Options::optionInterrupt(mCurrentGamePhase)].number
        || currentPlayer->offerInterruptOnPhase - 1 == mCurrentGamePhase
    ))
    {
        mLayers->stackLayer()->AddNextGamePhase();
    }
    else
    {
       nextGamePhase(); 
    }
}

void GameObserver::shuffleLibrary(Player* p)
{
    if(!p)
    {
        DebugTrace("FATAL: No Player To Shuffle");
        return;
    }
    logAction(p, "shufflelib");
    MTGLibrary * library = p->game->library;
    if(!library)
    {
        DebugTrace("FATAL: Player has no zones");
        return;
    }
    library->shuffle();

    for(unsigned int k = 0;k < library->placeOnTop.size();k++)
    {
        MTGCardInstance * toMove = library->placeOnTop[k];
        assert(toMove);
        p->game->putInZone(toMove,  p->game->temp, library);
    }
    library->placeOnTop.clear();

}


int GameObserver::forceShuffleLibraries()
{
    int result = 0;
    for (int i = 0; i < 2 ; ++i)
    {
        if (players[i]->game->library->needShuffle)
        {
            shuffleLibrary(players[i]);
            players[i]->game->library->needShuffle = false;
            ++result;
        }
    }

    return result;
}

void GameObserver::resetStartupGame()
{
    stringstream stream;
    startupGameSerialized = "";
    stream << *this;
    startupGameSerialized = stream.str();
//    DebugTrace("startGame\n");
//    DebugTrace(startupGameSerialized);
}

void GameObserver::startGame(GameType gtype, Rules * rules)
{
    mGameType = gtype;
    turn = 0;
    mRules = rules;
    if (rules) 
        rules->initPlayers(this);

    options.automaticStyle(players[0], players[1]);

    mLayers = NEW DuelLayers(this);

    currentPlayerId = 0;
    currentPlayer = players[currentPlayerId];
    currentActionPlayer = currentPlayer;
    phaseRing = NEW PhaseRing(this);

    resetStartupGame();

    if (rules) 
        rules->initGame(this);

    //Preload images from hand
    if (!players[0]->isAI())
    {
        for (int i = 0; i < players[0]->game->hand->nb_cards; i++)
        {
            WResourceManager::Instance()->RetrieveCard(players[0]->game->hand->cards[i], CACHE_THUMB);
            WResourceManager::Instance()->RetrieveCard(players[0]->game->hand->cards[i]);
        }
    }

    startedAt = time(0);

    //Difficult mode special stuff
    if (!players[0]->isAI() && players[1]->isAI())
    {
        int difficulty = options[Options::DIFFICULTY].number;
        if (options[Options::DIFFICULTY_MODE_UNLOCKED].number && difficulty)
        {
            Player * p = players[1];
            for (int level = 0; level < difficulty; level++)
            {
                MTGCardInstance * card = NULL;
                MTGGameZone * z = p->game->library;
                for (int j = 0; j < z->nb_cards; j++)
                {
                    MTGCardInstance * _card = z->cards[j];
                    if (_card->isLand())
                    {
                        card = _card;
                        j = z->nb_cards;
                    }
                }
                if (card)
                {
                    MTGCardInstance * copy = p->game->putInZone(card, p->game->library, p->game->stack);
                    Spell * spell = NEW Spell(this, copy);
                    spell->resolve();
                    delete spell;
                }
            }
        }
    }

    switch(gtype) {
        case GAME_TYPE_MOMIR:
        {
            addObserver(NEW MTGMomirRule(this, -1, MTGCollection()));
            break;
        }
        case GAME_TYPE_STONEHEWER:
        {
            addObserver(NEW MTGStoneHewerRule(this, -1,MTGCollection()));
            break;
        }
        case GAME_TYPE_HERMIT:
        {
            addObserver(NEW MTGHermitRule(this, -1));
            break;
        }
        default:
            break;
    }
}

void GameObserver::addObserver(MTGAbility * observer)
{
    mLayers->actionLayer()->Add(observer);
}

//Returns true if the Ability was correctly removed from the game, false otherwise
//Main (valid) reason of returning false is an attempt at removing an Ability that has already been removed
bool GameObserver::removeObserver(ActionElement * observer)
{
    if (!observer)
        return false;
    return mLayers->actionLayer()->moveToGarbage(observer);

}

bool GameObserver::operator==(const GameObserver& aGame)
{
    int error = 0;

    if (aGame.mCurrentGamePhase != mCurrentGamePhase)
    {
        error++;
    }
    for (int i = 0; i < 2; i++)
    {
        Player * p = aGame.players[i];

        if (p->life != players[i]->life)
        {
            error++;
        }
        if (p->poisonCount != players[i]->poisonCount)
        {
            error++;
        }
        if (!p->getManaPool()->canAfford(players[i]->getManaPool()))
        {
            error++;
        }
        if (!players[i]->getManaPool()->canAfford(p->getManaPool()))
        {
            error++;
        }
        MTGGameZone * aZones[] = { p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay, p->game->exile };
        MTGGameZone * thisZones[] = { players[i]->game->graveyard,
                                         players[i]->game->library,
                                         players[i]->game->hand,
                                         players[i]->game->inPlay,
                                         players[i]->game->exile };
        for (int j = 0; j < 5; j++)
        {
            MTGGameZone * zone = aZones[j];
            if (zone->nb_cards != thisZones[j]->nb_cards)
            {
                error++;
            }
            for (size_t k = 0; k < (size_t)thisZones[j]->nb_cards; k++)
            {
                MTGCardInstance* cardToCheck = (k<thisZones[j]->cards.size())?thisZones[j]->cards[k]:0;
                MTGCardInstance* card = (k<aZones[j]->cards.size())?aZones[j]->cards[k]:0;
                if(!card || !cardToCheck || cardToCheck->getId() != card->getId())
                {
                    error++;
                }
            }
        }
    }

    return (error == 0);
}

void GameObserver::dumpAssert(bool val)
{
    if(!val)
    {
        cerr << *this << endl;
        assert(0);
    }
}


void GameObserver::Update(float dt)
{
    Player * player = currentPlayer;
    if (MTG_PHASE_COMBATBLOCKERS == mCurrentGamePhase && BLOCKERS == combatStep)
    {
        player = player->opponent();
    }
    if(getCurrentTargetChooser() && getCurrentTargetChooser()->Owner && player != getCurrentTargetChooser()->Owner)
    {
        if(getCurrentTargetChooser()->Owner != currentlyActing())
        {
            player = getCurrentTargetChooser()->Owner;
            isInterrupting = player;
        }
    }
    currentActionPlayer = player;
    if (isInterrupting) 
        player = isInterrupting;
    if(mLayers)
    {
        mLayers->Update(dt, player);
        while (mLayers->actionLayer()->stuffHappened)
        {
            mLayers->actionLayer()->Update(0);
        }
        gameStateBasedEffects();
    }
    oldGamePhase = mCurrentGamePhase;
}

//applies damage to creatures after updates
//Players life test
//Handles game state based effects
void GameObserver::gameStateBasedEffects()
{
    if(getCurrentTargetChooser() && int(getCurrentTargetChooser()->getNbTargets()) == getCurrentTargetChooser()->maxtargets)
        getCurrentTargetChooser()->done = true;
    /////////////////////////////////////
    for (int d = 0; d < 2; d++)
    {
        ////check snow count
        if (players[d]->snowManaC > players[d]->getManaPool()->getCost(0) + players[d]->getManaPool()->getCost(6))
            players[d]->snowManaC = players[d]->getManaPool()->getCost(0) + players[d]->getManaPool()->getCost(6);
        if (players[d]->snowManaC < 0)
            players[d]->snowManaC = 0;
        if (players[d]->snowManaG > players[d]->getManaPool()->getCost(1))
            players[d]->snowManaG = players[d]->getManaPool()->getCost(1);
        if (players[d]->snowManaG < 0)
            players[d]->snowManaG = 0;
        if (players[d]->snowManaU > players[d]->getManaPool()->getCost(2))
            players[d]->snowManaU = players[d]->getManaPool()->getCost(2);
        if (players[d]->snowManaU < 0)
            players[d]->snowManaU = 0;
        if (players[d]->snowManaR > players[d]->getManaPool()->getCost(3))
            players[d]->snowManaR = players[d]->getManaPool()->getCost(3);
        if (players[d]->snowManaR < 0)
            players[d]->snowManaR = 0;
        if (players[d]->snowManaB > players[d]->getManaPool()->getCost(4))
            players[d]->snowManaB = players[d]->getManaPool()->getCost(4);
        if (players[d]->snowManaB < 0)
            players[d]->snowManaB = 0;
        if (players[d]->snowManaW > players[d]->getManaPool()->getCost(5))
            players[d]->snowManaW = players[d]->getManaPool()->getCost(5);
        if (players[d]->snowManaW < 0)
            players[d]->snowManaW = 0;

        MTGGameZone * dzones[] = { players[d]->game->inPlay, players[d]->game->graveyard, players[d]->game->hand, players[d]->game->library, players[d]->game->exile, players[d]->game->stack };
        for (int k = 0; k < 6; k++)
        {
            MTGGameZone * zone = dzones[k];
            if (mLayers->stackLayer()->count(0, NOT_RESOLVED) == 0)
            {
                for (int c = zone->nb_cards - 1; c >= 0; c--)
                {
                    zone->cards[c]->cardistargetted = 0;
                    zone->cards[c]->cardistargetter = 0;
                }
            }

            ///while checking all these zones, lets also strip devoid cards of thier colors
            for (int w = 0; w < zone->nb_cards; w++)
            {
                MTGCardInstance * card = zone->cards[w];
                for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
                {
                    if (card->has(Constants::DEVOID))
                    {
                        card->removeColor(i);
                    }
                }
                //reset alternate paid
                if(card && (isInGrave(card)||isInHand(card)||isInExile(card)))
                {
                    for (int i = 0; i < ManaCost::MANA_PAID_WITH_BESTOW +1; i++)
                        card->alternateCostPaid[i] = 0;
                }
            }


        }//check for losers if its GAMEOVER clear the stack to allow gamestateeffects to continue
        players[d]->DeadLifeState();
    }
    ////////////////////////////////////

    if (mLayers->stackLayer()->count(0, NOT_RESOLVED) != 0)
        return;
    if (mLayers->actionLayer()->menuObject) 
        return;
    if (getCurrentTargetChooser() || mLayers->actionLayer()->isWaitingForAnswer()) 
        return;
    ////////////////////////
    //---apply damage-----//
    //after combat effects//
    ////////////////////////
    for (int i = 0; i < 2; i++)
    {
        MTGGameZone * zone = players[i]->game->inPlay;
        players[i]->curses.clear();
        for (int j = zone->nb_cards - 1; j >= 0; j--)
        {
            MTGCardInstance * card = zone->cards[j];
            card->entersBattlefield = 0;
            card->LKIpower = card->power;
            card->LKItoughness = card->toughness;
            card->LKIbasicAbilities = card->basicAbilities;
            card->afterDamage();
            card->mPropertiesChangedSinceLastUpdate = false;
            if(card->hasType(Subtypes::TYPE_PLANESWALKER) && (!card->counters||!card->counters->hasCounter("loyalty",0,0)))
                players[i]->game->putInGraveyard(card);
            if(card->myPair && !isInPlay(card->myPair))
            {
                card->myPair->myPair = NULL;
                card->myPair = NULL;
            }
            ///clear imprints
            if(isInPlay(card) && card->imprintedCards.size())
            {
                for(size_t ic = 0; ic < card->imprintedCards.size(); ic++)
                {
                    if(!isInExile(card->imprintedCards[ic])) 
                    {
                        card->imprintG = 0;
                        card->imprintU = 0;
                        card->imprintR = 0;
                        card->imprintB = 0;
                        card->imprintW = 0;
                        card->currentimprintName = "";
                        card->imprintedNames.clear();
                        card->imprintedCards.erase(card->imprintedCards.begin() + ic);
                    }
                }
            }
            card->bypassTC = false; //turn off bypass
            ///////////////////////////
            //reset extracost shadows//
            ///////////////////////////
            card->isExtraCostTarget = false;
            if (mExtraPayment != NULL)
            {
                for (unsigned int ec = 0; ec < mExtraPayment->costs.size(); ec++)
                {

                    if (mExtraPayment->costs[ec]->tc)
                    {
                        vector<Targetable*>targetlist = mExtraPayment->costs[ec]->tc->getTargetsFrom();
                        for (vector<Targetable*>::iterator it = targetlist.begin(); it != targetlist.end(); it++)
                        {
                            Targetable * cardMasked = *it;
                            dynamic_cast<MTGCardInstance*>(cardMasked)->isExtraCostTarget = true;
                        }

                    }
                }
            }
            ////////////////////////////////////////////////////
            //Unattach Equipments that dont have valid targets//
            ////////////////////////////////////////////////////
            if (card->hasType(Subtypes::TYPE_EQUIPMENT))
            {
                if(isInPlay(card))
                {
                    for (size_t i = 1; i < mLayers->actionLayer()->mObjects.size(); i++)
                    {
                        MTGAbility * a = ((MTGAbility *) mLayers->actionLayer()->mObjects[i]);
                        AEquip * eq = dynamic_cast<AEquip*> (a);
                        if (eq && eq->source == card)
                        {
                            if(card->target)//unattach equipments from cards that has protection from quality ex. protection from artifacts
                            {
                                if((card->target)->protectedAgainst(card)||card->isCreature())
                                    ((AEquip*)a)->unequip();
                            }
                            if(card->controller())
                                ((AEquip*)a)->getActionTc()->Owner = card->controller();
                            //fix for equip ability when the equipment changed controller... 
                        }
                    }
                }
            }

            ///////////////////////////////////////////////////////
            //Remove auras that don't have a valid target anymore//
            ///////////////////////////////////////////////////////
            if (card->target && !isInPlay(card->target) && card->isBestowed && card->hasType("aura"))
            {
                card->removeType("aura");
                card->addType("creature");
                card->target = NULL;
                card->isBestowed = false;
            }

            if ((card->target||card->playerTarget) && !card->hasType(Subtypes::TYPE_EQUIPMENT))
            {
                if(card->target && !isInPlay(card->target))
                players[i]->game->putInGraveyard(card);
                /*if(card->target && isInPlay(card->target))
                {//what exactly does this section do?
                    if(card->spellTargetType.find("creature") != string::npos && !card->target->hasType("creature"))
                        players[i]->game->putInGraveyard(card);
                    if(card->spellTargetType.find("artifact") != string::npos && !card->target->hasType("artifact"))
                        players[i]->game->putInGraveyard(card);
                    if(card->spellTargetType.find("enchantment") != string::npos && !card->target->hasType("enchantment"))
                        players[i]->game->putInGraveyard(card);
                    if(card->spellTargetType.find("land") != string::npos && !card->target->hasType("land"))
                        players[i]->game->putInGraveyard(card);
                    if(card->spellTargetType.find("planeswalker") != string::npos && !card->target->hasType("planeswalker"))
                        players[i]->game->putInGraveyard(card);
                }*/
                if(card->target && isInPlay(card->target) && (card->target)->protectedAgainst(card) && !card->has(Constants::AURAWARD))//protection from quality except aura cards like flickering ward
                players[i]->game->putInGraveyard(card);
            }
            card->enchanted = false;
            if (card->target && isInPlay(card->target) && !card->hasType(Subtypes::TYPE_EQUIPMENT) && card->hasSubtype(Subtypes::TYPE_AURA))
            {
                card->target->enchanted = true;
            }
            if (card->playerTarget && card->hasType("curse"))
            {
                card->playerTarget->curses.push_back(card);
            }

            //////////////////////
            //reset morph hiding//
            //////////////////////
            if((card->previous && card->previous->morphed && !card->turningOver) || (card->morphed && !card->turningOver))
            {
                card->morphed = true;
                card->isMorphed = true;
            }
            else
            {
                card->isMorphed = false;
                card->morphed = false;
            }
            //////////////////////////
            //handles phasing events//
            //////////////////////////
            if(card->has(Constants::PHASING)&& mCurrentGamePhase == MTG_PHASE_UNTAP && currentPlayer == card->controller() && card->phasedTurn != turn && !card->isPhased)
            {
                card->isPhased = true;
                card->phasedTurn = turn;
                if(card->view)
                    card->view->alpha = 50;
                card->initAttackersDefensers();
            }
            else if((card->has(Constants::PHASING) || card->isPhased)&& mCurrentGamePhase == MTG_PHASE_UNTAP && currentPlayer == card->controller() && card->phasedTurn != turn)
            {
                card->isPhased = false;
                card->phasedTurn = turn;
                if(card->view)
                    card->view->alpha = 255;
            }
            if (card->target && isInPlay(card->target) && (card->hasSubtype(Subtypes::TYPE_EQUIPMENT) || card->hasSubtype(Subtypes::TYPE_AURA)))
            {
                card->isPhased = card->target->isPhased;
                card->phasedTurn = card->target->phasedTurn;
                if(card->view && card->target->view)
                    card->view->alpha = card->target->view->alpha;
            }
            //////////////////////////  
            //forceDestroy over ride//
            //////////////////////////
            if(card->isInPlay(this))
            {
                card->graveEffects = false;
                card->exileEffects = false;

                if(card->isCreature())
                {
                    if(card->life < 1 && !card->has(Constants::INDESTRUCTIBLE))
                        card->destroy();//manor gargoyle... recheck
                }
            }

            if(card->childrenCards.size())
            {
                MTGCardInstance * check = NULL;
                MTGCardInstance * matched = NULL;
                sort(card->childrenCards.begin(),card->childrenCards.end());
                for(size_t wC = 0; wC < card->childrenCards.size();wC++)
                {
                    check = card->childrenCards[wC];
                    for(size_t wCC = 0; wCC < card->childrenCards.size();wCC++)
                    {
                        if(check->isInPlay(this))
                        {
                            if(check->getName() == card->childrenCards[wCC]->getName() && check != card->childrenCards[wCC])
                            {
                                card->isDualWielding = true;
                                matched = card->childrenCards[wCC];
                            }
                        }
                    }
                    if(matched)
                        wC = card->childrenCards.size();
                }
                if(!matched)
                    card->isDualWielding = false;
            }
        }
    }
    //-------------------------------------

    for (int i = 0; i < 2; i++)
    {
        ///////////////////////////////////////////////////////////
        //life checks/poison checks also checks cant win or lose.//
        ///////////////////////////////////////////////////////////
        players[i]->DeadLifeState(true);//refactored
    }
    //////////////////////////////////////////////////////
    //-------------card based states effects------------//
    //////////////////////////////////////////////////////
    //ie:cantcast; extra land; extra turn;no max hand;--//
    //////////////////////////////////////////////////////

    for (int i = 0; i < 2; i++)
    {
        //checks if a player has a card which has the stated ability in play.
        Player * p = players[i];
        MTGGameZone * z = players[i]->game->inPlay;
        int nbcards = z->nb_cards;
        //------------------------------
        if(z->hasAbility(Constants::NOMAXHAND)||p->opponent()->inPlay()->hasAbility(Constants::OPPNOMAXHAND))
            p->nomaxhandsize = true;
        else
            p->nomaxhandsize = false;
        /////////////////////////////////////////////////
        //handle end of turn effects while we're at it.//
        /////////////////////////////////////////////////
        if (mCurrentGamePhase == MTG_PHASE_ENDOFTURN+1)
        {
            for (int j = 0; j < nbcards; ++j)
            {
                MTGCardInstance * c = z->cards[j];

                if(!c)break;
                while (c->flanked)
                {
                    /////////////////////////////////
                    //undoes the flanking on a card//
                    /////////////////////////////////
                    c->power += 1;
                    c->addToToughness(1);
                    c->flanked -= 1;
                }
                c->fresh = 0;
                if(c->wasDealtDamage && c->isInPlay(this))
                    c->wasDealtDamage = false;
                c->damageToController = false;
                c->damageToOpponent = false;
                c->combatdamageToOpponent = false;
                c->damageToCreature = false;
                c->isAttacking = NULL;
            }
            for (int t = 0; t < nbcards; t++)
            {
                MTGCardInstance * c = z->cards[t];

                if(!c->isPhased)
                {
                    if (c->has(Constants::TREASON))
                    {
                        MTGCardInstance * beforeCard = c;
                        p->game->putInGraveyard(c);
                        WEvent * e = NEW WEventCardSacrifice(beforeCard,c);
                        receiveEvent(e);
                    }
                    if (c->has(Constants::UNEARTH))
                    {
                        p->game->putInExile(c);

                    }
                }

                if(nbcards > z->nb_cards)
                {
                    t = 0;
                    nbcards = z->nb_cards;
                }
            }

            MTGGameZone * f = p->game->graveyard;
            for (int k = 0; k < f->nb_cards; k++)
            {
                MTGCardInstance * card = f->cards[k];
                card->fresh = 0;
            }
        }
        if (z->nb_cards == 0)
        {
            p->nomaxhandsize = false;
        }
        //////////////////////////
        // Check auras on a card//
        //////////////////////////
        enchantmentStatus();
        /////////////////////////////
        // Check affinity on a card//
        //    plus modify costs    //
        /////////////////////////////
        Affinity();
        /////////////////////////////////////
        // Check colored statuses on cards //
        /////////////////////////////////////
        for(int w = 0;w < z->nb_cards;w++)
        {  
            int colored = 0;
            for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
            {
                if (z->cards[w]->hasColor(i))
                    ++colored;
            }
            z->cards[w]->isMultiColored = (colored > 1) ? 1 : 0;
        }
    }
    ///////////////////////////////////
    //phase based state effects------//
    ///////////////////////////////////
    if (combatStep == TRIGGERS)
    {
        if (!mLayers->stackLayer()->getNext(NULL, 0, NOT_RESOLVED) && !targetChooser
            && !mLayers->actionLayer()->isWaitingForAnswer()) 
            mLayers->stackLayer()->AddNextCombatStep();
    }

    //Auto skip Phases
    int skipLevel = (currentPlayer->playMode == Player::MODE_TEST_SUITE || mLoading) ? Constants::ASKIP_NONE
        : options[Options::ASPHASES].number;

    if (skipLevel == Constants::ASKIP_SAFE || skipLevel == Constants::ASKIP_FULL)
    {
        if ((opponent()->isAI() && !(isInterrupting)) && ((mCurrentGamePhase == MTG_PHASE_UNTAP)
            || (mCurrentGamePhase == MTG_PHASE_DRAW) || (mCurrentGamePhase == MTG_PHASE_COMBATBEGIN)
            || ((mCurrentGamePhase == MTG_PHASE_COMBATATTACKERS) && (currentPlayer->noPossibleAttackers()))
            || mCurrentGamePhase == MTG_PHASE_COMBATEND || mCurrentGamePhase == MTG_PHASE_ENDOFTURN
            || ((mCurrentGamePhase == MTG_PHASE_CLEANUP) && (currentPlayer->game->hand->nb_cards < 8))))
            userRequestNextGamePhase();
    }
    if (skipLevel == Constants::ASKIP_FULL)
    {
        if ((opponent()->isAI() && !(isInterrupting)) && (mCurrentGamePhase == MTG_PHASE_UPKEEP
            || mCurrentGamePhase == MTG_PHASE_COMBATDAMAGE))
            userRequestNextGamePhase();
    }

    //WEventGameStateBasedChecked event checked
    receiveEvent(NEW WEventGameStateBasedChecked());
}

void GameObserver::enchantmentStatus()
{
    for (int i = 0; i < 2; i++)
    {
        MTGGameZone * zone = players[i]->game->inPlay;
        for (int k = zone->nb_cards - 1; k >= 0; k--)
        {
            MTGCardInstance * card = zone->cards[k];
            if (card && !card->hasType(Subtypes::TYPE_EQUIPMENT) && !card->hasSubtype(Subtypes::TYPE_AURA))
            {
                card->enchanted = false;
                card->auras = 0;
            }
        }
        for (int j = zone->nb_cards - 1; j >= 0; j--)
        {
            MTGCardInstance * card = zone->cards[j];
            if (card->target && isInPlay(card->target) && !card->hasType(Subtypes::TYPE_EQUIPMENT) && card->hasSubtype(Subtypes::TYPE_AURA))
            {
                card->target->enchanted = true;
                card->target->auras += 1;
            }
        }
    }
}

void GameObserver::Affinity()
{
    for (int dd = 0; dd < 2; dd++)
    {
        MTGGameZone * dzones[] = { players[dd]->game->graveyard, players[dd]->game->hand, players[dd]->game->library, players[dd]->game->exile };
        for (int kk = 0; kk < 4; kk++)
        { 
            MTGGameZone * zone = dzones[kk];
            for (int cc = zone->nb_cards - 1; cc >= 0; cc--)
            {//start
                MTGCardInstance * card = zone->cards[cc];
                if (!card)
                    continue;

                ///////////////////////////
                //reset extracost shadows//
                ///////////////////////////
                card->isExtraCostTarget = false;
                if (mExtraPayment != NULL)
                {
                    for (unsigned int ec = 0; ec < mExtraPayment->costs.size(); ec++)
                    {

                        if (mExtraPayment->costs[ec]->tc)
                        {
                            vector<Targetable*>targetlist = mExtraPayment->costs[ec]->tc->getTargetsFrom();
                            for (vector<Targetable*>::iterator it = targetlist.begin(); it != targetlist.end(); it++)
                            {
                                Targetable * cardMasked = *it;
                                dynamic_cast<MTGCardInstance*>(cardMasked)->isExtraCostTarget = true;
                            }

                        }
                    }
                }
                ///we handle trisnisphere seperately because its a desaster.
                if(card->getManaCost())//make sure we check, abiliy$!/token dont have a mancost object.
                {
                    //change cost to colorless for anytypeofmana ability
                    if(card->has(Constants::ANYTYPEOFMANA))
                    {
                        card->anymanareplacement = true;
                        int convertedC = card->getManaCost()->getConvertedCost();
                        card->getManaCost()->changeCostTo( NEW ManaCost(ManaCost::parseManaCost("{0}", NULL, card)) );
                        for (int jj = 0; jj < convertedC; jj++)
                        {
                            card->getManaCost()->add(Constants::MTG_COLOR_ARTIFACT, 1);
                        }
                    }
                    else
                    {
                        if (card->anymanareplacement)
                        {
                            card->getManaCost()->changeCostTo( card->model->data->getManaCost() );
                            card->anymanareplacement = false;
                        }
                    }

                    if (card->has(Constants::TRINISPHERE))
                    {
                        for (int jj = card->getManaCost()->getConvertedCost(); jj < 3; jj++)
                        {
                            card->getManaCost()->add(Constants::MTG_COLOR_ARTIFACT, 1);
                            card->countTrini++;
                        }
                    }
                    else
                    {
                        if (card->countTrini)
                        {
                            card->getManaCost()->remove(Constants::MTG_COLOR_ARTIFACT, card->countTrini);
                            card->countTrini = 0;
                        }
                    }
                }
                ///////////////////////
                bool NewAffinityFound = false;
                for (unsigned int na = 0; na < card->cardsAbilities.size(); na++)
                {
                    if (!card->cardsAbilities[na])
                        break;
                    ANewAffinity * newAff = dynamic_cast<ANewAffinity*>(card->cardsAbilities[na]);
                    if (newAff)
                    {
                        NewAffinityFound = true;
                    }
                }
                bool DoReduceIncrease = false;
                if (
                    (card->has(Constants::AFFINITYARTIFACTS) ||
                    card->has(Constants::AFFINITYFOREST) ||
                    card->has(Constants::AFFINITYGREENCREATURES) ||
                    card->has(Constants::AFFINITYISLAND) ||
                    card->has(Constants::AFFINITYMOUNTAIN) ||
                    card->has(Constants::AFFINITYPLAINS) ||
                    card->has(Constants::AFFINITYSWAMP) ||
                    card->has(Constants::CONDUITED) ||
                    card->getIncreasedManaCost()->getConvertedCost() ||
                    card->getReducedManaCost()->getConvertedCost() ||
                    NewAffinityFound)
                    &&
                    AffinityNeedsUpdate
                    )
                    DoReduceIncrease = true;
                if (!DoReduceIncrease)
                    continue;

                //above we check if there are even any cards that effect cards manacost
                //only do any of the following if a card with the stated ability is in your hand.
                //kicker is an addon to normal cost, suspend is not casting. add cost as needed EXACTLY as seen below.
                card->getManaCost()->resetCosts();
                ManaCost *newCost = NEW ManaCost();
                newCost->changeCostTo(card->computeNewCost(card, card->getManaCost(), card->model->data->getManaCost()));

                card->getManaCost()->changeCostTo(newCost);
                SAFE_DELETE(newCost);
                if (card->getManaCost()->getAlternative())
                {
                    card->getManaCost()->getAlternative()->resetCosts();
                    ManaCost * newCost = NEW ManaCost();
                    newCost->changeCostTo(card->computeNewCost(card, card->getManaCost()->getAlternative(), card->model->data->getManaCost()->getAlternative()));
                    card->getManaCost()->getAlternative()->changeCostTo(newCost);
                    SAFE_DELETE(newCost);
                }
                if (card->getManaCost()->getBestow())
                {
                    card->getManaCost()->getBestow()->resetCosts();
                    ManaCost * newCost = NEW ManaCost();
                    newCost->changeCostTo(card->computeNewCost(card, card->getManaCost()->getBestow(), card->model->data->getManaCost()->getBestow()));
                    card->getManaCost()->getBestow()->changeCostTo(newCost);
                    SAFE_DELETE(newCost);
                }
                if (card->getManaCost()->getRetrace())
                {
                    card->getManaCost()->getRetrace()->resetCosts();
                    ManaCost * newCost = NEW ManaCost();
                    newCost->changeCostTo(card->computeNewCost(card, card->getManaCost()->getRetrace(), card->model->data->getManaCost()->getRetrace()));
                    card->getManaCost()->getRetrace()->changeCostTo(newCost);
                    SAFE_DELETE(newCost);
                }
                if (card->getManaCost()->getBuyback())
                {
                    card->getManaCost()->getBuyback()->resetCosts();
                    ManaCost * newCost = NEW ManaCost();
                    newCost->changeCostTo(card->computeNewCost(card, card->getManaCost()->getBuyback(), card->model->data->getManaCost()->getBuyback()));
                    card->getManaCost()->getBuyback()->changeCostTo(newCost);
                    SAFE_DELETE(newCost);
                }
                if (card->getManaCost()->getFlashback())
                {
                    card->getManaCost()->getFlashback()->resetCosts();
                    ManaCost * newCost = NEW ManaCost();
                    newCost->changeCostTo(card->computeNewCost(card, card->getManaCost()->getFlashback(), card->model->data->getManaCost()->getFlashback()));
                    card->getManaCost()->getFlashback()->changeCostTo(newCost);
                    SAFE_DELETE(newCost);
                }
                if (card->getManaCost()->getMorph())
                {
                    card->getManaCost()->getMorph()->resetCosts();
                    ManaCost * newCost = NEW ManaCost();
                    newCost->changeCostTo(card->computeNewCost(card, card->getManaCost()->getMorph(), card->model->data->getManaCost()->getMorph()));
                    card->getManaCost()->getMorph()->changeCostTo(newCost);
                    SAFE_DELETE(newCost);
                }

            }//end
        }
    }
    AffinityNeedsUpdate = false;
}

void GameObserver::Render()
{
    if(mLayers)
        mLayers->Render();
    if (targetChooser || (mLayers && mLayers->actionLayer()->isWaitingForAnswer()))
        JRenderer::GetInstance()->DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(255,255,0,0));
    if (mExtraPayment) 
        mExtraPayment->Render();
    
    for (size_t i = 0; i < players.size(); ++i)
    {
        players[i]->Render();
    }
}

void GameObserver::ButtonPressed(PlayGuiObject * target)
{
    DebugTrace("GAMEOBSERVER Click");
    if (CardView* cardview = dynamic_cast<CardView*>(target))
    {
        MTGCardInstance * card = cardview->getCard();
        cardClick(card, card);
    }
    else if (GuiLibrary* library = dynamic_cast<GuiLibrary*>(target))
    {
        if (library->showCards)
        {
            library->toggleDisplay();
            forceShuffleLibraries();
        }
        else
        {
            TargetChooser * _tc = this->getCurrentTargetChooser();
            if (_tc && _tc->targetsZone(library->zone))
            {
                library->toggleDisplay();
                library->zone->needShuffle = true;
            }
        }
    }
    else if (GuiGraveyard* graveyard = dynamic_cast<GuiGraveyard*>(target))
        graveyard->toggleDisplay();
    else if (GuiExile* exile = dynamic_cast<GuiExile*>(target))
        exile->toggleDisplay();
    //opponenthand
    else if (GuiOpponentHand* opponentHand = dynamic_cast<GuiOpponentHand*>(target))
        if (opponentHand->showCards)
        {
            opponentHand->toggleDisplay();
        }
        else
        {
            TargetChooser * _tc = this->getCurrentTargetChooser();
            if (_tc && _tc->targetsZone(opponentHand->zone))
            {
                opponentHand->toggleDisplay();
            }
        }
    //end opponenthand
    else if (GuiAvatar* avatar = dynamic_cast<GuiAvatar*>(target))
    {
        cardClick(NULL, avatar->player);
    }
    else if (dynamic_cast<GuiPhaseBar*>(target))
    {
        mLayers->getPhaseHandler()->NextGamePhase();
    }
}

void GameObserver::stackObjectClicked(Interruptible * action)
{
    stringstream stream;
    stream << "stack[" << mLayers->stackLayer()->getIndexOf(action) << "]";
    logAction(currentlyActing(), stream.str());

    if (targetChooser != NULL)
    {
        int result = targetChooser->toggleTarget(action);
        if (result == TARGET_OK_FULL)
        {
            cardClick(cardWaitingForTargets, 0, false);
        }
        else
        {
            return;
        }
    }
    else
    {
        int reaction = mLayers->actionLayer()->isReactingToTargetClick(action);
        if (reaction == -1) 
            mLayers->actionLayer()->reactToTargetClick(action);
    }
}

bool GameObserver::WaitForExtraPayment(MTGCardInstance * card)
{
    bool result = false;
    if (mExtraPayment)
    {
        if (card)
        {
            mExtraPayment->tryToSetPayment(card);
        }
        if (mExtraPayment->isPaymentSet())
        {
            mLayers->actionLayer()->reactToClick(mExtraPayment->action, mExtraPayment->source);
            mExtraPayment = NULL;
        }
        result = true;
    }

    return result;
}

int GameObserver::cardClick(MTGCardInstance * card, MTGAbility *ability)
{
    MTGGameZone* zone = card->currentZone;
    size_t index  = 0;
    if(zone)
        index = zone->getIndex(card);
    int choice;
    bool logChoice = mLayers->actionLayer()->getMenuIdFromCardAbility(card, ability, choice);
    int result = ability->reactToClick(card);
    logAction(card, zone, index, result);

    if(logChoice) {
        stringstream stream;
        stream << "choice " << choice;
        logAction(currentActionPlayer, stream.str());
    }

    return result;
}

int GameObserver::cardClick(MTGCardInstance * card, int abilityType)
{
    int result = 0;
    MTGAbility * a = mLayers->actionLayer()->getAbility(abilityType);

    if(a)
    {
        result = cardClick(card, a);
    }

    return result;
}

int GameObserver::cardClickLog(bool log, Player* clickedPlayer, MTGGameZone* zone, MTGCardInstance*backup, size_t index, int toReturn)
{
    if(log)
    {
        if (clickedPlayer) {
            this->logAction(clickedPlayer);
        } else if(zone)  {
            this->logAction(backup, zone, index, toReturn);
        }
    }
    return toReturn;
}

int GameObserver::cardClick(MTGCardInstance * card, Targetable * object, bool log)
{
    Player * clickedPlayer = NULL;
    int toReturn = 0;
    int handmodified = 0;
    MTGGameZone* zone = NULL;
    size_t index = 0;
    MTGCardInstance* backup = NULL;

    if (!card) {
        clickedPlayer = ((Player *) object);
    } else {
        backup = card;
        zone = card->currentZone;
        if(zone)
        {
            index = zone->getIndex(card);
        }
    }

    do {
        if (targetChooser)
        {
            int result;
            if (card)
            {
                if (card == cardWaitingForTargets)
                {
                    int _result = targetChooser->ForceTargetListReady();
                    if(targetChooser->targetMin && int(targetChooser->getNbTargets()) < targetChooser->maxtargets)
                        _result = 0;

                    if (_result)
                    {
                        result = TARGET_OK_FULL;
                    }
                    else
                    {
                        result = targetChooser->targetsReadyCheck();
                    }
                }
                else
                {
                    result = targetChooser->toggleTarget(card);
                    WEvent * e = NEW WEventTarget(card,cardWaitingForTargets);
                    receiveEvent(e);
                }
            }
            else
            {
                result = targetChooser->toggleTarget(clickedPlayer);
                if(card)
                    card->playerTarget = clickedPlayer;
                else
                    targetChooser->source->playerTarget = clickedPlayer;
            }
            if (result == TARGET_OK_FULL)
                card = cardWaitingForTargets;
            else {
                toReturn = 1;
                break;
            }
        }
        ExtraManaCost * costType = NULL;
        if( mExtraPayment && mExtraPayment->costs.size())
            costType = dynamic_cast<ExtraManaCost*>(mExtraPayment->costs[0]);

        if (WaitForExtraPayment(card) && !costType) 
        {
            toReturn = 1;
            break;
        }

        int reaction = 0;

        if (ORDER == combatStep)
        {
            //TODO it is possible at this point that card is NULL. if so, what do we return since card->defenser would result in a crash?
            card->defenser->raiseBlockerRankOrder(card);
            toReturn = 1;
            break;
        }

        if (card)
        {
            //card played as normal, alternative cost, buyback, flashback, retrace.

            //the variable "paymenttype = int" only serves one purpose, to tell this bug fix what menu item you clicked on...
            // all alternative cost or play methods suffered from the fix because if the card contained "target="
            // it would automatically force the play method to putinplayrule...even charge you the original mana cost.

            /* Fix for Issue http://code.google.com/p/wagic/issues/detail?id=270
             put into play is hopefully the only ability causing that kind of trouble
             If the same kind of issue occurs with other abilities, let's think of a cleaner solution
             */
            if (targetChooser)
            {
                MTGAbility * a = mLayers->actionLayer()->getAbility(card->paymenttype);
                toReturn = a->reactToClick(card);
                return cardClickLog(log, clickedPlayer, zone, backup, index, toReturn);
            }

            reaction = mLayers->actionLayer()->isReactingToClick(card);
            if (reaction == -1) {
                toReturn = mLayers->actionLayer()->reactToClick(card);
                return cardClickLog(log, clickedPlayer, zone, backup, index, toReturn);
            }
        }
        else
        {//this handles abilities on a menu...not just when card is being played
            reaction = mLayers->actionLayer()->isReactingToTargetClick(object);
            if (reaction == -1) {
                toReturn = mLayers->actionLayer()->reactToTargetClick(object);
                return cardClickLog(log, clickedPlayer, zone, backup, index, toReturn);
            }
        }

        if (!card) {
            toReturn = 0;
            return cardClickLog(log, clickedPlayer, zone, backup, index, toReturn);
        }

        //Current player's hand
        handmodified = currentPlayer->handsize+currentPlayer->handmodifier;
        if(handmodified < 0)
            handmodified = 0;
        if (currentPlayer->game->hand->hasCard(card) && mCurrentGamePhase == MTG_PHASE_CLEANUP
                    && currentPlayer->game->hand->nb_cards > handmodified && currentPlayer->nomaxhandsize == false)
        {
            WEvent * e = NEW WEventCardDiscard(currentPlayer->game->hand->cards[0]);
            receiveEvent(e);
            currentPlayer->game->putInGraveyard(card);
        }
        else if (reaction)
        {
            if (reaction == 1)
            {
                toReturn = mLayers->actionLayer()->reactToClick(card);
                return cardClickLog(log, clickedPlayer, zone, backup, index, toReturn);
            }
            else
            {
                mLayers->actionLayer()->setMenuObject(object);
                toReturn = 1;
                return cardClickLog(log, clickedPlayer, zone, backup, index, toReturn);
            }
        }
        else if (card->isTapped() && card->controller() == currentPlayer)
        {
            toReturn = untap(card);
            return cardClickLog(log, clickedPlayer, zone, backup, index, toReturn);
        }
    } while(0);


    return cardClickLog(log, clickedPlayer, zone, backup, index, toReturn);
}

int GameObserver::untap(MTGCardInstance * card)
{
    if (!card->isUntapping())
    {
        return 0;
    }
    if (card->has(Constants::DOESNOTUNTAP))
        return 0;
    if (card->frozen > 0) 
        return 0;
    card->attemptUntap();
    return 1;
}

TargetChooser * GameObserver::getCurrentTargetChooser()
{
    if(mLayers)
    {
        TargetChooser * _tc = mLayers->actionLayer()->getCurrentTargetChooser();
        if (_tc)
            return _tc;
    }
    return targetChooser;
}

/* Returns true if the card is in one of the player's play zone */
int GameObserver::isInPlay(MTGCardInstance * card)
{
    for (int i = 0; i < 2; i++)
    {
        if (players[i]->game->isInPlay(card)) 
            return 1;
    }
    return 0;
}
int GameObserver::isInGrave(MTGCardInstance * card)
{

    for (int i = 0; i < 2; i++)
    {
        MTGGameZone * graveyard = players[i]->game->graveyard;
        if (players[i]->game->isInZone(card,graveyard)) 
            return 1;
    }
    return 0;
}
int GameObserver::isInExile(MTGCardInstance * card)
{

    for (int i = 0; i < 2; i++)
    {
        MTGGameZone * exile = players[i]->game->exile;
        if (players[i]->game->isInZone(card,exile)) 
            return 1;
    }
    return 0;
}
int GameObserver::isInHand(MTGCardInstance * card)
{

    for (int i = 0; i < 2; i++)
    {
        MTGGameZone * hand = players[i]->game->hand;
        if (players[i]->game->isInZone(card, hand))
            return 1;
    }
    return 0;
}
void GameObserver::cleanupPhase()
{
    currentPlayer->cleanupPhase();
    opponent()->cleanupPhase();
}

void GameObserver::untapPhase()
{
    currentPlayer->inPlay()->untapAll();
}

int GameObserver::receiveEvent(WEvent * e)
{
    if (!e) 
        return 0;
    eventsQueue.push(e);
    if (eventsQueue.size() > 1) 
        return -1; //resolving events can generate more events
    int result = 0;
    while (eventsQueue.size())
    {
        WEvent * ev = eventsQueue.front();
        result += mLayers->receiveEvent(ev);
        for (int i = 0; i < 2; ++i)
        {
            result += players[i]->receiveEvent(ev);
        }
        SAFE_DELETE(ev);
        eventsQueue.pop();
    }
    AffinityNeedsUpdate = true;
    return result;
}

Player * GameObserver::currentlyActing()
{
    if (isInterrupting) 
        return isInterrupting;
    return currentActionPlayer;
}

//TODO CORRECT THIS MESS
int GameObserver::targetListIsSet(MTGCardInstance * card)
{
    if (targetChooser == NULL)
    {
        TargetChooserFactory tcf(this);
        targetChooser = tcf.createTargetChooser(card);
        if (targetChooser == NULL)
        {
            return 1;
        }
    }
    if(targetChooser && targetChooser->validTargetsExist())
    {
        cardWaitingForTargets = card;
        return (targetChooser->targetListSet());
    }
    else
        SAFE_DELETE(targetChooser);
    return 0;
    
}

ostream& operator<<(ostream& out, const GameObserver& g)
{
    if(g.startupGameSerialized == "")
    {
        out << "[init]" << endl;
        out << "player=" << g.currentPlayerId + 1 << endl;
        if(g.mCurrentGamePhase != MTG_PHASE_INVALID)
            out << "phase=" << g.phaseRing->phaseName(g.mCurrentGamePhase) << endl;
        out << "[player1]" << endl;
        out << *(g.players[0]) << endl;
        out << "[player2]" << endl;
        out << *(g.players[1]) << endl;
        return out;
    }
    else
    {
        out << "seed:";
        out << g.mSeed;
        out << endl;
        out << "rvalues:";
        g.randomGenerator.saveUsedRandValues(out);
        out << endl;
        out << g.startupGameSerialized;
    }

    out << "[do]" << endl;
    list<string>::const_iterator it;

    for(it = (g.actionsList.begin()); it != (g.actionsList.end()); it++)
    {
        out << (*it) << endl;
    }

    out << "[end]" << endl;
    return out;
}

bool GameObserver::parseLine(const string& s)
{
    size_t limiter = s.find("=");
    if (limiter == string::npos) limiter = s.find(":");
    string areaS;
    if (limiter != string::npos)
    {
        areaS = s.substr(0, limiter);
        if (areaS.compare("player") == 0)
        {
            currentPlayerId = atoi(s.substr(limiter + 1).c_str()) - 1;
            return true;
        }
        else if (areaS.compare("phase") == 0)
        {
            mCurrentGamePhase = PhaseRing::phaseStrToInt(s.substr(limiter + 1).c_str());
            return true;
        }
    }
    return false;
}

bool GameObserver::load(const string& ss, bool undo, int controlledPlayerIndex
#ifdef TESTSUITE
                    , TestSuiteGame* testgame
#endif
                        )
{
    bool currentPlayerSet = false;
    int state = -1;
    string s;
    stringstream stream(ss);

    DebugTrace("Loading " + ss);
    randomGenerator.loadRandValues("");

    cleanup();

    while (std::getline(stream, s))
    {
        if (!s.size()) continue;
        if (s[s.size() - 1] == '\r') s.erase(s.size() - 1); //Handle DOS files
        if (!s.size()) continue;
        if (s[0] == '#') continue;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s.find("seed ") == 0)
        {
            mSeed = atoi(s.substr(5).c_str());
            randomGenerator.setSeed(mSeed);
            continue;
        }
        if (s.find("rvalues:") == 0)
        {
            randomGenerator.loadRandValues(s.substr(8).c_str());
            continue;
        }
        switch (state)
        {
        case -1:
            if (s.compare("[init]") == 0)
                state++;
            break;
        case 0:
            if (s.compare("[player1]") == 0)
            {
                state++;
            }
            else
            {
                currentPlayerSet  = parseLine(s);
            }
            break;
        case 1:
            if (s.compare("[player2]") == 0)
            {
                state++;
            }
            else
            {
                if(players.size() == 0 || !players[0])
                {
                    if (s.find("mode=") == 0)
                    {
                        createPlayer(s.substr(5)
             #ifdef TESTSUITE
                                     , testgame
             #endif //TESTSUITE
                                     );
                    }
                }
                players[0]->parseLine(s);
            }
            break;
        case 2:
            if (s.compare("[do]") == 0)
            {
                state++;
            }
            else
            {
                if(players.size() == 1 || !players[1])
                {
                    if (s.find("mode=") == 0)
                    {
                        createPlayer(s.substr(5)
#ifdef TESTSUITE
                                     , testgame
#endif //TESTSUITE
                                     );
                    }
                }
                players[1]->parseLine(s);
            }
            break;
        case 3:
            if (s.compare("[end]") == 0)
            {
                turn = 0;
                mLayers = NEW DuelLayers(this, controlledPlayerIndex);
                currentPlayer = players[currentPlayerId];
                phaseRing = NEW PhaseRing(this);
                startedAt = time(0);

                // take a snapshot before processing the actions
                resetStartupGame();

                if(mRules) mRules->initGame(this, currentPlayerSet);
                phaseRing->goToPhase(0, currentPlayer, false);
                phaseRing->goToPhase(mCurrentGamePhase, currentPlayer);

#ifdef TESTSUITE
                if(testgame)
                    testgame->initGame();
#endif //TESTSUITE

                processActions(undo
               #ifdef TESTSUITE
                               , testgame
               #endif //TESTSUITE
                               );
            }
            else
            {
                logAction(s);
            }
            break;
        }
    }

    return true;
}

bool GameObserver::processAction(const string& s)
{
    Player* p = players[1];
    if (s.find("p1") != string::npos)
        p = players[0];

    MTGGameZone* zone = NULL;
    if(s.find(string(p->game->hand->getName())+"[") != string::npos)
        zone = p->game->hand;
    else if(s.find(string(p->game->battlefield->getName())+"[") != string::npos)
        zone = p->game->battlefield;
    else if(s.find(string(p->game->graveyard->getName())+"[") != string::npos)
        zone = p->game->graveyard;
    else if(s.find(string(p->game->library->getName())+"[") != string::npos)
        zone = p->game->library;

    if(zone) {
        size_t begin = s.find("[")+1;
        size_t size = s.find("]")-begin;
        size_t index = atoi(s.substr(begin, size).c_str());
        dumpAssert(index < zone->cards.size());
        cardClick(zone->cards[index], zone->cards[index]);
    } else if (s.find("stack") != string::npos) {
        size_t begin = s.find("[")+1;
        size_t size = s.find("]")-begin;
        size_t index = atoi(s.substr(begin, size).c_str());
        stackObjectClicked((Interruptible*)mLayers->stackLayer()->getByIndex(index));
    } else if (s.find("yes") != string::npos) {
        mLayers->stackLayer()->setIsInterrupting(p);
    } else if (s.find("no") != string::npos) {
        mLayers->stackLayer()->cancelInterruptOffer();
    } else if (s.find("endinterruption") != string::npos) {
        mLayers->stackLayer()->endOfInterruption();
    } else if (s.find("next") != string::npos) {
        userRequestNextGamePhase();
    } else if (s.find("combatok") != string::npos) {
        mLayers->combatLayer()->clickOK();
    } else if (s == "p1" || s == "p2") {
        cardClick(NULL, p);
    } else if (s.find("choice") != string::npos) {
        int choice = atoi(s.substr(s.find("choice ") + 7).c_str());
            mLayers->actionLayer()->doReactTo(choice);
    } else if (s == "p1" || s == "p2") {
        cardClick(NULL, p);
    } else if(s.find("mulligan") != string::npos) {
        Mulligan(p);
    } else if(s.find("shufflelib") != string::npos) {
        // This should probably be differently and be automatically part of the ability triggered
        // that would allow the AI to use it as well.
        shuffleLibrary(p);
    } else {
        DebugTrace("no clue about: " + s);
    }

    return true;
}

bool GameObserver::processActions(bool undo
                                  #ifdef TESTSUITE
                                  , TestSuiteGame* testgame
                                  #endif
                                  )
{
    bool result = false;
    size_t cmdIndex = 0;

    loadingList = actionsList;
    actionsList.clear();

    mLoading = true;
    float counter = 0.0f;

    // To handle undo, we'll remove the last P1 action and all P2 actions after.
    if(undo && loadingList.size()) {
        while(loadingList.back().find("p2") != string::npos)
            loadingList.pop_back();
        // we do not undo "next phase" action to avoid abuse by users
        if(loadingList.back().find("next") == string::npos)
            loadingList.pop_back();
    }

    // We fake here cause the initialization before caused mana pool reset events to be triggered
    // So, we need them flushed to be able to set the manapool to whatever we need
    GameObserver::Update(counter);
    counter += 1.000f;

#ifdef TESTSUITE
    if(testgame)
    {
        testgame->ResetManapools();
    }
#endif

    for(loadingite = loadingList.begin(); loadingite != loadingList.end(); loadingite++, cmdIndex++)
    {
        processAction(*loadingite);

        size_t nb = actionsList.size();

        for (int i = 0; i<6; i++)
        {
            // let's fake an update
            GameObserver::Update(counter);
            counter += 1.000f;
        }
        dumpAssert(actionsList.back() == *loadingite);
        dumpAssert(nb == actionsList.size());
        dumpAssert(cmdIndex == (actionsList.size()-1));
    }

    mLoading = false;
    return result;
}

void GameObserver::logAction(Player* player, const string& s) {
    if(player == players[0])
        if(s != "")
            logAction("p1." + s);
        else
            logAction("p1");
    else
        if(s != "")
            logAction("p2." + s);
        else
            logAction("p2");
}

void GameObserver::logAction(MTGCardInstance* card, MTGGameZone* zone, size_t index, int result) {
    stringstream stream;
    if(zone == NULL) zone = card->currentZone;
    stream << "p" << ((card->controller()==players[0])?"1.":"2.")
           << zone->getName()<< "[" << index << "] "
           << result << card->getLCName();
    logAction(stream.str());
}

void GameObserver::logAction(const string& s)
{
    if(mLoading)
    {
        string toCheck = *loadingite;
        dumpAssert(toCheck == s);
    }
    actionsList.push_back(s);
};

bool GameObserver::undo()
{
    stringstream stream;
    stream << *this;
    DebugTrace(stream.str());
    return load(stream.str(), true);
}

void GameObserver::Mulligan(Player* player)
{
    if(!player) player = currentPlayer;
    logAction(player, "mulligan");
    player->takeMulligan();
}

void GameObserver::serumMulligan(Player* player)
{
    if(!player) player = currentPlayer;
    logAction(player, "mulligan serum powder");
    player->serumMulligan();
}

Player* GameObserver::createPlayer(const string& playerMode
                                #ifdef TESTSUITE
                                , TestSuiteGame* testgame
                                #endif //TESTSUITE
                                )
{
    Player::Mode aMode = (Player::Mode)atoi(playerMode.c_str());
    Player* pPlayer = 0;

    switch(aMode)
    {
    case Player::MODE_AI:
        AIPlayerFactory playerCreator;
        if(players.size())
            pPlayer = playerCreator.createAIPlayer(this, MTGCollection(), players[0]);
        else
            pPlayer = playerCreator.createAIPlayer(this, MTGCollection(), 0);
        break;
    case Player::MODE_HUMAN:
        pPlayer = new HumanPlayer(this, "", "");
        break;
    case Player::MODE_TEST_SUITE:
#ifdef TESTSUITE
        if(players.size())
            pPlayer = new TestSuiteAI(testgame, 1);
        else
            pPlayer = new TestSuiteAI(testgame, 0);
#endif //TESTSUITE
        break;
    }

    if(pPlayer)
    {
        players.push_back(pPlayer);
    }

    return pPlayer;
}

#ifdef TESTSUITE
void GameObserver::loadTestSuitePlayer(int playerId, TestSuiteGame* testSuite)
{
    loadPlayer(playerId, new TestSuiteAI(testSuite, playerId));
}
#endif //TESTSUITE

void GameObserver::loadPlayer(int playerId, Player* player)
{
    //Because we're using a vector instead of an array (why?),
    // we have to prepare the vector in order to be the right size to accomodate the playerId variable
    // see http://code.google.com/p/wagic/issues/detail?id=772
    if (players.size() > (size_t) playerId) {
        SAFE_DELETE(players[playerId]);
        players[playerId] = NULL;
    } else {
        while (players.size() <= (size_t) playerId)
        {
            players.push_back(NULL);
        }
    }

    players[playerId] = player;
}

void GameObserver::loadPlayer(int playerId, PlayerType playerType, int decknb, bool premadeDeck)
{
    if (decknb)
    {
        if (playerType == PLAYER_TYPE_HUMAN)
        { //Human Player
            if(playerId == 0)
            {
                char deckFile[255];
                if (premadeDeck)
                    sprintf(deckFile, "player/premade/deck%i.txt", decknb);
                else
                    sprintf(deckFile, "%s/deck%i.txt", options.profileFile().c_str(), decknb);
                char deckFileSmall[255];
                sprintf(deckFileSmall, "player_deck%i", decknb);

                loadPlayer(playerId, NEW HumanPlayer(this, deckFile, deckFileSmall, premadeDeck));
            }
        }
        else
        { //AI Player, chooses deck
            AIPlayerFactory playerCreator;
            Player * opponent = NULL;
            if (playerId == 1) opponent = players[0];

            loadPlayer(playerId, playerCreator.createAIPlayer(this, MTGCollection(), opponent, decknb));
        }
    }
    else
    {
        //Random deck
        AIPlayerFactory playerCreator;
        Player * opponent = NULL;

        // Reset the random logging.
        randomGenerator.loadRandValues("");

        if (playerId == 1) opponent = players[0];
#ifdef AI_CHANGE_TESTING
        if (playerType == PLAYER_TYPE_CPU_TEST)
            loadPlayer(playerId, playerCreator.createAIPlayerTest(this, MTGCollection(), opponent, playerId == 0 ? "ai/bakaA/" : "ai/bakaB/"));
        else
#endif
        {
            loadPlayer(playerId, playerCreator.createAIPlayer(this, MTGCollection(), opponent));
        }

        if (playerType == PLAYER_TYPE_CPU_TEST)
            ((AIPlayer *) players[playerId])->setFastTimerMode();
    }
}

#ifdef NETWORK_SUPPORT
NetworkGameObserver::NetworkGameObserver(JNetwork* pNetwork, WResourceManager* output, JGE* input)
    : GameObserver(output, input), mpNetworkSession(pNetwork),     mSynchronized(false)
{
    mpNetworkSession->registerCommand("loadPlayer", this, loadPlayer, ignoreResponse);
    mpNetworkSession->registerCommand("synchronize", this, synchronize, checkSynchro);
    mpNetworkSession->registerCommand("sendAction", this, sendAction, checkSynchro);
    mpNetworkSession->registerCommand("disconnect", this, disconnect, ignoreResponse);
}

NetworkGameObserver::~NetworkGameObserver()
{
    mpNetworkSession->sendCommand("disconnect", "");
}

void NetworkGameObserver::disconnect(void*pxThis, stringstream&, stringstream&)
{
    NetworkGameObserver* pThis = (NetworkGameObserver*)pxThis;
    pThis->setLoser(pThis->getView()->getRenderedPlayerOpponent());
}

void NetworkGameObserver::Update(float dt)
{
    mpNetworkSession->Update();
    ::GameObserver::Update(dt);
}

void NetworkGameObserver::loadPlayer(int playerId, Player* player)
{
    GameObserver::loadPlayer(playerId, player);
    stringstream out;
    out << *player;
    mpNetworkSession->sendCommand("loadPlayer", out.str());
}

void NetworkGameObserver::loadPlayer(void*pxThis, stringstream& in, stringstream&)
{
    NetworkGameObserver* pThis = (NetworkGameObserver*)pxThis;
    Player* pPlayer = 0;
    string s;

    while(std::getline(in, s))
    {
        if (s.find("mode=") == 0)
        {
            pPlayer = pThis->createPlayer(s.substr(5)
    #ifdef TESTSUITE
                            , 0
    #endif //TESTSUITE
                            );
        }

        if(pPlayer && (!pPlayer->parseLine(s)))
        {
            break;
        }
    }
}

void NetworkGameObserver::synchronize()
{
    if(!mSynchronized && mpNetworkSession->isServer())
    {
        stringstream out;
        out << *this;
        mpNetworkSession->sendCommand("synchronize", out.str());
        mSynchronized = true;
    }
}

void NetworkGameObserver::synchronize(void*pxThis, stringstream& in, stringstream& out)
{
    NetworkGameObserver* pThis = (NetworkGameObserver*)pxThis;
    // now, we need to load the game from player 2's perspective
    pThis->load(in.str(), false, 1);
    out << *pThis;
}


void NetworkGameObserver::checkSynchro(void*pxThis, stringstream& in, stringstream&)
{
    NetworkGameObserver* pThis = (NetworkGameObserver*)pxThis;
    
    GameObserver aGame;
    aGame.mRules = pThis->mRules;
    aGame.load(in.str());

    assert(aGame == *pThis);
}

void NetworkGameObserver::sendAction(void*pxThis, stringstream& in, stringstream&)
{
    NetworkGameObserver* pThis = (NetworkGameObserver*)pxThis;

    pThis->mForwardAction = false;
    pThis->processAction(in.str());
    pThis->mForwardAction = true;
    //out << *pThis;
}

void NetworkGameObserver::logAction(const string& s)
{
    GameObserver::logAction(s);
    if(mForwardAction)
        mpNetworkSession->sendCommand("sendAction", s);
}

#endif
