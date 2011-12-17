#include "PrecompiledHeader.h"

#include "utils.h"
#include "MTGDeck.h"
#include "Translate.h"
#include "OptionItem.h"
#include "StyleManager.h"
#include "Credits.h"

#ifdef IOS
#include "JGE.h"
#endif

const string Options::optionNames[] = {
//Global options
  "Profile",
  "Lang",
//Options set on a per-profile basis
  "Theme",
  "Mode",
  "musicVolume",
  "sfxVolume",
  "difficulty",
  "cheatmode",
	"optimizedhand",
	"cheatmodedecks",
  "displayOSD",
  "closed_hand",
  "hand_direction",
  "mana_display",
  "reverse_triggers",
  "disable_cards",
  "maxGrade",
  "ASPhases",
  "FirstPlayer",
  "KickerPay",
  "economic_difficulty",
  "transitions",
  "bgStyle",
  "interruptSeconds",
#if defined(SDL_CONFIG)
  "keybindings_sdl",
#elif defined(QT_CONFIG)
  "keybindings_qt",
#elif defined(WIN32)
  "keybindings_win",
#elif defined(LINUX)
  "keybindings_x",
#else
  "keybindings_psp",
#endif
  "aidecks",
  "interruptMySpells",
  "interruptMyAbilities",
  "saveDetailedDeckInfo",
//General interrupts
  "interruptBeforeBegin",
  "interruptUntap",
  "interruptUpkeep",
  "interruptDraw",
  "interruptFirstMain",
  "interruptBeginCombat",
  "interruptAttackers",
  "interruptBlockers",
  "interruptDamage",
  "interruptEndCombat",
  "interruptSecondMain",
  "interruptEndTurn",
  "interruptCleanup",
  "interruptAfterEnd",
//Unlocked modes
  "prx_handler",
  "prx_eviltwin",
  "prx_rnddeck",
  "aw_collector",

};

// MARK:  Options
int Options::getID(string name)
{
    if (0 == name.size())
        return INVALID_OPTION;

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    //Is it a named option?
    for (int x = 0; x < LAST_NAMED; x++)
    {
        string lower = Options::optionNames[x];
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == name)
            return x;
    }

    //Is it an unlocked set?
    if (name.find("unlocked_") == 0)
    {
        string setname = name.substr(strlen("unlocked_"));
        if (setlist.size())
        {
            int unlocked = setlist[setname];
            if (unlocked != -1)
                return Options::optionSet(unlocked);
        }
    }

    //Failure.
    return INVALID_OPTION;
}

string Options::getName(int option)
{
    //Invalid options
    if (option < 0)
        return "";

    //Standard named options
    if (option < LAST_NAMED)
        return optionNames[option];

    //Unlocked sets.
    int setID = option - SET_UNLOCKS;
    char buf[512];
    if (setID < 0 || setID > setlist.size())
        return "";

    sprintf(buf, "unlocked_%s", setlist[setID].c_str());
    return buf;

    //Failed.
    return "";
}

int Options::optionSet(int setID)
{
    //Sanity check if possible
    if (setID < 0 || (setID > setlist.size()))
        return INVALID_OPTION;

    return SET_UNLOCKS + setID;
}

int Options::optionInterrupt(int gamePhase)
{
    //Huge, nearly illegible switch block spread out to improve readability.
    switch (gamePhase)
    {
    case MTG_PHASE_BEFORE_BEGIN:
        return INTERRUPT_BEFOREBEGIN;

    case MTG_PHASE_UNTAP:
        return INTERRUPT_UNTAP;

    case MTG_PHASE_UPKEEP:
        return INTERRUPT_UPKEEP;

    case MTG_PHASE_DRAW:
        return INTERRUPT_DRAW;

    case MTG_PHASE_FIRSTMAIN:
        return INTERRUPT_FIRSTMAIN;

    case MTG_PHASE_COMBATBEGIN:
        return INTERRUPT_BEGINCOMBAT;

    case MTG_PHASE_COMBATATTACKERS:
        return INTERRUPT_ATTACKERS;

    case MTG_PHASE_COMBATBLOCKERS:
        return INTERRUPT_BLOCKERS;

    case MTG_PHASE_COMBATDAMAGE:
        return INTERRUPT_DAMAGE;

    case MTG_PHASE_COMBATEND:
        return INTERRUPT_ENDCOMBAT;

    case MTG_PHASE_SECONDMAIN:
        return INTERRUPT_SECONDMAIN;

    case MTG_PHASE_ENDOFTURN:
        return INTERRUPT_ENDTURN;

    case MTG_PHASE_CLEANUP:
        return INTERRUPT_CLEANUP;

    case MTG_PHASE_AFTER_EOT:
        return INTERRUPT_AFTEREND;
    }

    return INVALID_OPTION;
}

