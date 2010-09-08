#include "../include/DeckManager.h"
#include <JRenderer.h>

DeckManager::DeckManager()
{
}

DeckManager::~DeckManager()
{
}


vector<int> * DeckManager::getPlayerDeckOrderList()
{
    return &playerDeckOrderList;
}
    
vector<int> * DeckManager::getAIDeckOrderList()
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
