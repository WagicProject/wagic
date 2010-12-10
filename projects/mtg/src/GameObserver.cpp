#include "PrecompiledHeader.h"

#include "GameObserver.h"
#include "GameOptions.h"
#include "CardGui.h"
#include "Damage.h"
#include "Rules.h"
#include "ExtraCost.h"
#include <JLogger.h>
#include <JRenderer.h>

GameObserver * GameObserver::mInstance = NULL;

GameObserver* GameObserver::GetInstance()
{

    return mInstance;
}

void GameObserver::EndInstance()
{

    SAFE_DELETE(mInstance);
}

void GameObserver::Init(Player * _players[], int _nbplayers)
{
    mInstance = NEW GameObserver(_players, _nbplayers);
}

GameObserver::GameObserver(Player * _players[], int _nb_players)
{
    for (int i = 0; i < _nb_players; i++)
    {
        players[i] = _players[i];
    }
    currentPlayer = NULL;
    currentActionPlayer = NULL;
    isInterrupting = NULL;
    currentPlayerId = 0;
    nbPlayers = _nb_players;
    currentGamePhase = -1;
    targetChooser = NULL;
    cardWaitingForTargets = NULL;
    mExtraPayment = NULL;
    gameOver = NULL;
    phaseRing = NULL;
    replacementEffects = NEW ReplacementEffects();
    combatStep = BLOCKERS;
    mRules = NULL;
}

int GameObserver::getCurrentGamePhase()
{
    return currentGamePhase;
}

Player * GameObserver::opponent()
{
    int index = (currentPlayerId + 1) % nbPlayers;
    return players[index];
}

