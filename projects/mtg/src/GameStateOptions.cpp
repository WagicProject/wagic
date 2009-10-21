#include "../include/config.h"
#include "../include/GameStateOptions.h"
#include "../include/GameApp.h"
#include "../include/OptionItem.h"
#include "../include/SimpleMenu.h"
#include "../include/SimplePad.h"
#include "../include/GameOptions.h"
#include "../include/Translate.h"

GameStateOptions::GameStateOptions(GameApp* parent): GameState(parent) {
  optionsTabs = NULL;
  optionsMenu = NULL;
  mReload = false;
}


GameStateOptions::~GameStateOptions() {

}

void GameStateOptions::Start()
{
  newProfile = "";
  timer =  0;
  mState = SHOW_OPTIONS;
  JRenderer::GetInstance()->EnableVSync(true);

  WGuiList * optionsList;

  optionsList = NEW WGuiList("Settings");
  
  optionsList->Add(NEW WGuiHeader("General Options"));
  if (GameApp::HasMusic) 
    optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::MUSICVOLUME,"Music volume",100,10,100),OptionVolume::getInstance()));
  optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::SFXVOLUME,"SFX volume",100,10,100),OptionVolume::getInstance()));
  optionsList->Add(NEW OptionInteger(Options::OSD, "Display InGame extra information"));
  if (options[Options::DIFFICULTY_MODE_UNLOCKED].number)
    optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::DIFFICULTY,"Difficulty",3,1,0),OptionDifficulty::getInstance()));
  optionsList->Add(NEW OptionInteger(Options::INTERRUPT_SECONDS, "Seconds to pause for an Interrupt", 20, 1));
  optionsList->Add(NEW OptionInteger(Options::INTERRUPTMYSPELLS, "Interrupt my spells"));
  optionsList->Add(NEW OptionInteger(Options::INTERRUPTMYABILITIES, "Interrupt my abilities"));  
  optionsTabs = NEW WGuiTabMenu();  
  optionsTabs->Add(optionsList);

  optionsList = NEW WGuiList("Game");
  optionsList->Add(NEW WGuiHeader("Interface Options"));
  optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::CLOSEDHAND,"Closed hand",1,1,0)));
  optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::HANDDIRECTION,"Hand direction",1,1,0)));
  optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::MANADISPLAY,"Mana display",2,1,0)));
  optionsList->Add(NEW OptionInteger(Options::REVERSETRIGGERS, "Reverse left and right triggers"));
  optionsList->Add(NEW OptionInteger(Options::DISABLECARDS,"Disable card image loading"));
  optionsTabs->Add(optionsList);

  optionsList = NEW WGuiList("User");
  optionsList->Add(NEW WGuiHeader("User Options"));

  WDecoConfirm * cPrf = NEW WDecoConfirm(this,NEW OptionProfile(mParent,this));
  cPrf->confirm = "Use this Profile";
  OptionDirectory * od = NEW OptionTheme();
  WDecoConfirm * cThm = NEW WDecoConfirm(this,od);
  cThm->confirm = "Use this Theme";  
  
  optionsList->Add(NEW WGuiSplit(cPrf,cThm));
  optionsList->Add(NEW WGuiButton(NEW WGuiHeader("New Profile"),-102,4,this));
  optionsTabs->Add(optionsList);

  optionsList = NEW WGuiList("Credits");
  optionsList->failMsg = "";
  optionsTabs->Add(optionsList);

  JLBFont * mFont = resources.GetJLBFont("f3");
  optionsMenu = NEW SimpleMenu(-102, this,mFont, 50,170);
  optionsMenu->Add(1, "Save & Back to Main Menu");
  optionsMenu->Add(2, "Back to Main Menu");
  optionsMenu->Add(3, "Cancel");

  optionsTabs->Current()->Entering(0);
}


void GameStateOptions::End()
{
  JRenderer::GetInstance()->EnableVSync(false);
  SAFE_DELETE(optionsTabs);
  SAFE_DELETE(optionsMenu);
}


void GameStateOptions::Update(float dt)
{
  timer += dt;

  if(options.keypadActive()){
    options.keypadUpdate(dt);

    if(newProfile != ""){
        newProfile = options.keypadFinish();
        if(newProfile != ""){
          options[Options::ACTIVE_PROFILE] = newProfile;
          options.reloadProfile(false);
          optionsTabs->Reload();
        }
        newProfile = "";
    }
  }
  else switch(mState){
    default:
    case SHOW_OPTIONS:
      optionsTabs->Update(dt);

      if (mEngine->ReadButton() == PSP_CTRL_START){
        mState = SHOW_OPTIONS_MENU;
      }
      break;
    case SHOW_OPTIONS_MENU:
      optionsMenu->Update(dt);
    break;
  }  
  if(mReload){
    options.reloadProfile(true);
    optionsTabs->Reload();    
    mReload = false;
  }
}

void GameStateOptions::Render()
{
  //Erase
  JRenderer::GetInstance()->ClearScreen(ARGB(0,0,0,0));

  const char * const CreditsText[] = {
      "Wagic, The Homebrew?! by WilLoW",
      "",
      "updates, new cards, and more on http://wololo.net/wagic",
      "Many thanks to the devs and card creators who help this project",
      "",
      "Developped with the JGE++ Library (http://jge.khors.com)",
      "Player's avatar from http://mathieuchoinet.blogspot.com, under CC License",
      "Background picture and some art from the KDE project, www.kde.org",
      "SFX From www.soundsnap.com",
      "",
      "Music by Celestial Aeon Project, http://www.jamendo.com",
      "",
      "",
      "This work is not related to or endorsed by Wizards of the Coast, Inc",
      "",
      "Please support this project with donations at http://wololo.net/wagic",
    };

  JLBFont * mFont = resources.GetJLBFont("magic");
  mFont->SetColor(ARGB(255,200,200,200));
  mFont->SetScale(1.0);
  float startpos = 272 - timer * 10;
  float pos = startpos;
  int size = sizeof(CreditsText) / sizeof(CreditsText[0]);
  
  for (int i = 0; i < size; i++){
    pos = startpos +20*i;
    if (pos > -20){
      mFont->DrawString(_(CreditsText[i]).c_str(),SCREEN_WIDTH/2,pos ,JGETEXT_CENTER);
    }
  }

  if (pos < -20) 
    timer = 0;

  mFont->SetScale(1.f);

  optionsTabs->Render();

  if(mState == SHOW_OPTIONS_MENU)    
      optionsMenu->Render();

  if(options.keypadActive())
    options.keypadRender();
}

void GameStateOptions::ButtonPressed(int controllerId, int controlId)
{
  //Exit menu?
  if(controllerId == -102)
  switch (controlId){
  case 1:
    optionsTabs->save();
  case 2:
    mParent->SetNextState(GAME_STATE_MENU);
    break;
  case 3:
    mState = SHOW_OPTIONS;
    break;
  case 4:
    options.keypadStart("",&newProfile);
    options.keypadTitle("New Profile");
    break;
  case 5:
    mReload = true;
    break;
  }
  else
    optionsTabs->ButtonPressed(controllerId, controlId);
};