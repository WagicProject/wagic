#include "PrecompiledHeader.h"

#include "ActionLayer.h"
#include "GameObserver.h"
#include "Targetable.h"
#include "WEvent.h"
#include "AllAbilities.h"
#include "MTGRules.h"

MTGAbility* ActionLayer::getAbility(int type)
{
    for (size_t i = 1; i < mObjects.size(); i++)
    {
        MTGAbility * a = ((MTGAbility *) mObjects[i]);
        if (a->aType == type)
        {
            return a;
        }
    }
    return NULL;
}

int ActionLayer::removeFromGame(ActionElement * e)
{
    mReactions.erase(e);
    int i = getIndexOf(e);
    if (i == -1)
        return 0;

    if (isWaitingForAnswer() == e)
        setCurrentWaitingAction(NULL);
    assert(e);
    e->destroy();

    i = getIndexOf(e); //the destroy event might have changed the contents of mObjects, so we get the index again
    if (i == -1)
        return 0; //Should not happen, it means we deleted thesame object twice?
    AbilityFactory af(observer);

    MTGAbility * a = dynamic_cast<MTGAbility*>(e);
    
    if (a != NULL)
    {
        AManaProducer * manaObject = dynamic_cast<AManaProducer*>(af.getCoreAbility(a));
        if(manaObject)
        {
            for (size_t i = 0; i < manaObjects.size(); i++)
                if (manaObjects[i] == e)
                {
                    manaObjects.erase(manaObjects.begin() + i);
                }
        }
    }
    mObjects.erase(mObjects.begin() + i);
    return 1;

}

bool ActionLayer::moveToGarbage(ActionElement * e)
{
    if (removeFromGame(e))
    {
        garbage.push_back(e);
        return true;
    }
    return false;

}

void ActionLayer::cleanGarbage()
{
    for (size_t i = 0; i < garbage.size(); ++i)
    {
        delete (garbage[i]);
    }
    garbage.clear();
}

int ActionLayer::reactToClick(ActionElement * ability, MTGCardInstance * card)
{
    int result = ability->reactToClick(card);
    if (result)
        stuffHappened = 1;
    return result;
}

int ActionLayer::reactToTargetClick(ActionElement* ability, Targetable * card)
{
    int result = ability->reactToTargetClick(card);
    if (result)
        stuffHappened = 1;
    return result;
}

bool ActionLayer::CheckUserInput(JButton key)
{
    if (observer->mExtraPayment && key == JGE_BTN_SEC)
    {
        observer->mExtraPayment = NULL;
        return 1;
    }
    if (menuObject)
    {
        return false;
    }
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        if (mObjects[i] != NULL)
        {
            ActionElement * currentAction = (ActionElement *) mObjects[i];
            if (currentAction->CheckUserInput(key))
                return true;
        }
    }
    return false;
}

void ActionLayer::Update(float dt)
{
    stuffHappened = 0;
    if (menuObject)
    {
        abilitiesMenu->Update(dt);
        return;
    }
    modal = 0;
    for (int i = (int)(mObjects.size()) - 1; i >= 0; i--)
    {
        //a dirty hack, there might be cases when the mObject array gets reshaped if an ability removes some of its children abilites
        if ((int) mObjects.size() <= i)
        {
            i = (int) (mObjects.size()) - 1;
            if (i<0)
                break;
        }

        if (mObjects[i] != NULL)
        {
            ActionElement * currentAction = (ActionElement *) mObjects[i];
            if (currentAction->testDestroy())
                observer->removeObserver(currentAction);
        }
    }
    GamePhase newPhase = observer->getCurrentGamePhase();
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        if (mObjects[i] != NULL)
        {
            ActionElement * currentAction = (ActionElement *) mObjects[i];
            currentAction->newPhase = newPhase;
            currentAction->Update(dt);
            currentAction->currentPhase = newPhase;
        }
    }

    if (cantCancel)
    {
        ActionElement * ae = isWaitingForAnswer();
        int countTargets = 0;
        int maxTargets = 0;
        if(ae && ae->getActionTc())
        {
            if (!ae->getActionTc()->validTargetsExist())
            {
                cantCancel = 0;
                cancelCurrentAction();
                return;
            }
            countTargets = ae->getActionTc()->countValidTargets();
            maxTargets = ae->getActionTc()->maxtargets;
            if (countTargets < maxTargets)
            {
                /*
                @movedto(this|mygraveyard) from(mybattlefield):moveto(mybattlefield) 
                target(<2>creature[elf]|opponentgraveyard)
                and there were 3 in the grave, you have the valid amount needed, this function should not trigger
                ...however if you had only 1 in the grave, then the max targets is reset to the maximum you CAN
                use this effect on...in line with "up to" wording found on the cards with such abilities.
                without this, the game locks into a freeze state while you try to select the targets and dont have enough to
                fill the maxtargets list.
                */
                if(int(ae->getActionTc()->getNbTargets()) == countTargets-1)
                    ae->getActionTc()->done = true;
            }
        }
    }
}

