#include "../include/config.h"
#include "../include/ShopItem.h"
#include "../include/GameStateShop.h"
#include "../include/CardGui.h"
#include "../include/WResourceManager.h"
#include "../include/Translate.h"
#include <hge/hgedistort.h>



  float ShopItems::_x1[] = { 79, 19, 27,103,154,187,102,144,198,133,183};
  float ShopItems::_y1[] = {150,194,222,167,164,156,195,190,175,220,220};

  float ShopItems::_x2[] = {103, 48, 74,135,183,215,138,181,231,171,225};
  float ShopItems::_y2[] = {155,179,218,165,166,155,195,186,177,225,216};

  float ShopItems::_x3[] = { 48, 61, 9, 96,139,190, 81,146,187, 97,191};
  float ShopItems::_y3[] = {164,205,257,184,180,170,219,212,195,251,252};

  float ShopItems::_x4[] = { 76, 90, 65,131,171,221,123,187,225,141,237};
  float ShopItems::_y4[] = {169,188,250,182,182,168,220,208,198,259,245};


ShopItem::ShopItem(int id, JLBFont *font, char* text, JQuad * _quad,JQuad * _thumb,  float _xy[], bool hasFocus, int _price): JGuiObject(id), mFont(font), mText(text), quad(_quad), thumb(_thumb), price(_price)
{
  for (int i = 0; i < 8; ++i){
    xy[i] = _xy[i];
  }
  quantity = 10;
  card = NULL;
  mHasFocus = hasFocus;
  mRelease = false;

  mScale = 1.0f;
  mTargetScale = 1.0f;

 mesh=NEW hgeDistortionMesh(2,2);
 mesh->SetTexture(thumb->mTex);
 float x0,y0,w0,h0;
 thumb->GetTextureRect(&x0,&y0,&w0,&h0);
 mesh->SetTextureRect(x0,y0,w0,h0);
 mesh->Clear(ARGB(0xFF,0xFF,0xFF,0xFF));
 mesh->SetDisplacement(0, 0, xy[0],xy[1], HGEDISP_NODE);
 mesh->SetDisplacement(1, 0, xy[2] - w0,xy[3], HGEDISP_NODE);
 mesh->SetDisplacement(0, 1,xy[4],xy[5]-h0, HGEDISP_NODE);
 mesh->SetDisplacement(1, 1, xy[6]-w0,xy[7]-h0, HGEDISP_NODE);
     mesh->SetColor(1,1,ARGB(255,100,100,100));
     mesh->SetColor(0,1,ARGB(255,100,100,100));
     mesh->SetColor(1,0,ARGB(255,100,100,100));
     mesh->SetColor(0,0,ARGB(255,200,200,200));
  if (hasFocus)
    Entering();
}

ShopItem::ShopItem(int id, JLBFont *font, int _cardid, float _xy[], bool hasFocus, MTGAllCards * collection, int _price, DeckDataWrapper * ddw): JGuiObject(id), mFont(font),  price(_price){
  for (int i = 0; i < 8; ++i){
    xy[i] = _xy[i];
  }
  mHasFocus = hasFocus;
  mRelease = false;
  mScale = 1.0f;
  mTargetScale = 1.0f;

  if (hasFocus)
    Entering();

  card = collection->getCardById(_cardid);
  updateCount(ddw);

  quantity = 1 + (rand() % 4);
  if (card->getRarity() == Constants::RARITY_L) quantity = 50;
  quad = NULL;

  thumb = resources.RetrieveCard(card,RETRIEVE_LOCK,TEXTURE_SUB_THUMB);

  if (!thumb)
    thumb = CardGui::alternateThumbQuad(card);
  else
    mRelease = true;

  if (thumb){
     mesh=NEW hgeDistortionMesh(2,2);
     mesh->SetTexture(thumb->mTex);
     float x0,y0,w0,h0;
     thumb->GetTextureRect(&x0,&y0,&w0,&h0);
     mesh->SetTextureRect(x0,y0,w0,h0);
     mesh->Clear(ARGB(0xFF,0xFF,0xFF,0xFF));
     mesh->SetDisplacement(0, 0, xy[0],xy[1], HGEDISP_NODE);
     mesh->SetDisplacement(1, 0, xy[2] - w0,xy[3], HGEDISP_NODE);
     mesh->SetDisplacement(0, 1,xy[4],xy[5]-h0, HGEDISP_NODE);
     mesh->SetDisplacement(1, 1, xy[6]-w0,xy[7]-h0, HGEDISP_NODE);
     mesh->SetColor(1,1,ARGB(255,100,100,100));
     mesh->SetColor(0,1,ARGB(255,100,100,100));
     mesh->SetColor(1,0,ARGB(255,100,100,100));
     mesh->SetColor(0,0,ARGB(255,200,200,200));
  }else{
    mesh = NULL;
  }
}


