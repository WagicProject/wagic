#include "PrecompiledHeader.h"

#include "CardGui.h"
#include "Navigator.h"

namespace
{
    const Pos kDefaultCardPosition(300, 150, 1.0, 0.0, 220);
    // the diagonal length of a card, doubled
    const float kOverrideDistance = sqrtf(powf(CardGui::Width, 2) + powf(CardGui::Height, 2)) * 2;

    enum
    {
        kCardZone_Unknown = -1,
        kCardZone_PlayerHand = 0,
        kCardZone_PlayerAvatar,
        kCardZone_PlayerLibrary,
        kCardZone_PlayerGraveyard,
        kCardZone_PlayerLands,
        kCardZone_PlayerCreatures,
        kCardZone_PlayerEnchantmentsAndArtifacts,
        kCardZone_AIHand,
        kCardZone_AIAvatar,
        kCardZone_AILibrary,
        kCardZone_AIGraveyard,
        kCardZone_AILands,
        kCardZone_AICreatures,
        kCardZone_AIEnchantmentsAndArtifacts
    };
}

/**
 **  Helper class to Navigator. Represents a group of cards on the battlefield.
 */
class CardZone
{
public:

    /*
     **
     */
    CardZone() :
        mCurrentCard(0)
    {
    }

    /*
     **
     */
    void AddCard(PlayGuiObject* inCard)
    {
        mCards.push_back(inCard);
        inCard->zoom = 1.0f;
    }

    /*
     **
     */
    void RemoveCard(PlayGuiObject* inCard)
    {
        bool found = false;
        for (size_t index = 0; index < mCards.size(); ++index)
        {
            if (mCards[index] == inCard)
            {
                // if mCurrentCard points to a card at the end of an item but we're
                // about to delete something earlier in the container, mCurrentCard
                // won't be pointing anymore to the same element, so shift it
                if (mCurrentCard >= index)
                {
                    if (mCurrentCard > 0)
                        --mCurrentCard;
                }
                mCards[index]->zoom = 1.0f;

                mCards.erase(mCards.begin() + index);
                found = true;
                break;
            }
        }

        assert(found);
    }

    /*
     ** Generic handling of navigation - left/right moves through the container,
     ** up/down is rejected & moves to the next zone
     */
    virtual bool HandleSelection(JButton inKey)
    {
        bool changeZone = true;
        size_t oldIndex = mCurrentCard;
        if (inKey == JGE_BTN_LEFT)
        {
            if (mCurrentCard > 0)
            {
                --mCurrentCard;
                changeZone = false;
            }
        }

        if (inKey == JGE_BTN_RIGHT)
        {
            if (mCurrentCard + 1 < mCards.size())
            {
                ++mCurrentCard;
                changeZone = false;
            }
        }

        if (oldIndex != mCurrentCard)
        {
            AnimateSelectionChange(oldIndex, true);
            AnimateSelectionChange(mCurrentCard, false);
        }
        return changeZone;
    }

    /*
     **
     */
    void AnimateSelectionChange(size_t inIndex, bool inLeaving)
    {
        if (inIndex < mCards.size())
        {
            if (inLeaving)
            {
                mCards[inIndex]->zoom = 1.0f;
                mCards[inIndex]->Leaving(JGE_BTN_NONE);
            }
            else
            {
                mCards[inIndex]->zoom = 1.4f;
                mCards[inIndex]->Entering();
            }
        }
    }

    /*
     ** Return a neighbour CardZone (ie where to navigate to given a direction)
     ** If there is no neighbour in that direction, return self (this ensures that
     ** the parent Navigator class always has a legal currentZone pointer)
     */
    CardZone* GetNeighbour(JButton inDirection)
    {
        CardZone* neighbour = this;
        if (mNeighbours[inDirection])
            neighbour = mNeighbours[inDirection];

        return neighbour;
    }

