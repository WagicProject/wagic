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
    JGuiController(JGE::GetInstance(), id, listener), mFontId(fontId), mCollection(collection)
{
    mX = 19;
    mY = 66;
	mWidth = 180.0f;
    mTitle = _title;
    mMaxLines = 12;

    mTextFont = WResourceManager::Instance()->GetWFont(fontId);
    this->mCount = 1; // a hack to ensure the menus do book keeping correctly.  Since we aren't adding items to the menu, this is required
    mStatsWrapper = NULL;
    Update(deckMetaData);
}

void SimplePopup::Render()
{
    mClosed = false;

    JRenderer *r = JRenderer::GetInstance();
    string detailedInformation = getDetailedInformation(mDeckInformation->getFilename());

    const float textHeight = mTextFont->GetHeight() * mMaxLines;
    r->DrawRoundRect(mX, mY, mWidth, textHeight, 2.0f, ARGB( 255, 125, 255, 0) );
    r->FillRoundRect(mX, mY, mWidth, textHeight, 2.0f, ARGB( 255, 0, 0, 0 ) );

	// currently causes a crash on the PSP when drawing the corners.
	// TODO: clean up the image ot make it loook cleaner. Find solution to load gfx to not crash PSP
#if 0
	drawBoundingBox( mX, mY, mWidth, textHeight );
#endif
    mTextFont->DrawString(detailedInformation.c_str(), mX + 9 , mY + 15);

}

// draws a bounding box around the popup.
void SimplePopup::drawBoundingBox( float x, float y, float width, float height )
{

    //draw the corners
    string topCornerImageName = "top_corner.png";
    string bottomCornerImageName = "bottom_corner.png";
	string verticalBarImageName = "vert_bar.png";
    string horizontalBarImageName = "top_bar.png";

	const float boxWidth	= ( width + 15 ) / 3.0f;
	const float boxHeight = ( height + 15 ) / 3.0f;

	drawHorzPole( horizontalBarImageName, false, false, x, y, boxWidth );
	drawHorzPole( horizontalBarImageName, false, true, x, y + height, boxWidth );
	
	drawVertPole( verticalBarImageName, false, false, x, y, boxHeight );
	drawVertPole( verticalBarImageName, true, false, x + width, y, boxHeight );

	drawCorner( topCornerImageName, false, false, x, y );
	drawCorner( topCornerImageName, true, false, x + width, y );
	drawCorner( bottomCornerImageName, false, false, x, y + height );
	drawCorner( bottomCornerImageName, true, false, x + width, y + height );
}

void SimplePopup::Update(DeckMetaData* selectedDeck)
{
    mDeckInformation = selectedDeck;
    
    // get the information from the cache, if it doesn't exist create an entry
    mStatsWrapper = DeckManager::GetInstance()->getExtendedDeckStats( mDeckInformation, mCollection, (mDeckInformation->getFilename().find("baka") != string::npos) );
    
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

// drawing routines
void SimplePopup::drawCorner(string imageName, bool flipX, bool flipY, float x, float y)
{
	LOG(" Drawing a Corner! ");
    JRenderer* r = JRenderer::GetInstance();
    JQuadPtr horizontalBarImage = WResourceManager::Instance()->RetrieveTempQuad( imageName, TEXTURE_SUB_5551);
	horizontalBarImage->SetHFlip(flipX);
	horizontalBarImage->SetVFlip(flipY);

	r->RenderQuad(horizontalBarImage.get(), x, y);
	LOG(" Done Drawing a Corner! ");
}

void SimplePopup::drawHorzPole(string imageName, bool flipX = false, bool flipY = false, float x = 0, float y = 0, float width = SCREEN_WIDTH_F)
{
	LOG(" Drawing a horizontal border! ");
    JRenderer* r = JRenderer::GetInstance();
    JQuadPtr horizontalBarImage = WResourceManager::Instance()->RetrieveTempQuad( imageName, TEXTURE_SUB_5551);
	if ( horizontalBarImage != NULL )
	{
	horizontalBarImage->SetHFlip(flipX);
	horizontalBarImage->SetVFlip(flipY);

	r->RenderQuad(horizontalBarImage.get(), x, y, 0, width);
	}
	else
	{
		LOG ( "ERROR: Error trying to render horizontal edge! ");
	}
	LOG(" Done Drawing a horizontal border! ");
}

void SimplePopup::drawVertPole(string imageName, bool flipX = false, bool flipY = false, float x = 0, float y = 0, float height = SCREEN_HEIGHT_F)
{
	LOG(" Drawing a Vertical border! ");
    JRenderer* r = JRenderer::GetInstance();
    JQuadPtr verticalBarImage = WResourceManager::Instance()->RetrieveTempQuad( imageName, TEXTURE_SUB_5551);
	if ( verticalBarImage != NULL )
	{
		verticalBarImage->SetHFlip(flipX);
		verticalBarImage->SetVFlip(flipY);

		r->RenderQuad(verticalBarImage.get(), x, y, 0, 1.0f, height);
	}
	else
	{
		LOG ( "ERROR: Error trying to render vertical edge! ");
	}
	LOG(" DONE Drawing a horizontal border! ");
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
}

