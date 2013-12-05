#include "CarouselDeckView.h"

const float CarouselDeckView::scroll_speed = 5.0f;

CarouselDeckView::CarouselDeckView() :
    DeckView(10), mRotation(0), mSlide(0),  mScrollTarget(2), mStage(NONE)
{
}

CarouselDeckView::~CarouselDeckView()
{}

void CarouselDeckView::UpdateViewState(float dt)
{
    switch(mStage)
    {
    case SCROLL_TO_SELECTED:
        if(mScrollTarget < 2)
        { //scroll left
            mRotation -= dt * scroll_speed;
            if(mRotation <= -1.0f)
            {
                mRotation += 1.0f;
                deck()->prev();
                reloadIndexes();
                mScrollTarget += 1;
            }
        }
        else if(mScrollTarget > 2)
        {//scroll right
            mRotation += dt * scroll_speed;
            if(mRotation >= 1.0f)
            {
                mRotation -= 1.0f;
                deck()->next();
                reloadIndexes();
                mScrollTarget -= 1;
            }
        }
        else if(mScrollTarget == 2)
        {
            mRotation = 0;
            mStage = NONE;
        }
        dirtyCardPos = true;
        break;
    case SLIDE_DOWN:
        mSlide -= 0.05f;
        if (mSlide < -1.0f)
        {
            dirtyFilters = true;
            mSlide = 1;
        }
        else if (mSlide > 0 && mSlide < 0.05)
        {
            mStage = NONE;
            mSlide = 0;
        }
        dirtyCardPos = true;
        break;
    case SLIDE_UP:
        mSlide += 0.05f;
        if (mSlide > 1.0f)
        {
            dirtyFilters = true;
            mSlide = -1;
        }
        else if (mSlide < 0 && mSlide > -0.05)
        {
            mStage = NONE;
            mSlide = 0;
        }
        dirtyCardPos = true;
        break;
    default:
        mStage = NONE;
        break;
    }
}

void CarouselDeckView::UpdateCardPosition(CardRep &rep, int index)
{
    float rotation = mRotation + 8 - index;

    rep.x = x_center + cos((rotation) * M_PI / 12) * (right_border - x_center);
    rep.scale = max_scale / 1.12f * cos((rep.x - x_center) * 1.5f / (right_border - x_center)) + 0.2f * max_scale * cos(
                cos((rep.x - x_center) * 0.15f / (right_border - x_center)));
    rep.y = (SCREEN_HEIGHT_F) / 2.0f + SCREEN_HEIGHT_F * mSlide * (rep.scale + 0.2f);
}

void CarouselDeckView::Reset()
{
    mRotation = 0;
    mSlide = 0;
    mScrollTarget = 2;
    mStage = NONE;
    DeckView::Reset();
}

void CarouselDeckView::Render()
{
    // even though we want to draw the cards in a particular z order for layering, we want to prefetch them
    // in a different order, ie the center card should appear first, then the adjacent ones
    if (WResourceManager::Instance()->IsThreaded())
    {
        WResourceManager::Instance()->RetrieveCard(getCardRep(0).card);
        WResourceManager::Instance()->RetrieveCard(getCardRep(3).card);
        WResourceManager::Instance()->RetrieveCard(getCardRep(4).card);
        WResourceManager::Instance()->RetrieveCard(getCardRep(2).card);
        WResourceManager::Instance()->RetrieveCard(getCardRep(5).card);
        WResourceManager::Instance()->RetrieveCard(getCardRep(1).card);
        WResourceManager::Instance()->RetrieveCard(getCardRep(6).card);
    }

    renderCard(6);
    renderCard(5);
    renderCard(4);
    renderCard(0);

    if (mRotation < 0.5 && mRotation > -0.5)
    {
        renderCard(1);
        renderCard(3);
        renderCard(2);
    }
    else if (mRotation < -0.5)
    {
        renderCard(3);
        renderCard(2);
        renderCard(1);
    }
    else
    {
        renderCard(1);
        renderCard(2);
        renderCard(3);
    }
}

MTGCard * CarouselDeckView::Click(int x, int y)
{
    int n = getCardIndexNextTo(x, y);
    last_user_activity = 0;

    //clicked active card, and no animation is running
    if(n == 2 && mStage == NONE)
    {
        return getActiveCard();
    }

    //clicked not the active card, start animation:s
    if(n != 2 && mStage == NONE)
    {
        mScrollTarget = n;
        mStage = SCROLL_TO_SELECTED;
    }

    return NULL;
}

void CarouselDeckView::changePosition(int offset)
{
    if(offset > 0)
    {
        mScrollTarget += 1;
        mStage = SCROLL_TO_SELECTED;
    }
    else if(offset < 0)
    {
        mScrollTarget -= 1;
        mStage = SCROLL_TO_SELECTED;
    }

    last_user_activity = 0;
}

void CarouselDeckView::changeFilter(int offset)
{
    if(offset > 0)
    {
        mStage = SLIDE_UP;
        SwitchFilter(1);
    }
    else if(offset < 0)
    {
        mStage = SLIDE_DOWN;
        SwitchFilter(-1);
    }

    last_user_activity = 0;
}

MTGCard *CarouselDeckView::getActiveCard()
{
    return getCardRep(2).card;
}

