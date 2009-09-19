#include <math.h>
#include "../include/config.h"
#include "../include/GameStateMenu.h"
#include "../include/MenuItem.h"
#include "../include/GameOptions.h"
#include "../include/GameApp.h"
#include "../include/MTGCard.h"
#include "../include/Translate.h"
#include "../include/DeckStats.h"
#include "../include/PlayerData.h"
#include "../include/utils.h"
#include "../include/DeckDataWrapper.h"

static const char* GAME_VERSION = "WTH?! 0.8.1 - by wololo";

#define DEFAULT_ANGLE_MULTIPLIER 0.4
#define MAX_ANGLE_MULTIPLIER (3*M_PI)
#define MIN_ANGLE_MULTIPLIER 0.4
static const double STEP_ANGLE_MULTIPLIER = 0.0002;


enum ENUM_MENU_STATE_MAJOR
  {
    MENU_STATE_MAJOR_MAINMENU = 0x01,
    MENU_STATE_MAJOR_SUBMENU = 0x02,
    MENU_STATE_MAJOR_LOADING_MENU = 0x03,
    MENU_STATE_MAJOR_LOADING_CARDS = 0x04,
    MENU_STATE_MAJOR_FIRST_TIME = 0x05,
    MENU_STATE_MAJOR_DUEL = 0x06,

    MENU_STATE_MAJOR = 0xFF
  };

enum ENUM_MENU_STATE_MINOR
  {
    MENU_STATE_MINOR_NONE = 0,
    MENU_STATE_MINOR_SUBMENU_CLOSING = 0x100,

    MENU_STATE_MINOR = 0xF00
  };


enum
{
  MENUITEM_PLAY,
  MENUITEM_DECKEDITOR,
  MENUITEM_SHOP,
  MENUITEM_OPTIONS,
  MENUITEM_EXIT,
  SUBMENUITEM_1PLAYER,
  SUBMENUITEM_2PLAYER,
  SUBMENUITEM_DEMO,
  SUBMENUITEM_CANCEL,
  SUBMENUITEM_TESTSUITE,
  SUBMENUITEM_MOMIR,
  SUBMENUITEM_CLASSIC,
  SUBMENUITEM_RANDOM1,
  SUBMENUITEM_RANDOM2,
};


GameStateMenu::GameStateMenu(GameApp* parent): GameState(parent)
{
  mGuiController = NULL;
  subMenuController = NULL;
  gameTypeMenu = NULL;
  mSplash = NULL;
  mBg = NULL;
  mMovingW = NULL;
  //bgMusic = NULL;
  timeIndex = 0;
  angleMultiplier = MIN_ANGLE_MULTIPLIER;
  yW = 55;
  mVolume = 0;
  scroller = NULL;
}

GameStateMenu::~GameStateMenu() {}


void GameStateMenu::Create()
{
  mDip = NULL;
  mReadConf = 0;
  mCurrentSetName[0] = 0;

  //load all the icon images. Menu icons are managed, so we can do this here.
  int n = 0;
  char buf[512];

  for (int i=0;i<5;i++){
    for (int j=0;j<2;j++){
      sprintf(buf,"menuicons%d%d",i,j);
	    mIcons[n] = resources.RetrieveQuad("menuicons.png", 2 + i*36, 2 + j*36, 32, 32,buf);
	    mIcons[n]->SetHotSpot(16,16);
	    n++;
	  }
  }

  JLBFont * mFont = resources.GetJLBFont(Constants::MENU_FONT);
  mFont->SetColor(options[Metrics::LOADING_TC].asColor());
  mGuiController = NEW JGuiController(100, this);
  if (mGuiController)
    {
      mGuiController->Add(NEW MenuItem(MENUITEM_PLAY, mFont, "Play", 80,         50 + SCREEN_HEIGHT/2, mIcons[8], mIcons[9],"graphics/particle1.psi",resources.GetQuad("particles"),  true));
      mGuiController->Add(NEW MenuItem(MENUITEM_DECKEDITOR, mFont, "Deck Editor", 160, 50 + SCREEN_HEIGHT/2, mIcons[2], mIcons[3],"graphics/particle2.psi",resources.GetQuad("particles")));
      mGuiController->Add(NEW MenuItem(MENUITEM_SHOP, mFont, "Shop", 240,        50 + SCREEN_HEIGHT/2, mIcons[0], mIcons[1],"graphics/particle3.psi",resources.GetQuad("particles")));
      mGuiController->Add(NEW MenuItem(MENUITEM_OPTIONS, mFont, "Options", 320,     50 + SCREEN_HEIGHT/2, mIcons[6], mIcons[7],"graphics/particle4.psi",resources.GetQuad("particles")));
      mGuiController->Add(NEW MenuItem(MENUITEM_EXIT, mFont, "Exit", 400,        50 + SCREEN_HEIGHT/2, mIcons[4], mIcons[5],"graphics/particle5.psi",resources.GetQuad("particles")));
    }

  currentState = MENU_STATE_MAJOR_LOADING_CARDS | MENU_STATE_MINOR_NONE;
  scroller = NEW TextScroller(resources.GetJLBFont(Constants::MAIN_FONT), SCREEN_WIDTH/2 - 90 , SCREEN_HEIGHT-17,180);
  scrollerSet = 0;
}



