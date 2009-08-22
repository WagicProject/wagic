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
  menuFont = GameApp::CommonRes->GetJLBFont(Constants::MENU_FONT);
  itemFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);


  mStage = STAGE_SHOP_SHOP;

  bgTexture = JRenderer::GetInstance()->LoadTexture("graphics/shop.jpg", TEX_TYPE_USE_VRAM);
  mBg = NEW JQuad(bgTexture, 0, 0, 480, 272);		// Create background quad for rendering.
  mBack = GameApp::CommonRes->GetQuad("back");

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
    char buffer[4096];
    sprintf(buffer,"unlocked_%s", s.c_str());
    unlocked[i] = options[buffer].number;
    if (unlocked[i])
      ok = 1;
  }
  if (!ok){
    unlocked[defaultSet] = 1;
    string s = MtgSets::SetsList->values[defaultSet];
    char buffer[4096];
    sprintf(buffer,"unlocked_%s", s.c_str());
    options[buffer] = GameOption(1);
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
  JQuad * mBackThumb = GameApp::CommonRes->GetQuad("back_thumb");

  

  shop = NEW ShopItems(10, this, itemFont, 10, 0, mParent->collection, setIds);
  for (int i = 0; i < SHOP_BOOSTERS; i++){
    sprintf(setNames[i], "%s Booster (15 %s)",MtgSets::SetsList->values[setIds[i]].c_str(), _("cards").c_str());
    shop->Add(setNames[i],mBack,mBackThumb, 700);
  }
  
  MTGDeck * tempDeck = NEW MTGDeck(NULL,mParent->collection);
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
  SAFE_DELETE(shop);
  SAFE_DELETE(bgTexture);
  SAFE_DELETE(mBg);
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
  if (mBg)JRenderer::GetInstance()->RenderQuad(mBg,0,0);

  itemFont->SetColor(ARGB(255,255,255,255));
  char c[4096];
  sprintf(c, _("press [] to refresh").c_str());
  unsigned int len = 4 + itemFont->GetStringWidth(c);
  itemFont->DrawString(c,SCREEN_WIDTH-len,SCREEN_HEIGHT-12);

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


