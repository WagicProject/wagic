/*
  The shop is where the player can buy cards, decks...
*/
#include "../include/config.h"
#include <JRenderer.h>
#include "../include/GameStateShop.h"
#include "../include/GameApp.h"
#include "../include/MTGDeck.h"
#include "../include/MTGPack.h"
#include "../include/Translate.h"
#include "../include/GameOptions.h"
#include <hge/hgedistort.h>

float GameStateShop::_x1[] = { 79, 19, 27,103,154,187,102,144,198,133,183};
float GameStateShop::_y1[] = {150,194,222,167,164,156,195,190,175,220,220};

float GameStateShop::_x2[] = {103, 48, 74,135,183,215,138,181,231,171,225};
float GameStateShop::_y2[] = {155,179,218,165,166,155,195,186,177,225,216};

float GameStateShop::_x3[] = { 48, 61, 9, 96,139,190, 81,146,187, 97,191};
float GameStateShop::_y3[] = {164,205,257,184,180,170,219,212,195,251,252};

float GameStateShop::_x4[] = { 76, 90, 65,131,171,221,123,187,225,141,237};
float GameStateShop::_y4[] = {169,188,250,182,182,168,220,208,198,259,245};

GameStateShop::GameStateShop(GameApp* parent): GameState(parent) {
  menu = NULL;
  for(int i=0;i<8;i++)
    altThumb[i] = NULL;
  mBack = NULL;
  boosterDisplay = NULL;
  mBg = NULL;
  mBgTex = NULL;
  taskList = NULL;
  srcCards = NULL;
  shopMenu = NULL;
  bigDisplay = NULL;
  myCollection = NULL;
  packlist = NULL;
  pricelist = NULL;
  playerdata = NULL;
  booster = NULL;
  lightAlpha = 0;
  filterMenu = NULL;
  alphaChange = 0;
  for(int i=0;i<SHOP_ITEMS;i++){
    mPrices[i] = 0;
    mCounts[i] = 0;
  }
  mTouched = false;
}


GameStateShop::~GameStateShop() {
  End();
}

void GameStateShop::Create(){
}


void GameStateShop::Start(){
  menu = NULL;
  bListCards = false;
  mTouched = false;
  mStage = STAGE_FADE_IN;
  mElapsed = 0;
  needLoad = true;
  booster = NULL;
  srcCards = NEW WSrcUnlockedCards(0);
  srcCards->setElapsed(15);
  srcCards->addFilter(NEW WCFilterNOT(NEW WCFilterRarity("T")));

  bigSync = 0;
  shopMenu = NEW WGuiMenu(JGE_BTN_DOWN, JGE_BTN_UP, true, &bigSync);
  MTGAllCards * ac = GameApp::collection;
  playerdata = NEW PlayerData(ac);;
  myCollection = 	 NEW DeckDataWrapper(playerdata->collection);
  pricelist = NEW PriceList(RESPATH"/settings/prices.dat",ac);
  for(int i=0;i<SHOP_SLOTS;i++){
    WGuiCardDistort * dist;
    if(i < BOOSTER_SLOTS)
      dist = NEW WGuiCardDistort(NULL,true);
    else{
      dist = NEW WGuiCardDistort(srcCards,true);
      dist->mOffset.setOffset(i-BOOSTER_SLOTS);
    }
    dist->xy = WDistort(_x1[i],_y1[i],_x2[i],_y2[i],_x3[i],_y3[i],_x4[i],_y4[i]);
    shopMenu->Add(NEW WGuiButton(dist,-102,i,this));
  }
  shopMenu->Entering(JGE_BTN_NONE);

  if(!bigDisplay){
    bigDisplay = NEW WGuiCardImage(srcCards);
    bigDisplay->mOffset.Hook(&bigSync);
    bigDisplay->mOffset.setOffset(-BOOSTER_SLOTS);
    bigDisplay->setX(385);
    bigDisplay->setY(135);
  }

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

  taskList = NULL;
  packlist = NEW MTGPacks();
  packlist->loadAll();
  load();
}

