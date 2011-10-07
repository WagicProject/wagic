#include "PrecompiledHeader.h"

#include "GuiLayers.h"
#include "Player.h"
#include "AllAbilities.h"

GuiLayer::GuiLayer(GameObserver *observer)
    : observer(observer)
{
    modal = 0;
    hasFocus = false;
    mCurr = 0;
    mActionButton = JGE_BTN_OK;
}

GuiLayer::~GuiLayer()
{
    resetObjects();
}

void GuiLayer::Add(JGuiObject *object)
{
    mObjects.push_back(object);
    AbilityFactory af(observer);
    if(dynamic_cast<MTGAbility*>(object))
    {
        AManaProducer * manaObject = dynamic_cast<AManaProducer*>(af.getCoreAbility((MTGAbility*)object));
        if(manaObject)
        {
            manaObjects.push_back(object);
        }
    }
}

int GuiLayer::Remove(JGuiObject *object)
{

    AbilityFactory af(observer);
    if(dynamic_cast<MTGAbility*>(object))
    {
        AManaProducer * manaObject = dynamic_cast<AManaProducer*>(af.getCoreAbility((MTGAbility*)object));
        if(manaObject)
        {
            for (size_t i = 0; i < manaObjects.size(); i++)
                if (manaObjects[i] == object)
                {
                    manaObjects.erase(manaObjects.begin() + i);
                }
        }
    }
    for (size_t i = 0; i < mObjects.size(); i++)
        if (mObjects[i] == object)
        {
            delete mObjects[i];
            mObjects.erase(mObjects.begin() + i);
            if (mCurr == (int)(mObjects.size()))
                mCurr = 0;
            return 1;
        }
    return 0;
}

int GuiLayer::getMaxId()
{
    return (int) (mObjects.size());
}

void GuiLayer::Render()
{
    for (size_t i = 0; i < mObjects.size(); i++)
        if (mObjects[i] != NULL)
            mObjects[i]->Render();
}

void GuiLayer::Update(float dt)
{
    for (size_t i = 0; i < mObjects.size(); i++)
        if (mObjects[i] != NULL)
            mObjects[i]->Update(dt);
}

void GuiLayer::resetObjects()
{
    for (size_t i = 0; i < mObjects.size(); i++)
        if (mObjects[i])
        {
            // big, ugly hack around CardView / MTGCardInstance ownership problem - these two classes have an interdependency with naked pointers,
            // but the order of destruction can leave a dangling pointer reference inside of a CardView, which attempts to clean up its own pointer
            // reference in an MTGCardInstance when it's destroyed.  Ideally, CardView should only hold onto a weak reference, but that's a bigger overhaul.
            // For now, if we get here, clear out the MTGCardInstance pointer of a CardView before calling delete.
            CardView* cardView = dynamic_cast<CardView*>(mObjects[i]);
            if (cardView)
            {
                cardView->card = NULL;
            }
            delete mObjects[i];
        }
    mObjects.clear();
    mCurr = 0;
}

int GuiLayer::getIndexOf(JGuiObject * object)
{
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        if (mObjects[i] == object)
            return i;
    }
    return -1;
}

JGuiObject * GuiLayer::getByIndex(int index)
{
    return mObjects[index];
}

