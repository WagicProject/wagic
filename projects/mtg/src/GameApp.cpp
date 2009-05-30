#include "../include/config.h"
#include <JGE.h>
#include <JRenderer.h>
#if defined (WIN32) || defined (LINUX)
#else
#include <pspfpu.h>
#endif


#include "../include/GameApp.h"
#include "../include/Subtypes.h"
#include "../include/GameStateDeckViewer.h"
#include "../include/GameStateMenu.h"
#include "../include/GameStateDuel.h"
#include "../include/GameStateOptions.h"
#include "../include/GameStateShop.h"
#include "../include/DeckStats.h"
#include "../include/Translate.h"

const char * const GameState::menuTexts[]= {"--NEW--","Deck 1", "Deck 2", "Deck 3", "Deck 4", "Deck 5", "Deck 6"} ;
JResourceManager* GameApp::CommonRes = NEW JResourceManager();
hgeParticleSystem* GameApp::Particles[] = {NULL,NULL,NULL,NULL,NULL,NULL};
int GameApp::HasMusic = 1;
JMusic * GameApp::music = NULL;
string GameApp::systemError = "";

GameState::GameState(GameApp* parent): mParent(parent)
{
  mEngine = JGE::GetInstance();
}


GameApp::GameApp(): JApp()
{
  mScreenShotCount = 0;

  for (int i=0; i < MAX_STATE	; i++)
    mGameStates[i] = NULL;

  mShowDebugInfo = false;
  players[0] = 0;
  players[1] = 0;
  gameType = GAME_TYPE_CLASSIC;
  //gameType = GAME_TYPE_MOMIR;


}


GameApp::~GameApp()
{
}


void GameApp::Create()
{
#if defined (WIN32)
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#elif not defined (LINUX)
  pspfpu_set_enable(0); //disable FPU Exceptions until we find where the FPU errors come from
#endif
  //_CrtSetBreakAlloc(368);
  LOG("starting Game");

  //Test for Music files presence
  std::ifstream file(RESPATH"/sound/Track0.mp3");
  if(file){
    file.close();
  }else{
    HasMusic = 0;
  }
  std::ifstream file2(RESPATH"/sound/Track1.mp3");
  if(file2){
    file2.close();
  }else{
    HasMusic = 0;
  }



  CommonRes->CreateTexture("graphics/menuicons.png");
  //Creating thes quad in this specific order allows us to have them in the correct order to call them by integer id
  CommonRes->CreateQuad("c_artifact", "graphics/menuicons.png", 2 + 6*36, 38, 32, 32);
  CommonRes->CreateQuad("c_green", "graphics/menuicons.png", 2 + 0*36, 38, 32, 32);
  CommonRes->CreateQuad("c_blue", "graphics/menuicons.png", 2 + 1*36, 38, 32, 32);
  CommonRes->CreateQuad("c_red", "graphics/menuicons.png", 2 + 3*36, 38, 32, 32);
  CommonRes->CreateQuad("c_black", "graphics/menuicons.png", 2 + 2*36, 38, 32, 32);
  CommonRes->CreateQuad("c_white", "graphics/menuicons.png", 2 + 4*36, 38, 32, 32);
  CommonRes->CreateQuad("c_land", "graphics/menuicons.png", 2 + 5*36, 38, 32, 32);


  CommonRes->CreateTexture("sets/back.jpg");
  CommonRes->CreateQuad("back", "sets/back.jpg", 0, 0, 200, 285);
  CommonRes->CreateTexture("sets/back_thumb.jpg");
  CommonRes->CreateQuad("back_thumb", "sets/back_thumb.jpg", 0, 0, MTG_MINIIMAGE_WIDTH, MTG_MINIIMAGE_HEIGHT);

  CommonRes->CreateTexture("graphics/particles.png");
  CommonRes->CreateQuad("particles", "graphics/particles.png", 0, 0, 32, 32);
  CommonRes->GetQuad("particles")->SetHotSpot(16,16);

  CommonRes->CreateQuad("stars", "graphics/particles.png", 64, 0, 32, 32);
  CommonRes->GetQuad("stars")->SetHotSpot(16,16);

  CommonRes->LoadJLBFont("graphics/simon",11);
  CommonRes->GetJLBFont("graphics/simon")->SetTracking(-1);
  CommonRes->LoadJLBFont("graphics/f3",16);
  CommonRes->LoadJLBFont("graphics/magic",16);


  CommonRes->CreateTexture("graphics/phasebar.png");
  CommonRes->CreateTexture("graphics/background.png");
  CommonRes->CreateTexture("graphics/back.jpg");

  //CommonRes->CreateTexture("graphics/interrupt.png");
  //CommonRes->CreateQuad("interrupt", "graphics/interrupt.png", 0, 0, 256, 128);

  cache = NEW TexturesCache();
  collection = NEW MTGAllCards(cache);


  Particles[0] = NEW hgeParticleSystem("graphics/particle1.psi", CommonRes->GetQuad("particles"));
  Particles[1] = NEW hgeParticleSystem("graphics/particle2.psi", CommonRes->GetQuad("particles"));
  Particles[2] = NEW hgeParticleSystem("graphics/particle3.psi", CommonRes->GetQuad("particles"));
  Particles[3] = NEW hgeParticleSystem("graphics/particle4.psi", CommonRes->GetQuad("particles"));
  Particles[4] = NEW hgeParticleSystem("graphics/particle5.psi", CommonRes->GetQuad("particles"));
  Particles[5] = NEW hgeParticleSystem("graphics/particle7.psi", CommonRes->GetQuad("particles"));

  mGameStates[GAME_STATE_DECK_VIEWER] = NEW GameStateDeckViewer(this);
  mGameStates[GAME_STATE_DECK_VIEWER]->Create();

  mGameStates[GAME_STATE_MENU] = NEW GameStateMenu(this);
  mGameStates[GAME_STATE_MENU]->Create();

  mGameStates[GAME_STATE_DUEL] = NEW GameStateDuel(this);
  mGameStates[GAME_STATE_DUEL]->Create();

  mGameStates[GAME_STATE_SHOP] = NEW GameStateShop(this);
  mGameStates[GAME_STATE_SHOP]->Create();

  mGameStates[GAME_STATE_OPTIONS] = NEW GameStateOptions(this);
  mGameStates[GAME_STATE_OPTIONS]->Create();

  //mGameStates[GAME_STATE_GAME] = NEW GameStateGAME(this);

  mCurrentState = NULL;
  mNextState = mGameStates[GAME_STATE_MENU];

  effect = new CardEffect();

  char buf[512];
  sprintf(buf, "size of MTGCardInstance : %i\n" , sizeof(MTGCardInstance));
  OutputDebugString(buf);
}


