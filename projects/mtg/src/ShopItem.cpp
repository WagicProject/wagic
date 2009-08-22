#include "../include/config.h"
#include "../include/ShopItem.h"
#include "../include/GameStateShop.h"
#include "../include/CardGui.h"
#include "../include/Translate.h"
#include <hge/hgedistort.h>



  float ShopItems::_x1[] = { 40,  3, 23, 99,142,182, 90,132,177,106,163};
  float ShopItems::_y1[] = {156,174,194,166,166,162,184,185,180,211,208};

  float ShopItems::_x2[] = { 44, 25, 64,128,171,211,121,165,209,143,200};
  float ShopItems::_y2[] = {147,163,190,166,166,162,184,185,180,211,208};

  float ShopItems::_x3[] = { 86, 47, 12, 85,133,177, 73,120,170, 88,153};
  float ShopItems::_y3[] = {152,177,216,181,180,176,203,204,198,237,232};

  float ShopItems::_x4[] = { 86, 66, 58,118,164,207,108,156,205,130,199};
  float ShopItems::_y4[] = {145,167,211,181,180,176,203,204,198,237,232};

ShopItem::ShopItem(int id, JLBFont *font, char* text, JQuad * _quad,JQuad * _thumb,  float _xy[], bool hasFocus, int _price): JGuiObject(id), mFont(font), mText(text), quad(_quad), thumb(_thumb), price(_price)
{
  for (int i = 0; i < 8; ++i){
    xy[i] = _xy[i];
  }
  quantity = 10;
  card = NULL;
  mHasFocus = hasFocus;

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
  mScale = 1.0f;
  mTargetScale = 1.0f;

  if (hasFocus)
    Entering();

  card = collection->getCardById(_cardid);
  updateCount(ddw);

  quantity = 1 + (rand() % 4);
  if (card->getRarity() == Constants::RARITY_L) quantity = 50;
  quad = NULL;
  thumb = card->getThumb();
  if (!thumb) thumb = GameApp::CommonRes->GetQuad("back_thumb");
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
  //float x0 = mX;
  //float y0 = mY - (mScale > 1 ? 4 : 0);
 /* if (GetId()%2){
    float xs[] = {mX,   mX,   mX+230,mX+230};
    float ys[] = {mY-5+17,mY-5+19,mY-5+35,mY-5}    ;

    renderer->FillPolygon(xs,ys,4,ARGB(200,0,0,0));
    x0 = mX + 230 -30;
    mFont->DrawString(mText.c_str(), x0, mY + 8,JGETEXT_RIGHT);
    
  }else{
    float xs[] = {mX-5,   mX-5,   mX-5+230,mX-5+230,};
    float ys[] = {mY-5,mY-5+35,mY-5+17,mY-5+19}    ;
    renderer->FillPolygon(xs,ys,4,ARGB(128,0,0,0));
     mFont->DrawString(mText.c_str(), mX + 30, mY + 8);
  }*/
  //renderer->FillRect(mX-5, mY-5,230,35, );


  if (mesh){
    mesh->Render(0,0);
    //renderer->RenderQuad(thumb,x0,y0,0,mScale * 0.45,mScale * 0.45);
  }else{
    //NOTHING
  }
  if (mHasFocus){
    if (card){
      quad = card->getQuad();
    }
    if (quad){
      quad->SetColor(ARGB(255,255,255,255));
      renderer->RenderQuad(quad,SCREEN_WIDTH/2 + 50,5,0, 0.9f,0.9f);
    }else{
      if (card) CardGui::alternateRender(card,NULL,SCREEN_WIDTH/2 + 100 + 20,133,0, 0.9f);
    }
    mFont->DrawString(mText.c_str(),  100,  SCREEN_HEIGHT - 30);
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
  myCollection = 	 NEW DeckDataWrapper(NEW MTGDeck(RESPATH"/player/collection.dat", NULL,_collection));
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
	      dialog = NEW SimpleMenu(1,this,GameApp::CommonRes->GetJLBFont(Constants::MENU_FONT),SCREEN_WIDTH-300,SCREEN_HEIGHT/2,buffer);
	      dialog->Add(1,"Yes");
	      dialog->Add(2,"No");
      }
      else{
	dialog->Update(dt);
      }
    }else{
      SAFE_DELETE(dialog);
      JGuiController::Update(dt);
    }
  }

}


void ShopItems::Render(){
  JGuiController::Render();
  if (showPriceDialog==-1){

  }else{
    if(dialog){
      dialog->Render();
    }
  }
  char credits[512];
  sprintf(credits,_("credits: %i").c_str(), playerdata->credits);
  unsigned int len = 4 + mFont->GetStringWidth(credits);
  mFont->SetColor(ARGB(200,0,0,0));
  mFont->DrawString(credits, 5, SCREEN_HEIGHT - 13);
  mFont->SetColor(ARGB(255,255,255,255));
  mFont->DrawString(credits, 5, SCREEN_HEIGHT - 15);
  if (display) display->Render();
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

        MTGDeck * tempDeck = NEW MTGDeck(NULL,playerdata->collection->database);
        int rare_or_mythic = Constants::RARITY_R;
        int rnd = rand() % 8;
        if (rnd == 0) rare_or_mythic = Constants::RARITY_M;
        int sets[] = {setIds[showPriceDialog]};

        tempDeck->addRandomCards(1, sets,1,rare_or_mythic);
        tempDeck->addRandomCards(3, sets,1,Constants::RARITY_U);
        tempDeck->addRandomCards(11, sets,1,Constants::RARITY_C);
        
        playerdata->collection->add(tempDeck);
        myCollection->Add(tempDeck);

        for (int j = 0; j < mCount; j++){
          ShopItem * si =  ((ShopItem *)mObjects[j]);
          si->updateCount(myCollection);
        }
       
        int i = 0;
        for (map<int,int>::iterator it = tempDeck->cards.begin(); it!=tempDeck->cards.end(); it++){
          MTGCard * c = tempDeck->getCardById(it->first);
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
