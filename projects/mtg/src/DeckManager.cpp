#include "PrecompiledHeader.h"

#include "DeckManager.h"
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
    mInstance = new DeckManager();
    instanceFlag = true;
  }

  return mInstance;
}
