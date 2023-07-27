#include "PrecompiledHeader.h"
#include "AllAbilities.h"
#include "Translate.h"
#include "MTGRules.h"

//display a text animation, this is not a real ability.
MTGEventText::MTGEventText(GameObserver* observer, int _id, MTGCardInstance * card, string textToShow) :
MTGAbility(observer, _id,card)
{
    textAlpha = 255;
    text = textToShow;
}

void MTGEventText::Update(float dt)
{
    if (textAlpha)
    {
        textAlpha -= static_cast<int> (200 * dt);
        Render();
        if (textAlpha < 0)
        {
            textAlpha = 0;
            this->forceDestroy = 1;
        }
    }
    MTGAbility::Update(dt);
}

void MTGEventText::Render()
{
    if (!textAlpha)
        return;
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
    float backup = mFont->GetScale();
    mFont->SetScale(2 - (float) textAlpha / 130);
    mFont->SetColor(ARGB(255,255,255,255));
    mFont->DrawString(text.c_str(), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, JGETEXT_CENTER);
    mFont->SetScale(backup);
}

MTGEventText * MTGEventText::clone() const
{
    return NEW MTGEventText(*this);
}

//generic activated ability for wrapping reveals.
GenericRevealAbility::GenericRevealAbility(GameObserver* observer, int id, MTGCardInstance * source,
    Targetable * target, string _howMany) :
    ActivatedAbility(observer, id, source, NULL), howMany(_howMany)
{
    this->GetId();
    named = "";
}

int GenericRevealAbility::resolve()
{
    if(source->lastController->isAI() && source->getAICustomCode().size())
    {
        string abi = source->getAICustomCode();
        std::transform(abi.begin(), abi.end(), abi.begin(), ::tolower);//fix crash
        AbilityFactory af(game);
        MTGAbility * a3 = af.parseMagicLine(abi, this->GetId(), NULL, source);
        a3->oneShot = 1;
        a3->canBeInterrupted = false;
        a3->resolve();
        SAFE_DELETE(a3);
        return 1;
    }
    MTGAbility * ability = NEW MTGRevealingCards(game, this->GetId(), source, howMany);
    ability->addToGame();
    return 1;
}

const string GenericRevealAbility::getMenuText()
{
    if(named.size())
        return named.c_str();
    return "Reveal Cards";
}

GenericRevealAbility * GenericRevealAbility::clone() const
{
    GenericRevealAbility * a = NEW GenericRevealAbility(*this);
    return a;
}

GenericRevealAbility::~GenericRevealAbility()
{
    //SAFE_DELETE(ability);
}

//carddisplay created for use in abilities.
RevealDisplay::RevealDisplay(int id, GameObserver* game, int x, int y, JGuiListener * listener, TargetChooser * tc,
    int nb_displayed_items) :
    CardDisplay(id, game, x, y, listener, tc, nb_displayed_items)
{
}

void RevealDisplay::AddCard(MTGCardInstance * _card)
{
    CardGui * card = NEW CardView(CardView::nullZone, _card, static_cast<float> (x + 20 + (mObjects.size() - start_item) * 30),
        static_cast<float> (y + 25));
    Add(card);
}

bool RevealDisplay::CheckUserInput(JButton key)
{
    if (JGE_BTN_SEC == key || JGE_BTN_PRI == key || JGE_BTN_UP == key || JGE_BTN_DOWN == key)
        return false;

    return CardDisplay::CheckUserInput(key);
}

//display card selector box of specified zone.
MTGRevealingCards::MTGRevealingCards(GameObserver* observer, int _id, MTGCardInstance * card, string coreAbility) :
    MTGAbility(observer, _id, card), CardDisplay(_id, game, x, y, listener, NULL, nb_displayed_items)

{
    abilityToCast = NULL;
    revealDisplay = NULL;
    abilityFirst = NULL;
    abilitySecond = NULL;
    abilityString = coreAbility;
    initCD = false;

    afterReveal = "";
    afterEffectActivated = false;

    repeat = false;
    playerForZone = NULL;
    revealCertainTypes = "";
    revealUntil = "";

    if (card->playerTarget)
        playerForZone = card->playerTarget;
    else
        playerForZone = source->controller();

    RevealZone = playerForZone->game->reveal;
    zone = RevealZone;
    RevealFromZone = playerForZone->game->library;

    vector<string>amount = parseBetween(coreAbility, "", " ");
    if (amount.size())
    {
        number = amount[1];
    }

    vector<string>differentZone = parseBetween(coreAbility, "revealzone(", ")");
    if (differentZone.size())
    {
        RevealFromZone = MTGGameZone::stringToZone(game,differentZone[1],source,NULL);
    }

    vector<string>certainTypes = parseBetween(coreAbility, "revealtype(", ")");
    if (certainTypes.size())
    {
        revealCertainTypes = certainTypes[1];
    }

    vector<string>RevealCardUntil = parseBetween(coreAbility, "revealuntil(", ")");
    if (RevealCardUntil.size())
    {
        revealUntil = RevealCardUntil[1];
    }

    vector<string>first = parseBetween(coreAbility, "optionone ", " optiononeend");
    if (first.size())
    {
    abilityOne = first[1];
    }
    vector<string>second = parseBetween(coreAbility, "optiontwo ", " optiontwoend");
    if (second.size())
    {
    abilityTwo = second[1];
    }
    vector<string>afterEffect = parseBetween(coreAbility, "afterrevealed ", " afterrevealedend");
    if (afterEffect.size())
    {
        afterReveal = afterEffect[1];
    }

    repeat = coreAbility.find("repeat") != string::npos;

}

void MTGRevealingCards::Update(float dt)
{

    if (game->OpenedDisplay  != this->revealDisplay && !initCD)//wait your turn
    {
        //if any carddisplays are open, dont do anything until theyre closed, then wait your turn if multiple reveals trigger.
        return;
    }
    if (game->mLayers->actionLayer()->menuObject || game->LPWeffect)
        return;//dont do any of this if a menuobject exist.
    if (!source->getObserver()->mLayers->actionLayer()->getCurrentTargetChooser() && !revealDisplay && !initCD)
    {

        WParsedInt nbCardP(number, NULL, source);
        nbCard = nbCardP.getValue();
        int adjust = 0;
        switch (nbCard)
        {
            //adjust length and location of carddisplay box.
        case 1:adjust = 120; break;
        case 2:adjust = 145; break;
        case 3:adjust = 175; break;
        case 4:adjust = 200; break;
        case 5:adjust = 225; break;
        default:adjust = 225; break;
        }
        if (revealUntil.size())
        {
            adjust = 225;
            revealDisplay = NEW RevealDisplay(1, game, SCREEN_WIDTH - adjust, SCREEN_HEIGHT, listener, NULL,5);
        }
        else
        revealDisplay = NEW RevealDisplay(1, game, SCREEN_WIDTH - adjust, SCREEN_HEIGHT, listener, NULL, nbCard > 5 ? 5 : nbCard);
        revealDisplay->zone = RevealFromZone;
        trashDisplays.push_back(revealDisplay);

        if (revealCertainTypes.size())//revealing cards of a TARGETCHOOSER type.
        {
            TargetChooserFactory tcf(game);
            TargetChooser * rTc = tcf.createTargetChooser(revealCertainTypes, source);
            int startingNumber = RevealFromZone->nb_cards - 1;
            if (rTc)
                for (int i = startingNumber; i > -1; i--)
                {
                    if (!RevealFromZone->cards.size())
                        break;
                    MTGCardInstance * toMove = RevealFromZone->cards[i];
                    if (toMove)
                    {
                        if (rTc->canTarget(toMove, true))
                        {
                            CardViewBackup(toMove);
                            playerForZone->game->putInZone(toMove, RevealFromZone, RevealZone);
                            source->revealedLast = toMove;
                        }
                    }

                }
            SAFE_DELETE(rTc);
        }
        else if(revealUntil.size())//reveal cards until you reveal a TARGETCHOOSER.
        {
            TargetChooserFactory tcf(game);
            TargetChooser * rUc = tcf.createTargetChooser(revealUntil, source);
            bool foundCard = false;
            int howMany = nbCard;
            int startingNumber = RevealFromZone->nb_cards;
            for (int i = 0; i < startingNumber; i++)
            {
                if (foundCard && howMany == 0)
                    break;
                if (howMany == 0)
                    break; //not allowed to reveal until 0 of something is revealed.
                if (RevealFromZone->nb_cards - 1 < 0)
                    break;
                MTGCardInstance * toMove = RevealFromZone->cards[RevealFromZone->nb_cards - 1];
                if (toMove)
                {
                    if (rUc->canTarget(toMove, true))
                    {
                        foundCard = true;
                        howMany--;
                    }

                    CardViewBackup(toMove);
                    playerForZone->game->putInZone(toMove, RevealFromZone, RevealZone);
                    source->revealedLast = toMove;
                }

            }
            SAFE_DELETE(rUc);
        }
        else
        {
            for (int i = 0; i < nbCard; i++)//normal reveal
            {
                if (RevealFromZone->nb_cards - 1 < 0)
                    break;
                MTGCardInstance * toMove = RevealFromZone->cards[RevealFromZone->nb_cards - 1];
                if (toMove)
                {
                    CardViewBackup(toMove);
                    playerForZone->game->putInZone(toMove, RevealFromZone, RevealZone);
                    source->revealedLast = toMove;
                }

            }

         }

        //build the zone, create the first ability.
        revealDisplay->init(RevealZone);
        revealDisplay->zone = RevealZone;
        game->OpenedDisplay = revealDisplay;
        toResolve();    
        initCD = true;
    }


    //card display is ready and loaded, abilities have fired at this point.
    //critical for testdestroy, a function that determines if a ability can
    //exist in condiations such as source not being in play.
    
    if (!zone->cards.size())
    {
        //all possible actions are done, the zone is empty, lets NULL it so it clears it off the screen.
        //DO NOT SAFE_DELETE here, it destroys the card->view and backups kept for the second ability.
        revealDisplay = NULL;
        game->OpenedDisplay = revealDisplay;

        if (repeat)
        {
            initCD = false;
        }
        else if (afterReveal.size() && !afterEffectActivated)
        { 
            afterEffectActivated = true;
            abilityAfter = contructAbility(afterReveal);
            game->addObserver(abilityAfter);
        }
        else
            this->removeFromGame();
    }

    if (revealDisplay)
    {
        revealDisplay->Update(dt);
        Render();
    }

    MTGAbility::Update(dt);
}

void MTGRevealingCards::CardViewBackup(MTGCardInstance * backup)
{
    CardView* t;

    t = NEW CardView(CardView::nullZone, backup, 0, 0);
    //we store copies of the card view since the safe_delete of card displays also deletes the guis stored in them.
    t->actX = SCREEN_WIDTH;
    t->actY = SCREEN_HEIGHT * -2;
    //correct cards x and y, last known location was the reveal display.
    cards.push_back(t);
    return;
}

int MTGRevealingCards::testDestroy()
{
    if (game->mExtraPayment)
        return 0;
    if (revealDisplay)
        return 0;
    if (zone->cards.size())
        return 0;
    if (!initCD)
        return 0;
    if (game->mLayers->actionLayer()->menuObject)
        return 0;
    if (game->mLayers->actionLayer()->getIndexOf(abilityFirst) != -1)
        return 0;

    return 1;
}

int MTGRevealingCards::toResolve()
{

    TargetChooserFactory tcf(game);
    vector<string>splitTarget = parseBetween(abilityOne, "target(", ")");
    //we build a tc to check if the first ability has any valid targets, if it doesnt, just add the 2nd one.
    if (splitTarget.size()) 
    {
        TargetChooser * rTc = tcf.createTargetChooser(splitTarget[1].c_str(), source);

        if (rTc && rTc->countValidTargets())
        {
            abilityFirst = contructAbility(abilityOne);
            game->addObserver(abilityFirst);
            
        }
        else
        {
            repeat = false;
            abilitySecond = contructAbility(abilityTwo);
            game->addObserver(abilitySecond);
            
        }
        SAFE_DELETE(rTc);
    }
    else//the first ability is not targeted
    {
        abilityFirst = contructAbility(abilityOne);
        game->addObserver(abilityFirst);
    }
    return 1;
}

MTGAbility * MTGRevealingCards::contructAbility(string abilityToMake)
{
    AbilityFactory af(game);
    abilityToCast = af.parseMagicLine(abilityToMake, getMaxId(), NULL, source, false);
    if (!abilityToCast)
        return NULL;
    abilityToCast->canBeInterrupted = false;
    abilityToCast->forceDestroy = 1;
    return abilityToCast;
}

void MTGRevealingCards::Render()
{
    if (!revealDisplay)
        return;
    CheckUserInput(mEngine->ReadButton());
    revealDisplay->CheckUserInput(mEngine->ReadButton());
    revealDisplay->Render();
    return;
}

bool MTGRevealingCards::CheckUserInput(JButton key)
{
    //DO NOT REFACTOR BELOW, IT KEPT SPLIT UP TO MAINTAIN READABILITY.
    //we override check inputs, we MUST complete reveal and its effects before being allowed to do anything else.
    TargetChooser * tc = this->observer->mLayers->actionLayer()->getCurrentTargetChooser();
    if (this->source->controller()->isAI())
    {
        if (this->source->controller() != game->isInterrupting)
            game->mLayers->stackLayer()->cancelInterruptOffer(ActionStack::DONT_INTERRUPT, false);
        if (key == 0)
            key = JGE_BTN_NEXT;
        if (key != JGE_BTN_OK && key != JGE_BTN_NEXT)
            key = JGE_BTN_OK;
    }
    if (JGE_BTN_SEC == key || JGE_BTN_PREV == key || JGE_BTN_NEXT == key || JGE_BTN_MENU == key)//android back button
    {
        if (tc && (tc->targetMin == false || tc->maxtargets == TargetChooser::UNLITMITED_TARGETS))
        {
            tc->done = true;
            tc->autoChoice = false;
            tc->forceTargetListReadyByPlayer = 1;
            //this is for when we have <upto:x> targets but only want to move Y targets, it allows us to
            //tell the targetchooser we are done.
            if (!abilitySecond && !tc->getNbTargets() && tc->source)
            {//we selected nothing for the first ability.
                tc->source->getObserver()->cardClick(tc->source, 0, false);
                //remove the first ability to avoid a menu react.
                source->getObserver()->mLayers->stackLayer()->Remove(abilityFirst);
                game->removeObserver(abilityFirst);
                
                if (!this->source->controller()->isAI())
                game->Update(0);
                
                if (zone->cards.size())//generally only want to add ability 2 if anything is left in the zone.
                {
                    repeat = false;
                    abilitySecond = contructAbility(abilityTwo);
                    game->addObserver(abilitySecond);
                }
            }
            else if (tc->source)
            {
                tc->source->getObserver()->cardClick(tc->source, 0, false);
            }
        }
        else if (!tc && !abilitySecond)//the actions of the first card have finished and we're done looking at the cards.
        {           //or the first ability was an "all(" which was not a mover ability.
            CheckUserInput(JGE_BTN_OK);
        }
        return false;
    }
    if (JGE_BTN_OK == key)//for ease if we're sitting there looking at the card display and click a card after first ability.
    {                     //looks redundent and can be added above as another condiational, however we would end up with a massive
                          //if statement that becomes very very hard to follow. 
        if (!tc && !abilitySecond)
        {         
            source->getObserver()->mLayers->stackLayer()->Remove(abilityFirst);
            game->removeObserver(abilityFirst);
            if (!this->source->controller()->isAI())
            game->Update(1);

            if (zone->cards.size())
            {
                repeat = false;
                abilitySecond = contructAbility(abilityTwo);
                game->addObserver(abilitySecond);
            }

        }
    }
    if(revealDisplay)
        return revealDisplay->CheckUserInput(key);
    return false;
}

MTGRevealingCards * MTGRevealingCards::clone() const
{
    return NEW MTGRevealingCards(*this);
}

MTGRevealingCards::~MTGRevealingCards()
{
    for (vector<CardDisplay*>::iterator it = trashDisplays.begin(); it != trashDisplays.end(); ++it)
        SAFE_DELETE(*it);
    for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
        SAFE_DELETE(*it);
}

int MTGRevealingCards::receiveEvent(WEvent* e)
{

    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    {
        if (event->from == zone)
        {
            CardView* t;
            if (event->card->view)
                t = NEW CardView(CardView::nullZone, event->card, *(event->card->view));
            else
                t = NEW CardView(CardView::nullZone, event->card, (float)x, (float)y);
            //we store copies of the card view since moving to and from card displays also deletes the guis stored in cards.
            //GuiLayer::resetObjects() is the main reason we need to back them up. card views are set to NULL maybe more often than
            //they should be, possibly someone being to over cautious.
            t->actX = SCREEN_WIDTH;
            t->actY = SCREEN_HEIGHT * -2;
            //correct cards x and y, last known location was the reveal display.
            cards.push_back(t);
            return 1;
        } 
    }
    return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
////scry//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//below the effect of "scry X, THEN reveal and do stuff, was impossible to accomplish with reveal alone.
//if a card simply states "scry X" and nothing else use reveal:x. 
//this is for effects that want you to reveal AFTER you scry.
//this ability automatically creates effects, put on top, then whatever you dont get put on buttom,
//then it reveals the top card, and creates the ability written in primitive as core, then it
//handles putting the card back on top of the library when you are done.
///delayed changes the order, makes the ability fire after the 2nd reveal is finished.
///
MTGScryCards::MTGScryCards(GameObserver* observer, int _id, MTGCardInstance * card, string coreAbility) :
    MTGAbility(observer, _id, card), CardDisplay(_id, game, x, y, listener, NULL, nb_displayed_items)

{
    abilityToCast = NULL;
    revealDisplay = NULL;
    abilityFirst = NULL;
    abilitySecond = NULL;
    abilityString = coreAbility;
    delayedAbilityString = "";
    revealTopAmount = 1;//scry, then reveal the top card and do effect.

    initCD = false;
    RevealZone = source->controller()->game->reveal;
    zone =RevealZone;
    RevealFromZone = source->controller()->game->library;

    vector<string>amount = parseBetween(coreAbility, "", " ");
    if (amount.size())
    {
        number = amount[1];
    }

    vector<string>differentZone = parseBetween(coreAbility, "scryzone(", ")");
    if (differentZone.size())
    {
        RevealFromZone = MTGGameZone::stringToZone(game, differentZone[1], source, NULL);
    }

    abilityOne = "name(Place on top) target(<anyamount>*|myreveal) moveto(mylibrary)";
    delayed = coreAbility.find("delayed") != string::npos;
    dontRevealAfter = coreAbility.find("dontshow") != string::npos;
    if(dontRevealAfter)
        revealTopAmount = 0;
    vector<string>second = parseBetween(coreAbility, "scrycore ", " scrycoreend");
    if (second.size())
    {
        if (delayed)
        {
            abilityTwo = "target(*|reveal) name(Reveal the top card) donothing";
            delayedAbilityString = second[1];
        }
        else
        abilityTwo = second[1];
    }

    
}

void MTGScryCards::Update(float dt)
{
    if (game->OpenedDisplay != this->revealDisplay && !initCD)
        return;
    if (game->mLayers->actionLayer()->menuObject)
        return;
    if (!source->getObserver()->mLayers->actionLayer()->getCurrentTargetChooser() && !revealDisplay && !initCD)
    {
        WParsedInt nbCardP(number, NULL, source);
        nbCard = nbCardP.getValue();
        initDisplay(nbCard);
        toResolve();
    }
    initCD = true;
    if (!zone->cards.size() && abilitySecond)
    {
        revealDisplay = NULL;
        game->OpenedDisplay = revealDisplay;
        this->removeFromGame();
    }
    if (revealDisplay)
    {
        revealDisplay->Update(dt);
        Render();
    }
    MTGAbility::Update(dt);
}

void MTGScryCards::initDisplay(int value)
{

    if (RevealZone->cards.size())
    {
        do
        {
            MTGCardInstance * toMove = RevealZone->cards[0];
            if (toMove)
            {   
                MTGAbility * a = NEW AALibraryBottom(game, getMaxId(), source, toMove);
                a->oneShot = 1;
                a->resolve();
                SAFE_DELETE(a);
            }
        } while (RevealZone->cards.size());  

        game->Update(0);
        revealDisplay = NULL;
        game->OpenedDisplay = revealDisplay;
    }
    int adjust = 0;
    switch (value)
    {
    case 1:adjust = 120; break;
    case 2:adjust = 145; break;
    case 3:adjust = 175; break;
    case 4:adjust = 200; break;
    case 5:adjust = 225; break;
    default:adjust = 225; break;
    }
    revealDisplay = NEW RevealDisplay(1, game, SCREEN_WIDTH - adjust, SCREEN_HEIGHT, listener, NULL, nbCard > 5 ? 5 : nbCard);
    revealDisplay->zone = RevealFromZone;
    trashDisplays.push_back(revealDisplay);
    for (int i = 0; i < value; i++)
    {
        if (RevealFromZone->nb_cards - 1 < 0)
            break;
        MTGCardInstance * toMove = RevealFromZone->cards[RevealFromZone->nb_cards - 1];
        if (toMove)
        {
            CardView* t;
            t = NEW CardView(CardView::nullZone, toMove, 0, 0);
            t->actX = SCREEN_WIDTH;
            t->actY = SCREEN_HEIGHT * -2;
            cards.push_back(t);
            source->controller()->game->putInZone(toMove, RevealFromZone, RevealZone);
            source->revealedLast = toMove;
        }
    }
    revealDisplay->init(RevealZone);
    revealDisplay->zone = RevealZone;
    game->OpenedDisplay = revealDisplay;
}

int MTGScryCards::testDestroy()
{
    if (game->mExtraPayment)
        return 0;
    if (revealDisplay)
        return 0;
    if (zone->cards.size())
        return 0;
    if (!initCD)
        return 0;
    if (game->mLayers->actionLayer()->menuObject)
        return 0;
    if (game->mLayers->actionLayer()->getIndexOf(abilityFirst) != -1)
        return 0;

    return 1;
}

int MTGScryCards::toResolve()
{
    //scry will always have valid targets.
        abilityFirst = contructAbility(abilityOne);
        game->addObserver(abilityFirst);
    return 1;
}

MTGAbility * MTGScryCards::contructAbility(string abilityToMake)
{
    AbilityFactory af(game);
    abilityToCast = af.parseMagicLine(abilityToMake, getMaxId(), NULL, source, false);
    if (!abilityToCast)
        return NULL;
    abilityToCast->canBeInterrupted = false;
    abilityToCast->forceDestroy = 1;
    return abilityToCast;
}

void MTGScryCards::Render()
{
    if (!revealDisplay)
        return;
    CheckUserInput(mEngine->ReadButton());
    if (revealDisplay)
    {
        revealDisplay->CheckUserInput(mEngine->ReadButton());
        revealDisplay->Render();
    }
    return;
}

bool MTGScryCards::CheckUserInput(JButton key)
{
    //DO NOT REFACTOR BELOW
    TargetChooser * tc = this->observer->mLayers->actionLayer()->getCurrentTargetChooser();
    if (this->source->controller()->isAI())
    {//ai doesnt click button, and the engine has no way of knowing whos clicking button
        //for now we will cancel interrupts made when ai is making choice
        //in the future we will need a way to find out if the human is pressing the keys and which player.
        if (this->source->controller() != game->isInterrupting)
            game->mLayers->stackLayer()->cancelInterruptOffer(ActionStack::DONT_INTERRUPT, false);
        if (key == 0)
            key = JGE_BTN_NEXT;
        if (key != JGE_BTN_OK && key != JGE_BTN_NEXT)
            key = JGE_BTN_OK;
    }
    if (JGE_BTN_SEC == key || JGE_BTN_PREV == key || JGE_BTN_NEXT == key || JGE_BTN_MENU == key)
    {
        if (tc && (tc->targetMin == false || tc->maxtargets == TargetChooser::UNLITMITED_TARGETS))
        {
            tc->done = true;
            tc->autoChoice = false;
            tc->forceTargetListReadyByPlayer = 1;
            if (!abilitySecond && !tc->getNbTargets() && tc->source)
            {
                tc->source->getObserver()->cardClick(tc->source, 0, false);
                //remove the first ability to avoid a menu react.
                source->getObserver()->mLayers->stackLayer()->Remove(abilityFirst);
                game->removeObserver(abilityFirst);
                if (!this->source->controller()->isAI())
                    game->Update(0);
                if (zone->cards.size())
                {
                    initDisplay(revealTopAmount);
                    abilitySecond = contructAbility(abilityTwo);
                    game->addObserver(abilitySecond);
                    if (revealTopAmount == 0 && dontRevealAfter && delayed) // Execute delayed action even if dontshow option is active.
                    {
                        MTGAbility * delayedA = contructAbility(delayedAbilityString);
                        if (delayedA->oneShot)
                        {
                            delayedA->resolve();
                            SAFE_DELETE(delayedA);
                        }
                        else
                            delayedA->addToGame();
                    }
                }
            }
            else if (tc->source)
            {
                tc->source->getObserver()->cardClick(tc->source, 0, false);
            }
        }
        else if (!tc && !abilitySecond)
        {           
            CheckUserInput(JGE_BTN_OK);
        }
        return false;
    }
    if (JGE_BTN_OK == key)
    {                     
        if (!tc && !abilitySecond)
        {
            //remove the first ability to avoid a menu react.
            source->getObserver()->mLayers->stackLayer()->Remove(abilityFirst);
            game->removeObserver(abilityFirst);
            if (!this->source->controller()->isAI())
                game->Update(1);

            if (zone->cards.size() || (revealDisplay && !zone->cards.size()))
            {
                initDisplay(revealTopAmount);
                abilitySecond = contructAbility(abilityTwo);
                game->addObserver(abilitySecond);
                if(revealTopAmount == 0 && dontRevealAfter && delayed)
                {
                    MTGAbility * delayedA = contructAbility(delayedAbilityString);
                    if (delayedA->oneShot)
                    {
                        delayedA->resolve();
                        SAFE_DELETE(delayedA);
                    }
                    else
                        delayedA->addToGame();
                }
            }
        }
        if (!tc && abilitySecond && abilitySecond->testDestroy())
        {    
            do
            {
                if (!RevealZone->cards.size())
                    break;
                MTGCardInstance * toMove = RevealZone->cards[0];
                if (toMove)
                {
                    source->revealedLast = toMove;
                    MTGAbility * a = NEW AAMover(game, getMaxId(), source, toMove,"library", "Place on top");
                    a->oneShot = true;
                    a->resolve();
                    SAFE_DELETE(a);
                }
            } while (RevealZone->cards.size());

            if (delayed)
            {
                MTGAbility * delayedA = contructAbility(delayedAbilityString);
                if (delayedA->oneShot)
                {
                    delayedA->resolve();
                    SAFE_DELETE(delayedA);
                }
                else
                delayedA->addToGame();
            }
        }
    }
    if (revealDisplay)
        return revealDisplay->CheckUserInput(key);
    return false;
}

MTGScryCards * MTGScryCards::clone() const
{
    return NEW MTGScryCards(*this);
}

MTGScryCards::~MTGScryCards()
{
    for (vector<CardDisplay*>::iterator it = trashDisplays.begin(); it != trashDisplays.end(); ++it)
        SAFE_DELETE(*it);
    for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
        SAFE_DELETE(*it);
}

int MTGScryCards::receiveEvent(WEvent* e)
{

    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    {
        if (event->from == zone)
        {
            CardView* t;
            if (event->card->view)
                t = NEW CardView(CardView::nullZone, event->card, *(event->card->view));
            else
                t = NEW CardView(CardView::nullZone, event->card, (float)x, (float)y);
            //we store copies of the card view since moving to and from card displays also deletes the guis stored in cards.
            //GuiLayer::resetObjects() is the main reason we need to back them up. card views are set to NULL maybe more often than
            //they should be, possibly someone being to over cautious.
            t->actX = SCREEN_WIDTH;
            t->actY = SCREEN_HEIGHT * -2;
            //correct cards x and y, last known location was the reveal display.
            cards.push_back(t);
            return 1;
        }
    }
    return 0;
}

//scry wrapper
GenericScryAbility::GenericScryAbility(GameObserver* observer, int id, MTGCardInstance * source,
    Targetable * target, string _howMany) :
    ActivatedAbility(observer, id, source, NULL), howMany(_howMany)
{
    this->GetId();
}

int GenericScryAbility::resolve()
{
    bool replaceScry = false;
    for(int i = 0; i < source->lastController->game->battlefield->nb_cards; i ++){
        if(source->lastController->game->battlefield->cards[i]->has(Constants::REPLACESCRY))
            replaceScry = true;
    }
    if(!replaceScry && source->lastController->isAI() && source->getAICustomCode().size())
    {
        string abi = source->getAICustomCode();
        std::transform(abi.begin(), abi.end(), abi.begin(), ::tolower);//fix crash
        AbilityFactory af(game);
        MTGAbility * a3 = af.parseMagicLine(abi, this->GetId(), NULL, source);
        a3->oneShot = 1;
        a3->canBeInterrupted = false;
        a3->resolve();
        SAFE_DELETE(a3);
    } else if(!replaceScry) {
        MTGAbility * ability = NEW MTGScryCards(game, this->GetId(), source, howMany);
        ability->addToGame();
    }
    string number = "0";
    vector<string> amount = parseBetween(howMany, "", " ");
    if (amount.size())
        number = amount[1];
    WParsedInt nbCardP(number, NULL, source);
    source->scryedCards = nbCardP.getValue();
    WEvent * e = NEW WEventCardScryed(source);
    game->receiveEvent(e);
    return 1;
}

const string GenericScryAbility::getMenuText()
{
    return "Scry Cards";
}

GenericScryAbility * GenericScryAbility::clone() const
{
    GenericScryAbility * a = NEW GenericScryAbility(*this);
    return a;
}

GenericScryAbility::~GenericScryAbility()
{
    //SAFE_DELETE(ability);
}

////////////////////////
//Activated Abilities

//Generic Activated Abilities
GenericActivatedAbility::GenericActivatedAbility(GameObserver* observer, string newName, string castRestriction, int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost,
        string limit,MTGAbility * sideEffects,string usesBeforeSideEffects, int restrictions, MTGGameZone * dest) :
    ActivatedAbility(observer, _id, card, _cost, restrictions,limit,sideEffects,usesBeforeSideEffects,castRestriction), NestedAbility(a), activeZone(dest),newName(newName)
{
    counters = 0;
    target = ability->target;
}

GenericActivatedAbility::GenericActivatedAbility(const GenericActivatedAbility &other):
    ActivatedAbility(other), NestedAbility(other), activeZone(other.activeZone), newName(other.newName)
{

}

int GenericActivatedAbility::resolve()
{
    //Note: I've seen a similar block in some other MTGAbility, can this be refactored .
    if (abilityCost)
    {
        source->X = 0;
        ManaCost * diff = abilityCost->Diff(getCost());
        source->X = diff->hasX();
        SAFE_DELETE(diff);
    }
    ability->target = target; //may have been updated...
    if (ability)
        return ability->resolve();
    return 0;
}

const string GenericActivatedAbility::getMenuText()
{
if(newName.size())
return newName.c_str();
    if (ability)
        return ability->getMenuText();
    return "Error";
}

int GenericActivatedAbility::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    if (dynamic_cast<AAMorph*> (ability) && !card->isMorphed && !card->morphed && card->turningOver)
        return 0;
    return ActivatedAbility::isReactingToClick(card, mana);
}

void GenericActivatedAbility::Update(float dt)
{
    ActivatedAbility::Update(dt);
}

int GenericActivatedAbility::testDestroy()
{
    if (!activeZone)
        return ActivatedAbility::testDestroy();
    if (activeZone->hasCard(source))
        return 0;
    return 1;

}

GenericActivatedAbility * GenericActivatedAbility::clone() const
{
    GenericActivatedAbility * a = NEW GenericActivatedAbility(*this);

    a->ability = ability->clone();
    return a;
}

GenericActivatedAbility::~GenericActivatedAbility()
{
    SAFE_DELETE(ability);
}

//AA Alter Poison
AAAlterPoison::AAAlterPoison(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int poison, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), poison(poison)
{
}

int AAAlterPoison::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(!pTarget->inPlay()->hasAbility(Constants::POISONSHROUD) || poison < 0){
            _target->poisonCount += poison;
            if(pTarget->poisonCount < 0)
                pTarget->poisonCount = 0;
            if(poison > 0)
            {
                WEvent * e = NEW WEventplayerPoisoned(pTarget, poison); // Added an event when player receives any poison counter.
                game->receiveEvent(e);
            }//todo loses poison event
        }
    }
    return 0;
}

const string AAAlterPoison::getMenuText()
{
    WParsedInt parsedNum(poison);
    return _(parsedNum.getStringValue() + " Poison ").c_str();
}

AAAlterPoison * AAAlterPoison::clone() const
{
    return NEW AAAlterPoison(*this);
}

AAAlterPoison::~AAAlterPoison()
{
}

//AA Bearer Chosen
AARingBearerChosen::AARingBearerChosen(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
    andAbility = NULL;
}

int AARingBearerChosen::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(_target)
    {
        MTGCardInstance * currentBearer = NULL;
        for (int j = _target->controller()->game->inPlay->nb_cards - 1; j >= 0; --j){
            if(_target->controller()->game->inPlay->cards[j]->isRingBearer == 1){
                currentBearer = _target->controller()->game->inPlay->cards[j];
                _target->controller()->game->inPlay->cards[j]->isRingBearer = 0;
                break;
            }
        }
        _target->isRingBearer = 1;
        bool bearerChanged = false;
        if(currentBearer == NULL || currentBearer != _target){
            for (int j = _target->controller()->game->inPlay->nb_cards - 1; j >= 0; --j){
                if(_target->controller()->game->inPlay->cards[j]->name == "The Ring"){
                    for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
                    {
                        MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
                        AEquip * eq = dynamic_cast<AEquip*> (a);
                        if (eq && eq->source == _target->controller()->game->inPlay->cards[j])
                        {
                            ((AEquip*)a)->unequip();
                            ((AEquip*)a)->equip(_target);
                            bearerChanged = true;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        WEventCardBearerChosen * e = NEW WEventCardBearerChosen(_target);
        e->bearerChanged = bearerChanged;
        game->receiveEvent(e);
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AARingBearerChosen::getMenuText()
{
    return "The Ring bearer has been chosen";
}

AARingBearerChosen * AARingBearerChosen::clone() const
{
    AARingBearerChosen * a = NEW AARingBearerChosen(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AARingBearerChosen::~AARingBearerChosen()
{
    SAFE_DELETE(andAbility);
}

//AA Explores Event
AAExploresEvent::AAExploresEvent(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), card(_source)
{
}

int AAExploresEvent::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            WEvent * e = NEW WEventCardExplored(card);
            game->receiveEvent(e);
        }
    }
    return 0;
}

const string AAExploresEvent::getMenuText()
{
    return "Explores event called";
}

AAExploresEvent * AAExploresEvent::clone() const
{
    return NEW AAExploresEvent(*this);
}

AAExploresEvent::~AAExploresEvent()
{
}

//AA Boast Event
AABoastEvent::AABoastEvent(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), card(_source)
{
}

int AABoastEvent::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            WEvent * e = NEW WEventCardBoasted(card);
            game->receiveEvent(e);
        }
    }
    return 0;
}

const string AABoastEvent::getMenuText()
{
    return "Boast event called";
}

AABoastEvent * AABoastEvent::clone() const
{
    return NEW AABoastEvent(*this);
}

AABoastEvent::~AABoastEvent()
{
}

//AA Surveil Event
AASurveilEvent::AASurveilEvent(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), card(_source)
{
}

int AASurveilEvent::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            WEvent * e = NEW WEventCardSurveiled(card);
            game->receiveEvent(e);
        }
    }
    return 0;
}

const string AASurveilEvent::getMenuText()
{
    return "Surveil event called";
}

AASurveilEvent * AASurveilEvent::clone() const
{
    return NEW AASurveilEvent(*this);
}

AASurveilEvent::~AASurveilEvent()
{
}

//AA Surveil Offset
AAAlterSurveilOffset::AAAlterSurveilOffset(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int surveilOffset, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), surveilOffset(surveilOffset)
{
}

int AAAlterSurveilOffset::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            pTarget->surveilOffset += surveilOffset;
            if(pTarget->surveilOffset < 0)
                pTarget->surveilOffset = 0;
        }
    }
    return 0;
}

const string AAAlterSurveilOffset::getMenuText()
{
    WParsedInt parsedNum(surveilOffset);
    return _(parsedNum.getStringValue() + " Surveil Offset ").c_str();
}

AAAlterSurveilOffset * AAAlterSurveilOffset::clone() const
{
    return NEW AAAlterSurveilOffset(*this);
}

AAAlterSurveilOffset::~AAAlterSurveilOffset()
{
}

//AA Devotion Offset
AAAlterDevotionOffset::AAAlterDevotionOffset(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int devotionOffset, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), devotionOffset(devotionOffset)
{
}

int AAAlterDevotionOffset::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            pTarget->devotionOffset += devotionOffset;
            if(pTarget->devotionOffset < 0)
                pTarget->devotionOffset = 0;
        }
    }
    return 0;
}

const string AAAlterDevotionOffset::getMenuText()
{
    WParsedInt parsedNum(devotionOffset);
    return _(parsedNum.getStringValue() + " Devotion Offset ").c_str();
}

AAAlterDevotionOffset * AAAlterDevotionOffset::clone() const
{
    return NEW AAAlterDevotionOffset(*this);
}

AAAlterDevotionOffset::~AAAlterDevotionOffset()
{
}

//AA Dungeon Completed
AAAlterDungeonCompleted::AAAlterDungeonCompleted(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int dungeoncounter, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), dungeoncounter(dungeoncounter)
{
}

int AAAlterDungeonCompleted::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            source = pTarget->game->putInSideboard(source);
            source->basicAbilities[Constants::DUNGEONCOMPLETED] = 1;
            pTarget->dungeonCompleted += dungeoncounter;
            if(pTarget->dungeonCompleted < 0)
                pTarget->dungeonCompleted = 0;
            WEvent * e = NEW WEventCardDungeonCompleted(source, pTarget->dungeonCompleted, source->controller()->getDisplayName());
            game->receiveEvent(e);
        }
    }
    return 0;
}

const string AAAlterDungeonCompleted::getMenuText()
{
    WParsedInt parsedNum(dungeoncounter);
    return _(parsedNum.getStringValue() + " Dungeon Completed Counter ").c_str();
}

AAAlterDungeonCompleted * AAAlterDungeonCompleted::clone() const
{
    return NEW AAAlterDungeonCompleted(*this);
}

AAAlterDungeonCompleted::~AAAlterDungeonCompleted()
{
}