// MARK:  -

// MARK:  GameOption

GameOption::GameOption(int value) :
    number(value)
{
}
GameOption::GameOption(string value) :
    number(atoi(value.c_str())), str(value)
{
}
GameOption::GameOption(int num, string str) :
    number(num), str(str)
{
}

bool GameOption::isDefault()
{
    string test = str;
    std::transform(test.begin(), test.end(), test.begin(), ::tolower);

    if (!test.size() || test == "default")
        return true;

    return false;
}

PIXEL_TYPE GameOption::asColor(PIXEL_TYPE fallback)
{
    unsigned char color[4];
    string temp;
    int subpixel = 0;

    //The absolute shortest a color could be is 5 characters: "0,0,0" (implicit 255 alpha)
    if (str.length() < 5)
        return fallback;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (isspace(str[i]))
            continue;
        if (str[i] == ',')
        {
            if (temp == "")
                return fallback;
            color[subpixel] = (unsigned char) atoi(temp.c_str());
            temp = "";
            subpixel++;
            continue;
        }
        else if (!isdigit(str[i]))
            return fallback;
        if (subpixel > 3)
            return fallback;
        temp += str[i];
    }

    if (temp != "")
        color[subpixel] = (unsigned char) atoi(temp.c_str());
    if (subpixel == 2)
        color[3] = 255;

    return ARGB(color[3],color[0],color[1],color[2]);
}

bool GameOption::read(string input)
{
    bool bNumeric = true;

    if (!input.size())
        return true; //Default reader doesn't care about invalid formatting.

    //Is it a number?
    for (size_t x = 0; x < input.size(); x++)
    {
        if (!isdigit(input[x]))
        {
            bNumeric = false;
            break;
        }
    }

    if (bNumeric)
        number = atoi(input.c_str());
    else
        str = input;
    return true;
}
string GameOption::menuStr()
{
    if (number)
    {
        char buf[12];
        sprintf(buf, "%d", number);
    }

    if (str.size())
        return str;

    return "0";
}
bool GameOption::write(std::ofstream * file, string name)
{
    char writer[1024];

    if (!file)
        return false;

    if (str == "")
    {
        if (number == 0) //This is absolutely default. No need to write it.
            return true;

        //It's a number!
        sprintf(writer, "%s=%d\n", name.c_str(), number);
    }
    else
        sprintf(writer, "%s=%s\n", name.c_str(), str.c_str());

    (*file) << writer;
    return true;
}

// MARK:  -

// MARK:  GameOptions
GameOptions::GameOptions(string filename)
{
    mFilename = filename;
    GameOptions::load();
}