string GameStateShop::descPurchase(int controlId, bool tiny){
   char buffer[4096];
   string name;
   if(controlId < BOOSTER_SLOTS){
     name = mBooster[controlId].getName();
   }
   else{
     MTGCard * c = srcCards->getCard(controlId-BOOSTER_SLOTS);
     if(!c)
       return "";   
     name = _(c->data->getName());
   }
   if(mInventory[controlId] <= 0){
     if(tiny)
       sprintf(buffer,_("SOLD OUT").c_str(),name.c_str());
     else
       sprintf(buffer,_("%s : SOLD OUT").c_str(),name.c_str());
     return buffer;
   }

   if(tiny){
     if(controlId < BOOSTER_SLOTS || mCounts[controlId] == 0)
      return name;
     
     sprintf(buffer,_("%s (%i)").c_str(),name.c_str(),mCounts[controlId]);
     return buffer;
   }
   switch(options[Options::ECON_DIFFICULTY].number){
     case Constants::ECON_HARD:
     case Constants::ECON_NORMAL:
       if(mCounts[controlId] < 1)
         sprintf(buffer,_("%s").c_str(),name.c_str());
       else
         sprintf(buffer,_("%s (%i)").c_str(),name.c_str(),mCounts[controlId]);
       break;
     default:
       if(mCounts[controlId] < 1)
         sprintf(buffer,_("%s : %i credits").c_str(),name.c_str(),mPrices[controlId]);
       else
         sprintf(buffer,_("%s (%i) : %i credits").c_str(),name.c_str(),mCounts[controlId],mPrices[controlId]);
       break;
   }
   return buffer;
}
void GameStateShop::beginPurchase(int controlId){
  JLBFont * mFont = resources.GetJLBFont(Constants::MENU_FONT);
  mFont->SetScale(DEFAULT_MENU_FONT_SCALE);
  SAFE_DELETE(menu);
  if(mInventory[controlId] <= 0){
    menu = NEW SimpleMenu(-145,this,Constants::MENU_FONT,SCREEN_WIDTH-300,SCREEN_HEIGHT/2,_("Sold Out").c_str());
    menu->Add(-1,"Ok");
  }
  else if(playerdata->credits - mPrices[controlId] < 0){
    menu = NEW SimpleMenu(-145,this,Constants::MENU_FONT,SCREEN_WIDTH-300,SCREEN_HEIGHT/2,_("Not enough credits").c_str());
    menu->Add(-1,"Ok");
    if(options[Options::CHEATMODE].number) {
      menu->Add(-2,"Steal it");
    }
  }
  else{
    char buf[512];
    if(controlId < BOOSTER_SLOTS)
      sprintf(buf,_("Purchase Booster: %i credits").c_str(),mPrices[controlId]);
    else
      sprintf(buf,_("Purchase Card: %i credits").c_str(),mPrices[controlId]);
    menu = NEW SimpleMenu(-145,this,Constants::MENU_FONT,SCREEN_WIDTH-300,SCREEN_HEIGHT/2,buf);
    
    menu->Add(controlId,"Yes");
    menu->Add(-1,"No");
  }
}
void GameStateShop::cancelCard(int controlId){
  //Update prices
  MTGCard * c = srcCards->getCard(controlId-BOOSTER_SLOTS);
  if(!c || !c->data || playerdata->credits - mPrices[controlId] < 0) 
    return; //We only care about their opinion if they /can/ buy it.

  int price = mPrices[controlId];
  int rnd;
  switch(options[Options::ECON_DIFFICULTY].number){
    case Constants::ECON_HARD:
      rnd = rand() % 10; break;
    case Constants::ECON_EASY:
      rnd = rand() % 50; break;
    default: 
      rnd = rand() % 25; break;
  }  
  price = price - (rnd * price)/100;
  if (price < pricelist->getPrice(c->getMTGId())) //filters have a tendancy to increase the price instead of lowering it!
    pricelist->setPrice(c->getMTGId(),price);
  //Prices do not immediately go down when you ignore something.
  return;
}
void GameStateShop::cancelBooster(int controlId){
  return; //TODO FIXME Tie boosters into pricelist.
}
void GameStateShop::purchaseCard(int controlId){
  MTGCard * c = srcCards->getCard(controlId-BOOSTER_SLOTS);
  if(!c || !c->data || playerdata->credits - mPrices[controlId] < 0)
    return;
  myCollection->Add(c);
  int price = mPrices[controlId];
  pricelist->setPrice(c->getMTGId(),price); // In case they changed their minds after cancelling.
  playerdata->credits -= price;
  //Update prices
  int rnd;
  switch(options[Options::ECON_DIFFICULTY].number){
    case Constants::ECON_HARD:
      rnd = rand() % 50; break;
    case Constants::ECON_EASY:
      rnd = rand() % 10; break;
    default: 
      rnd = rand() % 25; break;
  }  
  price = price + (rnd * price)/100;
  pricelist->setPrice(c->getMTGId(),price);
  mPrices[controlId] = pricelist->getPurchasePrice(c->getMTGId()); //Prices go up immediately.
  mInventory[controlId]--; 
  updateCounts();
  mTouched = true;
  menu->Close();
}
void GameStateShop::purchaseBooster(int controlId){
  if(playerdata->credits - mPrices[controlId] < 0)
    return;
  playerdata->credits -= mPrices[controlId];
  mInventory[controlId]--;
  SAFE_DELETE(booster);
  deleteDisplay();
  booster = NEW MTGDeck(mParent->collection);
  boosterDisplay = NEW CardDisplay(12,NULL, SCREEN_WIDTH - 200, SCREEN_HEIGHT/2,this,NULL,5);
  mBooster[controlId].addToDeck(booster,srcCards);
  
  string sort = mBooster[controlId].getSort();
  DeckDataWrapper * ddw = NEW DeckDataWrapper(booster);
  if(sort == "alpha")
    ddw->Sort(WSrcCards::SORT_ALPHA);
  else if(sort == "collector")
    ddw->Sort(WSrcCards::SORT_COLLECTOR);
  else
    ddw->Sort(WSrcCards::SORT_RARITY);
  for (int x=0;x<ddw->Size();x++){
    MTGCard * c = ddw->getCard(x);
    MTGCardInstance * ci = NEW MTGCardInstance(c, NULL);
    boosterDisplay->AddCard(ci);
    subBooster.push_back(ci);
  }
  SAFE_DELETE(ddw);

  myCollection->loadMatches(booster);
  mTouched = true;
  save(true);
  menu->Close();
}