//AA Yidaro Count
AAAlterYidaroCount::AAAlterYidaroCount(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int yidarocount, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), yidarocount(yidarocount)
{
}

int AAAlterYidaroCount::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            pTarget->yidaroCount += yidarocount;
            if(pTarget->yidaroCount < 0)
                pTarget->yidaroCount = 0;
        }
    }
    return 0;
}

const string AAAlterYidaroCount::getMenuText()
{
    WParsedInt parsedNum(yidarocount);
    return _(parsedNum.getStringValue() + " Yidaro Cycling Counter ").c_str();
}

AAAlterYidaroCount * AAAlterYidaroCount::clone() const
{
    return NEW AAAlterYidaroCount(*this);
}

AAAlterYidaroCount::~AAAlterYidaroCount()
{
}

//AA Ring Temptations
AAAlterRingTemptations::AAAlterRingTemptations(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int temptations, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), temptations(temptations)
{
    andAbility = NULL;
}

int AAAlterRingTemptations::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            pTarget->ringTemptations += temptations;
            if(pTarget->ringTemptations < 0)
                pTarget->ringTemptations = 0;
            WEvent * e = NEW WEventplayerTempted(pTarget);
            game->receiveEvent(e);
            if(andAbility)
            {
                MTGAbility * andAbilityClone = andAbility->clone();
                andAbilityClone->target = _target;
                if(andAbility->oneShot)
                {
                    andAbilityClone->resolve();
                    SAFE_DELETE(andAbilityClone);
                }
                else
                {
                    andAbilityClone->addToGame();
                }
            }
        }
    }
    return 0;
}

const string AAAlterRingTemptations::getMenuText()
{
    return "The Ring tempts you";
}

AAAlterRingTemptations * AAAlterRingTemptations::clone() const
{
    AAAlterRingTemptations * a = NEW AAAlterRingTemptations(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AAAlterRingTemptations::~AAAlterRingTemptations()
{
    SAFE_DELETE(andAbility);
}

//AA Monarch
AAAlterMonarch::AAAlterMonarch(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who)
{
}

int AAAlterMonarch::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            if(!pTarget->monarch){
                pTarget->monarch = 1;
                pTarget->opponent()->monarch = 0;
                WEvent * e = NEW WEventplayerMonarch(pTarget);
                game->receiveEvent(e);
            }
        }
    }
    return 0;
}

const string AAAlterMonarch::getMenuText()
{
    return _("A player becomes the Monarch").c_str();
}

AAAlterMonarch * AAAlterMonarch::clone() const
{
    return NEW AAAlterMonarch(*this);
}

AAAlterMonarch::~AAAlterMonarch()
{
}

//AA Initiative
AAAlterInitiative::AAAlterInitiative(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who)
{
}

int AAAlterInitiative::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            pTarget->initiative = 1;
            pTarget->opponent()->initiative = 0;
            WEvent * e = NEW WEventplayerInitiative(pTarget);
            game->receiveEvent(e);
        }
    }
    return 0;
}

const string AAAlterInitiative::getMenuText()
{
    return _("A player takes the Initiative").c_str();
}

AAAlterInitiative * AAAlterInitiative::clone() const
{
    return NEW AAAlterInitiative(*this);
}

AAAlterInitiative::~AAAlterInitiative()
{
}

//AA Energy Counters
AAAlterEnergy::AAAlterEnergy(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int energy, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), energy(energy)
{
}

int AAAlterEnergy::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            if(!pTarget->inPlay()->hasAbility(Constants::ENERGYSHROUD) || energy < 0){
                pTarget->energyCount += energy;
                if(pTarget->energyCount < 0)
                    pTarget->energyCount = 0;
                if(energy > 0)
                {
                    WEvent * e = NEW WEventplayerEnergized(pTarget, energy);
                    game->receiveEvent(e);
                }//todo loses enegy event
            }
        }
    }
    return 0;
}

const string AAAlterEnergy::getMenuText()
{
    WParsedInt parsedNum(energy);
    return _(parsedNum.getStringValue() + " Energy ").c_str();
}

AAAlterEnergy * AAAlterEnergy::clone() const
{
    return NEW AAAlterEnergy(*this);
}

AAAlterEnergy::~AAAlterEnergy()
{
}

//AA Experience Counters
AAAlterExperience::AAAlterExperience(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int experience, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), experience(experience)
{
}

int AAAlterExperience::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(pTarget)
        {
            if(!pTarget->inPlay()->hasAbility(Constants::EXPSHROUD) || experience < 0){
                pTarget->experienceCount += experience;
                if(pTarget->experienceCount < 0)
                    pTarget->experienceCount = 0;
                if(experience > 0)
                {
                    WEvent * e = NEW WEventplayerExperienced(pTarget, experience);
                    game->receiveEvent(e);
                }//todo loses experience event
            }
        }
    }
    return 0;
}

const string AAAlterExperience::getMenuText()
{
    WParsedInt parsedNum(experience);
    return _(parsedNum.getStringValue() + " Experience ").c_str();
}

AAAlterExperience * AAAlterExperience::clone() const
{
    return NEW AAAlterExperience(*this);
}

AAAlterExperience::~AAAlterExperience()
{
}

//Damage Prevent
AADamagePrevent::AADamagePrevent(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int preventing, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), preventing(preventing)
{
    aType = MTGAbility::STANDARD_PREVENT;
}

int AADamagePrevent::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        _target->preventable += preventing;
    }
    return 0;
}

const string AADamagePrevent::getMenuText()
{
    return "Prevent Damage";
}

AADamagePrevent * AADamagePrevent::clone() const
{
    return NEW AADamagePrevent(*this);
}

AADamagePrevent::~AADamagePrevent()
{
}

//AADamager
AADamager::AADamager(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, string d, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), d(d)
{
    aType = MTGAbility::DAMAGER;
    redirected = false;
    andAbility = NULL;
}

int AADamager::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        WParsedInt damage(d, NULL, (MTGCardInstance *)source);
        if(_target == game->opponent() && game->opponent()->inPlay()->hasType("planeswalker") && !redirected)
        {
            vector<MTGAbility*>selection;
            MTGCardInstance * check = NULL;
            this->redirected = true;
            MTGAbility * setPlayer = this->clone();
            this->redirected = false;
            selection.push_back(setPlayer);
            int checkWalkers = ((Player*)_target)->game->battlefield->cards.size();
            for(int i = 0; i < checkWalkers;++i)
            {
                check = ((Player*)_target)->game->battlefield->cards[i];
                if(check->hasType(Subtypes::TYPE_PLANESWALKER))
                {
                    this->redirected = true;
                    MTGAbility * setWalker = this->clone();
                    this->redirected = false;
                    setWalker->oneShot = true;
                    setWalker->target = check;
                    selection.push_back(setWalker);
                }
            }
            if(selection.size())
            {
                MTGAbility * a1 = NEW MenuAbility(game, this->GetId(), source, source,true,selection);
                game->mLayers->actionLayer()->currentActionCard = source;
                a1->resolve();
            }
            return 1;
        }
        game->mLayers->stackLayer()->addDamage(source, _target, damage.getValue());
        game->mLayers->stackLayer()->resolve();
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = source;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

int AADamager::getDamage()
{
    WParsedInt damage(d, NULL, (MTGCardInstance *)source);
    return damage.getValue();
}

const string AADamager::getMenuText()
{
    MTGCardInstance * _target = dynamic_cast<MTGCardInstance*>(target);
    if(_target && (_target->hasType(Subtypes::TYPE_PLANESWALKER) || _target->hasType(Subtypes::TYPE_BATTLE)))
        return _target->name.c_str();
    if(redirected)
    {
        if(d.size())
        {
            WParsedInt parsedNum(d, NULL, source);
            return _("Deal " + parsedNum.getStringValue() + " Damage to Player").c_str();
        }
        return "Damage Player";
    }
        
    if(d.size())
    {
        WParsedInt parsedNum(d, NULL, source);
        return _("Deal " + parsedNum.getStringValue() + " Damage").c_str();
    }
    return "Damage";
}

AADamager * AADamager::clone() const
{
    AADamager * a = NEW AADamager(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AADamager::~AADamager()
{
    SAFE_DELETE(andAbility);
}

//AADepleter
AADepleter::AADepleter(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string nbcardsStr, ManaCost * _cost, int who, bool toexile, bool colorrepeat, bool namerepeat) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who),nbcardsStr(nbcardsStr),toexile(toexile), colorrepeat(colorrepeat), namerepeat(namerepeat)
{
    andAbility = NULL;
}

int AADepleter::resolve()
{
    Player * player = getPlayerFromTarget(getTarget());
    if (player)
    {
        WParsedInt numCards(nbcardsStr, NULL, source);
        MTGLibrary * library = player->game->library;
        if (colorrepeat && library->nb_cards)
        {
            bool repeating = false;
            do
            {
                repeating = false;
                vector<MTGCardInstance*>found;
                for (int i = 0; i < numCards.getValue(); i++)
                {
                    if (library->nb_cards)
                    {
                        if(library->nb_cards > i)
                        found.push_back(library->cards[(library->nb_cards - 1) - i]);
                    }
                }

                for (vector<MTGCardInstance*>::iterator it = found.begin(); it != found.end(); it++)
                {
                    MTGCardInstance * cardFirst = *it;
                    if (cardFirst->isLand())
                        continue;
                    for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
                    {
                        if (cardFirst->hasColor(i))
                        {
                            for (vector<MTGCardInstance*>::iterator secondit = found.begin(); secondit != found.end(); secondit++)
                            {
                                MTGCardInstance * cardSecond = *secondit;
                                if (cardSecond->isLand())
                                    continue;
                                if (cardSecond->hasColor(i) && cardFirst != cardSecond)
                                {
                                    repeating = true;
                                }
                            }
                        }
                    }
                }

                do
                {
                    if (found.size())
                    {
                        MTGCardInstance * toMove = found.back();
                        if (toMove)
                        {
                            if (toexile)
                                player->game->putInZone(toMove, library, player->game->exile);
                            else
                                player->game->putInZone(toMove, library, player->game->graveyard);
                            found.pop_back();
                        }
                    }
                } while (found.size());

            } while (repeating);
        }
        else if (namerepeat && library->nb_cards)
        {
            bool repeating = false;
            do
            {
                repeating = false;
                vector<MTGCardInstance*>found;
                for (int i = 0; i < numCards.getValue(); i++)
                {
                    if (library->nb_cards)
                    {
                        if (library->nb_cards  > i)
                        found.push_back(library->cards[(library->nb_cards - 1) - i]);
                    }
                }

                for (vector<MTGCardInstance*>::iterator it = found.begin(); it != found.end(); it++)
                {
                    MTGCardInstance * cardFirst = *it;
                    for (vector<MTGCardInstance*>::iterator secondit = found.begin(); secondit != found.end(); secondit++)
                    {
                        MTGCardInstance * cardSecond = *secondit;
                        if (cardSecond->name == cardFirst->name && cardFirst != cardSecond)
                        {
                            repeating = true;
                        }
                    }

                }

                do
                {
                    if (found.size())
                    {
                        MTGCardInstance * toMove = found.back();
                        if (toMove)
                        {
                            if (toexile)
                                player->game->putInZone(toMove, library, player->game->exile);
                            else
                                player->game->putInZone(toMove, library, player->game->graveyard);
                            found.pop_back();
                        }
                    }
                } while (found.size());
            } while (repeating);
        }
        else
        {
            for (int i = 0; i < numCards.getValue(); i++)
            {
                if (library->nb_cards)
                {
                    if (toexile)
                        player->game->putInZone(library->cards[library->nb_cards - 1], library, player->game->exile);
                    else
                        player->game->putInZone(library->cards[library->nb_cards - 1], library, player->game->graveyard);
                }
            }
        }
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = source;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
    }
    return 1;
}

const string AADepleter::getMenuText()
{
    if(toexile)
    {
        if(nbcardsStr.size())
        {
            WParsedInt parsedNum(nbcardsStr, NULL, source);
            return _("Ingest " + parsedNum.getStringValue()).c_str();
        }
        return "Ingest";
    }
    
    if(nbcardsStr.size())
    {
        WParsedInt parsedNum(nbcardsStr, NULL, source);
        return _("Deplete " + parsedNum.getStringValue()).c_str();
    }
    return "Deplete";
}

AADepleter * AADepleter::clone() const
{
    AADepleter * a = NEW AADepleter(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AADepleter::~AADepleter()
{
    SAFE_DELETE(andAbility);
}

//AACascade
AACascade::AACascade(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, string nbcardsStr, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0),nbcardsStr(nbcardsStr)
{
    selectedCards.clear();
    oldOrder.clear();
    newOrder.clear();
    castingThis = NULL;
}
int AACascade::resolve()
{
    Player * player = source->controller();
    if (!player)
        return 0;
    WParsedInt numCards(nbcardsStr, NULL, source);
    MTGLibrary * library = player->game->library;
    MTGRemovedFromGame * exile = player->game->exile;
    MTGCardInstance * viable = NULL;
    int counter = 0;
    bool found = false;
    for (int i = 0; i < numCards.getValue(); i++)
    {
        //*//*//*//
        if (found)
            continue;
        //////////////////////////////////////////////
        if (!library->nb_cards)
            continue;
        //////////////////////////////////////////////
        while (library->nb_cards && !found)
        {
            viable = library->cards[library->nb_cards -1];
            if (!found)
            {
                if (!viable->isLand() && (viable->getManaCost()->getConvertedCost() < source->getManaCost()->getConvertedCost()))
                {
                    viable = player->game->putInZone(viable, library, exile);
                    viable->isCascaded = true;
                    castingThis = viable;
                    found = true;
                }
                else
                {
                    viable = player->game->putInZone(viable, library, exile);
                    viable->isCascaded = true;
                    counter++;
                }
            }
        }

        //*//*//*//*
    }
    ////////////////////////////////////////////
    for (int j = 0; j < exile->nb_cards; j++)
    {
        if (exile->cards[j]->isCascaded)
        {
            MTGCardInstance * CardToPutBack = exile->cards[j];;
            CardToPutBack->isCascaded = false;
            selectedCards.push_back(CardToPutBack);
        }
    }
    //////////////////////////////////////////
    if (selectedCards.size())
    {
        do
        {
            MTGCardInstance * toMove = selectedCards.back();
            if (toMove)
            {
                MTGAbility * a = NEW AALibraryBottom(game, game->mLayers->actionLayer()->getMaxId(), source, toMove);
                a->oneShot = 1;
                a->resolve();
                SAFE_DELETE(a);
                selectedCards.pop_back();
            }
        } while (selectedCards.size());

        if (castingThis)
        {
            while (castingThis->next)
                castingThis = castingThis->next;
            toCastCard(castingThis);
        }
    }
    //////////////////////////////////////
    return 1;
}

void AACascade::toCastCard(MTGCardInstance * thisCard)
{
    MTGAbility *ac = NEW AACastCard(game, game->mLayers->actionLayer()->getMaxId(), thisCard, thisCard,false,false,true,"","",false,false);
    MayAbility *ma1 = NEW MayAbility(game, game->mLayers->actionLayer()->getMaxId(), ac->clone(), thisCard,false);
    MTGAbility *ga1 = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), thisCard,NULL,ma1->clone());
    SAFE_DELETE(ac);
    SAFE_DELETE(ma1);
    ga1->resolve();
    SAFE_DELETE(ga1);
    return;
}

const string AACascade::getMenuText()
{
    return "Cascade";
}

AACascade * AACascade::clone() const
{
    return NEW AACascade(*this);
}

//take extra turns or skip turns, values in the negitive will make you skip.
AAModTurn::AAModTurn(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string nbTurnStr, ManaCost * _cost, int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who),nbTurnStr(nbTurnStr)
{

}
    int AAModTurn::resolve()
    {
        Player * player = getPlayerFromTarget(getTarget());
        if (player)
        {
            WParsedInt numTurns(nbTurnStr, NULL, source);
            if(numTurns.getValue() > 0)
            {
                player->extraTurn += numTurns.getValue();
            }
            else
            {
                player->skippingTurn += abs(numTurns.getValue());

            }
        }
        return 1;
    }

    const string AAModTurn::getMenuText()
    {
        WParsedInt numTurns(nbTurnStr, NULL, source);
        if(numTurns.getValue() > 0)
            return _("Take " + numTurns.getStringValue() + " Extra Turn(s)").c_str();
        else
            return "Skip A Turn(s)";
    }

AAModTurn * AAModTurn::clone() const
{
    return NEW AAModTurn(*this);
}

//move target to specifc position of owners library from the top
AALibraryPosition::AALibraryPosition(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, unsigned int _position) :
ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
    andAbility = NULL;
    position = _position;
}