int GameOptions::load()
{
    std::string contents;
    if (JFileSystem::GetInstance()->readIntoString(mFilename, contents))
    {
        std::stringstream stream(contents);
        string s;

        while (std::getline(stream, s))
        {
            if (!s.size())
                continue;
            if (s[s.size() - 1] == '\r')
                s.erase(s.size() - 1); //Handle DOS files
            int found = s.find("=");
            string name = s.substr(0, found);
            string val = s.substr(found + 1);
            int id = Options::getID(name);
            if (id == INVALID_OPTION)
            {
                if (!unknownMap[name]) unknownMap[name] = factorNewGameOption(name, val);
                continue;
            }

            (*this)[id].read(val);
        }
    }
    // (PSY) Make sure that cheatmode is switched off for ineligible profiles:
    if (options[Options::ACTIVE_PROFILE].str != SECRET_PROFILE)
		{
        (*this)[Options::CHEATMODE].number = 0;
		    (*this)[Options::OPTIMIZE_HAND].number = 0;
				(*this)[Options::CHEATMODEAIDECK].number = 0;
		}

    //Default values. Anywhere else to put those ?
    if (!(*this)[Options::MAX_GRADE].number)
        (*this)[Options::MAX_GRADE].number = Constants::GRADE_BORDERLINE;

    if (!(*this)[Options::AIDECKS_UNLOCKED].number)
        (*this)[Options::AIDECKS_UNLOCKED].number = 10;

    return 1;
}
int GameOptions::save()
{
    // (PSY) Make sure that cheatmode is switched off for ineligible profiles:
    if (options[Options::ACTIVE_PROFILE].str != SECRET_PROFILE)
		{
        (*this)[Options::CHEATMODE].number = 0;
				(*this)[Options::OPTIMIZE_HAND].number = 0;
				(*this)[Options::CHEATMODEAIDECK].number = 0;
		}

    std::ofstream file;
    if (JFileSystem::GetInstance()->openForWrite(file, mFilename))
    {
        for (int x = 0; x < (int) values.size(); x++)
        {
            //Check that this is a valid option.
            string name = Options::getName(x);
            GameOption * opt = get(x);
            if (!name.size() || !opt)
                continue;

            //Save it.
            opt->write(&file, name);
        }

        for (map<string, GameOption *>::iterator it = unknownMap.begin(); it != unknownMap.end(); it++)
        {
            if (it->second)
            {
                if (it->second->str.size())
                    file << it->first << "=" <<  it->second->str << "\n";
                else if (it->second->number)
                    file << it->first << "=" <<  it->second->number << "\n";
            }
        }
        file.close();
    }
    return 1;
}


GameOption& GameOptions::operator[](int optionID)
{
    GameOption * go = get(optionID);
    if (!go)
        return GameSettings::invalid_option;

    return *go;
}

GameOption& GameOptions::operator[](string optionName)
{
    int id = Options::getID(optionName);
    if (id != INVALID_OPTION)
        return operator[](id);

    GameOption * go =  get(optionName);

    return * go;

}

GameOption * GameOptions::factorNewGameOption(string optionName, string value)
{
    GameOption * result =( Unlockable::unlockables.find(optionName) !=  Unlockable::unlockables.end())
        ? NEW GameOptionAward()
        : NEW GameOption();

    if (value.size())
        result->read(value);

    return result;
}

GameOption * GameOptions::get(string optionName)
{
   if (!unknownMap[optionName])
   {
       unknownMap[optionName] = factorNewGameOption(optionName);
   }
   return unknownMap[optionName];
}

GameOption * GameOptions::get(int optionID)
{
    //Invalid options!
    if (optionID < 0)
        return NULL;

    //Option doesn't exist, so build it
    int x = (int) values.size();
    values.reserve(optionID);

    while (x <= optionID)
    {
        GameOption * go = NULL;
        GameOptionEnum * goEnum = NULL;
        switch (x)
        {
        //Enum options
        case Options::HANDDIRECTION:
            goEnum = NEW GameOptionEnum();
            goEnum->def = OptionHandDirection::getInstance();
            go = goEnum;
            break;
        case Options::CLOSEDHAND:
            goEnum = NEW GameOptionEnum();
            goEnum->def = OptionClosedHand::getInstance();
            go = goEnum;
            break;
        case Options::MANADISPLAY:
            goEnum = NEW GameOptionEnum();
            goEnum->def = OptionManaDisplay::getInstance();
            go = goEnum;
            break;
        case Options::MAX_GRADE:
            goEnum = NEW GameOptionEnum();
            goEnum->def = OptionMaxGrade::getInstance();
            go = goEnum;
            break;
        case Options::ASPHASES:
            goEnum = NEW GameOptionEnum();
            goEnum->def = OptionASkipPhase::getInstance();
            go = goEnum;
            break;
        case Options::FIRSTPLAYER:
            goEnum = NEW GameOptionEnum();
            goEnum->def = OptionWhosFirst::getInstance();
            go = goEnum;
            break;
        case Options::KICKERPAYMENT:
            goEnum = NEW GameOptionEnum();
            goEnum->def = OptionKicker::getInstance();
            go = goEnum;
            break;
        case Options::KEY_BINDINGS:
            go = NEW GameOptionKeyBindings();
            break;
        case Options::ECON_DIFFICULTY:
            goEnum = NEW GameOptionEnum();
            goEnum->def = OptionEconDifficulty::getInstance();
            go = goEnum;
            break;
        default:
            if (x >= Options::BEGIN_AWARDS)
                go = NEW GameOptionAward();
            else
                go = NEW GameOption();
            break;
        }
        values.push_back(go);
        x++;
    }

    return values[optionID];
}

