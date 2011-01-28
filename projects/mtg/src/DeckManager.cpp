#include "PrecompiledHeader.h"

#include "DeckManager.h"
#include "Player.h"
#include <JRenderer.h>

void DeckManager::updateMetaDataList(vector<DeckMetaData *> * refList, bool isAI)
{
    if (refList)
    {
        vector<DeckMetaData *> * inputList = isAI ? &aiDeckOrderList : &playerDeckOrderList;
        inputList->clear();
        inputList->assign(refList->begin(), refList -> end());
    }
}

vector<DeckMetaData *> * DeckManager::getPlayerDeckOrderList()
{
    return &playerDeckOrderList;
}

vector<DeckMetaData *> * DeckManager::getAIDeckOrderList()
{
    return &aiDeckOrderList;
}

/*
** Predicate helper for getDeckMetadataByID()
*/
struct DeckIDMatch
{
    DeckIDMatch(int id) : mID(id)
    {
    }

    bool operator() (DeckMetaData* inPtr)
    {
        return inPtr->getDeckId() == mID;
    }

    int mID;
};

DeckMetaData* DeckManager::getDeckMetaDataById( int deckId, bool isAI )
{
    DeckMetaData* deck = NULL;
    std::vector<DeckMetaData *>& deckList = isAI ? aiDeckOrderList : playerDeckOrderList;

    std::vector<DeckMetaData *>::iterator pos = find_if(deckList.begin(), deckList.end(), DeckIDMatch(deckId));
    if (pos != deckList.end())
    {
        deck = *pos;
    }

    return deck;
}

StatsWrapper * DeckManager::getExtendedStatsForDeckId( int deckId, MTGAllCards *collection, bool isAI )
{
    DeckMetaData *selectedDeck = getDeckMetaDataById( deckId, isAI );
    return getExtendedDeckStats( selectedDeck, collection, isAI);
}


StatsWrapper * DeckManager::getExtendedDeckStats( DeckMetaData *selectedDeck, MTGAllCards *collection, bool isAI )
{
    StatsWrapper* stats = NULL;

    string deckName = selectedDeck->getFilename();
    int deckId = selectedDeck->getDeckId();

    map<string, StatsWrapper*>* statsMap = isAI ? &aiDeckStatsMap : &playerDeckStatsMap;
    if (statsMap->find(deckName) == statsMap->end())
    {
        stats = NEW StatsWrapper(deckId);
        stats->updateStats( deckName, collection);
        statsMap->insert( make_pair(deckName, stats));
    }
    else
    {
        stats = statsMap->find(deckName)->second;
    }

    return stats;
}


DeckManager * DeckManager::mInstance = NULL;
bool DeckManager::instanceFlag = false;

void DeckManager::EndInstance()
{
    if (mInstance)
    {
        mInstance->aiDeckOrderList.clear();
        mInstance->playerDeckOrderList.clear();
        map<string, StatsWrapper *>::iterator it;
        for (it = mInstance->aiDeckStatsMap.begin(); it != mInstance->aiDeckStatsMap.end(); it++){
            SAFE_DELETE(it->second);
        }
        for (it = mInstance->playerDeckStatsMap.begin(); it != mInstance->playerDeckStatsMap.end(); it++){
            SAFE_DELETE(it->second);
        }
        mInstance->aiDeckStatsMap.clear();
        mInstance->playerDeckStatsMap.clear();
        SAFE_DELETE( mInstance );
    }
}

DeckManager* DeckManager::GetInstance()
{
    if (!instanceFlag)
    {
        mInstance = NEW DeckManager();
        instanceFlag = true;
    }

    return mInstance;
}

//  p1 is assumed to be the player you want stats for
//  p2 is the opponent
int DeckManager::getDifficultyRating(Player *statsPlayer, Player *player)
{
    DeckMetaDataList * metas = DeckMetaDataList::decksMetaData;

    DeckMetaData *meta = metas->get(player->deckFile, statsPlayer);

    return meta->getDifficulty();
}

DeckManager::~DeckManager()
{
    instanceFlag = false;
}