    /*
     ** When a zone change occurs, this will be called. This allows a zone
     ** to 'pass through' so to speak, if a zone is empty, allow the navigation to
     ** travel to the next neighbour
     */
    virtual CardZone* EnterZone(JButton inDirection)
    {
        if (mCards.empty())
        {
            if (inDirection != JGE_BTN_NONE)
            {
                CardZone* nextNeighbour = GetNeighbour(inDirection);
                if (nextNeighbour)
                    return nextNeighbour->EnterZone(inDirection);
            }
        }

        // when entering a zone, animate the selection of the current card
        AnimateSelectionChange(mCurrentCard, false);
        return this;
    }

    /*
     **
     */
    void LeaveZone(JButton inDirection)
    {
        AnimateSelectionChange(mCurrentCard, true);
    }

    /*
     **
     */
    PlayGuiObject* GetCurrentCard()
    {
        PlayGuiObject* current = NULL;
        if (mCards.size())
        {
            if (mCurrentCard < mCards.size())
            {
                current = mCards[mCurrentCard];
            }
        }
        return current;
    }

    std::vector<PlayGuiObject*> mCards;
    size_t mCurrentCard;

    // you'll typically have up to 4 neighbours, ie left/right/up/down
    std::map<JButton, CardZone*> mNeighbours;
};

/*
 ** Derivation of CardZone, but with special key handling for the grid style layout,
 ** where we need to navigate up/down as well as left/right
 */
class GridCardZone: public CardZone
{
public:
    GridCardZone(bool inEnforceAxisAlignment = false) :
        mEnforceAxisAlignment(inEnforceAxisAlignment)
    {
    }

    virtual bool HandleSelection(JButton inKey)
    {
        size_t oldIndex = mCurrentCard;

        float minDistance = 100000;
        int selectedCardIndex = -1;
        bool isHorizontal = (inKey == JGE_BTN_LEFT || inKey == JGE_BTN_RIGHT);

        for (size_t index = 0; index < mCards.size(); ++index)
        {
            // skip yourself
            if (mCurrentCard == index)
                continue;

            // skip if the card isn't on the same axis that we're stepping in
            // this flag is an optional override. If enabled, it forces you to only be able to thumb over to the next card
            // that is exactly on the same x or y coordinate axis - any card not strictly parallel is ignored.
            if (mEnforceAxisAlignment)
            {
                if ((isHorizontal && mCards[mCurrentCard]->y != mCards[index]->y) || (!isHorizontal && mCards[mCurrentCard]->x
                                != mCards[index]->x))
                {
                    continue;
                }
            }

            // if it's going in the wrong direction, skip
            if (inKey == JGE_BTN_RIGHT && mCards[index]->x <= mCards[mCurrentCard]->x)
                continue;
            if (inKey == JGE_BTN_LEFT && mCards[index]->x >= mCards[mCurrentCard]->x)
                continue;
            if (inKey == JGE_BTN_DOWN && mCards[index]->y <= mCards[mCurrentCard]->y)
                continue;
            if (inKey == JGE_BTN_UP && mCards[index]->y >= mCards[mCurrentCard]->y)
                continue;

            // we've found a card on the same axis, stash its value & compare against the previous
            float yDiff = fabs(mCards[mCurrentCard]->y - mCards[index]->y);
            float xDiff = fabs(mCards[mCurrentCard]->x - mCards[index]->x);
            float distance = sqrtf(yDiff * yDiff + xDiff * xDiff);

            // CardSelector does this thing where if the distance in the axis you're moving is less than the distance od the card on the opposite axis,
            // it would ignore the move - this made for some weird behavior where the UI wouldn't let you move in certain directions when a card looked reachable.
            // instead, I'm using a different logic - if there's a card in the direction that you're stepping, and it's within a defined radius, go for it.
            if (distance < minDistance && distance < kOverrideDistance)
            {
                minDistance = distance;
                selectedCardIndex = index;
            }
        }

        bool changeZone = true;
        if (selectedCardIndex != -1)
        {
            mCurrentCard = selectedCardIndex;
            changeZone = false;

            if (oldIndex != mCurrentCard)
            {
                AnimateSelectionChange(oldIndex, true);
                AnimateSelectionChange(mCurrentCard, false);
            }
        }

        return changeZone;
    }

protected:
    bool mEnforceAxisAlignment;
};

/*
 **
 */
class HandCardZone: public GridCardZone
{
public:

