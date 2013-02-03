/*
 * GameStateMenu.cpp
 * Main Menu and Loading screen
 */

#include "PrecompiledHeader.h"

#include <math.h>

#include "GameStateMenu.h"
#include "DeckManager.h"
#include "MenuItem.h"
#include "GameApp.h"
#include "MTGCard.h"
#include "Translate.h"
#include "DeckStats.h"
#include "PlayerData.h"
#include "utils.h"
#include "WFont.h"
#include <JLogger.h>
#include "Rules.h"
#include "ModRules.h"
#include "Credits.h"
#include "AIPlayer.h"

#ifdef NETWORK_SUPPORT
#include <JNetwork.h>
#endif//NETWORK_SUPPORT

static const char* GAME_VERSION = "WTH?! " WAGIC_VERSION_STRING " - wololo.net";

enum ENUM_MENU_STATE_MAJOR
{
    MENU_STATE_MAJOR_MAINMENU = 0x01,
    MENU_STATE_MAJOR_SUBMENU = 0x02,
    MENU_STATE_MAJOR_LOADING_MENU = 0x03,
    MENU_STATE_MAJOR_LOADING_CARDS = 0x04,
    MENU_STATE_MAJOR_FIRST_TIME = 0x05,
    MENU_STATE_MAJOR_DUEL = 0x06,
    MENU_STATE_MAJOR_LANG = 0x07,
#ifdef NETWORK_SUPPORT
    MENU_STATE_NETWORK_DEFINE = 0x08,
    MENU_STATE_NETWORK_WAIT = 0x09,
#endif //NETWORK_SUPPORT
    MENU_STATE_MAJOR = 0xFF
};

enum ENUM_MENU_STATE_MINOR
{
    MENU_STATE_MINOR_NONE = 0,
    MENU_STATE_MINOR_SUBMENU_CLOSING = 0x100,
    MENU_STATE_MINOR_FADEIN = 0x200,
    MENU_STATE_MINOR = 0xF00
};

GameStateMenu::GameStateMenu(GameApp* parent) :
    GameState(parent, "menu")
{
    mGuiController = NULL;
    subMenuController = NULL;
    gameTypeMenu = NULL;
    //bgMusic = NULL;
    timeIndex = 0;
    mVolume = 0;
    scroller = NULL;
    langChoices = false;
    primitivesLoadCounter = -1;
}

GameStateMenu::~GameStateMenu()
{
}

void GameStateMenu::Create()
{
    mGuiController = NULL;
    mReadConf = 0;

    //load all the icon images. Menu icons are managed, so we can do this here.
    int n = 0;
    char buf[512];

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            sprintf(buf, "menuicons%d%d", i, j);
            mIcons[n] = WResourceManager::Instance()->RetrieveQuad("menuicons.png", 2 + i * 36.0f, 2.0f + j * 36.0f, 32.0f, 32.0f, buf);
            if (mIcons[n])
                mIcons[n]->SetHotSpot(16, 16);
            n++;
        }
    }

    currentState = MENU_STATE_MAJOR_LOADING_CARDS;
    bool langChosen = false;
    string lang = options[Options::LANG].str;
    if (lang.size())
    {
        string langpath = "lang/";
        langpath.append(lang);
        langpath.append(".txt");
        if (JFileSystem::GetInstance()->FileExists(langpath))
            langChosen = true;
    }
    if (!langChosen)
    {
        currentState = MENU_STATE_MAJOR_LANG | MENU_STATE_MINOR_NONE;
    }
    scroller = NEW TextScroller(Fonts::MAIN_FONT, SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT - 17, 180);
    scrollerSet = 0;
    splashTex = NULL;

    JFileSystem::GetInstance()->scanfolder("sets/", setFolders);
    mCurrentSetFolderIndex = 0;
}

void GameStateMenu::Destroy()
{
    SAFE_DELETE(mGuiController);
    SAFE_DELETE(subMenuController);
    SAFE_DELETE(gameTypeMenu);
    WResourceManager::Instance()->Release(bgTexture);
    SAFE_DELETE(scroller);
}

