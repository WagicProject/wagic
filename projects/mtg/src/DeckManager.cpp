#include "../include/DeckManager.h"
#include <JRenderer.h>

DeckManager::DeckManager()
{
}

DeckManager::~DeckManager()
{
}

void DeckManager::updateMetaDataList( vector<DeckMetaData *> * refList, bool isAI )
{
  if (refList)
  {
    vector<DeckMetaData *> * inputList = isAI? &aiDeckOrderList : &playerDeckOrderList;
    inputList->clear();
    inputList->assign( refList->begin(), refList->end());
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


DeckManager* DeckManager::GetInstance()
{
  if ( mInstance == NULL )
    mInstance = new DeckManager();
  return mInstance;
}

void DeckManager::EndInstance()
{
  SAFE_DELETE(mInstance);
}