    /*
     ** the card hand zone operates slightly differently than the default zones:
     ** if entering via up/down,
     ** set the current card selection to the bottom/top card
     */
    virtual CardZone* EnterZone(JButton inDirection)
    {
        // TODO, check if the hand is flattened
        if (mCards.size())
        {
            if (inDirection == JGE_BTN_UP)
                mCurrentCard = mCards.size() - 1;
            else if (inDirection == JGE_BTN_DOWN)
                mCurrentCard = 0;
        }

        return CardZone::EnterZone(inDirection);
    }
};

/*
 **
 */
class LandCardZone: public GridCardZone
{
public:
    virtual CardZone* EnterZone(JButton inDirection);
};

/*
 **
 */
class CreatureCardZone: public GridCardZone
{
public:
    virtual CardZone* EnterZone(JButton inDirection);
};

/*
 ** The base class dictates normally, if you enter a zone and it's empty, move to
 ** the next zone in the same direction.
 ** Adding an override here - if there are no creatures in play but there are land,
 ** and we're moving horizontally, jump up to the land instead
 */
CardZone* CreatureCardZone::EnterZone(JButton inDirection)
{
    if ((inDirection == JGE_BTN_LEFT || inDirection == JGE_BTN_RIGHT) && mCards.empty())
    {
        LandCardZone* landZone = dynamic_cast<LandCardZone*> (mNeighbours[JGE_BTN_DOWN]);
        if (landZone == NULL)
        {
            landZone = dynamic_cast<LandCardZone*> (mNeighbours[JGE_BTN_UP]);
        }

        if (landZone && !landZone->mCards.empty())
        {
            return landZone->EnterZone(inDirection);
        }
    }

    return CardZone::EnterZone(inDirection);
}

/*
 ** Same pattern to the CreatureCardZone pattern - if moving through an empty land zone,
 ** set the focus on the land zone if it has cards
 */
CardZone* LandCardZone::EnterZone(JButton inDirection)
{
    if ((inDirection == JGE_BTN_LEFT || inDirection == JGE_BTN_RIGHT) && mCards.empty())
    {
        CreatureCardZone* creatureZone = dynamic_cast<CreatureCardZone*> (mNeighbours[JGE_BTN_DOWN]);
        if (creatureZone == NULL)
        {
            creatureZone = dynamic_cast<CreatureCardZone*> (mNeighbours[JGE_BTN_UP]);
        }

        if (creatureZone && !creatureZone->mCards.empty())
        {
            return creatureZone->EnterZone(inDirection);
        }
    }

    return CardZone::EnterZone(inDirection);
}

/*
 ** Constructor.  All the navigation logic is initialized here, by pairing up each card zone with a set of neighbours.
 */
