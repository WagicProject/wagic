#include <math.h>
#include <iostream>

#include <JGE.h>
#include "../include/config.h"
#include "../include/GameStateDeckViewer.h"
#include "../include/Translate.h"
#include "../include/DeckStats.h"
#include "../include/ManaCostHybrid.h"
#include <vector>

GameStateDeckViewer::GameStateDeckViewer(GameApp* parent): GameState(parent) {
  bgMusic = NULL;
  scrollSpeed = MED_SPEED;
  nbDecks = 0;
  deckNum = 0;
  mSwitching = false;
}

GameStateDeckViewer::~GameStateDeckViewer() {
  SAFE_DELETE(bgMusic);
}


void GameStateDeckViewer::rotateCards(int direction){
  int maxCards=displayed_deck->getCount(colorFilter);
  if (maxCards==0)
    return;
  int left = direction;
  if (left){
    MTGCard * currentCard = displayed_deck->getNext(cardIndex[6],colorFilter);
    for (int i = 1; i<7; i++){
      cardIndex[i-1] = cardIndex[i];
    }
    cardIndex[6] = currentCard;
  }else{
    MTGCard * currentCard = displayed_deck->getPrevious(cardIndex[0],colorFilter);
    for (int i = 5; i>=0; i--){
      cardIndex[i+1] = cardIndex[i];
    }
    cardIndex[0] = currentCard;
  }
}

void GameStateDeckViewer::loadIndexes(MTGCard * current){
  for (int i = 0; i < 7; i++){
    cardIndex[i] = NULL;
  }
  MTGCard * _current = current;
  _current = displayed_deck->getNext(NULL,colorFilter);
  for (int i = 0; i < 7; i++){
    cardIndex[i] = _current;
#if defined (WIN32) || defined (LINUX)
    char buf[4096];
    sprintf(buf,"Loadindexes[%i] is NULL\n", i);
    if(_current) sprintf(buf, "LoadIndexes[%i]  : %s\n", i, _current->getName().c_str());
    OutputDebugString(buf);
#endif
    _current = displayed_deck->getNext(_current,colorFilter);
  }
}

void GameStateDeckViewer::switchDisplay(){
#if defined (WIN32) || defined (LINUX)
  OutputDebugString("Switching display");
#endif
  if (displayed_deck == myCollection){
    displayed_deck = myDeck;
  }else{
    displayed_deck = myCollection;
  }
  currentCard = NULL;
  loadIndexes();
}

void GameStateDeckViewer::updateDecks(){
  SAFE_DELETE(welcome_menu);

  welcome_menu = NEW SimpleMenu(10,this,menuFont,20,20);
  nbDecks = fillDeckMenu(welcome_menu,options.profileFile());
  deckNum = 0;
  newDeckname = "";
  welcome_menu->Add(nbDecks+1, "--NEW--");
  welcome_menu->Add(-1, "Cancel");

}

void GameStateDeckViewer::Start()
{
  newDeckname = "";
  hudAlpha = 0;
  mSwitching = false;
  delSellMenu = 0;
  pricelist = NEW PriceList(RESPATH"/settings/prices.dat",mParent->collection);
  playerdata = NEW PlayerData(mParent->collection);
  sellMenu = NULL;
  myCollection =    NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(PLAYER_COLLECTION).c_str(), mParent->collection));
  displayed_deck =  myCollection;
  myDeck = NULL;
  menuFont = resources.GetJLBFont(Constants::MENU_FONT);
  mFont = resources.GetJLBFont(Constants::MAIN_FONT);

  stw.currentPage = 0;
  stw.pageCount = 5;
  stw.needUpdate = true;

  menu = NEW SimpleMenu(11,this,menuFont,SCREEN_WIDTH/2-100,20);
  menu->Add(0,"Save");
  menu->Add(1,"Save & Rename");
  menu->Add(2,"Switch decks without saving");
  menu->Add(3,"Back to main menu");
  menu->Add(4,"Cancel");


  //icon images
  mIcons[Constants::MTG_COLOR_ARTIFACT] = resources.GetQuad("c_artifact");
  mIcons[Constants::MTG_COLOR_LAND] = resources.GetQuad("c_land");
  mIcons[Constants::MTG_COLOR_WHITE] = resources.GetQuad("c_white");
  mIcons[Constants::MTG_COLOR_RED] = resources.GetQuad("c_red");
  mIcons[Constants::MTG_COLOR_BLACK] = resources.GetQuad("c_black");
  mIcons[Constants::MTG_COLOR_BLUE] = resources.GetQuad("c_blue");
  mIcons[Constants::MTG_COLOR_GREEN] = resources.GetQuad("c_green");
  for (int i=0; i < 7; i++){
    mIcons[i]->SetHotSpot(16,16);
  }

  //Grab a texture in VRAM.
  pspIconsTexture = resources.RetrieveTexture("iconspsp.png", RETRIEVE_LOCK);

  char buf[512];
  for (int i=0; i < 8; i++){
    sprintf(buf,"iconspsp%d",i);
    pspIcons[i] = resources.RetrieveQuad("iconspsp.png", i*32, 0, 32, 32,buf);
    pspIcons[i]->SetHotSpot(16,16);
  }

  backQuad = resources.GetQuad("back");

  //menuFont = NEW JLBFont("graphics/f3",16);
  menuFont = resources.GetJLBFont("f3");
  welcome_menu = NEW SimpleMenu(10,this,menuFont,20,20);
  nbDecks = fillDeckMenu(welcome_menu,options.profileFile());
  deckNum = 0;
  welcome_menu->Add(nbDecks+1, "--NEW--");
  welcome_menu->Add(-1, "Cancel");

  if (GameApp::HasMusic && options[Options::MUSICVOLUME].number > 0){
    if (GameApp::music){
      JSoundSystem::GetInstance()->StopMusic(GameApp::music);
      SAFE_DELETE(GameApp::music);
    }
    GameApp::music = resources.ssLoadMusic("track1.mp3");
    if (GameApp::music){
      JSoundSystem::GetInstance()->PlayMusic(GameApp::music, true);
    }
  }
  colorFilter = ALL_COLORS;

  mStage = STAGE_WELCOME;

  mRotation = 0;
  mSlide = 0;
  mAlpha = 255;
  newDeckname = "";

  currentCard = NULL;
  loadIndexes(currentCard);
  last_user_activity = NO_USER_ACTIVITY_HELP_DELAY + 1;
  onScreenTransition = 0;

  mEngine->ResetInput();
}