void GameObserver::nextPlayer()
{
    turn++;
    currentPlayerId = (currentPlayerId + 1) % nbPlayers;
    currentPlayer = players[currentPlayerId];
    currentActionPlayer = currentPlayer;
    combatStep = BLOCKERS;
}
void GameObserver::nextGamePhase()
{
    Phase * cPhaseOld = phaseRing->getCurrentPhase();
    if (cPhaseOld->id == Constants::MTG_PHASE_COMBATDAMAGE)
        if (FIRST_STRIKE == combatStep || END_FIRST_STRIKE == combatStep || DAMAGE == combatStep)
        {
            nextCombatStep();
            return;
        }

    if (cPhaseOld->id == Constants::MTG_PHASE_COMBATBLOCKERS)
        if (BLOCKERS == combatStep || TRIGGERS == combatStep)
        {
            nextCombatStep();
            return;
        }

    phaseRing->forward();

    //Go directly to end of combat if no attackers
    if (cPhaseOld->id == Constants::MTG_PHASE_COMBATATTACKERS && !(currentPlayer->game->inPlay->getNextAttacker(NULL)))
    {
        phaseRing->forward();
        phaseRing->forward();
    }

    Phase * cPhase = phaseRing->getCurrentPhase();
    currentGamePhase = cPhase->id;

    if (Constants::MTG_PHASE_COMBATDAMAGE == currentGamePhase)
        nextCombatStep();

    if (currentPlayer != cPhase->player)
        nextPlayer();

    //init begin of turn
    if (currentGamePhase == Constants::MTG_PHASE_BEFORE_BEGIN)
    {
        cleanupPhase();
        currentPlayer->canPutLandsIntoPlay = 1;
        currentPlayer->castedspellsthisturn = 0;
        currentPlayer->opponent()->castedspellsthisturn = 0;
        currentPlayer->castcount = 0;
        currentPlayer->nocreatureinstant = 0;
        currentPlayer->nospellinstant = 0;
        currentPlayer->onlyoneinstant = 0;
        currentPlayer->damageCount = 0;
        currentPlayer->preventable = 0;
        mLayers->actionLayer()->cleanGarbage(); //clean abilities history for this turn;
        mLayers->stackLayer()->garbageCollect(); //clean stack history for this turn;
        mLayers->actionLayer()->Update(0);
        for (int i = 0; i < 2; i++)
        {
            delete (players[i]->game->garbage);
            players[i]->game->garbage = NEW MTGGameZone();
            players[i]->game->garbage->setOwner(players[i]);
        }
        combatStep = BLOCKERS;
        return nextGamePhase();
    }

    for (int i = 0; i < 2; ++i)
        players[i]->getManaPool()->init();

    if (currentGamePhase == Constants::MTG_PHASE_AFTER_EOT)
    {
        //Auto Hand cleaning, in case the player didn't do it himself
        while (currentPlayer->game->hand->nb_cards > 7 && currentPlayer->nomaxhandsize < 1)
            currentPlayer->game->putInGraveyard(currentPlayer->game->hand->cards[0]);
        mLayers->actionLayer()->Update(0);
        return nextGamePhase();
    }

    //Phase Specific actions
    switch (currentGamePhase)
    {
    case Constants::MTG_PHASE_UNTAP:
		DebugTrace("Untap Phase -------------   Turn " << turn );
        untapPhase();
        break;
    case Constants::MTG_PHASE_DRAW:
        //mLayers->stackLayer()->addDraw(currentPlayer,1);
        break;
    case Constants::MTG_PHASE_COMBATBLOCKERS:
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

void GameObserver::userRequestNextGamePhase()
{
	if (mLayers->stackLayer()->getNext(NULL, 0, NOT_RESOLVED))
		return;
	if (getCurrentTargetChooser())
		return;
	//if (mLayers->actionLayer()->isWaitingForAnswer())
	//	return;
    // Wil 12/5/10: additional check, not quite understanding why TargetChooser doesn't seem active at this point.
    // If we deem that an extra cost payment needs to be made, don't allow the next game phase to proceed.
    // Here's what I find weird - if the extra cost is something like a sacrifice, doesn't that imply a TargetChooser?
  if (WaitForExtraPayment(NULL))
        return;

	bool executeNextPhaseImmediately = true;

	Phase * cPhaseOld = phaseRing->getCurrentPhase();
	if ((cPhaseOld->id == Constants::MTG_PHASE_COMBATBLOCKERS && combatStep == ORDER) || (cPhaseOld->id
		== Constants::MTG_PHASE_COMBATBLOCKERS && combatStep == TRIGGERS) || cPhaseOld->id
		== Constants::MTG_PHASE_COMBATDAMAGE || opponent()->isAI()
		|| options[Options::optionInterrupt(currentGamePhase)].number)
	{
		executeNextPhaseImmediately = false;
	}

	if (executeNextPhaseImmediately)
	{
		nextGamePhase();
	}
	else
	{
		mLayers->stackLayer()->AddNextGamePhase();
	}

}

int GameObserver::forceShuffleLibraries()
{
    int result = 0;
    if (players[0]->game->library->needShuffle)
    {
        players[0]->game->library->shuffle();
        players[0]->game->library->needShuffle = false;
        ++result;
    }
    if (players[1]->game->library->needShuffle)
    {
        players[1]->game->library->shuffle();
        players[1]->game->library->needShuffle = false;
        ++result;
    }
    return result;
}

void GameObserver::startGame(Rules * rules)
{
    turn = 0;
    mRules = rules;
    if (rules)
        rules->initPlayers();

    options.automaticStyle(players[0], players[1]);

    mLayers = NEW DuelLayers();
    mLayers->init();

    currentPlayerId = 0;
    currentPlayer = players[0];
    currentActionPlayer = currentPlayer;
    phaseRing = NEW PhaseRing(players, nbPlayers);
    if (rules)
        rules->initGame();

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
                    if (_card->hasType("land"))
                    {
                        card = _card;
                        j = z->nb_cards;
                    }
                }
                if (card)
                {
                    MTGCardInstance * copy = p->game->putInZone(card, p->game->library, p->game->stack);
                    Spell * spell = NEW Spell(copy);
                    spell->resolve();
                    delete spell;
                }
            }
        }
    }
}

void GameObserver::addObserver(MTGAbility * observer)
{
    mLayers->actionLayer()->Add(observer);
}

void GameObserver::removeObserver(ActionElement * observer)
{
    if (observer)
        mLayers->actionLayer()->moveToGarbage(observer);

    else
    {
    } //TODO log error
}

