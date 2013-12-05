#ifndef _GRID_DECK_VIEW_H
#define _GRID_DECK_VIEW_H

#include "DeckView.h"

class Easing
{
public:
    float start_value;
    float delta_value;
    float value;
    float duration;
    float time_acc;


    Easing(float val): start_value(val), delta_value(0), value(val), duration(0), time_acc(0)
    {}

    void reset(){ value = start_value; time_acc = 0;}
    void finish(){ value = start_value + delta_value; time_acc = 0; duration = 0;}
    virtual void update(float dt) = 0;

    void start(float targetValue, float _duration){
        start_value = value;
        delta_value = targetValue - start_value;
        time_acc = 0;
        duration = _duration;
    }

    void transpose(float delta_value){
        start_value += delta_value;
        value += delta_value;
    }

    bool finished()
    {
        return time_acc >= duration;
    }
};

class InOutQuadEasing : public Easing
{
public:
    InOutQuadEasing(float val): Easing(val) {}

    void update(float dt){
        if(duration > 0){
            time_acc += dt;

            if(time_acc > duration)
            {
                time_acc = duration;
                value = start_value + delta_value;
            }
            else
            {
                float time_tmp = time_acc * 2 / duration;
                if (time_tmp < 1)
                {
                    value = delta_value/2*time_tmp*time_tmp + start_value;
                }
                else
                {
                    time_tmp -= 1;
                    value = -delta_value/2 * (time_tmp*(time_tmp-2) - 1) + start_value;
                }
            }
        }
    }
};

class GridDeckView : public DeckView
{
private:
    enum AnimationStage{
        NONE = 0,
        SLIDE_UP,
        SLIDE_DOWN//,
        //SCROLL_TO_SELECTED
    };

    static const float scroll_speed;
    static const float card_scale_small;
    static const float card_scale_big;
public:
    GridDeckView();
    virtual ~GridDeckView();
    void Reset();

    void UpdateViewState(float dt);
    void UpdateCardPosition(CardRep &rep, int index);

    void Render();
    MTGCard * Click(int x, int y);
    bool Button(Buttons button);
    MTGCard *getActiveCard();
private:
    int   mCols;
    int   mRows;
    float mSlide;            //[-1,1]. defines, the y-offset of the cards
    InOutQuadEasing mScrollOffset;
    int   mCurrentSelection; //0 <= mCurrentSelection < mCards.size(). defines the current selected and thus upscaled card
    //int   mColsToScroll;  //the number of cols we need to scroll
    AnimationStage mStage; // state machine state. for animation purposes
};

#endif //_GRID_DECK_VIEW_H