int ShopItem::updateCount(DeckDataWrapper * ddw){
  if (!card) return 0;
  nameCount = ddw->countByName(card);
  return nameCount;
}

ShopItem::~ShopItem(){
  OutputDebugString("delete shopitem\n");
  if(thumb)
    resources.Release(thumb->mTex);
  SAFE_DELETE(mesh);
}

const char * ShopItem::getText(){
  return mText.c_str();
}


void ShopItem::Render(){
  if (mHasFocus){
    mFont->SetColor(ARGB(255,255,255,0));
  }else{
    mFont->SetColor(ARGB(255,255,255,255));
  }
  if (!quantity){
    mFont->SetColor(ARGB(255,128,128,128));
  }

  if (card){
    if (nameCount){
      char buffer[512];
      sprintf(buffer, "%s (%i)", _(card->name).c_str(), nameCount );
      mText = buffer;
    }else{
      mText = _(card->name).c_str();
    }
  }

  JRenderer * renderer = JRenderer::GetInstance();

  if (mesh){
    mesh->Render(0,0);
  }else{
    //ERROR Management
  }
  if (mHasFocus){
    if (card) quad = resources.RetrieveCard(card);
    if (quad){
      quad->SetColor(ARGB(255,255,255,255));
      renderer->RenderQuad(quad,SCREEN_WIDTH - 105,SCREEN_HEIGHT/2 - 5,0, 0.9f,0.9f);
    }else{
      if (card) CardGui::alternateRender(card,Pos(SCREEN_WIDTH - 105,SCREEN_HEIGHT/2 - 5,0.9f* 285/250, 0,255));

    }
  }
}

void ShopItem::Update(float dt)
{
  if (mScale < mTargetScale){
      mScale += 8.0f*dt;
      if (mScale > mTargetScale)
	      mScale = mTargetScale;
   }else if (mScale > mTargetScale){
      mScale -= 8.0f*dt;
      if (mScale < mTargetScale)
	      mScale = mTargetScale;
   }
}




void ShopItem::Entering()
{
  for (int i = 0; i < 2; ++i){
    for (int j = 0; j < 2; ++j){
      mesh->SetColor(i,j,ARGB(255,255,255,255));
    }
  }


  mHasFocus = true;
  mTargetScale = 1.2f;
}


bool ShopItem::Leaving(u32 key)
{
 mesh->SetColor(1,1,ARGB(255,100,100,100));
 mesh->SetColor(0,1,ARGB(255,100,100,100));
 mesh->SetColor(1,0,ARGB(255,100,100,100));
 mesh->SetColor(0,0,ARGB(255,200,200,200));

  mHasFocus = false;
  mTargetScale = 1.0f;
  return true;
}


bool ShopItem::ButtonPressed()
{
  return (quantity >0);
}