GameObserver::~GameObserver()
{
    LOG("==Destroying GameObserver==");
    SAFE_DELETE(targetChooser);
    SAFE_DELETE(mLayers);
    SAFE_DELETE(phaseRing);
    SAFE_DELETE(replacementEffects);
    for (int i = 0; i < nbPlayers; ++i)
    {
        SAFE_DELETE(players[i]);
    }
    LOG("==GameObserver Destroyed==");
}

void GameObserver::Update(float dt)
{
    Player * player = currentPlayer;

    if (Constants::MTG_PHASE_COMBATBLOCKERS == currentGamePhase && BLOCKERS == combatStep)
        player = player->opponent();

    currentActionPlayer = player;
    if (isInterrupting)
        player = isInterrupting;
    mLayers->Update(dt, player);

    while (mLayers->actionLayer()->stuffHappened)
    {
        mLayers->actionLayer()->Update(0);
    }

    stateEffects();
    oldGamePhase = currentGamePhase;

  

}

//applies damage to creatures after updates
//Players life test
void GameObserver::stateEffects()
{
    if (mLayers->stackLayer()->count(0, NOT_RESOLVED) != 0)
        return;
    if (mLayers->actionLayer()->menuObject)
        return;
    if (targetChooser || mLayers->actionLayer()->isWaitingForAnswer())
        return;
    for (int i = 0; i < 2; i++)
    {
        MTGGameZone * zone = players[i]->game->inPlay;
        for (int j = zone->nb_cards - 1; j >= 0; j--)
        {
            MTGCardInstance * card = zone->cards[j];
            card->afterDamage();

            //Remove auras that don't have a valid target anymore
            if (card->target && !isInPlay(card->target) && !card->hasType("equipment"))
            {
                players[i]->game->putInGraveyard(card);
            }
        }
    }
    for (int i = 0; i < 2; i++)
        if (players[i]->life <= 0)
        {
            int cantlosers = 0;
            MTGGameZone * z = players[i]->game->inPlay;
            int nbcards = z->nb_cards;
            for (int j = 0; j < nbcards; ++j)
            {
                MTGCardInstance * c = z->cards[j];
                if (c->has(Constants::CANTLOSE) || c->has(Constants::CANTLIFELOSE))
                {
                    cantlosers++;
                }
            }
            MTGGameZone * k = players[i]->opponent()->game->inPlay;
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
                gameOver = players[i];
            }
        }

    for (int i = 0; i < 2; i++)
        if (players[i]->poisonCount >= 10)
            gameOver = players[i];

	if (combatStep == TRIGGERS)
		{
      if (!mLayers->stackLayer()->getNext(NULL, 0, NOT_RESOLVED) && !targetChooser && !mLayers->actionLayer()->isWaitingForAnswer())
				mLayers->stackLayer()->AddNextCombatStep();
		}

	  //Auto skip Phases
	GameObserver * game = game->GetInstance();
	int skipLevel = (game->currentPlayer->playMode == Player::MODE_TEST_SUITE) ? Constants::ASKIP_NONE : options[Options::ASPHASES].number;
    int nrCreatures = currentPlayer->game->inPlay->countByType("Creature");

    if (skipLevel == Constants::ASKIP_SAFE || skipLevel == Constants::ASKIP_FULL)
    {
        if ((opponent()->isAI() && !(isInterrupting)) && (currentGamePhase == Constants::MTG_PHASE_UNTAP || currentGamePhase
                        == Constants::MTG_PHASE_DRAW || currentGamePhase == Constants::MTG_PHASE_COMBATBEGIN || ((currentGamePhase
                        == Constants::MTG_PHASE_COMBATATTACKERS) && (nrCreatures == 0)) || currentGamePhase
                        == Constants::MTG_PHASE_COMBATEND || currentGamePhase == Constants::MTG_PHASE_ENDOFTURN
                        || ((currentGamePhase == Constants::MTG_PHASE_CLEANUP) && (currentPlayer->game->hand->nb_cards < 8))))
				    userRequestNextGamePhase();
    }
    if (skipLevel == Constants::ASKIP_FULL)
    {
  if ((opponent()->isAI() && !(isInterrupting)) && (currentGamePhase == Constants::MTG_PHASE_UPKEEP || currentGamePhase
                        == Constants::MTG_PHASE_COMBATDAMAGE))
            userRequestNextGamePhase();
		}

}