void GameApp::LoadGameStates()
{

  //mGameStates[GAME_STATE_MENU]->Create();
  //mGameStates[GAME_STATE_GAME]->Create();

}


void GameApp::Destroy()
{
  LOG("==Destroying GameApp==");
  for (int i=GAME_STATE_MENU;i<=MAX_STATE-1;i++)
    {
      if (mGameStates[i]){
	mGameStates[i]->Destroy();
	SAFE_DELETE(mGameStates[i]);
      }
    }

  for (int i= 0; i < 6; i++){
    SAFE_DELETE(Particles[i]);
  }

  if (collection){
    collection->destroyAllCards();
    SAFE_DELETE(collection);
  }
  SAFE_DELETE(cache);
  SampleCache::DestroyInstance();
  delete(DeckStats::GetInstance());

  SAFE_DELETE(CommonRes);

  GameOptions::Destroy(); //No delete ???

  SAFE_DELETE(Subtypes::subtypesList);
  SAFE_DELETE(MtgSets::SetsList);

  SAFE_DELETE(music);
  Translator::EndInstance();

  SimpleMenu::destroy();


  LOG("==Destroying GameApp Successful==");

}



void GameApp::Update()
{

  if (systemError.size()) return;
  JGE* mEngine = JGE::GetInstance();
  if (mEngine->GetButtonState(PSP_CTRL_START) && mEngine->GetButtonClick(PSP_CTRL_TRIANGLE))
    {
      char s[80];
      sprintf(s, "ms0:/psp/photo/MTG%d.png", mScreenShotCount++);
      JRenderer::GetInstance()->ScreenShot(s);
    }
  //Exit when START and X ARE PRESSED SIMULTANEOUSLY
  if (mEngine->GetButtonState(PSP_CTRL_START) && mEngine->GetButtonState(PSP_CTRL_CROSS)){
    mEngine->End();
    return;
  }

  float dt = mEngine->GetDelta();
  if (dt > 35.0f)		// min 30 FPS ;)
    dt = 35.0f;

  if (mCurrentState != NULL)
    mCurrentState->Update(dt);

  if (mNextState != NULL)
    {
      if (mCurrentState != NULL)
	      mCurrentState->End();

      mCurrentState = mNextState;
     

#if defined (WIN32) || defined (LINUX)
#else
   /*   
    int maxLinear = ramAvailableLineareMax();
      int ram = ramAvailable();
      char buf[512];
      sprintf(buf, "Ram : linear max: %i - total : %i\n",maxLinear, ram);
      fprintf(stderr,buf);
   */
#endif
      mCurrentState->Start();

      mNextState = NULL;
    }



}


void GameApp::Render()
{
  if (systemError.size()){
    fprintf(stderr, systemError.c_str());
    JLBFont * mFont= CommonRes->GetJLBFont("graphics/simon");
    if (mFont) mFont->DrawString(systemError.c_str(),1,1);
    return;
  }
  if (mCurrentState != NULL)
    {
      mCurrentState->Render();
    }


}

void GameApp::SetNextState(int state)
{
  mNextState = mGameStates[state];
}

void GameApp::Pause(){

}

void GameApp::Resume(){

}