void GameStateMenu::Start()
{
    LOG("GameStateMenu::Start");
    JRenderer::GetInstance()->EnableVSync(true);
    subMenuController = NULL;
    SAFE_DELETE(mGuiController);

    GameApp::playMusic("Track0.mp3");

    hasChosenGameType = false;
    mParent->gameType = GAME_TYPE_CLASSIC;

    //Manual clean up of some cache Data. Ideally those should clean themselves up, so this is kind of a hack for now
    WResourceManager::Instance()->ClearUnlocked();

    bgTexture = WResourceManager::Instance()->RetrieveTexture("menutitle.png", RETRIEVE_LOCK);
    mBg = WResourceManager::Instance()->RetrieveQuad("menutitle.png", 0, 0, 256, 166); // Create background quad for rendering.

    if (mBg)
        mBg->SetHotSpot(128, 50);

    if (MENU_STATE_MAJOR_MAINMENU == currentState)
        currentState = currentState | MENU_STATE_MINOR_FADEIN;

    wallpaper = "";
    scrollerSet = 0; // This will force-update the scroller text
    mPercentComplete = 0;
}

void GameStateMenu::genNbCardsStr()
{
    //How many cards total ?
    PlayerData * playerdata = NEW PlayerData(MTGCollection());
    size_t totalUnique =  MTGCollection()->primitives.size();
    size_t totalPrints = MTGCollection()->totalCards();

    if (totalUnique != totalPrints)
    {
        if (playerdata && !options[Options::ACTIVE_PROFILE].isDefault())
            sprintf(nbcardsStr, _("%s: %i cards (%i) (%i unique)").c_str(), options[Options::ACTIVE_PROFILE].str.c_str(),
                            playerdata->collection->totalCards(), totalPrints,totalUnique);
        else
            sprintf(nbcardsStr, _("%i cards (%i unique)").c_str(),totalPrints,totalUnique);
    }
    else
    {
        if (playerdata && !options[Options::ACTIVE_PROFILE].isDefault())
            sprintf(nbcardsStr, _("%s: %i cards (%i)").c_str(), options[Options::ACTIVE_PROFILE].str.c_str(),
                            playerdata->collection->totalCards(), totalPrints);
        else
            sprintf(nbcardsStr, _("%i cards").c_str(),totalPrints);
    }

    SAFE_DELETE(playerdata);
}

void GameStateMenu::fillScroller()
{
    scroller->Reset();
    char buff2[512];

    if (!options[Options::DIFFICULTY_MODE_UNLOCKED].number)
        scroller->Add(_("Unlock the difficult mode for more challenging duels!"));

    for (map<string, Unlockable *>::iterator it = Unlockable::unlockables.begin(); it !=  Unlockable::unlockables.end(); ++it) {
        Unlockable * award = it->second;
        if (!award->isUnlocked())
        {
            if (award->getValue("teaser").size())
                scroller->Add(_(award->getValue("teaser")));
        }
    }

    if (!options[Options::RANDOMDECK_MODE_UNLOCKED].number)
        scroller->Add(_("You haven't unlocked the random deck mode yet"));
    if (!options[Options::EVILTWIN_MODE_UNLOCKED].number)
        scroller->Add(_("You haven't unlocked the evil twin mode yet"));

    //Unlocked sets
    int nbunlocked = 0;
    for (int i = 0; i < setlist.size(); i++)
    {
        if (1 == options[Options::optionSet(i)].number)
            nbunlocked++;
    }
    sprintf(buff2, _("You have unlocked %i expansions out of %i").c_str(), nbunlocked, setlist.size());
    scroller->Add(buff2);

    scroller->Add(_("More cards and mods at http://wololo.net/wagic"));

    scrollerSet = 1;
    scroller->setRandom();
}

