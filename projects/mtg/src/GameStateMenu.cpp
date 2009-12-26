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

static const char* GAME_VERSION = "WTH?! 0.10.1 - by wololo";

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
    MENU_STATE_MAJOR_LANG = 0x07,

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
  //bgMusic = NULL;
  timeIndex = 0;
  angleMultiplier = MIN_ANGLE_MULTIPLIER;
  yW = 55;
  mVolume = 0;
  scroller = NULL;
  langChoices = false;
}

GameStateMenu::~GameStateMenu() {}


void GameStateMenu::Create()
{
  mDip = NULL;
  mGuiController = NULL;
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

  currentState = MENU_STATE_MAJOR_LOADING_CARDS | MENU_STATE_MINOR_NONE;
  bool langChosen = false;
  string lang = options[Options::LANG].str;
  if (lang.size()){
    lang = "Res/lang/" + lang +  ".txt";
    if (fileExists(lang.c_str())) langChosen = true;
  }
  if (!langChosen){
    currentState = MENU_STATE_MAJOR_LANG | MENU_STATE_MINOR_NONE;
  }
  scroller = NEW TextScroller(Constants::MAIN_FONT, SCREEN_WIDTH/2 - 90 , SCREEN_HEIGHT-17,180);
  scrollerSet = 0;

  splashTex = NULL;
  mSplash = NULL;

}



void GameStateMenu::Destroy()
{
  SAFE_DELETE(mGuiController);
  SAFE_DELETE(subMenuController);
  SAFE_DELETE(gameTypeMenu);
  resources.Release(bgTexture);
  SAFE_DELETE(scroller);
}

void GameStateMenu::Start(){
  JRenderer::GetInstance()->EnableVSync(true);
  subMenuController = NULL;
  SAFE_DELETE(mGuiController);

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
  
  bgTexture = resources.RetrieveTexture("menutitle.png", RETRIEVE_LOCK);
  mBg = resources.RetrieveQuad("menutitle.png", 0, 0, 256, 166);		// Create background quad for rendering.

  mBg->SetHotSpot(128,50);

  //How many cards total ?
  PlayerData * playerdata = NEW PlayerData(mParent->collection);
  if(playerdata && !options[Options::ACTIVE_PROFILE].isDefault())
    sprintf(nbcardsStr, _("%s: %i cards (%i)").c_str(), options[Options::ACTIVE_PROFILE].str.c_str(), playerdata->collection->totalCards(), mParent->collection->totalCards());
  else
    sprintf(nbcardsStr, _("Database: %i cards").c_str(), mParent->collection->totalCards());

  SAFE_DELETE(playerdata);

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
    scroller->Add(_("You haven't unlocked the random deck mode yet"));
  if (!options[Options::EVILTWIN_MODE_UNLOCKED].number)
    scroller->Add(_("You haven't unlocked the evil twin mode yet"));
  if (!options[Options::RANDOMDECK_MODE_UNLOCKED].number)
    scroller->Add(_("You haven't unlocked the random deck mode yet"));
  if (!options[Options::EVILTWIN_MODE_UNLOCKED].number)
    scroller->Add(_("You haven't unlocked the evil twin mode yet"));

  //Unlocked sets
  int nbunlocked = 0;
  for (int i = 0; i < setlist.size(); i++){
    if (1 == options[Options::optionSet(i)].number) nbunlocked++;
  }
  sprintf(buff2, _("You have unlocked %i expansions out of %i").c_str(),nbunlocked, setlist.size());
  scroller->Add(buff2);

  DeckDataWrapper* ddw = NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(PLAYER_COLLECTION).c_str(), mParent->collection));
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
  if (!found) resetDirectory();
  return found;
}

void GameStateMenu::End()
{
  JRenderer::GetInstance()->EnableVSync(false);
  
  resources.Release(bgTexture);
  SAFE_DELETE(mGuiController);
}

string GameStateMenu::getLang(string s){
    if (!s.size()) return "";
    if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
    size_t found = s.find("#LANG:");
    if (found != 0) return "";
    return s.substr(6);
}

void GameStateMenu::setLang(int id){
 options[Options::LANG].str = langs[id-1];
 options.save();
}

void GameStateMenu::loadLangMenu(){
  subMenuController = NEW SimpleMenu(103, this, Constants::MENU_FONT, 150,60);
  if (!subMenuController) return;
  resetDirectory();
  if (!mDip){
    mDip = opendir("Res/lang");
  }

  while ((mDit = readdir(mDip))){
    string filename = "Res/lang/";
    filename += mDit->d_name;
    std::ifstream file(filename.c_str());
    string s;
    string lang;
    if(file){
      if(std::getline(file,s)){
        lang = getLang(s);
      } 
      file.close();
    }
    if (lang.size()){
      langChoices = true;
      string filen = mDit->d_name;
      langs.push_back(filen.substr(0,filen.size()-4));
      subMenuController->Add(langs.size(),lang.c_str());
    }
  }
  resetDirectory();
}

