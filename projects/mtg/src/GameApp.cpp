#include "PrecompiledHeader.h"

#include <JGE.h>
#include <JRenderer.h>
#if defined (WIN32) || defined (LINUX)
#include <time.h>
#else
#include <pspfpu.h>
#endif

#include "WResourceManager.h"
#include "GameApp.h"
#include "Subtypes.h"
#include "GameStateTransitions.h"
#include "GameStateDeckViewer.h"
#include "GameStateMenu.h"
#include "GameStateDuel.h"
#include "GameStateOptions.h"
#include "GameStateShop.h"
#include "GameStateAwards.h"
#include "GameStateStory.h"
#include "DeckStats.h"
#include "DeckMetaData.h"
#include "Translate.h"
#include "WFilter.h"

#define DEFAULT_DURATION .25

MTGAllCards * GameApp::collection = NULL;
int GameApp::players[] = {0,0};
int GameApp::HasMusic = 1;
JMusic * GameApp::music = NULL;
string GameApp::currentMusicFile = "";
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

#ifdef DOLOG
   remove(LOG_FILE);
#endif

  mScreenShotCount = 0;

  for (int i=0; i < GAME_STATE_MAX	; i++)
    mGameStates[i] = NULL;

  mShowDebugInfo = false;
  players[0] = 0;
  players[1] = 0;
  gameType = GAME_TYPE_CLASSIC;

  
  mCurrentState = NULL;
  mNextState = NULL;
  collection = NULL;
  
  music = NULL;
}


GameApp::~GameApp()
{
}


void GameApp::Create()
{
   srand((unsigned int)time(0));  // initialize random
#ifndef QT_CONFIG
#if defined (WIN32)
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#elif not defined (LINUX)
  pspFpuSetEnable(0); //disable FPU Exceptions until we find where the FPU errors come from
#endif
#endif //QT_CONFIG

  //_CrtSetBreakAlloc(368);
  LOG("starting Game");

  //Link this to our settings manager.
  options.theGame = this;

  //Ensure that options are partially loaded before loading files.
  LOG("options.reloadProfile()");
  options.reloadProfile();

  LOG("Checking for music files");
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

  LOG("Loading Textures");
  LOG("--Loading menuicons.png");
  resources.RetrieveTexture("menuicons.png",RETRIEVE_MANAGE);
  LOG("---Gettings menuicons.png quads");
  //Creating thes quad in this specific order allows us to have them in the correct order to call them by integer id
  manaIcons[Constants::MTG_COLOR_GREEN] = resources.RetrieveQuad("menuicons.png", 2 + 0*36, 38, 32, 32, "c_green",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_BLUE] = resources.RetrieveQuad("menuicons.png", 2 + 1*36, 38, 32, 32, "c_blue",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_RED] = resources.RetrieveQuad("menuicons.png", 2 + 3*36, 38, 32, 32, "c_red",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_BLACK] = resources.RetrieveQuad("menuicons.png", 2 + 2*36, 38, 32, 32, "c_black",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_WHITE] = resources.RetrieveQuad("menuicons.png", 2 + 4*36, 38, 32, 32, "c_white",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_LAND] = resources.RetrieveQuad("menuicons.png", 2 + 5*36, 38, 32, 32, "c_land",RETRIEVE_MANAGE);
  manaIcons[Constants::MTG_COLOR_ARTIFACT] = resources.RetrieveQuad("menuicons.png", 2 + 6*36, 38, 32, 32, "c_artifact",RETRIEVE_MANAGE);


  for (int i = sizeof(manaIcons)/sizeof(manaIcons[0]) - 1; i >= 0; --i) manaIcons[i]->SetHotSpot(16,16);

  LOG("--Loading back.jpg");
  resources.RetrieveTexture("back.jpg",RETRIEVE_MANAGE);
  JQuad * jq = resources.RetrieveQuad("back.jpg", 0, 0, 0, 0, "back",RETRIEVE_MANAGE);
  if (jq) jq->SetHotSpot(jq->mWidth/2, jq->mHeight/2);

  resources.RetrieveTexture("back_thumb.jpg",RETRIEVE_MANAGE);
  resources.RetrieveQuad("back_thumb.jpg", 0, 0, MTG_MINIIMAGE_WIDTH, MTG_MINIIMAGE_HEIGHT, "back_thumb",RETRIEVE_MANAGE);

  LOG("--Loading particles.png");
  resources.RetrieveTexture("particles.png",RETRIEVE_MANAGE);
  jq = resources.RetrieveQuad("particles.png", 0, 0, 32, 32, "particles",RETRIEVE_MANAGE);
  jq->SetHotSpot(16,16);
  jq = resources.RetrieveQuad("particles.png", 64, 0, 32, 32, "stars",RETRIEVE_MANAGE);
  jq->SetHotSpot(16,16);

  LOG("--Loading fonts");
  string lang = options[Options::LANG].str;
  std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);
  resources.InitFonts(lang);

  LOG("--Loading various textures");
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

  LOG("Init Collection");
  collection = NEW MTGAllCards();

  LOG("Creating Game States");
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

  mGameStates[GAME_STATE_AWARDS] = NEW GameStateAwards(this);
  mGameStates[GAME_STATE_AWARDS]->Create();

  mGameStates[GAME_STATE_STORY] = NEW GameStateStory(this);
  mGameStates[GAME_STATE_STORY]->Create();

  mGameStates[GAME_STATE_TRANSITION] = NULL;

  mCurrentState = NULL;
  mNextState = mGameStates[GAME_STATE_MENU];

  //Set Audio volume
  JSoundSystem::GetInstance()->SetSfxVolume(options[Options::SFXVOLUME].number);
  JSoundSystem::GetInstance()->SetMusicVolume(options[Options::MUSICVOLUME].number);

  DebugTrace("size of MTGCard: "<< sizeof(MTGCard));
  DebugTrace("size of CardPrimitive: "<< sizeof(CardPrimitive));

  LOG("Game Creation Done.");
}