void GameStateDeckViewer::End()
{
  //mEngine->EnableVSync(false);
  if (GameApp::music){
    JSoundSystem::GetInstance()->StopMusic(GameApp::music);
    SAFE_DELETE(GameApp::music);
  }
  SAFE_DELETE(welcome_menu);
  SAFE_DELETE(menu);

  resources.Release(pspIconsTexture);
  SAFE_DELETE(myCollection);
  SAFE_DELETE(myDeck);
  SAFE_DELETE(pricelist);
  SAFE_DELETE(playerdata);
}


void GameStateDeckViewer::addRemove(MTGCard * card){
  if (!card) return;
  if (displayed_deck->Remove(card)){
    if (displayed_deck == myCollection){
      myDeck->Add(card);
    }else{
      myCollection->Add(card);
    }
  }
  stw.needUpdate = true;
}

int GameStateDeckViewer::Remove(MTGCard * card){
  if (!card) return 0;
  int result = displayed_deck->Remove(card);
  return result;
}


void GameStateDeckViewer::Update(float dt)
{
  if(options.keypadActive()){
    options.keypadUpdate(dt);

    if(newDeckname != ""){
      newDeckname = options.keypadFinish();

      if(newDeckname != ""){    
        if(myDeck && myDeck->parent){
          myDeck->parent->meta_name = newDeckname;
          myDeck->save();
        }
        mStage = STAGE_WAITING;
      }
      newDeckname = "";
    }
    //Prevent screen from updating.
    return;    
  }
  hudAlpha = 255-(last_user_activity * 500);
  if (hudAlpha < 0) hudAlpha = 0;
  if (sellMenu){
    sellMenu->Update(dt);
    if (delSellMenu){
      SAFE_DELETE(sellMenu);
      delSellMenu = 0;
    }
    return;
  }
  if (mStage == STAGE_WAITING || mStage == STAGE_ONSCREEN_MENU){
    switch (mEngine->ReadButton())
    {
    case PSP_CTRL_LEFT :
      last_user_activity = 0;
      currentCard = displayed_deck->getNext(currentCard,colorFilter);
      mStage = STAGE_TRANSITION_LEFT;
      break;
    case PSP_CTRL_RIGHT :
      last_user_activity = 0;
      currentCard = displayed_deck->getPrevious(currentCard,colorFilter);
      mStage = STAGE_TRANSITION_RIGHT;
      break;
    case PSP_CTRL_UP :
      last_user_activity = 0;
      mStage = STAGE_TRANSITION_UP;
      colorFilter--;
      if (colorFilter < -1) colorFilter = Constants::MTG_COLOR_LAND;
      break;
    case PSP_CTRL_DOWN :
      last_user_activity = 0;
      mStage = STAGE_TRANSITION_DOWN;
      colorFilter ++;
      if (colorFilter > Constants::MTG_COLOR_LAND) colorFilter =-1;
      break;
    case PSP_CTRL_TRIANGLE :
      if (last_user_activity > 0.2)
      {
        last_user_activity = 0;
        switchDisplay();
      }
      break;
    case PSP_CTRL_CIRCLE :
      last_user_activity = 0;
      addRemove(cardIndex[2]);
      break;
    case PSP_CTRL_CROSS :
      last_user_activity = 0;
      SAFE_DELETE(sellMenu);
      char buffer[4096];
      {
        MTGCard * card  = cardIndex[2];
        if (card && displayed_deck->cards[card]){
          int rnd = (rand() % 20);
          price = pricelist->getPrice(card->getMTGId()) / 2;
          price = price - price * (rnd -10)/100;
          sprintf(buffer,"%s : %i %s",_(card->getName()).c_str(),price,_("credits").c_str());
          sellMenu = NEW SimpleMenu(2,this,mFont,SCREEN_WIDTH-300,SCREEN_HEIGHT/2,buffer);
          sellMenu->Add(20,"Yes");
          sellMenu->Add(21,"No","",true);
        }
      }
      stw.needUpdate = true;
      break;
    case PSP_CTRL_SQUARE :
      if (last_user_activity < NO_USER_ACTIVITY_HELP_DELAY){
        last_user_activity = NO_USER_ACTIVITY_HELP_DELAY + 1;
      }else{
        last_user_activity = 0;
        mStage = STAGE_WAITING;
      }
      break;
    case PSP_CTRL_START :
      mStage = STAGE_MENU;
      break;
    case PSP_CTRL_SELECT :
      if (scrollSpeed == HIGH_SPEED)
        scrollSpeed = MED_SPEED;
      else if (scrollSpeed == MED_SPEED)
        scrollSpeed = LOW_SPEED;
      else
        scrollSpeed = HIGH_SPEED;
      break;
    case PSP_CTRL_LTRIGGER :
      if ((mStage == STAGE_ONSCREEN_MENU) && (--stw.currentPage < 0)) {
        stw.currentPage = stw.pageCount;
      }
      break;
    case PSP_CTRL_RTRIGGER :
      if ((mStage == STAGE_ONSCREEN_MENU) && (++stw.currentPage > stw.pageCount)) {
        stw.currentPage = 0;
      }
      break;
    default :  // no keypress
      if (last_user_activity > NO_USER_ACTIVITY_HELP_DELAY){
        if  (mStage != STAGE_ONSCREEN_MENU){
          mStage = STAGE_ONSCREEN_MENU;
          onScreenTransition = 1;
        stw.currentPage = 0;
        }else{
          if (onScreenTransition >0){
            onScreenTransition-= 0.05f;
          }else{
            onScreenTransition = 0;
          }
        }
      }else{
        last_user_activity+= dt;
      }
    }

  } if (mStage == STAGE_TRANSITION_RIGHT || mStage == STAGE_TRANSITION_LEFT) {
    //mAlpha = 128;
    if (mStage == STAGE_TRANSITION_RIGHT){
      mRotation -= dt * scrollSpeed;
      if (mRotation < -1.0f){
        do {
          rotateCards(mStage);
          mRotation += 1;
        } while (mRotation < -1.0f);
        mStage = STAGE_WAITING;
        mRotation = 0;
      }
    }else if(mStage == STAGE_TRANSITION_LEFT){
      mRotation += dt * scrollSpeed;
      if (mRotation > 1.0f){
        do {
          rotateCards(mStage);
          mRotation -= 1;
        } while (mRotation > 1.0f);
        mStage = STAGE_WAITING;
        mRotation = 0;
      }
    }
  } if (mStage == STAGE_TRANSITION_DOWN || mStage == STAGE_TRANSITION_UP){
    if (mStage == STAGE_TRANSITION_DOWN){
      mSlide -= 0.05f;
      if (mSlide < -1.0f){
        loadIndexes(currentCard);
        mSlide = 1;
      }else if (mSlide > 0 && mSlide < 0.05){
        mStage = STAGE_WAITING;
        mSlide = 0;
      }
    } if (mStage == STAGE_TRANSITION_UP){
      mSlide += 0.05f;
      if (mSlide > 1.0f){
        loadIndexes(currentCard);
        mSlide = -1;
      }else if (mSlide < 0 && mSlide > -0.05){
        mStage = STAGE_WAITING;
        mSlide = 0;
      }
    }


  }else if (mStage == STAGE_WELCOME){
    welcome_menu->Update(dt);
  }else if (mStage == STAGE_MENU){
    menu->Update(dt);
  }


}