void GameStateMenu::Destroy()
{
  SAFE_DELETE(mGuiController);
  SAFE_DELETE(subMenuController);
  SAFE_DELETE(gameTypeMenu);
  resources.Release(bgTexture);
  resources.Release(movingWTexture);
  SAFE_DELETE(scroller);
}

void GameStateMenu::Start(){
  subMenuController = NULL;

  if (GameApp::HasMusic && !GameApp::music && options[Options::MUSICVOLUME].number > 0){
    GameApp::music = resources.ssLoadMusic("Track0.mp3");
    JSoundSystem::GetInstance()->PlayMusic(GameApp::music, true);
  }

  if (GameApp::HasMusic && GameApp::music && options[Options::MUSICVOLUME].number == 0){
    JSoundSystem::GetInstance()->StopMusic(GameApp::music);
    SAFE_DELETE(GameApp::music);
  }

  hasChosenGameType = 1;
  if (options[Options::MOMIR_MODE_UNLOCKED].number) hasChosenGameType = 0;
  if (options[Options::RANDOMDECK_MODE_UNLOCKED].number) hasChosenGameType = 0;
  
  bgTexture = resources.RetrieveTexture("menutitle.png", RETRIEVE_VRAM);
  movingWTexture = resources.RetrieveTexture("movingW.png", RETRIEVE_VRAM);
  mBg = resources.RetrieveQuad("menutitle.png", 0, 0, 256, 166);		// Create background quad for rendering.
  mMovingW = resources.RetrieveQuad("movingW.png", 2, 2, 84, 62);

  mBg->SetHotSpot(105,50);
  mMovingW->SetHotSpot(72,16);
  JRenderer::GetInstance()->ResetPrivateVRAM();
  JRenderer::GetInstance()->EnableVSync(true);

  //How many cards total ?
  PlayerData * playerdata = NEW PlayerData(mParent->collection);
  if(playerdata && !options[Options::ACTIVE_PROFILE].isDefault())
    sprintf(nbcardsStr, "%s: %i cards (%i)", options[Options::ACTIVE_PROFILE].str.c_str(), playerdata->collection->totalCards(), mParent->collection->totalCards());
  else
    sprintf(nbcardsStr, "Database: %i cards", mParent->collection->totalCards());

  SAFE_DELETE(playerdata);

#if defined DEBUG_CACHE
  resources.ClearUnlocked();  //So we can tell if we've any extra locks.

  if(!resources.menuCached)  
    resources.menuCached = resources.CountCached();
    
  if(resources.CountCached() != resources.menuCached){
    char buf[512];
    unsigned int i = resources.CountCached()-resources.menuCached;
    sprintf(buf,"Warning: %u leftover locked items.",i);
    resources.debugMessage = buf;
  }
  
#endif
}


