#include "../include/config.h"
#include <JGE.h>
#include <JRenderer.h>
#if defined (WIN32) || defined (LINUX)
#include <time.h>
#else
#include <pspfpu.h>
#endif

#include "../include/WResourceManager.h"
#include "../include/GameApp.h"
#include "../include/Subtypes.h"
#include "../include/GameStateDeckViewer.h"
#include "../include/GameStateMenu.h"
#include "../include/GameStateDuel.h"
#include "../include/GameStateOptions.h"
#include "../include/GameStateShop.h"
#include "../include/DeckStats.h"
#include "../include/Translate.h"

hgeParticleSystem* GameApp::Particles[] = {NULL,NULL,NULL,NULL,NULL,NULL};
MTGAllCards * GameApp::collection = NULL;
int GameApp::HasMusic = 1;
JMusic * GameApp::music = NULL;
string GameApp::systemError = "";

JQuad* manaIcons[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

GameState::GameState(GameApp* parent): mParent(parent)
{
  mEngine = JGE::GetInstance();
}


GameApp::GameApp(): JApp()
{
#ifdef DEBUG
   nbUpdates = 0;
   totalFPS = 0;
#endif
  mScreenShotCount = 0;

  for (int i=0; i < MAX_STATE	; i++)
    mGameStates[i] = NULL;

  mShowDebugInfo = false;
  players[0] = 0;
  players[1] = 0;
  gameType = GAME_TYPE_CLASSIC;

  
  mCurrentState = NULL;
  mNextState = NULL;
  collection = NULL;
  
  for(int i=0;i<6;i++)
    Particles[i] = NULL;
 
  music = NULL;
}


GameApp::~GameApp()
{
}


void GameApp::Create()
{
   srand((unsigned int)time(0));  // initialize random
#if defined (WIN32)
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#elif not defined (LINUX)
  pspfpu_set_enable(0); //disable FPU Exceptions until we find where the FPU errors come from
#endif
  //_CrtSetBreakAlloc(368);
  LOG("starting Game");

  //Link this to our settings manager.
  options.theGame = this;

  //Ensure that options are partially loaded before loading files.
  options.reloadProfile();

  //Test for Music files presence
  string filepath = RESPATH;
  filepath = filepath + "/" + resources.musicFile("Track0.mp3");
  std::ifstream file(filepath.c_str());
  if (file)
    file.close();
  else
    HasMusic = 0;

  filepath = RESPATH;
  filepath = filepath + "/" + resources.musicFile("Track1.mp3");
  std::ifstream file2(filepath.c_str());
  if (file2)
    file2.close();
  else
    HasMusic = 0;

  resources.RetrieveTexture("menuicons.png",RETRIEVE_MANAGE);
  //Creating thes quad in this specific order allows us to have them in the correct order to call them by integer id
  manaIcons[Constants::MTG_COLOR_GREEN] = resources.RetrieveQuad("menuicons.png", 2 + 0*36, 38, 32, 32, "c_green",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_BLUE] = resources.RetrieveQuad("menuicons.png", 2 + 1*36, 38, 32, 32, "c_blue",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_RED] = resources.RetrieveQuad("menuicons.png", 2 + 3*36, 38, 32, 32, "c_red",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_BLACK] = resources.RetrieveQuad("menuicons.png", 2 + 2*36, 38, 32, 32, "c_black",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_WHITE] = resources.RetrieveQuad("menuicons.png", 2 + 4*36, 38, 32, 32, "c_white",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_LAND] = resources.RetrieveQuad("menuicons.png", 2 + 5*36, 38, 32, 32, "c_land",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_ARTIFACT] = resources.RetrieveQuad("menuicons.png", 2 + 6*36, 38, 32, 32, "c_artifact",RETRIEVE_MANAGE);

  for (int i = sizeof(manaIcons)/sizeof(manaIcons[0]) - 1; i >= 0; --i) manaIcons[i]->SetHotSpot(16,16);

  resources.RetrieveTexture("back.jpg",RETRIEVE_MANAGE);
  JQuad * jq = resources.RetrieveQuad("back.jpg", 0, 0, 200, 285, "back",RETRIEVE_MANAGE);
  jq->SetHotSpot(100, 145);

  resources.RetrieveTexture("back_thumb.jpg",RETRIEVE_MANAGE);
  resources.RetrieveQuad("back_thumb.jpg", 0, 0, MTG_MINIIMAGE_WIDTH, MTG_MINIIMAGE_HEIGHT, "back_thumb",RETRIEVE_MANAGE);

  resources.RetrieveTexture("particles.png",RETRIEVE_MANAGE);
  jq = resources.RetrieveQuad("particles.png", 0, 0, 32, 32, "particles",RETRIEVE_MANAGE);
  jq->SetHotSpot(16,16);
  jq = resources.RetrieveQuad("particles.png", 64, 0, 32, 32, "stars",RETRIEVE_MANAGE);
  jq->SetHotSpot(16,16);

  resources.LoadJLBFont("simon",11);
  resources.GetJLBFont("simon")->SetTracking(-1);
  resources.LoadJLBFont("f3",16);
  resources.LoadJLBFont("magic",16);


  resources.RetrieveTexture("phasebar.png",RETRIEVE_MANAGE);
  resources.RetrieveTexture("wood.png",RETRIEVE_MANAGE);
  resources.RetrieveTexture("gold.png",RETRIEVE_MANAGE);
  resources.RetrieveTexture("goldglow.png",RETRIEVE_MANAGE);
  resources.RetrieveTexture("backdrop.jpg",RETRIEVE_MANAGE);
  resources.RetrieveTexture("handback.png",RETRIEVE_MANAGE);
  resources.RetrieveTexture("BattleIcon.png",RETRIEVE_MANAGE);
  resources.RetrieveTexture("DefenderIcon.png",RETRIEVE_MANAGE);
  resources.RetrieveTexture("shadow.png",RETRIEVE_MANAGE);

  jq = resources.RetrieveQuad("BattleIcon.png", 0, 0, 25, 25,"BattleIcon",RETRIEVE_MANAGE);
  jq->SetHotSpot(12, 12);
  jq = resources.RetrieveQuad("DefenderIcon.png", 0, 0, 24, 23,"DefenderIcon",RETRIEVE_MANAGE);
  jq->SetHotSpot(12, 12);
  jq = resources.RetrieveQuad("shadow.png", 0, 0, 16, 16,"shadow",RETRIEVE_MANAGE);
  jq->SetHotSpot(8, 8);
  jq = resources.RetrieveQuad("phasebar.png",0,0,0,0,"phasebar",RETRIEVE_MANAGE);
  
  collection = NEW MTGAllCards();

  Particles[0] = NEW hgeParticleSystem("graphics/particle1.psi", resources.GetQuad("particles"));
  Particles[1] = NEW hgeParticleSystem("graphics/particle2.psi", resources.GetQuad("particles"));
  Particles[2] = NEW hgeParticleSystem("graphics/particle3.psi", resources.GetQuad("particles"));
  Particles[3] = NEW hgeParticleSystem("graphics/particle4.psi", resources.GetQuad("particles"));
  Particles[4] = NEW hgeParticleSystem("graphics/particle5.psi", resources.GetQuad("particles"));
  Particles[5] = NEW hgeParticleSystem("graphics/particle7.psi", resources.GetQuad("particles"));

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

  mCurrentState = NULL;
  mNextState = mGameStates[GAME_STATE_MENU];

  //  effect = NEW CardEffect();

  char buf[512];
  sprintf(buf, "size of MTGCard : %i\n" , sizeof(MTGCard));
  OutputDebugString(buf);
}


void GameApp::LoadGameStates()
{

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
  delete(DeckStats::GetInstance());

  SAFE_DELETE(Subtypes::subtypesList);

  SAFE_DELETE(music);
  Translator::EndInstance();

  SimpleMenu::destroy();

  options.theGame = NULL;
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
    JLBFont * mFont= resources.GetJLBFont("simon");
    if (mFont) mFont->DrawString(systemError.c_str(),1,1);
    return;
  }
  if (mCurrentState != NULL)
    {
      mCurrentState->Render();
    }

#ifdef DEBUG_CACHE
  resources.DebugRender();
#endif

#ifdef DEBUG
  JGE* mEngine = JGE::GetInstance();
  float fps = mEngine->GetFPS();
  totalFPS += fps;
  nbUpdates+=1;
  JLBFont * mFont= resources.GetJLBFont("simon");
  char buf[512];
  sprintf(buf, "avg:%f - %f fps",totalFPS/nbUpdates, fps);
  if (mFont) {
    mFont->SetColor(ARGB(255,255,255,255));
    mFont->DrawString(buf,1,1);
  }
#endif

}

void GameApp::SetNextState(int state)
{
  mNextState = mGameStates[state];
}

void GameApp::Pause(){

}

void GameApp::Resume(){

}