void GameStateDeckViewer::renderOnScreenBasicInfo(){
  char buffer[30], buffer2[30];

  float y = 0;
  JRenderer::GetInstance()->FillRoundRect(SCREEN_WIDTH-125,y-5,110,15,5,ARGB(128,0,0,0));
  sprintf(buffer, "DECK: %i", myDeck->getCount());
  mFont->DrawString(buffer, SCREEN_WIDTH-120 , y);

  if (colorFilter != ALL_COLORS){
    sprintf(buffer2, "(      %i)", myDeck->getCount(colorFilter));
    mFont->DrawString(buffer2, SCREEN_WIDTH-55 , y);
    JRenderer::GetInstance()->RenderQuad(mIcons[colorFilter], SCREEN_WIDTH-42  , y + 6 , 0.0f,0.5,0.5);
  }

}


void GameStateDeckViewer::renderSlideBar(){
  int total = displayed_deck->getCount(colorFilter);
  float filler = 15;
  float y = SCREEN_HEIGHT_F-25;
  float bar_size  = SCREEN_WIDTH_F - 2*filler;
  JRenderer * r = JRenderer::GetInstance();
  typedef map<MTGCard *,int,Cmp1>::reverse_iterator rit;

  int currentPos = 0;
  {
    rit end = rit(displayed_deck->cards.begin());
    rit it = rit(displayed_deck->cards.find(cardIndex[2]));
    if (-1 == colorFilter)
      for (; it != end; ++it)
        currentPos += it->second;
    else
      for (; it != end; ++it)
        if (it->first->hasColor(colorFilter)) currentPos += it->second;
  }
  float cursor_pos = bar_size * currentPos / total;

  r->FillRoundRect(filler + 5,y+5,bar_size,0,3,ARGB(hudAlpha/2,0,0,0));
  r->DrawLine(filler+cursor_pos + 5 ,y+5,filler+cursor_pos + 5,y+10,ARGB(hudAlpha/2,0,0,0));

  r->FillRoundRect(filler,y,bar_size,0,3,ARGB(hudAlpha/2,128,128,128));
  //r->FillCircle(filler+cursor_pos + 3 ,SCREEN_HEIGHT - 15 + 3,6,ARGB(255,128,128,128));
  r->DrawLine(filler+cursor_pos,y,filler+cursor_pos,y+5,ARGB(hudAlpha,255,255,255));
  char buffer[256];
  string deckname = "Collection";
  if (displayed_deck == myDeck){
    deckname = "Deck";
  }
  sprintf(buffer,"%s - %i/%i", deckname.c_str(),currentPos, total);
  mFont->SetColor(ARGB(hudAlpha,255,255,255));
  mFont->DrawString(buffer,SCREEN_WIDTH/2, y+5,JGETEXT_CENTER);


  mFont->SetColor(ARGB(255,255,255,255));
}

void GameStateDeckViewer::renderDeckBackground(){
  int max1 = 0;
  int maxC1 = 4;
  int max2 = 0;
  int maxC2 = 4;

  for (int i= 0; i < Constants::MTG_NB_COLORS -1; i++){
    int value = myDeck->getCount(i);
    if (value > max1){
      max2 = max1;
      maxC2 = maxC1;
      max1 = value;
      maxC1 = i;
    }else if (value > max2){
      max2 = value;
      maxC2 = i;
    }
  }
  if (max2 < max1/2){
    maxC2 = maxC1;
  }

  PIXEL_TYPE colors[] =
  {
    ARGB(255, Constants::_r[maxC1], Constants::_g[maxC1], Constants::_b[maxC1]),
    ARGB(255, Constants::_r[maxC1], Constants::_g[maxC1], Constants::_b[maxC1]),
    ARGB(255, Constants::_r[maxC2], Constants::_g[maxC2], Constants::_b[maxC2]),
    ARGB(255, Constants::_r[maxC2], Constants::_g[maxC2], Constants::_b[maxC2]),
  };

  JRenderer::GetInstance()->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,colors);

}