void ActionLayer::Render()
{
    if (menuObject)
    {
        abilitiesMenu->Render();
        return;
    }

    for (size_t i = 0; i < mObjects.size(); i++)
    {
        if (mObjects[i] != NULL)
        {
            ActionElement * currentAction = (ActionElement *) mObjects[i];
            currentAction->Render();
        }
    }
}

void ActionLayer::setCurrentWaitingAction(ActionElement * ae)
{
    assert(!ae || !currentWaitingAction);//this assert causes crashes when may abilities overlap each other on ai. this conidiation is preexsiting.
    currentWaitingAction = ae;
    if (!ae)
        cantCancel = 0;
}

TargetChooser * ActionLayer::getCurrentTargetChooser()
{
    if (currentWaitingAction && currentWaitingAction->waitingForAnswer)
        return currentWaitingAction->getActionTc();
    return NULL;
}

int ActionLayer::cancelCurrentAction()
{
    ActionElement * ae = isWaitingForAnswer();
    if (!ae)
        return 0;
    if (cantCancel && ae->getActionTc()->validTargetsExist())
        return 0;
    ae->waitingForAnswer = 0; //TODO MOVE THIS IN ActionElement
    setCurrentWaitingAction(NULL);
    return 1;
}

ActionElement * ActionLayer::isWaitingForAnswer()
{
    if (currentWaitingAction && currentWaitingAction->waitingForAnswer)
        return currentWaitingAction;
    return NULL;
}

int ActionLayer::stillInUse(MTGCardInstance * card)
{
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        ActionElement * currentAction = (ActionElement *) mObjects[i];
        if (currentAction->stillInUse(card))
            return 1;
    }
    return 0;
}

int ActionLayer::receiveEventPlus(WEvent * event)
{
    int result = 0;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        ActionElement * currentAction = (ActionElement *) mObjects[i];
        result += currentAction->receiveEvent(event);
    }
    return 0;
}

int ActionLayer::isReactingToTargetClick(Targetable * card)
{
    int result = 0;

    if (isWaitingForAnswer())
        return -1;

    for (size_t i = 0; i < mObjects.size(); i++)
    {
        ActionElement * currentAction = (ActionElement *) mObjects[i];
        result += currentAction->isReactingToTargetClick(card);
    }
    return result;
}

int ActionLayer::reactToTargetClick(Targetable * card)
{
    int result = 0;

    ActionElement * ae = isWaitingForAnswer();
    if (ae)
        return reactToTargetClick(ae, card);

    for (size_t i = 0; i < mObjects.size(); i++)
    {
        ActionElement * currentAction = (ActionElement *) mObjects[i];
        result += currentAction->reactToTargetClick(card);
    }
    return result;
}