ShopItems::ShopItems(int id, JGuiListener* listener, JLBFont* font, int x, int y, MTGAllCards * _collection, int _setIds[]): JGuiController(id, listener), mX(x), mY(y), mFont(font), collection(_collection){
  mHeight = 0;
  showPriceDialog = -1;
  dialog = NULL;
  pricelist = NEW PriceList(RESPATH"/settings/prices.dat",_collection);
  playerdata = NEW PlayerData(_collection);
  display = NULL;
  for (int i=0; i < SHOP_BOOSTERS; i++){
    setIds[i] = _setIds[i];
  };
  myCollection = 	 NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(PLAYER_COLLECTION).c_str(), _collection));
  showCardList = true;

  mBgAA = NULL;
  mBgAATex = resources.RetrieveTexture("shop_aliasing.png",RETRIEVE_LOCK);
  if(mBgAATex){
    mBgAA = resources.RetrieveQuad("shop_aliasing.png");
    mBgAA->SetTextureRect(0,1,250,119); 
  }
  
  lightAlpha = 0;
  alphaChange = 0;
}



void ShopItems::Add(int cardid){
  int rnd = (rand() % 20);
  int price = pricelist->getPrice(cardid);
  price = price + price * (rnd -10)/100;
  float xy[] = {_x1[mCount],_y1[mCount],_x2[mCount],_y2[mCount],_x3[mCount],_y3[mCount],_x4[mCount],_y4[mCount]};
  JGuiController::Add(NEW ShopItem(mCount, mFont, cardid, xy,  (mCount == 0),collection, price,myCollection));
  mHeight += 22;
}

void ShopItems::Add(char * text, JQuad * quad,JQuad * thumb, int price){
  float xy[] = {_x1[mCount],_y1[mCount],_x2[mCount],_y2[mCount],_x3[mCount],_y3[mCount],_x4[mCount],_y4[mCount]};
  JGuiController::Add(NEW ShopItem(mCount, mFont, text, quad, thumb, xy,  (mCount == 0), price));
  mHeight += 22;
}

void ShopItems::Update(float dt){
  if (display){
    while (u32 key = JGE::GetInstance()->ReadButton()) display->CheckUserInput(key);
    if (display) display->Update(dt);
  }else{
    if (showPriceDialog!=-1){
      ShopItem * item =  ((ShopItem *)mObjects[showPriceDialog]);
      int price = item->price;
      char buffer[4096];
      sprintf(buffer,"%s : %i credits",item->getText(),price);
      if(!dialog){
	      dialog = NEW SimpleMenu(1,this,Constants::MENU_FONT,SCREEN_WIDTH-300,SCREEN_HEIGHT/2,buffer);
	      dialog->Add(1,"Yes");
	      dialog->Add(2,"No");
        if(options[Options::CHEATMODE].number) {
          dialog->Add(3,"*Steal 1,000 credits*");
        }
      }
      else{
	      dialog->Update(dt);
      }
    }else{
      u32 buttons[] = {PSP_CTRL_LEFT,PSP_CTRL_DOWN,PSP_CTRL_RIGHT,PSP_CTRL_UP,PSP_CTRL_CIRCLE};
      for (int i = 0; i < 5; ++i){
        if (JGE::GetInstance()->GetButtonClick(buttons[i])){
          showCardList = false;
        }
      }
      if (JGE::GetInstance()->GetButtonClick(PSP_CTRL_TRIANGLE)){
          showCardList = !showCardList;
      }
      SAFE_DELETE(dialog);
      JGuiController::Update(dt);
    }

  }

  alphaChange = (500 - (rand() % 1000)) * dt;
  lightAlpha+= alphaChange;
  if (lightAlpha < 0) lightAlpha = 0;
  if (lightAlpha > 50) lightAlpha = 50;
}