void GameStateDeckViewer::renderOnScreenMenu(){
  JLBFont * font = resources.GetJLBFont(Constants::MAIN_FONT);
  font->SetColor(ARGB(255,255,255,255));
  JRenderer * r = JRenderer::GetInstance();
  float pspIconsSize = 0.5;

  float leftTransition = onScreenTransition*84;  
  float rightTransition = onScreenTransition*204;
  float leftPspX = 40 - leftTransition;
  float leftPspY = SCREEN_HEIGHT/2 - 20 ;
  float rightPspX = SCREEN_WIDTH-100 + rightTransition;
  float rightPspY = SCREEN_HEIGHT/2 - 20 ;

  if (stw.currentPage == 0) {
    //FillRects
    r->FillRect(0-(onScreenTransition*84),0,84,SCREEN_HEIGHT,ARGB(128,0,0,0));
    r->FillRect(SCREEN_WIDTH-204+(onScreenTransition*204),0,200,SCREEN_HEIGHT,ARGB(128,0,0,0));


    //LEFT PSP CIRCLE render
    r->FillCircle(leftPspX,leftPspY,40,ARGB(128,50,50,50));

    r->RenderQuad(pspIcons[0],leftPspX, leftPspY - 20,0,pspIconsSize,pspIconsSize);
    r->RenderQuad(pspIcons[1],leftPspX, leftPspY + 20,0,pspIconsSize,pspIconsSize);
    r->RenderQuad(pspIcons[2],leftPspX - 20, leftPspY,0,pspIconsSize,pspIconsSize);
    r->RenderQuad(pspIcons[3],leftPspX + 20, leftPspY,0,pspIconsSize,pspIconsSize);


    font->DrawString(_("Prev.").c_str(), leftPspX - 35, leftPspY-15);
    font->DrawString(_("Next").c_str(), leftPspX + 15, leftPspY-15);
    font->DrawString(_("card").c_str(), leftPspX - 35, leftPspY);
    font->DrawString(_("card").c_str(), leftPspX + 15, leftPspY);
    font->DrawString(_("Next color").c_str(), leftPspX - 33, leftPspY - 35);
    font->DrawString(_("Prev. color").c_str(), leftPspX -33 , leftPspY +25);

    //RIGHT PSP CIRCLE render
    r->FillCircle(rightPspX+(onScreenTransition*204),rightPspY,40,ARGB(128,50,50,50));
    r->RenderQuad(pspIcons[4],rightPspX+20, rightPspY,0,pspIconsSize,pspIconsSize);
    r->RenderQuad(pspIcons[5],rightPspX, rightPspY - 20,0,pspIconsSize,pspIconsSize);
    r->RenderQuad(pspIcons[6],rightPspX-20, rightPspY,0,pspIconsSize,pspIconsSize);
    r->RenderQuad(pspIcons[7],rightPspX, rightPspY + 20,0,pspIconsSize,pspIconsSize);

    if (displayed_deck == myCollection){
      font->DrawString(_("Add card").c_str(), rightPspX + 20, rightPspY-15);
      font->DrawString(_("Display Deck").c_str(), rightPspX - 35, rightPspY - 40);
    }else{
      font->DrawString(_("Remove card").c_str(), rightPspX + 20, rightPspY-15);
      font->DrawString(_("Display Collection").c_str(), rightPspX - 35, rightPspY - 40);
    }
    font->DrawString(_("Deck info").c_str(), rightPspX - 70 , rightPspY-15);
    font->DrawString(_("Sell card").c_str(), rightPspX - 30 , rightPspY+20);
    //Bottom menus
    font->DrawString(_("menu").c_str(), SCREEN_WIDTH-35 +rightTransition, SCREEN_HEIGHT-15);



    //Your Deck Information
    char buffer[300];
    int nb_letters = 0;
    for (int j=0; j<Constants::MTG_NB_COLORS;j++){
      int value = myDeck->getCount(j);
      if (value > 0){
        sprintf(buffer, "%i", value);
        font->DrawString(buffer, SCREEN_WIDTH-190+rightTransition + nb_letters*13, SCREEN_HEIGHT/2 + 40);
        r->RenderQuad(mIcons[j],SCREEN_WIDTH-197+rightTransition + nb_letters*13 , SCREEN_HEIGHT/2 + 46,0,0.5,0.5);
        if (value > 9){nb_letters += 3;}else{nb_letters+=2;}
      }
    }
    int value = myDeck->getCount();
    sprintf(buffer, _("Your Deck: %i cards").c_str(),  value);
    font->DrawString(buffer, SCREEN_WIDTH-200+rightTransition, SCREEN_HEIGHT/2 + 25);

    //TODO, put back !
    /*int nbCreatures = myDeck->countByType("Creature");
    int nbSpells = myDeck->countByType("Instant") + myDeck->countByType("Enchantment") + myDeck->countByType("Sorcery");

    sprintf(buffer, "Creatures: %i - Spells: %i", nbCreatures, nbSpells);
    mFont->DrawString(buffer, SCREEN_WIDTH-200+rightTransition, SCREEN_HEIGHT/2 + 55);
    */


    font->DrawString(_("You are currently viewing your").c_str(),  SCREEN_WIDTH-200+rightTransition, 5);
    if (displayed_deck == myCollection){
      font->DrawString(_("collection. Press TRIANGLE").c_str(),  SCREEN_WIDTH-200+rightTransition, 19);
      font->DrawString(_("to switch to your deck.").c_str(),  SCREEN_WIDTH-200+rightTransition, 33);
    }else{
      font->DrawString(_("deck. Press TRIANGLE to").c_str(),  SCREEN_WIDTH-200+rightTransition, 19);
      font->DrawString(_("switch to your collection.").c_str(),  SCREEN_WIDTH-200+rightTransition, 33);
    }
    font->DrawString(_("Press L/R to cycle through").c_str(),  SCREEN_WIDTH-200+rightTransition, 47);
    font->DrawString(_("deck statistics.").c_str(),  SCREEN_WIDTH-200+rightTransition, 61);
  } else {
    if (stw.needUpdate) {
      updateStats();
    }

    char buffer[300];

    leftTransition = -(onScreenTransition/2)*SCREEN_WIDTH;
    rightTransition = -leftTransition;

    r->FillRect(0+leftTransition,0,SCREEN_WIDTH/2,SCREEN_HEIGHT,ARGB(128,0,0,0));
    r->FillRect(SCREEN_WIDTH/2+rightTransition,0,SCREEN_WIDTH/2,SCREEN_HEIGHT,ARGB(128,0,0,0));
    r->FillRect(10+leftTransition,10,SCREEN_WIDTH/2-10,SCREEN_HEIGHT-20,ARGB(128,0,0,0));
    r->FillRect(SCREEN_WIDTH/2+rightTransition,10,SCREEN_WIDTH/2-10,SCREEN_HEIGHT-20,ARGB(128,0,0,0));
    font->DrawString(_("menu").c_str(), SCREEN_WIDTH-35 +rightTransition, SCREEN_HEIGHT-15);
    
    // Draw page id
    sprintf(buffer, _("statsPage#: %i").c_str(), stw.currentPage);
    font->DrawString(buffer, 10+leftTransition, 10);      
    
    int nb_letters = 0;
    float posX, posY;
    DWORD graphColor;
    
    graphColor = ARGB(200, 155, 155, 155);

    switch (stw.currentPage) {
      case 1: // Counts, price
        posY = 30;
        posX = 180;
        sprintf(buffer, _("Your Deck: %i cards").c_str(),  stw.cardCount);
        font->DrawString(buffer, 20 + leftTransition, posY);
        posY += 10;

        // Counts by color
        for (int j=0; j<Constants::MTG_NB_COLORS;j++){
          int value = myDeck->getCount(j);
          if (value > 0){
            sprintf(buffer, "%i", value);
            font->DrawString(buffer, 38 + nb_letters*13 + leftTransition, posY + 5);
            r->RenderQuad(mIcons[j], 30 + nb_letters*13 + leftTransition, posY + 11,0,0.5,0.5);
            if (value > 9){nb_letters += 3;}else{nb_letters+=2;}
          }
        }            
        posY += 25;
        
        r->DrawLine(posX - 4 + leftTransition, posY - 1, posX - 4 + leftTransition, posY + 177, ARGB(128, 255, 255, 255));
        r->DrawLine(19 + leftTransition, posY - 1, 19 + leftTransition, posY + 177, ARGB(128, 255, 255, 255));
        r->DrawLine(posX + 40 + leftTransition, posY - 1, posX + 40 + leftTransition, posY + 177, ARGB(128, 255, 255, 255));
        
        r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));

        sprintf(buffer, _("Lands").c_str());
        mFont->DrawString(buffer, 20 + leftTransition, posY);
        sprintf(buffer, _("%i").c_str(), stw.countLands);
        mFont->DrawString(buffer, posX + leftTransition, posY);
        
        posY += 14;
        r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
        sprintf(buffer, _("Creatures").c_str());
        mFont->DrawString(buffer, 20 + leftTransition, posY);
        sprintf(buffer, _("%i").c_str(), stw.countCreatures);
        mFont->DrawString(buffer, posX + leftTransition, posY);
        
        posY += 14;
        r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
        sprintf(buffer, _("Spells").c_str());
        mFont->DrawString(buffer, 20 + leftTransition, posY);
        sprintf(buffer, _("%i").c_str(), stw.countSpells);
        mFont->DrawString(buffer, posX + leftTransition, posY);

        posY += 10;
        sprintf(buffer, _("Instants").c_str());
        mFont->DrawString(buffer, 30 + leftTransition, posY);
        sprintf(buffer, _("%i").c_str(), stw.countInstants);
        mFont->DrawString(buffer, posX + leftTransition, posY);

        posY += 10;
        sprintf(buffer, _("Enchantments").c_str());
        mFont->DrawString(buffer, 30 + leftTransition, posY);
        sprintf(buffer, _("%i").c_str(), stw.countEnchantments);
        mFont->DrawString(buffer, posX + leftTransition, posY);

        posY += 10;
        sprintf(buffer, _("Sorceries").c_str());
        mFont->DrawString(buffer, 30 + leftTransition, posY);
        sprintf(buffer, _("%i").c_str(), stw.countSorceries);
        mFont->DrawString(buffer, posX + leftTransition, posY);
        //sprintf(buffer, "Artifacts: %i", stw.countArtifacts);
        //mFont->DrawString(buffer, 20, 123);            
        
        posY += 14;
        r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));

        sprintf(buffer, _("Average converted mana cost").c_str());
        font->DrawString(buffer, 20 + leftTransition, posY);
        sprintf(buffer, _("%2.2f").c_str(), stw.avgManaCost);
        font->DrawString(buffer, posX + leftTransition, posY);

        posY += 14;
        r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
        sprintf(buffer, _("Probabilities").c_str());
        font->DrawString(buffer, 20 + leftTransition, posY);
        
        posY += 10;
        sprintf(buffer, _("No land in 1st hand").c_str());
        font->DrawString(buffer, 30 + leftTransition, posY);
        sprintf(buffer, _("%2.2f%%").c_str(), stw.noLandsProbInTurn[0]);
        font->DrawString(buffer, posX + leftTransition, posY);

        posY += 10;
        sprintf(buffer, _("No land in 9 cards").c_str());
        font->DrawString(buffer, 30 + leftTransition, posY);
        sprintf(buffer, _("%2.2f%%").c_str(), stw.noLandsProbInTurn[2]);
        font->DrawString(buffer, posX + leftTransition, posY);
                    
        posY += 10;
        sprintf(buffer, _("No creatures in 1st hand").c_str());
        font->DrawString(buffer, 30 + leftTransition, posY);
        sprintf(buffer, _("%2.2f%%").c_str(), stw.noCreaturesProbInTurn[0]);
        font->DrawString(buffer, posX + leftTransition, posY);

        // Playgame Statistics
        posY += 14;
        r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
        sprintf(buffer, _("Playgame statistics").c_str());
        font->DrawString(buffer, 20 + leftTransition, posY);

        posY += 10;
        sprintf(buffer, _("Games played").c_str());
        font->DrawString(buffer, 30 + leftTransition, posY);
        sprintf(buffer, _("%i").c_str(), stw.gamesPlayed);
        font->DrawString(buffer, posX + leftTransition, posY);

        posY += 10;
        sprintf(buffer, _("Victory ratio").c_str());
        font->DrawString(buffer, 30 + leftTransition, posY);
        sprintf(buffer, _("%i%%").c_str(), stw.percentVictories);
        font->DrawString(buffer, posX + leftTransition, posY);
        
        posY += 15;
        r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
        sprintf(buffer, _("Total price (credits)").c_str(),  stw.totalPrice);
        font->DrawString(buffer, 20 + leftTransition, posY);
        sprintf(buffer, _("%i ").c_str(),  stw.totalPrice);
        font->DrawString(buffer, posX, posY);
        r->DrawLine(20 + leftTransition, posY + 13, posX + 40 + leftTransition, posY + 13, ARGB(128, 255, 255, 255));

        break;
        
      case 2: // Mana cost detail
        sprintf(buffer, _("Card counts per mana cost:").c_str());
        font->DrawString(buffer, 20 + leftTransition, 30);            

        posY = 70;
        
        // Column titles
        for (int j=0; j<Constants::MTG_NB_COLORS-1;j++){
          r->RenderQuad(mIcons[j], 67 + j*15 + leftTransition, posY - 11,0,0.5,0.5);
        }
        
        sprintf(buffer, _("C").c_str());
        font->DrawString(buffer, 30 + leftTransition, posY-16);              
        sprintf(buffer, _("#").c_str());
        font->DrawString(buffer, 45 + leftTransition, posY-16);              

        // Horizontal table lines
        r->DrawLine(27 + leftTransition, posY - 20, 75 + (Constants::MTG_NB_COLORS-2)*15 + leftTransition, posY - 20, ARGB(128, 255, 255, 255));
        r->DrawLine(27 + leftTransition, posY - 1, 75 + (Constants::MTG_NB_COLORS-2)*15 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
        r->DrawLine(27 + leftTransition, STATS_MAX_MANA_COST*10 + posY + 12, 75 + (Constants::MTG_NB_COLORS-2)*15 + leftTransition, STATS_MAX_MANA_COST*10 + posY + 12, ARGB(128, 255, 255, 255));
        
        // Vertical table lines
        r->DrawLine(26 + leftTransition, posY - 20, 26 + leftTransition, STATS_MAX_MANA_COST*10 + posY + 12, ARGB(128, 255, 255, 255));
        r->DrawLine(41 + leftTransition, posY - 20, 41 + leftTransition, STATS_MAX_MANA_COST*10 + posY + 12, ARGB(128, 255, 255, 255));
        r->DrawLine(58 + leftTransition, posY - 20, 58 + leftTransition, STATS_MAX_MANA_COST*10 + posY + 12, ARGB(128, 255, 255, 255));
        r->DrawLine(75 + leftTransition + (Constants::MTG_NB_COLORS-2)*15, posY - 20, 75 + leftTransition + (Constants::MTG_NB_COLORS-2)*15, STATS_MAX_MANA_COST*10 + posY + 12, ARGB(128, 255, 255, 255));

        for (int i=0; i<=STATS_MAX_MANA_COST; i++) {              
          sprintf(buffer, _("%i").c_str(), i);
          font->DrawString(buffer, 30 + leftTransition, posY);              
          sprintf(buffer, _("%i").c_str(), stw.countCardsPerCost[i]);
          font->DrawString(buffer, 45 + leftTransition, posY);              
          for (int j=0; j<Constants::MTG_NB_COLORS-1;j++){
            sprintf(buffer, (stw.countCardsPerCostAndColor[i][j]>0)?_("%i").c_str():".", stw.countCardsPerCostAndColor[i][j]);
            font->DrawString(buffer, 64 + leftTransition + j*15, posY);
          }
          r->FillRect(77 + leftTransition + (Constants::MTG_NB_COLORS-2)*15, posY + 2, stw.countCardsPerCost[i]*5, 8, graphColor);
          posY += 10;
        }
        
        posY += 10;
        sprintf(buffer, _("Average converted mana cost: %2.2f").c_str(), stw.avgManaCost);
        font->DrawString(buffer, 20 + leftTransition, posY);
        posY += 15;
        sprintf(buffer, _("C - Converted mana cost. Cards with cost>%i are included in the last row.").c_str(), STATS_MAX_MANA_COST);
        font->DrawString(buffer, 20 + leftTransition, posY);
        posY += 10;
        sprintf(buffer, _("# - Total number of cards with given cost").c_str());
        font->DrawString(buffer, 20 + leftTransition, posY);
        
        break;

      case 4: // No lands detail
        float graphScale, graphWidth;
        graphWidth = 100;
        graphScale = (stw.noLandsProbInTurn[0]==0) ? 0:(graphWidth/stw.noLandsProbInTurn[0]);

        sprintf(buffer, _("No lands in first n cards:").c_str());
        font->DrawString(buffer, 20 + leftTransition, 30);

        posY = 50;
        for (int i=0; i<STATS_FOR_TURNS; i++) {              
          sprintf(buffer, _("%i:").c_str(), i+7);
          font->DrawString(buffer, 30 + leftTransition, posY);              
          sprintf(buffer, _("%2.2f%%").c_str(), stw.noLandsProbInTurn[i]);
          font->DrawString(buffer, 45 + leftTransition, posY);              
          r->FillRect(84 + leftTransition, posY + 2, graphScale*stw.noLandsProbInTurn[i], 8, graphColor);
          posY += 10;
        }
        
        // No creatures probability detail
        sprintf(buffer, _("No creatures in first n cards:").c_str());
        posY += 10;
        font->DrawString(buffer, 20 + leftTransition, posY);
        posY += 20;
        graphScale = (stw.noCreaturesProbInTurn[0]==0) ? 0:(graphWidth/stw.noCreaturesProbInTurn[0]);
        
        for (int i=0; i<STATS_FOR_TURNS; i++) {              
          sprintf(buffer, _("%i:").c_str(), i+7);
          font->DrawString(buffer, 30 + leftTransition, posY);              
          sprintf(buffer, _("%2.2f%%").c_str(), stw.noCreaturesProbInTurn[i]);
          font->DrawString(buffer, 45 + leftTransition, posY);              
          r->FillRect(84 + leftTransition, posY + 2, graphScale*stw.noCreaturesProbInTurn[i], 8, graphColor);
          posY += 10;
        }

        break;

      case 3: // Total mana cost per color
        sprintf(buffer, _("Total colored manasymbols in cards' casting costs:").c_str());
        font->DrawString(buffer, 20 + leftTransition, 30);
        
        posY = 50;
        for (int i=1; i<Constants::MTG_NB_COLORS-1; i++) {
          if (stw.totalCostPerColor[i]>0) {
            sprintf(buffer, _("%i").c_str(), stw.totalCostPerColor[i]);
            font->DrawString(buffer, 20 + leftTransition, posY);
            sprintf(buffer, _("(%i%%)").c_str(), (int)(100*(float)stw.totalCostPerColor[i]/stw.totalColoredSymbols));
            font->DrawString(buffer, 33 + leftTransition, posY);
            posX = 70;
            for (int j=0; j<stw.totalCostPerColor[i]; j++) {
              r->RenderQuad(mIcons[i], posX + leftTransition, posY+6, 0, 0.5, 0.5);
              posX += ((j+1)%10==0)?17:13;
              if ((((j+1)%30)==0) && (j<stw.totalCostPerColor[i]-1)) {
                posX = 70;
                posY += 15;
              }
            }
            posY += 17;
          }
        }
        break;
      
      case 5: // Victory statistics
        sprintf(buffer, _("Victories against AI:").c_str());
        font->DrawString(buffer, 20 + leftTransition, 30);            
        
        sprintf(buffer, _("Games played: %i").c_str(), stw.gamesPlayed);
        font->DrawString(buffer, 20 + leftTransition, 45);
        sprintf(buffer, _("Victory ratio: %i%%").c_str(), stw.percentVictories);
        font->DrawString(buffer, 20 + leftTransition, 55);

        posY = 70;
        posX = 20;

        // ToDo: Multiple pages when too many AI decks are present
        // ToDo: Don't display AI decks with zero games played
        for (int i=0; i<(int)stw.aiVictoryRatio.size(); i++) {
          sprintf(buffer, _("%.21s").c_str(), stw.aiDeckNames.at(i).c_str());
          font->DrawString(buffer, posX + leftTransition, posY);
          sprintf(buffer, _("%i%%").c_str(), stw.aiVictoryRatio.at(i));
          font->DrawString(buffer, posX + leftTransition+115, posY);
          posY += 10;
          if (((i+1)%19)==0) {
            posY = 70;
            posX += 155;
          }
        }
        break;

      }
  }
}

