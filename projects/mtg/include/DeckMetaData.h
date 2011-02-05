#ifndef _DECKMETADATA_H_
#define _DECKMETADATA_H_

#include <string>
#include <vector>
#include <map>
#include "DeckStats.h"

using namespace std;
enum DECK_DIFFICULTY
{
    HARD = -1,
    NORMAL = 0,
    EASY = 1
};

class DeckMetaData
{
private:
    string mFilename;
    string mDescription;
    string mName;
    int mDeckId;
    string mAvatarFilename;
    string  mColorIndex;
    
    // statistical information
    int mGamesPlayed, mVictories, mPercentVictories, mDifficulty;

    DeckMetaData();

public:


    DeckMetaData(const string& filename);
    void LoadDeck();
    void LoadStats();

    // Accessors
    string getFilename();
    string getDescription();
    string getName();
    string getAvatarFilename();
    string getColorIndex();
    int getAvatarId(int deckId);
    string getStatsSummary();

    int getDeckId();
    int getGamesPlayed();
    int getVictories();
    int getVictoryPercentage();
    int getDifficulty();
    string getDifficultyString();

    // setters
    void setColorIndex(const string& colorIndex);
    void setDeckName( const string& newDeckTitle );
    void Invalidate();

    string mStatsFilename;
    string mPlayerDeck;
    bool mDeckLoaded;
    bool mStatsLoaded;
    bool mIsAI;
};


#endif