bool ActionLayer::getMenuIdFromCardAbility(MTGCardInstance *card, MTGAbility *ability, int& menuId)
{
    int ctr = 0;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        ActionElement * currentAction = (ActionElement *) mObjects[i];
        if (currentAction->isReactingToClick(card))
        {
            if(currentAction == ability) {
                // code corresponding to that is in setMenuObject
                menuId = ctr;
                ctr++;
            }
        }
    }

    if(ctr == 0 || ctr == 1)
    {
        return false;
    }
    else
        return true;
}

//TODO Simplify with only object !!!
int ActionLayer::isReactingToClick(MTGCardInstance * card)
{
    int result = 0;

    if (isWaitingForAnswer())
        return -1;

    for (size_t i = 0; i < mObjects.size(); i++)
    {
        ActionElement * currentAction = (ActionElement *) mObjects[i];
        if (currentAction->isReactingToClick(card))
        {
            ++result;
            mReactions.insert(currentAction);
        }
    }

    return result;
}

int ActionLayer::reactToClick(MTGCardInstance * card)
{
    int result = 0;

    ActionElement * ae = isWaitingForAnswer();
    if (ae)
        return reactToClick(ae, card);

    std::set<ActionElement*>::const_iterator iter = mReactions.begin();
    std::set<ActionElement*>::const_iterator end = mReactions.end();
    for (; iter !=end; ++iter)
    {
        result += reactToClick(*iter, card);
        if (result)
            break;
    }

#ifdef WIN32
	// if we hit this, then something strange has happened with the click logic - reactToClick()
	// should never be called if isReactingToClick() previously didn't have an object return true
    assert(!mReactions.empty());
#endif

    mReactions.clear();
    return result;
}

void ActionLayer::setMenuObject(Targetable * object, bool must)
{
    if (!object)
    {
        DebugTrace("FATAL: ActionLayer::setMenuObject");
        return;
    }
    menuObject = object;

    SAFE_DELETE(abilitiesMenu);
    abilitiesTriggered = NULL;

    abilitiesMenu = NEW SimpleMenu(observer->getInput(), 10, this, Fonts::MAIN_FONT, 100, 100, object->getDisplayName().c_str());
    abilitiesTriggered = NEW SimpleMenu(observer->getInput(), 10, this, Fonts::MAIN_FONT, 100, 100, object->getDisplayName().c_str());
    currentActionCard = NULL;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        ActionElement * currentAction = (ActionElement *) mObjects[i];
        if (currentAction->isReactingToTargetClick(object))
        {
            if(dynamic_cast<MTGAbility*>(currentAction)->getCost()||dynamic_cast<PermanentAbility*>(currentAction))
            {
                abilitiesMenu->Add(i, currentAction->getMenuText());
            }
            else
            {
                //the only time this condiation is hit is when we are about to display a menu of abilities
                //which were triggered through a triggered ability or abilities such as multiple target(
                //and may abilities appearing on cards ie: auto=may draw:1
                //this prevents abilities activated otherwise from displaying on the same menu as "triggered" and
                //"put in play" abilities. an activated ability of a card should never share a menu with
                //a triggered or may ability as it leads to exploits.
                //only exception is perminent abilities such as "cast card normally" which can share the menu with autohand=
                abilitiesTriggered->Add(i, currentAction->getMenuText());
            }
        }
    }
    if(abilitiesTriggered->mCount)
    {
        SAFE_DELETE(abilitiesMenu);
        abilitiesMenu = abilitiesTriggered;
    }
    else
    {
        SAFE_DELETE(abilitiesTriggered);
    }
    if (!must)
        abilitiesMenu->Add(kCancelMenuID, "Cancel");
    else
        cantCancel = 1;
    modal = 1;
}