void GameStateDeckViewer::updateStats() {
  if (!stw.needUpdate) {
    return;
  }
  stw.needUpdate = false; 

  stw.cardCount = myDeck->getCount();
  stw.countLands = myDeck->getCount(Constants::MTG_COLOR_LAND);
  stw.totalPrice = myDeck->totalPrice();

  // Mana cost
  int currentCount, convertedCost;
  ManaCost * currentCost;
  stw.totalManaCost = 0;    
  MTGCard * current = myDeck->getNext();

  // Clearing arrays
  for (int i=0; i<=STATS_MAX_MANA_COST; i++) {
    stw.countCardsPerCost[i] = 0;
  }

  for (int i=0; i<=Constants::MTG_NB_COLORS; i++) {
    stw.totalCostPerColor[i] = 0;
  }

  for (int i=0; i<=STATS_MAX_MANA_COST; i++) {
    for (int k=0; k<=Constants::MTG_NB_COLORS; k++) {
      stw.countCardsPerCostAndColor[i][k] = 0;
    }
  }

  while (current){
    currentCost = current->getManaCost();
    convertedCost = currentCost->getConvertedCost();
    currentCount = myDeck->cards[current];
    
    // Add to the cards per cost counters
    stw.totalManaCost += convertedCost * currentCount;
    if (convertedCost > STATS_MAX_MANA_COST) {
      convertedCost = STATS_MAX_MANA_COST;
    }
    stw.countCardsPerCost[convertedCost] += currentCount;
    
    // Add to the per color counters
    //  a. regular costs
    for (int j=0; j<Constants::MTG_NB_COLORS;j++){
      stw.totalCostPerColor[j] += currentCost->getCost(j)*currentCount;
      if (current-> hasColor(j)) { 
        // Add to the per cost and color counter
        stw.countCardsPerCostAndColor[convertedCost][j] += currentCount;
      }
    }
    
    //  b. Hybrid costs
    ManaCostHybrid * hybridCost;
    int i;
    i = 0;

    while ((hybridCost = currentCost->getHybridCost(i++)) != NULL) {
      stw.totalCostPerColor[hybridCost->color1] += hybridCost->value1*currentCount;
      stw.totalCostPerColor[hybridCost->color2] += hybridCost->value2*currentCount;
    }


    current = myDeck->getNext(current);
  }  

  stw.totalColoredSymbols = 0;
  for (int j=1; j<Constants::MTG_NB_COLORS;j++){
    stw.totalColoredSymbols += stw.totalCostPerColor[j];
  }

  stw.countCardsPerCost[0] -= stw.countLands; // Quick hack to exclude lands from zero costed card count

  stw.avgManaCost = ((stw.cardCount - stw.countLands) <= 0)?0:(float)stw.totalManaCost / (stw.cardCount - stw.countLands);

  // Counts by type
  stw.countCreatures = countCardsByType("Creature");
  stw.countInstants = countCardsByType("Instant");
  stw.countEnchantments = countCardsByType("Enchantment");
  stw.countSorceries = countCardsByType("Sorcery");
  stw.countSpells = stw.countInstants + stw.countEnchantments + stw.countSorceries;
  //stw.countArtifacts = countCardsByType("Artifact");

  // Probabilities
  // TODO: this could be optimized by reusing results
  for (int i=0; i<STATS_FOR_TURNS; i++) {
    stw.noLandsProbInTurn[i] = noLuck(stw.cardCount, stw.countLands, 7+i)*100;
    stw.noCreaturesProbInTurn[i] = noLuck(stw.cardCount, stw.countCreatures, 7+i)*100;
  }

}