void GameApp::LoadGameStates()
{

}


void GameApp::Destroy()
{
  LOG("==Destroying GameApp==");
  for (int i=GAME_STATE_MENU;i<=GAME_STATE_MAX-1;i++)
    {
      if (mGameStates[i]){
	mGameStates[i]->Destroy();
	SAFE_DELETE(mGameStates[i]);
      }
    }

  if (collection){
    collection->destroyAllCards();
    SAFE_DELETE(collection);
  }
  delete(DeckStats::GetInstance());

  SAFE_DELETE(Subtypes::subtypesList);
  SAFE_DELETE(DeckMetaDataList::decksMetaData);

  playMusic("none");

  Translator::EndInstance();
  WCFilterFactory::Destroy();
  SimpleMenu::destroy();
  DeckMenu::destroy();

  options.theGame = NULL;
  LOG("==Destroying GameApp Successful==");

}



void GameApp::Update()
{
  if (systemError.size()) return;
  JGE* mEngine = JGE::GetInstance();
  if (mEngine->GetButtonState(JGE_BTN_MENU) && mEngine->GetButtonClick(JGE_BTN_CANCEL))
    {
      char s[80];
      sprintf(s, "ms0:/psp/photo/MTG%d.png", mScreenShotCount++);
      JRenderer::GetInstance()->ScreenShot(s);
    }
  //Exit when START and X ARE PRESSED SIMULTANEOUSLY
  if (mEngine->GetButtonState(JGE_BTN_MENU) && mEngine->GetButtonState(JGE_BTN_SEC)){
    mEngine->End();
    return;
  }

  //Restart Rendering engine when START and SQUARE ARE PRESSED SIMULTANEOUSLY
  if (mEngine->GetButtonState(JGE_BTN_MENU) && mEngine->GetButtonState(JGE_BTN_PRI))
    JRenderer::Destroy();

  float dt = mEngine->GetDelta();
  if (dt > 35.0f)		// min 30 FPS ;)
    dt = 35.0f;

  TransitionBase * mTrans = NULL;
  if (mCurrentState){
       mCurrentState->Update(dt);
       if(mGameStates[GAME_STATE_TRANSITION] == mCurrentState)
         mTrans = (TransitionBase *) mGameStates[GAME_STATE_TRANSITION];
  }
  //Check for finished transitions.
  if(mTrans && mTrans->Finished()){
    mTrans->End();
    if(mTrans->to != NULL && !mTrans->bAnimationOnly){
      mCurrentState = mTrans->to;
      SAFE_DELETE(mGameStates[GAME_STATE_TRANSITION]);
      mCurrentState->Start();
    }
    else{
      mCurrentState = mTrans->from;
      SAFE_DELETE(mGameStates[GAME_STATE_TRANSITION]);
    }
  }
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
    fprintf(stderr, "%s", systemError.c_str());
    WFont * mFont= resources.GetWFont(Fonts::MAIN_FONT);
    if (mFont) mFont->DrawString(systemError.c_str(),1,1);
    return;
  }

  JRenderer * renderer = JRenderer::GetInstance();
  renderer->ClearScreen(ARGB(0,0,0,0));

  if (mCurrentState)
      mCurrentState->Render();