int GameStateMenu::gamePercentComplete() {
    if (mPercentComplete)
        return mPercentComplete;

    int done = 0;
    int total = 0;

    total++;
    if (options[Options::DIFFICULTY_MODE_UNLOCKED].number)
        done++;

    for (map<string, Unlockable *>::iterator it = Unlockable::unlockables.begin(); it !=  Unlockable::unlockables.end(); ++it) {
        total++;
        if (it->second->isUnlocked())
            total++;
    }

    total++;
    if (options[Options::RANDOMDECK_MODE_UNLOCKED].number)
        done++;

    total++;
    if (options[Options::EVILTWIN_MODE_UNLOCKED].number)
        done++;

    //Unlocked sets
    total+= setlist.size();
    for (int i = 0; i < setlist.size(); i++)
    {
        if (1 == options[Options::optionSet(i)].number)
            done++;
    }

    //unlocked AI decks
    int currentlyUnlocked = options[Options::AIDECKS_UNLOCKED].number;
    int totalAIDecks = AIPlayer::getTotalAIDecks();
    int reallyUnlocked = MIN(currentlyUnlocked, totalAIDecks);
    total+= totalAIDecks / 10;
    done+= reallyUnlocked / 10;

    mPercentComplete = 100 * done / total;
    return mPercentComplete;
}

int GameStateMenu::nextSetFolder(const string & root, const string & file)
{
    bool found = false;
    while (!found && (mCurrentSetFolderIndex < setFolders.size()))
    {
        vector<string> folders;
        folders.push_back(root);
        folders.push_back(setFolders[mCurrentSetFolderIndex]);
        mCurrentSetFileName = buildFilePath(folders, file);
        
        if (JFileSystem::GetInstance()->FileExists(mCurrentSetFileName))
        {
            mCurrentSetName = setFolders[mCurrentSetFolderIndex];
            if (mCurrentSetName.length() && (mCurrentSetName[mCurrentSetName.length() - 1] == '/'))
                mCurrentSetName.resize(mCurrentSetName.length() - 1);
            found = true;
        }
        mCurrentSetFolderIndex++;
    }

    return found;
}

void GameStateMenu::End()
{
    JRenderer::GetInstance()->EnableVSync(false);

    WResourceManager::Instance()->Release(bgTexture);
    SAFE_DELETE(mGuiController);
}

string GameStateMenu::loadRandomWallpaper()
{
    if (wallpaper.size())
        return wallpaper;

    vector<string> wallpapers;
    izfstream file;
    if (! JFileSystem::GetInstance()->openForRead(file, "graphics/wallpapers.txt"))
        return wallpaper;

    string s;
    while (std::getline(file, s))
    {
        if (!s.size())
            continue;
        if (s[s.size() - 1] == '\r')
            s.erase(s.size() - 1); //Handle DOS files
        wallpapers.push_back(s);
    }
    file.close();

    int rnd = rand() % (wallpapers.size());
    wallpaper = wallpapers[rnd];
    return wallpaper;

}

string GameStateMenu::getLang(string s)
{
    if (!s.size())
        return "";
    if (s[s.size() - 1] == '\r')
        s.erase(s.size() - 1); //Handle DOS files
    size_t found = s.find("#LANG:");
    if (found != 0)
        return "";
    return s.substr(6);
}

void GameStateMenu::setLang(int id)
{
    if(id != kCancelMenuID)
    {
        options[Options::LANG].str = langs[id - 1];
    }
    options.save();
}

void GameStateMenu::loadLangMenu()
{
    LOG("GameStateMenu::loadLangMenu");
    subMenuController = NEW SimpleMenu(JGE::GetInstance(), MENU_LANGUAGE_SELECTION, this, Fonts::MENU_FONT, 150, 60);
    if (!subMenuController)
        return;

    vector<string> langFiles = JFileSystem::GetInstance()->scanfolder("lang/");
    for (size_t i = 0; i < langFiles.size(); ++i)
    {
        izfstream file;
        string filePath = "lang/";
        filePath.append(langFiles[i]);
        if (! JFileSystem::GetInstance()->openForRead(file, filePath))
            continue;

        string s;
        string lang;

        if (std::getline(file, s))
        {
            lang = getLang(s);
        }

        file.close();

        if (lang.size())
        {
            langChoices = true;
            string filen = langFiles[i];
            langs.push_back(filen.substr(0, filen.size() - 4));
            subMenuController->Add(langs.size(), lang.c_str());
        }
    }
    LOG("GameStateMenu::loadLangMenu - Done");
}