void ShopItems::Render(){
  JGuiController::Render();
  JRenderer * r = JRenderer::GetInstance(); 
  
  
  if (mBgAA) 
    r->RenderQuad(mBgAA,0,SCREEN_HEIGHT-127);

  JQuad * quad = resources.RetrieveTempQuad("shop_light.jpg",TEXTURE_SUB_5551); 
  if (quad){
    r->EnableTextureFilter(false);
    r->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
    quad->SetColor(ARGB(lightAlpha,255,255,255));
    r->RenderQuad(quad,0,0);
    r->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    r->EnableTextureFilter(true);
  }
  

  if (display) display->Render();

  if (showPriceDialog==-1){

  }else{
    if(dialog){
      dialog->Render();
    }
  }

  mFont->SetColor(ARGB(255,255,255,255));
  char c[4096];
  r->FillRect(0,SCREEN_HEIGHT-17,SCREEN_WIDTH,17,ARGB(128,0,0,0));
  sprintf(c, _("[]:other cards /\\:list").c_str());
  unsigned int len = 4 + mFont->GetStringWidth(c);
  mFont->DrawString(c,SCREEN_WIDTH-len,SCREEN_HEIGHT-14);

  char credits[512];
  sprintf(credits,_("credits: %i").c_str(), playerdata->credits);
  mFont->SetColor(ARGB(200,0,0,0));
  mFont->DrawString(credits, 5, SCREEN_HEIGHT - 12);
  mFont->SetColor(ARGB(255,255,255,255));
  mFont->DrawString(credits, 5, SCREEN_HEIGHT - 14);
  
  if(mCurr >= 0){
    mFont->SetColor(ARGB(255,255,255,0));
    ShopItem * item = ((ShopItem *)mObjects[mCurr]);
    mFont->DrawString(item->mText.c_str(),  SCREEN_WIDTH/2 - 50,  SCREEN_HEIGHT - 14,JGETEXT_CENTER);
    mFont->SetColor(ARGB(255,255,255,255));
  }

  if (showCardList){
    r->FillRoundRect(290,5, 160, mCount * 20 + 15,5,ARGB(200,0,0,0));
   
    for (int i = 0; i< mCount; ++i){
      if (!mObjects[i]) continue;
      ShopItem * s = (ShopItem *)(mObjects[i]);
      if (i == mCurr)  mFont->SetColor(ARGB(255,255,255,0));
      else  mFont->SetColor(ARGB(255,255,255,255));
      char buffer[512];
      sprintf(buffer, "%s", s->getText());
      float x = 300;
      float y = 10 + 20*i;
      mFont->DrawString(buffer,x,y);
    }
  }
}

void ShopItems::pricedialog(int id, int mode){
  if (mode){
    showPriceDialog = id;
  }else{
    showPriceDialog = -1;
  }
}

