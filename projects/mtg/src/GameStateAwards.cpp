/*
  This is where the player views their awards, etc.
*/
#include "../include/config.h"
#include <JRenderer.h>
#include "../include/GameStateAwards.h"
#include "../include/GameApp.h"
#include "../include/MTGDeck.h"
#include "../include/Translate.h"
#include "../include/OptionItem.h"
#include "../include/GameOptions.h"
#include "../include/DeckDataWrapper.h"

enum ENUM_AWARDS_STATE{
    STATE_LISTVIEW,
    STATE_DETAILS,
};

GameStateAwards::GameStateAwards(GameApp* parent): GameState(parent){

}

GameStateAwards::~GameStateAwards() {

}

void GameStateAwards::End()
{
  SAFE_DELETE(menu);
  SAFE_DELETE(detailview);
  SAFE_DELETE(listview);
  SAFE_DELETE(setSrc);

  resources.Release(mBgTex);
  if(saveMe)
	  options.save();
}
void GameStateAwards::Start()
{ 
  char buf[256];
  mState = STATE_LISTVIEW;
  options.checkProfile();
  //resources.ClearUnlocked(); //Last resort.

  menu = NULL;
  saveMe = options.newAward();

  listview = NEW WGuiList("Listview");
  listview->setX(210);
  listview->setWidth(SCREEN_WIDTH - 220);
  detailview = NULL;
  WGuiAward * aw;
  WGuiButton * btn;
  
  WGuiHeader * wgh = NEW WGuiHeader("Achievements");
  listview->Add(wgh);

  aw = NEW WGuiAward(Options::DIFFICULTY_MODE_UNLOCKED,"Difficulty Modes","Achieved a 66% victory ratio.");
  btn = NEW WGuiButton(aw,-103,Options::DIFFICULTY_MODE_UNLOCKED,this);
  listview->Add(btn);

  aw = NEW WGuiAward(Options::MOMIR_MODE_UNLOCKED,"Momir Mode","Won with exactly 8 lands.");
  btn = NEW WGuiButton(aw,-103,Options::MOMIR_MODE_UNLOCKED,this);
  listview->Add(btn);

  aw = NEW WGuiAward(Options::EVILTWIN_MODE_UNLOCKED,"Evil Twin Mode","Won with same army size.");
  btn = NEW WGuiButton(aw,-103,Options::EVILTWIN_MODE_UNLOCKED,this);
  listview->Add(btn);

  aw = NEW WGuiAward(Options::RANDOMDECK_MODE_UNLOCKED,"Random Deck Mode","Won against a higher difficulty.");
  btn = NEW WGuiButton(aw,-103,Options::RANDOMDECK_MODE_UNLOCKED,this);
  listview->Add(btn);

  aw = NEW WGuiAward(Options::AWARD_COLLECTOR,"Valuable Collection","Collection valued over 10,000c.","Collection Info");
  btn = NEW WGuiButton(aw,-103,Options::AWARD_COLLECTOR,this);
  listview->Add(btn);

  wgh = NEW WGuiHeader("");
  listview->Add(wgh);

  int locked = 0;
  for (int i = 0; i < setlist.size(); i++){
    MTGSetInfo * si = setlist.getInfo(i);
    if(!si)
      continue;
    if(!options[Options::optionSet(i)].number){
      locked++;
      continue;
    }

    if(!si->author.size())
      sprintf(buf,"%i cards.",si->totalCards());
    else if(si->year > 0)
      sprintf(buf,"%s (%i): %i cards",si->author.c_str(),si->year,si->totalCards());
    else
      sprintf(buf,"%s: %i cards.",si->author.c_str(),si->totalCards());
    
      
    aw = NEW WGuiAward(Options::optionSet(i),si->getName(),buf,"Card Spoiler");
    btn = NEW WGuiButton(aw,-103,Options::optionSet(i),this);
    listview->Add(btn);
  }
  if(locked)
    sprintf(buf,_("%i locked sets remain.").c_str(),locked);
  else
    sprintf(buf,_("Unlocked all %i sets.").c_str(),setlist.size());
    
  wgh->setDisplay(buf);

  listview->Entering(0);
  detailview = NULL; 
  setSrc = NULL;
  showMenu = false;
  mBgTex = resources.RetrieveTexture("awardback.jpg",TEXTURE_SUB_5551);
  mBg = resources.RetrieveQuad("awardback.jpg");
}