int GameStateShop::purchasePrice(int offset){
  MTGCard * c = NULL;
  if(!pricelist || !srcCards || (c = srcCards->getCard(offset)) == NULL)
    return 0;
  float price = (float) pricelist->getPurchasePrice(c->getMTGId());
  float filteradd = srcCards->Size(true); 
  filteradd = ((filteradd - srcCards->Size())/filteradd);

  switch(options[Options::ECON_DIFFICULTY].number){
    case Constants::ECON_EASY:    filteradd /= 2; break;
    case Constants::ECON_HARD:    filteradd *= 2; break;
    default:                      break;    
  }
  return (int) price + price * (filteradd*srcCards->filterFee());
}
void GameStateShop::updateCounts(){
  for(int i=BOOSTER_SLOTS;i<SHOP_ITEMS;i++){
    MTGCard * c = srcCards->getCard(i-BOOSTER_SLOTS);
    if(!c)    mCounts[i] = 0;
    else      mCounts[i] = myCollection->countByName(c);
  }
}
void GameStateShop::load(){
  int nbsets = 0;
  int nbboostersets = 0;

  for(int i=0;i<BOOSTER_SLOTS;i++){
    mBooster[i].randomize(packlist);
    mInventory[i] = 1+rand()%mBooster[i].maxInventory();
    mPrices[i] = pricelist->getOtherPrice(mBooster[i].basePrice()); 
  }
  for(int i=BOOSTER_SLOTS;i<SHOP_ITEMS;i++){
    MTGCard * c = NULL;
    if((c = srcCards->getCard(i-BOOSTER_SLOTS)) == NULL){
      mPrices[i] = 0;
      mCounts[i] = 0;
      mInventory[i] = 0;
      continue;
    }
    mPrices[i] = purchasePrice(i-BOOSTER_SLOTS);
    mCounts[i] = myCollection->countByName(c);
    switch(c->getRarity()){ 
      case Constants::RARITY_C:
        mInventory[i] = 2 + rand() % 8;
        break;
      case Constants::RARITY_L:
        mInventory[i] = 100;
        break;
      default: //We're using some non-coded rarities (S) in cards.dat.
      case Constants::RARITY_U:
        mInventory[i] = 1 + rand() % 5;
        break;
      case Constants::RARITY_R:
        mInventory[i] = 1 + rand() % 2;
        break;
    }

  }
}
void GameStateShop::save(bool force)
{
  if(mTouched || force){
    if(myCollection)
      myCollection->Rebuild(playerdata->collection);
    if(playerdata)
      playerdata->save();
    if(pricelist)
      pricelist->save();
  }  
  mTouched = false;
}
void GameStateShop::End()
{
  save();
  JRenderer::GetInstance()->EnableVSync(false);
  resources.Release(mBgTex);
  mBgTex = NULL;
  mBg = NULL;
  mElapsed = 0; 
  SAFE_DELETE(shopMenu);
  SAFE_DELETE(bigDisplay);
  SAFE_DELETE(srcCards);
  SAFE_DELETE(playerdata);
  SAFE_DELETE(pricelist);
  SAFE_DELETE(myCollection);
  SAFE_DELETE(booster);
  SAFE_DELETE(filterMenu);
  SAFE_DELETE(packlist);
  deleteDisplay();

  //Release alternate thumbnails.
  for(int i=0;i<8;i++){
    resources.Release(altThumb[i]);
    altThumb[i] = NULL;
  }

  SAFE_DELETE(menu);
  SAFE_DELETE(taskList);
}