void GameStateMenu::Update(float dt)
{
  timeIndex += dt * 2;
  switch (MENU_STATE_MAJOR & currentState) {
    case MENU_STATE_MAJOR_LANG :
        if (MENU_STATE_MINOR_NONE == (currentState & MENU_STATE_MINOR)) {
          if (!subMenuController) loadLangMenu();
        }
        if(!langChoices){
          currentState = MENU_STATE_MAJOR_LOADING_CARDS;
          SAFE_DELETE(subMenuController);
        }
        else
          subMenuController->Update(dt);
    break;
    case MENU_STATE_MAJOR_LOADING_CARDS :
      if (mReadConf){
	      mParent->collection->load(mCurrentSetFileName, mCurrentSetName);
      }else{
	      mReadConf = 1;
        Translator::GetInstance()->init();
      }
      if (!nextDirectory(RESPATH"/sets/","_cards.dat")){
        //Remove temporary translations
        Translator::GetInstance()->tempValues.clear();

        //Force default, if necessary.
        if(options[Options::ACTIVE_PROFILE].str == "") 
          options[Options::ACTIVE_PROFILE].str = "Default";
        
        //Release splash texture
        resources.Release(splashTex);
        splashTex = NULL;
        mSplash = NULL;

        //check for deleted collection / first-timer
        std::ifstream file(options.profileFile(PLAYER_COLLECTION).c_str());
	      if(file){
	        file.close();
	        currentState = MENU_STATE_MAJOR_MAINMENU;
	      }else{
  	      currentState = MENU_STATE_MAJOR_FIRST_TIME;
        }

        //Reload list of unlocked sets, now that we know about the sets.
        options.reloadProfile(false);
        
        //List active profile and database size.        
        PlayerData * playerdata = NEW PlayerData(mParent->collection);
        if(playerdata && !options[Options::ACTIVE_PROFILE].isDefault())
          sprintf(nbcardsStr, _("%s: %i cards (%i)").c_str(), options[Options::ACTIVE_PROFILE].str.c_str(), playerdata->collection->totalCards(), mParent->collection->totalCards());
        else
          sprintf(nbcardsStr, _("Database: %i cards").c_str(), mParent->collection->totalCards());
        SAFE_DELETE(playerdata);
        resetDirectory();
        //All major things have been loaded, resize the cache to use it as efficiently as possible
        resources.autoResize();
      }
      break;    
    case MENU_STATE_MAJOR_FIRST_TIME :
      currentState &= MENU_STATE_MAJOR_MAINMENU;
      options.reloadProfile(); //Handles building a new deck, if needed.
      break;
    case MENU_STATE_MAJOR_MAINMENU :
      if (!scrollerSet) fillScroller();
      if (!mGuiController) {
        mGuiController = NEW JGuiController(100, this);
        if (mGuiController) {
          JLBFont * mFont = resources.GetJLBFont(Constants::MENU_FONT);
          mFont->SetColor(ARGB(255,255,255,255));
          mGuiController->Add(NEW MenuItem(MENUITEM_PLAY, mFont, "Play", 80,         50 + SCREEN_HEIGHT/2, mIcons[8], mIcons[9],"particle1.psi",resources.GetQuad("particles"),  true));
          mGuiController->Add(NEW MenuItem(MENUITEM_DECKEDITOR, mFont, "Deck Editor", 160, 50 + SCREEN_HEIGHT/2, mIcons[2], mIcons[3],"particle2.psi",resources.GetQuad("particles")));
          mGuiController->Add(NEW MenuItem(MENUITEM_SHOP, mFont, "Shop", 240,        50 + SCREEN_HEIGHT/2, mIcons[0], mIcons[1],"particle3.psi",resources.GetQuad("particles")));
          mGuiController->Add(NEW MenuItem(MENUITEM_OPTIONS, mFont, "Options", 320,     50 + SCREEN_HEIGHT/2, mIcons[6], mIcons[7],"particle4.psi",resources.GetQuad("particles")));
          mGuiController->Add(NEW MenuItem(MENUITEM_EXIT, mFont, "Exit", 400,        50 + SCREEN_HEIGHT/2, mIcons[4], mIcons[5],"particle5.psi",resources.GetQuad("particles")));
        }
      }
      if (mGuiController)
	      mGuiController->Update(dt);
      if(mEngine->GetButtonState(PSP_CTRL_RTRIGGER)) //Hook for GameStateAward state
      	mParent->SetNextState(GAME_STATE_AWARDS);
      break;
    case MENU_STATE_MAJOR_SUBMENU :
      subMenuController->Update(dt);
      mGuiController->Update(dt);
      break;
    case MENU_STATE_MAJOR_DUEL :
      if (MENU_STATE_MINOR_NONE == (currentState & MENU_STATE_MINOR)) {
	      if (!hasChosenGameType){
	        currentState = MENU_STATE_MAJOR_SUBMENU;
	        subMenuController = NEW SimpleMenu(102, this, Constants::MENU_FONT, 150,60);
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

  switch (MENU_STATE_MINOR & currentState){
    case MENU_STATE_MINOR_SUBMENU_CLOSING :
      if (subMenuController->closed) {
	      SAFE_DELETE(subMenuController);
	      currentState &= ~MENU_STATE_MINOR_SUBMENU_CLOSING;
	    } else
	      subMenuController->Update(dt);
      break;
    case MENU_STATE_MINOR_NONE :
      ;// Nothing to do.
    }
  
  if(mEngine->GetButtonState(PSP_CTRL_LTRIGGER)) {
    //Reset deck of cards
    angleMultiplier = MIN_ANGLE_MULTIPLIER;
    yW = 55;
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
	  yW = yW + 5*dt + (yW - 45) *5*  dt;
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
  if ((currentState & MENU_STATE_MAJOR) == MENU_STATE_MAJOR_LANG){
  }else if ((currentState & MENU_STATE_MAJOR) == MENU_STATE_MAJOR_LOADING_CARDS){
    if(!splashTex){
     splashTex = resources.RetrieveTexture("splash.jpg",RETRIEVE_LOCK);
     mSplash = resources.RetrieveTempQuad("splash.jpg");
    }
    if (mSplash)
      renderer->RenderQuad(mSplash,0,0);
    char text[512];
    mFont->SetColor(ARGB(255,255,255,255));
    if (mCurrentSetName[0]) {
      sprintf(text, _("LOADING SET: %s").c_str(), mCurrentSetName);
    }else{
      sprintf(text,"LOADING...");
    }
    mFont->DrawString(text,SCREEN_WIDTH/2,SCREEN_HEIGHT/2,JGETEXT_CENTER);
  }else{
    mFont = resources.GetJLBFont(Constants::MAIN_FONT);
    PIXEL_TYPE colors[] =
    {
      
      ARGB(255,3,3,0),
      ARGB(255,8,8,0),
      ARGB(255,21,21,10),
      ARGB(255,50,50,30),
    };
    renderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,colors);  

    if (mGuiController!=NULL)
      mGuiController->Render();

    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    mFont->SetColor(ARGB(128,255,255,255));
    mFont->DrawString(GAME_VERSION, SCREEN_WIDTH-74,5,JGETEXT_RIGHT);
    mFont->DrawString(nbcardsStr,10, 5);
    mFont->SetScale(1.f);
    mFont->SetColor(ARGB(255,255,255,255));

    renderer->FillRoundRect(SCREEN_WIDTH/2 - 100,SCREEN_HEIGHT, 191,6,5,ARGB(100,10,5,0));
    scroller->Render();

    
    if(mBg)
      renderer->RenderQuad(mBg,SCREEN_WIDTH/2,50);

    JQuad * jq = resources.RetrieveTempQuad("button_trophy.png");
    if(jq){
      int alp = 255;
      if(options.newAward())
        alp = (int) (sin(timeIndex) * 255);

      jq->SetColor(ARGB(abs(alp),255,255,255));        
      renderer->RenderQuad(jq, SCREEN_WIDTH-64, 0);
    }
  }
  if (subMenuController){
    subMenuController->Render();
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
  case 103:
    setLang(controlId);
    subMenuController->Close();
    currentState = MENU_STATE_MAJOR_LOADING_CARDS | MENU_STATE_MINOR_SUBMENU_CLOSING;
    break;
  case 101:
    options.createUsersFirstDeck(controlId);
    currentState = MENU_STATE_MAJOR_MAINMENU | MENU_STATE_MINOR_NONE;
    break;
  default:
    switch (controlId)
      {
      case MENUITEM_PLAY:
	    subMenuController = NEW SimpleMenu(102, this, Constants::MENU_FONT, 150,60);
	    if (subMenuController){
	      subMenuController->Add(SUBMENUITEM_1PLAYER,"1 Player");
        // TODO Put 2 players mode back
        // This requires to fix the hand (to accept 2 players) OR to implement network game
	      //subMenuController->Add(SUBMENUITEM_2PLAYER, "2 Players"); 
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
	     << " ; mBg : " << mBg
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
