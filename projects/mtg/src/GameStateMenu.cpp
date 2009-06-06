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

static const char* GAME_VERSION = "WTH?! 0.6.2 - by WilLoW";
#define ALPHA_WARNING 0

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
    MENU_STATE_MAJOR_WARNING = 0x06,
    MENU_STATE_MAJOR_DUEL = 0x07,

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
};


GameStateMenu::GameStateMenu(GameApp* parent): GameState(parent)
{
  mGuiController = NULL;
  subMenuController = NULL;
  gameTypeMenu = NULL;
  mIconsTexture = NULL;
  //bgMusic = NULL;
  timeIndex = 0;
  angleMultiplier = MIN_ANGLE_MULTIPLIER;
  yW = 55;
  mVolume = 0;
  splashTex = NULL;
  splashQuad = NULL;
  scroller = NULL;
}

GameStateMenu::~GameStateMenu() {}

void GameStateMenu::Create()
{


  mDip = NULL;
  mReadConf = 0;
  mCurrentSetName[0] = 0;

  mIconsTexture = JRenderer::GetInstance()->LoadTexture("graphics/menuicons.png", TEX_TYPE_USE_VRAM);
  bgTexture = JRenderer::GetInstance()->LoadTexture("graphics/menutitle.png", TEX_TYPE_USE_VRAM);
  movingWTexture = JRenderer::GetInstance()->LoadTexture("graphics/movingW.png", TEX_TYPE_USE_VRAM);
  mBg = NEW JQuad(bgTexture, 0, 0, 256, 166);		// Create background quad for rendering.
  mMovingW = NEW JQuad(movingWTexture, 2, 2, 84, 62);
  if (fileExists("graphics/splash.jpg")){
    splashTex = JRenderer::GetInstance()->LoadTexture("graphics/splash.jpg", TEX_TYPE_USE_VRAM);
    splashQuad = NEW JQuad(splashTex, 0, 0, 480, 272);
  }
  mBg->SetHotSpot(105,50);
  mMovingW->SetHotSpot(72,16);
  //load all the icon images
  int n = 0;
  for (int i=0;i<5;i++)
    {
      for (int j=0;j<2;j++)
	{
	  mIcons[n] = NEW JQuad(mIconsTexture, 2 + i*36, 2 + j*36, 32, 32);
	  mIcons[n]->SetHotSpot(16,16);
	  n++;
	}
    }

  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MENU_FONT);
  //mFont->SetBase(0);	// using 2nd font
  mGuiController = NEW JGuiController(100, this);
  //mGuiController->SetShadingBackground(10, 45, 80, 100, ARGB(255,0,0,0));
  if (mGuiController)
    {
      mGuiController->Add(NEW MenuItem(MENUITEM_PLAY, mFont, "Play", 80,         50 + SCREEN_HEIGHT/2, mIcons[8], mIcons[9],"graphics/particle1.psi",GameApp::CommonRes->GetQuad("particles"),  true));
      mGuiController->Add(NEW MenuItem(MENUITEM_DECKEDITOR, mFont, "Deck Editor", 160, 50 + SCREEN_HEIGHT/2, mIcons[2], mIcons[3],"graphics/particle2.psi",GameApp::CommonRes->GetQuad("particles")));
      mGuiController->Add(NEW MenuItem(MENUITEM_SHOP, mFont, "Shop", 240,        50 + SCREEN_HEIGHT/2, mIcons[0], mIcons[1],"graphics/particle3.psi",GameApp::CommonRes->GetQuad("particles")));
      mGuiController->Add(NEW MenuItem(MENUITEM_OPTIONS, mFont, "Options", 320,     50 + SCREEN_HEIGHT/2, mIcons[6], mIcons[7],"graphics/particle4.psi",GameApp::CommonRes->GetQuad("particles")));
      mGuiController->Add(NEW MenuItem(MENUITEM_EXIT, mFont, "Exit", 400,        50 + SCREEN_HEIGHT/2, mIcons[4], mIcons[5],"graphics/particle5.psi",GameApp::CommonRes->GetQuad("particles")));
    }

  currentState = MENU_STATE_MAJOR_LOADING_CARDS | MENU_STATE_MINOR_NONE;
  scroller = NEW TextScroller(GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT), SCREEN_WIDTH/2 - 100 , SCREEN_HEIGHT-15,200);
  scrollerSet = 0;
}