void GameStateShop::Destroy(){
}
void GameStateShop::beginFilters(){
  if(!filterMenu){
    filterMenu = NEW WGuiFilters("Ask about...",srcCards);
    filterMenu->setY(2);
    filterMenu->setHeight(SCREEN_HEIGHT-2);      
  }
  mStage = STAGE_ASK_ABOUT;
  filterMenu->Entering(JGE_BTN_NONE);
}
void GameStateShop::Update(float dt)
{
  if (menu && menu->closed)
    SAFE_DELETE(menu);
  srcCards->Update(dt);
  alphaChange = (500 - (rand() % 1000)) * dt;
  lightAlpha+= alphaChange;
  if (lightAlpha < 0) lightAlpha = 0;
  if (lightAlpha > 50) lightAlpha = 50;
  //  mParent->effect->UpdateSmall(dt);
  //  mParent->effect->UpdateBig(dt);
  if (mStage != STAGE_FADE_IN)
    mElapsed += dt;

  JButton btn;
  switch(mStage){
    case STAGE_SHOP_PURCHASE:
      if (menu)
        menu->Update(dt);
      beginPurchase(mBuying);
      mStage = STAGE_SHOP_SHOP;
      break;
    case STAGE_SHOP_MENU:
    if (menu)
      menu->Update(dt);
    else{
      menu = NEW SimpleMenu(11,this,Constants::MENU_FONT,SCREEN_WIDTH/2-100,20);
      menu->Add(22,"Ask about...");
      menu->Add(14,"Check task board");
      if (options[Options::CHEATMODE].number)
        menu->Add(-2,"Steal 1,000 credits");
      menu->Add(12,"Save & Back to Main Menu");
      menu->Add(13, "Cancel");
    }
    break;
    case STAGE_SHOP_TASKS:
      if (menu){
        menu->Update(dt);
        return;
      }
      if (taskList){
      btn = mEngine->ReadButton();
      taskList->Update(dt);
        if ( taskList->getState() != TaskList::TASKS_INACTIVE){
          if ( btn == JGE_BTN_SEC || btn == JGE_BTN_CANCEL || btn == JGE_BTN_PREV ){
             taskList->End();
             return;
          } else if (taskList->getState() == TaskList::TASKS_ACTIVE && btn == JGE_BTN_MENU){
            if (!menu) {
              menu = NEW SimpleMenu(11,this,Constants::MENU_FONT,SCREEN_WIDTH/2-100,20);
              menu->Add(15,"Return to shop");
              menu->Add(12,"Save & Back to Main Menu");
              menu->Add(13, "Cancel");
            }
          }
        }
        else
          mStage = STAGE_SHOP_SHOP;
      }

#ifdef TESTSUITE
      if ((mEngine->GetButtonClick(JGE_BTN_PRI)) && (taskList)) {
        taskList->passOneDay();
        if (taskList->getTaskCount() < 6) {
          taskList->addRandomTask();
          taskList->addRandomTask();
        }
       taskList->save();
      }
#endif
    break;
    case STAGE_ASK_ABOUT:
      btn = mEngine->ReadButton();
      if (menu && !menu->closed){
        menu->CheckUserInput(btn);
        menu->Update(dt);
        return;
      }
      if (filterMenu){
        if (btn == JGE_BTN_CTRL) {
          needLoad = filterMenu->Finish();
          filterMenu->Update(dt);
          return;
        }
        if (filterMenu->isFinished()){
          if (needLoad){
            srcCards->addFilter(NEW WCFilterNOT(NEW WCFilterRarity("T")));
            if(!srcCards->Size()){
              srcCards->clearFilters(); //Repetition of check at end of filterMenu->Finish(), for the token removal
              srcCards->addFilter(NEW WCFilterNOT(NEW WCFilterRarity("T")));
            }
            load();
          }
          mStage = STAGE_SHOP_SHOP;
        }else{
          filterMenu->CheckUserInput(btn);
          filterMenu->Update(dt);
        }
        return;
      }
      break;
    case STAGE_SHOP_SHOP:
      btn = mEngine->ReadButton();
      if (menu && !menu->closed){
        menu->CheckUserInput(btn);
        menu->Update(dt);
        return;
      }
      if (btn == JGE_BTN_MENU){
        if (boosterDisplay){
          deleteDisplay();
          return;
        }
        mStage = STAGE_SHOP_MENU;
        return;
      } else if (btn == JGE_BTN_CTRL)
        beginFilters();
      else if(btn == JGE_BTN_NEXT){
        mStage = STAGE_SHOP_TASKS;
        if (!taskList)
          taskList = NEW TaskList();
        taskList->Start();    
      }
      else if (btn == JGE_BTN_PRI) {
        srcCards->Shuffle();
        load();
      } else if (btn == JGE_BTN_CANCEL)
        options[Options::DISABLECARDS].number = !options[Options::DISABLECARDS].number;
      else if (boosterDisplay){
        if (btn == JGE_BTN_SEC)
          deleteDisplay();
        else {
          boosterDisplay->CheckUserInput(btn);
          boosterDisplay->Update(dt);}
        return;
      }else if (btn == JGE_BTN_SEC)
        bListCards = !bListCards;
      else if (shopMenu){
        if (shopMenu->CheckUserInput(btn))
          srcCards->Touch();
      }
      if (shopMenu)
        shopMenu->Update(dt);

      break;
    case STAGE_FADE_IN:
      mParent->DoAnimation(TRANSITION_FADE_IN);
      mStage = STAGE_SHOP_SHOP;
      break;
  }
}

