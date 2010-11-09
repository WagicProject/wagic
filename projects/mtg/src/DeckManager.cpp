#include "PrecompiledHeader.h"

#include "DeckManager.h"
#include "Player.h"
#include <JRenderer.h>

void DeckManager::updateMetaDataList( vector<DeckMetaData *> * refList, bool isAI )
{
  if (refList)
  {
    vector<DeckMetaData *> * inputList = isAI? &aiDeckOrderList : &playerDeckOrderList;
    inputList->clear();
    inputList->assign( refList->begin(), refList -> end() );
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


DeckManager * DeckManager::mInstance = NULL;
bool DeckManager::instanceFlag = false;

void DeckManager::EndInstance()
{
  if (mInstance)
  {
    mInstance->aiDeckOrderList.clear();
    mInstance->playerDeckOrderList.clear();
    SAFE_DELETE( mInstance );
  }
}


DeckManager* DeckManager::GetInstance()
{
  if ( !instanceFlag )
  {
    mInstance = NEW DeckManager();
    instanceFlag = true;
  }

  return mInstance;
}


//  p1 is assumed to be the player you want stats for
//  p2 is the opponent
int DeckManager::getDifficultyRating( Player *statsPlayer, Player *player )
{
  DeckMetaDataList * metas = DeckMetaDataList::decksMetaData;

  DeckMetaData *meta = metas->get( player->deckFile, statsPlayer );

  return meta->getDifficulty();
}
