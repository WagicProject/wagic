/*
  The shop is where the player can buy cards, decks...
*/
#include "../include/config.h"
#include <JRenderer.h>
#include "../include/GameStateShop.h"
#include "../include/GameApp.h"
#include "../include/MTGDeck.h"


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
  mBg = NEW JQuad(bgTexture, 0, 0, 400, 280);		// Create background quad for rendering.
  backTexture = JRenderer::GetInstance()->LoadTexture("sets/back.jpg", TEX_TYPE_USE_VRAM);
  mBack = NEW JQuad(backTexture, 0, 0, 200, 285);		// Create background quad for rendering.

  JRenderer::GetInstance()->ResetPrivateVRAM();
  JRenderer::GetInstance()->EnableVSync(true);


  int sets[500];
  int nbsets = 0;
  for (int i = 0; i < MtgSets::SetsList->nb_items; i++){
    if (mParent->collection->countBySet(i) > 100){ //Only sets with more than 100 cards can get boosters and starters
      sets[nbsets] = i;
      nbsets++;
    }
  }
  if (nbsets){
    for (int i = 0; i < SHOP_BOOSTERS; i++){
      setIds[i] = sets[(rand() % nbsets)];
    } 
  }else{
    for (int i = 0; i < SHOP_BOOSTERS; i++){
      setIds[i] = (rand() % MtgSets::SetsList->nb_items);
    }
  }
  JQuad * mBackThumb = GameApp::CommonRes->GetQuad("back_thumb");

  shop = NULL;

  shop = NEW ShopItems(10, this, itemFont, 10, 0, mParent->collection, setIds);
  for (int i = 0; i < SHOP_BOOSTERS; i++){
    sprintf(setNames[i], "%s Booster (15 cards)",MtgSets::SetsList->values[setIds[i]].c_str());
    shop->Add(setNames[i],mBack,mBackThumb, 1200);
  }
  
  for (int i = 0; i < 8; i++){
    shop->Add(mParent->collection->randomCardId());
  }

}


void GameStateShop::End()
{
  JRenderer::GetInstance()->EnableVSync(false);
  SAFE_DELETE(shop);
  SAFE_DELETE(mBack);
  SAFE_DELETE(backTexture);
  SAFE_DELETE(bgTexture);
  SAFE_DELETE(mBg);
  SAFE_DELETE(menu);

}

void GameStateShop::Destroy(){
}

void GameStateShop::Update(float dt)
{
  if (mStage == STAGE_SHOP_MENU){
    if (menu){
      menu->Update(dt);
    }else{
      menu = NEW SimpleMenu(11,this,menuFont,SCREEN_WIDTH/2-100,20);
      menu->Add(12,"Save & Back to main menu");
      menu->Add(13, "Cancel");
    }
  }else{
    if (mEngine->GetButtonClick(PSP_CTRL_START)){
      mStage = STAGE_SHOP_MENU;
    }
    if (shop)
      shop->Update(dt);
  }
}


void GameStateShop::Render()
{
  //Erase
  JRenderer::GetInstance()->ClearScreen(ARGB(0,0,0,0));
  if (mBg)JRenderer::GetInstance()->RenderQuad(mBg,0,0);
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


