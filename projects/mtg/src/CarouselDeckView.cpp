#include "PrecompiledHeader.h"
#include "CarouselDeckView.h"

const float CarouselDeckView::max_scale = 0.82f;
const float CarouselDeckView::x_center = 180;
const float CarouselDeckView::right_border = SCREEN_WIDTH + 180;
const float CarouselDeckView::slide_animation_duration = 0.5f;
const float CarouselDeckView::scroll_animation_duration = 0.2f;

CarouselDeckView::CarouselDeckView() :
    DeckView(10), mScrollOffset(0), mSlideOffset(0), mScrollEasing(mScrollOffset), mSlideEasing(mSlideOffset)
{
}

CarouselDeckView::~CarouselDeckView()
{}

void CarouselDeckView::UpdateViewState(float dt)
{
    if(!mScrollEasing.finished())
    {
        mScrollEasing.update(dt);

        if(mScrollOffset <= -1.0f)
        {
            changePosition(-1);
            mScrollEasing.translate(1.0f);
        }
        else if(mScrollOffset >= 1.0f)
        {
            changePosition(1);
            mScrollEasing.translate(-1.0f);
        }

        dirtyCardPos = true;
    }

    if(!mSlideEasing.finished())
    {
        mSlideEasing.update(dt);

        if(mSlideOffset < mSlideEasing.start_value)
        {
            //going downwards
            if(mSlideOffset < -1.0f)
            {
                mSlideEasing.translate(2.0f);
                changeFilter(1);
            }
        }
        else if(mSlideOffset > mSlideEasing.start_value)
        {
            //upwards
            if(mSlideOffset > 1.0f)
            {
                mSlideEasing.translate(-2.0f);
                changeFilter(-1);
            }
        }

        dirtyCardPos = true;
    }
}

void CarouselDeckView::UpdateCardPosition(int index)
{
    CardRep &rep = mCards[index];

    float rotation = mScrollOffset + 8 - index;

    rep.x = x_center + cos((rotation) * M_PI / 12) * (right_border - x_center);
    rep.scale = max_scale / 1.12f * cos((rep.x - x_center) * 1.5f / (right_border - x_center)) + 0.2f * max_scale * cos(
                cos((rep.x - x_center) * 0.15f / (right_border - x_center)));
    rep.y = (SCREEN_HEIGHT_F) / 2.1f + SCREEN_HEIGHT_F * mSlideOffset * (rep.scale + 0.2f);
}

void CarouselDeckView::Reset()
{
    mSlideEasing.finish();
    mScrollEasing.finish();

    DeckView::Reset();
}

void CarouselDeckView::Render()
{
    // even though we want to draw the cards in a particular z order for layering, we want to prefetch them
    // in a different order, ie the center card should appear first, then the adjacent ones
    bool prefetch = options[Options::CARDPREFETCHING].number?true:false;
    if (prefetch && WResourceManager::Instance()->IsThreaded())
    {
        WResourceManager::Instance()->RetrieveCard(mCards[0].card);
        WResourceManager::Instance()->RetrieveCard(mCards[3].card);
        WResourceManager::Instance()->RetrieveCard(mCards[4].card);
        WResourceManager::Instance()->RetrieveCard(mCards[2].card);
        WResourceManager::Instance()->RetrieveCard(mCards[5].card);
        WResourceManager::Instance()->RetrieveCard(mCards[1].card);
        WResourceManager::Instance()->RetrieveCard(mCards[6].card);
    }

    renderCard(6);
    renderCard(5);
    renderCard(4);
    renderCard(0);

    if (mScrollOffset < 0.5 && mScrollOffset > -0.5)
    {
        renderCard(1);
        renderCard(3);
        renderCard(2);
    }
    else if (mScrollOffset < -0.5)
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

bool CarouselDeckView::ButtonPressed(Buttons button)
{
    switch(button)
    {
    case JGE_BTN_LEFT:
        changePositionAnimated(-1);
        last_user_activity = 0;
        break;
    case JGE_BTN_RIGHT:
        changePositionAnimated(1);
        last_user_activity = 0;
        break;
    case JGE_BTN_UP:
        changeFilterAnimated(1);
        last_user_activity = 0;
        break;
    case JGE_BTN_DOWN:
        changeFilterAnimated(-1);
        last_user_activity = 0;
        break;
    default:
        return false;
    }
    return true;
}
MTGCard * CarouselDeckView::Click(int x, int y)
{
    int n = getCardIndexNextTo(x, y);
    last_user_activity = 0;

    //clicked active card, and no animation is running
    if(mSlideEasing.finished() && mScrollEasing.finished())
    {
        if(n == 2)
        {
            return getActiveCard();
        }
        else
        {
            changePositionAnimated(n - 2);
        }
    }

    return NULL;
}

MTGCard *CarouselDeckView::Click()
{
    if(mSlideEasing.finished() && mScrollEasing.finished())
    {
        return getActiveCard();
    }
    else
    {
        return NULL;
    }
}

void CarouselDeckView::changePositionAnimated(int offset)
{
    if(mScrollEasing.finished())
        mScrollEasing.start((float)offset, (float)(scroll_animation_duration * abs(offset)));
    last_user_activity = 0;
}

void CarouselDeckView::changeFilterAnimated(int offset)
{
    if(mSlideEasing.finished())
        mSlideEasing.start(2.0f * float(offset), float(slide_animation_duration * abs(offset)));
    last_user_activity = 0;
}

MTGCard *CarouselDeckView::getActiveCard()
{
    return mCards[2].card;
}