void ShopItems::ButtonPressed(int controllerId, int controlId){
  if (controllerId == 12){
    safeDeleteDisplay();
    return;
  }

  ShopItem * item =  ((ShopItem *)mObjects[showPriceDialog]);
  int price = item->price;
  switch(controlId){
  case 1:
    if (playerdata->credits >= price){
      playerdata->credits -= price;
      if (item->card){
	    int rnd = (rand() % 25);
	    price = price + (rnd * price)/100;
	    pricelist->setPrice(item->card->getMTGId(),price);
	    playerdata->collection->add(item->card);
	    item->quantity--;
      myCollection->Add(item->card);
      item->nameCount++;
      item->price = price;
    }else{
	      safeDeleteDisplay();
	      display = NEW CardDisplay(12,NULL, SCREEN_WIDTH - 200, SCREEN_HEIGHT/2,this,NULL,5);

        MTGDeck * tempDeck = NEW MTGDeck(playerdata->collection->database);
        int rare_or_mythic = Constants::RARITY_R;
        int rnd = rand() % 8;
        if (rnd == 0) rare_or_mythic = Constants::RARITY_M;
        int sets[] = {setIds[showPriceDialog]};
        MTGSetInfo* si = setlist.getInfo(setIds[showPriceDialog]);

        tempDeck->addRandomCards(si->booster[MTGSetInfo::RARE], sets,1,rare_or_mythic);
        tempDeck->addRandomCards(si->booster[MTGSetInfo::UNCOMMON], sets,1,Constants::RARITY_U);
        tempDeck->addRandomCards(si->booster[MTGSetInfo::COMMON], sets,1,Constants::RARITY_C);
        tempDeck->addRandomCards(si->booster[MTGSetInfo::LAND], sets,1,Constants::RARITY_L);

        //Check for duplicates. Does not guarentee none, just makes them extremely unlikely.
        //Code is kind of inefficient, but shouldn't be used often enough to matter.
        int loops=0;
         for(map<int,int>::iterator it = tempDeck->cards.begin();it!= tempDeck->cards.end() && loops < 15;it++,loops++){
          int dupes = it->second - 1;
          if(dupes <= 0)
            continue;

          for(int x=0;x<dupes;x++)
            tempDeck->remove(it->first);

          int rarity = (int) tempDeck->database->getCardById(it->first)->getRarity();
          tempDeck->addRandomCards(dupes,sets,1,rarity);
          it = tempDeck->cards.begin(); 
        }
   
        playerdata->collection->add(tempDeck);
        myCollection->Add(tempDeck);

        for (int j = 0; j < mCount; j++){
          ShopItem * si =  ((ShopItem *)mObjects[j]);
          si->updateCount(myCollection);
        }

        int i = 0;
        for(int cycle=0;cycle<4;cycle++)
        for (map<int,int>::iterator it = tempDeck->cards.begin(); it!=tempDeck->cards.end(); it++){
          MTGCard * c = tempDeck->getCardById(it->first);
          char rarity = c->getRarity();
          if((cycle == 0 && rarity == Constants::RARITY_L)
            || (cycle == 1 && rarity == Constants::RARITY_C)
            || (cycle == 2 && rarity == Constants::RARITY_U)
            || (cycle == 3 && (rarity == Constants::RARITY_R || rarity == Constants::RARITY_M)))
            for (int j = 0; j < it->second; j++){
              MTGCardInstance * card = NEW MTGCardInstance(c, NULL);
              displayCards[i] = card;
	            display->AddCard(card);
              i++;
            }
        }
        delete tempDeck;
      }
      showPriceDialog = -1;
    }else{
      //error not enough money
    }
    break;
  case 2:
    if (item->card){
      int rnd = (rand() % 25);
      price = price - (rnd * price)/100;
      pricelist->setPrice(item->card->getMTGId(),price);
    }
    showPriceDialog = -1;
    break;
  case 3:  // (PSY) Cheatmode: get free money
    playerdata->credits += 1000;
    break;
  }
}


void ShopItems::safeDeleteDisplay(){
  if (!display) return;
  for (int i = 0; i < display->mCount; i++){
    delete displayCards[i];
  }
  SAFE_DELETE(display);
}

void ShopItems::saveAll(){
  savePriceList();
  playerdata->save();
}

void ShopItems::savePriceList(){
  pricelist->save();
}

ShopItems::~ShopItems(){
  SAFE_DELETE(pricelist);
  SAFE_DELETE(playerdata);
  SAFE_DELETE(dialog);
  safeDeleteDisplay();
  SAFE_DELETE(myCollection);
  resources.Release(mBgAATex);
}

ostream& ShopItem::toString(ostream& out) const
{
  return out << "ShopItem ::: mHasFocus : " << mHasFocus
	     << " ; mFont : " << mFont
	     << " ; mText : " << mText
	     << " ; quad : " << quad
	     << " ; thumb : " << thumb
	     << " ; mScale : " << mScale
	     << " ; mTargetScale : " << mTargetScale
	     << " ; nameCount : " << nameCount
	     << " ; quantity : " << quantity
	     << " ; card : " << card
	     << " ; price : " << price;
}
