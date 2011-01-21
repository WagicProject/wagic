#ifndef _CARD_DISPLAY_H_
#define _CARD_DISPLAY_H_

#include "PlayGuiObjectController.h"

class TargetChooser;
class MTGGameZone;
class MTGCardInstance;

class CardDisplay: public PlayGuiObjectController
{
    int mId;
    GameObserver* game;
public:
    int x, y, start_item, nb_displayed_items;
    MTGGameZone * zone;
    TargetChooser * tc;
    JGuiListener * listener;
    CardDisplay();
    CardDisplay(int id, GameObserver* game, int x, int y, JGuiListener * listener = NULL, TargetChooser * tc = NULL,
            int nb_displayed_items = 7);
    void AddCard(MTGCardInstance * _card);
    void rotateLeft();
    void rotateRight();
    bool CheckUserInput(JButton key);
    bool CheckUserInput(int x, int y);
    virtual void Update(float dt);
    void Render();
    void init(MTGGameZone * zone);
    virtual ostream& toString(ostream& out) const;
};

std::ostream& operator<<(std::ostream& out, const CardDisplay& m);

#endif
