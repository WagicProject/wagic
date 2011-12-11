#include "PrecompiledHeader.h"

#include <JGE.h>
#include <JLogger.h>
#include <JRenderer.h>
#if defined (PSP) 
#include <pspfpu.h>
#else
#include <time.h>
#endif

#include "WResourceManager.h"
#include "ExtraCost.h"
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
#include "DeckManager.h"
#include "Translate.h"
#include "WFilter.h"
#include "Rules.h"
#include "ModRules.h"
#include "JFileSystem.h"
#include "Credits.h"
#include "AbilityParser.h"

#define DEFAULT_DURATION .25

PlayerType GameApp::players[] = { PLAYER_TYPE_CPU, PLAYER_TYPE_CPU };
bool GameApp::HasMusic = true;
JMusic * GameApp::music = NULL;
string GameApp::currentMusicFile = "";
string GameApp::systemError = "";

vector<JQuadPtr > manaIcons;

GameState::GameState(GameApp* parent, string id) :
    mParent(parent), mStringID(id)
{
    mEngine = JGE::GetInstance();
}

GameApp::GameApp() :
    JApp()
#ifdef NETWORK_SUPPORT
    ,mpNetwork(NULL)
#endif //NETWORK_SUPPORT
{
#ifdef DEBUG
    nbUpdates = 0;
    totalFPS = 0;
#endif

#ifdef DOLOG
    remove(LOG_FILE);
#endif

    mScreenShotCount = 0;

    for (int i = 0; i < GAME_STATE_MAX; i++)
        mGameStates[i] = NULL;

    mShowDebugInfo = false;
    players[0] = PLAYER_TYPE_CPU;
    players[1] = PLAYER_TYPE_CPU;
    gameType = GAME_TYPE_CLASSIC;

    mCurrentState = NULL;
    mNextState = NULL;

    music = NULL;
}

GameApp::~GameApp()
{
    WResourceManager::Terminate();
}