int AALibraryPosition::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    _target = _target->owner->game->putInLibrary(_target);
    if (_target)
    {
        MTGLibrary * library = _target->owner->game->library;
        vector<MTGCardInstance *>oldOrder = library->cards;
        vector<MTGCardInstance *>newOrder;
        if(position > oldOrder.size())
            position = oldOrder.size(); //Avoid to exceed the library dimension.
        for(unsigned int k = 0; k < oldOrder.size() - position; ++k)
        {
            MTGCardInstance * rearranged = oldOrder[k];
            if(rearranged != _target)
                newOrder.push_back(rearranged);
        }
        newOrder.push_back(_target);
        for(unsigned int k = oldOrder.size() - position ; k < oldOrder.size(); ++k)
        {
            MTGCardInstance * rearranged = oldOrder[k];
            if(rearranged != _target)
                newOrder.push_back(rearranged);
        }
        library->cards = newOrder;
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AALibraryPosition::getMenuText()
{
    return "Put in Library in a specific position from the top";
}

AALibraryPosition * AALibraryPosition::clone() const
{
    AALibraryPosition * a = NEW AALibraryPosition(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AALibraryPosition::~AALibraryPosition()
{
    SAFE_DELETE(andAbility);
}

//move target to bottom of owners library
AALibraryBottom::AALibraryBottom(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
    andAbility = NULL;
}

int AALibraryBottom::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    _target = _target->owner->game->putInLibrary(_target);
    if (_target)
    {
        MTGLibrary * library = _target->owner->game->library;
        vector<MTGCardInstance *>oldOrder = library->cards;
        vector<MTGCardInstance *>newOrder;
        newOrder.push_back(_target);
        for(unsigned int k = 0;k < oldOrder.size();++k)
        {
            MTGCardInstance * rearranged = oldOrder[k];
            if(rearranged != _target)
                newOrder.push_back(rearranged);
        }
        library->cards = newOrder;
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AALibraryBottom::getMenuText()
{
    return "Bottom Of Library";
}

AALibraryBottom * AALibraryBottom::clone() const
{
    AALibraryBottom * a = NEW AALibraryBottom(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AALibraryBottom::~AALibraryBottom()
{
    SAFE_DELETE(andAbility);
}

//AACopier
AACopier::AACopier(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, string optionsList) :
    ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
    andAbility = NULL;
    options = optionsList;
    isactivated = false;
}

int AACopier::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be copied, they will follow the fate of top-card
        bool tokencopied = false;
        if(_target->isToken || (_target->isACopier && _target->hasCopiedToken))
            tokencopied = true;
        /*this solves one thing.. if you copy a nontoken card with dragon breath attached that gives haste*/
        source->hasCopiedToken = tokencopied;
        /*since we look for the real card it will not copy granted haste ability however for token we copy all*/
        /*but how to do backup for token so we just copy the backup???*/
        bool nolegend = options.find("nolegend") != string::npos; // Check if the copy has to be legendary or not. (e.g. Echoing Equation)
        string keepname = "";
        if(options.find("keepname") != string::npos){ // Keep the original name after the copy. (e.g. "Olag, Ludevic's Hubris")
            keepname = source->getName();
        }
        if(tokencopied && !_target->isACopier && !_target->getMTGId())
        {
            source->copy(_target->tokCard, nolegend);
            //if the token doesn't have cda/dynamic pt then allow this...
            if(!_target->isCDA)
            {
                if(_target->pbonus > 0)
                    source->power = _target->power - _target->pbonus;
                else
                    source->power = _target->power + abs(_target->pbonus);
                if(_target->tbonus > 0)
                {
                    source->toughness = _target->toughness - _target->tbonus;
                    source->life = _target->toughness - _target->tbonus;
                }
                else
                {
                    source->toughness = _target->toughness + abs(_target->tbonus);
                    source->life = _target->toughness + abs(_target->tbonus);
                }
            }
        }
        else
        {
            source->nameOrig = source->name; // Saves the orignal card name before become a copy
            source->copy(_target, nolegend);
        }
        source->isACopier = true;
        source->copiedID = _target->copiedID;
        if(_target->isMorphed)
        {
            source->power = 2;
            source->life = 2;
            source->toughness = 2;
            source->setColor(0,1);
            source->name = "Morph";
            source->types.clear();
            string cre = "Creature";
            source->setType(cre.c_str());
            source->basicAbilities.reset();
            source->getManaCost()->resetCosts();
        }
        if(_target->TokenAndAbility)
        {//the source copied a token with tokenandAbility
            MTGAbility * TokenandAbilityClone = _target->TokenAndAbility->clone();
            TokenandAbilityClone->target = source;
            if(_target->TokenAndAbility->oneShot)
            {
                TokenandAbilityClone->resolve();
                SAFE_DELETE(TokenandAbilityClone);
            }
            else
            {
                TokenandAbilityClone->addToGame();
            }
        }
        if(source)
        {
            source->GrantedAndAbility = andAbility;
            AbilityFactory af(game);
            for(unsigned int i = 0;i < source->cardsAbilities.size();i++)
            {
                MTGAbility * a = dynamic_cast<MTGAbility *>(source->cardsAbilities[i]);

                if(a) game->removeObserver(a);
            }
            source->cardsAbilities.clear();
            source->magicText = _target->magicText;

            af.getAbilities(&currentAbilities, NULL, source);
            for (size_t i = 0; i < currentAbilities.size(); ++i)
            {
                MTGAbility * a = currentAbilities[i];
                a->source = (MTGCardInstance *) source;
                if (a)
                {
                    if (a->oneShot)
                    {
                        if(a->source->entersBattlefield)
                            a->resolve();
                        SAFE_DELETE(a);
                    }
                    else
                    {
                        a->addToGame();
                        MayAbility * dontAdd = dynamic_cast<MayAbility*>(a);
                        if(!dontAdd)
                        {
                            source->cardsAbilities.push_back(a);
                        }
                    }
                }
            }
            if(source->GrantedAndAbility)
            {
                MTGAbility * andAbilityClone = source->GrantedAndAbility->clone();
                andAbilityClone->target = source;
                if(andAbility->oneShot)
                {
                    andAbilityClone->resolve();
                    SAFE_DELETE(andAbilityClone);
                }
                else
                {
                    andAbilityClone->addToGame();
                }
            }
            if(keepname != "")
                source->name = keepname; // Keep the original name after the copy. (e.g. "Olag, Ludevic's Hubris")
        }
        currentAbilities.clear();
        return 1;
    }
    return 0;
}

const string AACopier::getMenuText()
{
    return "Copy";
}

AACopier * AACopier::clone() const
{
    AACopier * a = NEW AACopier(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AACopier::~AACopier()
{
    SAFE_DELETE(andAbility);
}

//phaseout
AAPhaseOut::AAPhaseOut(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
}

int AAPhaseOut::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be phased out, they will follow the fate of top-card
        _target->isPhased = true;
        
        _target->phasedTurn = game->turn;
        if(_target->view)
            _target->view->alpha = 50;
        _target->initAttackersDefensers();
        //add event phases out here
        WEvent * e = NEW WEventCardPhasesOut(_target,game->turn);
        game->receiveEvent(e);
        return 1;
    }
    return 0;
}

const string AAPhaseOut::getMenuText()
{
    return "Phase Out";
}

AAPhaseOut * AAPhaseOut::clone() const
{
    return NEW AAPhaseOut(*this);
}

//AAImprint
AAImprint::AAImprint(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
    andAbility = NULL;
}

int AAImprint::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be imprinted, they will follow the fate of top-card
        Player * p = _target->controller();
        if(p)
            p->game->putInExile(_target);

        while(_target->next)
            _target = _target->next;

        source->imprintedCards.push_back(_target);

        if (source->imprintedCards.size())
        {
            if (source->imprintedCards.back()->hasColor(Constants::MTG_COLOR_GREEN))
                source->imprintG += 1;
            if (source->imprintedCards.back()->hasColor(Constants::MTG_COLOR_BLUE))
                source->imprintU += 1;
            if (source->imprintedCards.back()->hasColor(Constants::MTG_COLOR_RED))
                source->imprintR += 1;
            if (source->imprintedCards.back()->hasColor(Constants::MTG_COLOR_BLACK))
                source->imprintB += 1;
            if (source->imprintedCards.back()->hasColor(Constants::MTG_COLOR_WHITE))
                source->imprintW += 1;
            if (source->imprintedCards.back()->getName().size())
            {
                source->currentimprintName = source->imprintedCards.back()->getName();
                source->imprintedNames.push_back(source->imprintedCards.back()->getName());
            }
        }
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AAImprint::getMenuText()
{
    return "Imprint";
}

AAImprint * AAImprint::clone() const
{
    AAImprint * a = NEW AAImprint(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AAImprint::~AAImprint()
{
    SAFE_DELETE(andAbility);
}

//AAHaunt
AAHaunt::AAHaunt(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
    andAbility = NULL;
}

int AAHaunt::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target && _target->hasType(Subtypes::TYPE_CREATURE))
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be haunted, they will follow the fate of top-card

        while(_target->next)
            _target = _target->next;

        _target->basicAbilities[Constants::ISPREY] = 1;
        source->hauntedCard = _target;

        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AAHaunt::getMenuText()
{
    return "Haunt";
}

AAHaunt * AAHaunt::clone() const
{
    AAHaunt * a = NEW AAHaunt(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AAHaunt::~AAHaunt()
{
    SAFE_DELETE(andAbility);
}

//AATrain
AATrain::AATrain(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
    andAbility = NULL;
}

int AATrain::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target && _target->hasType(Subtypes::TYPE_CREATURE) && _target->isAttacker())
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be trained, they will follow the fate of top-card

        while(_target->next)
            _target = _target->next;

        if(_target->counters)
            _target->counters->addCounter(1,1);

        WEvent * e = NEW WEventCardTrained(_target);
        game->receiveEvent(e);

        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AATrain::getMenuText()
{
    return "Training";
}

AATrain * AATrain::clone() const
{
    AATrain * a = NEW AATrain(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AATrain::~AATrain()
{
    SAFE_DELETE(andAbility);
}

//AAConjure
AAConjure::AAConjure(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, string _namedCard, string _cardZone) :
    ActivatedAbility(observer, _id, _source, _cost, 0),cardNamed(_namedCard),cardZone(_cardZone)
{
    target = _target;
    andAbility = NULL;
    theNamedCard = NULL;
}

MTGCardInstance * AAConjure::makeCard()
{
    string newName = cardNamed;
    if(newName.find(";")){//if it's a list of cards we choose one randomly (e.g. Tome of the Infinite)
        vector<string> names = split(newName, ';');
        newName = names.at(std::rand() % names.size());
    }
    MTGCardInstance * card = NULL;
    MTGCard * cardData = MTGCollection()->getCardByName(newName, source->setId);
    if(!cardData) return NULL;
    card = NEW MTGCardInstance(cardData, source->controller()->game);
    card->owner = source->controller();
    card->lastController = source->controller();
    source->controller()->game->sideboard->addCard(card);
    return card;
}

int AAConjure::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be conjured, they will follow the fate of top-card
        theNamedCard = makeCard();
        if(theNamedCard){
            Spell * spell = NULL;
            MTGGameZone * targetZone = MTGGameZone::stringToZone(game, cardZone, theNamedCard, NULL);
            MTGCardInstance * copy =  source->controller()->game->putInZone(theNamedCard, theNamedCard->currentZone, targetZone);
            if(!copy){
                this->forceDestroy = true;
                return 0;
            }
            if(targetZone == copy->controller()->game->battlefield){
                copy->changeController(source->controller(), true);
                if (game->targetChooser)
                {
                    game->targetChooser->Owner = source->controller();
                    spell = game->mLayers->stackLayer()->addSpell(copy, game->targetChooser, NULL, 1, 0);
                    game->targetChooser = NULL;
                }
                else
                {
                    spell = game->mLayers->stackLayer()->addSpell(copy, NULL, NULL, 1, 0);
                }
            }
            if(andAbility)
            {
                MTGAbility * andAbilityClone = andAbility->clone();
                andAbilityClone->target = copy;
                if(andAbility->oneShot)
                {
                    andAbilityClone->resolve();
                    SAFE_DELETE(andAbilityClone);
                }
                else
                {
                    andAbilityClone->addToGame();
                }
            }
            this->forceDestroy = true;
            return 1;
        }
    }
    return 0;
}

const string AAConjure::getMenuText()
{
    return "Conjure";
}

AAConjure * AAConjure::clone() const
{
    AAConjure * a = NEW AAConjure(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AAConjure::~AAConjure()
{
    SAFE_DELETE(andAbility);
}

//AAForetell
AAForetell::AAForetell(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
}

int AAForetell::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be foretold, they will follow the fate of top-card
        Player * p = _target->controller();
        if(p){
            MTGCardInstance * tmp = p->game->putInExile(_target);
            if(tmp){
                tmp->foretellTurn = source->getObserver()->turn;
                WEvent * e = NEW WEventCardForetold(tmp);
                game->receiveEvent(e);
            }
        }

        while(_target->next)
            _target = _target->next;

        return 1;
    }
    return 0;
}

const string AAForetell::getMenuText()
{
    return "Foretell";
}

AAForetell * AAForetell::clone() const
{
    return NEW AAForetell(*this);
}

//Counters
AACounter::AACounter(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target,string counterstring, const char * _name, int power, int toughness,
        int nb, int maxNb, bool noevent, ManaCost * cost) :
    ActivatedAbility(observer, id, source, cost, 0),counterstring(counterstring), nb(nb), maxNb(maxNb), power(power), toughness(toughness), name(_name), noevent(noevent)
{
    this->target = target;
    if (name.find("Level") != string::npos || name.find("level") != string::npos)
        aType = MTGAbility::STANDARD_LEVELUP;
    else
        aType = MTGAbility::COUNTERS;
        
    menu = "";
}

    int AACounter::resolve()
    {
        if (target)
        {
            MTGCardInstance * _target = (MTGCardInstance *) target;
            if(_target->isFlipped > 0 && _target->hasType(Subtypes::TYPE_PLANESWALKER))//is flipping pw
                return 0;

            AbilityFactory af(game);
            if(counterstring.size())
            {
                Counter * checkcounter = af.parseCounter(counterstring, source, NULL);
                nb = checkcounter->nb;
                delete checkcounter;
            }
            int totalcounters = 0;
            if (nb > 0)
            {
               if(_target->has(Constants::COUNTERSHROUD)) // Added to avoid the counter increasement (e.g. "Solemnity").
                   return 0;

                for (int i = 0; i < nb; i++)
                {
                    while (_target->next)
                        _target = _target->next;

                    
                    if(_target->getCurrentZone() != _target->controller()->game->battlefield ||
                        _target->getCurrentZone() != _target->controller()->opponent()->game->battlefield)
                    {
                        if(!_target->isFlipped && (power||toughness)) // Skipped control for flipped card to solve a bug on double faces cards (e.g. "Ashen Reaper").
                        {
                            if(_target->previousZone == _target->controller()->game->battlefield ||
                                _target->previousZone == _target->controller()->opponent()->game->battlefield)
                                return 0;
                        }
                    }

                    Counter * targetCounter = NULL;
                    int currentAmount = 0;
                    if (_target->counters && _target->counters->hasCounter(name.c_str(), power, toughness))
                    {
                        targetCounter = _target->counters->hasCounter(name.c_str(), power, toughness);
                        currentAmount = targetCounter->nb;
                    }
                    if(!maxNb || (maxNb && currentAmount < maxNb))
                    {
                        _target->counters->addCounter(name.c_str(), power, toughness, noevent, false, source);
                        totalcounters++;
                    }
                }
                if (!noevent)
                {
                    WEvent * w = NEW WEventTotalCounters(_target->counters, name.c_str(), power, toughness, true, false, totalcounters, false, source);
                    dynamic_cast<WEventTotalCounters*>(w)->targetCard = _target->counters->target;
                    _target->getObserver()->receiveEvent(w);
                }
            }
            else
            {
                for (int i = 0; i < -nb; i++)
                {
                    while (_target->next)
                        _target = _target->next;
                    _target->counters->removeCounter(name.c_str(), power, toughness, noevent, false, source);
                    totalcounters++;
                }
                if (!noevent)
                {
                    WEvent * e = NEW WEventTotalCounters(_target->counters, name.c_str(), power, toughness, false, true, totalcounters, false, source);
                    dynamic_cast<WEventTotalCounters*>(e)->targetCard = _target->counters->target;
                    _target->getObserver()->receiveEvent(e);
                }
            }

            _target->doDamageTest = 1;
            if(!_target->afterDamage())
            {
                //If a creature with +1/+1 counters on it gets enough -1/-1 counters to kill it, 
                //it dies before the two counters have the chance to cancel out. For example, 
                //if your Strangleroot Geist with a +1/+1 counter on it got three -1/-1 counters 
                //from Skinrender's "enters the battlefield" ability, the Geist would die with //
                //one +1/+1 counter and three -1/-1 counters and wouldn't return to the battlefield.
                for (int i = 0; i < _target->counters->mCount; i++)
                {
                    if (_target->counters->counters[i]->cancels(power, toughness) && !name.size() && _target->counters->counters[i]->nb > 0)
                    {
                        _target->counters->counters[i]->cancelCounter(power,toughness, source);
                    }
                }
            }

            //specail cases, indestructible creatures which recieve enough counters to kill it are destroyed as a state based effect
            if(_target->toughness <= 0 && _target->has(Constants::INDESTRUCTIBLE) && toughness < 0)
                _target->toGrave(true); // The indestructible cards can have different destination zone after death.
            return nb;
        }
        return 0;
    }

const string AACounter::getMenuText()
{
    if (menu.size())
    {
        return menu.c_str();
    }
    char buffer[128];

    if (name.size())
    {
        string s = name;
        menu.append(s.c_str());
    }

    if (power != 0 || toughness != 0)
    {
        sprintf(buffer, " %i/%i", power, toughness);
        menu.append(buffer);
    }

    menu.append(" Counter");
    if (nb != 1 && !(nb < -1000))
    {
        sprintf(buffer, ": %i", nb);
        menu.append(buffer);
    }

    sprintf(menuText, "%s", menu.c_str());
    return menuText;
}

AACounter * AACounter::clone() const
{
    return NEW AACounter(*this);
}

//shield a card from a certain type of counter.
ACounterShroud::ACounterShroud(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target,TargetChooser * tc, Counter * counter) :
MTGAbility(observer, id, source),csTc(tc),counter(counter),re(NULL)
{
}

int ACounterShroud::addToGame()
{
    SAFE_DELETE(re);
    re = NEW RECountersPrevention(this,source,(MTGCardInstance*)target,csTc,counter);
    if (re)
    {
        game->replacementEffects->add(re);
        return MTGAbility::addToGame();
    }
    return 0;
}

int ACounterShroud::destroy()
{
    game->replacementEffects->remove(re);
    SAFE_DELETE(re);
    return 1;
}

ACounterShroud * ACounterShroud::clone() const
{
    ACounterShroud * a = NEW ACounterShroud(*this);
    a->re = NULL;
    return a;
}

ACounterShroud::~ACounterShroud()
{
    SAFE_DELETE(re);
    SAFE_DELETE(counter);
}

//track counters placed on a card
ACounterTracker::ACounterTracker(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, string scounter) :
MTGAbility(observer, id, source, target),scounter(scounter)
{
    removed = 0;
}

int ACounterTracker::addToGame()
{
    MTGCardInstance * _target = (MTGCardInstance*)target;
    AbilityFactory af(game);
    Counter * counter = af.parseCounter(scounter, _target, NULL); //(Spell*)source);
    if (!counter)
    {
        return 0;
    }
    if(_target && !removed)
    {
        if(_target->counters->hasCounter(counter->name.c_str(),counter->power,counter->toughness) && _target->counters->hasCounter(counter->name.c_str(),counter->power,counter->toughness)->nb >= counter->nb)
        {
            for(int nb = 0;nb < counter->nb;nb++)
            {
                _target->counters->removeCounter(counter->name.c_str(),counter->power,counter->toughness);
                removed++;
            }
        }
        SAFE_DELETE(counter);
        return MTGAbility::addToGame();
    }
    SAFE_DELETE(counter);
    return 0;
}

int ACounterTracker::destroy()
{
    MTGCardInstance * _target = (MTGCardInstance*)target;
    AbilityFactory af(game);
    Counter * counter = af.parseCounter(scounter, _target, NULL); //(Spell*)source);
    if (!counter)
    {
        return 0;
    }
    if(_target)
    {
        if(removed == counter->nb)
        {
            for(int nb = 0;nb < counter->nb;nb++)
            {
                _target->counters->addCounter(counter->name.c_str(),counter->power,counter->toughness);
            }
        }
    }
    SAFE_DELETE(counter);
    return 1;
}

int ACounterTracker::testDestroy()
{
    if(this->source->isInPlay(game))
        return 0;
    return 1;
}

ACounterTracker * ACounterTracker::clone() const
{
    ACounterTracker * a = NEW ACounterTracker(*this);
    return a;
}

ACounterTracker::~ACounterTracker()
{
}

//removeall counters of a certain type or all.
AARemoveAllCounter::AARemoveAllCounter(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, const char * _name, int power, int toughness,
        int nb,bool all, ManaCost * cost) :
    ActivatedAbility(observer, id, source, cost, 0), nb(nb), power(power), toughness(toughness), name(_name),all(all)
{
    this->target = target;
    menu = "";
}

int AARemoveAllCounter::resolve()
{
    if (!target)
        return 0;

    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (all )
    {
        for(int amount = 0;amount < _target->counters->mCount;amount++)
        {
            while(_target->counters->counters[amount]->nb > 0)
                _target->counters->removeCounter(_target->counters->counters[amount]->name.c_str(),_target->counters->counters[amount]->power,_target->counters->counters[amount]->toughness);

        }
    }
    Counter * targetCounter = NULL;
    if (_target->counters && _target->counters->hasCounter(name.c_str(), power, toughness))
    {
        targetCounter = _target->counters->hasCounter(name.c_str(), power, toughness);
        nb = targetCounter->nb;
    }


    for (int i = 0; i < nb; i++)
    {
        while (_target->next)
            _target = _target->next;
        _target->counters->removeCounter(name.c_str(), power, toughness);
    }

    return nb;
}

const string AARemoveAllCounter::getMenuText()
{
    if (menu.size())
    {
        return menu.c_str();
    }
    char buffer[128];

    if (name.size())
    {
        string s = name;
        menu.append(s.c_str());
    }

    if (power != 0 || toughness != 0)
    {
        sprintf(buffer, " %i/%i", power, toughness);
        menu.append(buffer);
    }

    menu.append(" Counter Removed");
    if (nb != 1)
    {
        sprintf(buffer, ": %i", nb);
        menu.append(buffer);
    }

    return menu.c_str();
}

AARemoveAllCounter * AARemoveAllCounter::clone() const
{
    return NEW AARemoveAllCounter(*this);
}

//remove a single counter of a spefic kind chosen by user 
AARemoveSingleCounter::AARemoveSingleCounter(GameObserver* observer, int id, MTGCardInstance * source, Targetable * target, ManaCost * cost, int nb) :
ActivatedAbility(observer, id, source, cost, 0), nb(nb)
{
    this->GetId();
    allcounters = false;
}
 
int AARemoveSingleCounter::resolve()
{
    if (!target)
        return 0;

    vector<MTGAbility*>pcounters;
    
    Player * pTarget = dynamic_cast<Player *>(target);
    MTGCardInstance * cTarget = dynamic_cast<MTGCardInstance *>(target);

    if(pTarget && pTarget->poisonCount)
    {
        MTGAbility * a = NEW AAAlterPoison(game, game->mLayers->actionLayer()->getMaxId(), source, target, -nb, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    else if(pTarget && pTarget->energyCount)
    {
        MTGAbility * a = NEW AAAlterEnergy(game, game->mLayers->actionLayer()->getMaxId(), source, target, -nb, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    else if(pTarget && pTarget->experienceCount)
    {
        MTGAbility * a = NEW AAAlterExperience(game, game->mLayers->actionLayer()->getMaxId(), source, target, -nb, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    else if (cTarget && cTarget->counters)
    {
        Counters * counters = cTarget->counters;
        for(size_t i = 0; i < counters->counters.size(); ++i)
        {
            Counter * counter = counters->counters[i];
            MTGAbility * a = NEW AACounter(game, game->mLayers->actionLayer()->getMaxId(), source, cTarget, "", counter->name.c_str(), counter->power, counter->toughness, -nb, 0);
            a->oneShot = true;
            pcounters.push_back(a);
        }
    }
    if(pcounters.size())
    {
        if(allcounters)
        {
            for(size_t j = 0; j < pcounters.size(); j++)
            {
                pcounters[j]->resolve();
            }
        }
        else
        {
            MTGAbility * a = NEW MenuAbility(game, this->GetId(), target, source, true, pcounters);
            a->resolve();
        }
    }
    return 1;

}

const string AARemoveSingleCounter::getMenuText()
{
    return "Remove single specific counter";
}

AARemoveSingleCounter * AARemoveSingleCounter::clone() const
{
    return NEW AARemoveSingleCounter(*this);
}

AARemoveSingleCounter::~AARemoveSingleCounter()
{
}

//duplicate counters 
AADuplicateCounters::AADuplicateCounters(GameObserver* observer, int id, MTGCardInstance * source, Targetable * target, ManaCost * cost) :
ActivatedAbility(observer, id, source, cost, 0)
{
    this->GetId();
    allcounters = false;
    single = false;
}
 
int AADuplicateCounters::resolve()
{
    if (!target)
        return 0;

    vector<MTGAbility*>pcounters;
    
    Player * pTarget = dynamic_cast<Player *>(target);
    MTGCardInstance * cTarget = dynamic_cast<MTGCardInstance *>(target);

    if(pTarget && pTarget->poisonCount)
    {
        MTGAbility * a = NULL;
        if(single)
            a = NEW AAAlterPoison(game, game->mLayers->actionLayer()->getMaxId(), source, target, pTarget->poisonCount, NULL);
        else
            a = NEW AAAlterPoison(game, game->mLayers->actionLayer()->getMaxId(), source, target, 1, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    if(pTarget && pTarget->energyCount)
    {
        MTGAbility * a = NULL;
        if(single)
            a = NEW AAAlterEnergy(game, game->mLayers->actionLayer()->getMaxId(), source, target, 1, NULL);
        else
            a = NEW AAAlterEnergy(game, game->mLayers->actionLayer()->getMaxId(), source, target, pTarget->energyCount, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    if(pTarget && pTarget->experienceCount)
    {
        MTGAbility * a = NULL;
        if(single)
            a = NEW AAAlterExperience(game, game->mLayers->actionLayer()->getMaxId(), source, target, 1, NULL);
        else
            a = NEW AAAlterExperience(game, game->mLayers->actionLayer()->getMaxId(), source, target, pTarget->experienceCount, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    if (cTarget && cTarget->counters)
    {
        Counters * counters = cTarget->counters;
        for(size_t i = 0; i < counters->counters.size(); ++i)
        {
            Counter * counter = counters->counters[i];
            MTGAbility * a = NULL;
            if(single)
                a = NEW AACounter(game, game->mLayers->actionLayer()->getMaxId(), source, cTarget, "", counter->name.c_str(), counter->power, counter->toughness, 1, 0);
            else
                a = NEW AACounter(game, game->mLayers->actionLayer()->getMaxId(), source, cTarget, "", counter->name.c_str(), counter->power, counter->toughness, counter->nb, 0);
            a->oneShot = true;
            pcounters.push_back(a);
        }
    }
    if(pcounters.size())
    {
        if(allcounters)
        {
            for(size_t j = 0; j < pcounters.size(); j++)
            {
                pcounters[j]->resolve();
            }
        }
        else
        {
            MTGAbility * a = NEW MenuAbility(game, this->GetId(), target, source, true, pcounters);
            a->resolve();
        }
    }
    return 1;

}

const string AADuplicateCounters::getMenuText()
{
    if(allcounters)
        return "Duplicate all Counters";
    return "Duplicate specific Counters";
}

AADuplicateCounters * AADuplicateCounters::clone() const
{
    return NEW AADuplicateCounters(*this);
}

AADuplicateCounters::~AADuplicateCounters()
{
}

//proliferate a target
AAProliferate::AAProliferate(GameObserver* observer, int id, MTGCardInstance * source, Targetable * target, ManaCost * cost) :
ActivatedAbility(observer, id, source, cost, 0)
{
    this->GetId();
    allcounters = false;
    notrigger = false;
}
 
int AAProliferate::resolve()
{
    if (!target)
        return 0;

    vector<MTGAbility*>pcounters;
    
    Player * pTarget = dynamic_cast<Player *>(target);
    MTGCardInstance * cTarget = dynamic_cast<MTGCardInstance *>(target);

    if(pTarget && pTarget->poisonCount)
    {
        MTGAbility * a = NEW AAAlterPoison(game, game->mLayers->actionLayer()->getMaxId(), source, target, 1, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    if(pTarget && pTarget->energyCount)
    {
        MTGAbility * a = NEW AAAlterEnergy(game, game->mLayers->actionLayer()->getMaxId(), source, target, 1, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    if(pTarget && pTarget->experienceCount)
    {
        MTGAbility * a = NEW AAAlterExperience(game, game->mLayers->actionLayer()->getMaxId(), source, target, 1, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    if (cTarget && cTarget->counters)
    {
        Counters * counters = cTarget->counters;
        for(size_t i = 0; i < counters->counters.size(); ++i)
        {
            Counter * counter = counters->counters[i];
            MTGAbility * a = NEW AACounter(game, game->mLayers->actionLayer()->getMaxId(), source, cTarget, "", counter->name.c_str(), counter->power, counter->toughness, 1, 0);
            a->oneShot = true;
            pcounters.push_back(a);
        }
    }
    if(pcounters.size())
    {
        if(allcounters)
        {
             for(size_t j = 0; j < pcounters.size(); j++)
             {
                 pcounters[j]->resolve();
             }
        }
        else
        {
            MTGAbility * a = NEW MenuAbility(game, this->GetId(), target, source,false,pcounters);
            a->resolve();
        }
    }
    if(!notrigger){
        WEventplayerProliferated * e = NEW WEventplayerProliferated(source->controller());
        e->source = source;
        game->receiveEvent(e);
    }
    return 1;

}

const string AAProliferate::getMenuText()
{
    if(allcounters)
        return "Add Any Counters";
    return "Proliferate";
}

AAProliferate * AAProliferate::clone() const
{
    return NEW AAProliferate(*this);
}

AAProliferate::~AAProliferate()
{
}
//
//choosing a type or color or name
GenericChooseTypeColorName::GenericChooseTypeColorName(GameObserver* observer, int id, MTGCardInstance * source, Targetable *, string _toAdd, bool chooseColor, bool chooseName, bool chooseOppName, bool nonwall, bool nonbasicland, bool nonland, ManaCost * cost) :
ActivatedAbility(observer, id, source, cost, 0), baseAbility(_toAdd),chooseColor(chooseColor),chooseName(chooseName),chooseOppName(chooseOppName),ANonWall(nonwall),ANonBasicLand(nonbasicland),ANonLand(nonland)
{
    this->GetId();
    setColor = NULL;
    setName = NULL;
}
int GenericChooseTypeColorName::resolve()
{
    if (!target)
        return 0;
    vector<MTGAbility*>selection;
    if(chooseColor)
    {
        for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
        {
            setColor = NEW AASetColorChosen(game, game->mLayers->actionLayer()->getMaxId(), source,(MTGCardInstance*)target, i, baseAbility);
            MTGAbility * set = setColor->clone();
            set->oneShot = true;
            selection.push_back(set);
            SAFE_DELETE(setColor);
        }
    }
    else if(chooseName || chooseOppName)
    {
        vector<string> names;
        Player* p = (chooseName)?source->controller():source->controller()->opponent();
        MTGGameZone * zones[] = { p->game->inPlay, p->game->graveyard, p->game->hand, p->game->library, p->game->stack, p->game->exile, p->game->commandzone, p->game->sideboard, p->game->reveal };
        for (int k = 0; k < 9; k++){
            MTGGameZone * zone = zones[k];
            for (int j = zone->nb_cards - 1; j >= 0; --j){
                if ((!ANonBasicLand || (!zone->cards[j]->hasType(Subtypes::TYPE_BASIC) && !zone->cards[j]->hasType(Subtypes::TYPE_LAND))) && (!ANonLand || !zone->cards[j]->hasType(Subtypes::TYPE_LAND))){
                     bool added = false;
                     for (int i = names.size() - 1; i >= 0; --i)
                         if(names[i] == zone->cards[j]->name)
                             added = true;
                     if(!added)
                         names.push_back(zone->cards[j]->name);
                }
            }
        }
        for (size_t i = 0; i < names.size(); ++i){
            string menu = names[i];
            setName = NEW AASetNameChosen(game, game->mLayers->actionLayer()->getMaxId(), source, (MTGCardInstance*)target, names[i], menu, baseAbility);
            MTGAbility * set = setName->clone();
            set->oneShot = true;
            selection.push_back(set);
            SAFE_DELETE(setName);
        }
    }
    else
    {
        vector<string> values = MTGAllCards::getCreatureValuesById();
        for (size_t i = 0; i < values.size(); ++i)
        {
            string menu = values[i];
            if (!ANonWall || (menu != "wall" && menu != "Wall"))
            {
                setType = NEW AASetTypeChosen(game, game->mLayers->actionLayer()->getMaxId(), source,(MTGCardInstance*)target, i,menu,baseAbility);
                MTGAbility * set = setType->clone();
                set->oneShot = true;
                selection.push_back(set);
                SAFE_DELETE(setType);
            }
        }
    }

    if(selection.size())
    {
        MTGAbility * a1 = NEW MenuAbility(game, this->GetId(), target, source,true,selection);
        game->mLayers->actionLayer()->currentActionCard = (MTGCardInstance *)target;
        a1->resolve();
    }
    return 1;

}

const string GenericChooseTypeColorName::getMenuText()
{
    if(chooseColor)
        return "Choose a color";
    if(chooseName || chooseOppName)
        return "Choose a name";
    else
        return "Choose a type";
}

GenericChooseTypeColorName * GenericChooseTypeColorName::clone() const
{
    GenericChooseTypeColorName * a = NEW GenericChooseTypeColorName(*this);
    return a;
}

GenericChooseTypeColorName::~GenericChooseTypeColorName()
{
}

//set color choosen
 AASetColorChosen::AASetColorChosen(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target,int _color , string toAlter):
    InstantAbility(observer, id, source),color(_color), abilityToAlter(toAlter)
{
    this->target = _target;
    abilityAltered = NULL;
}
int AASetColorChosen::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    _target->chooseacolor = color;

    if(abilityToAlter.size())
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityToAlter, 0, NULL, _target);
        if(!abilityAltered)
            return 0;
        abilityAltered->canBeInterrupted = false;
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->target = _target;
            MayAbility * dontAdd = dynamic_cast<MayAbility*>(abilityAltered);
            if (!dontAdd)
            {
                _target->cardsAbilities.push_back(abilityAltered);
                for(unsigned int j = 0;j < _target->cardsAbilities.size();++j)
                {
                    if(_target->cardsAbilities[j] == this)
                        _target->cardsAbilities.erase(_target->cardsAbilities.begin() + j);
                }
            }
            abilityAltered->addToGame();
        }
        _target->skipDamageTestOnce = true;//some cards rely on this ability updating before damage test are run. otherwise they die before toughnes bonus applies.
    }
    return 1;
}

const string AASetColorChosen::getMenuText()
{
    return Constants::MTGColorStrings[color];
}

AASetColorChosen * AASetColorChosen::clone() const
{
    return NEW AASetColorChosen(*this);
}

AASetColorChosen::~AASetColorChosen()
{
}

//set type choosen
 AASetTypeChosen::AASetTypeChosen(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target,int _type ,string _menu,string toAlter):
    InstantAbility(observer, id, source),type(_type), abilityToAlter(toAlter), menutext(_menu)
{
    this->target = _target;
    abilityAltered = NULL;
}
int AASetTypeChosen::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    string typeChoosen = menutext;
    _target->chooseasubtype = typeChoosen;

    if(abilityToAlter.size())
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityToAlter, 0, NULL, _target);
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->target = _target;
            MayAbility * dontAdd = dynamic_cast<MayAbility*>(abilityAltered);
            if (!dontAdd)
            {
                _target->cardsAbilities.push_back(abilityAltered);
                for(unsigned int j = 0;j < _target->cardsAbilities.size();++j)
                {
                    if(_target->cardsAbilities[j] == this)
                        _target->cardsAbilities.erase(_target->cardsAbilities.begin() + j);
                }
            }

            abilityAltered->addToGame();
        }
         _target->skipDamageTestOnce = true;//some cards rely on this ability updating before damage test are run. otherwise they die before toughnes bonus applies.
    }
    return 1;
}

const string AASetTypeChosen::getMenuText()
{
    return menutext.c_str();
}

AASetTypeChosen * AASetTypeChosen::clone() const
{
    return NEW AASetTypeChosen(*this);
}

AASetTypeChosen::~AASetTypeChosen()
{
}

//set name choosen
 AASetNameChosen::AASetNameChosen(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target,string _name ,string _menu,string toAlter):
    InstantAbility(observer, id, source),name(_name), abilityToAlter(toAlter), menutext(_menu)
{
    this->target = _target;
    abilityAltered = NULL;
}
int AASetNameChosen::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    string nameChoosen = menutext;
    _target->chooseaname = nameChoosen;
    _target->controller()->lastChosenName = nameChoosen;

    if(abilityToAlter.size())
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityToAlter, 0, NULL, _target);
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->target = _target;
            MayAbility * dontAdd = dynamic_cast<MayAbility*>(abilityAltered);
            if (!dontAdd)
            {
                _target->cardsAbilities.push_back(abilityAltered);
                for(unsigned int j = 0;j < _target->cardsAbilities.size();++j)
                {
                    if(_target->cardsAbilities[j] == this)
                        _target->cardsAbilities.erase(_target->cardsAbilities.begin() + j);
                }
            }

            abilityAltered->addToGame();
        }
         _target->skipDamageTestOnce = true;//some cards rely on this ability updating before damage test are run. otherwise they die before toughnes bonus applies.
    }
    return 1;
}

const string AASetNameChosen::getMenuText()
{
    return menutext.c_str();
}

AASetNameChosen * AASetNameChosen::clone() const
{
    return NEW AASetNameChosen(*this);
}

AASetNameChosen::~AASetNameChosen()
{
}

//
//choosing flip coin
GenericFlipACoin::GenericFlipACoin(GameObserver* observer, int id, MTGCardInstance * source, Targetable *,string _toAdd, ManaCost * cost, int userchoice) :
ActivatedAbility(observer, id, source, cost, 0), baseAbility(_toAdd), userchoice(userchoice)
{
    this->GetId();
    setCoin = NULL;
}

int GenericFlipACoin::resolve()
{
    if (!target)
        return 0;
    vector<MTGAbility*>selection;
    if(userchoice > 0 && userchoice <= 2){
        setCoin = NEW AASetCoin(game, game->mLayers->actionLayer()->getMaxId(), source,(MTGCardInstance*)target, userchoice, baseAbility);
        MTGAbility * set = setCoin->clone();
        set->oneShot = true;
        game->mLayers->actionLayer()->currentActionCard = (MTGCardInstance *)target;
        set->resolve();
        SAFE_DELETE(setCoin);
    } else{
        for (int i = 1; i <=2; ++i)
        {
            setCoin = NEW AASetCoin(game, game->mLayers->actionLayer()->getMaxId(), source,(MTGCardInstance*)target, i, baseAbility);
            MTGAbility * set = setCoin->clone();
            set->oneShot = true;
            selection.push_back(set);
            SAFE_DELETE(setCoin);
        }
    }
    if(selection.size())
    {
        MTGAbility * a1 = NEW MenuAbility(game, this->GetId(), target, source,false,selection);
        game->mLayers->actionLayer()->currentActionCard = (MTGCardInstance *)target;
        a1->resolve();
    }
    return 1;

}

const string GenericFlipACoin::getMenuText()
{
return "Flip A Coin";
}

GenericFlipACoin * GenericFlipACoin::clone() const
{
    GenericFlipACoin * a = NEW GenericFlipACoin(*this);
    return a;
}

GenericFlipACoin::~GenericFlipACoin()
{
}

//set coin result
 AASetCoin::AASetCoin(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target, int _side, string toAlter):
    InstantAbility(observer, id, source),side(_side), abilityToAlter(toAlter)
{
    this->target = _target;
    abilityAltered = NULL;
}

int AASetCoin::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    _target->coinSide = side;

    int flip = 1 + game->getRandomGenerator()->random() % 2;
    _target->lastFlipResult = flip;
    WEvent * e = NEW WEventCardFlipCoin(_target, source->controller()->getDisplayName());
    game->receiveEvent(e);
    vector<string>Win = parseBetween(abilityToAlter,"winability "," winabilityend");
    if(Win.size())
    {
        abilityWin = Win[1];
    }
    vector<string>Lose = parseBetween(abilityToAlter,"loseability "," loseabilityend");
    if(Lose.size())
    {
        abilityLose = Lose[1];
    }

    if(abilityWin.size() && flip == side)
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityWin, 0, NULL, _target);
        abilityAltered->canBeInterrupted = false;
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->addToGame();
        }
        MTGAbility * message = NEW MTGEventText(game,this->GetId(), source, "You Won The Flip");
        message->oneShot = true;
        message->addToGame();
    }
    else if(abilityWin.size() && !abilityLose.size())
    {
        MTGAbility * message = NEW MTGEventText(game,this->GetId(), source, "You Lost The Flip");
        message->oneShot = true;
        message->addToGame();
    }
    else if(abilityLose.size() && flip != side)
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityLose, 0, NULL, _target);
        abilityAltered->canBeInterrupted = false;
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->addToGame();
        }
        MTGAbility * message = NEW MTGEventText(game,this->GetId(), source, "You Lost The Flip");
        message->oneShot = true;
        message->addToGame();
    }
    else if(abilityLose.size())
    {
        MTGAbility * message = NEW MTGEventText(game,this->GetId(), source, "You Won The Flip");
        message->oneShot = true;
        message->addToGame();
    }
    _target->skipDamageTestOnce = true;
    return 1;
}

const string AASetCoin::getMenuText()
{
    if(side == 1)
        return "Tails";
    return "Heads";
}

AASetCoin * AASetCoin::clone() const
{
    return NEW AASetCoin(*this);
}

AASetCoin::~AASetCoin()
{
}

//
//rolling a generic die
GenericRollDie::GenericRollDie(GameObserver* observer, int id, MTGCardInstance * source, Targetable *, string _toAdd, ManaCost * cost, int userchoice, int diefaces) :
ActivatedAbility(observer, id, source, cost, 0), baseAbility(_toAdd), userchoice(userchoice), diefaces(diefaces)
{
    this->GetId();
    setDie = NULL;
}

int GenericRollDie::resolve()
{
    if (!target)
        return 0;
    vector<MTGAbility*>selection;
    if(userchoice > 0 && userchoice <= diefaces){
        setDie = NEW AASetDie(game, game->mLayers->actionLayer()->getMaxId(), source,(MTGCardInstance*)target, userchoice, diefaces, baseAbility);
        MTGAbility * set = setDie->clone();
        set->oneShot = true;
        game->mLayers->actionLayer()->currentActionCard = (MTGCardInstance *)target;
        set->resolve();
        SAFE_DELETE(setDie);
    } else{
        for (int i = 1; i <= diefaces; ++i)
        {
            setDie = NEW AASetDie(game, game->mLayers->actionLayer()->getMaxId(), source,(MTGCardInstance*)target, i, diefaces, baseAbility);
            MTGAbility * set = setDie->clone();
            set->oneShot = true;
            selection.push_back(set);
            SAFE_DELETE(setDie);
        }
    }
    if(selection.size() > 1)
    {
        MTGAbility * a1 = NEW MenuAbility(game, this->GetId(), target, source, false, selection);
        game->mLayers->actionLayer()->currentActionCard = (MTGCardInstance *)target;
        a1->resolve();
    }
    return 1;

}

const string GenericRollDie::getMenuText()
{
    std::stringstream msg;
    msg << "Roll a " << diefaces << " faced Die";
    return msg.str();
}

GenericRollDie * GenericRollDie::clone() const
{
    GenericRollDie * a = NEW GenericRollDie(*this);
    return a;
}

GenericRollDie::~GenericRollDie()
{
}

//set die result
 AASetDie::AASetDie(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target, int _side, int _diefaces, string toAlter):
    InstantAbility(observer, id, source), side(_side), diefaces(_diefaces), abilityToAlter(toAlter)
{
    this->target = _target;
    abilityAltered = NULL;
}

int AASetDie::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    _target->dieSide = side;

    int roll = 1 + game->getRandomGenerator()->random() % diefaces;
    _target->lastRollResult = roll;
    _target->dieNumFaces = diefaces;
    WEvent * e = NEW WEventCardRollDie(_target, source->controller()->getDisplayName());
    game->receiveEvent(e);
    vector<string>Win = parseBetween(abilityToAlter, "winability ", " winabilityend");
    if(Win.size())
    {
        abilityWin = Win[1];
    }
    vector<string>Lose = parseBetween(abilityToAlter, "loseability ", " loseabilityend");
    if(Lose.size())
    {
        abilityLose = Lose[1];
    }

    std::stringstream msg;
    if(abilityWin.size() && roll == side)
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityWin, 0, NULL, _target);
        abilityAltered->canBeInterrupted = false;
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->addToGame();
        }
        msg << "Result is: " << roll << ". You Won The Die Roll";
        MTGAbility * message = NEW MTGEventText(game, this->GetId(), source, msg.str());
        message->oneShot = true;
        message->addToGame();
    }
    else if(abilityWin.size() && !abilityLose.size())
    {
        msg << "Result is: " << roll << ". You Lost The Die Roll";
        MTGAbility * message = NEW MTGEventText(game, this->GetId(), source, msg.str());
        message->oneShot = true;
        message->addToGame();
    }
    else if(abilityLose.size() && roll != side)
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityLose, 0, NULL, _target);
        abilityAltered->canBeInterrupted = false;
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->addToGame();
        }
        msg << "Result is: " << roll << ". You Lost The Die Roll";
        MTGAbility * message = NEW MTGEventText(game, this->GetId(), source, msg.str());
        message->oneShot = true;
        message->addToGame();
    }
    else if(abilityLose.size())
    {
        msg << "Result is: " << roll << ". You Won The Die Roll";
        MTGAbility * message = NEW MTGEventText(game, this->GetId(), source, msg.str());
        message->oneShot = true;
        message->addToGame();
    }
    _target->skipDamageTestOnce = true;
    return 1;
}

const string AASetDie::getMenuText()
{
    std::stringstream msg;
    msg << "Your choice is: " << side;
    return msg.str();
}

AASetDie * AASetDie::clone() const
{
    return NEW AASetDie(*this);
}

AASetDie::~AASetDie()
{
}

//paying for an ability as an effect but as a cost
GenericPaidAbility::GenericPaidAbility(GameObserver* observer, int id, MTGCardInstance * source,
    Targetable * target, string _newName, string _castRestriction, string mayCost, string _toAdd, bool asAlternate, ManaCost * cost) :
ActivatedAbility(observer, id, source, cost, 0),
    newName(_newName), restrictions(_castRestriction), baseCost(mayCost), baseAbilityStr(_toAdd), asAlternate(asAlternate)
{
    this->GetId();
    baseAbility = NULL;
    optionalCost = NULL;
}

int GenericPaidAbility::resolve()
{
    if (!target)
        return 0;

    if (restrictions.size())
    {
        AbilityFactory af(game);
        int checkCond = af.parseCastRestrictions(source,source->controller(),restrictions);
        if(!checkCond)
        {
            return 0;
        }
    }
    AbilityFactory Af(game);
    vector<string> baseAbilityStrSplit = split(baseAbilityStr,'?');
    vector<MTGAbility*> selection;
    MTGAbility * nomenuAbility = NULL;
    bool nomenu = false;
    if (baseAbilityStrSplit.size() > 1)
    {
        baseAbility = Af.parseMagicLine(baseAbilityStrSplit[0], this->GetId(), NULL, source);
        baseAbility->target = target;
        optionalCost =  ManaCost::parseManaCost(baseCost, NULL, source);

        /*// hacky way to produce better MenuText
        AAFakeAbility* isFake = dynamic_cast< AAFakeAbility* >( baseAbility );
        size_t findPayN = isFake->named.find(" {value} mana");
        if (isFake && findPayN != string::npos) {
            stringstream parseN;
            parseN << optionalCost->getCost(Constants::MTG_COLOR_ARTIFACT);
            isFake->named.replace(findPayN + 1, 7, parseN.str());
        }//commented out, it crashes cards with recover ability*/

        MTGAbility * set = baseAbility->clone();
        set->oneShot = true;
        selection.push_back(set);
        SAFE_DELETE(baseAbility);

        baseAbility = Af.parseMagicLine(baseAbilityStrSplit[1], this->GetId(), NULL, source);
        baseAbility->target = target;
        set = baseAbility->clone();
        set->oneShot = true;
        selection.push_back(set);
    }
    else
    {
        //dangerous code below, parse a string line that might not exist. baseAbilityStrSplit[0]
        //you either have a string and do stuff, or dont and leave the ability
        //not fixing this since its been heavily modified from the orginal implementation.
        nomenu = true;
        baseAbility = Af.parseMagicLine(baseAbilityStrSplit[0], this->GetId(), NULL, source);
        baseAbility->target = target;
        optionalCost =  ManaCost::parseManaCost(baseCost, NULL, source);
        MTGAbility * set = baseAbility->clone();
        nomenuAbility = baseAbility->clone();
        set->oneShot = true;
        selection.push_back(set);
    }

    if (selection.size())
    {
        bool must = baseAbilityStrSplit.size() > 1 ? true : false;
        //todo get increased - reduced cost if asAlternate cost to cast using castcard
        if(asAlternate)
        {
            must = true;
            //cost increase - reduce + trinisphere effect ability todo...
            optionalCost = ((MTGCardInstance *)target)->computeNewCost(((MTGCardInstance *)target),optionalCost,optionalCost);
            if(optionalCost->extraCosts)
            {
                for(unsigned int i = 0; i < optionalCost->extraCosts->costs.size();i++)
                    optionalCost->extraCosts->costs[i]->setSource(((MTGCardInstance *)target));
            }
        }
        if (source && source->previous && source->basicAbilities[(int)Constants::MADNESS])
        {
            must = true;
            optionalCost = source->computeNewCost(source->previous,optionalCost,optionalCost);
            if(optionalCost->extraCosts)
            {
                for(unsigned int i = 0; i < optionalCost->extraCosts->costs.size();i++)
                    optionalCost->extraCosts->costs[i]->setSource(source);
            }
        }
        if(asAlternate && nomenu && optionalCost->getConvertedCost() < 1)
            nomenuAbility->resolve();
        else
        {
            MenuAbility * a1 = NEW MenuAbility(game, this->GetId(), target, source, must, selection, NULL, newName);
            a1->optionalCosts.push_back(NEW ManaCost(optionalCost));
            game->mLayers->actionLayer()->currentActionCard = (MTGCardInstance *)target;
            a1->resolve();
        }
    }
    return 1;
}

const string GenericPaidAbility::getMenuText()
{
    if (newName.size())
        return newName.c_str();
    return "Pay For Effect";
}

GenericPaidAbility * GenericPaidAbility::clone() const
{
    GenericPaidAbility * a = NEW GenericPaidAbility(*this);
    return a;
}

GenericPaidAbility::~GenericPaidAbility()
{
    SAFE_DELETE(optionalCost);
    SAFE_DELETE(baseAbility);
}

//saves a listed mana type.
AManaPoolSaver::AManaPoolSaver(GameObserver* observer, int id, MTGCardInstance * source,string color, bool otherPlayer, bool removePool) :
MTGAbility(observer, id, source),Color(color),OtherPlayer(otherPlayer),RemovePool(removePool)
{
}

int AManaPoolSaver::addToGame()
{
    int colorInt = Constants::GetColorStringIndex(Color.c_str());
    if(OtherPlayer){
        if(RemovePool)
            source->controller()->opponent()->poolDoesntEmpty->remove(colorInt,1);
        else
            source->controller()->opponent()->poolDoesntEmpty->add(colorInt,1);
    }
    else {
        if(RemovePool)
            source->controller()->poolDoesntEmpty->remove(colorInt,1);
        else
            source->controller()->poolDoesntEmpty->add(colorInt,1);
    }
    return 1;
}

int AManaPoolSaver::destroy()
{
    int colorInt = Constants::GetColorStringIndex(Color.c_str());
    if(OtherPlayer)
        source->controller()->opponent()->poolDoesntEmpty->remove(colorInt,1);
    else
        source->controller()->poolDoesntEmpty->remove(colorInt,1);
    return 1;
}

AManaPoolSaver * AManaPoolSaver::clone() const
{
    AManaPoolSaver * a = NEW AManaPoolSaver(*this);
    return a;
}

AManaPoolSaver::~AManaPoolSaver()
{
}

//replace drawing a card with activation of an ability
ADrawReplacer::ADrawReplacer(GameObserver* observer, int id, MTGCardInstance * source, MTGAbility * replace, bool otherPlayer) :
MTGAbility(observer, id, source),re(NULL),replacer(replace),OtherPlayer(otherPlayer)
{
}

int ADrawReplacer::addToGame()
{
    SAFE_DELETE(re);
    if(OtherPlayer)
        re = NEW REDrawReplacement(this,source->controller()->opponent(),replacer);
    else
        re = NEW REDrawReplacement(this,source->controller(),replacer);
    if (re)
    {
        game->replacementEffects->add(re);
        return MTGAbility::addToGame();
    }
    return 0;
}

int ADrawReplacer::destroy()
{
    game->replacementEffects->remove(re);
    SAFE_DELETE(re);
    return 1;
}

ADrawReplacer * ADrawReplacer::clone() const
{
    ADrawReplacer * a = NEW ADrawReplacer(*this);
    a->re = NULL;
    return a;
}

ADrawReplacer::~ADrawReplacer()
{
    SAFE_DELETE(re);
    SAFE_DELETE(replacer);
}
//Reset Damage on creatures
 AAResetDamage::AAResetDamage(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target, ManaCost * cost):
    ActivatedAbility(observer, id, source, cost, 0)
{
    this->target = _target;
}
int AAResetDamage::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    if(!_target->has(Constants::NODAMAGEREMOVED)){ // Added to avoid damage is removed from a card (e.g. "Patient Zero").
        if (!_target->isCreature() && _target->hasType(Subtypes::TYPE_PLANESWALKER)){ // Fix life calculation for planeswalker damage.
            if (_target->counters && _target->counters->hasCounter("loyalty", 0, 0)) {
                _target->life = _target->counters->hasCounter("loyalty", 0, 0)->nb;
            }
        } else if (!_target->isCreature() && _target->hasType(Subtypes::TYPE_BATTLE)){ // Fix life calculation for battle damage.
            if (_target->counters && _target->counters->hasCounter("defense", 0, 0)) {
                _target->life = _target->counters->hasCounter("defense", 0, 0)->nb;
            }
        } else
            _target->life = _target->toughness;
    }
    return 1;
}

const string AAResetDamage::getMenuText()
{
    return "Reset Damages";
}

AAResetDamage * AAResetDamage::clone() const
{
    return NEW AAResetDamage(*this);
}

//ability that resolves to do nothing.
 AAFakeAbility::AAFakeAbility(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target, string _named,ManaCost * cost):
    ActivatedAbility(observer, id, source, cost, 0),named(_named)
{
    this->target = _target;
}
int AAFakeAbility::resolve()
{
    return 1;
}

const string AAFakeAbility::getMenuText()
{
    if(named.size())
        return named.c_str();
    return "Ability";
}

AAFakeAbility * AAFakeAbility::clone() const
{
    return NEW AAFakeAbility(*this);
}

//EPIC
 AAEPIC::AAEPIC(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target, string _named,ManaCost * cost, bool _ffield):
    ActivatedAbility(observer, id, source, cost, 0),named(_named),FField(_ffield)
{
    this->target = _target;
}
int AAEPIC::resolve()
{  
    MTGCardInstance * _target =  (MTGCardInstance *)target;
    if(FField)
        _target->controller()->forcefield = 1;
    else
        _target->controller()->epic = 1;
    return 1;
}

const string AAEPIC::getMenuText()
{
    if(named.size())
        return named.c_str();
    return "EPIC";
}

AAEPIC * AAEPIC::clone() const
{
    return NEW AAEPIC(*this);
}

// Fizzler
AAFizzler::AAFizzler(GameObserver* observer, int _id, MTGCardInstance * card, Spell * _target, ManaCost * _cost) :
ActivatedAbility(observer, _id, card, _cost, 0)
{
    aType = MTGAbility::STANDARD_FIZZLER;
    target = _target;

    // by default we put the spell to graveyard after fizzling
    fizzleMode = ActionStack::PUT_IN_GRAVEARD;
    // by default fizzle is not used to put spell somewhere
    spellMover = false;
}

int AAFizzler::resolve()
{
    ActionStack * stack = game->mLayers->stackLayer();
    //the next section helps Ai correctly recieve its targets for this effect
    if (!target && source->target)
    {
        //ai is casting a spell from its hand to fizzle.
        target = stack->getActionElementFromCard(source->target);
    }
    else if(MTGCardInstance * cTarget = dynamic_cast<MTGCardInstance *>(target))
    {
        //ai targeted using an ability on a card to fizzle.
        target = stack->getActionElementFromCard(cTarget);
    }
    Spell * sTarget = (Spell *) target;
    MTGCardInstance* sCard = NULL;
    if (sTarget)
        sCard = sTarget->source;
    if (!sCard || !sTarget || (sCard->has(Constants::NOFIZZLE) && !spellMover))
        return 0;
    if (sCard->has(Constants::NOFIZZLEALTERNATIVE) && (sCard->alternateCostPaid[ManaCost::MANA_PAID_WITH_ALTERNATIVE] && !spellMover)) // No fizzle if card has been paid with alternative cost.
        return 0;
    if (source->alias == 111057 && sTarget)//Draining Whelk
    {
        for (int j = sTarget->cost->getConvertedCost(); j > 0; j--)
        {
            source->counters->addCounter(1,1);
        }
    }
    stack->Fizzle(sTarget, source, fizzleMode);
    if(!source->storedCard)
        source->storedCard = sCard; // Store the fizzled card to retrive target information later (e.g. manacost for Reinterpret)
    return 1;
}

const string AAFizzler::getMenuText()
{
    return "Fizzle";
}

AAFizzler* AAFizzler::clone() const
{
    return NEW AAFizzler(*this);
}

// BanishCard implementations
// Bury

AABuryCard::AABuryCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
   ActivatedAbility(observer, _id, _source)
{
    target = _target;
    andAbility = NULL;
}

int AABuryCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be buried, they will follow the fate of top-card
        //Bury (Obsolete)
        //A term that meant “put [a permanent] into its owner’s graveyard.”
        //In general, cards that were printed with the term “bury” have received errata 
        //in the Oracle card reference to read, “Destroy [a permanent]. It can’t be regenerated,” 
        //or “Sacrifice [a permanent].”
        //_target->bury();
        _target->destroyNoRegen();//so totem armor will take effect on wrath effects since totem armor is not regeneration..
        while(_target->next)
            _target = _target->next;
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AABuryCard::getMenuText()
{
    if(menu.size())
        return menu.c_str();
    return "Bury";
}