// This should probably be cached in DeckDataWrapper
// or at least be calculated for all common types in one go
int GameStateDeckViewer::countCardsByType(const char * _type) {
  int result = 0;

  MTGCard * current = myDeck->getNext();
  while (current){
    if(current->hasType(_type)){
      result += myDeck->cards[current];
    }
    current = myDeck->getNext(current);
  }
  return result;
}

void GameStateDeckViewer::renderCard(int id, float rotation){
  MTGCard * card = cardIndex[id];


  float max_scale = 0.96;
  float x_center_0 = 180;
  float right_border = SCREEN_WIDTH - 20 ;

  float x_center = x_center_0 + cos((rotation + 8 - id)*M_PI/12)*(right_border-x_center_0);
  float scale = max_scale/ 1.12 * cos((x_center-x_center_0)*1.5/(right_border - x_center_0) ) + 0.2 * max_scale * cos (cos((x_center-x_center_0)*0.15/(right_border - x_center_0) ));
  float x =  x_center; // ;

  float y = (SCREEN_HEIGHT)/2 + SCREEN_HEIGHT*mSlide*(scale+0.2);

  int alpha = (int) (255 * (scale + 1.0 - max_scale));

  if (!card) return;
  JQuad * quad = NULL;

  int showName = 0;
  int cacheError = CACHE_ERROR_NONE;

  if(!options[Options::DISABLECARDS].number){
    quad = resources.RetrieveCard(card,RETRIEVE_EXISTING);
    cacheError = resources.RetrieveError();
    if (!quad && cacheError != CACHE_ERROR_404){
      if(last_user_activity > (abs(2-id) + 1)* NO_USER_ACTIVITY_SHOWCARD_DELAY)
        quad = resources.RetrieveCard(card);
      else{
        quad = backQuad;
        showName = 1;
      }
    }
  }

  if (quad){
    showName = 0;
    int quadAlpha = alpha;
    if ( !displayed_deck->cards[card]) quadAlpha /=2;
    quad->SetColor(ARGB(mAlpha,quadAlpha,quadAlpha,quadAlpha));
    JRenderer::GetInstance()->RenderQuad(quad, x   , y , 0.0f,scale,scale);
    if (showName){
      char buffer[4096];
      sprintf(buffer, "%s", _(card->getName()).c_str());
      float scaleBackup = mFont->GetScale();
      mFont->SetScale(scale);
      mFont->DrawString(buffer,x - 100*scale ,y - 145*scale);
      mFont->SetScale(scaleBackup);
    }
  }else{
    Pos pos = Pos(x, y, scale* 285/250, 0.0, 255);
    CardGui::alternateRender(card, pos);
    if(!options[Options::DISABLECARDS].number)
      quad = resources.RetrieveCard(card,CACHE_THUMB);
    if (quad){
      float _scale = 285 * scale / quad->mHeight;
      quad->SetColor(ARGB(40,255,255,255));
      JRenderer::GetInstance()->RenderQuad(quad,x,y,0,_scale,_scale);
    }
  }
  if (last_user_activity < 3){
    int fontAlpha = alpha;
    float qtY  = y -135*scale;
    float qtX = x + 40*scale;
    char buffer[4096];
    sprintf(buffer, "x%i", displayed_deck->cards[card]);
    JLBFont * font = mFont;
    font->SetColor(ARGB(fontAlpha/2,0,0,0));
    JRenderer::GetInstance()->FillRect(qtX, qtY,font->GetStringWidth(buffer) + 6,16,ARGB(fontAlpha/2,0,0,0));
    font->DrawString(buffer, qtX + 4, qtY + 4);
    font->SetColor(ARGB(fontAlpha,255,255,255));
    font->DrawString(buffer, qtX+2, qtY + 2);
    font->SetColor(ARGB(255,255,255,255));
  }
}