void ActionLayer::setCustomMenuObject(Targetable * object, bool must,vector<MTGAbility*>abilities)
{
    if (!object)
    {
        DebugTrace("FATAL: ActionLayer::setCustomMenuObject");
        return;
    }
    menuObject = object;
    SAFE_DELETE(abilitiesMenu);
    abilitiesMenu = NEW SimpleMenu(observer->getInput(), 10, this, Fonts::MAIN_FONT, 100, 100, object->getDisplayName().c_str());
    currentActionCard = NULL;
    abilitiesMenu->isMultipleChoice = false;
    if(abilities.size())
    {
        abilitiesMenu->isMultipleChoice = true;
        ActionElement * currentAction = NULL;
        for(int w = 0; w < int(abilities.size());w++)
        {
            currentAction = (ActionElement*)abilities[w];
            currentActionCard = (MTGCardInstance*)abilities[0]->target;
            abilitiesMenu->Add(mObjects.size()-1, currentAction->getMenuText(),"",false);
        }
    }
    if (!must)
        abilitiesMenu->Add(kCancelMenuID, "Cancel");
    else
        cantCancel = 1;
    modal = 1;
}

void ActionLayer::doReactTo(int menuIndex)
{

    if (menuObject)
    {
        int controlid = abilitiesMenu->mObjects[menuIndex]->GetId();
        DebugTrace("ActionLayer::doReactTo " << controlid);
        ButtonPressed(0, controlid);
    }
}

void ActionLayer::ButtonPressed(int controllerid, int controlid)
{
    stringstream stream;
    for(size_t i = 0; i < abilitiesMenu->mObjects.size(); i++)
    {   // this computes the reverse from the doReactTo method
        if(abilitiesMenu->mObjects[i]->GetId() == controlid)
        {
            stream << "choice " << i;
            observer->logAction(observer->currentActionPlayer, stream.str());
            break;
        }
    }

    if(this->abilitiesMenu && this->abilitiesMenu->isMultipleChoice)
    {
        return ButtonPressedOnMultipleChoice();
    }
    if (controlid >= 0 && controlid < static_cast<int>(mObjects.size()))
    {
        ActionElement * currentAction = (ActionElement *) mObjects[controlid];
        currentAction->reactToTargetClick(menuObject);
        menuObject = 0;
        currentActionCard = NULL;
    }
    else if (controlid == kCancelMenuID)
    {
        observer->mLayers->stackLayer()->endOfInterruption(false);
        menuObject = 0;
        currentActionCard = NULL;
    }
    else
    {
        // fallthrough case. We have an id we don't recognize - do nothing, don't clear the menu!
        //assert(false);
    }
}

void ActionLayer::ButtonPressedOnMultipleChoice(int choice)
{
    int currentMenuObject = -1;
    for(int i = int(mObjects.size()-1);i > 0;i--)
    {
        //the currently displayed menu is not always the currently listenning action object
        //find the menu which is displayed.
        MenuAbility * ma = dynamic_cast<MenuAbility *>(mObjects[i]);//find the active menu
        if(ma && ma->triggered)
        {
            currentMenuObject = i;
            break;
        }
    }
    if (currentMenuObject >= 0 && currentMenuObject < static_cast<int>(mObjects.size()))
    {
        ActionElement * currentAction = (ActionElement *) mObjects[currentMenuObject];
        currentAction->reactToChoiceClick(menuObject,choice > -1?choice:this->abilitiesMenu->getmCurr(),currentMenuObject);
        MenuAbility * ma = dynamic_cast<MenuAbility *>(mObjects[currentMenuObject]);
        if(ma)
            ma->removeMenu = true;//we clicked something, close menu now.
    }
    else if (currentMenuObject == kCancelMenuID)
    {
        observer->mLayers->stackLayer()->endOfInterruption(false);
    }
    menuObject = 0;
    currentActionCard = NULL;
}

ActionLayer::ActionLayer(GameObserver *observer)
    : GuiLayer(observer)
{
    menuObject = NULL;
    abilitiesMenu = NULL;
    abilitiesTriggered = NULL;
    stuffHappened = 0;
    currentWaitingAction = NULL;
    cantCancel = 0;
}

ActionLayer::~ActionLayer()
{
    while(mObjects.size())
        moveToGarbage((ActionElement *) mObjects[mObjects.size() - 1]);
    SAFE_DELETE(abilitiesMenu);
    cleanGarbage();
}