Navigator::Navigator(GameObserver *observer, DuelLayers* inDuelLayers) :
    CardSelectorBase(observer), mDrawPosition(kDefaultCardPosition), mDuelLayers(inDuelLayers), mLimitorEnabled(false)
{
    assert(mDuelLayers);

    // initialize the cardZone layout
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_PlayerHand, NEW HandCardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_PlayerAvatar, NEW CardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_PlayerLibrary, NEW CardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_PlayerGraveyard, NEW CardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_PlayerLands, NEW LandCardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_PlayerCreatures, NEW CreatureCardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_PlayerEnchantmentsAndArtifacts, NEW GridCardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_AIHand, NEW CardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_AIAvatar, NEW CardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_AILibrary, NEW CardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_AIGraveyard, NEW CardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_AILands, NEW LandCardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_AICreatures, NEW CreatureCardZone()));
    mCardZones.insert(std::pair<int, CardZone*>(kCardZone_AIEnchantmentsAndArtifacts, NEW GridCardZone()));

    // navigation rules: each zone has up to 4 neighbours, specified here
    mCardZones[kCardZone_PlayerHand]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_AIAvatar];
    mCardZones[kCardZone_PlayerHand]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_PlayerAvatar];
    mCardZones[kCardZone_PlayerHand]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_PlayerCreatures];
    mCardZones[kCardZone_PlayerHand]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_AIAvatar];

    mCardZones[kCardZone_PlayerAvatar]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_PlayerHand];
    mCardZones[kCardZone_PlayerAvatar]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_PlayerLibrary];
    mCardZones[kCardZone_PlayerAvatar]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_PlayerEnchantmentsAndArtifacts];
    mCardZones[kCardZone_PlayerAvatar]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_PlayerLands];

    mCardZones[kCardZone_PlayerLibrary]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_PlayerGraveyard];
    mCardZones[kCardZone_PlayerLibrary]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_PlayerAvatar];
    mCardZones[kCardZone_PlayerLibrary]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_PlayerLands];
    mCardZones[kCardZone_PlayerLibrary]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_PlayerAvatar];

    mCardZones[kCardZone_PlayerGraveyard]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_PlayerHand];
    mCardZones[kCardZone_PlayerGraveyard]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_PlayerLibrary];
    mCardZones[kCardZone_PlayerGraveyard]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_PlayerCreatures];
    mCardZones[kCardZone_PlayerGraveyard]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_PlayerAvatar];

    mCardZones[kCardZone_PlayerLands]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_PlayerCreatures];
    mCardZones[kCardZone_PlayerLands]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_PlayerAvatar];
    mCardZones[kCardZone_PlayerLands]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_PlayerEnchantmentsAndArtifacts];
    mCardZones[kCardZone_PlayerLands]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_PlayerAvatar];

    mCardZones[kCardZone_PlayerCreatures]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_AICreatures];
    mCardZones[kCardZone_PlayerCreatures]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_PlayerLands];
    mCardZones[kCardZone_PlayerCreatures]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_PlayerEnchantmentsAndArtifacts];
    mCardZones[kCardZone_PlayerCreatures]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_PlayerHand];

    mCardZones[kCardZone_PlayerEnchantmentsAndArtifacts]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_PlayerCreatures];
    mCardZones[kCardZone_PlayerEnchantmentsAndArtifacts]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_PlayerLands];
    // experiment, allow round tripping from the left edge over to the right side
    mCardZones[kCardZone_PlayerEnchantmentsAndArtifacts]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_PlayerAvatar];
    mCardZones[kCardZone_PlayerEnchantmentsAndArtifacts]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_PlayerCreatures];

    mCardZones[kCardZone_AIHand]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_AIAvatar];
    mCardZones[kCardZone_AIHand]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_AIEnchantmentsAndArtifacts];
    mCardZones[kCardZone_AIHand]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_AILibrary];

    mCardZones[kCardZone_AIAvatar]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_AILands];
    mCardZones[kCardZone_AIAvatar]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_AIHand];
    mCardZones[kCardZone_AIAvatar]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_AILibrary];
    mCardZones[kCardZone_AIAvatar]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_PlayerHand];

    mCardZones[kCardZone_AILibrary]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_AIGraveyard];
    mCardZones[kCardZone_AILibrary]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_AILands];
    mCardZones[kCardZone_AILibrary]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_AIAvatar];
    mCardZones[kCardZone_AILibrary]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_AILands];

    mCardZones[kCardZone_AIGraveyard]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_AIAvatar];
    mCardZones[kCardZone_AIGraveyard]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_AILibrary];
    mCardZones[kCardZone_AIGraveyard]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_AIAvatar];
    mCardZones[kCardZone_AIGraveyard]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_AILands];

    mCardZones[kCardZone_AILands]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_AIAvatar];
    mCardZones[kCardZone_AILands]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_AICreatures];
    mCardZones[kCardZone_AILands]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_AIEnchantmentsAndArtifacts];
    mCardZones[kCardZone_AILands]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_PlayerHand];

    mCardZones[kCardZone_AICreatures]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_AILands];
    mCardZones[kCardZone_AICreatures]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_PlayerCreatures];
    mCardZones[kCardZone_AICreatures]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_AIEnchantmentsAndArtifacts];
    mCardZones[kCardZone_AICreatures]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_PlayerHand];

    mCardZones[kCardZone_AIEnchantmentsAndArtifacts]->mNeighbours[JGE_BTN_UP] = mCardZones[kCardZone_AIAvatar];
    mCardZones[kCardZone_AIEnchantmentsAndArtifacts]->mNeighbours[JGE_BTN_DOWN] = mCardZones[kCardZone_AICreatures];
    // experiment, allow round tripping from the left edge over to the right side
    mCardZones[kCardZone_AIEnchantmentsAndArtifacts]->mNeighbours[JGE_BTN_LEFT] = mCardZones[kCardZone_PlayerHand];
    mCardZones[kCardZone_AIEnchantmentsAndArtifacts]->mNeighbours[JGE_BTN_RIGHT] = mCardZones[kCardZone_AICreatures];

    mCurrentZone = mCardZones[kCardZone_PlayerAvatar];
}