void GameObserver::Render()
{
    mLayers->Render();
    if (targetChooser || mLayers->actionLayer()->isWaitingForAnswer())
        JRenderer::GetInstance()->DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(255,255,0,0));
    if (mExtraPayment)
        mExtraPayment->Render();
    for (int i = 0; i < nbPlayers; ++i)
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
}

void GameObserver::stackObjectClicked(Interruptible * action)
{
    if (targetChooser != NULL)
    {
        int result = targetChooser->toggleTarget(action);
        if (result == TARGET_OK_FULL)
        {
            cardClick(cardWaitingForTargets);
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

int GameObserver::cardClick(MTGCardInstance * card, Targetable * object)
{
    Player * clickedPlayer = NULL;
    if (!card)
        clickedPlayer = ((Player *) object);
    if (targetChooser)
    {
        int result;
        if (card)
        {
            if (card == cardWaitingForTargets)
            {
                int _result = targetChooser->ForceTargetListReady();
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
            }
        }
        else
        {
            result = targetChooser->toggleTarget(clickedPlayer);
        }
        if (result == TARGET_OK_FULL)
            card = cardWaitingForTargets;
        else
            return 1;
    }

    if (WaitForExtraPayment(card))
        return 1;

	int reaction = 0;

    if (ORDER == combatStep)
    {
        card->defenser->raiseBlockerRankOrder(card);
        return 1;
    }

    if (card)
    {
        //card played as normal, alternative cost, buyback, flashback, retrace.

        //the varible "paymenttype = int" only serves one purpose, to tell this bug fix what menu item you clicked on...
        // all alternative cost or play methods suffered from the fix because if the card contained "target=" 
        // it would automatically force the play method to putinplayrule...even charge you the original mana cost.

        /* Fix for Issue http://code.google.com/p/wagic/issues/detail?id=270
         put into play is hopefully the only ability causing that kind of trouble
         If the same kind of issue occurs with other abilities, let's think of a cleaner solution
         */
        if (targetChooser)
        {
            MTGAbility * a = mLayers->actionLayer()->getAbility(card->paymenttype);
            return a->reactToClick(card);
        }

        reaction = mLayers->actionLayer()->isReactingToClick(card);
        if (reaction == -1)
            return mLayers->actionLayer()->reactToClick(card);
    }
    else
    {//this handles abilities on a menu...not just when card is being played
        reaction = mLayers->actionLayer()->isReactingToTargetClick(object);
        if (reaction == -1)
            return mLayers->actionLayer()->reactToTargetClick(object);
    }

    if (!card)
        return 0;

    //Current player's hand
    if (currentPlayer->game->hand->hasCard(card) && currentGamePhase == Constants::MTG_PHASE_CLEANUP
                    && currentPlayer->game->hand->nb_cards > 7 && currentPlayer->nomaxhandsize < 1)
    {
        currentPlayer->game->putInGraveyard(card);
    }
    else if (reaction)
    {
        if (reaction == 1)
        {
            return mLayers->actionLayer()->reactToClick(card);
        }
        else
        {
            mLayers->actionLayer()->setMenuObject(object);
            return 1;
        }
    }
    else if (card->isTapped() && card->controller() == currentPlayer)
    {
        return untap(card);
    }

    return 0;

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
    TargetChooser * _tc = mLayers->actionLayer()->getCurrentTargetChooser();
    if (_tc)
        return _tc;
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

void GameObserver::draw()
{
    currentPlayer->game->drawFromLibrary();
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
        TargetChooserFactory tcf;
        targetChooser = tcf.createTargetChooser(card);
        cardWaitingForTargets = card;
        if (targetChooser == NULL)
        {
            return 1;
        }
    }
    return (targetChooser->targetListSet());
}