AABuryCard * AABuryCard::clone() const
{
    AABuryCard * a = NEW AABuryCard(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AABuryCard::~AABuryCard()
{
    SAFE_DELETE(andAbility);
}

// Destroy

AADestroyCard::AADestroyCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
   ActivatedAbility(observer, _id, _source)
{
    target = _target;
    andAbility = NULL;
}

int AADestroyCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be destroyed, they will follow the fate of top-card
        _target->destroy();
        while(_target->next)
            _target = _target->next;
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AADestroyCard::getMenuText()
{
    return "Destroy";
}

AADestroyCard * AADestroyCard::clone() const
{
    AADestroyCard * a = NEW AADestroyCard(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;

}
AADestroyCard::~AADestroyCard()
{
    SAFE_DELETE(andAbility);
}
// Sacrifice
AASacrificeCard::AASacrificeCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
   ActivatedAbility(observer, _id, _source)
{
    target = _target;
    andAbility = NULL;
    isExploited = false;
}

int AASacrificeCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->has(Constants::CANTBESACRIFIED)) return 0; // The card cannot be sacrified (e.g. "Hithlain Rope")
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be sacrificed or exploited, they will follow the fate of top-card
        Player * p = _target->controller();
        MTGCardInstance * beforeCard = _target;
        p->game->putInGraveyard(_target);
        while(_target->next)
            _target = _target->next;
        WEvent * e = NEW WEventCardSacrifice(beforeCard, _target);
        game->receiveEvent(e);
        if(isExploited){
            WEvent * e = NEW WEventCardExploited(beforeCard, _target);
            game->receiveEvent(e);
        }
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AASacrificeCard::getMenuText()
{
    if(isExploited)
        return "Exploit";
    else
        return "Sacrifice";
}

AASacrificeCard * AASacrificeCard::clone() const
{
    AASacrificeCard * a = NEW AASacrificeCard(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}
AASacrificeCard::~AASacrificeCard()
{
    SAFE_DELETE(andAbility);
}

// Discard
AADiscardCard::AADiscardCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
   ActivatedAbility(observer, _id, _source)
{
    target = _target;
    andAbility = NULL;
}

int AADiscardCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        Player * p = _target->controller();
        WEvent * e = NEW WEventCardDiscard(_target);
        game->receiveEvent(e);
        if(this->source->storedSourceCard)
            _target->discarderOwner = this->source->storedSourceCard->controller();
        else
            _target->discarderOwner = this->source->controller();
        p->game->putInGraveyard(_target);
        while(_target->next)
            _target = _target->next;
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
    }
    return 0;
}

const string AADiscardCard::getMenuText()
{
    return "Discard";
}

AADiscardCard * AADiscardCard::clone() const
{
    AADiscardCard * a = NEW AADiscardCard(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AADiscardCard::~AADiscardCard()
{
    SAFE_DELETE(andAbility);
}

//Draw
AADrawer::AADrawer(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, string nbcardsStr,
        int who, bool noreplace) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who), nbcardsStr(nbcardsStr),noReplace(noreplace)
{
    aType = MTGAbility::STANDARD_DRAW;
    andAbility = NULL;
}

int AADrawer::resolve()
{
    Player * player = getPlayerFromTarget(getTarget());

    if (player)
    {
        WParsedInt numCards(nbcardsStr, NULL, source);
        WEvent * e = NEW WEventDraw(player, numCards.getValue(),this);
        if(!noReplace)
        e = game->replacementEffects->replace(e);
        if(e)
        {
            game->mLayers->stackLayer()->addDraw(player, numCards.getValue());
            game->mLayers->stackLayer()->resolve();
            for(int i = numCards.getValue(); i > 0;i--)
            {
                player->drawCounter += 1;
                if ((game->turn < 1) && game->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN
                && game->currentPlayer->game->inPlay->nb_cards == 0 && game->currentPlayer->game->graveyard->nb_cards == 0
                && game->currentPlayer->game->exile->nb_cards == 0 && game->currentlyActing() == (Player*)game->currentPlayer) //1st Play Check
                {
                    game->currentPlayer->drawCounter = 0;//Reset drawCounter for pre-game draw
                }
                WEvent * e = NEW WEventcardDraw(player, 1);
                game->receiveEvent(e);
            }
        }
        SAFE_DELETE(e);
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = source;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
    }
    return 1;
}

int AADrawer::getNumCards()
{
    WParsedInt numCards(nbcardsStr, NULL, source);
    return numCards.getValue();
}

const string AADrawer::getMenuText()
{
    if(nbcardsStr.size())
    {
        WParsedInt parsedNum(nbcardsStr, NULL, source);
        return _("Draw " + parsedNum.getStringValue()).c_str();
    }
    return "Draw";
}

AADrawer * AADrawer::clone() const
{
    AADrawer * a = NEW AADrawer(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AADrawer::~AADrawer()
{
    SAFE_DELETE(andAbility);
}

// AAFrozen: Prevent a card from untapping during next untap phase
AAFrozen::AAFrozen(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, bool tap, ManaCost * _cost) :
ActivatedAbility(observer, id, card, _cost, 0)
{
    target = _target;
    freeze = tap;
    andAbility = NULL;
}

int AAFrozen::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be frozen, they will follow the fate of top-card
        while (_target->next)
            _target = _target->next; //This is for cards such as rampant growth
        if (freeze)
        {
            _target->tap();//easier to manage for cards that allow you to tap and also freeze.
        }
        _target->frozen += 1;
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
    }
    return 1;
}

const string AAFrozen::getMenuText()
{
    return "Freeze";
}

AAFrozen * AAFrozen::clone() const
{
    AAFrozen * a = NEW AAFrozen(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AAFrozen::~AAFrozen()
{
    SAFE_DELETE(andAbility);
}

// chose a new target for an aura or enchantment and equip it note: VERY basic right now.
AANewTarget::AANewTarget(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost, bool retarget, bool reequip, bool newhook, int mutation, bool fromplay, bool untap) :
ActivatedAbility(observer, id, card, _cost, 0), retarget(retarget), reequip(reequip), newhook(newhook), mutation(mutation), fromplay(fromplay), untap(untap)
{
    target = _target;
}

int AANewTarget::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(retarget)
    {
        _target = source;
        source = (MTGCardInstance *) target;
    }
    if (_target && !reequip && !mutation)
    {
        while (_target->next)
            _target = _target->next;
        if(!fromplay){
            _target->controller()->game->putInZone(_target, _target->currentZone, _target->owner->game->exile);
            _target = _target->next;
        }
        MTGCardInstance * refreshed = source->controller()->game->putInZone(_target, _target->currentZone, source->controller()->game->battlefield);
        Spell * reUp = NEW Spell(game, refreshed);
        if(reUp->source->hasSubtype(Subtypes::TYPE_AURA))
        {
            reUp->source->target = source;
            reUp->resolve();
            if(reUp->source->spellTargetType == "") reUp->source->spellTargetType = "creature"; // Fix to prevent flipped auras go to graveyard.
        }
        if(_target->hasSubtype(Subtypes::TYPE_EQUIPMENT))
        {
            reUp->resolve();
            for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
            {
                MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
                AEquip * eq = dynamic_cast<AEquip*> (a);
                if (eq && eq->source == reUp->source)
                {
                    ((AEquip*)a)->unequip();
                    ((AEquip*)a)->equip(source);
                    if(untap)
                        source->untap();
                }
            }
        }
        delete reUp;
        if(retarget)
        {
            target = source;
            source = _target;
        }
    }
    if (_target && _target->currentZone == _target->controller()->game->battlefield && reequip && !mutation)
    {
        if(!newhook)
        {
            _target = source;
            source = (MTGCardInstance *) target;  
        }
        else
        {
            while (_target->next)
                _target = _target->next;  
        }
        if(_target->hasSubtype(Subtypes::TYPE_AURA))
        {
            _target->target = source;
        }
        if(_target->hasSubtype(Subtypes::TYPE_EQUIPMENT))
        {
            for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
            {
                MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
                AEquip * eq = dynamic_cast<AEquip*> (a);
                if (eq && eq->source == _target)
                {
                    ((AEquip*)a)->unequip();
                    ((AEquip*)a)->equip(source);
                    if(untap)
                        source->untap();
                }
            }
        }
        if(!newhook)
        {
            target = source;
            source = _target;
        }
    }
    if (_target && _target->currentZone == _target->controller()->game->battlefield && mutation > 0)
    {
        _target = source;
        source = (MTGCardInstance *) target;
        for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
        {
            MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
            AEquip * eq = dynamic_cast<AEquip*> (a);
            if (eq && eq->source == _target)
            {
                uint8_t sourceoldcolors = source->colors; // Read the original colors before mutation
                uint8_t _targetoldcolors = _target->colors;
                ((AEquip*)a)->mutate(source); // hook the cards one each other
                source->colors = sourceoldcolors; // Restore the original colors after the mutation
                if(mutation == 1){
                    int deltapower = source->getPower() - source->origpower; // keep counters and power/toughness increasement
                    int deltatoughness = source->getToughness() - source->origtoughness;
                    source->origpower = _target->origpower;
                    source->origtoughness = _target->origtoughness;
                    source->basepower = _target->basepower;
                    source->basetoughness = _target->basetoughness;
                    source->setPower(_target->getPower() + deltapower);
                    source->setToughness(_target->getToughness() + deltatoughness);
                    source->colors = _targetoldcolors; // The mutated card gain all colors from the parent
                    std::string oldname = source->getName(); // The mutated card swap its name with the parent
                    source->setName(_target->getName());
                    _target->setName(oldname);
                    for (int i = ((int)source->types.size())-1; i >= 0; --i) // The mutated card looses all its types
                        if(source->types[i] != 1)
                            source->removeType(source->types[i]);
                    for (int i = 0; i < ((int)_target->types.size()); i++) // The mutated card gains all the types of the source card
                        if(_target->types[i] != 1)
                             source->addType(_target->types[i]);
                    if(source->types[0] == 1 && source->types[1] == 7){ // Fix order for Legendary Creatures
                        source->types[0] = 7;
                        source->types[1] = 1;
                    }
                    source->mPropertiesChangedSinceLastUpdate = false;
                    if(source->hasType(Subtypes::TYPE_LEGENDARY)){ // Check if the mutated card is a duplicated legendary card
                        MTGNewLegend *testlegend = NEW MTGNewLegend(source->getObserver(),source->getObserver()->mLayers->actionLayer()->getMaxId());
                        testlegend->CheckLegend(source);
                        SAFE_DELETE(testlegend);
                    }
                }
                for (unsigned int i = 0; i < (unsigned int)Constants::NB_BASIC_ABILITIES; i++){
                    if(_target->basicAbilities[i] == 1){
                        source->basicAbilities[i] = 1; // The mutated card gains all abilities from the parent
                        _target->basicAbilities[i] = 0; // The parent card looses all abilities.
                    }
                }
                _target->basicAbilities[(int)Constants::INDESTRUCTIBLE] = 1; // The parent card cannot be directly destroyed or targeted from anything because it has to follow the fate of the mutated card
                _target->basicAbilities[(int)Constants::PROTECTIONBLACK] = 1;
                _target->basicAbilities[(int)Constants::PROTECTIONBLUE] = 1;
                _target->basicAbilities[(int)Constants::PROTECTIONGREEN] = 1;
                _target->basicAbilities[(int)Constants::PROTECTIONRED] = 1;
                _target->basicAbilities[(int)Constants::PROTECTIONWHITE] = 1;
                _target->basicAbilities[(int)Constants::PROTECTIONFROMCOLOREDSPELLS] = 1;
                TargetChooserFactory tcf(_target->getObserver());
                TargetChooser * fromTc = tcf.createTargetChooser("*", _target);
                if (fromTc){
                    fromTc->setAllZones();
                    _target->getObserver()->addObserver(NEW ACantBeTargetFrom(_target->getObserver(), _target->getObserver()->mLayers->actionLayer()->getMaxId(), _target, source, fromTc));
                }
                _target->colors = 0; // The parent card loose all its colors 
                for (int i = ((int)_target->types.size())-1; i >= 0; --i) // The parent card looses all types and becomes a dummy Mutated type
                    _target->removeType(_target->types[i]);
                _target->setType("Mutated");
            }
        }
        target = source;
        source = _target;
    }
    return 1;
}

const string AANewTarget::getMenuText()
{
    return "New Target";
}

AANewTarget * AANewTarget::clone() const
{
    AANewTarget * a = NEW AANewTarget(*this);
    a->oneShot = 1;
    return a;
}
// morph a card
AAMorph::AAMorph(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
ActivatedAbility(observer, id, card, _cost, restrictions)
{
    target = _target;
    face = false;
}

int AAMorph::resolve()
{
    MTGCardInstance * Morpher = (MTGCardInstance*)source;
    if(!Morpher->isMorphed && !Morpher->morphed && Morpher->turningOver)
        return 0;
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be morphed, they will follow the fate of top-card
        while (_target->next)
            _target = _target->next; 

        AbilityFactory af(game);
        _target->morphed = false;
        _target->isMorphed = false;
        _target->turningOver = true;
        af.getAbilities(&currentAbilities, NULL, _target, 0);
        for (size_t i = 0; i < currentAbilities.size(); ++i)
        {
            MTGAbility * a = currentAbilities[i];
            a->source = (MTGCardInstance *) _target;
            if( a && dynamic_cast<AAMorph *> (a))
            {
                a->removeFromGame();
                game->removeObserver(a);
            }
            if (a)
            {
                if (a->oneShot)
                {
                    a->resolve();
                    delete (a);
                }
                else
                {
                    a->addToGame();
                    MayAbility * dontAdd = dynamic_cast<MayAbility*>(a);
                    if(!dontAdd)
                    {
                        _target->cardsAbilities.push_back(a);
                    }
                }
            }
        }
        _target->isFacedown = false;
        WEvent * e = NEW WEventCardFaceUp(_target);
        game->receiveEvent(e);
        currentAbilities.clear();
        testDestroy();
    }
    return 1;
}

int AAMorph::testDestroy()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(target)
    {
        if(_target->turningOver && !_target->isMorphed && !_target->morphed)
        {
            game->removeObserver(this);
            return 1;
        }
    }
    return 0;
}

const string AAMorph::getMenuText()
{
    if(face && target)
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if(_target && _target->model)
        {
            std::ostringstream abname;
            abname << "Face Up " << _target->model->data->getManaCost()->toString();
        return abname.str();
        }
    }
    return "Morph";
}

AAMorph * AAMorph::clone() const
{
    AAMorph * a = NEW AAMorph(*this);
    a->forceDestroy = 1;
    return a;
}

//Melded From Setter
AAMeldFrom::AAMeldFrom(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, string MeldedName) :
    ActivatedAbility(observer, id, card, 0), _MeldedName(MeldedName)
{
    target = _target;
    // aType = MTGAbility::Melder;
}

int AAMeldFrom::resolve()
{
    source->MeldedFrom = _MeldedName;
    return 1;
}

const string AAMeldFrom::getMenuText()
{
    return "Melded From";
}

AAMeldFrom * AAMeldFrom::clone() const
{
    return NEW AAMeldFrom(*this);
}

//Melding
AAMeld::AAMeld(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, string MeldedName) :
    ActivatedAbility(observer, id, card, 0), _MeldedName(MeldedName)
{
    target = _target;
    andAbility = NULL;
   // aType = MTGAbility::Melder;
}

int AAMeld::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if (_target && _target->controller() == source->controller() && _target->owner == source->owner && !_target->isToken && !source->isToken)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be melded, they will follow the fate of top-card
        source->controller()->game->putInExile(source);
        _target->controller()->game->putInExile(_target);
        source->next->controller()->game->putInZone(source->next, source->next->currentZone, source->next->controller()->game->temp);
        _target->next->controller()->game->putInZone(_target->next, _target->next->currentZone, _target->next->controller()->game->temp);
        MTGAbility *a = NEW AACastCard(game, game->mLayers->actionLayer()->getMaxId(), source, source, false, false, false, _MeldedName, _MeldedName, false, true);
        a->oneShot = false;
        a->canBeInterrupted = false;
        if(andAbility)
            ((AACastCard*)a)->andAbility = andAbility->clone();
        a->addToGame();

        return 1;
    }
    return 0;
}

const string AAMeld::getMenuText()
{
    return "Meld";
}

AAMeld * AAMeld::clone() const
{
    AAMeld * a = NEW AAMeld(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AAMeld::~AAMeld()
{
    SAFE_DELETE(andAbility);
}

//Turn side of double faced cards
AATurnSide::AATurnSide(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, string SideName) :
    ActivatedAbility(observer, id, card, 0), _SideName(SideName)
{
    target = _target;
}

int AATurnSide::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if (_target && _target->currentZone != _target->controller()->game->battlefield) // It's not allowed to turn side on battlefield.
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be turned, they will follow the fate of top-card
        MTGCard * fcard;
        MTGCardInstance* sideCard;
        if(_target->controller()->isAI() && _target->isFlipped > 0) _target->isFlipped = 0; // If it's AI calling back we just have to reset isFLipped flag and then return.
        if(_target->isFlipped == 0 && _SideName == "") return 0; // No need to turn front if card has not been flipped before.
        if(_target->isFlipped == 0){
            if(_SideName == "backside" && _target->backSide != "") 
                _SideName = _target->backSide; // Added to allow to turn a card on its backside.
            fcard = MTGCollection()->getCardByName(_SideName, _target->setId);
            if(!fcard) return 0;
            sideCard = NEW MTGCardInstance(fcard, _target->controller()->game);
            _target->nameOrig = _target->name; 
            _target->name = sideCard->name;
            _target->setName(sideCard->name);
            _target->backSide = sideCard->backSide;
            if(!sideCard) return 0;
            if(sideCard->getManaCost()){
                if(_target->getManaCost()->getAlternative()){
                    sideCard->getManaCost()->setAlternative(NEW ManaCost());
                    sideCard->getManaCost()->getAlternative()->copy(_target->getManaCost()->getAlternative()); // Keep orignal alternative cost to cast the original card with other.
                }
                if(_target->getManaCost()->getMorph()){
                    sideCard->getManaCost()->setMorph(NEW ManaCost());
                    sideCard->getManaCost()->getMorph()->copy(_target->getManaCost()->getMorph()); // Keep orignal morph cost to cast the original card with morph.
                }
                if(_target->getManaCost()->getRetrace()){
                    sideCard->getManaCost()->setRetrace(NEW ManaCost());
                    sideCard->getManaCost()->getRetrace()->copy(_target->getManaCost()->getRetrace()); // Keep orignal retrace cost to cast the original card with retrace (e.g. cards with disturb cost).
                }
                _target->getManaCost()->copy(sideCard->getManaCost()); // Show the other side cost mana symbols.
                if(_target->numofcastfromcommandzone > 0){ //In case you turn side of a previuosly casted commander
                    _target->getManaCost()->add(Constants::MTG_COLOR_ARTIFACT,2*_target->numofcastfromcommandzone);
                }
            }
        } else {
            fcard = MTGCollection()->getCardByName(_target->nameOrig, _target->setId);
            if(!fcard) return 0;
            _target->name = _target->nameOrig;
            _target->setName(_target->nameOrig);
            _target->nameOrig = "";
            sideCard = NEW MTGCardInstance(fcard, _target->controller()->game);
            if(!sideCard) return 0;
            if(sideCard->getManaCost()){
                _target->getManaCost()->resetCosts();
                _target->getManaCost()->copy(sideCard->getManaCost()); // Restore the original side cost mana symbols.
                if(_target->numofcastfromcommandzone > 0){ //In case you turn side of a previuosly casted commander
                    _target->getManaCost()->add(Constants::MTG_COLOR_ARTIFACT,2*_target->numofcastfromcommandzone);
                    if(_target->getManaCost()->getAlternative())
                        _target->getManaCost()->getAlternative()->add(Constants::MTG_COLOR_ARTIFACT,2*_target->numofcastfromcommandzone);
                    if(_target->getManaCost()->getMorph())
                        _target->getManaCost()->getMorph()->add(Constants::MTG_COLOR_ARTIFACT,2*_target->numofcastfromcommandzone);
                }
            }
        }
        if(_target->owner->playMode != Player::MODE_TEST_SUITE)
        {
            _target->setMTGId(sideCard->getMTGId());
            _target->setId = sideCard->setId;
        }
        _target->power = sideCard->power;
        _target->toughness = sideCard->toughness;
        _target->origpower = sideCard->origpower;
        _target->origtoughness = sideCard->origtoughness;
        _target->basepower = sideCard->basepower;
        _target->basetoughness = sideCard->basetoughness;
        _target->types = sideCard->types;
        _target->text = sideCard->text;
        _target->formattedText = sideCard->formattedText;
        _target->magicText = sideCard->magicText;
        _target->colors = sideCard->colors;
        _target->isFlipped = (_target->isFlipped > 0)?0:1;
        SAFE_DELETE(sideCard);
        return 1;
    }
    return 0;
}

const string AATurnSide::getMenuText()
{
    return "Flip Side";
}

AATurnSide * AATurnSide::clone() const
{
    return NEW AATurnSide(*this);
}

// flip a card
AAFlip::AAFlip(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target,string flipStats, bool isflipcard, bool forcedcopy, string forcetype, bool backfromcopy) :
InstantAbility(observer, id, card, _target),flipStats(flipStats),isflipcard(isflipcard),forcedcopy(forcedcopy),forcetype(forcetype),backfromcopy(backfromcopy)
{
    target = _target;
    andAbility = NULL;
}

int AAFlip::resolve()
{
    int cdaDamage = 0;
    int activatedanyability = 0;
    MTGCardInstance * Flipper = (MTGCardInstance*)source;
    this->oneShot = true;
    if(Flipper->isFlipped > 0 && forcetype == "" && flipStats != "myorigname" && flipStats != "chosenname" && flipStats != "backside") // Fixed a problem on some backside cards (e.g. "Edgar Markov's Coffin").
    {
        game->removeObserver(this);
        return 0;
    }
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be flipped, they will follow the fate of top-card
        if(((_target->isACopier || (_target->isToken && _target->backSide == "" && !backfromcopy)) && !isflipcard && !backfromcopy) || _target->has(Constants::CANTTRANSFORM))
        {
            game->removeObserver(this);
            return 0;
        }

        while (_target->next)
            _target = _target->next;

        MTGGameZone * currentZone = NULL;
        if(forcetype != "" && _target) // Added to flip Modal Double Faced cards (e.g. Zendikar Rising).
        {
            currentZone = _target->currentZone; // Added to keep track of current zone before flip.
            for (int i = ((int)_target->types.size())-1; i >= 0; --i)
                _target->removeType(_target->types[i]);
            list<int> typesToAdd;
            PopulateSubtypesIndexVector(typesToAdd,forcetype);
            list<int>::iterator it;
            for (it = typesToAdd.begin(); it != typesToAdd.end(); it++)
                _target->addType(*it);
            _target = _target->currentZone->removeCard(_target, true);
        }

        AbilityFactory af(game);
        _target->isFlipped = 1;
        GameObserver * game = _target->getObserver();
        if(flipStats.size())
        {
            if(flipStats == "myorigname" && _target->nameOrig != "") 
                flipStats = _target->nameOrig; // Added to undo the copy effect at end of turn for a generic card (e.g. Shapeshifter transformations).
            else if(flipStats == "chosenname" && _target->chooseaname != "") 
                flipStats = _target->chooseaname; // Added to allow the transformation of a card in a choosen name.
            else if(flipStats == "backside" && _target->backSide != "") 
                flipStats = _target->backSide; // Added to allow the transformation of a card in its backside (e.g. Werewolves transformations).
            MTGCard * fcard = MTGCollection()->getCardByName(flipStats, _target->setId);
            if(!fcard) return 0;
            MTGCardInstance * myFlip = NEW MTGCardInstance(fcard, _target->controller()->game);
            MTGCardInstance * myParent = NULL;
            if(_target->target)
                myParent = _target->target;
            string nameOrig = _target->name;
            if(_target->nameOrig == "")
                _target->nameOrig = nameOrig; // Saves the orignal card name before to flip the card.
            _target->name = myFlip->name;
            _target->setName(myFlip->name);
            _target->backSide = myFlip->backSide;
            if(!isflipcard)//transform card
            {
                _target->getManaCost()->resetCosts();
                if(myFlip->getManaCost())
                    _target->getManaCost()->copy(myFlip->getManaCost());
            }
            _target->spellTargetType = myFlip->spellTargetType; // Fix to prevent flipped auras go to graveyard.
            _target->colors = myFlip->colors;
            _target->types = myFlip->types;
            _target->text = myFlip->text;
            _target->formattedText = myFlip->formattedText;
            if(_target->enchanted || _target->equipment > 0){ // Try to keep auras and equipment effects on basicAbilities (issue #1065).
                MTGCardInstance * myOrig = NEW MTGCardInstance(MTGCollection()->getCardByName(nameOrig, _target->setId), _target->controller()->game);
                for(unsigned int i = 0; i < _target->basicAbilities.size(); i++) {
                    if(myOrig->basicAbilities[i] == 1)
                        _target->basicAbilities[i] = 0;
                    if(myFlip->model->data->basicAbilities[i] == 1)
                        _target->basicAbilities[i] = 1;
                }
                SAFE_DELETE(myOrig);
            } else{
                for(size_t i = 0; i < _target->basicAbilities.size(); i++) {
                    if(i != Constants::GAINEDEXILEDEATH && i != Constants::GAINEDHANDDEATH && i != Constants::GAINEDDOUBLEFACEDEATH && 
                        i != Constants::DUNGEONCOMPLETED && i != Constants::PERPETUALDEATHTOUCH && i != Constants::PERPETUALLIFELINK)
                        _target->basicAbilities[i] = myFlip->model->data->basicAbilities[i]; // Try to keep the original special abilities on card flip.
                }
            }
            cdaDamage = _target->damageCount;
            _target->copiedID = myFlip->getMTGId();//for copier
            if(_target->owner->playMode != Player::MODE_TEST_SUITE)
            {
                _target->setMTGId(myFlip->getMTGId());
                _target->setId = myFlip->setId;
            }
            //check pw
            if(_target->hasType(Subtypes::TYPE_PLANESWALKER))
            {
                for(unsigned int k = 0;k < _target->cardsAbilities.size();++k)
                {
                    ActivatedAbility * check = dynamic_cast<ActivatedAbility*>(_target->cardsAbilities[k]);
                    if(check && check->counters)
                        activatedanyability++;
                }
            }
            //
            for(unsigned int i = 0;i < _target->cardsAbilities.size();i++)
            {
                MTGAbility * a = dynamic_cast<MTGAbility *>(_target->cardsAbilities[i]);

                if(a) game->removeObserver(a);
            }
            _target->cardsAbilities.clear();
            _target->magicText = myFlip->magicText;
            af.getAbilities(&currentAbilities, NULL, _target);
            for (size_t i = 0; i < currentAbilities.size(); ++i)
            {
                MTGAbility * a = currentAbilities[i];
                a->source = (MTGCardInstance *) _target;
                if (a)
                {
                    if (a->oneShot)
                    {
                        if(!backfromcopy){ // Fix to avoid triggering of oneshot abilities when flip is used to return from a copy.
                            if(_target->hasType(Subtypes::TYPE_PLANESWALKER)){ // Fix to don't let planeswalker die on flip (since the counter ability is not resolving correctly during flip).
                                AACounter * tmp = dynamic_cast<AACounter *>(a);
                                if(tmp && tmp->counterstring.find("loyalty") != string::npos){
                                    for (int j = 0; j < tmp->nb; j++)
                                        _target->counters->addCounter("loyalty", 0, 0, true); 
                                } else a->resolve();
                            } else a->resolve();
                        }
                        SAFE_DELETE(a);
                    }
                    else
                    {
                        MayAbility * dontAdd = dynamic_cast<MayAbility*>(a);
                        if(!dontAdd){
                            a->addToGame();
                            _target->cardsAbilities.push_back(a);
                        } else if(!backfromcopy) // Fix to avoid triggering of may abilities when flip is used to return from a copy (e.g. Mirror of the Forebears).
                            a->addToGame();
                    }
                }
            }
            //limit pw abi
            if(activatedanyability)
            {
                if(_target->hasType(Subtypes::TYPE_PLANESWALKER))
                {
                    for(unsigned int k = 0;k < _target->cardsAbilities.size();++k)
                    {
                        ActivatedAbility * check = dynamic_cast<ActivatedAbility*>(_target->cardsAbilities[k]);
                        if(check)//is there a better way?
                            check->counters++;
                    }
                }
            }
            //power
            int powerMod = 0;
            int toughMod = 0;
            bool powerlessThanOriginal = false;
            bool toughLessThanOriginal = false;
            if(_target->power < _target->origpower)
            {
                powerMod = _target->origpower - _target->power;
                powerlessThanOriginal = true;
            }
            else
            {
                powerMod =_target->power - _target->origpower;
            }
            //toughness
            if(_target->toughness <= _target->origtoughness)
            {
                toughMod = _target->origtoughness - _target->toughness;
                toughLessThanOriginal = true;
            }
            else
            {
                toughMod =_target->toughness - _target->origtoughness;
            }
            if(!_target->isCDA)
            {
                _target->power = powerlessThanOriginal?myFlip->power - powerMod:myFlip->power + powerMod;
                _target->life = toughLessThanOriginal?myFlip->toughness - toughMod:myFlip->toughness + toughMod;
                _target->toughness = toughLessThanOriginal?myFlip->toughness - toughMod:myFlip->toughness + toughMod;
                _target->origpower = myFlip->origpower;
                _target->origtoughness = myFlip->origtoughness;
            }
            else
            {//pbonus & tbonus are already computed except damage taken...
                _target->life -= cdaDamage;
            }
            if(_target->hasSubtype(Subtypes::TYPE_EQUIPMENT))
            {
                if(myParent)
                    _target->target = myParent;
            }
            SAFE_DELETE(myFlip);
            _target->mPropertiesChangedSinceLastUpdate = true;
            if(backfromcopy)
                _target->isACopier = false; //the card is no longer a copy (e.g. "Renegade Doppelganger" and "Scion of the Ur-Dragon")
            if(!isflipcard && !backfromcopy)
            {
                if(_target->isFacedown)
                    _target->isFacedown = false;
                else
                    _target->isFacedown = true;

                if(forcetype != "" && _target && _target->isPermanent()) // Added to flip Modal Double Faced cards (e.g. Zendikar Rising).
                {
                    if(!currentZone) currentZone = _target->controller()->game->hand; // If NULL, we consider hand as the default currentZone.
                    _target->castMethod = Constants::CAST_NORMALLY;
                    _target->controller()->game->battlefield->addCard(_target);
                    WEvent * e = NEW WEventZoneChange(_target, currentZone, _target->controller()->game->battlefield);
                    game->receiveEvent(e);
                } else {
                    WEvent * e = NEW WEventCardTransforms(_target);
                    game->receiveEvent(e);
                }
            }
        }
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        currentAbilities.clear();
        testDestroy();
    }
    return 1;
}

int AAFlip::testDestroy()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(target)
    {
        if(_target->isFlipped > 0)
        {
            this->forceDestroy = 1;
            //_target->getObserver()->removeObserver(this);
            //originally added as a safegaurd to insure the ability was removed
            //it's been so long and so much has changed that it appears to do nothing but cause a crash now
            _target->isFlipped = 0;
            return 1;
        }
    }
    return 0;
}

const string AAFlip::getMenuText()
{
    string s = flipStats;
    sprintf(menuText, "Transform:%s", s.c_str());
    return menuText;
}

