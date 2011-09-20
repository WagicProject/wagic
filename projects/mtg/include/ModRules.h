/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

/*
ModRules class describes global game rules used for a given Wagic Mod.
These rules describe some high level Game rules,
some graphical effects, what parts of the game are made accessible to the player, etc...
They are accessed through the global variable gModRules, and loaded from rules/modrules.xml
*/

#ifndef _MODRULES_H_
#define _MODRULES_H_

#include <string>
#include <vector>
using namespace std;

#include "CardGui.h"

class TiXmlElement;


enum
{
    SUBMENUITEM_CANCEL = kCancelMenuID,
    MENUITEM_PLAY,
    MENUITEM_DECKEDITOR,
    MENUITEM_SHOP,
    MENUITEM_OPTIONS,
    MENUITEM_EXIT,
    MENUITEM_TROPHIES,
    SUBMENUITEM_1PLAYER,
#ifdef NETWORK_SUPPORT
    SUBMENUITEM_2PLAYERS,
    SUBMENUITEM_HOST_GAME,
    SUBMENUITEM_JOIN_GAME,
#endif //NETWORK_SUPPORT
    SUBMENUITEM_DEMO,
    SUBMENUITEM_TESTSUITE,
    SUBMENUITEM_TESTAI,
    SUBMENUITEM_END_OFFSET
};

class ModRulesMenuItem
{
protected:
    static int strToAction(string str);
public:
    int mActionId;
    string mDisplayName;
    ModRulesMenuItem(string actionIdStr, string displayName);
    //most actionIds are associated to a game state. e.g. MENUITEM_DECKEDITOR <--> GAME_STATE_DECK_VIEWER
    //This function returns the game state that matches the actionId, if any
    int getMatchingGameState();
    static int getMatchingGameState(int actionId);
};

class ModRulesMainMenuItem: public ModRulesMenuItem
{
public:
    int mIconId;
    string mParticleFile;
    ModRulesMainMenuItem(string actionIdStr, string displayName, int iconId, string particleFile);
};

class ModRulesOtherMenuItem: public ModRulesMenuItem
{
public:
    JButton mKey;
    ModRulesOtherMenuItem(string actionIdStr, string displayName, string keyStr);
    static JButton strToJButton(string keyStr);
};

class ModRulesMenu
{
public:
    vector<ModRulesMainMenuItem *> main;
    vector<ModRulesOtherMenuItem *> other;

    void parse(TiXmlElement* element);
    ~ModRulesMenu();
};

class ModRulesGame
{
public:
    bool mCanInterrupt;
public:
    bool canInterrupt() {return mCanInterrupt;};
    ModRulesGame();
    void parse(TiXmlElement* element);
};

class ModRulesGeneral
{
protected:
    bool mHasDeckEditor;
    bool mHasShop;
public:
    bool hasDeckEditor() {return mHasDeckEditor;};
    bool hasShop() {return mHasShop;};
    ModRulesGeneral();
    void parse(TiXmlElement* element);
};

class ModRulesCards
{
public:
    SimpleCardEffect * activateEffect;
    static SimpleCardEffect * parseEffect(string str);
    ModRulesCards();
    ~ModRulesCards();
    void parse(TiXmlElement* element);
};

class ModRules
{
public:
    ModRulesGeneral general;
    ModRulesCards cards;
    ModRulesMenu menu;
    ModRulesGame game;

    bool load(string filename);
    static int getValueAsInt(TiXmlElement* element, string childName);

};

extern ModRules gModRules;


#endif