/*
 **
 */
Navigator::~Navigator()
{
    std::map<int, CardZone*>::iterator iter = mCardZones.begin();
    for (; iter != mCardZones.end(); ++iter)
    {
        SAFE_DELETE(iter->second);
    }
}

/*
 **
 */
bool Navigator::CheckUserInput(JButton inKey)
{
    bool result = true;

    switch (inKey)
    {
    case JGE_BTN_SEC:
        observer->cancelCurrentAction();
        return true;
    case JGE_BTN_OK:
        observer->ButtonPressed(GetCurrentCard());
        return true;
        break;
    case JGE_BTN_LEFT:
    case JGE_BTN_RIGHT:
    case JGE_BTN_UP:
    case JGE_BTN_DOWN:
        HandleKeyStroke(inKey);
        break;
    case JGE_BTN_CANCEL:
        mDrawMode = (mDrawMode + 1) % DrawMode::kNumDrawModes;
        if (mDrawMode == DrawMode::kText)
            options[Options::DISABLECARDS].number = 1;
        else
            options[Options::DISABLECARDS].number = 0;
        break;
    default:
        result = false;
    }

    return result;
}

bool Navigator::CheckUserInput(int x, int y)
{
    // TODO - figure out what to do with mouse support
    return false;
}

/*
 ** reposition the selected card's draw location
 */
void Navigator::Update(float dt)
{
    float boundary = mDuelLayers->RightBoundary();
    float position = boundary - CardGui::BigWidth / 2;
    if (GetCurrentCard() != NULL)
    {
        if ((GetCurrentCard()->x + CardGui::Width / 2 > position - CardGui::BigWidth / 2) && (GetCurrentCard()->x - CardGui::Width
                        / 2 < position + CardGui::BigWidth / 2))
        {
            position = CardGui::BigWidth / 2 - 10;
        }
    }

    if (position < CardGui::BigWidth / 2)
        position = CardGui::BigWidth / 2;

    mDrawPosition.x = position;
    mDrawPosition.Update(dt);
}

/*
 **
 */
PlayGuiObject* Navigator::GetCurrentCard()
{
    return mCurrentZone ? mCurrentZone->GetCurrentCard() : NULL;
}

/*
 **
 */
void Navigator::Render()
{
    if (GetCurrentCard() != NULL)
    {
        GetCurrentCard()->Render();

        CardView* card = dynamic_cast<CardView*> (GetCurrentCard());
        if (card)
        {
            card->DrawCard(mDrawPosition, mDrawMode);
        }
    }
}

/*
 **
 */
void Navigator::HandleKeyStroke(JButton inKey)
{
    assert(mCurrentZone);
    if (mCurrentZone)
    {
        bool changeZone = mCurrentZone->HandleSelection(inKey);

        if (changeZone && !mLimitorEnabled)
        {
            mCurrentZone->LeaveZone(inKey);
            mCurrentZone = mCurrentZone->GetNeighbour(inKey);
            mCurrentZone = mCurrentZone->EnterZone(inKey);
        }
    }
}

/*
 ** unused. This is CardSelector specific.
 */
void Navigator::PopLimitor()
{
}

/*
 ** same as above.
 */
void Navigator::PushLimitor()
{
}

/*
 **
 */