void GameStateAwards::Create()
{
}
void GameStateAwards::Destroy()
{
}


void GameStateAwards::Render()
{
    JRenderer * r = JRenderer::GetInstance();
    r->ClearScreen(ARGB(0,0,0,0));
    if(mBg)
      r->RenderQuad(mBg, 0, 0);
 
    switch(mState){
      case STATE_LISTVIEW:
        if(listview)
          listview->Render();
        break;
      case STATE_DETAILS:
        if(detailview)
          detailview->Render();
        break;
    }
  
    if(showMenu && menu)
      menu->Render();
}

void GameStateAwards::Update(float dt)
{
  if(mEngine->GetButtonClick(PSP_CTRL_TRIANGLE))
    options[Options::DISABLECARDS].number = !options[Options::DISABLECARDS].number;

  if(showMenu){
    menu->Update(dt);
  }
  else{
    switch(mState){
      case STATE_LISTVIEW:
        if(listview)
          listview->Update(dt);
        break;
      case STATE_DETAILS:
        if(detailview)
          detailview->Update(dt);
        break;
    }
    switch(mEngine->ReadButton()){
      case PSP_CTRL_START:
        showMenu = true;

        SAFE_DELETE(menu);
        menu = NEW SimpleMenu(-102, this,Constants::MENU_FONT, 50,170);
        if(mState == STATE_DETAILS)
          menu->Add(2, "Back to Trophies");
        menu->Add(1, "Back to Main Menu");
        menu->Add(3, "Cancel");
        break;
      case PSP_CTRL_LTRIGGER:
        mParent->SetNextState(GAME_STATE_MENU);
        break;
      case PSP_CTRL_CROSS:
        if(mState == STATE_LISTVIEW)
          mParent->SetNextState(GAME_STATE_MENU);
        else{
          mState = STATE_LISTVIEW;
          SAFE_DELETE(detailview);
        }
        break;
    }

  }

  if(setSrc)
    setSrc->Update(dt);
}

