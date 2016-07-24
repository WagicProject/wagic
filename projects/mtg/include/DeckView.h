#ifndef _DECK_VIEW_H_
#define _DECK_VIEW_H_

#include <vector>

#include "MTGCard.h"
#include "DeckDataWrapper.h"
#include "WFont.h"
#include "WResourceManager.h"
#include "Pos.h"

/*! \brief A abstract base class for deck views
 *
 * The deck editor uses a deck view to present the cards
 * e.g. in a circular "Carousel" layout or in a flat grid
 * layout. Both layouts inherit this base class to ensure
 * a common interface which the deck editor can rely on.
 */
class DeckView
{
protected:
    /*! \brief defines the delay until additional card informations get shown
     *
     * \note I am not entirely sure about that
     */
    static const float no_user_activity_show_card_delay;

    /*! \brief Represents a card for internal use in the deck view
     *
     * It stores positional information and a pointer to the actual card structure.
     */
    struct CardRep{
        float x;
        float y;
        float scale;
        MTGCard * card;
    };
public:
    /*! \brief Defines if the filter needs an update
     *
     * The owner of the deck that is shown is responsible for updating the filters.
     */
    bool dirtyFilters;

    /*! \brief Defines if the card positions need an update
     *
     * If the card positions are dirty, UpdateCardPosition will get called on
     * all cards during Update(float dt);
     *
     * \see Update
     * \see UpdateCardPosition
     */
    bool dirtyCardPos;

    /*! \brief Constructs the view and initializes datamembers
     *
     * It sets the dirty states to true, the currently shown deck to NULL and selects filter 0.
     *
     * \param numberOfCards the number of cards the view handles (this includes hidden cards for caching)
     */
    DeckView(int numberOfCards);

    /*! \brief Does nothing but is needed to ensure proper deletion of derived classes.
     */
    virtual ~DeckView();

    /*! \brief Resets nearly all datamembers to their initial values
     *
     * Does not reset mCards.
     */
    virtual void Reset();

    /*! \brief Advances the view by dt time units
     *
     * This method calls UpdateViewState unconditionally and UpdateCardPosition on every card
     * if dirtyCardPos is set. It then resets dirtyCardPos.
     *
     * \param dt the number of time units to advance
     * \see UpdateViewState
     * \see UpdateCardPosition
     */
    void Update(float dt);

    /*! \brief Sets the deck that this view shows
     *
     * This method replaces the currently shown deck with toShow, sets all dirty states and
     * reloads the mtg cards. No ownership changes.
     *
     * \param toShow the deck to show
     * \see reloadIndexes
     */
    void SetDeck(DeckDataWrapper *toShow);

    /*! \brief Returns a pointer to the current deck.
     */
    DeckDataWrapper *deck();

    /*! \brief Performs an immediate switch of the filter without animations
     *
     * This method rotates the currently selected filter by delta and sets dirtyFilters.
     *
     * \param delta the filter to select relatively to the currently selected filter
     * \see dirtyFilters
     */
    void changeFilter(int delta);

    /*! \brief Performs an immediate switch of the position without animations
     *
     * If the i-th card stored in mCards points to the j-th card in the deck, it will point
     * to the (j+delta)-th card after this method is called. No dirty states are set.
     *
     * \param delta the number of cards to advances
     * \see mCards
     */
    void changePosition(int delta);

    /*! \brief Returns the number of the currently selected filter
     *
     * \return the currently selected filter
     */
    int filter();

    /*! \brief Reloads the mtg card pointers of mCards from the deck
     *
     * This is called when: We change the position in the deck or the deck structure changes
     * (due to filtering or addition or removal of cards).
     */
    void reloadIndexes();

    /*! \brief Returns the current position in the deck
     */
    int getPosition();

    /*! \brief Renders the view
     */
    virtual void Render() = 0;

    /*! \brief Reacts to selections by a pointer device (e. g. mouse, touch)
     *
     * If the selection in view internal i. e. a card got selected, there is
     * no outside action performed and this method will return NULL. If a action got
     * triggered i. e. a selected card was activated, it returns that card
     * for further handling by the caller.
     *
     * \param x the x coordinate of the pointer during the action
     * \param y the y coordinate of the pointer during the action
     * \returns the card the action corresponds to
     */
    virtual MTGCard * Click(int x, int y) = 0;

    /*! \brief Reacts to selections by pointerless devices (e. g. buttons)
     *
     * \see Click(int x, int y)
     * \returns the card the actions corresponds to
     */
    virtual MTGCard * Click() = 0;

    /*! \brief Handles ordinary button presses
     *
     * \param the pressed JButton
     * \returns true if the view reacted to the button and false otherwise
     */
    virtual bool ButtonPressed(Buttons button) = 0;

    /*! \brief Returns the currently active card
     */
    virtual MTGCard *getActiveCard() = 0;

    /*! \brief Changes the position by a given offset
     *
     * Advances the view by offset cards and animates the change.
     *
     * \param offset the number of positions to advance
     */
    virtual void changePositionAnimated(int offset) = 0;

    /*! \brief Changes the filter by a given offset
     *
     * Rotates the selected filter by the given offset and animates the change.
     */
    virtual void changeFilterAnimated(int offset) = 0;
protected:

    /*! \brief The number of time units since an user activity occurred
     */
    float last_user_activity;

    /*! \brief The currently selected filter
     */
    int mFilter;

    /*! \brief The currently selected deck
     *
     * This class does not take ownership of the deck
     */
    DeckDataWrapper *mCurrentDeck;

    /*! \brief The card positions and pointers
     */
    vector<CardRep> mCards;

    /*! \brief Renders a card with given alpha value
     *
     * \param index of the card in mCards to render
     * \param alpha the alpha value of the card
     * \param asThumbnail renders the thumbnail image of the card if set to true
     *
     * \see mCards
     */
    void renderCard(int index, int alpha, bool asThumbnail = false, bool addWHborder = false);

    /*! \brief Returns the index in mCards of the card that is nearest to the given point
     *
     * \note This method uses the euclidian distance to the center of the card
     *
     * \param x the reference points x coordinate
     * \param y the reference points y coordinate
     * \returns the index of the nearest card to the reference point and -1 of mCards is empty
     */
    int getCardIndexNextTo(int x, int y);
private:

    /*! \brief Updates the state of the view e. g. view transitions
     *
     * \param dt the passes time since the last update
     */
    virtual void UpdateViewState(float dt) = 0;

    /*! \brief Updates the given card reps positional members
     *
     * This method is called from Update when dirtyCardPos is set
     *
     * \param index the index in mCards of the card to update
     *
     * \see Update
     * \see mCards
     */
    virtual void UpdateCardPosition(int index) = 0;
};

#endif // _DECK_VIEW_H_