AAFlip * AAFlip::clone() const
{
    AAFlip * a = NEW AAFlip(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    a->forceDestroy = 1;
    return a;
}

AAFlip::~AAFlip()
{
    SAFE_DELETE(andAbility);
}

// AADYNAMIC: dynamic ability builder
AADynamic::AADynamic(GameObserver* observer, int id, MTGCardInstance * card, Damageable * _target,int type,int effect,int who,int amountsource,MTGAbility * storedAbility, ManaCost * _cost) :
ActivatedAbility(observer, id, card, _cost, 0),type(type),effect(effect),who(who),amountsource(amountsource),storedAbility(storedAbility)
{
    target = _target;
    sourceamount = 0;
    targetamount = 0;
    eachother = false;
    tosrc = false;
    menu = "";
    OriginalSrc = source;
    clonedStored = NULL;
    mainAbility = NULL;
}

int AADynamic::resolve()
{
    Damageable * _target = (Damageable *) target;
    Damageable * secondaryTarget = NULL;
    if(amountsource == 2)
        source = (MTGCardInstance * )_target;
    switch(who)
    {
    case DYNAMIC_ABILITY_WHO_EACHOTHER://each other, both take the effect
        eachother = true;
        break;
    case DYNAMIC_ABILITY_WHO_ITSELF:
        source = ((MTGCardInstance *) _target);
        break;
    case DYNAMIC_ABILITY_WHO_TARGETCONTROLLER:
        secondaryTarget = ((MTGCardInstance *) _target)->controller();
        break;
    case DYNAMIC_ABILITY_WHO_TARGETOPPONENT:
        secondaryTarget = ((MTGCardInstance *) _target)->controller()->opponent();
        break;
    case DYNAMIC_ABILITY_WHO_TOSOURCE:
        tosrc = true;
        break;
    case DYNAMIC_ABILITY_WHO_SOURCECONTROLLER:
        secondaryTarget = OriginalSrc->controller();
        break;
    case DYNAMIC_ABILITY_WHO_SOURCEOPPONENT:
        secondaryTarget = OriginalSrc->controller()->opponent();
        break;
    case DYNAMIC_ABILITY_WHO_ABILITYCONTROLLER:
        {
            if(OriginalSrc->storedSourceCard)
                secondaryTarget = OriginalSrc->storedSourceCard->controller();
            else
                secondaryTarget = OriginalSrc->controller();
        }
        break;
    default:
        break;
    }
    if(amountsource == DYNAMIC_MYSELF_AMOUNT)
        _target = OriginalSrc->controller();//looking at controller for amount
    if(amountsource == DYNAMIC_MYFOE_AMOUNT)
        _target = OriginalSrc->controller()->opponent();//looking at controllers opponent for amount
    if(!_target)
        return 0;
    while (dynamic_cast<MTGCardInstance *>(_target) && ((MTGCardInstance *)_target)->next)
        _target = ((MTGCardInstance *)_target)->next;

    //find the amount variables that will be used
    sourceamount = 0;
    targetamount = 0;
    int colored = 0;
    switch(type)
    {
    case DYNAMIC_ABILITY_TYPE_POWER:
        sourceamount = ((MTGCardInstance *) source)->getCurrentPower();
        targetamount = ((MTGCardInstance *) _target)->getCurrentPower();
        if(eachother )
            sourceamount = ((MTGCardInstance *) source)->getCurrentPower();
        break;
    case DYNAMIC_ABILITY_TYPE_TOUGHNESS:
        sourceamount = ((MTGCardInstance *) source)->getCurrentToughness();
        targetamount = ((MTGCardInstance *) _target)->getCurrentToughness();
        if(eachother )
            sourceamount = ((MTGCardInstance *) source)->getCurrentToughness();
        break;
    case DYNAMIC_ABILITY_TYPE_MANACOST:
        if(amountsource == 1)
            sourceamount = ((MTGCardInstance *) source)->getManaCost()->getConvertedCost();
        else
            sourceamount = ((MTGCardInstance *) _target)->getManaCost()->getConvertedCost();
        break;
    case DYNAMIC_ABILITY_TYPE_COLORS:
        for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
        {
            if (amountsource == 1 && ((MTGCardInstance *)source)->hasColor(i))
                ++colored;
            else
                if (amountsource == 2 && ((MTGCardInstance *)_target)->hasColor(i))
                    ++colored;
        }
        sourceamount = colored;
        break;
    case DYNAMIC_ABILITY_TYPE_AGE:
        {
            Counter * targetCounter = NULL;
            if(amountsource == 2)
            {
                if (((MTGCardInstance *)_target)->counters && ((MTGCardInstance *)_target)->counters->hasCounter("age", 0, 0))
                {
                    targetCounter = ((MTGCardInstance *)_target)->counters->hasCounter("age", 0, 0);
                    sourceamount = targetCounter->nb;
                }
            }
            else
            {
                if (((MTGCardInstance *)source)->counters && ((MTGCardInstance *)source)->counters->hasCounter("age", 0, 0))
                {
                    targetCounter = ((MTGCardInstance *)source)->counters->hasCounter("age", 0, 0);
                    sourceamount = targetCounter->nb;
                }
            }
            break;
        }
    case DYNAMIC_ABILITY_TYPE_CHARGE:
        {
            Counter * targetCounter = NULL;
            if(amountsource == 2)
            {
                if (((MTGCardInstance *)_target)->counters && ((MTGCardInstance *)_target)->counters->hasCounter("charge", 0, 0))
                {
                    targetCounter = ((MTGCardInstance *)_target)->counters->hasCounter("charge", 0, 0);
                    sourceamount = targetCounter->nb;
                }
            }
            else
            {
                if (((MTGCardInstance *)source)->counters && ((MTGCardInstance *)source)->counters->hasCounter("charge", 0, 0))
                {
                    targetCounter = ((MTGCardInstance *)source)->counters->hasCounter("charge", 0, 0);
                    sourceamount = targetCounter->nb;
                }
            }
            break;
        }
    case DYNAMIC_ABILITY_TYPE_ONEONECOUNTERS:
        {
            Counter * targetCounter = NULL;
            if(amountsource == 2)
            {
                if (((MTGCardInstance *)_target)->counters && ((MTGCardInstance *)_target)->counters->hasCounter(1, 1))
                {
                    targetCounter = ((MTGCardInstance *)_target)->counters->hasCounter(1,1);
                    sourceamount = targetCounter->nb;
                }
            }
            else
            {
                if (((MTGCardInstance *)source)->counters && ((MTGCardInstance *)source)->counters->hasCounter(1, 1))
                {
                    targetCounter = ((MTGCardInstance *)source)->counters->hasCounter(1,1);
                    sourceamount = targetCounter->nb;
                }
            }
            break;
        }
    case DYNAMIC_ABILITY_TYPE_THATMUCH:
        {
            sourceamount = _target->thatmuch;
            break;
        }
    default:
        break;
    }

    if(secondaryTarget != NULL)
        _target = secondaryTarget;
    if (_target)
    {
        while (dynamic_cast<MTGCardInstance *>(_target) && ((MTGCardInstance *)_target)->next)
            _target = ((MTGCardInstance *)_target)->next;
        if(sourceamount < 0)
            sourceamount = 0;
        if(targetamount < 0)
            targetamount = 0;
        std::stringstream out;
        std::stringstream out2;
        out << sourceamount;
        string sourceamountstring = out.str();
        out2 << targetamount;
        string targetamountstring = out2.str();
        //set values less then 0 to 0, it was reported that negitive numbers such as a creature who get -3/-3 having the power become
        //negitive, if then used as the amount, would cuase weird side effects on resolves.
        switch(effect)
        {
        case DYNAMIC_ABILITY_EFFECT_STRIKE://deal damage
            {
                mainAbility = NEW AADamager(game, this->GetId(), source,tosrc == true?(Targetable*)OriginalSrc:(Targetable*)_target,sourceamountstring);
                activateMainAbility(mainAbility,source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target);
                if(eachother)
                {
                    mainAbility = NEW AADamager(game, this->GetId(), (MTGCardInstance*)_target,(Targetable*)OriginalSrc,targetamountstring);
                    activateMainAbility(mainAbility,source,OriginalSrc);
                }
                return 1;
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_DRAW://draw cards
            {
                mainAbility = NEW AADrawer(game, this->GetId(), source,_target,NULL, sourceamountstring);
                return activateMainAbility(mainAbility,source,_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_LIFEGAIN://gain life
            {
                mainAbility = NEW AALifer(game, this->GetId(), source,_target, sourceamountstring);
                return activateMainAbility(mainAbility,source,_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_PUMPPOWER://pump power
            {
                mainAbility = NEW PTInstant(game, this->GetId(), source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target,NEW WParsedPT(sourceamount,0));
                return activateMainAbility(mainAbility,source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_PUMPTOUGHNESS://pump toughness
            {
                mainAbility = NEW PTInstant(game, this->GetId(), source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target,NEW WParsedPT(0,sourceamount));
                return activateMainAbility(mainAbility,source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_PUMPBOTH://pump both
            {
                mainAbility = NEW PTInstant(game, this->GetId(), source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target,NEW WParsedPT(sourceamount,sourceamount));
                return activateMainAbility(mainAbility,source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_LIFELOSS://lose life
            {
                string altered = "-";
                altered.append(sourceamountstring);
                mainAbility = NEW AALifer(game, this->GetId(), source,_target, altered);
                return activateMainAbility(mainAbility,source,_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_DEPLETE://deplete cards
            {
                mainAbility = NEW AADepleter(game, this->GetId(), source,_target, sourceamountstring);
                return activateMainAbility(mainAbility,source,_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_COUNTERSONEONE:
            {
                if(!dynamic_cast<MTGCardInstance *>(_target))
                    _target = OriginalSrc;
                for(int j = 0;j < sourceamount;j++)
                    ((MTGCardInstance*)_target)->counters->addCounter(1,1);
                break;
            }
        default:
            return 0;
        }
    }

    return 0;
}
int AADynamic::activateMainAbility(MTGAbility * toActivate,MTGCardInstance * , Damageable *)
{
    if(storedAbility)
        activateStored();
    if(!toActivate)
        return 0;
    if(PTInstant * a = dynamic_cast<PTInstant *>(toActivate))
    {
        a->addToGame();
        return 1;
    }
    toActivate->oneShot = true;
    toActivate->forceDestroy = 1;
    toActivate->resolve();
    SAFE_DELETE(toActivate);
    return 1;
}

int AADynamic::activateStored()
{
    clonedStored = storedAbility->clone();
    clonedStored->target = target;
    if (clonedStored->oneShot)
    {
        clonedStored->resolve();
        delete (clonedStored);
    }
    else
    {
        clonedStored->addToGame();
    }
    return 1;
}

const string AADynamic::getMenuText()
{
    if (menu.size())
    {
        return menu.c_str();
    }

    switch(type)
    {
    case 0:
        menu.append("Power");
        break;
    case 1:
        menu.append("Tough");
        break;
    case 2:
        menu.append("Mana");
        break;
    case 3:
        menu.append("color");
        break;
    case 4:
        menu.append("Elder");
        break;
    case 5:
        menu.append("Charged");
        break;
    case 6:
        menu.append("Counter");
        break;
    case 7:
        menu.append("That Many ");
        break;
    default:
        break;
    }

    switch(effect)
    {
    case 0:
        menu.append("Strike");
        break;
    case 1:
        menu.append("Draw");
        break;
    case 2:
        menu.append("Life");
        break;
    case 3:
        menu.append("Pump");
        break;
    case 4:
        menu.append("Fortify");
        break;
    case 5:
        menu.append("Buff");
        break;
    case 6:
        menu.append("Drain");
        break;
    case 7:
        menu.append("Deplete!");
        break;
    case 8:
        menu.append("Counters!");
        break;
    default:
        break;
    }
    
    return menu.c_str();
}

AADynamic * AADynamic::clone() const
{
    AADynamic * a = NEW AADynamic(*this);
    a->storedAbility = storedAbility? storedAbility->clone() : NULL;
    return a;
}

AADynamic::~AADynamic()
{
    SAFE_DELETE(storedAbility);
}

//AALifer
AALifer::AALifer(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, string life_s, bool siphon, ManaCost * _cost, int who) :
ActivatedAbilityTP(observer, _id, card, _target, _cost, who),life_s(life_s),siphon(siphon)
{
    aType = MTGAbility::LIFER;
    andAbility = NULL;
}

int AALifer::resolve()
{  
    Damageable * _target = (Damageable *) getTarget();
    if (!_target)
        return 0;

    WParsedInt life(life_s, NULL, source);
    if (_target->type_as_damageable == Damageable::DAMAGEABLE_MTGCARDINSTANCE)
    {
        _target = ((MTGCardInstance *) _target)->controller();
    }
    Player *player = (Player*)_target;
    int slife = abs(player->gainOrLoseLife(life.getValue(), source));
    if(siphon && (slife > 0) && (life.getValue() < 0))
        source->controller()->gainOrLoseLife(slife, source);

    if(andAbility)
    {
        MTGAbility * andAbilityClone = andAbility->clone();
        andAbilityClone->target = source;
        if(andAbility->oneShot)
        {
            andAbilityClone->resolve();
            SAFE_DELETE(andAbilityClone);
        }
        else
        {
            andAbilityClone->addToGame();
        }
    }

    return 1;
}

int AALifer::getLife()
{
    WParsedInt life(life_s, NULL, source);
    return life.getValue();
}

const string AALifer::getMenuText()
{
    if(getLife() < 0)
        return "Life Loss";
    return "Life";
}

AALifer * AALifer::clone() const
{
    AALifer * a = NEW AALifer(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AALifer::~AALifer()
{
    SAFE_DELETE(andAbility);
}

//Extra for Bestow ... partial fix since there's no update when react to click for bestow cards...
//There should be no problem if the bestow cards has chosen mode then update its bestow code on react to click but
//I cant find alternate way... This Ability is general for enchantments since aura is an enchantment type however
//it can't target card specific attributes... This one adds on the players side...
AAuraIncreaseReduce::AAuraIncreaseReduce(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int amount, int color, int who) :
    AbilityTP(observer, _id, _source, _target, who), amount(amount), color(color)
{
    manaReducer = source;
}

int AAuraIncreaseReduce::addToGame()
{
    Damageable * _target = (Damageable *) getTarget();
    Player * p = getPlayerFromDamageable(_target);

    if (!p)
        return 0;

    if (amount > 0)
    {
        p->AuraIncreased->add(color,amount);
    }
    else
    {
        p->AuraReduced->add(color,abs(amount));
    }

    return MTGAbility::addToGame();
}

int AAuraIncreaseReduce::destroy()
{
    Damageable * _target = (Damageable *) getTarget();
    Player * p = getPlayerFromDamageable(_target);

    if (!p)
        return 0;

    if(!this->manaReducer->isInPlay(game))
    {
        if (amount > 0)
        {
            p->AuraIncreased->remove(color,amount);
        }
        else
        {
            p->AuraReduced->remove(color,abs(amount));
        }
        return MTGAbility::testDestroy();
    }

    return 0;
}

int AAuraIncreaseReduce::testDestroy()
{
    if(!this->manaReducer->isInPlay(game))
    {
        return MTGAbility::testDestroy();
    }

    return 0;
}

const string AAuraIncreaseReduce::getMenuText()
{
    return "Aura Increaser/Reducer";
}

AAuraIncreaseReduce * AAuraIncreaseReduce::clone() const
{
    return NEW AAuraIncreaseReduce(*this);
}

//players modify hand size
AModifyHand::AModifyHand(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, string hand, int who) :
    AbilityTP(observer, _id, _source, _target, who), hand(hand)
{
}

int AModifyHand::addToGame()
{
    Damageable * _target = (Damageable *) getTarget();
    Player * p = getPlayerFromDamageable(_target);

    if (!p)
        return 0;

    WParsedInt handmodifier(hand, NULL, source);
    p->handmodifier += handmodifier.getValue();

    return MTGAbility::addToGame();
}

int AModifyHand::destroy()
{
    Damageable * _target = (Damageable *) getTarget();
    Player * p = getPlayerFromDamageable(_target);

    if (!p)
        return 0;
    
    WParsedInt handmodifier(hand, NULL, source);
    p->handmodifier -= handmodifier.getValue();

    return 1;
}

const string AModifyHand::getMenuText()
{
    return "Modify Hand Size";
}

AModifyHand * AModifyHand::clone() const
{
    return NEW AModifyHand(*this);
}

//players max hand size
AASetHand::AASetHand(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int hand, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), hand(hand)
{
}

int AASetHand::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    Player * p = getPlayerFromDamageable(_target);

    if (!p)
        return 0;

    p->handsize = hand;

    return 1;
}

const string AASetHand::getMenuText()
{
    return "Set Hand Size";
}

AASetHand * AASetHand::clone() const
{
    return NEW AASetHand(*this);
}

//Lifeset
AALifeSet::AALifeSet(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, WParsedInt * life, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), life(life)
{
}

int AALifeSet::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    Player * p = getPlayerFromDamageable(_target);

    if (!p)
        return 0;

    int lifeDiff = life->getValue() - p->life ;
    p->gainOrLoseLife(lifeDiff, source);

    return 1;
}

const string AALifeSet::getMenuText()
{
    return "Set Life";
}

AALifeSet * AALifeSet::clone() const
{
    AALifeSet * a = NEW AALifeSet(*this);
    a->life = NEW WParsedInt(*(a->life));
    return a;
}

AALifeSet::~AALifeSet()
{
    SAFE_DELETE(life);
}

//AACloner 
//cloning...this makes a token thats a copy of the target.
AACloner::AACloner(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, int who,
        string abilitiesStringList, string TypesList, string optionsList) :
    ActivatedAbility(observer, _id, _source, _cost, 0), who(who)
{
    aType = MTGAbility::CLONING;
    target = _target;
    source = _source;
    options = optionsList;
    battleReady = (abilitiesStringList.find("battleready") != string::npos)?true:false;
    if (abilitiesStringList.size() > 0)
    {
        PopulateAbilityIndexVector(awith, abilitiesStringList);
        PopulateColorIndexVector(colors, abilitiesStringList);
    }
    if (TypesList.size())
    {
        PopulateSubtypesIndexVector(typesToAdd,TypesList);
    }
    andAbility = NULL;
}

int AACloner::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (!_target)
        return 0;
    if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be cloned, they will follow the fate of top-card

    MTGCard * clone = NULL;

    if(_target->isToken || _target->hasCopiedToken)
    {
        if(_target->getMTGId() > 0)//not generated token
            clone = MTGCollection()->getCardById(_target->getMTGId());
        else
        {
            clone = _target->tokCard;
            if(clone) // Check if clone is not null to avoid null pointer exception. #ISSUE 1040
                clone->data = _target->tokCard;//?wtf
        }
    }
    else
         clone = MTGCollection()->getCardById(_target->copiedID);

    if(!clone)
        source = _target;

    Player * targetPlayer = who == 1 ? source->controller()->opponent() : source->controller();

    int tokenize = 1;//tokenizer support for cloning
    if (targetPlayer->game->battlefield->hasAbility(Constants::TOKENIZER))
    {
        int nbcards = targetPlayer->game->battlefield->nb_cards;
        for (int j = 0; j < nbcards; j++)
        {
            if (targetPlayer->game->battlefield->cards[j]->has(Constants::TOKENIZER))
                tokenize *= 2;
        }
    }

    for (int i = 0; i < tokenize; ++i)
    {
        MTGCardInstance * myClone = NEW MTGCardInstance(clone, targetPlayer->game);
        targetPlayer->game->temp->addCard(myClone);
                
        Spell * spell = NEW Spell(game, myClone);
        spell->source->isToken = 1;
        if(spell->source->hasType(Subtypes::TYPE_LEGENDARY) && options.find("nolegend") != string::npos){ // check if the token has to be legendary or not. (e.g. Double Major)
            spell->source->removeType(Subtypes::TYPE_LEGENDARY);
        }
        spell->resolve();
        spell->source->owner = targetPlayer;
        spell->source->lastController = targetPlayer;
        spell->source->fresh = 1;
        spell->source->entersBattlefield = 1;
        spell->source->model = spell->source;
        spell->source->model->data = spell->source;
        spell->source->tokCard = spell->source->clone();
        spell->source->TokenAndAbility = _target->TokenAndAbility;//token andAbility
        //if the token doesn't have cda/dynamic pt then allow this...
        if((_target->isToken) && (!_target->isCDA))
        {
            if(_target->pbonus > 0)
                spell->source->power = _target->power - _target->pbonus;
            else
                spell->source->power = _target->power + abs(_target->pbonus);
            if(_target->tbonus > 0)
            {
                spell->source->toughness = _target->toughness - _target->tbonus;
                spell->source->life = _target->toughness - _target->tbonus;
            }
            else
            {
                spell->source->toughness = _target->toughness + abs(_target->tbonus);
                spell->source->life = _target->toughness + abs(_target->tbonus);
            }
        }
        list<int>::iterator it;
        for (it = awith.begin(); it != awith.end(); it++)
        {//there must be a layer of temporary abilities and original abilities
            spell->source->basicAbilities[*it] = 1;
        }
        for (it = colors.begin(); it != colors.end(); it++)
        {
            spell->source->setColor(*it);
        }
        for (it = typesToAdd.begin(); it != typesToAdd.end(); it++)
        {
            spell->source->addType(*it);
        }
        if(spell->source->TokenAndAbility)
        {//the source copied a token with andAbility
            MTGAbility * TokenandAbilityClone = spell->source->TokenAndAbility->clone();
            TokenandAbilityClone->target = spell->source;
            if(spell->source->TokenAndAbility->oneShot)
            {
                TokenandAbilityClone->resolve();
                SAFE_DELETE(TokenandAbilityClone);
            }
            else
            {
                TokenandAbilityClone->addToGame();
            }
        }
        if(battleReady)
        {
            spell->source->summoningSickness = 0;
            spell->source->tap();
            spell->source->setAttacker(1);
        }
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = spell->source;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        if(options.find("notrigger") == string::npos){ // check if the @tokencreated trigger has to be activated or not
            WEvent * e = NEW WEventTokenCreated(spell->source);
            spell->getObserver()->receiveEvent(e); // triggers the @tokencreated event for any other listener.
        }
        delete spell;
    }
    return 1;

}

const string AACloner::getMenuText()
{
    if (who == 1)
        return "Clone For Opponent";
    return "Clone";
}

ostream& AACloner::toString(ostream& out) const
{
    out << "AACloner ::: with : ?" // << abilities
            << " (";
    return ActivatedAbility::toString(out) << ")";
}

AACloner * AACloner::clone() const
{
    AACloner * a = NEW AACloner(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}
AACloner::~AACloner()
{
    SAFE_DELETE(andAbility);
}

// Cast/Play Restriction modifier
ACastRestriction::ACastRestriction(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, TargetChooser * _restrictionsScope, WParsedInt * _value, bool _modifyExisting, int _zoneId, int who) :
   AbilityTP(observer, _id, card, _target, who), restrictionsScope(_restrictionsScope), value(_value), modifyExisting(_modifyExisting),zoneId(_zoneId)
{
    existingRestriction = NULL;
    targetPlayer = NULL;
}

int ACastRestriction::addToGame()
{
    Targetable * _target = getTarget();
    targetPlayer = getPlayerFromTarget(_target);
    if (!targetPlayer)
        return 0;

    if (modifyExisting)
    {
        //For now the only modifying rule is the one for lands, so this is hardcoded here.
        //This means that a modifying rule for anything lands will actually modify the lands rule.
        //In the future, we need a way to "identify" rules that modify an existing restriction, probably by doing a comparison of the TargetChoosers
        existingRestriction = targetPlayer->game->playRestrictions->getMaxPerTurnRestrictionByTargetChooser(restrictionsScope);
        if(existingRestriction && existingRestriction->maxPerTurn != MaxPerTurnRestriction::NO_MAX)
            existingRestriction->maxPerTurn += value->getValue();
    }
    else
    {
        TargetChooser * _tc = restrictionsScope->clone();
        existingRestriction = NEW MaxPerTurnRestriction(_tc, value->getValue(), MTGGameZone::intToZone(zoneId, targetPlayer));
        targetPlayer->game->playRestrictions->addRestriction(existingRestriction);

    }
    AbilityTP::addToGame();
    return 1;
}

int ACastRestriction::destroy()
{
    if (!existingRestriction)
        return 0;

    if (modifyExisting)
    {
        if(existingRestriction->maxPerTurn != MaxPerTurnRestriction::NO_MAX)
            existingRestriction->maxPerTurn -= value->getValue();
    }
    else
    {
         targetPlayer->game->playRestrictions->removeRestriction(existingRestriction);
         SAFE_DELETE(existingRestriction);
    }
    return 1;
}

const string ACastRestriction::getMenuText()
{
    if (modifyExisting)
        return "Additional Lands"; //hardoced because only the lands rule allows to modify existing rule for now
    return "Cast Restriction";
}

ACastRestriction * ACastRestriction::clone() const
{
    ACastRestriction * a = NEW ACastRestriction(*this);
    a->value = NEW WParsedInt(*(a->value));
    a->restrictionsScope = restrictionsScope->clone();
    return a;
}

ACastRestriction::~ACastRestriction()
{
    SAFE_DELETE(value);
    SAFE_DELETE(restrictionsScope);
}


AInstantCastRestrictionUEOT::AInstantCastRestrictionUEOT(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, TargetChooser * _restrictionsScope, WParsedInt * _value, bool _modifyExisting, int _zoneId, int who) :
    InstantAbilityTP(observer, _id, card, _target, who)
{
    ability = NEW ACastRestriction(observer, _id, card, _target, _restrictionsScope, _value, _modifyExisting,  _zoneId, who);
}

int AInstantCastRestrictionUEOT::resolve()
{
    ACastRestriction * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}
const string AInstantCastRestrictionUEOT::getMenuText()
{
    return ability->getMenuText();
}

AInstantCastRestrictionUEOT * AInstantCastRestrictionUEOT::clone() const
{
    AInstantCastRestrictionUEOT * a = NEW AInstantCastRestrictionUEOT(*this);
    a->ability = this->ability->clone();
    return a;
}

AInstantCastRestrictionUEOT::~AInstantCastRestrictionUEOT()
{
    SAFE_DELETE(ability);
}

//AAMover
AAMover::AAMover(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, string dest,string newName, ManaCost * _cost, bool undying, bool persist) :
    ActivatedAbility(observer, _id, _source, _cost, 0), destination(dest),named(newName),undying(undying),persist(persist)
{
    if (_target)
        target = _target;
    andAbility = NULL;
    if(!named.size() && source->controller()->isAI())
        named = overrideNamed(destination);
    necro = false;
}

MTGGameZone * AAMover::destinationZone(Targetable * target)
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(destination == "previousbattlefield")
    {
        if(_target->previousController)
            return _target->previousController->inPlay();
        else
            return _target->controller()->inPlay();
    }
    return MTGGameZone::stringToZone(game, destination, source, _target);
}

int AAMover::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be moved to any zone, they will follow the fate of top-card
        if(necro)
            _target->basicAbilities[Constants::NECROED] = 1;
        Player* p = _target->controller();
        if (p)
        {
            MTGGameZone * fromZone = _target->getCurrentZone();
            MTGGameZone * destZone = destinationZone(target);

            //inplay is a special zone !
            for (int i = 0; i < 2; i++)
            {
                if (!_target->isSorceryorInstant() && 
                    !_target->hasSubtype(Subtypes::TYPE_AURA) && 
                    destZone == game->players[i]->game->inPlay && 
                    fromZone != game->players[i]->game->inPlay && 
                    fromZone != game->players[i]->opponent()->game->inPlay)
                {
                    MTGCardInstance * copy = game->players[i]->game->putInZone(_target, fromZone, game->players[i]->game->temp);
                    Spell * spell = NEW Spell(game, copy);
                    spell->resolve();
                    if(andAbility)
                    {
                        MTGAbility * andAbilityClone = andAbility->clone();
                        andAbilityClone->target = spell->source;
                        if(andAbility->oneShot)
                        {
                            andAbilityClone->resolve();
                            SAFE_DELETE(andAbilityClone);
                        }
                        else
                        {
                            andAbilityClone->addToGame();
                        }
                    }
                    if(persist)
                        spell->source->counters->addCounter(-1,-1);
                    if(undying)
                        spell->source->counters->addCounter(1,1);
                    delete spell;
                    return 1;
                }
                if (destZone == game->players[i]->game->graveyard && fromZone == game->players[i]->game->hand)
                {
                //movers that take a card from hand and place them in graveyard are always discards. we send an event for it here.

                    WEvent * e = NEW WEventCardDiscard(_target);
                    game->receiveEvent(e);
                }

            }

            if(_target->hasSubtype(Subtypes::TYPE_AURA) && (destZone == game->players[0]->game->inPlay || destZone == game->players[1]->game->inPlay))
            {//put into play aura if there is no valid targets then it will be in its current zone
                MTGAbility *a = NEW AACastCard(game, game->mLayers->actionLayer()->getMaxId(), _target, _target,false,false,false,"","Put in play",false,true);
                a->oneShot = false;
                a->canBeInterrupted = false;
                a->addToGame();
                if(andAbility && _target->next)
                {//if successful target->next should be valid
                    MTGAbility * andAbilityClone = andAbility->clone();
                    andAbilityClone->target = _target->next;
                    if(andAbility->oneShot)
                    {
                        andAbilityClone->resolve();
                        SAFE_DELETE(andAbilityClone);
                    }
                    else
                    {
                        andAbilityClone->addToGame();
                    }
                }
            }
            else
            {
                if(_target->isSorceryorInstant() && (destZone == game->players[0]->game->inPlay || destZone == game->players[1]->game->inPlay))
                {
                    if(andAbility)
                    {
                        if(!dynamic_cast<AAFlip *>(andAbility))
                            return 0;
                    }
                    else
                        return 0;
                }
                MTGCardInstance *newTarget = p->game->putInZone(_target, fromZone, destZone);
                /*while(_target->next)
                    _target = _target->next;*/
                if(newTarget)
                {
                    if(necro)
                        newTarget->basicAbilities[Constants::NECROED] = 1;
                    if(andAbility)
                    {
                        MTGAbility * andAbilityClone = andAbility->clone();
                        andAbilityClone->target = newTarget;
                        if(andAbility->oneShot)
                        {
                            andAbilityClone->resolve();
                            SAFE_DELETE(andAbilityClone);
                        }
                        else
                        {
                            andAbilityClone->addToGame();
                        }
                    }
                }
            }
            return 1;
        }
    }
    return 0;
}

string AAMover::overrideNamed(string destination)
{
    string name = "Move";
    if(destination.size())
    {
        if(destination.find("library") != string::npos)
            name = "Put in Library";
        else if(destination.find("hand") != string::npos)
            name = "Put in Hand";
        else if(destination.find("exile") != string::npos)
            name = "Put in Exile";
        else if(destination.find("removedfromgame") != string::npos)
            name = "Put in Exile";
        else if(destination.find("graveyard") != string::npos)
            name = "Put in Graveyard";
        else if(destination.find("previous") != string::npos)
            name = "Previous Zone";
        else if(destination.find("inplay") != string::npos)
            name = "Put in Play";
        else if(destination.find("battlefield") != string::npos)
            name = "Put in Play";
    }
    return name;
}

const string AAMover::getMenuText()
{
    if(named.size())
        return named.c_str();
    return "Move";
}

const char* AAMover::getMenuText(TargetChooser * tc)
{
    if(named.size())
        return named.c_str();
    MTGGameZone * dest = destinationZone();

    for (int i = 0; i < 2; i++)
    {
        // Move card to hand
        if (dest == game->players[i]->game->hand)
        {
            if (tc->targetsZone(game->players[i]->game->inPlay))
                return "Bounce";
            if (tc->targetsZone(game->players[i]->game->graveyard))
                return "Reclaim";
            if (tc->targetsZone(game->opponent()->game->hand))
                return "Steal";
        }

        // Move card to graveyard
        else if (dest == game->players[i]->game->graveyard)
        {
            if (tc->targetsZone(game->players[i]->game->inPlay))
                return "Sacrifice";
            if (tc->targetsZone(game->players[i]->game->hand))
                return "Discard";
            if (tc->targetsZone(game->opponent()->game->hand))
                return "Opponent Discards";
        }

        // move card to library
        else if (dest == game->players[i]->game->library)
        {
            if (tc->targetsZone(game->players[i]->game->graveyard))
                return "Recycle";
            return "Put in Library";
        }

        // move card to battlefield
        else if (dest == game->players[i]->game->battlefield)
        {
            if (tc->targetsZone(game->players[i]->game->graveyard))
                return "Reanimate";
            return "Put in Play";
        }

        // move card into exile
        else if (dest == game->players[i]->game->exile)
        {
            return "Exile";
        }

        // move card from Library
        else if (tc->targetsZone(game->players[i]->game->library))
        {
            return "Fetch";
        }
    }

    return "Move";
}

AAMover * AAMover::clone() const
{
    AAMover * a = NEW AAMover(*this);
    if(andAbility)
    a->andAbility = andAbility->clone();
    return a;
}

AAMover::~AAMover()
{
SAFE_DELETE(andAbility);
}

//random movement of a card from zone to zone
AARandomMover::AARandomMover(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, string _tcs, string _from, string _to) :
    ActivatedAbility(observer, _id, _source, NULL, 0), abilityTC(_tcs),fromZone(_from),toZone(_to)
{
    if (_target)
        target = _target;
    andAbility = NULL;
}

MTGGameZone * AARandomMover::destinationZone(Targetable * target,string zone)
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    return MTGGameZone::stringToZone(game, zone, source, _target);
}

int AARandomMover::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be randomly moved to any zone, they will follow the fate of top-card
        Player* p = _target->controller();
        if (p)
        {
            MTGGameZone * fromDest = destinationZone(target,fromZone);
            MTGGameZone * toDest = destinationZone(target,toZone);

            if (!fromDest->nb_cards)
                return 0;

            TargetChooserFactory tcf(game);
            TargetChooser * rTc = tcf.createTargetChooser(abilityTC, source);
            rTc->targetter = NULL;
            rTc->setAllZones();
            vector<MTGCardInstance*>selectedCards;
            for(unsigned int i = 0; i < fromDest->cards.size();++i)
            {
                if(rTc->canTarget(fromDest->cards[i]))
                    selectedCards.push_back(fromDest->cards[i]);
            }
            SAFE_DELETE(rTc);
            if(!selectedCards.size())
                return 0;
            int r = fromDest->owner->getObserver()->getRandomGenerator()->random() % (selectedCards.size());
            MTGCardInstance * toMove = selectedCards[r];


            //inplay is a special zone !
            for (int i = 0; i < 2; i++)
            {
                if (toDest == game->players[i]->game->inPlay && fromDest != game->players[i]->game->inPlay && fromDest
                        != game->players[i]->opponent()->game->inPlay)
                {
                    MTGCardInstance * copy = game->players[i]->game->putInZone(toMove, fromDest, game->players[i]->game->temp);
                    Spell * spell = NEW Spell(game, copy);
                    spell->resolve();
                    delete spell;
                    return 1;
                }
            }
            MTGCardInstance *newTarget = p->game->putInZone(toMove, fromDest, toDest);
            if(newTarget)
            {
                if(andAbility)
                {
                    MTGAbility * andAbilityClone = andAbility->clone();
                    andAbilityClone->target = newTarget;
                    if(andAbility->oneShot)
                    {
                        andAbilityClone->resolve();
                        SAFE_DELETE(andAbilityClone);
                    }
                    else
                    {
                        andAbilityClone->addToGame();
                    }
                }
            }
            return 1;
        }
    }
    return 0;
}

const string AARandomMover::getMenuText()
{
    return "Dig";
}

AARandomMover * AARandomMover::clone() const
{
    AARandomMover * a = NEW AARandomMover(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AARandomMover::~AARandomMover()
{
    SAFE_DELETE(andAbility);
}

//Random Discard
AARandomDiscarder::AARandomDiscarder(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string nbcardsStr, ManaCost * _cost,
         int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who), nbcardsStr(nbcardsStr)
{
}

int AARandomDiscarder::resolve()
{
    Targetable * _target = getTarget();
    Player * player = getPlayerFromTarget(_target);
    MTGCardInstance * _stored = NULL;

    if(this->source->storedSourceCard)
        _stored = this->source->storedSourceCard;
    else
        _stored = this->source;

    if (player)
    {
        WParsedInt numCards(nbcardsStr, NULL, source);
        for (int i = 0; i < numCards.intValue; i++)
        {
            player->game->discardRandom(player->game->hand, _stored);
        }
    }
            
    return 1;
}

const string AARandomDiscarder::getMenuText()
{
    if(nbcardsStr.size())
    {
        WParsedInt parsedNum(nbcardsStr, NULL, source);
        return _("Discard " + parsedNum.getStringValue() + " at random").c_str();
    }
    return "Discard Random";
}

AARandomDiscarder * AARandomDiscarder::clone() const
{
    return NEW AARandomDiscarder(*this);
}

// Shuffle 
AAShuffle::AAShuffle(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who)
{
    andAbility = NULL;
}

int AAShuffle::resolve()
{
    Player * player = getPlayerFromTarget(getTarget());
    if (player)
    {
        MTGLibrary * library = player->game->library;
        library->shuffle();
    }
    return 1;
}

const string AAShuffle::getMenuText()
{
    return "Shuffle";
}

AAShuffle * AAShuffle::clone() const
{
    AAShuffle * a = NEW AAShuffle(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AAShuffle::~AAShuffle()
{
    SAFE_DELETE(andAbility);
}

// Mulligan 
AAMulligan::AAMulligan(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who)
{
}

int AAMulligan::resolve()
{
    Player * player = getPlayerFromTarget(getTarget());
    if (player)
    {
            player->serumMulligan();
    }
    return 1;
}

const string AAMulligan::getMenuText()
{
    return "Mulligan";
}

AAMulligan * AAMulligan::clone() const
{
    return NEW AAMulligan(*this);
}

// Remove Mana From ManaPool
AARemoveMana::AARemoveMana(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, string manaDesc, int who, bool forceclean) :
    ActivatedAbilityTP(observer, _id, card, _target, NULL, who), forceclean(forceclean)
{
    if (!manaDesc.size())
    {
        DebugTrace("ALL_ABILITIES: AARemoveMana ctor error"); 
        return;
    }
    mRemoveAll = (manaDesc[0] == '*');
    if (mRemoveAll)
        manaDesc = manaDesc.substr(1);
    
    mManaDesc = (manaDesc.size()) ? ManaCost::parseManaCost(manaDesc) : NULL;

}

int AARemoveMana::resolve()
{
    Player * player = getPlayerFromTarget(getTarget());
    if (player)
    {
        ManaPool * manaPool = player->getManaPool();
        if (mRemoveAll)
        {
            if (mManaDesc) // Remove all mana Matching a description
            {
                for (int i = 0; i < Constants::NB_Colors; i++)
                {
                    if (mManaDesc->hasColor(i))
                        manaPool->removeAll(i);
                }
            }
            else //Remove all mana
            {
                if(game->getCurrentGamePhase() != MTG_PHASE_ENDOFTURN && !forceclean)
                {
                    if (player->doesntEmpty->getConvertedCost() && !player->poolDoesntEmpty->getConvertedCost())
                    {
                        ManaCost * toRemove =  manaPool->Diff(player->doesntEmpty);
                        player->getManaPool()->pay(toRemove);
                        delete(toRemove);
                        return 1;
                    }
                    else if(!player->doesntEmpty->getConvertedCost() && player->poolDoesntEmpty->getConvertedCost())
                    {
                        ManaCost * toSave = NEW ManaCost();
                        for(int k = Constants::MTG_COLOR_ARTIFACT; k < Constants::NB_Colors;k++)
                        {
                            if(player->poolDoesntEmpty->getCost(k))
                                toSave->add(k,manaPool->getCost(k));
                        }
                        player->getManaPool()->pay(manaPool->Diff(toSave));
                        delete(toSave);
                        return 1;
                    }
                    else if(player->doesntEmpty->getConvertedCost() && player->poolDoesntEmpty->getConvertedCost())
                    {
                        ManaCost * toSave = NEW ManaCost();
                        for(int k = Constants::MTG_COLOR_ARTIFACT; k < Constants::NB_Colors;k++)
                        {
                            if(player->poolDoesntEmpty->getCost(k))
                            {
                                toSave->add(k,manaPool->getCost(k));//save the whole amount of k;
                            }
                            else if(player->doesntEmpty->getCost(k))
                            {
                                toSave->add(k,player->doesntEmpty->getCost(k));//save the amount of doesnt empty
                            }
                        }
                        player->getManaPool()->pay(manaPool->Diff(toSave));//remove the manacost equal to the difference of toSave and the manapool.
                        delete(toSave);
                        return 1;
                    }
                    manaPool->Empty();
                } 
                else if(game->getCurrentGamePhase() == MTG_PHASE_ENDOFTURN && !forceclean)
                {
                    if(player->poolDoesntEmpty->getConvertedCost())
                    {
                        ManaCost * toSave = NEW ManaCost();
                        for(int k = Constants::MTG_COLOR_ARTIFACT; k < Constants::NB_Colors;k++)
                        {
                            if(player->poolDoesntEmpty->getCost(k))
                                toSave->add(k,manaPool->getCost(k));
                        }
                        player->getManaPool()->pay(manaPool->Diff(toSave));
                        delete(toSave);
                        return 1;
                    }
                    manaPool->Empty();
                }
                else
                    manaPool->Empty();
            }
        }
        else //remove a "standard" mana Description
        {
            manaPool->pay(mManaDesc); //Changed because the mana icons were not disappearing.
            //((ManaCost *)manaPool)->remove(mManaDesc); //why do I have to cast here?
        }
    }
    return 1;
}

const string AARemoveMana::getMenuText()
{
    if (mRemoveAll && !mManaDesc)
        return "Empty Manapool";
    return "Remove Mana";
}

AARemoveMana * AARemoveMana::clone() const
{
    AARemoveMana * a = NEW AARemoveMana(*this);
    a->mManaDesc = mManaDesc ? NEW ManaCost(mManaDesc) : NULL;
    return a;
}

AARemoveMana::~AARemoveMana()
{
    SAFE_DELETE(mManaDesc);
}

//Bestow
ABestow::ABestow(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, id, card, _cost, 0)
{
    target = _target;
    aType = MTGAbility::TAPPER;
    _card = card;
}

int ABestow::resolve()
{
    if (target)
    {
        if (_card->hasType("creature"))
        {
            _card->removeType("creature");
            _card->addType("aura");
        }
        _card->target = (MTGCardInstance*)target;
        _card->isBestowed = true;
    }
    return 1;
}

const string ABestow::getMenuText()
{
    return "Bestow";
}

ABestow * ABestow::clone() const
{
    return NEW ABestow(*this);
}

//Tapper
AATapper::AATapper(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost, bool sendNoEvent) :
    ActivatedAbility(observer, id, card, _cost, 0),_sendNoEvent(sendNoEvent)
{
    target = _target;
    aType = MTGAbility::TAPPER;
    andAbility = NULL;
}

int AATapper::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be tapped, they will follow the fate of top-card
        while (_target->next)
            _target = _target->next; //This is for cards such as rampant growth
        _target->tap(_sendNoEvent);
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
    }
    return 1;
}

const string AATapper::getMenuText()
{
    return "Tap";
}

AATapper * AATapper::clone() const
{
    AATapper * a = NEW AATapper(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AATapper::~AATapper()
{
    SAFE_DELETE(andAbility);
}

//AA Untapper
AAUntapper::AAUntapper(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, id, card, _cost, 0)
{
    target = _target;
    aType = MTGAbility::UNTAPPER;
    andAbility = NULL;
}

int AAUntapper::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be untapped, they will follow the fate of top-card
        while (_target->next)
            _target = _target->next; //This is for cards such as rampant growth
        _target->untap();
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
    }
    return 1;
}

const string AAUntapper::getMenuText()
{
    return "Untap";
}

AAUntapper * AAUntapper::clone() const
{
    AAUntapper * a = NEW AAUntapper(*this);
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AAUntapper::~AAUntapper()
{
    SAFE_DELETE(andAbility);
}

AAWhatsMax::AAWhatsMax(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance *, ManaCost * _cost, int value) :
    ActivatedAbility(observer, id, card, _cost, 0), value(value)
{
}

int AAWhatsMax::resolve()
{

    if (source)
    {
        source->MaxLevelUp = value;
        source->isLeveler = 1;
    }
    return 1;
}

AAWhatsMax * AAWhatsMax::clone() const
{
    return NEW AAWhatsMax(*this);
}
//set X value
AAWhatsX::AAWhatsX(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance *, int value, MTGAbility * _costRule) :
    ActivatedAbility(observer, id, card, NULL, 0), value(value),costRule(_costRule)
{
}

int AAWhatsX::resolve()
{
    if (source)
    {
        source->setX = value;
        
    }
    costRule->reactToClick(source);
    return 1;
}

AAWhatsX * AAWhatsX::clone() const
{
    return NEW AAWhatsX(*this);
}
//count objects on field before doing an effect
AACountObject::AACountObject(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance *, ManaCost * _cost, string value) :
    ActivatedAbility(observer, id, card, _cost, 0), value(value)
{
}

int AACountObject::resolve()
{

    if (source)
    {
        int amount = 0;
        WParsedInt * use = NEW WParsedInt(value, NULL, source);
        amount = use->getValue();
        source->CountedObjects = amount;
        SAFE_DELETE(use);
    }
    return 1;
}

AACountObject * AACountObject::clone() const
{
    return NEW AACountObject(*this);
}
//count objects on field before doing an effect
AACountObjectB::AACountObjectB(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance *, ManaCost * _cost, string value) :
    ActivatedAbility(observer, id, card, _cost, 0), value(value)
{
}

int AACountObjectB::resolve()
{

    if (source)
    {
        int amount = 0;
        WParsedInt * use = NEW WParsedInt(value, NULL, source);
        amount = use->getValue();
        source->CountedObjectsB = amount;
        SAFE_DELETE(use);
    }
    return 1;
}

AACountObjectB * AACountObjectB::clone() const
{
    return NEW AACountObjectB(*this);
}

// Win Game
AAWinGame::AAWinGame(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who)
{
}

int AAWinGame::resolve()
{
    Player * p = getPlayerFromDamageable((Damageable *) getTarget());
    if (!p)
        return 0;

    bool canwin = true;

    MTGGameZone * z = p->opponent()->game->inPlay;
    int nbcards = z->nb_cards;

    for (int i = 0; i < nbcards; i++)
    {
        MTGCardInstance * c = z->cards[i];
        if (c->has(Constants::CANTLOSE))
        {
            canwin = false;
            break;
        }
    }

    if (canwin)
    {
        MTGGameZone * k = p->game->inPlay;
        int onbcards = k->nb_cards;
        for (int m = 0; m < onbcards; ++m)
        {
            MTGCardInstance * e = k->cards[m];
            if (e->has(Constants::CANTWIN))
            {
                canwin = false;
                break;
            }
        }
    }

    if (canwin)
    {
        game->setLoser(p->opponent());
    }

    return 1;
}

const string AAWinGame::getMenuText()
{
    return "Win Game";
}

AAWinGame * AAWinGame::clone() const
{
    return NEW AAWinGame(*this);
}

//Generic Abilities

//a new affinity
ANewAffinity::ANewAffinity(GameObserver* observer, int _id, MTGCardInstance * _source, string Tc, string mana) :
MTGAbility(observer, _id, _source), tcString(Tc), manaString(mana)
{
}

void ANewAffinity::Update(float)
{
    testDestroy();
    return;
}

int ANewAffinity::testDestroy()
{
    if(this->source->isInPlay(game))
        return 1;
    return 0;
}
ANewAffinity * ANewAffinity::clone() const
{
    return NEW ANewAffinity(*this);
}

//IfThenEffect
IfThenAbility::IfThenAbility(GameObserver* observer, int _id, MTGAbility * delayedAbility, MTGAbility * delayedElseAbility, MTGCardInstance * _source, Targetable * _target, int type,string Cond) :
InstantAbility(observer, _id, _source),delayedAbility(delayedAbility),delayedElseAbility(delayedElseAbility), type(type),Cond(Cond)
{
    target = _target;
}

int IfThenAbility::resolve()
{
    MTGCardInstance * card = (MTGCardInstance*)source;
    AbilityFactory af(game);
    Targetable* aTarget = (Targetable*)target;
    int checkCond = af.parseCastRestrictions(card,card->controller(),Cond);
    if(Cond.find("cantargetcard(") != string::npos)
    {
        TargetChooser * condTc = NULL;
        vector<string>splitTarget = parseBetween(Cond, "card(", ")");
        if (splitTarget.size())
        {
            TargetChooserFactory tcf(game);
            condTc = tcf.createTargetChooser(splitTarget[1], source);
            condTc->targetter = NULL;
            if(aTarget)
                checkCond = condTc->canTarget(aTarget);
            SAFE_DELETE(condTc);
        }

    }
    MTGAbility * a1 = NULL;
    if((checkCond && type == 1)||(!checkCond && type == 2))
    {
        a1 = delayedAbility->clone();
    }
    else if(delayedElseAbility)
    {
        a1 = delayedElseAbility->clone();
    }
    if (!a1)
        return 0;
    else
    {
        if(a1->target && !dynamic_cast<Player *>(a1->target))
            a1->target = aTarget;

        if(a1->oneShot)
        {
            a1->resolve();
            SAFE_DELETE(a1);
        }
        else
            a1->addToGame();
        return 1;
    }
    return 0;
}

const string IfThenAbility::getMenuText()
{
    return "";
}

IfThenAbility * IfThenAbility::clone() const
{
    IfThenAbility * a = NEW IfThenAbility(*this);
    a->delayedAbility = delayedAbility->clone();
    return a;
}

IfThenAbility::~IfThenAbility()
{
    if(delayedAbility && (std::find(deletedpointers.begin(), deletedpointers.end(), delayedAbility) == deletedpointers.end())) {
        deletedpointers.push_back(delayedAbility); // Fix to avoid crash on May abilities nested in IfThenElse Abilities.
        SAFE_DELETE(delayedAbility);
    }
    if(delayedElseAbility && (std::find(deletedpointers.begin(), deletedpointers.end(), delayedElseAbility) == deletedpointers.end())) {
        deletedpointers.push_back(delayedElseAbility); // Fix to avoid crash on May abilities nested in IfThenElse Abilities.
        SAFE_DELETE(delayedElseAbility);
    }
}

//May Abilities
MayAbility::MayAbility(GameObserver* observer, int _id, MTGAbility * _ability, MTGCardInstance * _source, bool must,string _cond) :
    MTGAbility(observer, _id, _source), NestedAbility(_ability), must(must), Cond(_cond)
{
    triggered = 0;
    mClone = NULL;
}

void MayAbility::Update(float dt)
{
    MTGAbility::Update(dt);
    if (!triggered && !game->getCurrentTargetChooser() && (!game->mLayers->actionLayer()->menuObject||game->mLayers->actionLayer()->menuObject == source))
    {
        triggered = 1;
        if(Cond.size())
        {
            AbilityFactory af(game);
            int checkCond = af.parseCastRestrictions(source,source->controller(),Cond);
            if(!checkCond)
            {
                return;
            }
        }
        if (TargetAbility * ta = dynamic_cast<TargetAbility *>(ability))
        {
            if (!ta->getActionTc()->validTargetsExist() || ta->getActionTc()->maxtargets == 0)
                return;
        }
        game->mLayers->actionLayer()->setMenuObject(source, must);
        previousInterrupter = game->isInterrupting;
        game->mLayers->stackLayer()->setIsInterrupting(source->controller(), false);
    }
}

const string MayAbility::getMenuText()
{
    return ability->getMenuText();
}

int MayAbility::testDestroy()
{
    if (!triggered)
        return 0;
    if (game->mLayers->actionLayer()->menuObject)
        return 0;
    if (game->mLayers->actionLayer()->getIndexOf(mClone) != -1)
        return 0;
    if(game->currentPlayer == source->controller() && game->isInterrupting == source->controller() && dynamic_cast<AManaProducer*>(AbilityFactory::getCoreAbility(ability)))
        //if its my turn, and im interrupting myself(why?) then set interrupting to previous interrupter if the ability was a manaability
        //special case since they don't use the stack.
        game->mLayers->stackLayer()->setIsInterrupting(previousInterrupter, false);
    return 1;
}

int MayAbility::isReactingToTargetClick(Targetable * card)
{
    if (card == source)
    {
        if(Cond.size())
        {
            AbilityFactory af(game);
            int checkCond = af.parseCastRestrictions(source,source->controller(),Cond);
            if(!checkCond)
            {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

int MayAbility::reactToTargetClick(Targetable * object)
{
    mClone = ability->clone();
    mClone->addToGame();
    mClone->forceDestroy = 1;
    return mClone->reactToTargetClick(object);
}

MayAbility * MayAbility::clone() const
{
    MayAbility * a = NEW MayAbility(*this);
    a->ability = ability->clone();
    return a;
}

MayAbility::~MayAbility()
{
    SAFE_DELETE(ability);
}

//Menu building ability Abilities
MenuAbility::MenuAbility(GameObserver* observer, int _id, Targetable * mtarget, MTGCardInstance * _source, bool must,vector<MTGAbility*>abilities,Player * who, string newName) :
MayAbility(observer, _id,NULL,_source,must), must(must),abilities(abilities),who(who),newNameString(newName)
{
    triggered = 0;
    mClone = NULL;
    this->target = mtarget;
    removeMenu = false;
    vector<ManaCost*>optionalCost = vector<ManaCost*>();
    toPay = NULL;
    processed = false;
}

bool MenuAbility::CheckUserInput(JButton key)
{
    if (game->mExtraPayment && key == JGE_BTN_SEC)
    {
        if(toPay && toPay->extraCosts == game->mExtraPayment)
        {
            //the user cancelled the paidability. fireAbility() on the second menu item.
            //paidability will always occupy the abilities[0]; in the vector.
            if(abilities.size() > 1)
            {
                abilities[1]->target = abilities[0]->target;
                abilities[1]->fireAbility();
            }
        }
        return false;
    }
    return false;
}

void MenuAbility::Update(float dt)
{
    MTGAbility::Update(dt);
    ActionLayer * object = game->mLayers->actionLayer();
    if(toPay && game->mExtraPayment && !processed)
    {
        if(game->mExtraPayment->isPaymentSet() && game->mExtraPayment->canPay() )
        {
            if (game->mExtraPayment->costs.size())
            {
                if (game->mExtraPayment->costs[0]->costToPay)
                {
                    ManaCost * diff = game->mExtraPayment->costs[0]->costToPay;
                    ManaCost * c = source->controller()->getManaPool()->Diff(diff);
                    source->X = c->getCost(Constants::NB_Colors);
                    delete c;
                }
            }

            game->mExtraPayment->doPay();
            game->mLayers->actionLayer()->reactToClick(game->mExtraPayment->action, game->mExtraPayment->source);
            game->mExtraPayment = NULL;
            processAbility();
            return;
        }

    }
    if (!triggered && !object->menuObject && !object->getCurrentTargetChooser())
    {

        triggered = 1;
        object->currentActionCard = (MTGCardInstance*)this->target;
        if (TargetAbility * ta = dynamic_cast<TargetAbility *>(ability))
        {
            if (!ta->getActionTc()->validTargetsExist())
                return;
        }
    }
    if(object->currentActionCard && this->target != object->currentActionCard)
    {
        triggered = 0;
    }
    if(triggered && !game->mExtraPayment && !processed)
    {
        game->mLayers->actionLayer()->setCustomMenuObject(source, must,abilities,newNameString.size()?newNameString.c_str():"");
        previousInterrupter = game->isInterrupting;
        game->mLayers->stackLayer()->setIsInterrupting(source->controller(), false);
    }
}

int MenuAbility::resolve()
{
    this->triggered = 1;
    MTGAbility * a = this;
    return a->addToGame();
}

const string MenuAbility::getMenuText()
{
    if((abilities.size() > 1 && must)||(abilities.size() > 2 && !must))
        return "choose one";
    return "Action";
}

int MenuAbility::testDestroy()
{
    if (game->mExtraPayment)
        return 0;
    if (!removeMenu)
        return 0;
    if (game->mLayers->actionLayer()->menuObject)
        return 0;
    if (game->mLayers->actionLayer()->getIndexOf(mClone) != -1)
        return 0;

    return 1;
}

int MenuAbility::isReactingToTargetClick(Targetable * card){return 0/*MayAbility::isReactingToTargetClick(card)*/;}
int MenuAbility::reactToTargetClick(Targetable * object){return 1;}

int MenuAbility::processAbility()
{
    if(!mClone)
        return 0;
    if(processed)
        return 0;
    if(abilities[0])
    mClone->target = abilities[0]->target;
    if(MayAbility * toCheck = dynamic_cast<MayAbility*>(mClone))
    {
        toCheck->must = true;
        mClone->addToGame();
    }
    else
    {
        mClone->oneShot = true;
        mClone->forceDestroy = 1;
        mClone->canBeInterrupted = false;
        mClone->resolve();
        SAFE_DELETE(mClone);
        if (source->controller() == game->isInterrupting)
            game->mLayers->stackLayer()->cancelInterruptOffer(ActionStack::DONT_INTERRUPT, false);
    }

    processed = true;
    this->forceDestroy = 1;
    removeMenu = true;
    return 1;
}

int MenuAbility::reactToChoiceClick(Targetable * object,int choice,int control)
{
    ActionElement * currentAction = (ActionElement *) game->mLayers->actionLayer()->mObjects[control];
    if(currentAction != (ActionElement*)this)
        return 0;
    if(!abilities.size()||!triggered)
        return 0;
    for(int i = 0;i < int(abilities.size());i++)
    {

        if(choice == i)
            mClone = abilities[choice]->clone();
        else if(!optionalCosts.size())
            SAFE_DELETE(abilities[i]);
        if (mClone && !toPay && optionalCosts.size() && i < int(optionalCosts.size()) && optionalCosts[i])//paidability only supports the first ability as paid for now.
        {
            toPay = NEW ManaCost();
            if (optionalCosts[i]->extraCosts)
                toPay->extraCosts = optionalCosts[i]->extraCosts->clone();
            toPay->addExtraCost(NEW ExtraManaCost(NEW ManaCost(optionalCosts[i])));
            toPay->setExtraCostsAction(this, source);
            game->mExtraPayment = toPay->extraCosts;
            return 0;
        }
    }
    if(!mClone)
    {
        if (source->controller() == game->isInterrupting)
            game->mLayers->stackLayer()->cancelInterruptOffer(ActionStack::DONT_INTERRUPT, false);
        return 0;
    }
    mClone->target = abilities[choice]->target;
    processAbility();
    return reactToTargetClick(object);
}

MenuAbility * MenuAbility::clone() const
{
    MenuAbility * a = NEW MenuAbility(*this);
    a->canBeInterrupted = false;
    if(abilities.size())
    {
        for(int i = 0;i < int(abilities.size());i++)
        {
            a->abilities.push_back(abilities[i]->clone());
            a->optionalCosts.push_back(NEW ManaCost(optionalCosts[i]));
            a->abilities[i]->target = abilities[i]->target;
        }
    }
    else
    a->ability = ability->clone();
    return a;
}

MenuAbility::~MenuAbility()
{
    if(abilities.size())
    {
        for(int i = 0;i < int(abilities.size());i++)
        {
            if(abilities[i])
            {
                AASetColorChosen * chooseA = dynamic_cast<AASetColorChosen *>(abilities[i]);
                if(chooseA && chooseA->abilityAltered)
                    SAFE_DELETE(chooseA->abilityAltered);
                SAFE_DELETE(abilities[i]);
            }
        }
    }
    else
        SAFE_DELETE(ability);
    SAFE_DELETE(toPay);
    //SAFE_DELETE(mClone);//crash fix with generated castcard with pay ability
    if(mClone)
    {
        mClone = NULL;
        delete mClone;
    }
    if(optionalCosts.size())
        for(int i = 0;i < int(optionalCosts.size());i++)
        {
            if(optionalCosts[i])
            {
                SAFE_DELETE(optionalCosts[i]);
            }

        }
}
///
//MultiAbility : triggers several actions for a cost
MultiAbility::MultiAbility(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, card, _cost, 0)
{
    if (_target)
        target = _target;
}

int MultiAbility::Add(MTGAbility * ability)
{
    abilities.push_back(ability);
    return 1;
}

int MultiAbility::resolve()
{
    Targetable * Phaseactiontarget = NULL;
    for (size_t i = 0; i < abilities.size(); ++i)
    {
        if (abilities[i] == NULL)
            continue;
        Targetable * backup = abilities[i]->target;

        if (target && target != source && abilities[i]->target == abilities[i]->source)
        {
            abilities[i]->target = target;
            Phaseactiontarget = target;
        }
        abilities[i]->resolve();
        abilities[i]->target = backup;
        if(Phaseactiontarget && dynamic_cast<APhaseActionGeneric *> (abilities[i]))
            abilities[i]->target = Phaseactiontarget;
    }
    return 1;
}

int MultiAbility::addToGame()
{
    for (size_t i = 0; i < abilities.size(); ++i)
    {
        if (abilities[i] == NULL)
            continue;

        MTGAbility * a = abilities[i]->clone();
        a->target = target;
        a->addToGame();
        clones.push_back(a);
    }
    MTGAbility::addToGame();
    return 1;
}

int MultiAbility::destroy()
{
    for (size_t i = 0; i < clones.size(); ++i)
    {
        //I'd like to call game->removeObserver here instead of using forceDestroy, but I get a weird crash after that, need to investigate a bit
        clones[i]->forceDestroy = 1;
    }
    clones.clear();
    return ActivatedAbility::destroy();
}

const string MultiAbility::getMenuText()
{
    if (abilities.size() && abilities[0])
        return abilities[0]->getMenuText();
    return "";
}

MultiAbility * MultiAbility::clone() const
{
    MultiAbility * a = NEW MultiAbility(*this);
    a->abilities.clear();
    for (size_t i = 0; i < abilities.size(); ++i)
    {
        if(abilities[i])
        a->abilities.push_back(abilities[i]->clone());
    }
    return a;
}

MultiAbility::~MultiAbility()
{
    for (size_t i = 0; i < abilities.size(); ++i)
    {
        SAFE_DELETE(abilities[i]);
    }

    abilities.clear();
}
//Generic Target Ability
GenericTargetAbility::GenericTargetAbility(GameObserver* observer, string newName, string castRestriction, int _id, MTGCardInstance * _source, TargetChooser * _tc, MTGAbility * a,
        ManaCost * _cost, string limit,MTGAbility * sideEffects,string usesBeforeSideEffects, int restrictions, MTGGameZone * dest,string _tcString) :
    TargetAbility(observer, _id, _source, _tc, _cost, restrictions, castRestriction), limit(limit), activeZone(dest),newName(newName),sideEffects(sideEffects),usesBeforeSideEffects(usesBeforeSideEffects),tcString(_tcString)
{
    ability = a;
    MTGAbility * core = AbilityFactory::getCoreAbility(a);
    if (dynamic_cast<AACopier *> (core))
        tc->other = true; //http://code.google.com/p/wagic/issues/detail?id=209 (avoid inifinite loop)
    counters = 0;
}

const string GenericTargetAbility::getMenuText()
{
    if (!ability)
        return "Error";
    if (newName.size())
        return newName.c_str();

    //Special case for move
    MTGAbility * core = AbilityFactory::getCoreAbility(ability);
    if (AAMover * move = dynamic_cast<AAMover *>(core))
        return (move->getMenuText(tc));
    return ability->getMenuText();

}

int GenericTargetAbility::resolve()
{
    counters++;
    tc->done = false;
    if(sideEffects && usesBeforeSideEffects.size())
    {
        WParsedInt * use = NEW WParsedInt(usesBeforeSideEffects.c_str(),NULL,source);
        uses = use->getValue();
        delete use;
        if(counters == uses)
        {
            sa = sideEffects->clone();
            sa->target = this->target;
            sa->source = this->source;
            if(sa->oneShot)
            {
                sa->fireAbility();
            }
            else
            {
                GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), sa);
                wrapper->addToGame();
            }
        }
    }
    return TargetAbility::resolve();
}

int GenericTargetAbility::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    limitPerTurn = 0;
    if(limit.size())
    {
        WParsedInt value(limit.c_str(),NULL,source);
        limitPerTurn = value.getValue();
    }
    if (limitPerTurn && counters >= limitPerTurn)
        return 0;
    if(tcString.size() && !tc->targetListSet())
    {
        TargetChooser * current = this->getActionTc();
        TargetChooserFactory tcf(game);
        TargetChooser *refreshed = tcf.createTargetChooser(tcString, source, this);
        refreshed->setTargetsTo(current->getTargetsFrom());
        this->setActionTC(refreshed);
        SAFE_DELETE(current);
    }
    return TargetAbility::isReactingToClick(card, mana);
}

void GenericTargetAbility::Update(float dt)
{
    if (newPhase != currentPhase && newPhase == MTG_PHASE_AFTER_EOT)
    {
        counters = 0;
    }
    TargetAbility::Update(dt);
}

int GenericTargetAbility::testDestroy()
{
    if (!activeZone)
        return TargetAbility::testDestroy();
    if (activeZone->hasCard(source))
        return 0;
    return 1;

}

GenericTargetAbility * GenericTargetAbility::clone() const
{
    GenericTargetAbility * a = NEW GenericTargetAbility(*this);
    a->ability = ability->clone();
    return a;
}

GenericTargetAbility::~GenericTargetAbility()
{
    SAFE_DELETE(ability);
    SAFE_DELETE(sideEffects);
}

//Alter Cost
AAlterCost::AAlterCost(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, int amount, int type) :
MTGAbility(observer, id, source, target), amount(amount), type(type)
{
    manaReducer = source;
}

int AAlterCost::addToGame()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(!_target || _target->hasType("land"))
    {
        this->forceDestroy = 1;
        return MTGAbility::addToGame();
    }
    if (amount > 0)
    {
        if(!_target->getIncreasedManaCost()->getConvertedCost())
        {
            ManaCost * increased = NEW ManaCost();
            _target->getIncreasedManaCost()->copy(increased);
            delete increased;

        }
        _target->getIncreasedManaCost()->add(type,amount);
    }
    else
    {
        if(!_target->getReducedManaCost()->getConvertedCost())
        {
            ManaCost * reduced = NEW ManaCost();
            _target->getReducedManaCost()->copy(reduced);
            delete reduced;
        }
        _target->getReducedManaCost()->add(type,abs(amount));
    }
    return MTGAbility::addToGame();
}

int AAlterCost::destroy()
{
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if(!this->manaReducer->isInPlay(game))
    {
        if (amount > 0)
        {
            _target->getIncreasedManaCost()->remove(type,amount);
            refreshCost(_target);//special case for 0 cost.
        }
        else
        {
            _target->getReducedManaCost()->remove(type,abs(amount));
            refreshCost(_target);//special case for 0 cost.
        }
        return MTGAbility::testDestroy();
    }
    return 0;
}

int AAlterCost::testDestroy()
{
    if(!this->manaReducer->isInPlay(game))
    {
        return MTGAbility::testDestroy();
    }
    return 0;
}
void AAlterCost::refreshCost(MTGCardInstance * card)
{
    ManaCost * original = NEW ManaCost();
    original->copy(card->model->data->getManaCost());
    if(card->getIncreasedManaCost()->getConvertedCost())
        original->add(card->getIncreasedManaCost());
    if(card->getReducedManaCost()->getConvertedCost())
        original->remove(card->getReducedManaCost());
    card->getManaCost()->copy(original);
    delete original;
    return;
}
void AAlterCost::increaseTheCost(MTGCardInstance * card)
{
    if(card->getIncreasedManaCost()->getConvertedCost())
    {
        for(int k = Constants::MTG_COLOR_ARTIFACT; k < Constants::NB_Colors;k++)
        {
            card->getManaCost()->add(k,card->getIncreasedManaCost()->getCost(k));
        }
    }
    return;
}

void AAlterCost::decreaseTheCost(MTGCardInstance * card)
{
    if(card->getReducedManaCost()->getConvertedCost())
    {
        for(int k = Constants::MTG_COLOR_ARTIFACT; k < Constants::NB_Colors;k++)
        {
            card->getManaCost()->remove(k,card->getReducedManaCost()->getCost(k));
        }
    }
    return;
}

AAlterCost * AAlterCost::clone() const
{
    return NEW AAlterCost(*this);
}

AAlterCost::~AAlterCost()
{
}

// ATransformer
ATransformer::ATransformer(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, string stypes, string sabilities,string newpower,bool newpowerfound,string newtoughness,bool newtoughnessfound,vector<string> newAbilitiesList,bool newAbilityFound,bool aForever,bool aUntilNext,bool aUntilEndNext,string _menu) :
    MTGAbility(observer, id, source, target),newpower(newpower),newpowerfound(newpowerfound),newtoughness(newtoughness),newtoughnessfound(newtoughnessfound),newAbilitiesList(newAbilitiesList),newAbilityFound(newAbilityFound),aForever(aForever),UYNT(aUntilNext),UENT(aUntilEndNext),menutext(_menu)
{

    if (target != source) {
        target->storedSourceCard = source;
    }

    PopulateAbilityIndexVector(abilities, sabilities);
    PopulateColorIndexVector(colors, sabilities);
    if(sabilities.find("chosencolor") != string::npos)
    {
        colors.push_back(source->chooseacolor);
    }
    myCurrentTurn = 1000;
    //this subkeyword adds a color without removing the existing colors.
    removemc = (sabilities.find("removemc") != string::npos);
    removeAllColors = (sabilities.find("removeallcolors") != string::npos);
    addNewColors = (sabilities.find("newcolors") != string::npos);
    remove = (stypes.find("removealltypes") != string::npos);
    removeCreatureSubtypes = (stypes.find("removecreaturesubtypes") != string::npos);
    removeTypes = (stypes.find("removetypes") != string::npos);
    removeAllSubtypes = (stypes.find("removeallsubtypes") != string::npos);

    if (stypes.find("allsubtypes") != string::npos || stypes.find("removecreaturesubtypes") != string::npos)
    {  
        const vector<string> values = MTGAllCards::getCreatureValuesById();
        for (size_t i = 0; i < values.size(); ++i)
            types.push_back(MTGAllCards::findType(values.at(i)));
    }
    else
    {
        if(stypes.find("chosentype") != string::npos)
        {
            stypes = source->chooseasubtype;
        }
        if(stypes.find("evicttypes") == string::npos) // The imprinted types and subtypes will be replaced later.
            PopulateSubtypesIndexVector(types, stypes);
    }

    menu = stypes;
}

int ATransformer::addToGame()
{
    if(UYNT || UENT){
        myCurrentTurn = game->turn;
        controllerId = source->controller()->getId();
    }
    MTGCardInstance * _target = NULL;
        Interruptible * action = (Interruptible *) target;
    if (action && action->type == ACTION_SPELL && action->state == NOT_RESOLVED)
    {
        Spell * spell = (Spell *) action;
        _target = spell->source;
        aForever = true;
        //when targeting the stack, set the effect to forever, incase the person does not use it
        //otherwise we will end up with a null pointer on the destroy.
    }
    else
    {
        _target = (MTGCardInstance *) target;
    }

    if (!_target)
    {
        DebugTrace("ALL_ABILITIES: Target not set in ATransformer::addToGame\n");
        return 0;
    }
        
    while (_target->next)
        _target = _target->next;

    for (int j = 0; j < Constants::NB_Colors; j++)
    {
        if (_target->hasColor(j))
            oldcolors.push_back(j);
    }
    for (size_t j = 0; j < _target->types.size(); ++j)
        oldtypes.push_back( _target->types[j]);

    list<int>::iterator it;
    for (it = colors.begin(); it != colors.end(); it++)
    {
        if(!addNewColors)
            _target->setColor(0, 1);
    }
    if (removeAllColors)
    {
        for (it = oldcolors.begin(); it != oldcolors.end(); it++)
        {
            _target->removeColor(*it);
        }
    }
    if (removeTypes)
    {
        //remove the main types from a card, ie: hidden enchantment cycle.
        for (int i = 0; i < Subtypes::LAST_TYPE; ++ i)
            _target->removeType(i,1);
    }
    else if (removeAllSubtypes)
    {
        //remove all the types from a card without removing official supertypes (e.g. Imprisoned in the Moon)
        for (it = oldtypes.begin(); it != oldtypes.end(); it++)
        {
           if(*it != Subtypes::TYPE_LEGENDARY && *it != Subtypes::TYPE_BASIC && *it != Subtypes::TYPE_SNOW && *it != Subtypes::TYPE_WORLD)
                _target->removeType(*it);
        }
    }
    else if (remove)
    {
        for (it = oldtypes.begin(); it != oldtypes.end(); it++)
        {
            _target->removeType(*it);
        }
    }
    else
    {
        if(menu.find("evicttypes") != string::npos)
        {
            menu = "";
            if(source->imprintedCards.size() > 0){
                for (int i = 0; i < ((int)source->imprintedCards.back()->types.size()); i++) // read all the types of the last imprinted card.
                    menu = menu + MTGAllCards::findType(source->imprintedCards.back()->types[i]) + " ";
                menu.erase(menu.find_last_not_of("\t\n\v\f\r ") + 1);
                menu.erase(0, menu.find_first_not_of("\t\n\v\f\r "));
                PopulateSubtypesIndexVector(types, menu);
            }
        }
        for (it = types.begin(); it != types.end(); it++)
        {

            if(removeCreatureSubtypes)
            {
                _target->removeType(*it);
            }
            else if(_target->hasSubtype(*it))
            {
                //we generally don't want to give a creature type creature again
                //all it does is create a sloppy mess of the subtype line on alternative quads
                //also creates instances where a card gained a type from an ability like this one
                //then loses the type through another ability, when this effect is destroyed the creature regains
                //the type, which is wrong.
                dontremove.push_back(*it);
            }
            else
            {
                _target->addType(*it);
            }
        }
    }
    for (it = colors.begin(); it != colors.end(); it++)
    {
        _target->setColor(*it);
    }

    for (it = abilities.begin(); it != abilities.end(); it++)
    {
        _target->basicAbilities.set(*it);
    }

    if(newAbilityFound)
    {
        for (unsigned int k = 0 ; k < newAbilitiesList.size();k++)
        {
            AbilityFactory af(game);
            MTGAbility * aNew = af.parseMagicLine(newAbilitiesList[k], 0, NULL, _target);
            if(!aNew)
                continue;
            GenericTargetAbility * gta = dynamic_cast<GenericTargetAbility*> (aNew);
            if (gta)
            {
                ((GenericTargetAbility *)aNew)->source = _target;
                ((GenericTargetAbility *)aNew)->ability->source = _target;
            }
            GenericActivatedAbility * gaa = dynamic_cast<GenericActivatedAbility*> (aNew);
            if (gaa)
            {
                ((GenericActivatedAbility *)aNew)->source = _target;
                ((GenericActivatedAbility *)aNew)->ability->source = _target;
            }
            MultiAbility * abi = dynamic_cast<MultiAbility*>(aNew);
            if (abi)
            {
                ((MultiAbility *)aNew)->source = _target;
                ((MultiAbility *)aNew)->abilities[0]->source = _target;
            }

            aNew->target = _target;
            aNew->source = (MTGCardInstance *) _target;
            if(aNew->oneShot)
            {
                aNew->resolve();
                delete aNew;
            }
            else
            {
                aNew->addToGame();
                newAbilities[_target].push_back(aNew);
            }
        }
    }
    if(newpowerfound || newtoughnessfound)
        _target->isSettingBase += 1;
    if(newpowerfound )
    {
        WParsedInt * val = NEW WParsedInt(newpower,NULL, source);
        if(_target->isSwitchedPT)
        {
            _target->switchPT(false);
            _target->addbaseP(val->getValue());
            _target->switchPT(true);
        }
        else
            _target->addbaseP(val->getValue());
        delete val;
    }
    if(newtoughnessfound )
    {//we should consider the damage if there is, if you have a 5/5 creature with 1 damage,
     //and you turn it into 1/1, the 1 damage is still there and the creature must die...
     //the toughness is intact but what we see in the game is the life...
        WParsedInt * val = NEW WParsedInt(newtoughness,NULL, source);
        if(_target->isSwitchedPT)
        {
            _target->switchPT(false);
            _target->addbaseT(val->getValue());
            _target->switchPT(true);
        }
        else
            _target->addbaseT(val->getValue());
        delete val;

        if(_target->isCreature() && (_target->hasType(Subtypes::TYPE_BATTLE) || _target->hasType(Subtypes::TYPE_PLANESWALKER)) && _target->life > _target->toughness)
            _target->life = _target->toughness; // Fix for a Planeswalker or a Battle that becomes a creature (e.g. "Spark Rupture").
    }
    //remove manacost
    if(removemc)
        _target->getManaCost()->resetCosts();

    return MTGAbility::addToGame();
}
    
    int ATransformer::reapplyCountersBonus(MTGCardInstance * rtarget,bool , bool toughnessapplied)
    {
        if(!rtarget->counters || !rtarget->counters->counters.size())
            return 0;
        Counter * c = rtarget->counters->counters[0];
        int rNewPower = 0;
        int rNewToughness = 0;
        for (int t = 0; t < rtarget->counters->mCount; t++)
        {
            if (c)
            {
                for(int i = 0;i < c->nb;i++)
                {
                    rNewPower += c->power;
                    rNewToughness += c->toughness;   
                }
            }
            c = rtarget->counters->getNext(c);
        }
        if(toughnessapplied)
            return rNewToughness;
        return rNewPower;
    }

    int ATransformer::testDestroy()
    {
        if(UYNT)
        {
            if(myCurrentTurn != 1000 && game->turn > myCurrentTurn && controllerId == game->currentPlayer->getId())
                return 1;
            return 0; // Fixed an issue when the transformation with uynt is triggered by instant/sorcery or by card that left the battlefield before the ability ending turn.
        } 
        else if(UENT)
        {
            if(myCurrentTurn != 1000 && game->turn > (myCurrentTurn + 1) && controllerId != game->currentPlayer->getId())
                return 1;
            return 0; // Fixed an issue when the transformation with uent is triggered by instant/sorcery or by card that left the battlefield before the ability ending turn.
        }
        return MTGAbility::testDestroy();
    }

int ATransformer::destroy()
{
    if(aForever)
        return 0;
        
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next;
        list<int>::iterator it;
        
        if (!remove)
        {
            for (it = types.begin(); it != types.end(); it++)
            {
                    bool removing = true;
                    for(unsigned int k = 0;k < dontremove.size();k++)
                    {
                        if(dontremove[k] == *it)
                            removing = false;
                    }
                    if(removing)
                        _target->removeType(*it);
            }
            //iterators annoy me :/
        }

        for (it = colors.begin(); it != colors.end(); it++)
        {
            _target->removeColor(*it);
        }

        for (it = abilities.begin(); it != abilities.end(); it++)
        {
            _target->basicAbilities.reset(*it);
        }

        for (it = oldcolors.begin(); it != oldcolors.end(); it++)
        {
            _target->setColor(*it);
        }
        
        if(newpowerfound || newtoughnessfound)
            _target->isSettingBase -= 1;

        if(newpowerfound )
        {
            _target->revertbaseP();
        }
        if(newtoughnessfound )
        {
            _target->revertbaseT();
        }
        if(newAbilityFound)
        {
            for (unsigned int i = 0;i < newAbilities[_target].size(); i++)
            {
                // The mutated cards probably cause a double free error and a crash, so for now they have been exluded...
                if(newAbilities[_target].at(i) && !_target->mutation && _target->currentZone != _target->owner->game->library && !(_target->name == "" && (UYNT || UENT)))
                {
                    newAbilities[_target].at(i)->forceDestroy = 1;
                    newAbilities[_target].at(i)->removeFromGame();
                }
            }
            if (newAbilities.find(_target) != newAbilities.end())
            {
                newAbilities.erase(_target);
            }
        }
        if (remove || removeCreatureSubtypes || removeAllSubtypes)
        {
            for (it = oldtypes.begin(); it != oldtypes.end(); it++)
            {
                if(!_target->hasSubtype(*it))
                    _target->addType(*it);
            }
        }
        ////in the case that we removed or added types to a card, so that it retains its original name when the effect is removed.
        //if(_target->model->data->name.size())//tokens don't have a model name.
        //    _target->setName(_target->model->data->name.c_str());

        //edit: this ability shouldn't have to reset the name on a card becuase removing a subtype changes the name of a land.
        //that should be handled in addType...not here.
        //im sure commenting this out will reintroduce a bug somewhere but it needs to be handled correctly. furthermore, why does adding and removing a type touch the name of a card?
    }
    return 1;
}

const string ATransformer::getMenuText()
{
    if(menutext.size())
        return menutext.c_str();
    string s = menu;
    sprintf(menuText, "Becomes %s", s.c_str());
    return menuText;
}

ATransformer * ATransformer::clone() const
{
    return NEW ATransformer(*this);
}

ATransformer::~ATransformer()
{
}

//ATransformerInstant
ATransformerInstant::ATransformerInstant(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, string types, string abilities,string newpower,bool newpowerfound,string newtoughness,bool newtoughnessfound,vector<string>newAbilitiesList,bool newAbilityFound,bool aForever,bool aUntilNext,bool aUntilEndNext,string _menu) :
    InstantAbility(observer, id, source, target),newpower(newpower),newpowerfound(newpowerfound),newtoughness(newtoughness),newtoughnessfound(newtoughnessfound),newAbilitiesList(newAbilitiesList),newAbilityFound(newAbilityFound),aForever(aForever),UYNT(aUntilNext),UENT(aUntilEndNext),menu(_menu)
{
    ability = NEW ATransformer(game, id, source, target, types, abilities,newpower,newpowerfound,newtoughness,newtoughnessfound,newAbilitiesList,newAbilityFound,aForever,aUntilNext,aUntilEndNext,_menu);
    aType = MTGAbility::STANDARD_BECOMES;
}

int ATransformerInstant::resolve()
{
    ATransformer * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}
const string ATransformerInstant::getMenuText()
{
    if(menu.size())
        return menu.c_str();
    return ability->getMenuText();
}

ATransformerInstant * ATransformerInstant::clone() const
{
    ATransformerInstant * a = NEW ATransformerInstant(*this);
    a->ability = this->ability->clone();
    return a;
}

ATransformerInstant::~ATransformerInstant()
{
    SAFE_DELETE(ability);
}

//P/t ueot
PTInstant::PTInstant(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, WParsedPT * wppt, string s, bool nonstatic) :
InstantAbility(observer, id, source, target), wppt(wppt), s(s), nonstatic(nonstatic)
{
    ability = NEW APowerToughnessModifier(game, id, source, target, wppt, s, nonstatic);
    aType = MTGAbility::STANDARD_PUMP;
    lastTriggeredTurn = source->getObserver()->turn;
}

int PTInstant::resolve()
{
    if(lastTriggeredTurn == source->getObserver()->turn)
        ability->triggers++;
    else
        ability->triggers = 1;
    lastTriggeredTurn = source->getObserver()->turn;
    APowerToughnessModifier * a = ability->clone();
    a->triggers = ability->triggers;
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    ((Damageable *) (this->target))->afterDamage();//additional check the negative pt after resolving..
    return 1;
}

const string PTInstant::getMenuText()
{
    return ability->getMenuText();
}

PTInstant * PTInstant::clone() const
{
    PTInstant * a = NEW PTInstant(*this);
    a->ability = this->ability->clone();
    return a;
}

PTInstant::~PTInstant()
{
    SAFE_DELETE(ability);
}

// ASwapPTUEOT
ASwapPTUEOT::ASwapPTUEOT(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target) :
    InstantAbility(observer, id, source, target)
{
    ability = NEW ASwapPT(observer, id, source, target);
}

int ASwapPTUEOT::resolve()
{
    ASwapPT * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}

const string ASwapPTUEOT::getMenuText()
{
    return ability->getMenuText();
}

ASwapPTUEOT * ASwapPTUEOT::clone() const
{
    ASwapPTUEOT * a = NEW ASwapPTUEOT(*this);
    a->ability = this->ability->clone();
    return a;
}

ASwapPTUEOT::~ASwapPTUEOT()
{
    SAFE_DELETE(ability);
}

//exhange life with targetchooser
AAExchangeLife::AAExchangeLife(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost,
    int who) :
ActivatedAbilityTP(observer, _id, _source, _target, _cost, who)
{
}

int AAExchangeLife::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        if(_target->type_as_damageable == Damageable::DAMAGEABLE_PLAYER && ((Player*)_target)->inPlay()->hasAbility(Constants::CANTCHANGELIFE))
            return 0;
        Player *player = source->controller();
        int oldlife = player->getLife();
        int targetOldLife = _target->getLife();
        int modifier = oldlife > targetOldLife? oldlife - targetOldLife:targetOldLife - oldlife;
        if (_target->type_as_damageable == Damageable::DAMAGEABLE_MTGCARDINSTANCE)
        {
            int increaser = 0;
            MTGCardInstance * card = ((MTGCardInstance*)_target);
            int toughMod = 0;
            targetOldLife <= card->origtoughness?toughMod = card->origtoughness - targetOldLife: toughMod = targetOldLife - card->origtoughness;
            if(oldlife > targetOldLife)
            {
                increaser = oldlife - targetOldLife;
                player->gainOrLoseLife(modifier * -1, source);
                card->addToToughness(increaser+toughMod);
            }
            else
            {
                _target->life = oldlife;
                card->toughness = oldlife;
                player->gainOrLoseLife(modifier, source);
            }

            return 1;
        }
        Player * opponent = (Player*)_target;
        if(oldlife > targetOldLife)
        {
            player->gainOrLoseLife(modifier * -1, source);
            opponent->gainOrLoseLife(modifier, source);
        }
        else
        {
            player->gainOrLoseLife(modifier, source);
            opponent->gainOrLoseLife(modifier * -1, source);
        }
        return 1;
    }
    return 0;
}

const string AAExchangeLife::getMenuText()
{
    return "Exchange life";
}

AAExchangeLife * AAExchangeLife::clone() const
{
    return NEW AAExchangeLife(*this);
}

//ALoseAbilities
ALoseAbilities::ALoseAbilities(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target) :
    MTGAbility(observer, id, source)
{
    target = _target;
}

int ALoseAbilities::addToGame()
{
    if (storedAbilities.size())
    {
        DebugTrace("FATAL:storedAbilities shouldn't be already set inALoseAbilitie\n");
        return 0;
    }
    MTGCardInstance * _target = (MTGCardInstance *)target;

    ActionLayer * al = game->mLayers->actionLayer();


    //Build a list of Lords in game, this is a hack mostly for lands, see below
    vector <ALord *> lordsInGame;
    for (int i = (int)(al->mObjects.size()) - 1; i > 0; i--)      //0 is not a mtgability...hackish
    {
        if (al->mObjects[i])
        {
            MTGAbility * currentAction = (MTGAbility *) al->mObjects[i];
            ALord * l = dynamic_cast<ALord*> (currentAction);
            if(l)
                lordsInGame.push_back(l);
        }
    }

    for (int i = (int)(al->mObjects.size()) - 1; i > 0; i--)      //0 is not a mtgability...hackish
    {
        if (al->mObjects[i])
        {
            MTGAbility * currentAction = (MTGAbility *) al->mObjects[i];
            ALoseAbilities * la = dynamic_cast<ALoseAbilities*> (currentAction);
            if(la)
                continue;
            if (currentAction->source == _target)
            {
                bool canRemove = true;

                //Hack: we don't remove abilities on the card if they are provided by an external lord ability.
                //This is partly to solve the following issues:
                // http://code.google.com/p/wagic/issues/detail?id=647
                // http://code.google.com/p/wagic/issues/detail?id=700
                // But also because "most" abilities granted by lords will actually go away by themselves,
                // based on the fact that we usually remove abilities AND change the type of the card
                //Also in a general way we don't want to remove the card's abilities if it is provided by a Lord,
                //although there is also a problem with us not handling the P/T layer correctly
                for (size_t i = 0; i < lordsInGame.size(); ++i)
                {
                    if (lordsInGame[i]->isParentOf(_target, currentAction))
                    {
                        canRemove = false;
                        break;
                    }
                }
                
                if (canRemove)
                {
                    storedAbilities.push_back(currentAction);
                    al->removeFromGame(currentAction);
                }
            }
        }
    }

    return MTGAbility::addToGame();
}

int ALoseAbilities::destroy()
{
    for (size_t i = 0; i < storedAbilities.size(); ++i)
    {
        MTGAbility * a = storedAbilities[i];
        //OneShot abilities are not supposed to stay in the game for long.
        // If we copied one, something wrong probably happened
        if (a->oneShot)
        {
            DebugTrace("ALLABILITIES: Ability should not be one shot");
            continue;
        }

        //Avoid inifinite loop of removing/putting back abilities
        if (dynamic_cast<ALoseAbilities*> (a))
        {
            DebugTrace("ALLABILITIES: loseability won't be put in the loseability list");
            continue;
        }

         a->addToGame();
    }
    storedAbilities.clear();
    return 1;
}

ALoseAbilities * ALoseAbilities::clone() const
{
    return NEW ALoseAbilities(*this);
}

//ALoseSubtypes
ALoseSubtypes::ALoseSubtypes(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target, int parentType, bool specificType) :
MTGAbility(observer, id, source), parentType(parentType), specificType(specificType)
{
    target = _target;
}

int ALoseSubtypes::addToGame()
{
    if (storedSubtypes.size())
    {
        DebugTrace("FATAL:storedSubtypes shouldn't be already set inALoseSubtypes\n");
        return 0;
    }
    MTGCardInstance * _target = (MTGCardInstance *)target;

    for (int i = ((int)_target->types.size())-1; i >= 0; --i)
    {
        int subtype = _target->types[i];
        if ((!specificType && MTGAllCards::isSubtypeOfType(subtype, parentType)) || (specificType && subtype == parentType)) // added to remove a specific type (e.g. "Conversion").
        {
            storedSubtypes.push_back(subtype);
            _target->removeType(subtype);
        }
    }

    return MTGAbility::addToGame();
}

int ALoseSubtypes::destroy()
{
    MTGCardInstance * _target = (MTGCardInstance *)target;
    for (size_t i = 0; i < storedSubtypes.size(); ++i)
        _target->addType(storedSubtypes[i]);
    storedSubtypes.clear();
    return 1;
}

ALoseSubtypes * ALoseSubtypes::clone() const
{
    return NEW ALoseSubtypes(*this);
}

//APreventDamageTypes
APreventDamageTypes::APreventDamageTypes(GameObserver* observer, int id, MTGCardInstance * source, string to, string from, int type) :
    MTGAbility(observer, id, source), to(to), from(from), type(type)
{
    re = NULL;
}

int APreventDamageTypes::addToGame()
{
    if (re)
    {
        DebugTrace("FATAL:re shouldn't be already set in APreventDamageTypes\n");
        return 0;
    }
    TargetChooserFactory tcf(game);
    TargetChooser *toTc = tcf.createTargetChooser(to, source, this);
    if (toTc)
        toTc->targetter = NULL;
    TargetChooser *fromTc = tcf.createTargetChooser(from, source, this);
    if (fromTc)
        fromTc->targetter = NULL;
    if (type != 1 && type != 2)
    {//not adding this creates a memory leak.
        re = NEW REDamagePrevention(this, fromTc, toTc, -1, false, Damage::DAMAGE_COMBAT);
    }
    else if (type == 1)
    {
        re = NEW REDamagePrevention(this, fromTc, toTc, -1, false, Damage::DAMAGE_ALL_TYPES);
    }
    else if (type == 2)
    {
        re = NEW REDamagePrevention(this, fromTc, toTc, -1, false, Damage::DAMAGE_OTHER);
    }
    game->replacementEffects->add(re);
    return MTGAbility::addToGame();
}

int APreventDamageTypes::destroy()
{
    game->replacementEffects->remove(re);
    SAFE_DELETE(re);
    return 1;
}

APreventDamageTypes * APreventDamageTypes::clone() const
{
    APreventDamageTypes * a = NEW APreventDamageTypes(*this);
    a->re = NULL;
    return a;
}

APreventDamageTypes::~APreventDamageTypes()
{
    SAFE_DELETE(re);
}

//APreventDamageTypesUEOT
APreventDamageTypesUEOT::APreventDamageTypesUEOT(GameObserver* observer, int id, MTGCardInstance * source, string to, string from, int type) :
    InstantAbility(observer, id, source)
{
    ability = NEW APreventDamageTypes(observer, id, source, to, from, type);
}

int APreventDamageTypesUEOT::resolve()
{
    APreventDamageTypes * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}

int APreventDamageTypesUEOT::destroy()
{
    for (size_t i = 0; i < clones.size(); ++i)
    {
        clones[i]->forceDestroy = 0;
    }
    clones.clear();
    return 1;
}

const string APreventDamageTypesUEOT::getMenuText()
{
    return ability->getMenuText();
}

APreventDamageTypesUEOT * APreventDamageTypesUEOT::clone() const
{
    APreventDamageTypesUEOT * a = NEW APreventDamageTypesUEOT(*this);
    a->ability = this->ability->clone();
    return a;
}

APreventDamageTypesUEOT::~APreventDamageTypesUEOT()
{
    SAFE_DELETE(ability);
}

//AVanishing creature also fading
// Comprehensive 702.31a:
// Fading is a keyword that represents two abilities.
// “Fading N” means “This permanent comes into play with N fade counters on it” and
// “At the beginning of your upkeep, remove a fade counter from this permanent.
// If you can’t, sacrifice the permanent.”

// Comprehensive 702.62a:
// Vanishing is a keyword that represents three abilities.
// "Vanishing N" means "This permanent comes into play with N time counters on it,"
// "At the beginning of your upkeep, if this permanent has a time counter on it, remove a time counter from it," and
// "When the last time counter is removed from this permanent, sacrifice it."
AVanishing::AVanishing(GameObserver* observer, int _id, MTGCardInstance * card, ManaCost *, int, int amount, string counterName) :
MTGAbility(observer, _id, source, target),amount(amount),counterName(counterName)
{
    target = card;
    source = card;
    for (int i = 0; i < amount; i++)
        source->counters->addCounter(counterName.c_str(), 0, 0);
}

void AVanishing::Update(float dt)
{
    if (newPhase != currentPhase && source->controller() == game->currentPlayer)
    {
        if (newPhase == MTG_PHASE_UPKEEP)
        {
            bool hasCounters = (source->counters && source->counters->hasCounter(counterName.c_str(), 0, 0));
            if (hasCounters)
            {
                // sacrifice of card with time counter is handled in removeCounter
                source->counters->removeCounter(counterName.c_str(), 0, 0);
            } else
            {
                if (counterName == "fade")
                {
                    MTGCardInstance * beforeCard = source;
                    source->controller()->game->putInGraveyard(source);
                    WEvent * e = NEW WEventCardSacrifice(beforeCard,source);
                    game->receiveEvent(e);
                }
            }
        }
    }
    MTGAbility::Update(dt);
}

int AVanishing::resolve()
{
    return 1;
}

const string AVanishing::getMenuText()
{
    if (counterName.find("fade") != string::npos)
    {
        return "Fading";
    }
    return "Vanishing";
}

AVanishing * AVanishing::clone() const
{
    return NEW AVanishing(*this);
}

AVanishing::~AVanishing()
{
}

//Produce Mana
AProduceMana::AProduceMana(GameObserver* observer, int _id, MTGCardInstance * _source, string ManaDescription) :
MTGAbility(observer, _id, source),ManaDescription(ManaDescription)
{
    source = _source;
    mana[0] = "{g}"; mana[1] = "{u}"; mana[2] = "{r}"; mana[3] = "{b}"; mana[4] = "{w}";
}

int AProduceMana::receiveEvent(WEvent * event)
{
    if(WEventCardTappedForMana * isTappedForMana = dynamic_cast<WEventCardTappedForMana *> (event))
    {
        if ((isTappedForMana->card == source)||(isTappedForMana->card == source->target && ManaDescription == "selectmana"))
            produce();
    }
    return 1;
}

int AProduceMana::produce()
{
    if(ManaDescription == "selectmana")
    {
        //I tried menu ability and vector<MTGAbility*abi> to have a shorter code but it crashes at end of turn...
        //The may ability on otherhand works but the ability is cumulative...
        //This must be wrapped on menuability so we can use it on successions...
        AManaProducer *ap0 = NEW AManaProducer(game, game->mLayers->actionLayer()->getMaxId(), source, source->controller(), ManaCost::parseManaCost(mana[0],NULL,source), NULL, 0,"",false);
        MayAbility *mw0 = NEW MayAbility(game, game->mLayers->actionLayer()->getMaxId(), ap0, source,true);
        MTGAbility *ga0 = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), source,NULL,mw0);

        AManaProducer *ap1 = NEW AManaProducer(game, game->mLayers->actionLayer()->getMaxId(), source, source->controller(), ManaCost::parseManaCost(mana[1],NULL,source), NULL, 0,"",false);
        MayAbility *mw1 = NEW MayAbility(game, game->mLayers->actionLayer()->getMaxId(), ap1, source,true);
        MTGAbility *ga1 = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), source,NULL,mw1);

        AManaProducer *ap2 = NEW AManaProducer(game, game->mLayers->actionLayer()->getMaxId(), source, source->controller(), ManaCost::parseManaCost(mana[2],NULL,source), NULL, 0,"",false);
        MayAbility *mw2 = NEW MayAbility(game, game->mLayers->actionLayer()->getMaxId(), ap2, source,true);
        MTGAbility *ga2 = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), source,NULL,mw2);

        AManaProducer *ap3 = NEW AManaProducer(game, game->mLayers->actionLayer()->getMaxId(), source, source->controller(), ManaCost::parseManaCost(mana[3],NULL,source), NULL, 0,"",false);
        MayAbility *mw3 = NEW MayAbility(game, game->mLayers->actionLayer()->getMaxId(), ap3, source,true);
        MTGAbility *ga3 = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), source,NULL,mw3);

        AManaProducer *ap4 = NEW AManaProducer(game, game->mLayers->actionLayer()->getMaxId(), source, source->controller(), ManaCost::parseManaCost(mana[4],NULL,source), NULL, 0,"",false);
        MayAbility *mw4 = NEW MayAbility(game, game->mLayers->actionLayer()->getMaxId(), ap4, source,true);
        MTGAbility *ga4 = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), source,NULL,mw4);

        ga0->resolve();
        ga1->resolve();
        ga2->resolve();
        ga3->resolve();
        ga4->resolve();
    }
    else
    {
        AManaProducer *amp = NEW AManaProducer(game, game->mLayers->actionLayer()->getMaxId(), source, source->controller(), ManaCost::parseManaCost(ManaDescription,NULL,source), NULL, 0,"",false);
        amp->resolve();
        SAFE_DELETE(amp);//once you call resolve() on a ability, you can safely delete it.
    }
    return 1;
}