void GameStateMenu::fillScroller(){
  scroller->Reset();
  char buffer[4096];
  char buff2[512];

  DeckStats * stats = DeckStats::GetInstance();
  int totalGames = 0;

  for (int j=1; j<6; j++){
    
    sprintf(buffer, "stats/player_deck%i.txt",j);
    string deckstats = options.profileFile(buffer);
    if(fileExists(deckstats.c_str())){
		  stats->load(deckstats.c_str());
		  int percentVictories = stats->percentVictories();

      sprintf(buff2, _("You have a %i%% victory ratio with Deck%i").c_str(),percentVictories,j);
      scroller->Add(buff2);
      int nbGames = stats->nbGames();
      totalGames+= nbGames;
      sprintf(buff2, _("You have played %i games with Deck%i").c_str(),nbGames,j);
      scroller->Add(buff2);
    }
  }
  if (totalGames){
    sprintf(buff2, _("You have played a total of %i games").c_str(),totalGames);
      scroller->Add(buff2);
  }

  if (!options[Options::DIFFICULTY_MODE_UNLOCKED].number)
    scroller->Add(_("Unlock the difficult mode for more challenging duels!"));
  if (!options[Options::MOMIR_MODE_UNLOCKED].number)
    scroller->Add(_("Interested in playing Momir Basic? You'll have to unlock it first :)"));
  if (!options[Options::RANDOMDECK_MODE_UNLOCKED].number)
    scroller->Add(_("You haven't locked the random deck mode yet"));
  if (!options[Options::EVILTWIN_MODE_UNLOCKED].number)
    scroller->Add(_("You haven't unlocked the evil twin mode yet"));
  if (!options[Options::RANDOMDECK_MODE_UNLOCKED].number)
    scroller->Add(_("You haven't unlocked the random deck mode yet"));
  if (!options[Options::EVILTWIN_MODE_UNLOCKED].number)
    scroller->Add(_("You haven't unlocked the evil twin mode yet"));

  //Unlocked sets
  int nbunlocked = 0;
  for (int i = 0; i < MtgSets::SetsList->nb_items; i++){
    string s = MtgSets::SetsList->values[i];
    sprintf(buffer,"unlocked_%s", s.c_str());
    if (1 == options[buffer].number) nbunlocked++;
  }
  sprintf(buff2, _("You have unlocked %i expansions out of %i").c_str(),nbunlocked, MtgSets::SetsList->nb_items);
  scroller->Add(buff2);

  DeckDataWrapper* ddw = NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(PLAYER_COLLECTION,"",false).c_str(), mParent->collection));
  int totalCards = ddw->getCount();
  if (totalCards){
    sprintf(buff2, _("You have a total of %i cards in your collection").c_str(),totalCards);
      scroller->Add(buff2);

      int estimatedValue = ddw->totalPrice();
      sprintf(buff2, _("The shopkeeper would buy your entire collection for around %i credits").c_str(),estimatedValue/2);
      scroller->Add(buff2);

      sprintf(buff2, _("The cards in your collection have an average value of %i credits").c_str(),estimatedValue/totalCards);
      scroller->Add(buff2);
  }
  delete ddw;

  PlayerData * playerdata = NEW PlayerData(mParent->collection);
  sprintf(buff2, _("You currently have %i credits").c_str(),playerdata->credits);
  SAFE_DELETE(playerdata);
  scroller->Add(buff2);

  scroller->Add(_("More cards and mods at http://wololo.net/wagic"));

  scroller->Add(_("These stats will be updated next time you run Wagic"));

  scrollerSet = 1;
  scroller->setRandom();
}
void GameStateMenu::resetDirectory(){
  if(mDip != NULL)  { 
    closedir(mDip);
    mDip = NULL;
  }
}
int GameStateMenu::nextDirectory(const char * root, const char * file){
  int found = 0;
  if (!mDip){
    mDip = opendir(root);
  }

  while (!found && (mDit = readdir(mDip))){
    sprintf(mCurrentSetFileName, "%s/%s/%s", root, mDit->d_name, file);
    std::ifstream file(mCurrentSetFileName);
    if(file){
      sprintf(mCurrentSetName, "%s", mDit->d_name);
      file.close();
      found = 1;
    }
  }
  if (!mDit) {
    closedir(mDip);
    mDip = NULL;
  }
  return found;
}

void GameStateMenu::End()
{
  JRenderer::GetInstance()->EnableVSync(false);
  
  resources.Release(bgTexture);
  resources.Release(movingWTexture);
}



