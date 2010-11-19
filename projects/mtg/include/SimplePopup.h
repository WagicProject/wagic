/*
 * SimplePopup.h
 * Created on: Nov 18, 2010
 *
 *      Simple popup dialog box for displaying information only.
 */

#ifndef SIMPLEPOPUP_H_
#define SIMPLEPOPUP_H_

#pragma once
#include <JGui.h>
#include <JTypes.h>
#include <WFont.h>
#include <DeckMetaData.h>

class SimplePopup: public JGuiController
{

private:
    float mHeight, mWidth, mX, mY;
    int mMaxLines;
    int mFontId;
    DeckMetaData * mDeckInformation;
    string mTitle;
    WFont *mTextFont;
    StatsWrapper *mStatsWrapper;
    bool mClosed;
    MTGAllCards * mCollection;

    void drawHorzPole(float x, float y, float width);
    void drawVertPole(float x, float y, float height);

public:
    bool autoTranslate;


    SimplePopup(int id, JGuiListener* listener, const int fontId, const char * _title = "", DeckMetaData* deckInfo = NULL, MTGAllCards * collection = NULL);
    ~SimplePopup(void);
    
    bool isClosed() { return mClosed; }
    MTGAllCards* getCollection() { return mCollection; }
    void Render();
    void Update(DeckMetaData* deckMetaData);

    string getDetailedInformation(string deckFilename);

    void Update(float dt);
    void Close();
};

#endif /* SIMPLEPOPUP_H_ */