GameOptions::~GameOptions()
{
    for (vector<GameOption*>::iterator it = values.begin(); it != values.end(); it++)
        SAFE_DELETE(*it);
    values.clear();

    for (map<string, GameOption*>::iterator it = unknownMap.begin(); it != unknownMap.end(); it++)
        SAFE_DELETE(it->second);
    unknownMap.clear();
}

// MARK:  - 

// MARK:  GameSettings

GameSettings options;

GameSettings::GameSettings()
{
    styleMan = NULL;
    globalOptions = NULL;
    theGame = NULL;
    profileOptions = NULL;
    //reloadProfile should be before using options.
}

WStyle * GameSettings::getStyle()
{
    if (!styleMan){
          styleMan = new StyleManager();
          styleMan->determineActive(NULL,NULL);
    }
    return styleMan->get();
}

StyleManager * GameSettings::getStyleMan()
{
    if (!styleMan)
        styleMan = new StyleManager();
    return styleMan;
}

void GameSettings::automaticStyle(Player * p1, Player * p2)
{
    if (!styleMan)
        styleMan = new StyleManager();
    MTGDeck * decks[2];
    for (int i = 0; i < 2; i++)
    {
        decks[i] = new MTGDeck(MTGCollection());
        Player * p;
        if (i == 0)
            p = p1;
        else
            p = p2;
        map<MTGCardInstance *, int>::iterator it;
        for (it = p->game->library->cardsMap.begin(); it != p->game->library->cardsMap.end(); it++)
        {
            decks[i]->add(it->first);
        }
    }
    styleMan->determineActive(decks[0], decks[1]);
    for (int i = 0; i < 2; i++)
    {
        SAFE_DELETE(decks[i]);
    }
}

GameSettings::~GameSettings()
{
    SAFE_DELETE(globalOptions);
    SAFE_DELETE(profileOptions);
    SAFE_DELETE(keypad);
    SAFE_DELETE(styleMan);
}

bool GameSettings::newAward()
{
    if (!profileOptions)
        return false;

    for (int x = Options::BEGIN_AWARDS; x < Options::SET_UNLOCKS + setlist.size(); x++)
    {
        GameOptionAward * goa = dynamic_cast<GameOptionAward *> (profileOptions->get(x));
        if (!goa)
            continue;
        if (!goa->isViewed())
            return true;
    }
    return false;
}

GameOption GameSettings::invalid_option = GameOption(0);

GameOption& GameSettings::operator[](int optionID)
{
    GameOption * go = get(optionID);
    if (!go)
        return invalid_option;

    return *go;
}

GameOption& GameSettings::operator[](string optionName)
{
    int id = Options::getID(optionName);
    if (id != INVALID_OPTION)
        return operator[](id);

    if (!profileOptions)
        return invalid_option;
    
    GameOption * go =  profileOptions->get(optionName);

    assert(go);

    return *go;
}

GameOption* GameSettings::get(int optionID)
{
    if (optionID < 0)
        return &invalid_option;
    else if (globalOptions && optionID <= Options::LAST_GLOBAL)
        return globalOptions->get(optionID);
    else if (profileOptions)
        return profileOptions->get(optionID);

    return &invalid_option;
}

void GameSettings::createProfileFolders()
{
     if (!profileOptions)
         return;

    string temp = profileFile("", "", false);
    JFileSystem::GetInstance()->MakeDir(temp);
    temp += "/stats";
    JFileSystem::GetInstance()->MakeDir(temp);
    temp = profileFile(PLAYER_SETTINGS, "", false);
        
    profileOptions->save();
}