void GameStateMenu::Destroy()
{
  SAFE_DELETE(mGuiController);
  SAFE_DELETE(subMenuController);
  SAFE_DELETE(gameTypeMenu);
  SAFE_DELETE(mIconsTexture);

  for (int i = 0; i < 10 ; i++){
    SAFE_DELETE(mIcons[i]);
  }

  SAFE_DELETE(mBg);
  SAFE_DELETE(mMovingW);
  SAFE_DELETE(movingWTexture);
  SAFE_DELETE(bgTexture);
  SAFE_DELETE(scroller);

  //SAFE_DELETE (bgMusic);
}



void GameStateMenu::Start(){
  JRenderer::GetInstance()->ResetPrivateVRAM();
  JRenderer::GetInstance()->EnableVSync(true);
  subMenuController = NULL;

  if (GameApp::HasMusic && !GameApp::music && GameOptions::GetInstance()->values[OPTIONS_MUSICVOLUME].getIntValue() > 0){
    GameApp::music = JSoundSystem::GetInstance()->LoadMusic("sound/Track0.mp3");
    JSoundSystem::GetInstance()->PlayMusic(GameApp::music, true);
  }

  if (GameApp::HasMusic && GameApp::music && GameOptions::GetInstance()->values[OPTIONS_MUSICVOLUME].getIntValue() == 0){
    JSoundSystem::GetInstance()->StopMusic(GameApp::music);
    SAFE_DELETE(GameApp::music);
  }

  hasChosenGameType = 1;
  if (GameOptions::GetInstance()->values[OPTIONS_MOMIR_MODE_UNLOCKED].getIntValue()) hasChosenGameType =0;

  


}


void GameStateMenu::fillScroller(){
  scroller->Reset();
  char buffer[4096];
  char buff2[512];

  DeckStats * stats = DeckStats::GetInstance();
  int totalGames = 0;
  for (int j=1; j<6; j++){
    sprintf(buffer, RESPATH"/player/stats/player_deck%i.txt",j);
    if(fileExists(buffer)){
		  stats->load(buffer);
		  int percentVictories = stats->percentVictories();

      sprintf(buff2, "You have a %i%% victory ratio with Deck%i",percentVictories,j);
      scroller->Add(buff2);
      int nbGames = stats->nbGames();
      totalGames+= nbGames;
      sprintf(buff2, "You have played %i games with Deck%i",nbGames,j);
      scroller->Add(buff2);
    }
  }
  if (totalGames){
      sprintf(buff2, "You have played a total of %i games",totalGames);
      scroller->Add(buff2);
  }
  GameOptions * go = GameOptions::GetInstance();

  if (!go->values[OPTIONS_DIFFICULTY_MODE_UNLOCKED].getIntValue()){
    scroller->Add("Unlock the difficult mode for more challenging duels!");   
  }
  if (!go->values[OPTIONS_MOMIR_MODE_UNLOCKED].getIntValue()){
    scroller->Add("Interested in playing Momir Basic? You'll have to unlock it first :)");   
  }

  DeckDataWrapper* ddw = NEW DeckDataWrapper(NEW MTGDeck(RESPATH"/player/collection.dat", mParent->cache,mParent->collection));
  int totalCards = ddw->getCount();
  if (totalCards){
      sprintf(buff2, "You have a total of %i cards in your collection",totalCards);
      scroller->Add(buff2);

      int estimatedValue = ddw->totalPrice();
      sprintf(buff2, "The shopkeeper would buy your entire collection for around %i credits",estimatedValue/2);
      scroller->Add(buff2);

      sprintf(buff2, "The cards in your collection have an average value of %i credits",estimatedValue/totalCards);
      scroller->Add(buff2);
  }
  delete ddw;

  PlayerData * playerdata = NEW PlayerData(mParent->collection);
  sprintf(buff2, "You currently have %i credits",playerdata->credits);
  delete playerdata;
  scroller->Add(buff2);

  scroller->Add("Need more cards? Go to http://wololo.net/wagic");

  scroller->Add("These stats will be updated next time you run Wagic");

  scrollerSet = 1;
  scroller->setRandom();
}

