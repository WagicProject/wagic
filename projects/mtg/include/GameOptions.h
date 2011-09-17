#ifndef _GAME_OPTIONS_H_
#define _GAME_OPTIONS_H_

#include <map>
#include <string>
using std::map;
using std::string;
#include <JGE.h>
#include <time.h>
#include "SimplePad.h"

#define GLOBAL_SETTINGS "settings/options.txt"
#define PLAYER_SAVEFILE "data.dat"
#define PLAYER_SETTINGS "options.txt"
#define PLAYER_COLLECTION "collection.dat"
#define PLAYER_TASKS "tasks.dat"
#define SECRET_PROFILE "Maxglee"

#define INVALID_OPTION -1

class WStyle;
class StyleManager;
class Player;
class GameApp;


class Options
{
public:
    friend class GameSettings;
    enum 
    {
        //Global settings
        ACTIVE_PROFILE,
        LANG,
        LAST_GLOBAL = LANG, //This must be the value above, to keep ordering.
        //Values /must/ match ordering in optionNames, or everything loads wrong.
        //Profile settings        
        ACTIVE_THEME,
        ACTIVE_MODE,
        MUSICVOLUME,
        SFXVOLUME,
        DIFFICULTY,
        CHEATMODE,
        OPTIMIZE_HAND,
        CHEATMODEAIDECK,
        OSD,
        CLOSEDHAND,
        HANDDIRECTION,
        MANADISPLAY,
        REVERSETRIGGERS,
        DISABLECARDS,
        MAX_GRADE,
        ASPHASES,
        FIRSTPLAYER,
        KICKERPAYMENT,
        ECON_DIFFICULTY,
        TRANSITIONS,
        GUI_STYLE,
        INTERRUPT_SECONDS,
        KEY_BINDINGS,
        AIDECKS_UNLOCKED,
        //My interrupts    
        INTERRUPTMYSPELLS,
        INTERRUPTMYABILITIES,
        SAVEDETAILEDDECKINFO,
        //Other interrupts
        INTERRUPT_BEFOREBEGIN,
        INTERRUPT_UNTAP,
        INTERRUPT_UPKEEP,
        INTERRUPT_DRAW,
        INTERRUPT_FIRSTMAIN,
        INTERRUPT_BEGINCOMBAT,
        INTERRUPT_ATTACKERS,
        INTERRUPT_BLOCKERS,
        INTERRUPT_DAMAGE,
        INTERRUPT_ENDCOMBAT,
        INTERRUPT_SECONDMAIN,
        INTERRUPT_ENDTURN,
        INTERRUPT_CLEANUP,
        INTERRUPT_AFTEREND,
        BEGIN_AWARDS, //Options after this use the GameOptionAward struct, which includes a timestamp.
        DIFFICULTY_MODE_UNLOCKED = BEGIN_AWARDS,
        MOMIR_MODE_UNLOCKED,
		STONEHEWER_MODE_UNLOCKED,
		HERMIT_MODE_UNLOCKED,
        EVILTWIN_MODE_UNLOCKED,
        RANDOMDECK_MODE_UNLOCKED,    
        AWARD_COLLECTOR,
        LAST_NAMED, //Any option after this does not look up in optionNames.
        SET_UNLOCKS = LAST_NAMED + 1, //For sets.
    };

    static int optionSet(int setID);
    static int optionInterrupt(int gamePhase);

    static int getID(string name);
    static string getName(int option);

private:
    static const string optionNames[];
};

class GameOption
{
public:
    virtual ~GameOption()
    {
    }

    int number;
    string str;
    //All calls to asColor should include a fallback color for people without a theme.
    PIXEL_TYPE asColor(PIXEL_TYPE fallback = ARGB(255,255,255,255));

    virtual bool isDefault(); //Returns true when number is 0 and string is "" or "Default"
    virtual string menuStr(); //The string we'll use for GameStateOptions.
    virtual bool write(std::ofstream * file, string name);
    virtual bool read(string input);

    GameOption(int value = 0);
    GameOption(string);
    GameOption(int, string);
};

struct EnumDefinition
{
    int findIndex(int value);

    typedef pair<int, string> assoc;
    vector<assoc> values;
};

class GameOptionEnum : public GameOption
{
public:
    virtual string menuStr();
    virtual bool write(std::ofstream * file, string name);
    virtual bool read(string input);
    EnumDefinition * def;
};

class GameOptionAward : public GameOption
{
public:
    GameOptionAward();
    virtual string menuStr();
    virtual bool write(std::ofstream * file, string name);
    virtual bool read(string input);
    virtual bool giveAward(); //Returns false if already awarded
    virtual bool isViewed();

    virtual void setViewed(bool v = true)
    {
        viewed = v;
    }

private:
    time_t achieved; //When was it awarded?
    bool viewed;     //Flag it as "New!" or not.  
};

class GameOptionKeyBindings : public GameOption
{
    virtual bool read(string input);
    virtual bool write(std::ofstream*, string);
};

