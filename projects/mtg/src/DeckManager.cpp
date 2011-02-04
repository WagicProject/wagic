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
    else
    {
        ostringstream deckFilename;
        string filepath;
        if ( isAI )
            filepath = options.profileFile( "ai/baka/"); 
        else
            filepath = options.profileFile( "" );
            
        deckFilename << filepath << "/deck" << deckId << ".txt";
        AddMetaData( deckFilename.str(), isAI );
        deck = deckList.back();
    }
    return deck;
}

/*
** Predicate helper for getDeckMetadataByFilename()
*/
struct DeckFilenameMatch
{
    DeckFilenameMatch(const std::string& filename) : mFilename(filename)
    {
    }

    bool operator() (DeckMetaData* inPtr)
    {
        return inPtr->getFilename() == mFilename;
    }

    std::string mFilename;
};

DeckMetaData* DeckManager::getDeckMetaDataByFilename(const string& filename, bool isAI)
{
    DeckMetaData* deck = NULL;
    std::vector<DeckMetaData *>& deckList = isAI ? aiDeckOrderList : playerDeckOrderList;

    std::vector<DeckMetaData *>::iterator pos = find_if(deckList.begin(), deckList.end(), DeckFilenameMatch(filename));
    if (pos != deckList.end())
    {
        deck = *pos;
    }
    else
    {
        if ( FileExists( filename) )
        {
            AddMetaData( filename, isAI );
            deck = deckList.back();
        }
    }
    return deck;
}

void DeckManager::saveDeck( MTGDeck *deck, MTGAllCards *collection )
{
    if ( deck->meta_deck_colors == "" )
    {
        bool isAI = deck->getFilename().find("baka") != string::npos;
        StatsWrapper *stats = getExtendedStatsForDeckId( deck->meta_id, collection, isAI );
        ostringstream manaColorIndex;
        for (int i = Constants::MTG_COLOR_ARTIFACT; i < Constants::MTG_COLOR_LAND; ++i)
        {
            if (stats->totalCostPerColor[i] != 0)
                manaColorIndex << "1";
            else
                manaColorIndex <<"0";
        }
        deck->meta_deck_colors = manaColorIndex.str();
        // save the deck to disk
        deck->save( deck->getFilename(), true, deck->meta_name, deck->meta_desc );
        // update DeckMetaData object
        DeckMetaData *meta = getDeckMetaDataByFilename(deck->getFilename(), isAI);
        meta->setColorIndex( deck->meta_deck_colors );
    }
}

void DeckManager::saveDeck( const string& deckFilename, MTGAllCards *collection )
{
    bool isAI = deckFilename.find("baka") != string::npos;
    DeckMetaData *metaData = getDeckMetaDataByFilename( deckFilename, isAI );
    if ( metaData->getColorIndex() == "" )
    {
        MTGDeck *tempDeck = NEW MTGDeck( deckFilename.c_str(), collection, 0 );
        saveDeck(tempDeck, collection);
        SAFE_DELETE( tempDeck );
    }
}


void DeckManager::AddMetaData( const string& filename, bool isAI )
{
    if (isAI)
    {
        aiDeckOrderList.push_back ( NEW DeckMetaData( filename ) );
        aiDeckStatsMap.insert( make_pair( filename.c_str(), new StatsWrapper( aiDeckOrderList.back()->getDeckId()) ));
    }
    else
    {
        playerDeckOrderList.push_back ( NEW DeckMetaData( filename ) );
        playerDeckStatsMap.insert( make_pair( filename.c_str(), new StatsWrapper( playerDeckOrderList.back()->getDeckId()) ));
    }
}

StatsWrapper * DeckManager::getExtendedStatsForDeckId( int deckId, MTGAllCards *collection, bool isAI )
{
    DeckMetaData *selectedDeck = getDeckMetaDataById( deckId, isAI );
    if (selectedDeck == NULL)
    {
        ostringstream deckName;
        deckName << options.profileFile() << "/deck" << deckId << ".txt";
        map<string, StatsWrapper*>* statsMap = isAI ? &aiDeckStatsMap : &playerDeckStatsMap;
        StatsWrapper * stats = NEW StatsWrapper( deckId );      
        statsMap->insert( make_pair(deckName.str(), stats));
        return stats;
    }
    return getExtendedDeckStats( selectedDeck, collection, isAI);
}


StatsWrapper * DeckManager::getExtendedDeckStats( DeckMetaData *selectedDeck, MTGAllCards *collection, bool isAI )
{
    StatsWrapper* stats = NULL;

    string deckName = selectedDeck->getFilename();
    int deckId = selectedDeck->getDeckId();

    map<string, StatsWrapper*>* statsMap = isAI ? &aiDeckStatsMap : &playerDeckStatsMap;
    if ( statsMap->find(deckName) == statsMap->end())
    {
        stats = NEW StatsWrapper(deckId);
        stats->updateStats( deckName, collection);
        statsMap->insert( make_pair(deckName, stats));
    }
    else
    {
        stats = statsMap->find(deckName)->second;
        if ( stats->needUpdate )
            stats->updateStats( deckName, collection );
    }
    return stats;
}


DeckManager * DeckManager::mInstance = NULL;
bool DeckManager::instanceFlag = false;

void DeckManager::EndInstance()
{
    if (mInstance)
    {
       
        map<string, StatsWrapper *>::iterator it;
        vector<DeckMetaData *>::iterator metaDataIter;
        
        for (it = mInstance->aiDeckStatsMap.begin(); it != mInstance->aiDeckStatsMap.end(); it++){
            SAFE_DELETE(it->second);
        }
        for (it = mInstance->playerDeckStatsMap.begin(); it != mInstance->playerDeckStatsMap.end(); it++){
            SAFE_DELETE(it->second);
        }

        for( metaDataIter =  mInstance->aiDeckOrderList.begin(); metaDataIter !=  mInstance->aiDeckOrderList.end(); ++metaDataIter)
        {
            SAFE_DELETE( *metaDataIter );
        }
        for( metaDataIter = mInstance->playerDeckOrderList.begin(); metaDataIter !=  mInstance->playerDeckOrderList.end(); ++metaDataIter)
        {
            SAFE_DELETE( *metaDataIter );
        }
        mInstance->aiDeckOrderList.clear();
        mInstance->playerDeckOrderList.clear();
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
    DeckMetaData *meta = DeckManager::GetInstance()->getDeckMetaDataByFilename(player->deckFile, (player->isAI() == 1) );
    return meta->getDifficulty();
}

DeckManager::~DeckManager()
{
    instanceFlag = false;
}