void GameStateMenu::listPrimitives()
{
    LOG("GameStateMenu::listPrimitives");
    vector<string> primitiveFiles = JFileSystem::GetInstance()->scanfolder("sets/primitives/");

    if (!primitiveFiles.size())
    {
        DebugTrace("GameStateMenu.cpp:WARNING:Primitives folder is missing");
        primitivesLoadCounter = 0;
        return;
    }

    for (size_t i = 0; i < primitiveFiles.size(); ++i)
    {
        string filename = "sets/primitives/";
        filename.append(primitiveFiles[i]);

        if (! JFileSystem::GetInstance()->FileExists(filename))
            continue;
        primitives.push_back(filename);
    }
    primitivesLoadCounter = 0;
    LOG("GameStateMenu::listPrimitives - Done");
}

void GameStateMenu::ensureMGuiController()
{
    if (!mGuiController)
    {
        mGuiController = NEW JGuiController(JGE::GetInstance(), 100, this);
        if (mGuiController)
        {
            WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MENU_FONT);
            mFont->SetColor(ARGB(255,255,255,255));
            vector<ModRulesMainMenuItem *>items = gModRules.menu.main;

            int numItems = (int)items.size();
            float startX = 80.f;
            float totalSize = SCREEN_WIDTH_F - (2 * startX);
            float space = 0;
            if (numItems < 2)
                startX = SCREEN_WIDTH_F/2;
            else 
                space = totalSize/(numItems - 1);

            for (size_t i = 0; i < items.size(); ++i) {
                ModRulesMainMenuItem * item = items[i];
                int iconId = (item->mIconId - 1) * 2;
                mGuiController->Add(NEW MenuItem(
                    item->mActionId, 
                    mFont, item->mDisplayName, 
                    startX + (i * space), 40 + SCREEN_HEIGHT / 2, 
                    mIcons[iconId].get(), mIcons[iconId + 1].get(),
                    item->mParticleFile.c_str(), WResourceManager::Instance()->GetQuad("particles").get(),
                    (i == 0)));
            }

            JQuadPtr jq = WResourceManager::Instance()->RetrieveTempQuad("button_shoulder.png");
            if (!jq.get()) return;
            jq->SetHFlip(false);
            jq->SetColor(ARGB(abs(255),255,255,255));
            mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
            vector<ModRulesOtherMenuItem *>otherItems = gModRules.menu.other;
            if (otherItems.size()) {
                mGuiController->Add(NEW OtherMenuItem(
                                       otherItems[0]->mActionId,
                                       mFont, otherItems[0]->mDisplayName,
                                       SCREEN_WIDTH - 64, 2,
                                       jq.get(), jq.get(), otherItems[0]->mKey, false
                                       ));
            }
        }
    }
}

