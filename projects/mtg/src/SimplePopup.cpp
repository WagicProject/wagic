/*
 * SimplePopup.cpp
 *
 *  Created on: Nov 18, 2010
 *      Author: Michael
 */

#include "PrecompiledHeader.h"
#include "SimplePopup.h"
#include "JTypes.h"
#include "GameApp.h"
#include "DeckStats.h"
#include "DeckManager.h"
#include <iomanip>

SimplePopup::SimplePopup(int id, JGuiListener* listener, const int fontId, const char * _title, DeckMetaData* deckMetaData, MTGAllCards * collection) :
    JGuiController(id, listener), mFontId(fontId), mCollection(collection)
{
    mX = 35;
    mY = 50;
    mTitle = _title;
    mMaxLines = 10;
    mTextFont = resources.GetWFont(fontId);
    this->mCount = 1;
    stw = NULL;
    Update(deckMetaData);
}

void SimplePopup::Render()
{
    closed = false;

    JRenderer *r = JRenderer::GetInstance();
    string detailedInformation = getDetailedInformation(mDeckInformation->getFilename());

    mTextFont->SetScale(0.85f);
    const float textWidth = 183.0f;
    const float textHeight = mTextFont->GetHeight() * 10;
    r->DrawRoundRect(mX, mY, textWidth, textHeight, 2.0f, ARGB( 255, 125, 255, 0) );
    r->FillRoundRect(mX, mY, textWidth, textHeight, 2.0f, ARGB( 255, 0, 0, 0 ) );

    mTextFont->DrawString(detailedInformation.c_str(), mX + 20 , mY + 10);

}
void SimplePopup::Update(DeckMetaData* selectedDeck)
{
    mDeckInformation = selectedDeck;
    SAFE_DELETE(stw);
    stw = NEW StatsWrapper(mDeckInformation->getDeckId());
    stw->updateStats(mDeckInformation->getFilename(), mCollection);
}


string SimplePopup::getDetailedInformation(string filename)
{
    ostringstream oss;
    oss
        << "------- Deck Summary -----" << endl
        << "Cards: "<< stw->cardCount << endl
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
        << "  --- Mana Curve ---  " << endl;

    for ( int costIdx = 0; costIdx < Constants::STATS_MAX_MANA_COST+1; ++costIdx )
            if ( stw->countCardsPerCost[ costIdx ] > 0 )
                oss << costIdx << ": " << setw(2) << left << stw->countCardsPerCost[ costIdx ] << "  ";

    oss << endl;

    oss
        << " --- Average Cost --- " << endl
        << "Creature: "<< setprecision(2) << stw->avgCreatureCost << endl
        << "Mana: " << setprecision(2) << stw->avgManaCost << "   "
        << "Spell: " << setprecision(2) << stw->avgSpellCost << endl;

    return oss.str();
}

  void SimplePopup::Update(float dt)
{
    JButton key = mEngine->ReadButton();
    CheckUserInput(key);
}

void SimplePopup::Close()
{
    closed = true;
    mCount = 0;
}

SimplePopup::~SimplePopup(void)
{
    mTextFont = NULL;
    mDeckInformation = NULL;
    SAFE_DELETE(stw);
}

void SimplePopup::drawHorzPole(float x, float y, float width)
{

}

void SimplePopup::drawVertPole(float x, float y, float height)
{

}