void GameStateMenu::Update(float dt)
{

  timeIndex += dt * 2;
  switch (MENU_STATE_MAJOR & currentState)
    {
    case MENU_STATE_MAJOR_LOADING_CARDS :
      if (mReadConf){
	      mParent->collection->load(mCurrentSetFileName, mCurrentSetName);
      }else{
	      mReadConf = 1;
      }
      if (!nextDirectory(RESPATH"/sets/","_cards.dat")){

        //Force default, if necessary.
        if(options[Options::ACTIVE_PROFILE].str == "") 
          options[Options::ACTIVE_PROFILE].str = "Default";

        //check for deleted collection / first-timer
        std::ifstream file(options.profileFile(PLAYER_COLLECTION,"",false).c_str());
	      if(file){
	        file.close();
          resources.Release(mSplash);
	        currentState = MENU_STATE_MAJOR_MAINMENU | MENU_STATE_MINOR_NONE;
	      }else{
  	      currentState = MENU_STATE_MAJOR_FIRST_TIME | MENU_STATE_MINOR_NONE;
        }
        
        //List active profile and database size.        
        PlayerData * playerdata = NEW PlayerData(mParent->collection);
        if(playerdata && !options[Options::ACTIVE_PROFILE].isDefault())
          sprintf(nbcardsStr, "%s: %i cards (%i)", options[Options::ACTIVE_PROFILE].str.c_str(), playerdata->collection->totalCards(), mParent->collection->totalCards());
        else
          sprintf(nbcardsStr, "Database: %i cards", mParent->collection->totalCards());
        SAFE_DELETE(playerdata);
        resetDirectory();
      }
      break;    
    case MENU_STATE_MAJOR_FIRST_TIME :
      currentState = MENU_STATE_MAJOR_MAINMENU | MENU_STATE_MINOR_NONE;
      options.checkProfile(); //Handles building a new deck, if needed.
      break;
    case MENU_STATE_MAJOR_MAINMENU :
      if (!scrollerSet) fillScroller();
      if (NULL != mGuiController)
	      mGuiController->Update(dt);
      break;
    case MENU_STATE_MAJOR_SUBMENU :
      subMenuController->Update(dt);
      mGuiController->Update(dt);
      break;
    case MENU_STATE_MAJOR_DUEL :
      if (MENU_STATE_MINOR_NONE == (currentState & MENU_STATE_MINOR))
	{
	  if (!hasChosenGameType){
	    currentState = MENU_STATE_MAJOR_SUBMENU;
	    JLBFont * mFont = resources.GetJLBFont(Constants::MENU_FONT);
	    subMenuController = NEW SimpleMenu(102, this, mFont, 150,60);
	    if (subMenuController){
	      subMenuController->Add(SUBMENUITEM_CLASSIC,"Classic");
	      if (options[Options::MOMIR_MODE_UNLOCKED].number)
		subMenuController->Add(SUBMENUITEM_MOMIR, "Momir Basic");
	      if (options[Options::RANDOMDECK_MODE_UNLOCKED].number){
		subMenuController->Add(SUBMENUITEM_RANDOM1, "Random 1 Color");
		subMenuController->Add(SUBMENUITEM_RANDOM2, "Random 2 Colors");
	      }
	      subMenuController->Add(SUBMENUITEM_CANCEL, "Cancel");
	    }
	  }else{
	    mParent->SetNextState(GAME_STATE_DUEL);
	    currentState = MENU_STATE_MAJOR_MAINMENU;
	  }
	}
    }
  switch (MENU_STATE_MINOR & currentState)
    {
    case MENU_STATE_MINOR_SUBMENU_CLOSING :
      if (subMenuController->closed)
	{
	  SAFE_DELETE(subMenuController);
	  currentState &= ~MENU_STATE_MINOR_SUBMENU_CLOSING;
	}
      else
	subMenuController->Update(dt);
      break;
    case MENU_STATE_MINOR_NONE :
      ;// Nothing to do.
    }
  if (yW <= 55)
    {
      if (mEngine->GetButtonState(PSP_CTRL_SQUARE)) angleMultiplier += STEP_ANGLE_MULTIPLIER;
      else angleMultiplier *= 0.9999;
      if (angleMultiplier > MAX_ANGLE_MULTIPLIER) angleMultiplier = MAX_ANGLE_MULTIPLIER;
      else if (angleMultiplier < MIN_ANGLE_MULTIPLIER) angleMultiplier = MIN_ANGLE_MULTIPLIER;

      if (mEngine->GetButtonState(PSP_CTRL_TRIANGLE) && (dt != 0))
	{
	  angleMultiplier = (cos(timeIndex)*angleMultiplier - M_PI/3 - 0.1 - angleW) / dt;
	  yW = yW + 5*dt + (yW - 55) *5*  dt;
	}
      else
	angleW = cos(timeIndex)*angleMultiplier - M_PI/3 - 0.1;
    }
  else
    {
      angleW += angleMultiplier * dt;
      yW = yW + 5*dt + (yW - 55) *5*dt;
    }

  scroller->Update(dt);
}