void GameStateDeckViewer::renderCard (int id){
  renderCard(id, 0);
}

void GameStateDeckViewer::Render()
{
  //  void RenderQuad(JQuad* quad, float xo, float yo, float angle=0.0f, float xScale=1.0f, float yScale=1.0f);
  JRenderer * r = JRenderer::GetInstance();
  r->ClearScreen(ARGB(0,0,0,0));


  if(displayed_deck == myDeck){
    renderDeckBackground();
  }




  int order[3] = {1,2,3};
  if (mRotation < 0.5 && mRotation > -0.5){
    order[1]=3;
    order[2]=2;
  }else if (mRotation < -0.5){
    order[0] = 3;
    order[2] = 1;
  }

  renderCard(6,mRotation);
  renderCard(5,mRotation);
  renderCard(4,mRotation);
  renderCard(0,mRotation);

  for (int i =0; i< 3; i++){
    renderCard(order[i],mRotation);
  }

  if (displayed_deck->getCount(colorFilter)>0){
    renderSlideBar();
  }else{
    mFont->DrawString(_("No Card").c_str(), SCREEN_WIDTH/2, SCREEN_HEIGHT/2,JGETEXT_CENTER);
  }

  if (mStage == STAGE_ONSCREEN_MENU){
    renderOnScreenMenu();
  }else if (mStage == STAGE_WELCOME){
    r->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(200,0,0,0));
    welcome_menu->Render();
  }else{
    renderOnScreenBasicInfo();

  }
  if (mStage == STAGE_MENU){
    menu->Render();
  }
  if (sellMenu) sellMenu->Render();

  if(options.keypadActive())
    options.keypadRender();
}


