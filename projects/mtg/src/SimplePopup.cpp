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
    mStatsWrapper = NULL;
    Update(deckMetaData);
}

void SimplePopup::Render()
{
    mClosed = false;

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
    SAFE_DELETE(mStatsWrapper);
    mStatsWrapper = NEW StatsWrapper(mDeckInformation->getDeckId());
    mStatsWrapper->updateStats(mDeckInformation->getFilename(), mCollection);
}


string SimplePopup::getDetailedInformation(string filename)
{
    ostringstream oss;
    oss
        << "------- Deck Summary -----" << endl
        << "Cards: "<< mStatsWrapper->cardCount << endl
        << "Creatures: "<< setw(2) << mStatsWrapper->countCreatures
        << "  Enchantments: " << mStatsWrapper->countEnchantments << endl
        << "Instants: " << setw(4) << mStatsWrapper->countInstants
        << "   Sorceries:      " << setw(2) << mStatsWrapper->countSorceries << endl
        << "Lands: "
        << "A: " << setw(2) << left  << mStatsWrapper->countLandsPerColor[ Constants::MTG_COLOR_ARTIFACT ] + mStatsWrapper->countBasicLandsPerColor[ Constants::MTG_COLOR_ARTIFACT ] << " "
        << "G: " << setw(2) << left  << mStatsWrapper->countLandsPerColor[ Constants::MTG_COLOR_GREEN ] + mStatsWrapper->countLandsPerColor[ Constants::MTG_COLOR_GREEN ] << " "
        << "R: " << setw(2) << left  << mStatsWrapper->countLandsPerColor[ Constants::MTG_COLOR_RED ] + mStatsWrapper->countBasicLandsPerColor[ Constants::MTG_COLOR_RED ] << " "
        << "U: " << setw(2) << left  << mStatsWrapper->countLandsPerColor[ Constants::MTG_COLOR_BLUE ] + mStatsWrapper->countBasicLandsPerColor[ Constants::MTG_COLOR_BLUE ] << " "
        << "B: " << setw(2) << left  << mStatsWrapper->countLandsPerColor[ Constants::MTG_COLOR_BLACK ] + mStatsWrapper->countBasicLandsPerColor[ Constants::MTG_COLOR_BLACK ] << " "
        << "W: " << setw(2) << left  << mStatsWrapper->countLandsPerColor[ Constants::MTG_COLOR_WHITE ] + mStatsWrapper->countBasicLandsPerColor[ Constants::MTG_COLOR_WHITE ] << endl
        << "  --- Mana Curve ---  " << endl;

    for ( int costIdx = 0; costIdx < Constants::STATS_MAX_MANA_COST+1; ++costIdx )
            if ( mStatsWrapper->countCardsPerCost[ costIdx ] > 0 )
                oss << costIdx << ": " << setw(2) << left << mStatsWrapper->countCardsPerCost[ costIdx ] << "  ";

    oss << endl;

    oss
        << " --- Average Cost --- " << endl
        << "Creature: "<< setprecision(2) << mStatsWrapper->avgCreatureCost << endl
        << "Mana: " << setprecision(2) << mStatsWrapper->avgManaCost << "   "
        << "Spell: " << setprecision(2) << mStatsWrapper->avgSpellCost << endl;

    return oss.str();
}

  void SimplePopup::Update(float dt)
{
    JButton key = mEngine->ReadButton();
    CheckUserInput(key);
}

void SimplePopup::Close()
{
    mClosed = true;
    mCount = 0;
}

SimplePopup::~SimplePopup(void)
{
    mTextFont = NULL;
    mDeckInformation = NULL;
    SAFE_DELETE(mStatsWrapper);
}

void SimplePopup::drawHorzPole(float x, float y, float width)
{

}

void SimplePopup::drawVertPole(float x, float y, float height)
{

}
