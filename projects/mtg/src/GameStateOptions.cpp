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
  confirmMenu = NULL;
}


GameStateOptions::~GameStateOptions() {

}

void GameStateOptions::Start()
{
  timer =  0;
  mState = SHOW_OPTIONS;
  JRenderer::GetInstance()->EnableVSync(true);

  OptionsList * optionsList;

  optionsList = NEW OptionsList("Settings");
  
  optionsList->Add(NEW OptionHeader("General Options"));
  if (GameApp::HasMusic) optionsList->Add(NEW OptionVolume(Options::MUSICVOLUME, "Music volume", true));
  optionsList->Add(NEW OptionVolume(Options::SFXVOLUME, "SFX volume"));
  optionsList->Add(NEW OptionInteger(Options::OSD, "Display InGame extra information"));
  if (options[Options::DIFFICULTY_MODE_UNLOCKED].number)
    optionsList->Add(NEW OptionInteger(Options::DIFFICULTY, "Difficulty", 3, 1));
  optionsList->Add(NEW OptionInteger(Options::INTERRUPT_SECONDS, "Seconds to pause for an Interrupt", 20, 1));
  optionsList->Add(NEW OptionInteger(Options::INTERRUPTMYSPELLS, "Interrupt my spells"));
  optionsList->Add(NEW OptionInteger(Options::INTERRUPTMYABILITIES, "Interrupt my abilities"));  
  optionsList->Add(NEW OptionInteger(Options::CACHESIZE, "Use large cache"));
  optionsTabs = NEW OptionsMenu();  
  optionsTabs->Add(optionsList);

  optionsList = NEW OptionsList("Game");
  optionsList->Add(NEW OptionClosedHand(Options::CLOSEDHAND, "Closed hand"));
  optionsList->Add(NEW OptionHandDirection(Options::HANDDIRECTION, "Hand direction"));
  optionsList->Add(NEW OptionInteger(Options::REVERSETRIGGERS, "Reverse left and right triggers"));
  optionsList->Add(NEW OptionInteger(Options::DISABLECARDS,"Disable card image loading"));
  optionsTabs->Add(optionsList);

  optionsList = NEW OptionsList("Profiles");
  OptionNewProfile * key = NEW OptionNewProfile("New Profile");
  key->bShowValue = false;
  optionsList->Add(key);
  OptionProfile * pickProf = NEW OptionProfile(mParent);
  optionsList->Add(pickProf);
  OptionTheme * theme = NEW OptionTheme();  
  optionsList->Add(theme);
  
  optionsTabs->Add(optionsList);
  optionsList = NEW OptionsList("Credits");
  optionsList->failMsg = "";
  optionsTabs->Add(optionsList);

  JLBFont * mFont = resources.GetJLBFont("f3");
  optionsMenu = NEW SimpleMenu(102, this,mFont, 50,170);
  optionsMenu->Add(1, "Save & Back to Main Menu");
  optionsMenu->Add(2, "Back to Main Menu");
  optionsMenu->Add(3, "Cancel");

  confirmMenu = NEW SimpleMenu(103, this,mFont, 50,170);
  confirmMenu->Add(1, "Use this profile");
  confirmMenu->Add(2, "Cancel");
}


void GameStateOptions::End()
{
  JRenderer::GetInstance()->EnableVSync(false);
  SAFE_DELETE(optionsTabs);
  SAFE_DELETE(optionsMenu);
  SAFE_DELETE(confirmMenu);
}


void GameStateOptions::Update(float dt)
{
  timer += dt;

  if(options.keypadActive()){
    options.keypadUpdate(dt);
  }
  else if (mState == SHOW_OPTIONS){
    
    switch(optionsTabs->Submode()){
     case OPTIONS_SUBMODE_RELOAD:
      optionsTabs->acceptSubmode();
      optionsTabs->reloadValues();
      mState = SHOW_OPTIONS;
       break;
     case OPTIONS_SUBMODE_PROFILE:
      mState = SHOW_OPTIONS_PROFILE;
      break;
     default:
      if (PSP_CTRL_START == mEngine->ReadButton() )
        mState = SHOW_OPTIONS_MENU;
      
      optionsTabs->Update(dt);
      break;
    }

  }else if(mState == SHOW_OPTIONS_MENU){
    optionsMenu->Update(dt);
  }else if(mState == SHOW_OPTIONS_PROFILE){
    confirmMenu->Update(dt);
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

  switch(mState){
  case SHOW_OPTIONS_MENU:
    optionsMenu->Render();
    break;
  case SHOW_OPTIONS_PROFILE:
    confirmMenu->Render();
    break;
  }

  if(options.keypadActive())
    options.keypadRender();
}


void GameStateOptions::ButtonPressed(int controllerId, int controlId)
{
  //Exit menu?
  if(controllerId == 102)
  switch (controlId){
  case 1:
    optionsTabs->save();
  case 2:
    mParent->SetNextState(GAME_STATE_MENU);
    break;
  case 3:
    mState = SHOW_OPTIONS;
    break;
  }
  //Profile confirmation?
  else if(controllerId == 103)
  switch (controlId){
  case 1:
    //Load the New profile.
    optionsTabs->acceptSubmode();
    optionsTabs->reloadValues();
    //Reset the current settings to those of the profile...
    mState = SHOW_OPTIONS;
    break;
  case 2:
    optionsTabs->cancelSubmode();
    mState = SHOW_OPTIONS;
    break;
  }
};


