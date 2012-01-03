#include "PrecompiledHeader.h"

#include "GameStateOptions.h"
#include "GameApp.h"
#include "OptionItem.h"
#include "SimpleMenu.h"
#include "SimplePad.h"
#include "Translate.h"

namespace
{
    const int kSaveAndBackToMainMenuID = 1;
    const int kBackToMainMenuID = 2;
    const int kNewProfileID = 4;
    const int kReloadID = 5;

}

GameStateOptions::GameStateOptions(GameApp* parent) :
    GameState(parent, "options"), mReload(false), grabber(NULL), optionsMenu(NULL), optionsTabs(NULL)
{
}
GameStateOptions::~GameStateOptions()
{
}

void GameStateOptions::Start()
{
    newProfile = "";
    timer = 0;
    mState = SHOW_OPTIONS;
    JRenderer::GetInstance()->EnableVSync(true);

    WGuiList * optionsList;

    optionsList = NEW WGuiList("Settings");

    optionsList->Add(NEW WGuiHeader("General Options"));
    if (GameApp::HasMusic)
        optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::MUSICVOLUME, "Music volume", 100, 10, 100),
                        OptionVolume::getInstance()));
    optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::SFXVOLUME, "SFX volume", 100, 10, 100), OptionVolume::getInstance()));
    if (options[Options::DIFFICULTY_MODE_UNLOCKED].number)
    {
        optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::DIFFICULTY, "Difficulty", 3, 1, 0),
                        OptionDifficulty::getInstance()));
        optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::ECON_DIFFICULTY, "Economic Difficuly", Constants::ECON_EASY)));
    }
    optionsList->Add(NEW OptionInteger(Options::INTERRUPT_SECONDS, "Seconds to pause for an Interrupt", 20, 1));
    optionsList->Add(NEW OptionInteger(Options::INTERRUPTMYSPELLS, "Interrupt my spells"));
    optionsList->Add(NEW OptionInteger(Options::INTERRUPTMYABILITIES, "Interrupt my abilities"));
    optionsList->Add(NEW OptionInteger(Options::INTERRUPT_SECONDMAIN, "Interrupt opponent's end of turn"));
    optionsTabs = NEW WGuiTabMenu();
    optionsTabs->Add(optionsList);

    optionsList = NEW WGuiList("Game");
    optionsList->Add(NEW WGuiHeader("Interface Options"));
    optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::CLOSEDHAND, "Closed hand", 1, 1, 0)));
    optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::HANDDIRECTION, "Hand direction", 1, 1, 0)));
    optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::MANADISPLAY, "Mana display", 3, 1, 0)));
    optionsList->Add(NEW OptionInteger(Options::REVERSETRIGGERS, "Reverse left and right triggers"));
    optionsList->Add(NEW OptionInteger(Options::DISABLECARDS, "Disable card images"));
    optionsList->Add(NEW OptionInteger(Options::TRANSITIONS, "Disable screen transitions"));
    optionsList->Add(NEW OptionInteger(Options::OSD, "Display InGame extra information"));
    optionsTabs->Add(optionsList);

    optionsList = NEW WGuiList("User");
    optionsList->Add(NEW WGuiHeader("User Options"));
    WDecoConfirm * cPrf = NEW WDecoConfirm(this, NEW OptionProfile(mParent, this));
    cPrf->confirm = "Use this Profile";
    OptionThemeStyle * ots = NEW OptionThemeStyle("Theme Style");
    OptionDirectory * od = NEW OptionTheme(ots);
    WDecoConfirm * cThm = NEW WDecoConfirm(this, od);
    cThm->confirm = "Use this Theme";

    WDecoConfirm * cStyle = NEW WDecoConfirm(this, ots);
    cStyle->confirm = "Use this Style";

    optionsList->Add(NEW WGuiSplit(cPrf, cThm));
    optionsList->Add(cStyle);
    optionsList->Add(NEW WGuiButton(NEW WGuiHeader("New Profile"), -102, kNewProfileID, this));

    optionsList->Add(NEW WDecoCheat(NEW OptionInteger(Options::CHEATMODE, "Enable Cheat Mode")));
		optionsList->Add(NEW WDecoCheat(NEW OptionInteger(Options::OPTIMIZE_HAND, "Optimize Starting Hand")));
		optionsList->Add(NEW WDecoCheat(NEW OptionInteger(Options::CHEATMODEAIDECK, "Unlock All Ai Decks")));

    optionsTabs->Add(optionsList);

    optionsList = NEW WGuiList("Advanced");
    optionsList->Add(NEW WGuiHeader("Advanced Options"));
    WDecoStyled * wAdv = NEW WDecoStyled(NEW WGuiHeader("The following options require a restart."));
    wAdv->mStyle = WDecoStyled::DS_STYLE_ALERT;
    optionsList->Add(wAdv);
    WDecoConfirm * cLang = NEW WDecoConfirm(this, NEW OptionLanguage("Language"));
    cLang->confirm = "Use this Language";
    optionsList->Add(cLang);
    WDecoEnum * oGra = NEW WDecoEnum(NEW OptionInteger(Options::MAX_GRADE, "Minimum Card Grade", Constants::GRADE_DANGEROUS, 1,
                    Constants::GRADE_BORDERLINE, "", Constants::GRADE_SUPPORTED));
    optionsList->Add(oGra);
    WDecoEnum * oASPhases = NEW WDecoEnum(NEW OptionInteger(Options::ASPHASES, "Phase Skip Automation", Constants::ASKIP_FULL, 1,
                    Constants::ASKIP_NONE, "", Constants::ASKIP_NONE));
    optionsList->Add(oASPhases);
    optionsTabs->Add(optionsList);

    WDecoEnum * oFirstPlayer = NEW WDecoEnum(NEW OptionInteger(Options::FIRSTPLAYER, "First Turn Player", Constants::WHO_R, 1,
                    Constants::WHO_P, "", Constants::WHO_P));
    optionsList->Add(oFirstPlayer);
    
    WDecoEnum * oKickerPay = NEW WDecoEnum(NEW OptionInteger(Options::KICKERPAYMENT, "Kicker Cost", Constants::KICKER_CHOICE, 1,
        Constants::KICKER_ALWAYS, "", Constants::KICKER_ALWAYS));
    optionsList->Add(oKickerPay);