void GameStateShop::deleteDisplay(){
  vector<MTGCardInstance*>::iterator i;
  for(i=subBooster.begin();i!=subBooster.end();i++){
    if (!*i) continue;
    delete *i;
  }
  subBooster.clear();
  SAFE_DELETE(boosterDisplay);
}
void GameStateShop::Render()
{
  //Erase
  JLBFont * mFont = resources.GetJLBFont(Constants::MAIN_FONT);
  JRenderer * r = JRenderer::GetInstance();
  r->ClearScreen(ARGB(0,0,0,0));
  if(mStage == STAGE_FADE_IN)
    return;

  if (mBg)
    r->RenderQuad(mBg,0,0);

  JQuad * quad = resources.RetrieveTempQuad("shop_light.jpg",TEXTURE_SUB_5551); 
  if (quad){
    r->EnableTextureFilter(false);
    r->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
    quad->SetColor(ARGB(lightAlpha,255,255,255));
    r->RenderQuad(quad,0,0);
    r->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    r->EnableTextureFilter(true);
  }
  
  if(shopMenu)
    shopMenu->Render();
  if(filterMenu && !filterMenu->isFinished())
    filterMenu->Render();
  else{
    if(boosterDisplay)
      boosterDisplay->Render();
    else if(bigDisplay){
      if(bigDisplay->mOffset.getPos() >= 0)
        bigDisplay->setSource(srcCards);
      else
        bigDisplay->setSource(NULL);
      bigDisplay->Render();
      float elp = srcCards->getElapsed();
      //Render the card list overlay.
      if( bListCards || elp > LIST_FADEIN){
        char alpha = 200;
        if(!bListCards && elp < LIST_FADEIN+.25){
          alpha = 800 *(elp-LIST_FADEIN);
        }
        r->FillRoundRect(300,10, 160, SHOP_SLOTS * 20 + 15,5,ARGB(alpha,0,0,0));
        alpha+=55;
        for(int i=0;i<SHOP_SLOTS;i++){
          if (i == shopMenu->getSelected())
            mFont->SetColor(ARGB(alpha,255,255,0));
          else  
            mFont->SetColor(ARGB(alpha,255,255,255));
          char buffer[512];
          string s = descPurchase(i,true);
          sprintf(buffer, "%s", s.c_str());
          float x = 310;
          float y = 25 + 20*i;
          mFont->DrawString(buffer,x,y);
        }
      }
    }
  }

  //Render the info bar
  r->FillRect(0,SCREEN_HEIGHT-17,SCREEN_WIDTH,17,ARGB(128,0,0,0));
  char c[512];
  sprintf(c,_("credits: %i").c_str(), playerdata->credits);
  mFont->SetColor(ARGB(255,255,255,255));
  mFont->DrawString(c, 5, SCREEN_HEIGHT - 12);
  sprintf(c, "%s", _("[]:other cards").c_str());
  unsigned int len = 4 + mFont->GetStringWidth(c);
  mFont->DrawString(c,SCREEN_WIDTH-len,SCREEN_HEIGHT-14);

  mFont->SetColor(ARGB(255,255,255,0));
  mFont->DrawString(descPurchase(bigSync.getPos()).c_str(),  SCREEN_WIDTH/2,  SCREEN_HEIGHT - 14,JGETEXT_CENTER);
  mFont->SetColor(ARGB(255,255,255,255));

  if (mStage == STAGE_SHOP_TASKS && taskList) {
    taskList->Render();
  }
  if (menu)
    menu->Render();
}