void Navigator::Limit(LimitorFunctor<PlayGuiObject>* inLimitor, CardView::SelectorZone inZone)
{
    mLimitorEnabled = (inLimitor != NULL);
    if (inZone == CardView::handZone)
    {
        mCurrentZone->LeaveZone(JGE_BTN_NONE);

        if (mLimitorEnabled)
        {
            mCurrentZoneStack.push(mCurrentZone);
            mCurrentZone = mCardZones[kCardZone_PlayerHand];
        }
        else
        {
            mCurrentZone = mCurrentZoneStack.top();
            mCurrentZoneStack.pop();
            assert(mCurrentZone);
            if (mCurrentZone == NULL)
            {
                mCurrentZone = mCardZones[kCardZone_PlayerHand];
            }
        }

        mCurrentZone->EnterZone(JGE_BTN_NONE);
    }
}

/*
 **
 */
int Navigator::CardToCardZone(PlayGuiObject* inCard)
{
    int result = kCardZone_Unknown;
    GuiAvatar* avatar = dynamic_cast<GuiAvatar*> (inCard);
    if (avatar)
    {
        if (avatar->player->isAI())
        {
            result = kCardZone_AIAvatar;
        }
        else
        {
            result = kCardZone_PlayerAvatar;
        }
    }

    GuiGraveyard* graveyard = dynamic_cast<GuiGraveyard*> (inCard);
    if (graveyard)
    {
        if (graveyard->player->isAI())
        {
            result = kCardZone_AIGraveyard;
        }
        else
        {
            result = kCardZone_PlayerGraveyard;
        }
    }

    GuiLibrary* library = dynamic_cast<GuiLibrary*> (inCard);
    if (library)
    {
        if (library->player->isAI())
        {
            result = kCardZone_AILibrary;
        }
        else
        {
            result = kCardZone_PlayerLibrary;
        }
    }

    GuiOpponentHand* opponentHand = dynamic_cast<GuiOpponentHand*> (inCard);
    if (opponentHand)
    {
        result = kCardZone_AIHand;
    }

    CardView* card = dynamic_cast<CardView*> (inCard);
    {
        if (card)
        {
            if (card->owner == CardView::handZone)
            {
                result = kCardZone_PlayerHand;
            }
            else if (card->owner == CardView::playZone)
            {
                int isAI = card->getCard()->owner->isAI();

                if (card->getCard()->isCreature())
                {
                    result = isAI ? kCardZone_AICreatures : kCardZone_PlayerCreatures;
                }
                else if (card->getCard()->isLand())
                {
                    result = isAI ? kCardZone_AILands : kCardZone_PlayerLands;
                }
                else if (card->getCard()->isSpell())
                {

                    if (card->getCard()->target != NULL)
                        isAI = card->getCard()->target->owner->isAI();

                    // nasty hack:  the lines above don't always work, as when an enchantment comes into play, its ability hasn't been activated yet,
                    // so it doesn't yet have a target.  Instead, we now look at the card's position, if it's in the top half of the screen, it goes into an AI zone
                    //isAI = card->y < JRenderer::GetInstance()->GetActualHeight() / 2;

                    // enchantments that target creatures are treated as part of the creature zone
                    if (card->getCard()->spellTargetType.find("creature") != string::npos)
                    {
                        result = isAI ? kCardZone_AICreatures : kCardZone_PlayerCreatures;
                    }
                    else if (card->getCard()->spellTargetType.find("land") != string::npos)
                    {
                        result = isAI ? kCardZone_AILands : kCardZone_PlayerLands;
                    }
                    else
                    {
                        result = isAI ? kCardZone_AIEnchantmentsAndArtifacts : kCardZone_PlayerEnchantmentsAndArtifacts;
                    }
                }
                else
                    assert(false);
            }
            else
            {
                assert(false);
            }
        }
    }

    assert(result != kCardZone_Unknown);
    return result;
}

/*
 **
 */
void Navigator::Add(PlayGuiObject* card)
{
    // figure out what card's been added, add it to the appropriate pile
    int zone = CardToCardZone(card);
    if (zone != kCardZone_Unknown)
    {
        mCardZones[zone]->AddCard(card);
    }
}

/*
 **
 */
void Navigator::Remove(PlayGuiObject* card)
{
    int zone = CardToCardZone(card);
    if (zone != kCardZone_Unknown)
    {
        mCardZones[zone]->RemoveCard(card);
    }
}
