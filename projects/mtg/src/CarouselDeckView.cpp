#include "CarouselDeckView.h"

const float CarouselDeckView::slide_animation_duration = 0.6f;

CarouselDeckView::CarouselDeckView() :
    DeckView(10), mScrollOffset(0), mSlide(0)
{
}

CarouselDeckView::~CarouselDeckView()
{}

void CarouselDeckView::UpdateViewState(float dt)
{
    if(!mScrollOffset.finished())
    {
        mScrollOffset.update(dt);

        if(mScrollOffset.value <= -1.0f)
        {
            mScrollOffset.translate(1.0f);
            deck()->prev();
            reloadIndexes();
        }
        else if(mScrollOffset.value >= 1.0f)
        {
            mScrollOffset.translate(-1.0f);
            deck()->next();
            reloadIndexes();
        }

        dirtyCardPos = true;
    }

    if(!mSlide.finished())
    {
        mSlide.update(dt);

        if(mSlide.value < mSlide.start_value)
        {
            //going downwards
            if(mSlide.value < -1.0f)
            {
                mSlide.translate(2.0f);
                SwitchFilter(1);
            }
        }
        else if(mSlide.value > mSlide.start_value)
        {
            //upwards
            if(mSlide.value > 1.0f)
            {
                mSlide.translate(-2.0f);
                SwitchFilter(-1);
            }
        }

        dirtyCardPos = true;
    }
}

void CarouselDeckView::UpdateCardPosition(CardRep &rep, int index)
{
    float rotation = mScrollOffset.value + 8 - index;

    rep.x = x_center + cos((rotation) * M_PI / 12) * (right_border - x_center);
    rep.scale = max_scale / 1.12f * cos((rep.x - x_center) * 1.5f / (right_border - x_center)) + 0.2f * max_scale * cos(
                cos((rep.x - x_center) * 0.15f / (right_border - x_center)));
    rep.y = (SCREEN_HEIGHT_F) / 2.0f + SCREEN_HEIGHT_F * mSlide.value * (rep.scale + 0.2f);
}

void CarouselDeckView::Reset()
{
    mScrollOffset = 0;
    mSlide = 0;
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

    if (mScrollOffset.value < 0.5 && mScrollOffset.value > -0.5)
    {
        renderCard(1);
        renderCard(3);
        renderCard(2);
    }
    else if (mScrollOffset.value < -0.5)
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
    if(mSlide.finished() && mScrollOffset.finished())
    {
        if(n == 2)
        {
            return getActiveCard();
        }
        else
        {
            DebugTrace(">>>>> " << n);
            changePosition(n - 2);
        }
    }

    return NULL;
}

void CarouselDeckView::changePosition(int offset)
{
    mScrollOffset.start(offset, 0.3f*abs(offset));

    last_user_activity = 0;
}

void CarouselDeckView::changeFilter(int offset)
{
    if(offset < 0)
    {
        mSlide.start(-2.0f, slide_animation_duration);
    }
    else if(offset > 0)
    {
        mSlide.start(2.0f, slide_animation_duration);
    }
    last_user_activity = 0;
}

MTGCard *CarouselDeckView::getActiveCard()
{
    return getCardRep(2).card;
}

