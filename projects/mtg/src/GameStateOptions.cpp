#include "../include/config.h"
#include "../include/GameStateOptions.h"
#include "../include/GameApp.h"
#include "../include/OptionItem.h"
#include "../include/SimpleMenu.h"
#include "../include/SimplePad.h"
#include "../include/GameOptions.h"
#include "../include/Translate.h"

GameStateOptions::GameStateOptions(GameApp* parent): GameState(parent), mReload(false), grabber(NULL), optionsMenu(NULL), optionsTabs(NULL) {}
GameStateOptions::~GameStateOptions() {}

void GameStateOptions::Start()
{
  newProfile = "";
  timer =  0;
  mState = SHOW_OPTIONS;
  JRenderer::GetInstance()->EnableVSync(true);

  WGuiList * optionsList;

  optionsList = NEW WGuiList("Settings");

  optionsList->Add(NEW WGuiHeader("General Options"));
  if (GameApp::HasMusic)
    optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::MUSICVOLUME,"Music volume",100,10,100),OptionVolume::getInstance()));
  optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::SFXVOLUME,"SFX volume",100,10,100),OptionVolume::getInstance()));
  optionsList->Add(NEW OptionInteger(Options::OSD, "Display InGame extra information"));
  if (options[Options::DIFFICULTY_MODE_UNLOCKED].number){
    optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::DIFFICULTY,"Difficulty",3,1,0),OptionDifficulty::getInstance()));
    optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::ECON_DIFFICULTY,"Economic Difficuly",Constants::ECON_EASY)));
  }
  optionsList->Add(NEW OptionInteger(Options::INTERRUPT_SECONDS, "Seconds to pause for an Interrupt", 20, 1));
  optionsList->Add(NEW OptionInteger(Options::INTERRUPTMYSPELLS, "Interrupt my spells"));
  optionsList->Add(NEW OptionInteger(Options::INTERRUPTMYABILITIES, "Interrupt my abilities"));
  optionsList->Add(NEW OptionInteger(Options::INTERRUPT_SECONDMAIN, "Interrupt opponent's end of turn"));
  optionsTabs = NEW WGuiTabMenu();
  optionsTabs->Add(optionsList);

  optionsList = NEW WGuiList("Game");
  optionsList->Add(NEW WGuiHeader("Interface Options"));
  optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::CLOSEDHAND,"Closed hand",1,1,0)));
  optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::HANDDIRECTION,"Hand direction",1,1,0)));
  optionsList->Add(NEW WDecoEnum(NEW OptionInteger(Options::MANADISPLAY,"Mana display",2,1,0)));
  optionsList->Add(NEW OptionInteger(Options::REVERSETRIGGERS, "Reverse left and right triggers"));
  optionsList->Add(NEW OptionInteger(Options::DISABLECARDS,"Disable card images"));
  optionsList->Add(NEW OptionInteger(Options::TRANSITIONS,"Disable screen transitions"));
  optionsTabs->Add(optionsList);

  optionsList = NEW WGuiList("User");
  optionsList->Add(NEW WGuiHeader("User Options"));
  WDecoConfirm * cPrf = NEW WDecoConfirm(this,NEW OptionProfile(mParent,this));
  cPrf->confirm = "Use this Profile";
  OptionDirectory * od = NEW OptionTheme();
  WDecoConfirm * cThm = NEW WDecoConfirm(this,od);
  cThm->confirm = "Use this Theme";

  optionsList->Add(NEW WGuiSplit(cPrf,cThm));
  optionsList->Add(NEW WGuiButton(NEW WGuiHeader("New Profile"),-102,4,this));
  optionsList->Add(NEW WDecoCheat(NEW OptionInteger(Options::CHEATMODE, "Enable cheat mode")));
  optionsTabs->Add(optionsList);

  optionsList = NEW WGuiList("Advanced");
  optionsList->Add(NEW WGuiHeader("Advanced Options"));
  WDecoStyled * wAdv = NEW WDecoStyled(NEW WGuiHeader("The following options require a restart."));
  wAdv->mStyle = WDecoStyled::DS_STYLE_ALERT;
  optionsList->Add(wAdv);
  WDecoConfirm * cLang = NEW WDecoConfirm(this,NEW OptionLanguage("Language"));
  cLang->confirm = "Use this Language";
  optionsList->Add(cLang);
  WDecoEnum * oGra = NEW WDecoEnum(NEW OptionInteger(Options::MAX_GRADE,"Minimum Card Grade",Constants::GRADE_DANGEROUS,1,Constants::GRADE_BORDERLINE,"",Constants::GRADE_SUPPORTED));
  optionsList->Add(oGra);
  optionsTabs->Add(optionsList);

  optionsList = NEW WGuiKeyBinder("Key Bindings", this);
  optionsTabs->Add(optionsList);

  optionsList = NEW WGuiList("Credits");
  optionsList->failMsg = "";
  optionsTabs->Add(optionsList);

  optionsMenu = NEW SimpleMenu(-102, this,Constants::MENU_FONT, 50,170);
  optionsMenu->Add(2, "Back to Main Menu");
  optionsMenu->Add(1, "Save & Back to Main Menu");
  optionsMenu->Add(3, "Cancel");

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

  if(options.keypadActive()){
    options.keypadUpdate(dt);

    if(newProfile != ""){
        newProfile = options.keypadFinish();
        if(newProfile != ""){
          options[Options::ACTIVE_PROFILE] = newProfile;
          options.reloadProfile(false);
          optionsTabs->Reload();
        }
        newProfile = "";
    }
  }
  else switch(mState){
    default:
    case SHOW_OPTIONS: {
        JGE* j = JGE::GetInstance();
        JButton key;
        if (grabber) {
            LocalKeySym sym;
            if (LOCAL_KEY_NONE != (sym = j->ReadLocalKey()))
              grabber->KeyPressed(sym);
          }
        else while ((key = JGE::GetInstance()->ReadButton())){
            if(!optionsTabs->CheckUserInput(key) && key == JGE_BTN_MENU)
              mState = SHOW_OPTIONS_MENU;
          }
        optionsTabs->Update(dt);
        break;
      }
    case SHOW_OPTIONS_MENU:
      optionsMenu->Update(dt);
      break;
    }
  if(mReload){
    options.reloadProfile(true);
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
      "Art: Ilya B, Julio, Jeck, J, Lakeesha",
      "Check themeinfo.txt for the full credits of each theme!",
      "",
      "Dev Team: Abrasax, Daddy32, Dr.Solomat, J, Jeck",
      "Leungclj, Superhiro, Psyringe, Wololo, Yeshua",
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
      "for their (sometimes indirect) help.",
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

  JLBFont * mFont = resources.GetJLBFont(Constants::MAGIC_FONT);
  mFont->SetColor(ARGB(255,200,200,200));
  mFont->SetScale(1.0);
  float startpos = 272 - timer;
  float pos = startpos;
  int size = sizeof(CreditsText) / sizeof(CreditsText[0]);

  for (int i = 0; i < size; i++){
    pos = startpos + 20 * i;
    if (pos > -20 && pos < SCREEN_HEIGHT + 20){
      mFont->DrawString(CreditsText[i],SCREEN_WIDTH/2,pos ,JGETEXT_CENTER);
    }
  }

  if (pos < -20)
    timer = 0;


  optionsTabs->Render();

  if(mState == SHOW_OPTIONS_MENU)
      optionsMenu->Render();

  if(options.keypadActive())
    options.keypadRender();
}

void GameStateOptions::ButtonPressed(int controllerId, int controlId)
{
  //Exit menu?
  if(controllerId == -102)
  switch (controlId){
  case 1:
    optionsTabs->save();
    //Set Audio volume
    JSoundSystem::GetInstance()->SetSfxVolume(options[Options::SFXVOLUME].number);
    JSoundSystem::GetInstance()->SetMusicVolume(options[Options::MUSICVOLUME].number);
  case 2:
    mParent->DoTransition(TRANSITION_FADE,GAME_STATE_MENU);
    break;
  case 3:
    mState = SHOW_OPTIONS;
    break;
  case 4:
    options.keypadStart("",&newProfile);
    options.keypadTitle("New Profile");
    break;
  case 5:
    mReload = true;
    break;
  }
  else
    optionsTabs->ButtonPressed(controllerId, controlId);
};

void GameStateOptions::GrabKeyboard(KeybGrabber* g) {
  grabber = g;
}
void GameStateOptions::UngrabKeyboard(const KeybGrabber* g) {
  if (g == grabber) grabber = NULL;
}