void GameStateShop::ButtonPressed(int controllerId, int controlId)
{
  int sel = bigSync.getOffset();

  switch(controllerId){
    case -102: //Buying something...
      mStage = STAGE_SHOP_PURCHASE;
      if(menu)
        menu->Close();
      mBuying = controlId;
      return;
    case -145: 
      if(controlId == -1){ //Nope, don't buy.
        if(sel < BOOSTER_SLOTS)
          cancelBooster(sel);
        else
          cancelCard(sel);
        menu->Close();
        mStage = STAGE_SHOP_SHOP;
        return;
      }
      if(sel > -1 && sel < SHOP_ITEMS){        
        if(controlId == -2)
          playerdata->credits += mPrices[sel]; //We stole it.
        if(sel < BOOSTER_SLOTS) //Clicked a booster.
          purchaseBooster(sel);
        else
          purchaseCard(sel);
      }
      mStage = STAGE_SHOP_SHOP;
      return;    
  }
  //Basic Menu.
  switch(controlId){
  case 12:
    if (taskList) taskList->save();
    mStage = STAGE_SHOP_SHOP;
    mParent->DoTransition(TRANSITION_FADE,GAME_STATE_MENU);
    save();
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
  case 22:
    beginFilters();
    break;
  case -2:
    playerdata->credits += 1000;
  default:
    mStage = STAGE_SHOP_SHOP;
  }
  menu->Close();
}