int GameSettings::save()
{
    if (globalOptions)
        globalOptions->save();

    createProfileFolders();

    checkProfile();

    return 1;
}

string GameSettings::profileFile(string filename, string fallback, bool sanity)
{
    char buf[512];
    string profile = (*this)[Options::ACTIVE_PROFILE].str;

    if (!(*this)[Options::ACTIVE_PROFILE].isDefault())
    {
        //No file, return root of profile directory
        if (filename == "")
        {
            sprintf(buf, "profiles/%s", profile.c_str());
            return buf;
        }
        //Return file
        sprintf(buf, "profiles/%s/%s", profile.c_str(), filename.c_str());
        if (fileExists(buf))
        {
            return buf;
        }
    }
    else
    {
        //Use the default directory.
        sprintf(buf, "player%s%s", (filename == "" ? "" : "/"), filename.c_str());
        return buf;
    }

    //Don't fallback if sanity checking is disabled..
    if (!sanity)
    {
        sprintf(buf, "profiles/%s%s%s", profile.c_str(), (filename == "" ? "" : "/"), filename.c_str());
        return buf;
    }

    //No fallback directory. This is often a crash.
    if (fallback == "")
        return "";

    sprintf(buf, "%s%s%s", fallback.c_str(), (filename == "" ? "" : "/"), filename.c_str());
    return buf;
}

void GameSettings::reloadProfile()
{
    SAFE_DELETE(profileOptions);
    checkProfile();
}

void GameSettings::checkProfile()
{
    if (!globalOptions)
        globalOptions = NEW GameOptions(GLOBAL_SETTINGS);

    //If it doesn't exist, load current profile.
    if (!profileOptions)
    {
        profileOptions = NEW GameOptions(profileFile(PLAYER_SETTINGS, "", false));
        //Backwards compatibility hack for unlocked modes.
        for (int x = Options::BEGIN_AWARDS; x < Options::LAST_NAMED; x++)
        {
            GameOptionAward * goa = dynamic_cast<GameOptionAward *> (globalOptions->get(x));
            if (goa)
            {
                GameOptionAward * dupe = dynamic_cast<GameOptionAward *> (profileOptions->get(x));
                if (dupe && goa->number && !dupe->number)
                    dupe->giveAward();
            }
        }
    }

    //Validation of collection, etc, only happens if the game is up.
    if (theGame == NULL || MTGCollection() == NULL)
        return;

    string pcFile = profileFile(PLAYER_COLLECTION, "", false);
    if (!pcFile.size() || !fileExists(pcFile.c_str()))
    {
        //If we had any default settings, we'd set them here.

        //Create proper directories
        createProfileFolders();
    }

    //Find the set for which we have the most variety
    int setId = -1;
    int maxcards = 0;
    int ok = 0;
    for (int i = 0; i < setlist.size(); i++)
    {
        int value = MTGCollection()->countBySet(i);
        if (value > maxcards)
        {
            maxcards = value;
            setId = i;
        }
        if (options[Options::optionSet(i)].number)
        {
            ok = 1;
            break;
        }
    }
    if (!ok && setId >= 0)
    {
        //Save this set as "unlocked"
        (*profileOptions)[Options::optionSet(setId)] = 1;
        profileOptions->save();

        //Give the player their first deck
        createUsersFirstDeck(setId);
    }
    getStyleMan()->determineActive(NULL, NULL);
}