void GameStateMenu::Update(float dt)
{
#ifdef NETWORK_SUPPORT
    if (options.keypadActive())
    {
        options.keypadUpdate(dt);

        if (mParent->mServerAddress != "")
        {
            mParent->mServerAddress = options.keypadFinish();

            if (mParent->mServerAddress != "")
            {
                mParent->mpNetwork = new JNetwork();

                mParent->mpNetwork->connect(mParent->mServerAddress);
                // we let the server choose the game mode
                mParent->gameType = GAME_TYPE_SLAVE;
			          // just to select one, the HOST is in control here.
                mParent->rules = Rules::getRulesByFilename("classic.txt");
                hasChosenGameType = true;
                subMenuController->Close();
                currentState = MENU_STATE_NETWORK_WAIT | MENU_STATE_MINOR_SUBMENU_CLOSING;
            }
            mParent->mServerAddress = "";
        }
        //Prevent screen from updating.
        return;
    }
#endif //NETWORK_SUPPORT

    timeIndex += dt * 2;
    switch (MENU_STATE_MAJOR & currentState)
    {
    case MENU_STATE_MAJOR_LANG:
        if (MENU_STATE_MINOR_NONE == (currentState & MENU_STATE_MINOR))
        {
            if (!subMenuController)
                loadLangMenu();
        }
        if (!langChoices)
        {
            currentState = MENU_STATE_MAJOR_LOADING_CARDS;
            SAFE_DELETE(subMenuController);
        }
        else
            subMenuController->Update(dt);
        break;
    case MENU_STATE_MAJOR_LOADING_CARDS:
        if (primitivesLoadCounter == -1)
        {
            listPrimitives();
        }
        if (primitivesLoadCounter < (int) (primitives.size()))
        {
#ifdef _DEBUG
            int startTime = JGEGetTime();
#endif
            MTGCollection()->load(primitives[primitivesLoadCounter].c_str());
#if _DEBUG
            int endTime = JGEGetTime();
            int elapsedTime = (endTime - startTime);
            DebugTrace("Time elapsed while loading " << primitives[primitivesLoadCounter] << " : " << elapsedTime << " ms");
#endif

            primitivesLoadCounter++;
            break;
        }
        primitivesLoadCounter = primitives.size() + 1;
        if (mReadConf)
        {
            MTGCollection()->load(mCurrentSetFileName.c_str(), mCurrentSetName.c_str());
        }
        else
        {
            mReadConf = 1;
        }
        if (!nextSetFolder("sets/", "_cards.dat"))
        {
            //Remove temporary translations
            Translator::GetInstance()->tempValues.clear();

            DebugTrace(std::endl << "==" << std::endl <<
                            "Total MTGCards: " << MTGCollection()->collection.size() << std::endl <<
                            "Total CardPrimitives: " << MTGCollection()->primitives.size() << std::endl << "==");

            //Force default, if necessary.
            if (options[Options::ACTIVE_PROFILE].str == "")
                options[Options::ACTIVE_PROFILE].str = "Default";

            //Release splash texture
            WResourceManager::Instance()->Release(splashTex);
            splashTex = NULL;

            //check for deleted collection / first-timer
            if (JFileSystem::GetInstance()->FileExists(options.profileFile(PLAYER_COLLECTION)))
            {
                currentState = MENU_STATE_MAJOR_MAINMENU;
            }
            else
            {
                currentState = MENU_STATE_MAJOR_FIRST_TIME;
            }

            //Reload list of unlocked sets, now that we know about the sets.
            options.reloadProfile();
            genNbCardsStr();
            //All major things have been loaded, resize the cache to use it as efficiently as possible
            WResourceManager::Instance()->ResetCacheLimits();
        }
        break;
    case MENU_STATE_MAJOR_FIRST_TIME:
        currentState &= MENU_STATE_MAJOR_MAINMENU;
        options.reloadProfile(); //Handles building a new deck, if needed.
        break;
    case MENU_STATE_MAJOR_MAINMENU: 
        {
            if (!scrollerSet)
                fillScroller();
            ensureMGuiController();
            if (mGuiController)
                mGuiController->Update(dt);

            //Hook for Top Menu actions
            vector<ModRulesOtherMenuItem *>items = gModRules.menu.other;
            for (size_t i = 0; i < items.size(); ++i)
            {
                if (mEngine->GetButtonState(items[i]->mKey) && items[i]->getMatchingGameState())
                     mParent->DoTransition(TRANSITION_FADE, items[i]->getMatchingGameState()); //TODO: Add the transition as a parameter in the rules file
            }
            break;
         }
#ifdef NETWORK_SUPPORT
    case MENU_STATE_NETWORK_DEFINE:
        if(MENU_STATE_MINOR_NONE == (currentState & MENU_STATE_MINOR))
        {
			currentState = MENU_STATE_MAJOR_SUBMENU;
			subMenuController = NEW SimpleMenu(JGE::GetInstance(), MENU_FIRST_DUEL_SUBMENU, this, Fonts::MENU_FONT, 150, 60);
			if (subMenuController)
			{
				subMenuController->Add(SUBMENUITEM_HOST_GAME, "Host a game");
				subMenuController->Add(SUBMENUITEM_JOIN_GAME, "Join a game");
				subMenuController->Add(SUBMENUITEM_CANCEL, "Cancel");
			}
		}
		break;
    case MENU_STATE_NETWORK_WAIT:
        if(MENU_STATE_MINOR_NONE == (currentState & MENU_STATE_MINOR))
        {
            if(mParent->mpNetwork->isConnected())
            {
                if(subMenuController) subMenuController->Close();
                currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
            }
            else if(!subMenuController)
            {
//                currentState = MENU_STATE_MAJOR_SUBMENU;
                subMenuController = NEW SimpleMenu(JGE::GetInstance(), MENU_FIRST_DUEL_SUBMENU, this, Fonts::MENU_FONT, 150, 60);
                if (subMenuController)
                {
                    subMenuController->Add(SUBMENUITEM_CANCEL, "Cancel connection");
                }
            }
            else{
                if (subMenuController)
                    subMenuController->Update(dt);
                ensureMGuiController();
                mGuiController->Update(dt);
                break;
            }
        }
        break;
#endif //NETWORK_SUPPORT
    case MENU_STATE_MAJOR_SUBMENU:
        if (subMenuController)
            subMenuController->Update(dt);
        ensureMGuiController();
        mGuiController->Update(dt);
        break;
    case MENU_STATE_MAJOR_DUEL:
        if (MENU_STATE_MINOR_NONE == (currentState & MENU_STATE_MINOR))
        {
            if (!hasChosenGameType)
            {
                currentState = MENU_STATE_MAJOR_SUBMENU;
                subMenuController = NEW SimpleMenu(JGE::GetInstance(), MENU_FIRST_DUEL_SUBMENU, this, Fonts::MENU_FONT, 150, 60);
                if (subMenuController)
                {
                    for (size_t i = 0; i < Rules::RulesList.size(); ++i)
                    {
                        Rules * rules = Rules::RulesList[i];
                        bool unlocked = rules->unlockOption == INVALID_OPTION 
                            ? (rules->mUnlockOptionString.size() == 0 ||  options[rules->mUnlockOptionString].number !=0)
                            :  options[rules->unlockOption].number != 0;
                        if (!rules->hidden && (unlocked))
                        {
                            subMenuController->Add(SUBMENUITEM_END_OFFSET + i, rules->displayName.c_str());
                        }
                    }
                    subMenuController->Add(SUBMENUITEM_CANCEL, "Cancel");
                }
            }
            else
            {
                if (mParent->gameType == GAME_TYPE_STORY)
                    mParent->DoTransition(TRANSITION_FADE, GAME_STATE_STORY);
                else
                    mParent->DoTransition(TRANSITION_FADE, GAME_STATE_DUEL);
                currentState = MENU_STATE_MAJOR_MAINMENU;
            }
        }
    }

    switch (MENU_STATE_MINOR & currentState)
    {
    case MENU_STATE_MINOR_SUBMENU_CLOSING:
        if (!subMenuController)
        { //http://code.google.com/p/wagic/issues/detail?id=379
            currentState &= ~MENU_STATE_MINOR_SUBMENU_CLOSING;
            break;
        }
        if (subMenuController->isClosed())
        {
            SAFE_DELETE(subMenuController);
            currentState &= ~MENU_STATE_MINOR_SUBMENU_CLOSING;
        }
        else
            subMenuController->Update(dt);
        break;
    case MENU_STATE_MINOR_NONE:
        ;// Nothing to do.
    }

    scroller->Update(dt);
    if ((currentState & MENU_STATE_MINOR) == MENU_STATE_MINOR_FADEIN)
    {
        currentState = currentState ^ MENU_STATE_MINOR_FADEIN;
        mParent->DoAnimation(TRANSITION_FADE_IN, 0.15f);
    }
}