const string AProduceMana::getMenuText()
{
    return "Produce Mana";
}

AProduceMana * AProduceMana::clone() const
{
    return NEW AProduceMana(*this);
}

AProduceMana::~AProduceMana()
{
}

//AUpkeep
AUpkeep::AUpkeep(GameObserver* observer, int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost, int restrictions, int _phase,
        int _once,bool Cumulative) :
    ActivatedAbility(observer, _id, card, _cost, restrictions), NestedAbility(a), phase(_phase), once(_once),Cumulative(Cumulative)
{
    paidThisTurn = 0;
    aType = MTGAbility::UPCOST;
    if(Cumulative)
    {
        backupMana = NEW ManaCost();
        backupMana->copy(this->getCost());
        //backupMana->addExtraCosts(this->getCost()->extraCosts);
    }
}

    int AUpkeep::receiveEvent(WEvent * event)
    {
        if (WEventPhaseChange* pe = dynamic_cast<WEventPhaseChange*>(event))
        {
            if (MTG_PHASE_DRAW == pe->to->id && MTG_PHASE_UPKEEP == pe->from->id)
            {
                if (source->controller() == game->currentPlayer && once < 2 && paidThisTurn < 1)
                {
                    ability->resolve();
                }
            }
        }
        return 1;
    }
    