int GameStateMenu::nextCardSet(){
  int found = 0;
  if (!mDip){
    mDip = opendir(RESPATH"/sets/");
  }

  while (!found && (mDit = readdir(mDip))){
    sprintf(mCurrentSetFileName, RESPATH"/sets/%s/_cards.dat", mDit->d_name);
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
      if (!nextCardSet()){
	      //How many cards total ?
	      sprintf(nbcardsStr, "Database: %i cards", mParent->collection->totalCards());
	      //Check for first time comer
	      std::ifstream file(RESPATH"/player/collection.dat");
	      if(file){
	        file.close();
	        currentState = MENU_STATE_MAJOR_WARNING | MENU_STATE_MINOR_NONE;
	      }else{
	        currentState = MENU_STATE_MAJOR_FIRST_TIME | MENU_STATE_MINOR_NONE;
	      }
        SAFE_DELETE(splashQuad);
        SAFE_DELETE(splashTex);
      }
      break;
    case MENU_STATE_MAJOR_FIRST_TIME :
      {
	//Give the player cards from the set for which we have the most variety
	int setId = 0;
	int maxcards = 0;
	for (int i=0; i< MtgSets::SetsList->nb_items; i++){
	  int value = mParent->collection->countBySet(i);
	  if (value > maxcards){
	    maxcards = value;
	    setId = i;
	  }
	}
	createUsersFirstDeck(setId);
      }
      currentState = MENU_STATE_MAJOR_WARNING | MENU_STATE_MINOR_NONE;
      break;
    case MENU_STATE_MAJOR_WARNING :
      if (!ALPHA_WARNING){
	currentState = MENU_STATE_MAJOR_MAINMENU | MENU_STATE_MINOR_NONE;
      }else{
	if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) currentState = MENU_STATE_MAJOR_MAINMENU | MENU_STATE_MINOR_NONE;
      }
      break;
    case MENU_STATE_MAJOR_MAINMENU :
      if (!scrollerSet) fillScroller();
      if (mGuiController!=NULL){
	      mGuiController->Update(dt);
      }

      
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
      JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MENU_FONT);
      subMenuController = NEW SimpleMenu(102, this, mFont, 150,60);
	    if (subMenuController){
	      subMenuController->Add(SUBMENUITEM_CLASSIC,"Classic");
	      subMenuController->Add(SUBMENUITEM_MOMIR, "Momir Basic");
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



void GameStateMenu::createUsersFirstDeck(int setId){
#if defined (WIN32) || defined (LINUX)
  char buf[4096];
  sprintf(buf, "setID: %i", setId);
  OutputDebugString(buf);
#endif
  MTGDeck *mCollection = NEW MTGDeck(RESPATH"/player/collection.dat", mParent->cache, mParent->collection);
  //10 lands of each
  if (!mCollection->addRandomCards(10, setId,Constants::RARITY_L,"Forest")){
    mCollection->addRandomCards(10, -1,Constants::RARITY_L,"Forest");
  }
  if (!mCollection->addRandomCards(10, setId,Constants::RARITY_L,"Plains")){
    mCollection->addRandomCards(10, -1,Constants::RARITY_L,"Plains");
  }
  if (!mCollection->addRandomCards(10, setId,Constants::RARITY_L,"Swamp")){
    mCollection->addRandomCards(10, -1,Constants::RARITY_L,"Swamp");
  }
  if (!mCollection->addRandomCards(10, setId,Constants::RARITY_L,"Mountain")){
    mCollection->addRandomCards(10, -1,Constants::RARITY_L,"Mountain");
  }
  if (!mCollection->addRandomCards(10, setId,Constants::RARITY_L,"Island")){
    mCollection->addRandomCards(10, -1,Constants::RARITY_L,"Island");
  }


#if defined (WIN32) || defined (LINUX)
  OutputDebugString("1\n");
#endif

  //Starter Deck
  mCollection->addRandomCards(3, setId,Constants::RARITY_R,NULL);
  mCollection->addRandomCards(9, setId,Constants::RARITY_U,NULL);
  mCollection->addRandomCards(48, setId,Constants::RARITY_C,NULL);

#if defined (WIN32) || defined (LINUX)
  OutputDebugString("2\n");
#endif
  //Boosters
  for (int i = 0; i< 2; i++){
    mCollection->addRandomCards(1, setId,Constants::RARITY_R);
    mCollection->addRandomCards(3, setId,Constants::RARITY_U);
    mCollection->addRandomCards(11, setId,Constants::RARITY_C);
  }
  mCollection->save();
  SAFE_DELETE(mCollection);
}



void GameStateMenu::Render()
{
  JRenderer * renderer = JRenderer::GetInstance();
  renderer->ClearScreen(ARGB(0,0,0,0));
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MENU_FONT);
  if ((currentState & MENU_STATE_MAJOR) == MENU_STATE_MAJOR_LOADING_CARDS){
    if (splashQuad){
      renderer->RenderQuad(splashQuad,0,0);
    }else{
      char text[512];
      sprintf(text, "LOADING SET: %s", mCurrentSetName);
      mFont->DrawString(text,SCREEN_WIDTH/2,SCREEN_HEIGHT/2,JGETEXT_CENTER);
    }
  }else{
    mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
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
    mFont->SetColor(ARGB(128,255,255,255));
    mFont->DrawString(GAME_VERSION, SCREEN_WIDTH-10,5,JGETEXT_RIGHT);
    mFont->DrawString(nbcardsStr,10, 5);
    mFont->SetScale(1.f);
    mFont->SetColor(ARGB(255,255,255,255));

    scroller->Render();

    if (subMenuController){
      subMenuController->Render();
    }

    if ((currentState & MENU_STATE_MAJOR) == MENU_STATE_MAJOR_WARNING){
      renderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(128,0,0,0));

      mFont->DrawString("IMPORTANT NOTE" ,SCREEN_WIDTH/2,10,JGETEXT_CENTER);
      mFont->DrawString("This is an alpha version with lots of bugs.",SCREEN_WIDTH/2,35,JGETEXT_CENTER);
      mFont->DrawString("It WILL crash your psp" ,SCREEN_WIDTH/2,50,JGETEXT_CENTER);
      mFont->DrawString("If you use it anyway, your feedback is welcome" ,SCREEN_WIDTH/2,65,JGETEXT_CENTER);

      mFont->DrawString("This freeware game is NOT published or endorsed" ,SCREEN_WIDTH/2,110,JGETEXT_CENTER);
      mFont->DrawString("by Wizard of the Coast, Inc." ,SCREEN_WIDTH/2,125,JGETEXT_CENTER);
      mFont->DrawString("Infos & updates at http://wololo.net/wagic/" ,SCREEN_WIDTH/2,170,JGETEXT_CENTER);
      mFont->DrawString("PRESS CIRCLE TO CONTINUE OR HOME TO QUIT" ,SCREEN_WIDTH/2,210,JGETEXT_CENTER);

    }
  }

}


void GameStateMenu::ButtonPressed(int controllerId, int controlId)
{
JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MENU_FONT);
#if defined (WIN32) || defined (LINUX)
  char buf[4096];
  sprintf(buf, "cnotrollerId: %i", controllerId);
  OutputDebugString(buf);
#endif
  switch (controllerId){
  case 101:
    createUsersFirstDeck(controlId);
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
	     << " ; mIconsTexture : " << mIconsTexture
	     << " ; bgTexture : " << bgTexture
	     << " ; movingWTexture : " << movingWTexture
	     << " ; mBg : " << mBg
	     << " ; mMovingW : " << mMovingW
	     << " ; splashTex : " << splashTex
	     << " ; splashQuad : " << splashQuad
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
