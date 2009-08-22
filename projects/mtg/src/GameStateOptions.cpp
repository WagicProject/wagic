#include "../include/config.h"
#include "../include/GameStateOptions.h"
#include "../include/GameApp.h"
#include "../include/OptionItem.h"
#include "../include/SimpleMenu.h"
#include "../include/GameOptions.h"
#include "../include/Translate.h"

GameStateOptions::GameStateOptions(GameApp* parent): GameState(parent) {
  optionsList = NULL;
  optionsMenu = NULL;
}


GameStateOptions::~GameStateOptions() {

}

void GameStateOptions::Start()
{

  timer =  0;
  mState = SHOW_OPTIONS;
  JRenderer::GetInstance()->ResetPrivateVRAM();
  JRenderer::GetInstance()->EnableVSync(true);

  optionsList = NEW OptionsList();
  if (GameApp::HasMusic) optionsList->Add(NEW OptionItem(Options::MUSICVOLUME, "Music volume", 100, 10));
  optionsList->Add(NEW OptionItem(Options::SFXVOLUME, "SFX volume", 100, 10));
  optionsList->Add(NEW OptionItem(Options::INTERRUPTMYSPELLS, "interrupt my spells"));
  optionsList->Add(NEW OptionItem(Options::INTERRUPTMYABILITIES, "interrupt my abilities"));
  optionsList->Add(NEW OptionItem(Options::INTERRUPT_SECONDS, "Seconds to pause for an Interrupt", 20, 1));
  optionsList->Add(NEW OptionItem(Options::OSD, "Display InGame extra information"));
  if (options[Options::DIFFICULTY_MODE_UNLOCKED].number)
    optionsList->Add(NEW OptionItem(Options::DIFFICULTY, "Difficulty", 3, 1));
  optionsList->Add(NEW OptionItem(Options::CACHESIZE, "Image Cache Size", 60, 5));
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  optionsMenu = NEW SimpleMenu(102, this,mFont, 50,170);
  optionsMenu->Add(1, "Save & Back to Main Menu");
  optionsMenu->Add(2, "Back to Main Menu");
  optionsMenu->Add(3, "Cancel");
}


void GameStateOptions::End()
{
  JRenderer::GetInstance()->EnableVSync(false);
  SAFE_DELETE(optionsList);
  SAFE_DELETE(optionsMenu);
}


void GameStateOptions::Update(float dt)
{

  timer+= dt;
  if (mState == SHOW_OPTIONS){
    if (PSP_CTRL_START == mEngine->ReadButton()){
      mState = SHOW_OPTIONS_MENU;
    }

    optionsList->Update(dt);
  }else{
    optionsMenu->Update(dt);
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



  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/magic");
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

  if (pos < -20) timer = 0;
  mFont->SetScale(1.f);

  optionsList->Render();

  if (mState == SHOW_OPTIONS_MENU){
    optionsMenu->Render();
  }
}


void GameStateOptions::ButtonPressed(int controllerId, int controlId)
{
  switch (controlId){
  case 1:
    optionsList->save();
  case 2:
    mParent->SetNextState(GAME_STATE_MENU);
    break;
  case 3:
    mState = SHOW_OPTIONS;
    break;
  }
};