void GameSettings::createUsersFirstDeck(int setId)
{

    if (theGame == NULL || MTGCollection() == NULL)
        return;

    MTGDeck *mCollection = NEW MTGDeck(options.profileFile(PLAYER_COLLECTION, "", false).c_str(), MTGCollection());
    if (mCollection->totalCards() > 0)
        return;

    //10 lands of each
    int sets[] = { setId };
    if (!mCollection->addRandomCards(10, sets, 1, Constants::RARITY_L, "Forest"))
        mCollection->addRandomCards(10, 0, 0, Constants::RARITY_L, "Forest");
    if (!mCollection->addRandomCards(10, sets, 1, Constants::RARITY_L, "Plains"))
        mCollection->addRandomCards(10, 0, 0, Constants::RARITY_L, "Plains");
    if (!mCollection->addRandomCards(10, sets, 1, Constants::RARITY_L, "Swamp"))
        mCollection->addRandomCards(10, 0, 0, Constants::RARITY_L, "Swamp");
    if (!mCollection->addRandomCards(10, sets, 1, Constants::RARITY_L, "Mountain"))
        mCollection->addRandomCards(10, 0, 0, Constants::RARITY_L, "Mountain");
    if (!mCollection->addRandomCards(10, sets, 1, Constants::RARITY_L, "Island"))
        mCollection->addRandomCards(10, 0, 0, Constants::RARITY_L, "Island");

    //Starter Deck
    mCollection->addRandomCards(3, sets, 1, Constants::RARITY_R, NULL);
    mCollection->addRandomCards(9, sets, 1, Constants::RARITY_U, NULL);
    mCollection->addRandomCards(48, sets, 1, Constants::RARITY_C, NULL);

    //Boosters
    for (int i = 0; i < 2; i++)
    {
        mCollection->addRandomCards(1, sets, 1, Constants::RARITY_R);
        mCollection->addRandomCards(3, sets, 1, Constants::RARITY_U);
        mCollection->addRandomCards(11, sets, 1, Constants::RARITY_C);
    }
    mCollection->save();
    SAFE_DELETE(mCollection);
}

void GameSettings::keypadTitle(string set)
{
    if (keypad != NULL)
        keypad->title = set;
}

SimplePad * GameSettings::keypadStart(string input, string * _dest, bool _cancel, bool _numpad, float _x, float _y)
{
    if (keypad == NULL)
        keypad = NEW SimplePad();
    // show keyboard
#ifdef IOS
    JGE *engine = JGE::GetInstance();
    engine->SendCommand( "displayKeyboard", input);
#elif ANDROID
    JGE *engine = JGE::GetInstance();
    engine->SendCommand( "displayKeyboard:" << input);    
#endif
    keypad->bShowCancel = _cancel;
    keypad->bShowNumpad = _numpad;
    keypad->mX = _x;
    keypad->mY = _y;
    keypad->Start(input, _dest);
    return keypad;
}

string GameSettings::keypadFinish()
{
    if (keypad == NULL)
        return "";
    return keypad->Finish();
}

void GameSettings::keypadShutdown()
{
    SAFE_DELETE(keypad);
}

// MARK:  - 

// MARK:  EnumDefinition

//EnumDefinition
int EnumDefinition::findIndex(int value)
{
    vector<assoc>::iterator it;
    for (it = values.begin(); it != values.end(); it++)
    {
        if (it->first == value)
            return it - values.begin();
    }

    return INVALID_ID; //Failed!
}

//GameOptionEnum
string GameOptionEnum::menuStr()
{
    if (def)
    {
        int idx = def->findIndex(number);
        if (idx != INVALID_ID)
            return def->values[idx].second;
    }

    char buf[32];
    sprintf(buf, "%d", number);
    return buf;
}

bool GameOptionEnum::write(std::ofstream * file, string name)
{
    if (!file || !def || number <= 0 || number >= (int) def->values.size())
        return false;

    (*file) << name << "=" << menuStr() << endl;
    return true;
}

bool GameOptionEnum::read(string input)
{
    if (!def)
        return false;

    number = 0;
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);

    vector<EnumDefinition::assoc>::iterator it;
    for (it = def->values.begin(); it != def->values.end(); it++)
    {
        string v = it->second;
        std::transform(v.begin(), v.end(), v.begin(), ::tolower);
        if (v == input)
        {
            number = it->first;
            return true;
        }
    }

    return false;
}

// MARK:  - 

// MARK:  OptionMaxGrade

//Enum Definitions
OptionMaxGrade OptionMaxGrade::mDef;
OptionMaxGrade::OptionMaxGrade()
{
    mDef.values.push_back(EnumDefinition::assoc(Constants::GRADE_SUPPORTED, "1: 100% Supported"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::GRADE_BORDERLINE, "0: Borderline (99% OK)"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::GRADE_UNOFFICIAL, "-1: Unofficial (unverified cards)"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::GRADE_CRAPPY, "-2: Crappy (bugs)"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::GRADE_UNSUPPORTED, "-3: Unsupported"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::GRADE_DANGEROUS, "-4: Dangerous (risk of crash)"));

}
;
// MARK:  - 

