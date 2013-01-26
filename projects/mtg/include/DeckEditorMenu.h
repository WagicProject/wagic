#pragma once
#include "DeckMenu.h"
#include "DeckDataWrapper.h"
#include "DeckStats.h" 

class DeckEditorMenu: public DeckMenu
{
protected:
    string deckTitle;

private:
    void drawDeckStatistics();

    DeckDataWrapper *selectedDeck;
    StatsWrapper *stw;

public:
    DeckEditorMenu(int id, JGuiListener* listener = NULL, int fontId = 1, const char * _title = "", DeckDataWrapper *selectedDeck = NULL, StatsWrapper *stats = NULL);
    void Render();
    virtual ~DeckEditorMenu();
};