class OptionVolume : public EnumDefinition
{
public:
    enum
    {
        MUTE = 0, 
        MAX = 100 
    };
    
    static EnumDefinition * getInstance()
    {
        return &mDef;
    }

private:
    OptionVolume();
    static OptionVolume mDef;
};


class OptionClosedHand : public EnumDefinition
{
public:
    enum
    { 
        INVISIBLE = 0,
        VISIBLE = 1
    };

    static EnumDefinition* getInstance()
    {
        return &mDef;
    }

private:  
    OptionClosedHand();
    static OptionClosedHand mDef;
};

class OptionHandDirection : public EnumDefinition
{
public:
    enum
    {
        VERTICAL = 0,
        HORIZONTAL = 1
    };

    static EnumDefinition * getInstance()
    {
        return &mDef;
    }

private:
    OptionHandDirection();
    static OptionHandDirection mDef;
};

class OptionManaDisplay : public EnumDefinition
{
public:
    enum
    {
        DYNAMIC = 0,
        STATIC = 1,
        NOSTARSDYNAMIC = 2,
        BOTH = 3
    };
    
    static EnumDefinition * getInstance()
    {
        return &mDef;
    }

private:
    OptionManaDisplay();
    static OptionManaDisplay mDef;
};

class OptionMaxGrade : public EnumDefinition
{
public:
    static EnumDefinition * getInstance()
    {
        return &mDef;
    }

private:
    OptionMaxGrade();
    static OptionMaxGrade mDef;
};

class OptionASkipPhase : public EnumDefinition
{
public:
    static EnumDefinition * getInstance()
    {
        return &mDef;
    }

private:
    OptionASkipPhase();
    static OptionASkipPhase mDef;
};

class OptionWhosFirst : public EnumDefinition
{
public:
    enum
    {
        WHO_P = 0,
        WHO_O = 1, 
        WHO_R = 2
    };
    
    static EnumDefinition * getInstance()
    {
        return &mDef;
    }

private:
    OptionWhosFirst();
    static OptionWhosFirst mDef;
};

class OptionKicker : public EnumDefinition
{
public:
    enum
    {
        KICKER_ALWAYS = 0,
        KICKER_CHOICE = 1, 
    };
    
    static EnumDefinition * getInstance()
    {
        return &mDef;
    }

private:
    OptionKicker();
    static OptionKicker mDef;
};

class OptionEconDifficulty : public EnumDefinition
{
public:
    static EnumDefinition * getInstance()
    {
        return &mDef;
    }

private:
    OptionEconDifficulty();
    static OptionEconDifficulty mDef;
};

class OptionDifficulty: public EnumDefinition
{
public:
    enum
    { 
        NORMAL = 0,
        HARD = 1,
        HARDER = 2,
        EVIL = 3
    };

    static EnumDefinition* getInstance()
    {
        return &mDef;
    }

private:
    OptionDifficulty();
    static OptionDifficulty mDef;
};

class GameOptions
{
public:
    string mFilename;
    int save();
    int load();

    GameOption * get(int);
    GameOption * get(string optionName);
    GameOption& operator[](int);
    GameOption& operator[](string);
    GameOptions(string filename);
    ~GameOptions();

private:
    vector<GameOption*> values;
    map<string,GameOption*> unknownMap;
};

class GameSettings
{
public:
    friend class GameApp;

    GameSettings();
    ~GameSettings();
    int save();

    SimplePad * keypadStart(string input, string * _dest = NULL, bool _cancel = true, bool _numpad = false, float _x = SCREEN_WIDTH_F / 2, float _y = SCREEN_HEIGHT_F / 2);
    string keypadFinish();
    void keypadShutdown();
    void keypadTitle(string set);
    
    bool keypadActive()
    {
        if(keypad)
            return keypad->isActive();
        
        return false;
    }
    
    void keypadUpdate(float dt)
    {
        if(keypad)
            keypad->Update(dt);
    }
    
    void keypadRender() 
    {
        if(keypad)
            keypad->Render();
    }

    bool newAward();

    //These return a filepath accurate to the current mode/profile/theme, and can
    //optionally fallback to a file within a certain directory. 
    //The sanity=false option returns the adjusted path even if the file doesn't exist.
    string profileFile(string filename="", string fallback="", bool sanity=false);

    void reloadProfile(); //Reloads profile using current options[ACTIVE_PROFILE]
    void checkProfile();  //Confirms that a profile is loaded and contains a collection.
    void createUsersFirstDeck(int setId); 

    GameOption* get(int);
    GameOption& operator[](int);
    GameOption& operator[](string);

    GameOptions* profileOptions;
    GameOptions* globalOptions;

    static GameOption invalid_option;

    WStyle * getStyle();
    StyleManager * getStyleMan();
    void automaticStyle(Player* p1, Player* p2);

private:
    GameApp* theGame;  
    SimplePad* keypad;
    StyleManager* styleMan;
    void createProfileFolders();
};

extern GameSettings options;

#endif