// MARK:  OptionASkipPhase

OptionASkipPhase OptionASkipPhase::mDef;
OptionASkipPhase::OptionASkipPhase()
{
    mDef.values.push_back(EnumDefinition::assoc(Constants::ASKIP_NONE, "Off"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::ASKIP_SAFE, "Safe"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::ASKIP_FULL, "Full"));
}
;
// MARK:  - 

// MARK:  OptionWhosFirst

OptionWhosFirst OptionWhosFirst::mDef;
OptionWhosFirst::OptionWhosFirst()
{
    mDef.values.push_back(EnumDefinition::assoc(Constants::WHO_P, "Player"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::WHO_O, "Opponent"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::WHO_R, "Random"));
}
;

OptionClosedHand OptionClosedHand::mDef;
OptionClosedHand::OptionClosedHand()
{
    mDef.values.push_back(EnumDefinition::assoc(INVISIBLE, "invisible"));
    mDef.values.push_back(EnumDefinition::assoc(VISIBLE, "visible"));
}
;

OptionHandDirection OptionHandDirection::mDef;
OptionHandDirection::OptionHandDirection()
{
    mDef.values.push_back(EnumDefinition::assoc(VERTICAL, "vertical"));
    mDef.values.push_back(EnumDefinition::assoc(HORIZONTAL, "horizontal"));
}
;
OptionManaDisplay OptionManaDisplay::mDef;
OptionManaDisplay::OptionManaDisplay()
{
    mDef.values.push_back(EnumDefinition::assoc(DYNAMIC, "Eye candy"));
    mDef.values.push_back(EnumDefinition::assoc(STATIC, "Simple"));
	  mDef.values.push_back(EnumDefinition::assoc(NOSTARSDYNAMIC, "No Glitter"));
    mDef.values.push_back(EnumDefinition::assoc(BOTH, "Both"));//no luck in getting this to show up as an option.
		//Both should still work as always however the enum and this dont want to pair up, no "both" in options now.
}
;
OptionVolume OptionVolume::mDef;
OptionVolume::OptionVolume()
{
    mDef.values.push_back(EnumDefinition::assoc(MUTE, "Mute"));
    mDef.values.push_back(EnumDefinition::assoc(MAX, "Max"));
}
;
OptionDifficulty OptionDifficulty::mDef;
OptionDifficulty::OptionDifficulty()
{
    mDef.values.push_back(EnumDefinition::assoc(NORMAL, "Normal"));
    mDef.values.push_back(EnumDefinition::assoc(HARD, "Hard"));
    mDef.values.push_back(EnumDefinition::assoc(HARDER, "Harder"));
    mDef.values.push_back(EnumDefinition::assoc(EVIL, "Evil"));
}
;
OptionEconDifficulty OptionEconDifficulty::mDef;
OptionEconDifficulty::OptionEconDifficulty()
{
    mDef.values.push_back(EnumDefinition::assoc(Constants::ECON_NORMAL, "Normal"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::ECON_HARD, "Hard"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::ECON_LUCK, "Luck"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::ECON_EASY, "Easy"));
}
;
OptionKicker OptionKicker::mDef;
OptionKicker::OptionKicker()
{
    mDef.values.push_back(EnumDefinition::assoc(Constants::KICKER_ALWAYS, "Always Pay"));
    mDef.values.push_back(EnumDefinition::assoc(Constants::KICKER_CHOICE, "Offer Choice"));
}
;
// MARK:  - 

// MARK:  GameOptionAward