#ifdef DEBUG_CACHE
  resources.DebugRender();
#endif

#ifdef DEBUG
  JGE* mEngine = JGE::GetInstance();
  float fps = mEngine->GetFPS();
  totalFPS += fps;
  nbUpdates+=1;
  WFont * mFont= resources.GetWFont(Fonts::MAIN_FONT);
  char buf[512];
  sprintf(buf, "avg:%.02f - %.02f fps",totalFPS/nbUpdates, fps);
  if (mFont) {
    mFont->SetColor(ARGB(255,255,255,255));
    mFont->DrawString(buf,1,10);
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

void GameApp::DoTransition(int trans, int tostate, float dur, bool animonly){
  TransitionBase * tb = NULL;
  GameState * toState = NULL;
  if(options[Options::TRANSITIONS].number != 0){
    if(tostate != GAME_STATE_NONE)
      SetNextState(tostate);
    return;
  }

  if(tostate > GAME_STATE_NONE && tostate < GAME_STATE_MAX)
    toState = mGameStates[tostate];

  if(mGameStates[GAME_STATE_TRANSITION]){
   tb =(TransitionBase*) mGameStates[GAME_STATE_TRANSITION];
   if(toState)
    tb->to = toState; //Additional calls to transition merely update the destination.
   return;
  }

  if(dur < 0)
    dur = DEFAULT_DURATION; // Default to this value. 
  switch(trans){
    case TRANSITION_FADE_IN:
      tb = NEW TransitionFade(this,mCurrentState,toState,dur,true);
      break;
    case TRANSITION_FADE:
    default:
      tb = NEW TransitionFade(this,mCurrentState,toState,dur,false);
  }
  if(tb){
    tb->bAnimationOnly = animonly;
    mGameStates[GAME_STATE_TRANSITION] = tb;
    mGameStates[GAME_STATE_TRANSITION]->Start();
    mCurrentState = tb;  //The old current state is ended inside our transition.
  } else if(toState) { //Somehow failed, just do standard SetNextState behaviour
    mNextState = toState;
  }
}


void GameApp::DoAnimation(int trans, float dur){
  DoTransition(trans,GAME_STATE_NONE,dur,true);
}

void GameApp::playMusic(string filename, bool loop) {
  if (filename.compare(currentMusicFile) == 0)
      return;

  if (music) {
    JSoundSystem::GetInstance()->StopMusic(music);
    SAFE_DELETE(music);
  }

  if (HasMusic && options[Options::MUSICVOLUME].number > 0){
    music = resources.ssLoadMusic(filename.c_str());
    if (music) JSoundSystem::GetInstance()->PlayMusic(music, loop);
    currentMusicFile = filename;
  }
}