bool GameStateAwards::enterSet(int setid){
  MTGSetInfo * si = setlist.getInfo(setid);
  map<int, MTGCard *>::iterator it;

  if(!si)
    return false;

  SAFE_DELETE(detailview);
  SAFE_DELETE(setSrc);

  setSrc = NEW WSrcMTGSet(setid);
  detailview = NEW WGuiMenu(PSP_CTRL_DOWN,PSP_CTRL_UP);
  
  WGuiList * spoiler = NEW WGuiList("Spoiler",setSrc);
  spoiler->setX(210);
  spoiler->setWidth(SCREEN_WIDTH - 220);
  while(true){
    MTGCard * c = setSrc->getCard();
    if(c)
      spoiler->Add(NEW WGuiItem(c->data->name));
    if(!setSrc->next())
      break;
  }
  setSrc->setPos(0);
  spoiler->Entering(0);
  WGuiCardImage * wi = NEW WGuiCardImage(setSrc);
  wi->setX(105);
  wi->setY(137);
  detailview->Add(wi);
  detailview->Add(spoiler);
  detailview->Entering(0);
  return true;
}
bool GameStateAwards::enterStats(int option){
  if(option != Options::AWARD_COLLECTOR)
    return false;
  DeckDataWrapper* ddw = NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(PLAYER_COLLECTION).c_str(), mParent->collection));
  if(!ddw)
    return false;

  SAFE_DELETE(detailview);
  detailview = NEW WGuiList("Details");
  
  detailview->Add(NEW WGuiHeader("Collection Stats"));
  detailview->Entering(0);

  //Discover favorite set and unique cards
  int unique = 0;

  if(setlist.size() > 0){
    int * counts = (int*)calloc(setlist.size(),sizeof(int));
    int setid = -1;
    int dupes = 0;
    MTGCard * many = NULL;
    MTGCard * costly = NULL;
    MTGCard * strong = NULL;
    MTGCard * tough = NULL;
    map<MTGCard *,int,Cmp1>::iterator it;

    for (it = ddw->cards.begin(); it!=ddw->cards.end(); it++){
      MTGCard * c = it->first;
      if(!c)
        continue;
      if(!c->data->isLand() && (many == NULL || it->second > dupes)){
        many = c;
        dupes = it->second;
      }
      unique++;
      counts[c->setId]+=it->second;
      if(costly == NULL 
        || c->data->getManaCost()->getConvertedCost() > costly->data->getManaCost()->getConvertedCost())
        costly = c;

      if(c->data->isCreature() && (strong == NULL || c->data->getPower() > strong->data->getPower()))
        strong = c;

      if(c->data->isCreature() && (tough == NULL || c->data->getToughness() > tough->data->getToughness()))
        tough = c;

    }
    for(int i=0;i<setlist.size();i++){
      if(setid < 0 || counts[i] > counts[setid])
        setid = i;      
    }
    free(counts);

    char buf[1024];
    sprintf(buf,_("Total Value: %ic").c_str(),ddw->totalPrice());
    detailview->Add(NEW WGuiItem(buf));//ddw->colors
    
    sprintf(buf,_("Total Cards (including duplicates): %i").c_str(),ddw->getCount());
    detailview->Add(NEW WGuiItem(buf));//ddw->colors

    sprintf(buf,_("Unique Cards: %i").c_str(),unique);
    detailview->Add(NEW WGuiItem(buf));

    if(many){
      sprintf(buf,_("Most Duplicates: %i (%s)").c_str(),dupes,many->data->getName().c_str());
      detailview->Add(NEW WGuiItem(buf));
    }
    if(setid >= 0){
    sprintf(buf,_("Favorite Set: %s").c_str(),setlist[setid].c_str());
    detailview->Add(NEW WGuiItem(buf));
    }
    if(costly){
      sprintf(buf,_("Highest Mana Cost: %i (%s)").c_str(),costly->data->getManaCost()->getConvertedCost(),costly->data->getName().c_str());
      detailview->Add(NEW WGuiItem(buf));
    }
    if(strong){
      sprintf(buf,_("Most Powerful: %i (%s)").c_str(),strong->data->getPower(),strong->data->getName().c_str());
      detailview->Add(NEW WGuiItem(buf));
    }
    if(tough){
      sprintf(buf,_("Toughest: %i (%s)").c_str(),tough->data->getToughness(),strong->data->getName().c_str());
      detailview->Add(NEW WGuiItem(buf));
    }
  }

  return true;
}
void GameStateAwards::ButtonPressed(int controllerId, int controlId)
{
   if(controllerId == -102)
  switch (controlId){
    case 1:
      mParent->SetNextState(GAME_STATE_MENU);
      showMenu = false;
      break;
    case 2:
      mState = STATE_LISTVIEW;
      SAFE_DELETE(detailview);
      showMenu = false;
      break;
    case 3:
      showMenu = false;
      break;
   }
   else if(controllerId == -103){
     int setid = controlId-Options::SET_UNLOCKS;

     if(controlId >= Options::SET_UNLOCKS && enterSet(setid)){
       mState = STATE_DETAILS;
       mDetailItem = controlId;
       
     }else if(controlId == Options::AWARD_COLLECTOR && enterStats(controlId)){
       mState = STATE_DETAILS;
     }
   }
}