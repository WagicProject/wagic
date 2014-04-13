#ifndef _GRID_DECK_VIEW_H
#define _GRID_DECK_VIEW_H

#include "DeckView.h"
#include "Easing.h"

/*! \brief Implements a grid view
 *
 * This view displays 12 cards in two rows as thumbnails. The currently
 * selected card is dislayed bigger than the rest and uses the fullsize
 * image. Scrolling the view horizontally and toggeling filters is
 * animated and uses quadratic easing.
 *
 * It also implements a button mode for pointerless devices.
 */
class GridDeckView : public DeckView
{
private:
    static const float scroll_animation_duration;
    static const float slide_animation_duration;
    static const float card_scale_small;
    static const float card_scale_big;
public:
    /*! \brief Constructs a grid view with no decks set
     */
    GridDeckView();

    /*! \brief Does nothing but is needed to ensure proper deletion of derived classes.
     */
    virtual ~GridDeckView();

    /*! \brief Resets almost all member variables but mRows and mCols
     */
    void Reset();

    /*! \brief Advances scrolling and sliding animations
     *
     * \param dt the time since the last update
     *
     * \see DeckView::UpdateViewState()
     */
    void UpdateViewState(float dt);

    /*! \brief Updates the cards position
     *
     * \see DeckView::UpdateCardPosition()
     */
    void UpdateCardPosition(int index);

    /*! \brief Renders the view
     *
     * This method prefetches all rendered cards as thumbnails except the
     * selected card to reduce cache pressure.
     */
    void Render();

    /*! \brief Handles button presses
     *
     * The mapping is as follows:
     *  JGE_BTN_LEFT        moves the position to the left if not in button mode
     *                      moves the selection otherwise
     *  JGE_BTN_RIGHT       move the position to the right if not in button mode
     *                      moves the selection otherwise
     *  JGE_BTN_UP          select the previous filter if not in button mode
     *                      moves the selection otherwise
     *  JGE_BTN_DOWN        select the next filter if not in button mode
     *                      moves the selection otherwise
     *  JGE_BTN_CTRL        deactivate button mode
     *
     * \param button the pressed button
     * \returns if the view handled the button
     */
    bool ButtonPressed(Buttons button);

    /*! \brief Handles clicks and triggers scrolling and the selection of cards
     *
     * This method deactivates the button mode and searches for the nearest
     * card to the given position. If this card is in column 0 or 1 it scrolls
     * left. If it is in column (mCols-1) or (mCols-2) it scrolls to the right.
     * In any other case, it selects the card.
     *
     * \param x the clicks x coordinate
     * \param y the clicks y coordinate
     *
     * \return selected card c if c was already selected and no animation is running, NULL otherwise
     */
    MTGCard * Click(int x, int y);

    /*! \brief Handles pointerless clicks (JGE_BTN_OK)
     *
     * If no card is selected, this method activates button mode and selects a card.
     *
     * \returns selected card, NULL otherwise
     */
    MTGCard * Click();

    /*! \brief Scrolls the view horizontally
     *
     * \param offset the number of columns to scroll
     */
    void changePositionAnimated(int offset);

    /*! \brief Rotates the selected filter and slides vertically
     *
     * \param the number of filters to rotate
     */
    void changeFilterAnimated(int offset);

    /*! \brief Returns the currently selected card
     *
     * \returns card c if c is selected and in column 4 to 6 and NULL otherwise*/
    MTGCard *getActiveCard();
private:
    /*! \brief The amount of columns (visible and hidden)
     */
    const int mCols;

    /*! \brief The amount of rows
     */
    const int mRows;

    /*! \brief The current scrolling offset
     */
    float mScrollOffset;

    /*! \brief The current sliding offset
     */
    float mSlideOffset;

    /*! \brief The easing functor that gets applied while scrolling
     */
    InOutQuadEasing mScrollEasing;

    /*! \brief The easing functor that gets applied while sliding
     */
    InOutQuadEasing mSlideEasing;

    /*! \brief The current selected card index
     */
    int mCurrentSelection;

    /*! \brief Stores if we are in button mode.
     */
    bool mButtonMode;

    /*! \brief Moves the card selection by an offset.
     *
     * \param offset the offset to move the selection
     * \param alignIfOutOfBounds the view will scroll if the selection moves out of bound if set to true
     */
    void moveSelection(int offset, bool alignIfOutOfBounds);
};

#endif //_GRID_DECK_VIEW_H
