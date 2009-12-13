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

  if(saveMe)
	  options.save();
}
void GameStateAwards::Start()
{ 
  char buf[256];
  mState = STATE_LISTVIEW;

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
  wgh = NEW WGuiHeader("");
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

  listview->Add(NEW WGuiHeader("Click a set for details."));
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
    
      
    aw = NEW WGuiAward(Options::optionSet(i),si->getName(),buf);
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
    JQuad * bg = resources.RetrieveTempQuad("awardback.jpg");
    r->RenderQuad(bg, 0, 0);
 
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
  MTGAllCards * c = GameApp::collection;
  for(it = c->collection.begin();it!=c->collection.end();it++){
   if(it->second && it->second->setId == setid)
     spoiler->Add(NEW WGuiItem(it->second->name));
  }
  spoiler->Entering(0);
  WGuiCardImage * wi = NEW WGuiCardImage(setSrc);
  wi->setX(105);
  wi->setY(137);
  detailview->Add(wi);
  detailview->Add(spoiler);
  detailview->Entering(0);
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
    //Enter "Details Mode" for that item. TODO: Details for non-sets
     if(controlId >= Options::SET_UNLOCKS){
       mState = STATE_DETAILS;
       mDetailItem = controlId;
       int setid = controlId-Options::SET_UNLOCKS;
       enterSet(setid);
     }
   }
}