#include "PrecompiledHeader.h"
#include "DeckEditorMenu.h"
#include "DeckDataWrapper.h"
#include "DeckStats.h"
#include "JTypes.h"
#include "GameApp.h"
#include <iomanip>
#include "Translate.h"

DeckEditorMenu::DeckEditorMenu(int id, JGuiListener* listener, int fontId, const char * _title, DeckDataWrapper *_selectedDeck, StatsWrapper *stats) :
    DeckMenu(id, listener, fontId, _title), selectedDeck(_selectedDeck), stw(stats)
{
    backgroundName = "DeckEditorMenuBackdrop";
    mShowDetailsScreen = false;
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
    SAFE_DELETE(mScroller); // need to delete the scroller init in the base class
    mScroller = NEW VerticalTextScroller(Fonts::MAIN_FONT, 40, 230, scrollerWidth, 100);

}

void DeckEditorMenu::Render()
{
    JRenderer *r = JRenderer::GetInstance();
    r->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(200,0,0,0));

    DeckMenu::Render();
    if (deckTitle.size() > 0)
    {
        WFont *mainFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
        DWORD currentColor = mainFont->GetColor();
        mainFont->SetColor(ARGB(255,255,255,255));
        mainFont->DrawString(deckTitle.c_str(), statsX + (statsWidth / 2), statsHeight / 2, JGETEXT_CENTER);
        mainFont->SetColor(currentColor);
    }

    if (stw && selectedDeck) drawDeckStatistics();

}

void DeckEditorMenu::drawDeckStatistics()
{
    ostringstream deckStatsString;

    deckStatsString
        << _("------- Deck Summary -----") << endl
        << _("Cards: ") << stw->cardCount << endl
        << _("Creatures: ") << setw(2) << stw->countCreatures
        << _("  Enchantments: ") << stw->countEnchantments << endl
        << _("Instants: ") << setw(4) << stw->countInstants
        << _("   Sorceries:      ") << setw(2) << stw->countSorceries << endl
        << _("Lands: ")
        << _("A: ") << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_ARTIFACT ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_ARTIFACT ] << " "
        << _("G: ") << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_GREEN ] + stw->countLandsPerColor[ Constants::MTG_COLOR_GREEN ] << " "
        << _("R: ") << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_RED ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_RED ] << " "
        << _("U: ") << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_BLUE ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_BLUE ] << " "
        << _("B: ") << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_BLACK ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_BLACK ] << " "
        << _("W: ") << setw(2) << left  << stw->countLandsPerColor[ Constants::MTG_COLOR_WHITE ] + stw->countBasicLandsPerColor[ Constants::MTG_COLOR_WHITE ] << endl
        << _("  --- Card color count ---  ") << endl
        << _("A: ") << setw(2) << left  << selectedDeck->getCount(Constants::MTG_COLOR_ARTIFACT) << " "
        << _("G: ") << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_GREEN) << " "
        << _("U: ") << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_BLUE) << " "
        << _("R: ") << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_RED) << " "
        << _("B: ") << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_BLACK) << " "
        << _("W: ") << setw(2) << left << selectedDeck->getCount(Constants::MTG_COLOR_WHITE) << endl

        << _(" --- Average Cost --- ") << endl
        << _("Creature: ") << setprecision(2) << stw->avgCreatureCost << endl
        << _("Mana: ") << setprecision(2) << stw->avgManaCost << "   "
        << _("Spell: ") << setprecision(2) << stw->avgSpellCost << endl;

    WFont *mainFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    mainFont->DrawString(deckStatsString.str().c_str(), descX, descY + 25);
}

DeckEditorMenu::~DeckEditorMenu()
{
    SAFE_DELETE( mScroller );
}