void GameStateMenu::Render()
{
  JRenderer * renderer = JRenderer::GetInstance();
  renderer->ClearScreen(ARGB(0,0,0,0));
  JLBFont * mFont = resources.GetJLBFont(Constants::MENU_FONT);
  if ((currentState & MENU_STATE_MAJOR) == MENU_STATE_MAJOR_LOADING_CARDS){
    if(!mSplash)
     mSplash = resources.RetrieveQuad("splash.jpg");
    if (mSplash){
      renderer->RenderQuad(mSplash,0,0);
    }else{
      char text[512];
      sprintf(text, _("LOADING SET: %s").c_str(), mCurrentSetName);
      mFont->DrawString(text,SCREEN_WIDTH/2,SCREEN_HEIGHT/2,JGETEXT_CENTER);
    }
  }else{
    mFont = resources.GetJLBFont(Constants::MAIN_FONT);
    PIXEL_TYPE colors[] =
      {
	      ARGB(255, 3, 2, 0),
	      ARGB(255, 8, 3, 0),
	      ARGB(255,21,12, 0),
	      ARGB(255,50,34, 0)
      };

    renderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,colors);
    renderer->RenderQuad(mBg, SCREEN_WIDTH/2, 50);
    if (yW < 2*SCREEN_HEIGHT) renderer->RenderQuad(mMovingW, SCREEN_WIDTH/2 - 10, yW, angleW);
    if (mGuiController!=NULL)
      mGuiController->Render();

    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    mFont->SetColor(options[Metrics::STATS_TC].asColor(ARGB(128,255,255,255)));
    mFont->DrawString(GAME_VERSION, SCREEN_WIDTH-10,5,JGETEXT_RIGHT);
    mFont->DrawString(nbcardsStr,10, 5);
    mFont->SetScale(1.f);
    mFont->SetColor(options[Metrics::SCROLLER_TC].asColor());

    renderer->FillRoundRect(SCREEN_WIDTH/2 - 100,SCREEN_HEIGHT-20, 191,6,5,options[Metrics::SCROLLER_FC].asColor(ARGB(100,10,5,0)));
    scroller->Render();

    if (subMenuController){
      subMenuController->Render();
    }
  }

}