//Renders the "sub" menu with shoulder button links
void GameStateMenu::RenderTopMenu()
{
    float leftTextPos = 10;
    float rightTextPos = SCREEN_WIDTH - 10;
    JRenderer * renderer = JRenderer::GetInstance();

    vector<ModRulesOtherMenuItem *>items = gModRules.menu.other;
    for (size_t i = 0; i < items.size(); ++i)
    {
        switch(items[i]->mKey)
        {
        case JGE_BTN_PREV:
            leftTextPos += 64;
            break;
        case JGE_BTN_NEXT:
            rightTextPos -= 64;
            break;
        default:
            DebugTrace("not supported yet!");
            break;
        }
    }

    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    mFont->SetColor(ARGB(128,255,255,255));
    mFont->DrawString(GAME_VERSION, rightTextPos, 5, JGETEXT_RIGHT);
    mFont->DrawString(nbcardsStr, leftTextPos, 5);
    renderer->FillRect(leftTextPos, 26, 104, 8, ARGB(255, 100, 90, 60));
    renderer->FillRect(leftTextPos + 2, 28, (float)(gamePercentComplete()), 4, ARGB(255,220,200, 125));
    char buf[512];
    sprintf(buf, _("achieved: %i%%").c_str(), gamePercentComplete());
    mFont->DrawString(buf, (leftTextPos + 104) / 2, 35, JGETEXT_CENTER);
    mFont->SetScale(1.f);
    mFont->SetColor(ARGB(255,255,255,255));
}