void GameApp::Create()
{
    srand((unsigned int) time(0)); // initialize random
#if !defined(QT_CONFIG) && !defined(IOS)
#if defined (WIN32)
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#elif defined (PSP)
    pspFpuSetEnable(0); //disable FPU Exceptions until we find where the FPU errors come from
    pspDebugScreenPrintf("Wagic:Loading resources...");
#endif
#endif //QT_CONFIG
    //_CrtSetBreakAlloc(368);
    LOG("starting Game");

    string systemFolder = "Res/";
    string foldersRoot = "";

    //Find the Res folder
    ifstream mfile("Res.txt");
    string resPath;
    if (mfile)
    {
        bool found = false;
        while (!found && std::getline(mfile, resPath))
        {
            if (resPath[resPath.size() - 1] == '\r')
                resPath.erase(resPath.size() - 1); //Handle DOS files
            string testfile = resPath + systemFolder;
            testfile.append("graphics/simon.dat");
            ifstream tempfile(testfile.c_str());
            if (tempfile)
            {
                found = true;
                tempfile.close();
                foldersRoot = resPath;
            }
        }
        mfile.close();
    }
    LOG("init Res Folder at " + foldersRoot);
    JFileSystem::init(foldersRoot + "User/", foldersRoot + systemFolder);

    // Create User Folders (for write access) if they don't exist
    {
        const char* folders[] = { "ai", "ai/baka", "ai/baka/stats", "campaigns", "graphics", "lang", "packs", "player", "player/stats", "profiles", "rules", "sets", "settings", "sound", "sound/sfx", "themes", "test"};

        for (size_t i = 0; i < sizeof(folders)/sizeof(folders[0]); ++i)
        {
            JFileSystem::GetInstance()->MakeDir(string(folders[i]));
        }
    }

    LOG("Loading Modrules");
    //Load Mod Rules before everything else
    gModRules.load("rules/modrules.xml");

    LOG("Loading Unlockables");
    //Load awards (needs to be loaded before any option are accessed)
    Unlockable::load();

    //Link this to our settings manager.
    options.theGame = this;

    //Ensure that options are partially loaded before loading files.
    LOG("options.reloadProfile()");
    options.reloadProfile();


    //Setup Cache before calling any gfx/sfx functions
	WResourceManager::Instance()->ResetCacheLimits();


    LOG("Checking for music files");
    //Test for Music files presence
    JFileSystem * jfs = JFileSystem::GetInstance();
    HasMusic = jfs->FileExists(WResourceManager::Instance()->musicFile("Track0.mp3")) && jfs->FileExists(WResourceManager::Instance()->musicFile("Track1.mp3"));

    LOG("Loading Textures");
    LOG("--Loading menuicons.png");
    WResourceManager::Instance()->RetrieveTexture("menuicons.png", RETRIEVE_MANAGE);
    LOG("---Gettings menuicons.png quads");
   
    //Load all icons from gModRules and save in manaIcons -> todo. Change the icons positions on menuicons.png to avoid use item->mColorId
    vector<ModRulesBackGroundCardGuiItem *>items = gModRules.cardgui.background;
    for(size_t i= 0; i < items.size(); i ++)
    {
        
        ModRulesBackGroundCardGuiItem * item = items[i];
        if (item->mMenuIcon == 1)
        {
            manaIcons.push_back(WResourceManager::Instance()->RetrieveQuad("menuicons.png", 2 + (float)item->mColorId * 36, 38, 32, 32, "c_" + item->MColorName, RETRIEVE_MANAGE));
            Constants::MTGColorStrings.push_back(item->MColorName.c_str());
        }
    }
    Constants::NB_Colors = (int)Constants::MTGColorStrings.size();
    
    items.erase(items.begin(),items.end());
    

    for (int i = manaIcons.size()-1; i >= 0; --i)
        if (manaIcons[i].get())
            manaIcons[i]->SetHotSpot(16, 16);

    LOG("--Loading back.jpg");
    WResourceManager::Instance()->RetrieveTexture("back.jpg", RETRIEVE_MANAGE);
    JQuadPtr jq = WResourceManager::Instance()->RetrieveQuad("back.jpg", 0, 0, 0, 0, kGenericCardID, RETRIEVE_MANAGE);
    if (jq.get())
        jq->SetHotSpot(jq->mWidth / 2, jq->mHeight / 2);

    WResourceManager::Instance()->RetrieveTexture("back_thumb.jpg", RETRIEVE_MANAGE);
    WResourceManager::Instance()->RetrieveQuad("back_thumb.jpg", 0, 0, MTG_MINIIMAGE_WIDTH, MTG_MINIIMAGE_HEIGHT, kGenericCardThumbnailID, RETRIEVE_MANAGE);

    LOG("--Loading particles.png");
    WResourceManager::Instance()->RetrieveTexture("particles.png", RETRIEVE_MANAGE);
    jq = WResourceManager::Instance()->RetrieveQuad("particles.png", 0, 0, 32, 32, "particles", RETRIEVE_MANAGE);
    if (jq)
        jq->SetHotSpot(16, 16);
    jq = WResourceManager::Instance()->RetrieveQuad("particles.png", 64, 0, 32, 32, "stars", RETRIEVE_MANAGE);
    if (jq)
        jq->SetHotSpot(16, 16);

    LOG("--Loading fonts");
    string lang = options[Options::LANG].str;
    std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);
    WResourceManager::Instance()->InitFonts(lang);
    Translator::GetInstance()->init();
    // The translator is ready now.

    LOG("--Loading various textures");
    // Load in this function only textures that are used frequently throughout the game. These textures will constantly stay in Ram, so be frugal
    WResourceManager::Instance()->RetrieveTexture("phasebar.png", RETRIEVE_MANAGE);
    WResourceManager::Instance()->RetrieveTexture("wood.png", RETRIEVE_MANAGE);
    WResourceManager::Instance()->RetrieveTexture("gold.png", RETRIEVE_MANAGE);
    WResourceManager::Instance()->RetrieveTexture("goldglow.png", RETRIEVE_MANAGE);
    WResourceManager::Instance()->RetrieveTexture("backdrop.jpg", RETRIEVE_MANAGE);
    WResourceManager::Instance()->RetrieveTexture("handback.png", RETRIEVE_MANAGE);
    WResourceManager::Instance()->RetrieveTexture("shadows.png", RETRIEVE_MANAGE);

    jq = WResourceManager::Instance()->RetrieveQuad("shadows.png", 2, 2, 16, 16, "white", RETRIEVE_MANAGE);
    if (jq)
        jq->SetHotSpot(8, 8);
    jq = WResourceManager::Instance()->RetrieveQuad("shadows.png", 20, 2, 16, 16, "shadow", RETRIEVE_MANAGE);
    if (jq)
        jq->SetHotSpot(8, 8);
    jq = WResourceManager::Instance()->RetrieveQuad("shadows.png", 38, 2, 16, 16, "extracostshadow", RETRIEVE_MANAGE);
    if (jq)
        jq->SetHotSpot(8, 8);

    jq = WResourceManager::Instance()->RetrieveQuad("phasebar.png", 0, 0, 0, 0, "phasebar", RETRIEVE_MANAGE);


    LOG("Init Collection");
    MTGAllCards::loadInstance();

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

    LOG("--Load Game rules");
    Rules::loadAllRules();

    //Set Audio volume
    JSoundSystem::GetInstance()->SetSfxVolume(options[Options::SFXVOLUME].number);
    JSoundSystem::GetInstance()->SetMusicVolume(options[Options::MUSICVOLUME].number);

    DebugTrace("size of MTGCardInstance: " << sizeof(MTGCardInstance));
    DebugTrace("size of MTGCard: "<< sizeof(MTGCard));
    DebugTrace("size of CardPrimitive: "<< sizeof(CardPrimitive));
    DebugTrace("size of ExtraCost: " << sizeof(ExtraCost));
    DebugTrace("size of ManaCost: " << sizeof(ManaCost));

    LOG("Game Creation Done.");
}

