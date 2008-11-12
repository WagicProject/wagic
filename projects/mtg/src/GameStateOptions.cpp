#include "../include/debug.h"
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
  mState = SHOW_OPTIONS;
  JRenderer::GetInstance()->ResetPrivateVRAM();
  JRenderer::GetInstance()->EnableVSync(true);

  optionsList = NEW OptionsList();
  if (GameApp::HasMusic) optionsList->Add(NEW OptionItem(OPTIONS_MUSICVOLUME, "Music volume", 100, 10));
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  optionsMenu = NEW SimpleMenu(102, this,mFont, 50,170,SCREEN_WIDTH-120);
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
  if (mState == SHOW_OPTIONS){
    if (mEngine->GetButtonClick(PSP_CTRL_START)){
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
  optionsList->Render();

  const char * const CreditsText[] = {
    "Wagic, The Homebrew ?! by WilLoW",
    "This is a work in progress and it contains bugs, deal with it",
    "updates on http://www.wololo.net/wagic",
    "",
    "Developped with the JGE++ Library",
    "http://jge.khors.com",
    "",
    "this freeware app is not endorsed by Wizards of the Coast, Inc",

  };

  const char * const MusicText[] = {
    "",
    "Music by Celestial Aeon Project, under Creative Commons License",
    "Their music can be downloaded at http://www.jamendo.com"
  };


  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  mFont->SetColor(ARGB(255,200,200,200));
  mFont->SetScale(0.80);
  for (int i = 0; i < 8; i++){
    mFont->DrawString(CreditsText[i],SCREEN_WIDTH/2, 40 +18*i,JGETEXT_CENTER);
  }

  if (GameApp::HasMusic){
    for (int i = 0; i < 3; i++){
      mFont->DrawString(MusicText[i],SCREEN_WIDTH/2, 40 +18*(8+i),JGETEXT_CENTER);
    }
  }
  mFont->SetScale(1.f);

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


