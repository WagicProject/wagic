#include "PrecompiledHeader.h"

#include "DeckMetaData.h"
#include "DeckStats.h"
#include "MTGDeck.h"
#include "utils.h"
#include "Translate.h"

//Possible improvements:
//Merge this with DeckStats
//Have this class handle all the Meta Data rather than relying on MTGDeck. Then MTGDeck would have a MetaData object...

DeckMetaData::DeckMetaData(const string& filename, bool isAI)
    : mFilename(filename), mGamesPlayed(0), mVictories(0), mPercentVictories(0), mDifficulty(0),
      mDeckLoaded(false), mStatsLoaded(false), mIsAI(isAI)
{
    // TODO, figure out how we can defer this to later - currently, 
    // there's a catch 22, as we sort the deck list alphabetically, so we need to open the deck file
    // to get its name.  This means that for the opponent list, we crack open 106 files just to read the deck name
    //, which is the bulk of the remaining 4 second delay we see the first time we try to pick an opponent on the first match
    LoadDeck();
}

void DeckMetaData::LoadStats()
{
    if (!mStatsLoaded)
    {
        DeckStats * stats = DeckStats::GetInstance();
        if (mIsAI)
        {
            stats->load(mPlayerDeck);
            DeckStat * opponentDeckStats = stats->getDeckStat(mStatsFilename);
            if (opponentDeckStats)
            {
                mPercentVictories = stats->percentVictories(mStatsFilename);
                mVictories = opponentDeckStats->victories;
                mGamesPlayed = opponentDeckStats->nbgames;
                mColorIndex = opponentDeckStats->manaColorIndex;
 
                if (mPercentVictories < 34)
                {
                    mDifficulty = HARD;
                }
                else if (mPercentVictories < 55)
                {
                    mDifficulty = NORMAL;
                }
                else
                {
                    mDifficulty = EASY;
                }
            }
        }
        else
        {
            if (FileExists(mStatsFilename))
            {
                stats->load(mStatsFilename);
                mGamesPlayed = stats->nbGames();               
                mPercentVictories = stats->percentVictories();
                mVictories = static_cast<int>(mGamesPlayed * (mPercentVictories / 100.0f));
            }
        }

        mStatsLoaded = true;
    }

}

// since we only have 100 stock avatar images, we need to recycle the images for deck numbers > 99
int DeckMetaData::getAvatarId()
{
    if ( mDeckId < 101 )
        return mDeckId;
    int tempDeckId = mDeckId;
    int avatarId = 0;
    while(tempDeckId > 100)
    {
        tempDeckId -= 100;
    }
    avatarId = tempDeckId;
    return avatarId;
}

void DeckMetaData::LoadDeck()
{
    if (!mDeckLoaded)
    {
        MTGDeck deck(mFilename.c_str(), NULL, 1);
        mName = trim(deck.meta_name);
        mDescription = trim(deck.meta_desc);
        mDeckId = atoi((mFilename.substr(mFilename.find("deck") + 4, mFilename.find(".txt"))).c_str());
        mDeckLoaded = true;
        if (!mIsAI)
            mAvatarFilename = "avatar.jpg";
        else
        {
            ostringstream avatarFilename;
            avatarFilename << "avatar" << getAvatarId() << ".jpg";
            mAvatarFilename = avatarFilename.str();
        }        
    }


}

//Accessors

string DeckMetaData::getFilename()
{
    return mFilename;
}

string DeckMetaData::getName()
{
    return mName;
}

int DeckMetaData::getDeckId()
{
    return mDeckId;
}

string DeckMetaData::getAvatarFilename()
{
    return mAvatarFilename;
}

string DeckMetaData::getColorIndex()
{
    return mColorIndex;
}

int DeckMetaData::getGamesPlayed()
{
    return mGamesPlayed;
}

int DeckMetaData::getVictories()
{
    return mVictories;
}

int DeckMetaData::getVictoryPercentage()
{
    return mPercentVictories;
}

int DeckMetaData::getDifficulty()
{
    return mDifficulty;
}

string DeckMetaData::getDifficultyString()
{
    string difficultyString = "Normal";
    switch (mDifficulty)
    {
    case HARD:
        difficultyString = "Hard";
        break;
    case EASY:
        difficultyString = "Easy";
        break;
    }

    return difficultyString;
}

string DeckMetaData::getDescription()
{
    return mDescription;
}

string DeckMetaData::getStatsSummary()
{
    LoadStats();

    ostringstream statsSummary;
    statsSummary << _("Difficulty: ") << _(getDifficultyString()) << endl
                    << _("Victory %: ") << getVictoryPercentage() << endl
                    << _("Games Played: ") << getGamesPlayed() << endl;

    return statsSummary.str();
}

void DeckMetaData::setColorIndex(const string& colorIndex)
{
    mColorIndex = colorIndex;
}

void DeckMetaData::setDeckName(const string& newDeckTitle)
{
    mName = newDeckTitle;
}

void DeckMetaData::Invalidate()
{
    mStatsLoaded = false;
}