void GameApp::LoadGameStates()
{

}

void GameApp::Destroy()
{
    LOG("==Destroying GameApp==");

#ifdef TRACK_OBJECT_USAGE
    ObjectAnalytics::DumpStatistics();
#endif

    for (int i = GAME_STATE_MENU; i <= GAME_STATE_MAX - 1; i++)
    {
        if (mGameStates[i])
        {
            mGameStates[i]->Destroy();
            SAFE_DELETE(mGameStates[i]);
        }
    }

    MTGAllCards::unloadAll();

    DeckManager::EndInstance();
    DeckStats::EndInstance();

    SAFE_DELETE(Subtypes::subtypesList);

    stopMusic();

    Translator::EndInstance();
    WCFilterFactory::Destroy();
    SimpleMenu::destroy();
    DeckMenu::destroy();
    DeckEditorMenu::destroy();

    options.theGame = NULL;
    Unlockable::Destroy();

    AutoLineMacro::Destroy();

    Rules::unloadAllRules();
    LOG("==Destroying GameApp Successful==");

#ifdef TRACK_FILE_USAGE_STATS
    wagic::ifstream::Dump();
#endif

}

void GameApp::Update()
{
    if (systemError.size())
        return;
    JGE* mEngine = JGE::GetInstance();
#if defined (PSP)
    if (mEngine->GetButtonState(JGE_BTN_MENU) && mEngine->GetButtonClick(JGE_BTN_CANCEL))
    {
        char s[80];
        sprintf(s, "ms0:/psp/photo/MTG%d.png", mScreenShotCount++);
        JRenderer::GetInstance()->ScreenShot(s);
    }
    //Exit when START and X ARE PRESSED SIMULTANEOUSLY
    if (mEngine->GetButtonState(JGE_BTN_MENU) && mEngine->GetButtonState(JGE_BTN_SEC))
    {
        mEngine->End();
        return;
    }

    //Restart Rendering engine when START and SQUARE ARE PRESSED SIMULTANEOUSLY
    if (mEngine->GetButtonState(JGE_BTN_MENU) && mEngine->GetButtonState(JGE_BTN_PRI))
        JRenderer::Destroy();
#endif //PSP

    float dt = mEngine->GetDelta();
    if (dt > 35.0f) // min 30 FPS ;)
        dt = 35.0f;

    TransitionBase * mTrans = NULL;
    if (mCurrentState)
    {
        mCurrentState->Update(dt);
        if (mGameStates[GAME_STATE_TRANSITION] == mCurrentState)
            mTrans = (TransitionBase *) mGameStates[GAME_STATE_TRANSITION];
    }
    //Check for finished transitions.
    if (mTrans && mTrans->Finished())
    {
        mTrans->End();
        if (mTrans->to != NULL && !mTrans->bAnimationOnly)
        {
            SetCurrentState(mTrans->to);
            SAFE_DELETE(mGameStates[GAME_STATE_TRANSITION]);
            mCurrentState->Start();
        }
        else
        {
            SetCurrentState(mTrans->from);
            SAFE_DELETE(mGameStates[GAME_STATE_TRANSITION]);
        }
    }
    if (mNextState != NULL)
    {
        if (mCurrentState != NULL)
            mCurrentState->End();

        SetCurrentState(mNextState);

#if defined (PSP)
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
    if (systemError.size())
    {
        fprintf(stderr, "%s", systemError.c_str());
        WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
        if (mFont)
            mFont->DrawString(systemError.c_str(), 1, 1);
        return;
    }

    JRenderer * renderer = JRenderer::GetInstance();
    renderer->ClearScreen(ARGB(0,0,0,0));

    if (mCurrentState)
        mCurrentState->Render();

#ifdef DEBUG_CACHE
    WResourceManager::Instance()->DebugRender();
#endif

#ifdef DEBUG
    JGE* mEngine = JGE::GetInstance();
    float fps = mEngine->GetFPS();
    totalFPS += fps;
    nbUpdates+=1;
    WFont * mFont= WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    char buf[512];
    sprintf(buf, "avg:%.02f - %.02f fps",totalFPS/nbUpdates, fps);
    if (mFont)
    {
        mFont->SetColor(ARGB(255,255,255,255));
        mFont->DrawString(buf, 10, SCREEN_HEIGHT-25);
    }
#endif

}

void GameApp::OnScroll(int inXVelocity, int inYVelocity)
{
    if (mCurrentState != NULL)
    {
        mCurrentState->OnScroll(inXVelocity, inYVelocity);
    }
}

void GameApp::SetNextState(int state)
{
    mNextState = mGameStates[state];
}

void GameApp::SetCurrentState(GameState * state)
{
    if (mCurrentState == state)
        return;

    if (mCurrentState)
        JGE::GetInstance()->SendCommand("leavegamestate:" + mCurrentState->getStringID());

    mCurrentState = state;

    if (mCurrentState)
        JGE::GetInstance()->SendCommand("entergamestate:" + mCurrentState->getStringID());
}

void GameApp::Pause()
{
    stopMusic();
}

void GameApp::Resume()
{
    playMusic();
}

void GameApp::DoTransition(int trans, int tostate, float dur, bool animonly)
{
    TransitionBase * tb = NULL;
    GameState * toState = NULL;
    if (options[Options::TRANSITIONS].number != 0)
    {
        if (tostate != GAME_STATE_NONE)
            SetNextState(tostate);
        return;
    }

    if (tostate > GAME_STATE_NONE && tostate < GAME_STATE_MAX)
        toState = mGameStates[tostate];

    if (mGameStates[GAME_STATE_TRANSITION])
    {
        tb = (TransitionBase*) mGameStates[GAME_STATE_TRANSITION];
        if (toState)
            tb->to = toState; //Additional calls to transition merely update the destination.
        return;
    }

    if (dur < 0)
        dur = DEFAULT_DURATION; // Default to this value.
    switch (trans)
    {
    case TRANSITION_FADE_IN:
        tb = NEW TransitionFade(this, mCurrentState, toState, dur, true);
        break;
    case TRANSITION_FADE:
    default:
        tb = NEW TransitionFade(this, mCurrentState, toState, dur, false);
    }
    if (tb)
    {
        tb->bAnimationOnly = animonly;
        mGameStates[GAME_STATE_TRANSITION] = tb;
        mGameStates[GAME_STATE_TRANSITION]->Start();
        SetCurrentState(tb); //The old current state is ended inside our transition.
    }
    else if (toState)
    { //Somehow failed, just do standard SetNextState behavior
        mNextState = toState;
    }
}

void GameApp::DoAnimation(int trans, float dur)
{
    DoTransition(trans, GAME_STATE_NONE, dur, true);
}

void GameApp::playMusic(string filename, bool loop)
{
    if(filename == "") filename = currentMusicFile;

    if (filename.compare(currentMusicFile) == 0 && music)
        return;

    if (music)
    {
        JSoundSystem::GetInstance()->StopMusic(music);
        SAFE_DELETE(music);
    }

    if (HasMusic && options[Options::MUSICVOLUME].number > 0)
    {
        music = WResourceManager::Instance()->ssLoadMusic(filename.c_str());
        if (music)
            JSoundSystem::GetInstance()->PlayMusic(music, loop);
        currentMusicFile = filename;
    }
}

void GameApp::stopMusic()
{
    if (music && currentMusicFile != "")
    {
        JSoundSystem::GetInstance()->StopMusic(music);
        SAFE_DELETE(music);
    }
}