int GameStateDeckViewer::loadDeck(int deckid){
  SAFE_DELETE(myCollection); 
  stw.currentPage = 0;
  stw.pageCount = 5;
  stw.needUpdate = true;

  string profile = options[Options::ACTIVE_PROFILE].str;
  myCollection =    NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(PLAYER_COLLECTION).c_str(), mParent->collection));
  displayed_deck = myCollection;
  char deckname[256];
  sprintf(deckname,"deck%i.txt",deckid);
  SAFE_DELETE(myDeck);
  myDeck = NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(deckname,"",false,false).c_str(), mParent->collection));
  MTGCard * current = myDeck->getNext();
  while (current){
    int howmanyinDeck = myDeck->cards[current];
    for (int i = 0; i < howmanyinDeck; i++){
      int deleted = myCollection->Remove(current);
      if (!deleted){
        myDeck->Remove(current);
      }
    }
    current = myDeck->getNext(current);
  }
  currentCard = NULL;
  loadIndexes();
    // Load deck statistics
    // TODO: Code cleanup (Copypasted with slight changes from GameStateMenu.cpp)
    char buffer[512];
    DeckStats * stats = DeckStats::GetInstance();
    stw.aiVictoryRatio.clear();
    stw.aiDeckNames.clear();

    sprintf(buffer, "stats/player_deck%i.txt", deckid);
    string deckstats = options.profileFile(buffer);
    
    if(fileExists(deckstats.c_str())){
      stats->load(deckstats.c_str());
      stw.percentVictories = stats->percentVictories();
      stw.gamesPlayed = stats->nbGames();
      
      // Detailed deck statistics against AI
      // Yet another copypaste (GameState.cpp)
      int found = 1;
      int nbDecks = 0;
      while (found){
        found = 0;
        char buffer[512];
        char smallDeckName[512];
        //char deckDesc[512];
        sprintf(buffer, "%s/deck%i.txt",RESPATH"/ai/baka",nbDecks+1);
        if(fileExists(buffer)){
          MTGDeck * mtgd = NEW MTGDeck(buffer,NULL,1);
          found = 1;
          nbDecks++;

          sprintf(smallDeckName, "%s_deck%i","ai_baka",nbDecks);
          int percentVictories = stats->percentVictories(string(smallDeckName));
          stw.aiVictoryRatio.push_back(percentVictories);
          stw.aiDeckNames.push_back(string(mtgd->meta_name));
          delete mtgd;
        }
      }
    } else {
      stw.gamesPlayed = 0;
      stw.percentVictories = 0;
    }    
  return 1;
}

void GameStateDeckViewer::ButtonPressed(int controllerId, int controlId)
{
  switch(controllerId){
      case 10:  //Deck menu
        if (controlId == -1){
          if(!mSwitching)
            mParent->SetNextState(GAME_STATE_MENU);
          else
            mStage = STAGE_WAITING;

          mSwitching = false;
          break;
        }
        loadDeck(controlId);
        mStage = STAGE_WAITING;
        deckNum = controlId;
        break;
      case 11: //Save / exit menu
        switch (controlId)
        {
        case 0:
          myDeck->save();
          playerdata->save();
          pricelist->save();
          mStage =  STAGE_WAITING;
          break;
        case 1:
          if(myDeck && myDeck->parent){
          options.keypadStart(myDeck->parent->meta_name,&newDeckname);
          options.keypadTitle("Rename deck");
          }
          break;
        case 2:
          updateDecks();
          mStage =  STAGE_WELCOME;
          mSwitching = true;
          break;
        case 3:
          mParent->SetNextState(GAME_STATE_MENU);
          break;
        case 4:
          mStage =  STAGE_WAITING;
          break;
        }
      break;
    case 2:
    switch (controlId){
      case 20:
        {
          MTGCard * card  = cardIndex[2];
          if (card){
            int rnd = (rand() % 25);
            playerdata->credits += price;
            price = price - (rnd * price)/100;
            pricelist->setPrice(card->getMTGId(),price*2);
    #if defined (WIN32) || defined (LINUX)
            char buf[4096];
            sprintf(buf, "CARD'S NAME : %s", card->getName().c_str());
            OutputDebugString(buf);
    #endif
            playerdata->collection->remove(card->getMTGId());
            Remove(card);
          }
        }
      case 21:
        delSellMenu = 1;
        break;
    }
  }
}

// n cards total, a of them are of desired type (A), x drawn 
// returns probability of no A's
float noLuck(int n, int a, int x) {
  if ( (a >= n) || (a == 0)) {
    return 1;
  }
  if ((n == 0) || (x == 0) || (x > n) || (n-a < x)) {
    return 0;
  }
  
  a = n - a;
  float result = 1;
  
  for (int i=0; i<x; i++) {
    result *= (float)(a-i)/(n-i);
  }
  return result;
} 