//GameOptionAward
GameOptionAward::GameOptionAward()
{
    achieved = time(NULL);
    number = 0;
    viewed = false;
}
bool GameOptionAward::read(string input)
{
    //This is quick and dirty.

    achieved = time(NULL);
    tm * at = localtime(&achieved);
    viewed = false;

    size_t inlen = input.size();
    if (!inlen)
        return true; //Default reader doesn't care about invalid formatting.
    else if (inlen < 8 || input != "0") //Regardless of what garbage this is fed, a non-zero value is "Awarded"
        number = 1;

    size_t w = input.find("V");

    if (w != string::npos)
        viewed = true;

    //TODO: Something cleaner.
    int tvals[5];
    int i;
    for (i = 0; i < 5; i++)
        tvals[i] = 0;

    string buf;
    for (size_t t = 0, i = 0; t < input.size(); t++)
    {
        if (!isdigit(input[t]))
        {
            if (!isspace(input[t]) && buf.size())
            {
                tvals[i] = atoi(buf.c_str());
                if (tvals[i] < 0)
                    tvals[i] = 0;
                buf.clear();
                i++; //Advance through input.
            }
        }
        else
            buf += input[t];

        if (i >= 5)
            break;
    }

    if (tvals[0] >= 1900)
        tvals[0] -= 1900;
    if (tvals[1] > 0)
        tvals[1]--;
    at->tm_year = tvals[0];
    at->tm_mon = tvals[1];
    at->tm_mday = tvals[2];
    if (tvals[3])
        at->tm_hour = tvals[3];
    if (tvals[4])
        at->tm_min = tvals[4];
    at->tm_isdst = -1;

    achieved = mktime(at);
    if (achieved == -1)
        achieved = time(NULL);
    return true;
}

bool GameOptionAward::write(std::ofstream * file, string name)
{
    char writer[1024];

    if (!file)
        return false;

    if (number == 0) //Is not unlocked. Don't write.
        return true;

    tm * at = localtime(&achieved);
    if (!at)
        return false; //Hurrah for paranoia.


    sprintf(writer, "%s=%d/%d/%d@%d:%d %s\n", name.c_str(), at->tm_year + 1900, at->tm_mon + 1, at->tm_mday, at->tm_hour,
                    at->tm_min, (viewed ? "V" : ""));
    (*file) << writer;
    return true;
}
bool GameOptionAward::giveAward()
{
    if (number)
        return false;

    achieved = time(NULL);
    viewed = false;
    number = 1;
    options.save(); //TODO - Consider efficiency of this placement.
    return true;
}

bool GameOptionAward::isViewed()
{
    if (!number)
        return true;
    return viewed;
}
;
string GameOptionAward::menuStr()
{
    if (!number)
        return _("Not unlocked.");
    else if (achieved == 1)
        return _("Unlocked.");

    char buf[256];

    tm * lt = localtime(&achieved);
    if (!lt)
        return "Error";
    strftime(buf, 255, _("%B %d, %I:%M%p %Y").c_str(), lt);
    return buf;
}

static JButton u32_to_button(u32 b)
{
    if (b < JGE_BTN_MAX)
        return static_cast<JButton> (b);
    else
        return JGE_BTN_NONE;
}

// MARK:  - 

// MARK:  GameOptionKeyBindings

bool GameOptionKeyBindings::read(string input)
{
    istringstream iss(input);
    vector<pair<LocalKeySym, JButton> > assoc;

    while (iss.good())
    {
        stringstream s;
        iss.get(*(s.rdbuf()), ',');
        iss.get();

        LocalKeySym local;
        char sep;
        u32 button;
        s >> local >> sep >> button;
        if (':' != sep)
            return false;
        assoc.push_back(make_pair(local, u32_to_button(button)));
    }

    if (assoc.empty())
        return false;

    JGE* j = JGE::GetInstance();

    j->ClearBindings();
    for (vector<pair<LocalKeySym, JButton> >::const_iterator it = assoc.begin(); it != assoc.end(); ++it)
        j->BindKey(it->first, it->second);

    return true;
}

bool GameOptionKeyBindings::write(std::ofstream* file, string name)
{
    JGE* j = JGE::GetInstance();
    *file << name << "=";
    JGE::keybindings_it start = j->KeyBindings_begin(), end = j->KeyBindings_end();
    if (start != end)
    {
        *file << start->first << ":" << start->second;
        ++start;
    }
    for (JGE::keybindings_it it = start; it != end; ++it)
        *file << "," << it->first << ":" << it->second;
    *file << endl;
    return true;
}
// MARK:  -