void AUpkeep::Update(float dt)
{
    // once: 0 means always go off, 1 means go off only once, 2 means go off only once and already has.
    if (newPhase != currentPhase && source->controller() == game->currentPlayer && once < 2)
    {
        if (newPhase == MTG_PHASE_BEFORE_BEGIN)
        {
            paidThisTurn = 0;
        }
        else if(newPhase == MTG_PHASE_UPKEEP && Cumulative )
        {
            source->counters->addCounter("age",0,0);
                Counter * targetCounter = NULL;
                currentage = 0;

                if (source->counters && source->counters->hasCounter("age", 0, 0))
                {
                    targetCounter = source->counters->hasCounter("age", 0, 0);
                    currentage = targetCounter->nb - 1;
                }
            if(currentage)
            {
                paidThisTurn = 0;
                this->getCost()->copy(backupMana);
                for(int age = 0;age < currentage;age++)
                {
                    this->getCost()->add(backupMana); 
                    this->getCost()->addExtraCosts(backupMana->extraCosts);
                }
            }
        }
        if (newPhase == phase + 1 && once)
            once = 2;
    }
    ActivatedAbility::Update(dt);
}

int AUpkeep::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    if (currentPhase != phase || paidThisTurn > 0 || once >= 2)
        return 0;
    return ActivatedAbility::isReactingToClick(card, mana);
}

int AUpkeep::resolve()
{
    paidThisTurn += 1;
    return 1;
}

const string AUpkeep::getMenuText()
{
    return "Upkeep";
}

ostream& AUpkeep::toString(ostream& out) const
{
    out << "AUpkeep ::: paidThisTurn : " << paidThisTurn << " (";
    return ActivatedAbility::toString(out) << ")";
}

AUpkeep * AUpkeep::clone() const
{
    AUpkeep * a = NEW AUpkeep(*this);
    a->ability = ability->clone();
    return a;
}

AUpkeep::~AUpkeep()
{
    if(Cumulative)
    {
        SAFE_DELETE(backupMana);
    }
    SAFE_DELETE(ability);
}

//A Phase based Action
APhaseAction::APhaseAction(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance *, string sAbility, int, int _phase,bool forcedestroy,bool next,bool myturn,bool opponentturn,bool once, bool checkexile) :
MTGAbility(observer, _id, card),sAbility(sAbility), phase(_phase),forcedestroy(forcedestroy),next(next),myturn(myturn),opponentturn(opponentturn),once(once),checkexile(checkexile)
{
    abilityId = _id;
    abilityOwner = card->controller();
    psMenuText = "";
    AbilityFactory af(game);
    ability = af.parseMagicLine(sAbility, abilityId, NULL, NULL);
    if(ability)
        psMenuText = ability->getMenuText();
    else
        psMenuText = sAbility.c_str();
    delete (ability);

}

void APhaseAction::Update(float dt)
{
    if(checkexile)
    {
        MTGCardInstance* tocheck = (((MTGCardInstance *)target)->next)?((MTGCardInstance *)target)->next:((MTGCardInstance *)target);
        if((tocheck->getCurrentZone() != ((MTGCardInstance *)target)->owner->game->exile) && (tocheck->getCurrentZone() != ((MTGCardInstance *)target)->owner->opponent()->game->exile))
        {
            this->forceDestroy = 1;
            return;
        }
    }
    if (newPhase != currentPhase)
    {
        if((myturn && game->currentPlayer == source->controller())|| 
            (opponentturn && game->currentPlayer != source->controller())/*||*/
            /*(myturn && opponentturn)*/)
        {
            if(newPhase == phase && next )
            {
                MTGCardInstance * _target = NULL;
                bool isTargetable = false;
                
                if(target)
                {
                    _target = static_cast<MTGCardInstance *>(target);
                    isTargetable = (_target && !_target->currentZone && _target != this->source);
                }
                
                if(!sAbility.size() || (!target || isTargetable))
                {
                    this->forceDestroy = 1;
                    return;
                }
                else
                {
                    while(_target && _target->next)
                        _target = _target->next;
                }
                
                AbilityFactory af(game);
                MTGAbility * ability = af.parseMagicLine(sAbility, abilityId, NULL, _target);

                MTGAbility * a = ability->clone();
                a->target = _target;
                a->resolve();
                delete (a);
                delete (ability);
                if(this->oneShot || once)
                {
                    this->forceDestroy = 1;
                }
            }
            else if(newPhase == phase && next == false)
                next = true;
        }
    }
    MTGAbility::Update(dt);
}

int APhaseAction::resolve()
{

    return 0;
}

const string APhaseAction::getMenuText()
{
    if(psMenuText.size())
    return psMenuText.c_str();
    else
    return "Phase Based Action";
}

APhaseAction * APhaseAction::clone() const
{
    APhaseAction * a = NEW APhaseAction(*this);
    if(forcedestroy == false)
        a->forceDestroy = -1;// we want this ability to stay alive until it resolves.
    return a;
}

APhaseAction::~APhaseAction()
{

}

// the main ability
APhaseActionGeneric::APhaseActionGeneric(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * target, string sAbility, int restrictions, int _phase,bool forcedestroy,bool next,bool myturn,bool opponentturn,bool once, bool checkexile) :
    InstantAbility(observer, _id, card, target)
{
    MTGCardInstance * _target = target;
    ability = NEW APhaseAction(game, _id, card,_target, sAbility, restrictions, _phase,forcedestroy,next,myturn,opponentturn,once,checkexile);
}

int APhaseActionGeneric::resolve()
{
        APhaseAction * a = ability->clone();
        a->target = target;
        a->addToGame();
        return 1;
}

const string APhaseActionGeneric::getMenuText()
{
    return ability->getMenuText();
}

APhaseActionGeneric * APhaseActionGeneric::clone() const
{
    APhaseActionGeneric * a = NEW APhaseActionGeneric(*this);
    a->ability = this->ability->clone();
    a->oneShot = 1;
    return a;
}

APhaseActionGeneric::~APhaseActionGeneric()
{
    SAFE_DELETE(ability);
}

//AAttackSetCost
AAttackSetCost::AAttackSetCost(GameObserver* observer, int _id, MTGCardInstance * _source, string number, bool pw) :
    MTGAbility(observer, _id, _source), number(number), pw(pw)
{
}

void AAttackSetCost::Update(float dt)
{
    if(game->getCurrentGamePhase() != MTG_PHASE_COMBATATTACKERS)
    {
        source->attackCost = source->attackCostBackup;
        if(pw)
            source->attackPlaneswalkerCost = source->attackPlaneswalkerCostBackup;
        MTGAbility::Update(dt);
    }
}

int AAttackSetCost::addToGame()
{
    WParsedInt attackcost(number, NULL, source);
    source->attackCost += attackcost.getValue();
    source->attackCostBackup += attackcost.getValue();
    if(pw)
    {
        source->attackPlaneswalkerCost += attackcost.getValue();
        source->attackPlaneswalkerCostBackup += attackcost.getValue();
    }

    return MTGAbility::addToGame();
}

int AAttackSetCost::destroy()
{
    
    WParsedInt attackcost(number, NULL, source);
    source->attackCost -= attackcost.getValue();
    source->attackCostBackup -= attackcost.getValue();
    if(pw)
    {
        source->attackPlaneswalkerCost -= attackcost.getValue();
        source->attackPlaneswalkerCostBackup -= attackcost.getValue();
    }

    return 1;
}

const string AAttackSetCost::getMenuText()
{
    if(number.size())
    {
        WParsedInt parsedNum(number, NULL, source);
        return _("Pay " + parsedNum.getStringValue() + " to attack").c_str();
    }
    return "Attack Cost";
}

AAttackSetCost * AAttackSetCost::clone() const
{
    return NEW AAttackSetCost(*this);
}

//ABlockSetCost
ABlockSetCost::ABlockSetCost(GameObserver* observer, int _id, MTGCardInstance * _source, string number) :
    MTGAbility(observer, _id, _source), number(number)
{
}

void ABlockSetCost::Update(float dt)
{
    if(game->getCurrentGamePhase() != MTG_PHASE_COMBATBLOCKERS)
    {
        source->blockCost = source->blockCostBackup;
        MTGAbility::Update(dt);
    }
}

int ABlockSetCost::addToGame()
{
    WParsedInt blockCost(number, NULL, source);
    source->blockCost += blockCost.getValue();
    source->blockCostBackup += blockCost.getValue();

    return MTGAbility::addToGame();
}

int ABlockSetCost::destroy()
{
    
    WParsedInt blockCost(number, NULL, source);
    source->blockCost -= blockCost.getValue();
    source->blockCostBackup -= blockCost.getValue();

    return 1;
}

const string ABlockSetCost::getMenuText()
{
    if(number.size())
    {
        WParsedInt parsedNum(number, NULL, source);
        return _("Pay " + parsedNum.getStringValue() + " to block").c_str();
    }
    return "Block Cost";
}

ABlockSetCost * ABlockSetCost::clone() const
{
    return NEW ABlockSetCost(*this);
}

//ASeize
ASeize::ASeize(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target) :
MTGAbility(observer, _id, card)
{
    target = _target;
    Seized = NULL;
    previousController = NULL;
    resolved = false;
}

void ASeize::Update(float dt)
{
    if (resolved == false)
    {
        resolved = true;
        resolveSeize();
    }

    if (!source->isInPlay(game))
    {
            if (Seized == NULL || !Seized->isInPlay(game))
                MTGAbility::Update(dt);
            MTGCardInstance * _target = Seized;
            returntoOwner(_target);
    }
    MTGAbility::Update(dt);
}

void ASeize::resolveSeize()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        previousController = _target->controller();
        previousController->game->putInZone(_target, _target->currentZone,
            source->controller()->game->inPlay);
        Seized = _target;
        source->seized = Seized;
        Seized->seized = source;
    }
}

void ASeize::returntoOwner(MTGCardInstance* _target) {
    MTGCardInstance * cardToReturn = _target;
    if(!cardToReturn)
    {
        if (source)
            source->seized = NULL;
        this->forceDestroy = 1;
        return;
    }
    if(previousController && cardToReturn->isInPlay(game))
    {
        cardToReturn->seized = NULL;
        cardToReturn->controller()->game->putInZone(_target, _target->currentZone,
            previousController->game->inPlay);
    }
    if (source)
        source->seized = NULL;
    this->forceDestroy = 1;
    Seized = NULL;
    return;
}

int ASeize::resolve()
{
    return 0;
}

int ASeize::receiveEvent(WEvent * event)
{
    WEventCardControllerChange * enters = dynamic_cast<WEventCardControllerChange *> (event);
    if (enters && source)
    {
        if(enters->card == source)
        {
            if(Seized && Seized->controller() != enters->card->controller())
                returntoOwner(Seized);
            return 1;
        }
    }
    return 0;
}

const string ASeize::getMenuText()
{
    return "Gain Control";
}

ASeize * ASeize::clone() const
{
    ASeize * a = NEW ASeize(*this);
    a->forceDestroy = -1;
    return a;
};
ASeize::~ASeize()
{
}

ASeizeWrapper::ASeizeWrapper(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target) :
    InstantAbility(observer, _id, source, _target)
{
    ability = NEW ASeize(observer, _id,card,_target);
    andAbility = NULL;
}

int ASeizeWrapper::resolve()
{
    ASeize * a = ability->clone();
    a->target = target;
    a->addToGame();
    if(andAbility)
    {
        MTGAbility * andAbilityClone = andAbility->clone();
        andAbilityClone->target = target;
        if(andAbility->oneShot)
        {
            andAbilityClone->resolve();
            SAFE_DELETE(andAbilityClone);
        }
        else
        {
            andAbilityClone->addToGame();
        }
    }
    return 1;
}

const string ASeizeWrapper::getMenuText()
{
    return "Gain Control";
}

ASeizeWrapper * ASeizeWrapper::clone() const
{
    ASeizeWrapper * a = NEW ASeizeWrapper(*this);
    a->ability = this->ability->clone();
    a->oneShot = 1;
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

ASeizeWrapper::~ASeizeWrapper()
{
    SAFE_DELETE(ability);
    SAFE_DELETE(andAbility);
}

//AShackle
AShackle::AShackle(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target) :
MTGAbility(observer, _id, card)
{
    target = _target;
    Shackled = NULL;
    previousController = NULL;
    resolved = false;
}

void AShackle::Update(float dt)
{
    if (resolved == false)
    {
        resolved = true;
        resolveShackle();
    }

    if (!source->isTapped() || !source->isInPlay(game))
    {
            if (Shackled == NULL || !Shackled->isInPlay(game))
                MTGAbility::Update(dt);
            MTGCardInstance * _target = Shackled;
            returntoOwner(_target);
    }
    MTGAbility::Update(dt);
}

void AShackle::resolveShackle()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        previousController = _target->controller();
        previousController->game->putInZone(_target, _target->currentZone,
            source->controller()->game->inPlay);
        Shackled = _target;
        source->shackled = Shackled;
        Shackled->shackled = source;
    }
}

void AShackle::returntoOwner(MTGCardInstance* _target) {
    MTGCardInstance * cardToReturn = _target;
    if(!cardToReturn)
    {
        if (source)
            source->shackled = NULL;
        this->forceDestroy = 1;
        return;
    }
    if(previousController && cardToReturn->isInPlay(game))
    {
        cardToReturn->shackled = NULL;
        cardToReturn->controller()->game->putInZone(_target, _target->currentZone,
            previousController->game->inPlay);
    }
    if (source)
        source->shackled = NULL;
    this->forceDestroy = 1;
    Shackled = NULL;
    return;
}

int AShackle::resolve()
{
    return 0;
}
const string AShackle::getMenuText()
{
    return "Gain Control";
}

AShackle * AShackle::clone() const
{
    AShackle * a = NEW AShackle(*this);
    a->forceDestroy = -1;
    return a;
};
AShackle::~AShackle()
{
}

AShackleWrapper::AShackleWrapper(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target) :
    InstantAbility(observer, _id, source, _target)
{
    ability = NEW AShackle(observer, _id,card,_target);
}

int AShackleWrapper::resolve()
{
    AShackle * a = ability->clone();
    a->target = target;
    a->addToGame();
    return 1;
}

const string AShackleWrapper::getMenuText()
{
    return "Gain Control";
}

AShackleWrapper * AShackleWrapper::clone() const
{
    AShackleWrapper * a = NEW AShackleWrapper(*this);
    a->ability = this->ability->clone();
    a->oneShot = 1;
    return a;
}

AShackleWrapper::~AShackleWrapper()
{
    SAFE_DELETE(ability);
}

//grant
AGrant::AGrant(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target, MTGAbility * _Grant) :
    MTGAbility(observer, _id, card)
{
    Granted = _Grant;
    target = _target;
    Blessed = NULL;
    resolved = false;
    toGrant = NULL;
}

void AGrant::Update(float dt)
{
    if (resolved == false)
    {
        resolved = true;
        resolveGrant();
    }

    if (!source->isTapped() || !source->isInPlay(game))
    {
        if (Blessed == NULL || !Blessed->isInPlay(game))
            MTGAbility::Update(dt);
        MTGCardInstance * _target = Blessed;
        removeGranted(_target);
    }
    else
        resolveGrant();
    MTGAbility::Update(dt);
}

void AGrant::resolveGrant()
{
    if (toGrant) return;
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if (_target)
    {
        toGrant = Granted->clone();
        toGrant->target = _target;
        toGrant->addToGame();
        Blessed = _target;
    }
}

void AGrant::removeGranted(MTGCardInstance* _target)
{
    if (!toGrant) return;
    game->removeObserver(toGrant);
    game->removeObserver(this);
    Blessed = NULL;
    return;
}

int AGrant::resolve()
{
    return 0;
}
const string AGrant::getMenuText()
{
    return Granted->getMenuText();
}

AGrant * AGrant::clone() const
{
    AGrant * a = NEW AGrant(*this);
    a->forceDestroy = -1;
    a->Granted = Granted->clone();
    return a;
};
AGrant::~AGrant()
{
    SAFE_DELETE(Granted);
}

AGrantWrapper::AGrantWrapper(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target, MTGAbility * _Grant) :
    InstantAbility(observer, _id, source, _target), Granted(_Grant)
{
    ability = NEW AGrant(observer, _id, card, _target,_Grant);
}

int AGrantWrapper::resolve()
{
    AGrant * a = ability->clone();
    a->target = target;
    a->addToGame();
    return 1;
}

const string AGrantWrapper::getMenuText()
{
    return "Grant";
}

AGrantWrapper * AGrantWrapper::clone() const
{
    AGrantWrapper * a = NEW AGrantWrapper(*this);
    a->ability = this->ability->clone();
    a->oneShot = 1;
    return a;
}

AGrantWrapper::~AGrantWrapper()
{
    SAFE_DELETE(ability);
}

//a blink
ABlink::ABlink(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target, bool blinkueot, bool blinkForSource, bool blinkhand, MTGAbility * stored) :
MTGAbility(observer, _id, card),blinkueot(blinkueot),blinkForSource(blinkForSource),blinkhand(blinkhand),stored(stored)
{
    target = _target;
    Blinked = NULL;
    resolved = false;
}

void ABlink::Update(float dt)
{
    if (resolved == false)
    {
        resolved = true;
        resolveBlink();
    }

    if ((blinkueot && currentPhase == MTG_PHASE_ENDOFTURN) || (blinkForSource && !source->isInPlay(game)))
    {
        if(Blinked->blinked)
        {
            if (Blinked == NULL)
                MTGAbility::Update(dt);
            MTGCardInstance * _target = Blinked;
            returnCardIntoPlay(_target);
        }
    }
    MTGAbility::Update(dt);
}

void ABlink::resolveBlink()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        //going to comment this condiational out, i can't remember if i added this to fix some kind of bug 
        //which is why i plan on leaving it here. seems to work fine though without it.
        //if(blinkhand && !_target->controller()->game->isInZone(_target,_target->controller()->game->hand))
        //{
        //    this->forceDestroy = 1;
        //    return;
        //}
        //else if(!blinkhand && !_target->controller()->game->isInZone(_target,_target->controller()->game->battlefield))
        //{
        //    this->forceDestroy = 1;
        //    return;
        //}
        if (_target->MeldedFrom.size())
        {
            //cards with meld are handled very different from normal cards with this specific ability giving us about 3 of the
            //core rules for the ability. below we split the card up, and we send them to garbage, move the original to temp where
            //it is later moved to garbage by garbage collection.
            //then we build 2 seperate blinks with the 2 parts as the targets.
            vector<string> names = split(_target->MeldedFrom, '|');
            MTGCard * cardone = MTGCollection()->getCardByName(names[0], _target->setId);
            MTGCardInstance * cardOne = NEW MTGCardInstance(cardone, _target->owner->game);
            MTGCard * cardtwo = MTGCollection()->getCardByName(names[1], _target->setId);
            MTGCardInstance * cardTwo = NEW MTGCardInstance(cardtwo, _target->owner->game);
            _target->controller()->game->putInZone(_target, _target->currentZone,
                _target->owner->game->temp);
            _target->controller()->game->garbage->addCard(cardOne);
            _target->controller()->game->garbage->addCard(cardTwo);
            MTGAbility * a = NEW ABlinkGeneric(game, game->mLayers->actionLayer()->getMaxId(), source, cardOne, blinkueot, blinkForSource, blinkhand, stored);
            a->target = (Targetable*)cardOne;
            a->oneShot = false;
            a->canBeInterrupted = false;
            a->resolve();
            SAFE_DELETE(a);


            MTGAbility * a2 = NEW ABlinkGeneric(game, game->mLayers->actionLayer()->getMaxId(), source, cardTwo, blinkueot, blinkForSource, blinkhand, stored);
            a2->target = (Targetable*)cardTwo;
            a2->oneShot = false;
            a2->canBeInterrupted = false;
            a2->resolve();
            SAFE_DELETE(a2);
            this->forceDestroy = 1;
            this->removeFromGame();
            return;
        }
        else
        _target->controller()->game->putInZone(_target, _target->currentZone,
            _target->owner->game->exile);
        if (_target->MeldedFrom.size() || !_target)
        {
            return;
        }
        if(_target->isToken)
        {
            //if our target is a token, we're done as soon as its sent to exile.
            this->forceDestroy = 1;
            return;
        }
        if (_target && _target->next)
            _target = _target->next;
        _target->blinked = true;
        Blinked = _target;
        if(source->isPermanent()&&!source->isInPlay(game))
        {
            Blinked->blinked = false;
        }
        if (!blinkueot && !blinkForSource)
        {
            returnCardIntoPlay(_target);
        }

    }
}

void ABlink::returnCardIntoPlay(MTGCardInstance* _target) {
    MTGCardInstance * Blinker = NULL;
    if(!_target->blinked || _target->hasSubtype(Subtypes::TYPE_INSTANT) || _target->hasSubtype(Subtypes::TYPE_SORCERY))
    {
        this->forceDestroy = 1;
        return;
    }
    if (!blinkhand)
        Blinker = _target->controller()->game->putInZone(
            _target,
            _target->currentZone,
            _target->owner->game->battlefield);
    if (blinkhand)
    {
        _target->controller()->game->putInZone(
            _target,
            _target->currentZone,
            _target->owner->game->hand);
        return;
    }
    Spell * spell = NEW Spell(game, Blinker);
    spell->source->counters->init();
    if (spell->source->hasSubtype(Subtypes::TYPE_AURA) && !blinkhand)
    {
        TargetChooserFactory tcf(game);
        TargetChooser * tc = tcf.createTargetChooser(
            spell->source->spellTargetType,
            spell->source);
        if (!tc->validTargetsExist())
        {
            spell->source->owner->game->putInExile(spell->source);
            SAFE_DELETE(spell);
            SAFE_DELETE(tc);
            this->forceDestroy = 1;
            return;
        }

        /*MTGGameZone * inplay = spell->source->owner->game->inPlay;
        spell->source->target = NULL;
        for (int i = game->getRandomGenerator()->random()%inplay->nb_cards;;i = game->getRandomGenerator()->random()%inplay->nb_cards)
        {
            if(tc->canTarget(inplay->cards[i]) && spell->source->target == NULL)
            {
                spell->source->target = inplay->cards[i];
                spell->getNextCardTarget();
                spell->resolve();
                SAFE_DELETE(spell);
                SAFE_DELETE(tc);
                this->forceDestroy = 1;
                return;
            }
        }*/
        //replaced with castcard(putinplay)
        MTGAbility *a = NEW AACastCard(game, game->mLayers->actionLayer()->getMaxId(), Blinker, Blinker, false, false, false, "", "Return to Play", false, true);
        a->oneShot = false;
        a->canBeInterrupted = false;
        a->addToGame();
        SAFE_DELETE(spell);
        SAFE_DELETE(tc);
        this->forceDestroy = 1;
        return;
    }
    spell->source->power = spell->source->origpower;
    spell->source->toughness = spell->source->origtoughness;
    spell->source->X = 0;
    if (!spell->source->hasSubtype(Subtypes::TYPE_AURA)) {
        MTGGameZone* prev = spell->source->previousZone; // Save the previous zone of card before spell resolution.
        spell->resolve();
        if(prev && spell->source->currentZone == spell->source->previousZone)
            spell->source->previousZone = prev; // Fixed issue on previous zone (e.g. "Otherworldly Journey").
        if (stored)
        {
            MTGAbility * clonedStored = stored->clone();
            clonedStored->target = spell->source;
            if (clonedStored->oneShot)
            {
                clonedStored->resolve();
                delete (clonedStored);
            }
            else
            {
                clonedStored->addToGame();
            }
        }
    }
    SAFE_DELETE(spell);
    SAFE_DELETE(tc);
    SAFE_DELETE(stored);
    this->forceDestroy = 1;
    Blinked = NULL;
}

int ABlink::resolve()
{
    return 0;
}
const string ABlink::getMenuText()
{
    return "Blink";
}

ABlink * ABlink::clone() const
{
    ABlink * a = NEW ABlink(*this);
    a->stored = stored ? stored->clone() : NULL;
    a->forceDestroy = -1;
    return a;
};
ABlink::~ABlink()
{
    SAFE_DELETE(stored);
}

ABlinkGeneric::ABlinkGeneric(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target,bool blinkueot,bool blinkForSource,bool blinkhand,MTGAbility * stored) :
    InstantAbility(observer, _id, source, _target)
{
    ability = NEW ABlink(observer, _id,card,_target,blinkueot,blinkForSource,blinkhand,stored);
}

int ABlinkGeneric::resolve()
{
    ABlink * a = ability->clone();
    a->target = target;
    a->addToGame();
    return 1;
}

const string ABlinkGeneric::getMenuText()
{
    return "Blink";
}

