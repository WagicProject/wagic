#ifndef _CAROUSEL_DECK_VIEW_H_
#define _CAROUSEL_DECK_VIEW_H_

#include "DeckView.h"
#include "Easing.h"

class CarouselDeckView : public DeckView
{
private:
    enum AnimationStage{
        NONE = 0,
        SLIDE_UP,
        SLIDE_DOWN
    };

    static const float scroll_speed;

public:
    CarouselDeckView();
    virtual ~CarouselDeckView();
    void Reset();

    void UpdateViewState(float dt);
    void UpdateCardPosition(CardRep &rep, int index);
    void renderCard(int index)
    {
        int alpha = (int) (255 * (getCardRep(index).scale + 1.0 - max_scale));
        DeckView::renderCard(index, alpha);
    }

    void Render();

    MTGCard * Click(int x, int y);

    void changePosition(int offset);
    void changeFilter(int offset);

    MTGCard *getActiveCard();

    //maintains the current rotation for fluid animations
private:
    InOutQuadEasing mScrollOffset;   //[-1,1]. defines the current rotation of the cards
    float mSlide;      //[-1,1]. defines, the y-offset of the cards
    int mScrollTarget; //0 <= mScrollTarget < mCards.size(). defines where to scroll to if the current animation is a scroll animation
    AnimationStage mStage; // state machine state. for animation purposes
};

#endif //_CAROUSEL_DECK_VIEW_H_