//ShopBooster
ShopBooster::ShopBooster(){
  pack = NULL;
  mainSet = NULL;
  altSet = NULL;
}
string ShopBooster::getSort() {
  if(pack) 
    return pack->getSort(); 
  return "";
};
string ShopBooster::getName(){
  char buffer[512];
  if(!mainSet && pack) return pack->getName();
  if(altSet == mainSet) altSet = NULL;
  if(altSet)
    sprintf(buffer,_("%s & %s (15 Cards)").c_str(),mainSet->id.c_str(),altSet->id.c_str());
  else if(mainSet)
    sprintf(buffer,_("%s Booster (15 Cards)").c_str(),mainSet->id.c_str());
  return buffer;
}

void ShopBooster::randomize(MTGPacks * packlist){
  mainSet = NULL;  altSet = NULL;  pack = NULL;
  if(!setlist.size()) return;
  if(packlist && setlist.size() > 10){  //FIXME make these an unlockable item.
    int rnd = rand() % 100;
    if(rnd <= Constants::CHANCE_CUSTOM_PACK){  
      randomCustom(packlist);
      return;
    }
  }
  randomStandard();
}
int ShopBooster::basePrice(){
  if(pack)
    return pack->getPrice();
  else if(altSet)
    return Constants::PRICE_MIXED_BOOSTER;
  return Constants::PRICE_BOOSTER;
}
void ShopBooster::randomCustom(MTGPacks * packlist){
  pack = packlist->randomPack();
  if(pack && !pack->isUnlocked())
    pack = NULL;
  if(!pack)
    randomStandard();
}
void ShopBooster::randomStandard(){
  int mSet = -1;
  MTGSetInfo * si = setlist.randomSet(-1);
  mainSet = si;
  altSet = NULL;

  int mSetCount = si->counts[MTGSetInfo::TOTAL_CARDS];
  if(mSetCount < 80){
    if(rand() % 100 < Constants::CHANCE_PURE_OVERRIDE){ //Chance of picking a pure pack instead.
      si = setlist.randomSet(-1,80);
      mSetCount = si->counts[MTGSetInfo::TOTAL_CARDS];
      mainSet = si;
    }else
      altSet = setlist.randomSet(si->block,80-mSetCount);
  }
  else if(rand() % 100 < Constants::CHANCE_MIXED_OVERRIDE) //Chance of having a mixed booster anyways.
    altSet = setlist.randomSet(si->block);      

  for(int attempts=0;attempts<10;attempts++){ //Try to prevent altSet == mainSet.
    if(altSet != mainSet) break;
    altSet = setlist.randomSet(-1,80-mSetCount);
  }
  if(altSet == mainSet) altSet = NULL; //Prevent "10E & 10E Booster"
  if(!altSet) pack = mainSet->mPack;
  
}
int ShopBooster::maxInventory(){
  if(altSet || pack)
    return 2;
  return 5;
}
void ShopBooster::addToDeck(MTGDeck * d, WSrcCards * srcCards){
  if(!pack){ //A combination booster.
    MTGPack * mP = MTGPacks::getDefault();
    if(!altSet && mainSet->mPack) mP = mainSet->mPack;
    char buf[512];
    if(!altSet) sprintf(buf,"set:%s;",mainSet->id.c_str());
    else sprintf(buf,"set:%s;|set:%s;",mainSet->id.c_str(),altSet->id.c_str());
    mP->pool = buf;
    mP->assemblePack(d); //Use the primary packfile. assemblePack deletes pool.
  }
  else
    pack->assemblePack(d);
}