void GameStateMenu::ButtonPressed(int controllerId, int controlId)
{
JLBFont * mFont = resources.GetJLBFont(Constants::MENU_FONT);
#if defined (WIN32) || defined (LINUX)
  char buf[4096];
  sprintf(buf, "cnotrollerId: %i", controllerId);
  OutputDebugString(buf);
#endif
  switch (controllerId){
  case 101:
    options.createUsersFirstDeck(controlId);
    currentState = MENU_STATE_MAJOR_MAINMENU | MENU_STATE_MINOR_NONE;
    break;
  default:
    switch (controlId)
      {
      case MENUITEM_PLAY:
	    subMenuController = NEW SimpleMenu(102, this, mFont, 150,60);
	    if (subMenuController){
	      subMenuController->Add(SUBMENUITEM_1PLAYER,"1 Player");
	      subMenuController->Add(SUBMENUITEM_2PLAYER, "2 Players");
	      subMenuController->Add(SUBMENUITEM_DEMO,"Demo");
	      subMenuController->Add(SUBMENUITEM_CANCEL, "Cancel");
#ifdef TESTSUITE
	      subMenuController->Add(SUBMENUITEM_TESTSUITE, "Test Suite");
#endif
	  currentState = MENU_STATE_MAJOR_SUBMENU | MENU_STATE_MINOR_NONE;
	    }
	    break;
      case MENUITEM_DECKEDITOR:
	mParent->SetNextState(GAME_STATE_DECK_VIEWER);
	break;
      case MENUITEM_SHOP:
	mParent->SetNextState(GAME_STATE_SHOP);
	break;
      case MENUITEM_OPTIONS:
	mParent->SetNextState(GAME_STATE_OPTIONS);
	break;
      case MENUITEM_EXIT:
	mEngine->End();
	break;
      case SUBMENUITEM_1PLAYER:
	mParent->players[0] = PLAYER_TYPE_HUMAN;
	mParent->players[1] = PLAYER_TYPE_CPU;
	subMenuController->Close();
	currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
	break;
      case SUBMENUITEM_2PLAYER:
	mParent->players[0] = PLAYER_TYPE_HUMAN;
	mParent->players[1] = PLAYER_TYPE_HUMAN;
	subMenuController->Close();
	currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
	break;
      case SUBMENUITEM_DEMO:
	mParent->players[0] = PLAYER_TYPE_CPU;
	mParent->players[1] = PLAYER_TYPE_CPU;
	subMenuController->Close();
	currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
	break;
      case SUBMENUITEM_CANCEL:
	subMenuController->Close();
	currentState = MENU_STATE_MAJOR_MAINMENU | MENU_STATE_MINOR_SUBMENU_CLOSING;
	break;

  case SUBMENUITEM_CLASSIC:
    this->hasChosenGameType = 1;
    mParent->gameType = GAME_TYPE_CLASSIC;
    subMenuController->Close();
    currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
    break;

  case SUBMENUITEM_MOMIR:
    this->hasChosenGameType = 1;
    mParent->gameType = GAME_TYPE_MOMIR;
    subMenuController->Close();
    currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
    break;

  case SUBMENUITEM_RANDOM1:
    this->hasChosenGameType = 1;
    mParent->gameType = GAME_TYPE_RANDOM1;
    subMenuController->Close();
    currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
    break;

  case SUBMENUITEM_RANDOM2:
    this->hasChosenGameType = 1;
    mParent->gameType = GAME_TYPE_RANDOM2;
    subMenuController->Close();
    currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
    break;
#ifdef TESTSUITE
      case SUBMENUITEM_TESTSUITE:
	mParent->players[0] = PLAYER_TYPE_TESTSUITE;
	mParent->players[1] = PLAYER_TYPE_TESTSUITE;
	subMenuController->Close();
	currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
	break;
#endif
      }
    break;
  }
}

ostream& GameStateMenu::toString(ostream& out) const
{
  return out << "GameStateMenu ::: scroller : " << scroller
	     << " ; scrollerSet : " << scrollerSet
	     << " ; mGuiController : " << mGuiController
	     << " ; subMenuController : " << subMenuController
	     << " ; gameTypeMenu : " << gameTypeMenu
	     << " ; hasChosenGameType : " << hasChosenGameType
	     << " ; mIcons : " << mIcons
	     << " ; bgTexture : " << bgTexture
	     << " ; movingWTexture : " << movingWTexture
	     << " ; mBg : " << mBg
	     << " ; mMovingW : " << mMovingW
	     << " ; mCreditsYPos : " << mCreditsYPos
	     << " ; currentState : " << currentState
	     << " ; mVolume : " << mVolume
	     << " ; nbcardsStr : " << nbcardsStr
	     << " ; mDip : " << mDip
	     << " ; mDit : " << mDit
	     << " ; mCurrentSetName : " << mCurrentSetName
	     << " ; mCurrentSetFileName : " << mCurrentSetFileName
	     << " ; mReadConf : " << mReadConf
	     << " ; timeIndex : " << timeIndex
	     << " ; angleMultiplier : " << angleMultiplier
	     << " ; angleW : " << angleW
	     << " ; yW : " << yW;
}