void GameStateMenu::Render()
{
    if ((currentState & MENU_STATE_MINOR) == MENU_STATE_MINOR_FADEIN)
        return;

    JRenderer * renderer = JRenderer::GetInstance();
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MENU_FONT);
    if ((currentState & MENU_STATE_MAJOR) == MENU_STATE_MAJOR_LANG)
    {
    }
    else if ((currentState & MENU_STATE_MAJOR) == MENU_STATE_MAJOR_LOADING_CARDS)
    {
        string wp = loadRandomWallpaper();
        if (wp.size())
        {
            JTexture * wpTex = WResourceManager::Instance()->RetrieveTexture(wp);
            if (wpTex)
            {
                JQuadPtr wpQuad = WResourceManager::Instance()->RetrieveTempQuad(wp);
                renderer->RenderQuad(wpQuad.get(), 0, 0, 0, SCREEN_WIDTH_F / wpQuad->mWidth, SCREEN_HEIGHT_F / wpQuad->mHeight);
            }
        }

        char text[512];
        if (mCurrentSetName.size())
        {
            sprintf(text, _("LOADING SET: %s").c_str(), mCurrentSetName.c_str());
        }
        else
        {
            if (primitivesLoadCounter <= (int) (primitives.size()))
                sprintf(text, _("LOADING PRIMITIVES").c_str());
            else
                sprintf(text, _("LOADING...").c_str());
        }
        mFont->SetColor(ARGB(170,0,0,0));
        mFont->DrawString(text, SCREEN_WIDTH / 2 + 2, SCREEN_HEIGHT - 50 + 2, JGETEXT_CENTER);
        mFont->SetColor(ARGB(255,255,255,255));
        mFont->DrawString(text, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, JGETEXT_CENTER);
    }
    else
    {
        PIXEL_TYPE colors[] = {

        ARGB(255,3,3,0), ARGB(255,8,8,0), ARGB(255,21,21,10), ARGB(255,50,50,30), };
        renderer->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, colors);

        if (mGuiController)
            mGuiController->Render();

        renderer->FillRoundRect(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT, 191, 6, 5, ARGB(100,10,5,0));
        scroller->Render();

        if (mBg.get())
            renderer->RenderQuad(mBg.get(), SCREEN_WIDTH / 2, 50);

        RenderTopMenu();

    }
    if (subMenuController)
    {
        subMenuController->Render();
    }

    if (options.keypadActive()) options.keypadRender();
}