ABlinkGeneric * ABlinkGeneric::clone() const
{
    ABlinkGeneric * a = NEW ABlinkGeneric(*this);
    a->ability = this->ability->clone();
    a->oneShot = 1;
    return a;
}

ABlinkGeneric::~ABlinkGeneric()
{
    SAFE_DELETE(ability);
}

// target becomes blocked by source
AABlock::AABlock(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost *) :
InstantAbility(observer, id, card, target)
{
    target = _target;
}

int AABlock::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be blocked, they will follow the fate of top-card
    source = (MTGCardInstance*)source;
    if (_target && source->canBlock(_target))
    {
       source->toggleDefenser(_target);
       source->getObserver()->isInterrupting = NULL;
    }
    return 1;
}

AABlock * AABlock::clone() const
{
    return NEW AABlock(*this);
}

// target becomes pair of source
PairCard::PairCard(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost *) :
InstantAbility(observer, id, card, target)
{
    target = _target;
    oneShot = true;
    forceDestroy = 1;
}

int PairCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    source = (MTGCardInstance*)source;
    if (_target && !_target->myPair && source)
    {
        source->myPair = _target;
        _target->myPair = source;
    }
    return 1;
}

PairCard * PairCard::clone() const
{
    return NEW PairCard(*this);
}
//target is dredged
dredgeCard::dredgeCard(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost *) :
InstantAbility(observer, id, card, target)
{
    target = _target;
    oneShot = true;
    forceDestroy = 1;
}

int dredgeCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be dredged, they will follow the fate of top-card
    if(_target)
    {
        for(int j = 0; j < _target->data->dredge();j++)
        {
            _target->controller()->game->putInZone(
                _target->controller()->game->library->cards[_target->controller()->game->library->nb_cards - 1],
                _target->controller()->game->library, _target->controller()->game->graveyard);
        }
        _target->controller()->game->putInZone(_target,_target->currentZone,_target->controller()->game->hand);
    }
    return 1;
}

dredgeCard * dredgeCard::clone() const
{
    return NEW dredgeCard(*this);
}

// target becomes a parent of card(source)
AAConnect::AAConnect(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost *) :
InstantAbility(observer, id, card, target)
{
    target = _target;
}

int AAConnect::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(_target->mutation && _target->parentCards.size() > 0) return 0; // Mutated down cards cannot be connected, they will follow the fate of top-card
    if (_target)
    {
        while (_target->next)
            _target = _target->next;
        _target->childrenCards.push_back(source);
        source->parentCards.push_back(_target);
        //weapon
        if(source->hasSubtype(Subtypes::TYPE_EQUIPMENT))
        {
            for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
            {
                MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
                AEquip * eq = dynamic_cast<AEquip*> (a);
                if (eq && eq->source == source)
                {
                    ((AEquip*)a)->unequip();
                    ((AEquip*)a)->equip(_target);
                }
            }
        }
        else
        {
            if(source->target)
                source->target = NULL;
            //clearing the source target allows us to use target= line
            //without creating side effects on any other abilities a card has
            //connect has to be the first ability in the cards lines unless you want it to do effects to the targeted card!!!
        }
    }
    return 1;
}

AAConnect * AAConnect::clone() const
{
    return NEW AAConnect(*this);
}

AEquip::AEquip(GameObserver* observer, int _id, MTGCardInstance * _source, ManaCost * _cost, int restrictions) :
    TargetAbility(observer, _id, _source, NULL, _cost, restrictions)
{
    aType = MTGAbility::STANDARD_EQUIP;
    isAttach = restrictions != ActivatedAbility::AS_SORCERY;
    isReconfiguration = false;
}

int AEquip::unequip()
{
    if (source->target)
    {
        source->target->equipment -= 1;
        source->parentCards.clear();
        for (unsigned int w = 0; w < source->target->childrenCards.size(); w++)
        {
            MTGCardInstance * child = source->target->childrenCards[w];
            if (child == source)
                source->target->childrenCards.erase(source->target->childrenCards.begin() + w);
        }
    }
    source->target = NULL;
    for (size_t i = 0; i < currentAbilities.size(); ++i)
    {
        MTGAbility * a = currentAbilities[i];
        if (dynamic_cast<AEquip *> (a) || dynamic_cast<ATeach *> (a) || dynamic_cast<AAConnect *> (a)
            || dynamic_cast<AANewTarget *> (AbilityFactory::getCoreAbility(a))
            || (a->aType == MTGAbility::STANDARD_TOKENCREATOR && a->oneShot))
        {
            SAFE_DELETE(a);
            continue;
        }
        game->removeObserver(currentAbilities[i]);
    }
    if(isReconfiguration && !source->hasType(Subtypes::TYPE_CREATURE))
        source->addType(Subtypes::TYPE_CREATURE);
    currentAbilities.clear();
    WEvent * e = NEW WEventCardUnattached(source);
    game->receiveEvent(e);
    return 1;
}

int AEquip::equip(MTGCardInstance * equipped)
{
    if(isReconfiguration && source->hasType(Subtypes::TYPE_CREATURE))
        source->removeType(Subtypes::TYPE_CREATURE);
    source->target = equipped;
    source->target->equipment += 1;
    source->parentCards.push_back(equipped);
    source->target->childrenCards.push_back((MTGCardInstance*)source);
    AbilityFactory af(game);
    af.getAbilities(&currentAbilities, NULL, source);
    for (size_t i = 0; i < currentAbilities.size(); ++i)
    {
        MTGAbility * a = currentAbilities[i];
        if (dynamic_cast<AEquip *> (a)) continue;
        if (dynamic_cast<ATeach *> (a)) continue;
        if (dynamic_cast<AAConnect *> (a)) continue;
        if (dynamic_cast<AANewTarget *> (af.getCoreAbility(a))) continue;
        if (a->aType == MTGAbility::STANDARD_TOKENCREATOR && a->oneShot)
        {
            a->forceDestroy = 1;
            continue;
        }
        if (dynamic_cast<AACopier *> (af.getCoreAbility(a)))
        {
            a->forceDestroy = 1;
            continue;
        }
        //we generally dont want to pass oneShot tokencreators to the cards
        //we equip...
        a->addToGame();
    }
    WEvent * e = NEW WEventCardEquipped(source);
    game->receiveEvent(e);
    return 1;
}

int AEquip::mutate(MTGCardInstance * mutated)
{
    source->target = mutated;
    source->target->mutation += 1;
    source->mutation += 1;
    source->parentCards.push_back(mutated);
    source->target->childrenCards.push_back((MTGCardInstance*)source);
    AbilityFactory af(game);
    af.getAbilities(&currentAbilities, NULL, source);
    for (size_t i = 0; i < currentAbilities.size(); ++i)
    {
        MTGAbility * a = currentAbilities[i];
        if (dynamic_cast<AEquip *> (a)) continue;
        if (dynamic_cast<ATeach *> (a)) continue;
        if (dynamic_cast<AAConnect *> (a)) continue;
        if (dynamic_cast<AANewTarget *> (af.getCoreAbility(a))) continue;
        if (a->aType == MTGAbility::STANDARD_TOKENCREATOR && a->oneShot)
        {
            a->forceDestroy = 1;
            continue;
        }
        if (dynamic_cast<AACopier *> (af.getCoreAbility(a)))
        {
            a->forceDestroy = 1;
            continue;
        }
        //we generally dont want to pass oneShot tokencreators to the cards
        //we mutate...
        a->addToGame();
    }
    WEvent * e = NEW WEventCardMutated(mutated);
    source->getObserver()->receiveEvent(e); // triggers the @mutated event for any other listener.
    return 1;
}

int AEquip::resolve()
{
    MTGCardInstance * mTarget = tc->getNextCardTarget();
    if (!mTarget) return 0;
    if (mTarget == source) return 0;
    if (source->mutation) return 0; // No need to unequip mutation cards.
    unequip();
    equip(mTarget);
    return 1;
}

const string AEquip::getMenuText()
{
    if (isAttach)
        return "Attach";
    else if (isReconfiguration)
        return "Reconfigure Attach";
    else
        return "Equip";
}

int AEquip::testDestroy()
{
    if(source->mutation) // No need to unequip mutation cards.
        return 0;

    if (source->target && !game->isInPlay(source->target))
        //unequip();//testfix for equipment when the card it equip moves to other battlefield
    if (!game->connectRule)
    {
        if (source->target && TargetAbility::tc && !TargetAbility::tc->canTarget((Targetable *)source->target,true))
            unequip();
    }
    return TargetAbility::testDestroy();
}

int AEquip::destroy()
{
    if(source->mutation) // No need to unequip mutation cards.
        return 0;
    unequip();
    return TargetAbility::destroy();
}

AEquip * AEquip::clone() const
{
    return NEW AEquip(*this);
}

// casting a card for free, or casting a copy of a card.
AACastCard::AACastCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, bool _restricted, bool _copied, bool asNormal, string _namedCard, string _name, bool _noEvent, bool putinplay, bool madness, bool alternative, int kicked, int costx, bool flipped, bool flashback) :
   MTGAbility(observer, _id, _source), restricted(_restricted), asCopy(_copied), normal(asNormal), cardNamed(_namedCard), nameThis(_name), noEvent(_noEvent), putinplay(putinplay), asNormalMadness(madness), alternative(alternative), kicked(kicked), costx(costx), flipped(flipped), flashback(flashback)
{
    target = _target;
    andAbility = NULL;
    processed = false;
    theNamedCard = NULL;
}


void AACastCard::Update(float dt)
{
    MTGAbility::Update(dt);
    if (processed)
        return;
    if(cardNamed.size() && !theNamedCard)
    {
        if(cardNamed.find("randomcard") != string::npos){ //cast a random card from collection.
            MTGCard *rndCard = NULL;
            while(!rndCard || rndCard->data->isLand())
                rndCard = MTGCollection()->getCardById(MTGCollection()->ids.at(std::rand() % (MTGCollection()->ids).size()));
            cardNamed = rndCard->data->name;
        }
        if (cardNamed.find("imprintedcard") != string::npos)
        {
            if (source && source->currentimprintName.size())
            {
                cardNamed = source->currentimprintName;
            }
        }
        theNamedCard = makeCard();
        //if somehow the imprinted card leaves its zone destroy this...
        if(cardNamed.find("imprintedcard") != string::npos && !theNamedCard)
        {
            this->forceDestroy = 1;
            return;
        }
    }
    if(putinplay)
    {
        MTGCardInstance * toCheck = (MTGCardInstance*)target;
        toCheck->target = NULL;
        toCheck->playerTarget = NULL;
        toCheck->bypassTC = true;
        TargetChooserFactory tcf(game);
        TargetChooser * atc = tcf.createTargetChooser(toCheck->spellTargetType,toCheck);
        if ((toCheck->hasType(Subtypes::TYPE_AURA) && !atc->validTargetsExist())||toCheck->isToken)
        {
            processed = true;
            this->forceDestroy = 1;
            return;
        }
        SAFE_DELETE(atc);
    }
    if (restricted)
    {
        MTGCardInstance * toCheck = (MTGCardInstance*)target;
        if(theNamedCard)
            toCheck = theNamedCard;
        if (game->currentActionPlayer->game->playRestrictions->canPutIntoZone(toCheck, source->controller()->game->stack) == PlayRestriction::CANT_PLAY)
        {
            processed = true;
            if(andAbility) // Allow to use and!()! even when restriction occurred (e.g. "Gix, Yawgmoth Praetor").
            {
                MTGAbility * andAbilityClone = andAbility->clone();
                andAbilityClone->target = toCheck;
                if(andAbility->oneShot)
                {
                    andAbilityClone->resolve();
                    SAFE_DELETE(andAbilityClone);
                }
                else
                {
                    andAbilityClone->addToGame();
                }
            }
            this->forceDestroy = 1;
            return ;
        }
        if(!allowedToCast(toCheck,source->controller()))
        {
            processed = true;
            if(andAbility) // Allow to use and!()! even when restriction occurred (e.g. "Gix, Yawgmoth Praetor").
            {
                MTGAbility * andAbilityClone = andAbility->clone();
                andAbilityClone->target = toCheck;
                if(andAbility->oneShot)
                {
                    andAbilityClone->resolve();
                    SAFE_DELETE(andAbilityClone);
                }
                else
                {
                    andAbilityClone->addToGame();
                }
            }
            this->forceDestroy = 1;
            return;
        }
        if(toCheck->isLand())
        {
            TargetChooser* tc = NEW TypeTargetChooser(game, "land");
            if(game->currentActionPlayer->game->battlefield->seenThisTurn(tc, Constants::CAST_DONT_CARE, false) >= game->currentActionPlayer->game->playRestrictions->getMaxPerTurnRestrictionByTargetChooser(tc)->maxPerTurn){
                processed = true;
                if(andAbility) // Allow to use and!()! even when restriction occurred (e.g. "Gix, Yawgmoth Praetor").
                {
                    MTGAbility * andAbilityClone = andAbility->clone();
                    andAbilityClone->target = toCheck;
                    if(andAbility->oneShot)
                    {
                        andAbilityClone->resolve();
                        SAFE_DELETE(andAbilityClone);
                    }
                    else
                    {
                        andAbilityClone->addToGame();
                    }
                }
                this->forceDestroy = 1;
                return;
            }
        } 
    }
    MTGCardInstance * toCheck = (MTGCardInstance*)target;
    if(theNamedCard)
        toCheck = theNamedCard;
    if(toCheck && toCheck->spellTargetType.size())
    {
        string backupST = toCheck->spellTargetType;
        if((toCheck->spellTargetType == "opponent") && (toCheck->owner != source->controller()))
            toCheck->spellTargetType = "controller";
        else if((toCheck->spellTargetType.find("|opponent") != string::npos) && (toCheck->owner != source->controller()))
        {
            string replaceMe = backupST;
            toCheck->spellTargetType = cReplaceString(replaceMe, "|opponent", "|my");
        }
        //Since we control the card to cast, if the card should target an opponent,
        //direct it to source ability controller->opponent
        //example card is Bribery, if we cast it targeting from opponent's library,
        //we should target the source ability controller->opponent

        TargetChooserFactory tcf(game);
        TargetChooser * stc = tcf.createTargetChooser(toCheck->spellTargetType,toCheck);
        if (!stc->validTargetsExist()||toCheck->isToken)
        {
            toCheck->spellTargetType = backupST;

            processed = true;
            this->forceDestroy = 1;
            return;
        }
        SAFE_DELETE(stc);
    }
    if (Spell * checkSpell = dynamic_cast<Spell*>(target))
    {
        toCheck = checkSpell->source;
    }
    if (!game->targetListIsSet(toCheck))
    {
        if(game->targetChooser)
            game->targetChooser->Owner = source->controller();//sources controller is the caster
        return;
    }
    resolveSpell();
    this->forceDestroy = 1;
    return;
}
int AACastCard::isReactingToTargetClick(Targetable * card){return 0;}
int AACastCard::reactToTargetClick(Targetable * object)
{
    if (MTGCardInstance * cObject = dynamic_cast<MTGCardInstance *>(object))
        return reactToClick(cObject);

    if (waitingForAnswer)
    {
        if (tc->toggleTarget(object) == TARGET_OK_FULL)
        {
            waitingForAnswer = 0;
            game->mLayers->actionLayer()->setCurrentWaitingAction(NULL);
            return MTGAbility::reactToClick(source);
        }
        return 1;
    }
    return 0;
}

MTGCardInstance * AACastCard::makeCard()
{
    MTGCardInstance * card = NULL;
    MTGCard * cardData = MTGCollection()->getCardByName(cardNamed, source->setId);
    if(!cardData) return NULL;
    card = NEW MTGCardInstance(cardData, source->controller()->game);
    card->owner = source->controller();
    card->lastController = source->controller();
    source->controller()->game->sideboard->addCard(card);
    return card;
}

int AACastCard::resolveSpell()
{
    if (processed)
        return 0;
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(theNamedCard)
        _target = theNamedCard;
    if (Spell * checkSpell = dynamic_cast<Spell*>(target))
    {
        _target = checkSpell->source;
    }
    if(asCopy)
    {
        MTGCard * cardToCopy = MTGCollection()->getCardById(_target->getId());
        MTGCardInstance * myDummy = NULL;
        myDummy = NEW MTGCardInstance(cardToCopy, source->controller()->game);
        myDummy->setObserver(source->controller()->getObserver());
        source->controller()->game->garbage->addCard(myDummy);
        _target = myDummy;
        _target->isToken = 1;
        _target->changeController(source->controller(),true);
    }
    if (_target)
    {
        if (_target->isLand())
        {
            if(theNamedCard)
            {
                _target = source->controller()->game->putInZone(theNamedCard, theNamedCard->currentZone, source->controller()->game->stack, noEvent); // Fixed a bug when using noevent option with namedcard option.
                if(asCopy) 
                    _target->isToken = 1; // Fixed a bug when using copied option with namedcard option.
                Spell * spell = game->mLayers->stackLayer()->addSpell(_target, NULL, NULL, 1, 0);
                spell->resolve();
            }
            else
            {
                MTGAbility * a = NEW AAMover(game, -1, source, _target, "mybattlefield", "");
                a->oneShot = true;
                a->resolve();
                SAFE_DELETE(a);
            }
            if(andAbility) // Allow to use and!()! with lands.
            {
                MTGAbility * andAbilityClone = andAbility->clone();
                andAbilityClone->target = _target;
                if(andAbility->oneShot)
                {
                    andAbilityClone->resolve();
                    SAFE_DELETE(andAbilityClone);
                }
                else
                {
                    andAbilityClone->addToGame();
                }
            }
            this->forceDestroy = true;
            processed = true;
            return 1;
        }
        if(theNamedCard)
        {
            Spell * spell = NULL;
            MTGCardInstance * copy = NULL;
            if ((normal || asNormalMadness) || !theNamedCard->isSorceryorInstant())
            {
                if (putinplay && theNamedCard->isPermanent())
                    copy = theNamedCard->controller()->game->putInZone(theNamedCard, theNamedCard->currentZone, source->controller()->game->battlefield, noEvent); // Fixed a problem with double activation of card abilities.
                else if (asCopy && theNamedCard->isPermanent()){
                    theNamedCard->isToken = 0; // Fixed a bug when using copied option with namedcard option.
                    copy = _target->controller()->game->putInZone(theNamedCard, theNamedCard->currentZone, source->controller()->game->stack, noEvent);
                    copy->isToken = 1; // Fixed a bug when using copied option with namedcard option.
                    copy = _target->controller()->game->putInZone(copy, copy->currentZone, source->controller()->game->battlefield, noEvent); // Fixed a problem with copied option for permanent.
                }
                else
                    copy = theNamedCard->controller()->game->putInZone(theNamedCard, theNamedCard->currentZone, source->controller()->game->stack, noEvent); // Fixed a bug when using noevent option with namedcard option.
            }
            else
            {
                if (putinplay && theNamedCard->isPermanent())
                    copy = theNamedCard->controller()->game->putInZone(theNamedCard, theNamedCard->currentZone, source->controller()->game->battlefield, noEvent); // Fixed a problem with double activation of card abilities.
                else if (asCopy && theNamedCard->isPermanent()){
                    theNamedCard->isToken = 0; // Fixed a bug when using copied option with namedcard option.
                    copy = _target->controller()->game->putInZone(theNamedCard, theNamedCard->currentZone, source->controller()->game->stack, noEvent);
                    copy->isToken = 1; // Fixed a bug when using copied option with namedcard option.
                    copy = _target->controller()->game->putInZone(copy, copy->currentZone, source->controller()->game->battlefield, noEvent); // Fixed a problem with copied option for permanent.
                }
                else
                    copy = theNamedCard->controller()->game->putInZone(theNamedCard, theNamedCard->currentZone, source->controller()->game->stack, noEvent); // Fixed a bug when using noevent option with namedcard option.
            }
            if(!copy){
                this->forceDestroy = true;
                processed = false;
                return 0;
            }
            copy->changeController(source->controller(),true);
            if(asNormalMadness)
                copy->MadnessPlay = true;
            if(asCopy) 
                copy->isToken = 1; // Fixed a bug when using copied option with namedcard option.
            if(alternative)
                copy->alternateCostPaid[ManaCost::MANA_PAID_WITH_ALTERNATIVE] = 1;
            if(flashback)
                copy->alternateCostPaid[ManaCost::MANA_PAID_WITH_FLASHBACK] = 1;
            if(kicked > 0){
                copy->alternateCostPaid[ManaCost::MANA_PAID_WITH_KICKER] = 1;
                copy->kicked = kicked;
            }
            if(costx > 0){
                copy->castX = costx;
                copy->setX = costx;
                copy->X = costx;
            }
            if (game->targetChooser)
            {
                game->targetChooser->Owner = source->controller();
                spell = game->mLayers->stackLayer()->addSpell(copy, game->targetChooser, NULL, 1, 0);
                game->targetChooser = NULL;
            }
            else
            {
                spell = game->mLayers->stackLayer()->addSpell(copy, NULL, NULL, 1, 0);
            }
            if (copy->has(Constants::STORM))
            {
                int storm = source->controller()->game->stack->seenThisTurn("*", Constants::CAST_ALL) + source->controller()->opponent()->game->stack->seenThisTurn("*", Constants::CAST_ALL);
            
                for (int i = storm; i > 1; i--)
                {
                    spell = game->mLayers->stackLayer()->addSpell(copy, NULL, 0, 1, 1);

                }
            }
            if (!copy->has(Constants::STORM))
            {
                if(costx <= 0){
                    copy->X = 0;
                    copy->castX = copy->X;
                }
            }
            if(andAbility)
            {
                MTGAbility * andAbilityClone = andAbility->clone();
                andAbilityClone->target = copy;
                if(andAbility->oneShot)
                {
                    andAbilityClone->resolve();
                    SAFE_DELETE(andAbilityClone);
                }
                else
                {
                    andAbilityClone->addToGame();
                }
            }
            this->forceDestroy = true;
            processed = true;
            return 1;
        }
        Spell * spell = NULL;
        MTGCardInstance * copy = NULL;
        if ((normal || asNormalMadness) || !_target->isSorceryorInstant())
        {
            if (putinplay  && _target->isPermanent())
                copy = _target->controller()->game->putInZone(_target, _target->currentZone, source->controller()->game->battlefield, noEvent); // Fixed a problem with double activation of card abilities.
            else if (asCopy && _target->isPermanent()){
                _target->isToken = 0; // Fixed a bug when using copied option for permanent.
                copy = _target->controller()->game->putInZone(_target, _target->currentZone, source->controller()->game->stack, noEvent);
                copy->isToken = 1; // Fixed a bug when using copied option for permanent.
                copy = _target->controller()->game->putInZone(copy, copy->currentZone, source->controller()->game->battlefield, noEvent); // Fixed a problem with copied option for permanent.
            }
            else
                copy = _target->controller()->game->putInZone(_target, _target->currentZone, source->controller()->game->stack, noEvent); // Fixed a bug when using noevent option with namedcard option.
        }
        else
        {
            if (putinplay && _target->isPermanent())
                copy = _target->controller()->game->putInZone(_target, _target->currentZone, source->controller()->game->battlefield, noEvent); // Fixed a problem with double activation of card abilities.
            else if (asCopy && _target->isPermanent()){
                _target->isToken = 0; // Fixed a bug when using copied option for permanent.
                copy = _target->controller()->game->putInZone(_target, _target->currentZone, source->controller()->game->stack, noEvent);
                copy->isToken = 1; // Fixed a bug when using copied option for permanent.
                copy = _target->controller()->game->putInZone(copy, copy->currentZone, source->controller()->game->battlefield, noEvent); // Fixed a problem with copied option for permanent.
            }
            else
                copy = _target->controller()->game->putInZone(_target, _target->currentZone, source->controller()->game->stack, noEvent); // Fixed a bug when using noevent option with namedcard option.
        }
        if(!copy){
            this->forceDestroy = true;
            processed = false;
            return 0;
        }
        copy->changeController(source->controller(), true);
        if(asNormalMadness)
            copy->MadnessPlay = true;
        if(asCopy) 
            copy->isToken = 1; // Fixed a bug when using copied option for permanent.
        if(alternative)
            copy->alternateCostPaid[ManaCost::MANA_PAID_WITH_ALTERNATIVE] = 1;
        if(flashback)
            copy->alternateCostPaid[ManaCost::MANA_PAID_WITH_FLASHBACK] = 1;
        if(kicked > 0){
            copy->alternateCostPaid[ManaCost::MANA_PAID_WITH_KICKER] = 1;
            copy->kicked = kicked;
        }
        if(costx > 0){
            copy->castX = costx;
            copy->setX = costx;
            copy->X = costx;
        }
        if (game->targetChooser)
        {
            game->targetChooser->Owner = source->controller();
            if(putinplay)
            {
                spell =  NEW Spell(game, 0, copy, game->targetChooser, NULL, 1);
                spell->resolve();
            }
            else
                spell = game->mLayers->stackLayer()->addSpell(copy, game->targetChooser, NULL, 1, 0);
            game->targetChooser = NULL;
        }
        else
        {
            if(putinplay)
            {
                spell =  NEW Spell(game, 0, copy, NULL, NULL, 1);
                spell->resolve();
            }
            else if(!flipped)
                spell = game->mLayers->stackLayer()->addSpell(copy, NULL, NULL, 1, 0);
        }
        if (copy->has(Constants::STORM))
        {
            int storm = _target->controller()->game->stack->seenThisTurn("*", Constants::CAST_ALL) + source->controller()->opponent()->game->stack->seenThisTurn("*", Constants::CAST_ALL);
            
            for (int i = storm; i > 1; i--)
            {
                spell = game->mLayers->stackLayer()->addSpell(copy, NULL, 0, 1, 1);

            }
        }
        if (!copy->has(Constants::STORM))
        {
            if(costx <= 0){
                copy->X = _target->X;
                copy->castX = copy->X;
            }
        }
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = copy;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        this->forceDestroy = true;
        processed = true;
        return 1;
    }
    return 0;
}

const string AACastCard::getMenuText()
{
    if(nameThis.size())
        return nameThis.c_str();
    if(putinplay)
        return "Put Into Play";
    return "Cast Card";
}

AACastCard * AACastCard::clone() const
{
    AACastCard * a = NEW AACastCard(*this);
    if(tc)
    a->tc = tc->clone();
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}
AACastCard::~AACastCard()
{
    SAFE_DELETE(tc);
    SAFE_DELETE(andAbility);
}

//Tutorial Messaging

ATutorialMessage::ATutorialMessage(GameObserver* observer, MTGCardInstance * source, string message, int limit)
    : MTGAbility(observer, 0, source), IconButtonsController(observer->getInput(), 0, 0), mLimit(limit)
{
    mBgTex = NULL;

    mElapsed = 0;
    mIsImage = false;

    for (int i = 0; i < 9; i++)
        mBg[i] = NULL;

    if(game->getResourceManager())
    {
        string gfx = game->getResourceManager()->graphicsFile(message);
        if (fileExists(gfx.c_str()))
        {
            mIsImage = true;
            mMessage = message;
        }
        else
        {
            mMessage = _(message); //translate directly here, remove this and translate at rendering time if it bites us
            ReplaceString(mMessage, "\\n", "\n");
        }
    }

    if (mIsImage)
    {
        mX = SCREEN_WIDTH_F / 2;
        mY = SCREEN_HEIGHT_F / 2;

    }
    else
    {
        mX = 0;
        mY = -SCREEN_HEIGHT_F - 0.1f; //Offscreen
    }
    mDontShow = mUserCloseRequest = (mLimit > 0) && (alreadyShown() >= mLimit);

    if(mDontShow)
        forceDestroy = 1;
}


string ATutorialMessage::getOptionName()
{
    std::stringstream out;
    out << "tuto_";
    out << hash_djb2(mMessage.c_str());
    return out.str();
}

int ATutorialMessage::alreadyShown()
{
    return options[getOptionName()].number;
}

bool ATutorialMessage::CheckUserInput(JButton key)
{
    if (mUserCloseRequest) return false;

    if(key == JGE_BTN_SEC || key == JGE_BTN_OK)
    {
        ButtonPressed(0, 1);
        return true;
    }

    //Required for Mouse/touch input
    IconButtonsController::CheckUserInput(key);

    return true; //this ability is modal, so it catches all key events until it gets closed
}

void ATutorialMessage::Update(float dt)
{
    if (!game->mLayers->stackLayer()->getCurrentTutorial() && !mDontShow)
        game->mLayers->stackLayer()->setCurrentTutorial(this);

    if (game->mLayers->stackLayer()->getCurrentTutorial() != this)
        return;

    if (mUserCloseRequest && mY < -SCREEN_HEIGHT)
        mDontShow = true;

    if (mDontShow)
    {
        game->mLayers->stackLayer()->setCurrentTutorial(0);
        forceDestroy = 1;
        return;
    }

    mElapsed += dt;

    if(!mUserCloseRequest)
        IconButtonsController::Update(dt);

    if (mIsImage)
        return;

    //Below this only affects "text" mode
    if (!mUserCloseRequest && mY < 0)
    {
        mY = -SCREEN_HEIGHT + (SCREEN_HEIGHT * mElapsed / 0.75f); //Todo: more physical drop-in.
        if (mY >= 0)
            mY = 0;
    }
    else if (mUserCloseRequest && mY > -SCREEN_HEIGHT)
    {
        mY = -(SCREEN_HEIGHT * mElapsed / 0.75f);
    }
}

void ATutorialMessage::ButtonPressed(int, int)
{
    //TODO : cancel ALL tips/tutorials for JGE_BTN_SEC?
    if (mLimit)
    {
        string optionName = getOptionName();
        options[optionName].number = options[optionName].number + 1;
        options.save(); //TODO: if we experience I/O slowness in tutorials, move this save at the end of a turn, or at the end of the game.
    }
    mElapsed = 0;
    mUserCloseRequest = true;
}

void ATutorialMessage::Render()
{
    if (mDontShow)
        return;

    if (mY < -SCREEN_HEIGHT)
        return;

    if (!mBgTex)
    {
        if (mIsImage)
        {
            mBgTex = game->getResourceManager()->RetrieveTexture(mMessage, RETRIEVE_LOCK);
            if (mBgTex)
            {
                mBg[0] = NEW JQuad(mBgTex, 0, 0, (float) mBgTex->mWidth, (float) mBgTex->mHeight);
                mBg[0]->SetHotSpot(mBg[0]->mWidth / 2, mBg[0]->mHeight / 2);

                //Continue Button
                JQuadPtr quad =  game->getResourceManager()->RetrieveQuad("iconspsp.png", 4 * 32, 0, 32, 32, "iconpsp4", RETRIEVE_MANAGE);
                quad->SetHotSpot(16, 16);
                IconButton * iconButton = NEW IconButton(1, this, quad.get(), 0, mBg[0]->mHeight / 2, 0.7f, Fonts::MAGIC_FONT, _("continue"), 0, 16, true);
                Add(iconButton);
            }

            if (options[Options::SFXVOLUME].number > 0)
            {
                game->getResourceManager()->PlaySample("tutorial.wav");
            }
        }
        else
        {
            mBgTex = game->getResourceManager()->RetrieveTexture("taskboard.png", RETRIEVE_LOCK);

            float unitH = static_cast<float> (mBgTex->mHeight / 4);
            float unitW = static_cast<float> (mBgTex->mWidth / 4);
            if (unitH == 0 || unitW == 0) return;

            if (mBgTex)
            {
                mBg[0] = NEW JQuad(mBgTex, 0, 0, unitW, unitH);
                mBg[1] = NEW JQuad(mBgTex, unitW, 0, unitW * 2, unitH);
                mBg[2] = NEW JQuad(mBgTex, unitW * 3, 0, unitW, unitH);
                mBg[3] = NEW JQuad(mBgTex, 0, unitH, unitW, unitH * 2);
                mBg[4] = NEW JQuad(mBgTex, unitW, unitH, unitW * 2, unitH * 2);
                mBg[5] = NEW JQuad(mBgTex, unitW * 3, unitH, unitW, unitH * 2);
                mBg[6] = NEW JQuad(mBgTex, 0, unitH * 3, unitW, unitH);
                mBg[7] = NEW JQuad(mBgTex, unitW, unitH * 3, unitW * 2, unitH);
                mBg[8] = NEW JQuad(mBgTex, unitW * 3, unitH * 3, unitW, unitH);
            }

            //Continue Button
            JQuadPtr quad =  game->getResourceManager()->RetrieveQuad("iconspsp.png", 4 * 32, 0, 32, 32, "iconpsp4", RETRIEVE_MANAGE);
            quad->SetHotSpot(16, 16);
            IconButton * iconButton = NEW IconButton(1, this, quad.get(), SCREEN_WIDTH_F / 2,  SCREEN_HEIGHT_F - 60, 0.7f, Fonts::MAGIC_FONT, _("continue"), 0, 16, true);
            Add(iconButton);

            mSH = 64 / unitH;
            mSW = 64 / unitW;

            if (options[Options::SFXVOLUME].number > 0)
            {
                game->getResourceManager()->PlaySample("chain.wav");
            }
        }
    }

    JRenderer * r = JRenderer::GetInstance();

    //Render background board
    if (mBgTex)
    {
        if (mIsImage)
        {
            int alpha = mUserCloseRequest ? MAX(0, 255 - (int)(mElapsed * 500)) : MIN(255, (int)(mElapsed * 500)) ;
            if (mUserCloseRequest && alpha == 0)
                mDontShow = true;

            r->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(alpha / 2,0,0,0));
            mBg[0]->SetColor(ARGB(alpha,255,255,255));
            r->RenderQuad(mBg[0], SCREEN_WIDTH_F /2 , SCREEN_HEIGHT_F / 2 , 0);
            IconButtonsController::SetColor(ARGB(alpha,255,255,255));
        }
        else 
        {
            //Setup fonts.
            WFont * f2 = game->getResourceManager()->GetWFont(Fonts::MAGIC_FONT);
            f2->SetColor(ARGB(255, 205, 237, 240));

            r->FillRect(0, mY, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(128,0,0,0));
            r->RenderQuad(mBg[0], 0, mY, 0, mSW, mSH); //TL
            r->RenderQuad(mBg[2], SCREEN_WIDTH - 64, mY, 0, mSW, mSH); //TR
            r->RenderQuad(mBg[6], 0, mY + SCREEN_HEIGHT - 64, 0, mSW, mSH); //BL
            r->RenderQuad(mBg[8], SCREEN_WIDTH - 64, mY + SCREEN_HEIGHT - 64, 0, mSW, mSH); //BR

            //Stretch the sides
            float stretchV = (144.0f / 128.0f) * mSH;
            float stretchH = (176.0f / 128.0f) * mSW;
            r->RenderQuad(mBg[3], 0, mY + 64, 0, mSW, stretchV); //L
            r->RenderQuad(mBg[5], SCREEN_WIDTH - 64, mY + 64, 0, mSW, stretchV); //R
            r->RenderQuad(mBg[1], 64, mY, 0, stretchH, mSH); //T1
            r->RenderQuad(mBg[1], 240, mY, 0, stretchH, mSH); //T1
            r->RenderQuad(mBg[7], 64, mY + 208, 0, stretchH, mSH); //B1
            r->RenderQuad(mBg[7], 240, mY + 208, 0, stretchH, mSH); //B1
            r->RenderQuad(mBg[4], 64, mY + 64, 0, stretchH, stretchV); //Center1
            r->RenderQuad(mBg[4], 240, mY + 64, 0, stretchH, stretchV); //Center2
        }
    }
    else
    {
        r->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(128,0,0,0));
        r->FillRect(10, 10 + mY, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, ARGB(128,0,0,0));
    }

    if (!mBgTex || !mIsImage)
    {
        float posX = 40, posY = mY + 20;
        string title = _("Help");

        WFont * f = game->getResourceManager()->GetWFont(Fonts::MAGIC_FONT);
        WFont * f3 = game->getResourceManager()->GetWFont(Fonts::MENU_FONT); //OPTION_FONT
        f->SetColor(ARGB(255, 55, 46, 34));
        f3->SetColor(ARGB(255, 219, 206, 151));

        f3->DrawString(title.c_str(), static_cast<float> ((SCREEN_WIDTH - 20) / 2 - title.length() * 4), posY);
        posY += 30;

        f->DrawString(_(mMessage).c_str(), posX, posY);
    
        f->SetScale(1);
    }

    IconButtonsController::Render();

}

ATutorialMessage * ATutorialMessage::clone() const
{
    ATutorialMessage * copy =  NEW ATutorialMessage(*this);
    copy->mUserCloseRequest = (copy->alreadyShown() > 0);
    return copy;
}

ATutorialMessage::~ATutorialMessage()
{
    if (mBgTex)
    {
        game->getResourceManager()->Release(mBgTex);
        for (int i = 0; i < 9; i++)
            SAFE_DELETE(mBg[i]);
    }
}

// utility functions

// Given a delimited string of abilities, add the ones to the list that are "Basic"  MTG abilities
void PopulateAbilityIndexVector(list<int>& abilities, const string& abilityStringList, char delimiter)
{
    vector<string> abilitiesList = split(abilityStringList, delimiter);
    for (vector<string>::iterator iter = abilitiesList.begin(); iter != abilitiesList.end(); ++iter)
    {
        int abilityIndex = Constants::GetBasicAbilityIndex(*iter);

        if (abilityIndex != -1)
            abilities.push_back(abilityIndex);
    }
}

void PopulateColorIndexVector(list<int>& colors, const string& colorStringList, char delimiter)
{
    vector<string> abilitiesList = split(colorStringList, delimiter);
    for (vector<string>::iterator iter = abilitiesList.begin(); iter != abilitiesList.end(); ++iter)
    {
        // if the text is not a basic ability but contains a valid color add it to the color vector
        if((*iter).find("newcolors[") != string::npos){
            size_t start_pos = (*iter).find("newcolors[");
            (*iter).replace(start_pos, 10, "");
            start_pos = (*iter).find("]");
            (*iter).replace(start_pos, 1, "");
        }
        for (int colorIndex = Constants::MTG_COLOR_ARTIFACT; colorIndex < Constants::NB_Colors; ++colorIndex)
        {
            // We match now exactly the color to avoid wrong color assignment from gained abilities (e.g. protection from blue)
            if ((Constants::GetBasicAbilityIndex(*iter) == -1) && ((*iter) == Constants::MTGColorStrings[colorIndex]))
                colors.push_back(colorIndex);
        }
    }
}

void PopulateSubtypesIndexVector(list<int>& types, const string& subTypesStringList, char delimiter)
{
    vector<string> subTypesList = split(subTypesStringList, delimiter);
    for (vector<string>::iterator it = subTypesList.begin(); it != subTypesList.end(); ++it)
    {
        string subtype = *it;
        size_t id = MTGAllCards::findType(subtype);
        if (id != string::npos)
            types.push_back(id);
    }
}
