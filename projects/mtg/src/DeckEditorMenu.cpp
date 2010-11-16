#include "PrecompiledHeader.h"
#include "DeckEditorMenu.h"
#include "DeckDataWrapper.h"
#include "DeckStats.h"
#include "JTypes.h"
#include "GameApp.h"
#include <iomanip>

DeckEditorMenu::DeckEditorMenu(int id, JGuiListener* listener, int fontId, const char * _title, DeckDataWrapper *_selectedDeck,
                StatsWrapper *stats) :
    DeckMenu(id, listener, fontId, _title), selectedDeck(_selectedDeck), stw(stats)
{
    backgroundName = "DeckEditorMenuBackdrop";

    deckTitle = selectedDeck ? selectedDeck->parent->meta_name : "";

    mX = 123;
    mY = 70;
    starsOffsetX = 50;

    titleX = 110; // center point in title box
    titleY = 25;
    titleWidth = 180; // width of inner box of title

    descX = 275;
    descY = 80;
    descHeight = 154;
    descWidth = 175;

    statsHeight = 50;
    statsWidth = 185;
    statsX = 280;
    statsY = 12;

    avatarX = 222;
    avatarY = 8;

    float scrollerWidth = 80;
    SAFE_DELETE(scroller); // need to delete the scroller init in the base class
    scroller = NEW TextScroller(Fonts::MAIN_FONT, 40, 230, scrollerWidth, 100, 1, 1);

}

void DeckEditorMenu::Render()
{
    JRenderer *r = JRenderer::GetInstance();
    r->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(200,0,0,0));

    DeckMenu::Render();
    if (deckTitle.size() > 0)
    {
        WFont *mainFont = resources.GetWFont(Fonts::OPTION_FONT);
        DWORD currentColor = mainFont->GetColor();
        mainFont->SetColor(ARGB(255,255,255,255));
        mainFont->DrawString(deckTitle.c_str(), statsX + (statsWidth / 2), statsHeight / 2, JGETEXT_CENTER);
        mainFont->SetColor(currentColor);
    }

    if (stw && selectedDeck)
        drawDeckStatistics();

}

void DeckEditorMenu::drawDeckStatistics()
{
    ostringstream deckStatsString;

    deckStatsString
        << "------- Deck Summary -----" << endl
        << "Cards: "<< selectedDeck->getCount() << endl
        << "Creatures: "<< setw(2) << stw->countCreatures
        << "  Enchantments: " << stw->countEnchantments << endl
        << "Instants: " << setw(4) << stw->countInstants
        << "   Sorceries:      " << setw(2) << stw->countSorceries << endl
        << "Lands: "
        << "A: " << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_ARTIFACT ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_ARTIFACT ] << " "
        << "G: " << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_GREEN ] + stw->countLandsPerColor[ Constants::MTG_COLOR_GREEN ] << " "
        << "R: " << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_RED ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_RED ] << " "
        << "U: " << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_BLUE ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_BLUE ] << " "
        << "B: " << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_BLACK ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_BLACK ] << " "
        << "W: " << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_WHITE ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_WHITE ] << endl

        << "  --- Card color count ---  " << endl
        << "A: " << setw(2) << left  << selectedDeck->getCount(Constants::MTG_COLOR_ARTIFACT) << " "
        << "G: " << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_GREEN) << " "
        << "U: " << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_BLUE) << " "
        << "R: " << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_RED) << " "
        << "B: " << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_BLACK) << " "
        << "W: " << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_WHITE) << endl

        << " --- Average Cost --- " << endl
        << "Creature: "<< setprecision(2) << stw->avgCreatureCost << endl
        << "Mana: " << setprecision(2) << stw->avgManaCost << "   "
        << "Spell: " << setprecision(2) << stw->avgSpellCost << endl;

    WFont *mainFont = resources.GetWFont( Fonts::MAIN_FONT );
    mainFont->DrawString( deckStatsString.str().c_str(), descX, descY + 25 );
}

DeckEditorMenu::~DeckEditorMenu()
{
    SAFE_DELETE( scroller );
}
