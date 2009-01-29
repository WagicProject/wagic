#include "../include/config.h"
#include "../include/GameStateOptions.h"
#include "../include/GameApp.h"
#include "../include/OptionItem.h"
#include "../include/SimpleMenu.h"
#include "../include/GameOptions.h"

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
  if (GameApp::HasMusic) optionsList->Add(NEW OptionItem(OPTIONS_MUSICVOLUME, "Music volume", 100, 10));
  optionsList->Add(NEW OptionItem(OPTIONS_SFXVOLUME, "SFX volume", 100, 10));
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
    "Wagic, The Homebrew ?! by WilLoW",
    "This is a work in progress and it contains bugs",
    "updates on http://www.wololo.net/wagic",
    "Many thanks to J for his help in this release, and to all card creators",
    "",
    "Developped with the JGE++ Library (http://jge.khors.com)",
    "",
    "this freeware app is not endorsed by Wizards of the Coast, Inc",
    "",
    "SFX From www.soundsnap.com",

  };

  const char * const MusicText[] = {
    "",
    "Music by Celestial Aeon Project, under Creative Commons License",
    "Their music can be downloaded at http://www.jamendo.com"
  };


  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/magic");
  mFont->SetColor(ARGB(255,200,200,200));
  mFont->SetScale(1.0);
  float startpos = 272 - timer * 10;
  float pos = startpos;
  for (int i = 0; i < 10; i++){
    pos = startpos +20*i;
    if (pos > -20){
      mFont->DrawString(CreditsText[i],SCREEN_WIDTH/2,pos ,JGETEXT_CENTER);
    }
  }

  if (GameApp::HasMusic){
    for (int i = 0; i < 3; i++){
      pos = startpos +20*(10+i);
      if (pos > -20){
        mFont->DrawString(MusicText[i],SCREEN_WIDTH/2, pos,JGETEXT_CENTER);
      }
    }
  }
  if (pos < -20) timer = 0;
  mFont->SetScale(1.f);

  if (mState == SHOW_OPTIONS_MENU){
    optionsMenu->Render();
  }

  optionsList->Render();

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


