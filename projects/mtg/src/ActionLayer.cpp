#include "PrecompiledHeader.h"

#include "ActionLayer.h"
#include "GameObserver.h"
#include "Targetable.h"
#include "WEvent.h"

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
    e->destroy();

    i = getIndexOf(e); //the destroy event might have changed the contents of mObjects, so we get the index again
    if (i == -1)
        return 0; //Should not happen, it means we deleted thesame object twice?

    mObjects.erase(mObjects.begin() + i);
    return 1;

}

int ActionLayer::moveToGarbage(ActionElement * e)
{
    if (removeFromGame(e))
    {
        garbage.push_back(e);
        return 1;
    }
    return 0;

}

int ActionLayer::cleanGarbage()
{
    for (size_t i = 0; i < garbage.size(); ++i)
    {
        delete (garbage[i]);
    }
    garbage.clear();
    return 1;
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
    GameObserver * g = GameObserver::GetInstance();
    if (g->mExtraPayment && key == JGE_BTN_SEC)
    {
        g->mExtraPayment = NULL;
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
    GameObserver* game = GameObserver::GetInstance();
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
                game->removeObserver(currentAction);
        }
    }
    int newPhase = game->getCurrentGamePhase();
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
        if (ae && !ae->tc->validTargetsExist())
        {
            cantCancel = 0;
            cancelCurrentAction();
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
        return currentWaitingAction->tc;
    return NULL;
}

int ActionLayer::cancelCurrentAction()
{
    ActionElement * ae = isWaitingForAnswer();
    if (!ae)
        return 0;
    if (cantCancel && ae->tc->validTargetsExist())
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

    abilitiesMenu = NEW SimpleMenu(10, this, Fonts::MAIN_FONT, 100, 100, object->getDisplayName().c_str());

    for (size_t i = 0; i < mObjects.size(); i++)
    {
        ActionElement * currentAction = (ActionElement *) mObjects[i];
        if (currentAction->isReactingToTargetClick(object))
        {
            abilitiesMenu->Add(i, currentAction->getMenuText());
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
    if (controlid >= 0 && controlid < static_cast<int>(mObjects.size()))
    {
        ActionElement * currentAction = (ActionElement *) mObjects[controlid];
        currentAction->reactToTargetClick(menuObject);
        menuObject = 0;
    }
    else if (controlid == kCancelMenuID)
    {
        GameObserver::GetInstance()->mLayers->stackLayer()->endOfInterruption();
        menuObject = 0;
    }
    else
    {
        // fallthrough case. We have an id we don't recognize - do nothing, don't clear the menu!
        //assert(false);
    }
}

ActionLayer::ActionLayer()
{
    menuObject = NULL;
    abilitiesMenu = NULL;
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