#ifndef IOS
    optionsList = NEW WGuiKeyBinder("Key Bindings", this);
    optionsTabs->Add(optionsList);
#endif
    optionsList = NEW WGuiList("Credits");
    optionsList->failMsg = "";
    optionsTabs->Add(optionsList);

    optionsMenu = NEW SimpleMenu(JGE::GetInstance(), -102, this, Fonts::MENU_FONT, 50, 170);
    optionsMenu->Add(kBackToMainMenuID, "Back to Main Menu");
    optionsMenu->Add(kSaveAndBackToMainMenuID, "Save & Back to Main Menu");
    optionsMenu->Add(kCancelMenuID, "Cancel");

    optionsTabs->Entering(JGE_BTN_NONE);
}

void GameStateOptions::End()
{
    JRenderer::GetInstance()->EnableVSync(false);
    SAFE_DELETE(optionsTabs);
    SAFE_DELETE(optionsMenu);
}

void GameStateOptions::Update(float dt)
{
    timer += dt * 10;

    if (options.keypadActive())
    {
        options.keypadUpdate(dt);

        if (newProfile != "")
        {
            newProfile = options.keypadFinish();
            if (newProfile != "")
            {
                options[Options::ACTIVE_PROFILE] = newProfile;
                options.reloadProfile();
                optionsTabs->Reload();
            }
            newProfile = "";
        }
    }
    else
        switch (mState)
        {
        default:
        case SAVE:
            switch (optionsTabs->needsConfirm())
            {
            case WGuiBase::CONFIRM_CANCEL:
                mState = SHOW_OPTIONS;
                break;
            case WGuiBase::CONFIRM_OK:
                optionsTabs->save();
                JSoundSystem::GetInstance()->SetSfxVolume(options[Options::SFXVOLUME].number);
                JSoundSystem::GetInstance()->SetMusicVolume(options[Options::MUSICVOLUME].number);
                mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
                mState = SHOW_OPTIONS;
                break;
            case WGuiBase::CONFIRM_NEED:
                optionsTabs->yieldFocus();
                break;
            }
            // Note : No break here : must continue to continue updating the menu elements.
        case SHOW_OPTIONS:
        {
            JGE* j = JGE::GetInstance();
            JButton key = JGE_BTN_NONE;
            int x, y;
            if (grabber)
            {
                LocalKeySym sym;
                if (LOCAL_KEY_NONE != (sym = j->ReadLocalKey()))
                    grabber->KeyPressed(sym);
            }
            else
                while ((key = JGE::GetInstance()->ReadButton()) || JGE::GetInstance()->GetLeftClickCoordinates(x,y))
                {
                    if (!optionsTabs->CheckUserInput(key) && key == JGE_BTN_MENU)
                        mState = SHOW_OPTIONS_MENU;
                }
            optionsTabs->Update(dt);
            break;
        }
        case SHOW_OPTIONS_MENU:
            optionsMenu->Update(dt);
            break;
        }
    if (mReload)
    {
        options.reloadProfile();
        Translator::EndInstance();
        Translator::GetInstance()->init();
        optionsTabs->Reload();
        mReload = false;
    }
}

