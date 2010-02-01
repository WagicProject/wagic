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
  mStage = STAGE_FADE_IN;
  
  //alternateRender doesn't lock, so lock our thumbnails for hgeDistort.
  altThumb[0] = resources.RetrieveTexture("artifact_thumb.jpg", RETRIEVE_LOCK);
  altThumb[1] = resources.RetrieveTexture("green_thumb.jpg", RETRIEVE_LOCK);
  altThumb[2] = resources.RetrieveTexture("blue_thumb.jpg", RETRIEVE_LOCK);
  altThumb[3] = resources.RetrieveTexture("red_thumb.jpg", RETRIEVE_LOCK);
  altThumb[4] = resources.RetrieveTexture("black_thumb.jpg", RETRIEVE_LOCK);
  altThumb[5] = resources.RetrieveTexture("white_thumb.jpg", RETRIEVE_LOCK);
  altThumb[6] = resources.RetrieveTexture("land_thumb.jpg", RETRIEVE_LOCK);
  altThumb[7] = resources.RetrieveTexture("gold_thumb.jpg", RETRIEVE_LOCK);
  
  mBack = resources.GetQuad("back");
  resources.Unmiss("shop.jpg"); //Last resort.
  mBgTex = resources.RetrieveTexture("shop.jpg", RETRIEVE_LOCK, TEXTURE_SUB_5551);
  if(mBgTex)
    mBg = resources.RetrieveQuad("shop.jpg");  
  else
    mBg = NULL;

  JRenderer::GetInstance()->EnableVSync(true);

  shop = NULL;
  taskList = NULL;
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

  //Figure out which sets are available.
  for (int i = 0; i < setlist.size(); i++){    
    unlocked[i] = options[Options::optionSet(i)].number;
  }
  for (int i = 0; i < setlist.size(); i++){
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
      setIds[i] = (rand() % setlist.size());
    }
  }
  JQuad * mBackThumb = resources.GetQuad("back_thumb");

  

  shop = NEW ShopItems(10, this, resources.GetJLBFont(Constants::MAIN_FONT), 10, 0, mParent->collection, setIds);
  MTGSetInfo * si = NULL;
  for (int i = 0; i < SHOP_BOOSTERS; i++){
    si = setlist.getInfo(setIds[i]);
    if(!si)
      continue;

    sprintf(setNames[i], "%s %s (%i %s)", si->id.c_str(), _("Booster").c_str(), si->boosterSize(), _("Cards").c_str());
    shop->Add(setNames[i],mBack,mBackThumb, si->boosterCost());
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
  resources.Release(mBgTex);
  mBgTex = NULL;
  mBg = NULL;

  //Release alternate thumbnails.
  for(int i=0;i<8;i++){
    resources.Release(altThumb[i]);
  }

  SAFE_DELETE(shop);
  SAFE_DELETE(menu);
  SAFE_DELETE(taskList);
}

void GameStateShop::Destroy(){
}

void GameStateShop::Update(float dt)
{
  //  mParent->effect->UpdateSmall(dt);
  //  mParent->effect->UpdateBig(dt);
  u32 btn; 
  switch(mStage){
    case STAGE_SHOP_MENU:
    if (menu){
      menu->Update(dt);
    }else{
      menu = NEW SimpleMenu(11,this,Constants::MENU_FONT,SCREEN_WIDTH/2-100,20);
      menu->Add(12,"Save & Back to Main Menu");
      menu->Add(14,"See available tasks");
      menu->Add(13, "Cancel");
    } 
    break;
    case STAGE_SHOP_TASKS:      
      if(menu){
        menu->Update(dt);
        return;
      }
      if(taskList){
      btn = mEngine->ReadButton();
      taskList->Update(dt);
        if ( taskList->getState() != TaskList::TASKS_INACTIVE){
          if( btn == PSP_CTRL_CROSS || btn == PSP_CTRL_TRIANGLE ){
             taskList->End();
             return;
          }else if(taskList->getState() == TaskList::TASKS_ACTIVE && btn == PSP_CTRL_START ){
            if(!menu){
              menu = NEW SimpleMenu(11,this,Constants::MENU_FONT,SCREEN_WIDTH/2-100,20);
              menu->Add(12,"Save & Back to Main Menu");
              menu->Add(15,"Close tasks");
              menu->Add(13, "Cancel");
            }
          }
        }
        else
          mStage = STAGE_SHOP_SHOP;
      }

#ifdef TESTSUITE
      if ((mEngine->GetButtonClick(PSP_CTRL_SQUARE)) && (taskList)) {
        taskList->passOneDay();
        if (taskList->getTaskCount() < 6) {
          taskList->addRandomTask();
          taskList->addRandomTask();
        }
       taskList->save();
      }
#endif
    break;
    case STAGE_SHOP_SHOP:
      btn = mEngine->ReadButton();
      if (btn == PSP_CTRL_START){
        mStage = STAGE_SHOP_MENU;
        return;
      }else if(btn == PSP_CTRL_SQUARE){
        load();
      }
      if (shop){
        shop->CheckUserInput(btn);
        shop->Update(dt);
      }
      break;
    case STAGE_FADE_IN:
      mParent->DoAnimation(TRANSITION_FADE_IN);
      mStage = STAGE_SHOP_SHOP;
      break;
  }
}


void GameStateShop::Render()
{
  //Erase
  JRenderer * r = JRenderer::GetInstance();
  r->ClearScreen(ARGB(0,0,0,0));
  if(mStage == STAGE_FADE_IN)
    return;

  if (mBg) r->RenderQuad(mBg,0,0);


  if (shop)
    shop->Render();
 
  if (mStage == STAGE_SHOP_TASKS && taskList) {
    taskList->Render();
  }

  if (menu){
    menu->Render();
  }
}

void GameStateShop::ButtonPressed(int controllerId, int controlId)
{
  if (controllerId == 10){
    if (shop) 
      shop->pricedialog(controlId);
  }
  else{
    switch(controlId){
      case 12:
        if (shop) shop->saveAll();
        if (taskList) taskList->save();
        mParent->DoTransition(TRANSITION_FADE,GAME_STATE_MENU);
        mStage = STAGE_SHOP_SHOP;
        break;
      case 14:          
        mStage = STAGE_SHOP_TASKS;
        if (!taskList)
          taskList = NEW TaskList();
        taskList->Start();    
        break;
      case 15:
        if(taskList)
          taskList->End();
        break;
      default:
        mStage = STAGE_SHOP_SHOP;
    }
    SAFE_DELETE(menu);
  }
}