void GameStateMenu::ButtonPressed(int controllerId, int controlId)
{
    DebugTrace("GameStateMenu: controllerId " << controllerId << " selected");
    switch (controllerId)
    {
    case MENU_LANGUAGE_SELECTION:
        if ( controlId == kInfoMenuID )
            break;
        setLang(controlId);
        WResourceManager::Instance()->ReloadWFonts(); // Fix for choosing Chinese language at first time.
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
            subMenuController = NEW SimpleMenu(JGE::GetInstance(), MENU_FIRST_DUEL_SUBMENU, this, Fonts::MENU_FONT, 150, 60);
            if (subMenuController)
            {
#ifdef NETWORK_SUPPORT
                subMenuController->Add(SUBMENUITEM_1PLAYER, "1 Player");
#else
                subMenuController->Add(SUBMENUITEM_1PLAYER, _("Play Game").c_str());
#endif
                // TODO Put 2 players mode back
                // This requires to fix the hand (to accept 2 players) OR to implement network game
#ifdef NETWORK_SUPPORT
                subMenuController->Add(SUBMENUITEM_2PLAYERS, "2 Players");
#endif //NETWORK_SUPPORT
                subMenuController->Add(SUBMENUITEM_DEMO, _("Demo").c_str());
                subMenuController->Add(SUBMENUITEM_CANCEL, _("Cancel").c_str());
#ifdef TESTSUITE
                if (Rules::getRulesByFilename("testsuite.txt"))
                    subMenuController->Add(SUBMENUITEM_TESTSUITE, "Test Suite");
#endif

#ifdef AI_CHANGE_TESTING
                subMenuController->Add(SUBMENUITEM_TESTAI, "AI A/B Testing");
#endif
                currentState = MENU_STATE_MAJOR_SUBMENU | MENU_STATE_MINOR_NONE;
            }
            break;

        case MENUITEM_DECKEDITOR:
        case MENUITEM_SHOP:
        case MENUITEM_OPTIONS:
        case MENUITEM_TROPHIES:
            mParent->DoTransition(TRANSITION_FADE,  ModRulesMenuItem::getMatchingGameState(controlId));
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
#ifdef NETWORK_SUPPORT
        case SUBMENUITEM_2PLAYERS:
            mParent->players[0] = PLAYER_TYPE_HUMAN;
            mParent->players[1] = PLAYER_TYPE_HUMAN;
            subMenuController->Close();
            currentState = MENU_STATE_NETWORK_DEFINE | MENU_STATE_MINOR_SUBMENU_CLOSING;
            break;
        case SUBMENUITEM_HOST_GAME:
        {
            if(!mParent->mpNetwork)
            {
                mParent->mpNetwork = new JNetwork();
            }
            mParent->mpNetwork->connect();
            subMenuController->Close();
            currentState = MENU_STATE_NETWORK_WAIT | MENU_STATE_MINOR_SUBMENU_CLOSING;
            break;
        }
        case SUBMENUITEM_JOIN_GAME:
        {
            if(!mParent->mpNetwork)
            {
                options.keypadStart("127.0.0.1", &(mParent->mServerAddress), true, true);
                options.keypadTitle("Enter device address to connect");
            }
            break;
        }
#endif //NETWORK_SUPPORT
        case SUBMENUITEM_DEMO:
            mParent->players[0] = PLAYER_TYPE_CPU;
            mParent->players[1] = PLAYER_TYPE_CPU;
            mParent->gameType = GAME_TYPE_DEMO;
            subMenuController->Close();
            currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
            break;
        case SUBMENUITEM_CANCEL:
        case kInfoMenuID: // Triangle button
            if (subMenuController != NULL)
            {
                subMenuController->Close();
            }
#ifdef NETWORK_SUPPORT
			SAFE_DELETE(mParent->mpNetwork);
#endif //NETWORK_SUPPORT
            currentState = MENU_STATE_MAJOR_MAINMENU | MENU_STATE_MINOR_SUBMENU_CLOSING;
            break;
#ifdef AI_CHANGE_TESTING
        case SUBMENUITEM_TESTAI:
             options[Options::AIDECKS_UNLOCKED].number = 5000; //hack to force-test all decks
            mParent->players[0] = PLAYER_TYPE_CPU_TEST;
            mParent->players[1] = PLAYER_TYPE_CPU_TEST;
            mParent->gameType = GAME_TYPE_DEMO;
            subMenuController->Close();
            currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
            break;
#endif
#ifdef TESTSUITE
            case SUBMENUITEM_TESTSUITE:
            mParent->rules = Rules::getRulesByFilename("testsuite.txt");
            this->hasChosenGameType = true;
            mParent->gameType = GAME_TYPE_CLASSIC;
            mParent->players[0] = PLAYER_TYPE_TESTSUITE;
            mParent->players[1] = PLAYER_TYPE_TESTSUITE;
            subMenuController->Close();
            currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
            break;
#endif
            default: //Game modes
            this->hasChosenGameType = true;
            mParent->rules = Rules::RulesList[controlId - SUBMENUITEM_END_OFFSET];
            mParent->gameType = (mParent->rules->gamemode); //TODO can we get rid of gameType in the long run, since it is also stored in the rules object ?
            subMenuController->Close();
            currentState = MENU_STATE_MAJOR_DUEL | MENU_STATE_MINOR_SUBMENU_CLOSING;
            break;
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
                 << " ; mCurrentSetName : " << mCurrentSetName
                 << " ; mCurrentSetFileName : " << mCurrentSetFileName
                 << " ; mReadConf : " << mReadConf
                 << " ; timeIndex : " << timeIndex;
}
