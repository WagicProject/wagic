/*
  The shop is where the player can buy cards, decks...
*/
#include "../include/config.h"
#include <JRenderer.h>
#include "../include/GameStateShop.h"
#include "../include/GameApp.h"
#include "../include/MTGDeck.h"
#include "../include/Translate.h"
#include "../include/GameOptions.h"


GameStateShop::GameStateShop(GameApp* parent): GameState(parent) {}


GameStateShop::~GameStateShop() {
  //End(); TODO FIX THAT
}

void GameStateShop::Create(){


}


void GameStateShop::Start()
{
  menu = NULL;


  mStage = STAGE_SHOP_SHOP;

  bgTexture = resources.RetrieveTexture("shop.jpg", RETRIEVE_VRAM);
  
  //alternateRender doesn't lock, so lock our thumbnails for hgeDistort.
  altThumb[0] = resources.RetrieveTexture("artifact_thumb.jpg", RETRIEVE_LOCK);
  altThumb[1] = resources.RetrieveTexture("green_thumb.jpg", RETRIEVE_LOCK);
  altThumb[2] = resources.RetrieveTexture("blue_thumb.jpg", RETRIEVE_LOCK);
  altThumb[3] = resources.RetrieveTexture("red_thumb.jpg", RETRIEVE_LOCK);
  altThumb[4] = resources.RetrieveTexture("black_thumb.jpg", RETRIEVE_LOCK);
  altThumb[5] = resources.RetrieveTexture("white_thumb.jpg", RETRIEVE_LOCK);
  altThumb[6] = resources.RetrieveTexture("land_thumb.jpg", RETRIEVE_LOCK);
  altThumb[7] = resources.RetrieveTexture("gold_thumb.jpg", RETRIEVE_LOCK);

  mBg = resources.RetrieveQuad("shop.jpg");
  mBack = resources.GetQuad("back");

  menuFont = resources.GetJLBFont(Constants::MENU_FONT);
  itemFont = resources.GetJLBFont(Constants::MAIN_FONT);

  JRenderer::GetInstance()->ResetPrivateVRAM();
  JRenderer::GetInstance()->EnableVSync(true);


  shop = NULL;
  load();

}


void GameStateShop::load(){
  if (shop) shop->saveAll();
  SAFE_DELETE(shop);
  int sets[500];
  int boosterSets[500];
  int unlocked[500];
  int nbsets = 0;
  int nbboostersets = 0;

  //Unlock a default set if no set is unlocked
  int ok = 0;
  int defaultSet = 0;
  for (int i = 0; i < MtgSets::SetsList->nb_items; i++){
    string s = MtgSets::SetsList->values[i];
    if (s.compare("10E") == 0) defaultSet = i;
    
    unlocked[i] = options[Options::optionSet(i)].number;
    if (unlocked[i])
      ok = 1;
  }
  if (!ok){
    unlocked[defaultSet] = 1;
    options[Options::optionSet(defaultSet)] = GameOption(1);
    options.save();
  }

  for (int i = 0; i < MtgSets::SetsList->nb_items; i++){
    if (unlocked[i]){
      sets[nbsets] = i;
      nbsets++;
      if (mParent->collection->countBySet(i) > 80){ //Only sets with more than 80 cards can get boosters and starters
        boosterSets[nbboostersets] = i;
        nbboostersets++;
      }
    }
  }
  if (nbboostersets){
    for (int i = 0; i < SHOP_BOOSTERS; i++){
      setIds[i] = boosterSets[(rand() % nbboostersets)];
    } 
  }else{
    for (int i = 0; i < SHOP_BOOSTERS; i++){
      setIds[i] = (rand() % MtgSets::SetsList->nb_items);
    }
  }
  JQuad * mBackThumb = resources.GetQuad("back_thumb");

  

  shop = NEW ShopItems(10, this, itemFont, 10, 0, mParent->collection, setIds);
  for (int i = 0; i < SHOP_BOOSTERS; i++){
    sprintf(setNames[i], "%s Booster (15 %s)",MtgSets::SetsList->values[setIds[i]].c_str(), _("cards").c_str());
    shop->Add(setNames[i],mBack,mBackThumb, 700);
  }
  
  MTGDeck * tempDeck = NEW MTGDeck(mParent->collection);
  tempDeck->addRandomCards(8,sets,nbsets);
  for (map<int,int>::iterator it = tempDeck->cards.begin(); it!=tempDeck->cards.end(); it++){
    for (int j = 0; j < it->second; j++){
      shop->Add(it->first);
    }
  }
  delete tempDeck;
}

void GameStateShop::End()
{
  JRenderer::GetInstance()->EnableVSync(false);
  resources.Release(mBg);
  resources.Release(bgTexture);

  //Release alternate thumbnails.
  for(int i=0;i<8;i++){
    resources.Release(altThumb[i]);
  }

  SAFE_DELETE(shop);
  SAFE_DELETE(menu);
}

void GameStateShop::Destroy(){
}

void GameStateShop::Update(float dt)
{
  //  mParent->effect->UpdateSmall(dt);
  //  mParent->effect->UpdateBig(dt);
  if (mStage == STAGE_SHOP_MENU){
    if (menu){
      menu->Update(dt);
    }else{
      menu = NEW SimpleMenu(11,this,menuFont,SCREEN_WIDTH/2-100,20);
      menu->Add(12,"Save & Back to Main Menu");
      menu->Add(13, "Cancel");
    }
  }else{
    if (mEngine->GetButtonClick(PSP_CTRL_START)){
      mStage = STAGE_SHOP_MENU;
    }
    if (mEngine->GetButtonClick(PSP_CTRL_SQUARE)){
      load();
    }
    if (shop)
      shop->Update(dt);
  }
}


void GameStateShop::Render()
{
  //Erase
  JRenderer * r = JRenderer::GetInstance();
  r->ClearScreen(ARGB(0,0,0,0));
  if (mBg) r->RenderQuad(mBg,0,0);

  if (shop)
    shop->Render();

  if (mStage == STAGE_SHOP_MENU && menu){
    menu->Render();
  }
}

void GameStateShop::ButtonPressed(int controllerId, int controlId)
{
  switch (controllerId){
  case 10:
    if (shop) shop->pricedialog(controlId);
    break;
  case 11:
    if (controlId == 12){
      if (shop) shop->saveAll();
      mParent->SetNextState(GAME_STATE_MENU);
    }else{
      mStage = STAGE_SHOP_SHOP;
    }
    break;
  }
}