void GameStateOptions::Render()
{
    //Erase
    JRenderer::GetInstance()->ClearScreen(ARGB(0,0,0,0));

    const char * const CreditsText[] = {
        "Wagic, The Homebrew?! by Wololo",
        "",
        "updates, new cards, and more on http://wololo.net/wagic",
        "Many thanks to the people who help this project",
        "",
        "",
        "Art: Ilya B, Julio, Jeck, J, Kaioshin, Lakeesha",
        "Check themeinfo.txt for the full credits of each theme!",
        "",
        "Dev Team:",
        "Abrasax, almosthumane, Daddy32, DJardin, Dr.Solomat,",
        "J, Jeck, Leungclj, linshier, Mootpoint, Mnguyen, Psyringe,",
        "Salmelo, Superhiro, Wololo, Yeshua, Zethfox",
        "",
        "Music by Celestial Aeon Project, http://www.jamendo.com",
        "",
        "Deck Builders: Abrasax, AzureKnight, colarchon",
        "Hehotfarv, Jeremy, Jog1118, JonyAS, Kaioshin",
        "Lachaux, Link17, Muddobbers, Nakano, Niegen",
        "Psyringe, r1c47, Superhiro, Szei, Thanatos02",
        "Whismer, Wololo",
        "",
        "Thanks also go to Dr.Watson, Orine, Raphael, Sakya, Tyranid",
        "for their help.",
        "",
        "Thanks to everyone who contributes code/content on the forums!",
        "",
        "Developed with the JGE++ Library (http://code.google.com/p/wagic)",
        "SFX From www.soundsnap.com",

        "",
        "",
        "This work is not related to or endorsed by Wizards of the Coast, Inc",
        "",
        "Please support this project with donations at http://wololo.net/wagic",
    };

    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAGIC_FONT);
    mFont->SetColor(ARGB(255,200,200,200));
    mFont->SetScale(1.0);
    float startpos = 272 - timer;
    float pos = startpos;
    int size = sizeof(CreditsText) / sizeof(CreditsText[0]);

    for (int i = 0; i < size; i++)
    {
        pos = startpos + 20 * i;
        if (pos > -20 && pos < SCREEN_HEIGHT + 20)
        {
            mFont->DrawString(CreditsText[i], SCREEN_WIDTH / 2, pos, JGETEXT_CENTER);
        }
    }

    if (pos < -20)
        timer = 0;

    optionsTabs->Render();

    if (mState == SHOW_OPTIONS_MENU)
        optionsMenu->Render();

    if (options.keypadActive())
        options.keypadRender();
}

void GameStateOptions::ButtonPressed(int controllerId, int controlId)
{
    //Exit menu?
    if (controllerId == -102)
        switch (controlId)
        {
        case kSaveAndBackToMainMenuID:
            mState = SAVE;
            break;
            //Set Audio volume
        case kBackToMainMenuID:
            mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
            break;
        case kCancelMenuID:
            mState = SHOW_OPTIONS;
            break;
        case kNewProfileID:
            options.keypadStart("", &newProfile);
            options.keypadTitle("New Profile");
            break;
        case kReloadID:
            mReload = true;
            break;
        }
    else
        optionsTabs->ButtonPressed(controllerId, controlId);
}
;

void GameStateOptions::GrabKeyboard(KeybGrabber* g)
{
    grabber = g;
}
void GameStateOptions::UngrabKeyboard(const KeybGrabber* g)
{
    if (g == grabber)
        grabber = NULL;
}
