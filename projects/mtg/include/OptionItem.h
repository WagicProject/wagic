#ifndef _OPTION_ITEM_H_
#define _OPTION_ITEM_H_
/**
  @file OptionItem.h
  Includes classes and functionality related to the options menu.
*/
#include <JGui.h>
#include <vector>
#include <string>
#include "GameApp.h"
#include "GameStateOptions.h"
#include "GameOptions.h"
#include "WFilter.h"
#include "WDataSrc.h"
#include "WGui.h"

using std::string;

#define MAX_OPTION_TABS 5
#define MAX_ONSCREEN_OPTIONS 8
#define OPTION_CENTER 4
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
  @defgroup WGuiOptions Options Gui
  @ingroup WGui
  @{
*/

/**
  The base class for all option menu items.
*/
class OptionItem: public WGuiItem
{
public:
    OptionItem(int _id, string _displayValue);
    virtual ~OptionItem() {};

    /**
      Returns the index into ::options used to store and retrieve this option.
    */    
    virtual int getId()
    {
        return id;
    }

    /**
      Changes the index into ::options used to store and retrieve this option.
    */
    virtual void setId(int _id)
    {
        id = _id;
    }

protected:
    int id;
};

/**
  A numeric option item. Can be decorated with WDecoEnum to provide a string representation of the numeric values.
*/
class OptionInteger: public OptionItem
{
public:
    int value; ///< Current value the option is displaying.
    int defValue; ///< Default value for the option.
    string strDefault; ///< What to call the default value in the menu.
    int maxValue; ///< Maximum value of the option.
    int minValue; ///< Minimum value of the option.
    int increment; ///< Amount to increment the option by when clicked.

    OptionInteger(int _id, string _displayValue, int _maxValue = 1, int _increment = 1, int _defV = 0, string _sDef = "", int _minValue = 0);

    virtual void Reload()
    {
        if (id != INVALID_OPTION)
            value = options[id].number;
    }

    virtual bool Changed()
    {
        return value != options[id].number;
    }

    virtual void Render();
    virtual void setData();
    virtual void updateValue()
    {
        value += increment;
        if (value > maxValue)
            value = minValue;
    }

};

/**
  An option represented as one of a set of strings. 
*/
class OptionSelect: public OptionItem
{
public:
    size_t value; ///< Currently selected option, an index into selections.
    vector<string> selections; ///< Vector containing all possible values.

    virtual void addSelection(string s);
    OptionSelect(int _id, string _displayValue) :
        OptionItem(_id, _displayValue)
    {
        value = 0;
    }
    ;
    virtual void Reload()
    {
        initSelections();
    }
    ;
    virtual void Render();
    virtual bool Selectable();
    virtual void Entering(JButton key);
    virtual bool Changed()
    {
        return (value != prior_value);
    }

    virtual void setData();
    virtual void initSelections();
    virtual void updateValue()
    {
        value++;
        if (value > selections.size() - 1)
            value = 0;
    }
    ;
protected:
    size_t prior_value; ///< The prior selected value, in case a change is cancelled.
};

/**
  An option representing possible languages. Automatically loads the list of possibilities from the lang/ folder.
*/
class OptionLanguage: public OptionSelect
{
public:
    OptionLanguage(string _displayValue);

    virtual void addSelection(string s)
    {
        addSelection(s, s);
    }
    ;
    virtual void addSelection(string s, string show);
    virtual void initSelections();
    virtual void confirmChange(bool confirmed);
    virtual void Reload();
    virtual bool Visible();
    virtual bool Selectable();
    virtual void setData();
protected:
    vector<string> actual_data; ///< An array containing the actual value we set the option to, rather than the display value in selections.
};

/**
  An option representing possible theme substyles. Automatically loads the list of possibilities from the current theme.
*/
class OptionThemeStyle: public OptionSelect
{
public:
    virtual bool Visible();
    virtual void Reload();
    virtual void confirmChange(bool confirmed);
    OptionThemeStyle(string _displayValue);
};

/**
  An option allowing the user to choose a directory, provided it contains a certain file.
*/
class OptionDirectory: public OptionSelect
{
public:
    virtual void Reload();
    OptionDirectory(string root, int id, string displayValue, const string type);
protected:
    const string root; ///< The root directory to search for subdirectories.
    const string type; ///< The file to check for in a useable subdirectory.
};
/**
  An option allowing the player to choose a theme directory. Requires that the theme directory contains a preview.png.
*/
class OptionTheme: public OptionDirectory
{
private:
    static const string DIRTESTER; ///< A particular file to look for when building the list of possible directories.
public:
    OptionTheme(OptionThemeStyle * style = NULL);
    JQuadPtr getImage();
    virtual void updateValue();
    virtual float getHeight();
    virtual void Render();
    virtual void confirmChange(bool confirmed);
    virtual bool Visible();

protected:
    OptionThemeStyle * ts; ///< The current theme style.
    string author;  ///< The theme author
    bool bChecked;  ///< Whether or not the theme has been checked for metadata
};

/**
  An option allowing the player to choose a profile directory. Requires that the profile directory contains a collection.dat.
*/
class OptionProfile: public OptionDirectory
{
private:
    static const string DIRTESTER; ///< A particular file to look for when building the list of possible directories.
public:
    OptionProfile(GameApp * _app, JGuiListener * jgl);
    virtual void addSelection(string s);
    virtual bool Selectable()
    {
        return canSelect;
    }
    ;
    virtual bool Changed()
    {
        return (initialValue != value);
    }
    ;
    virtual void Entering(JButton key);
    virtual void Reload();
    virtual void Render();
    virtual void initSelections();
    virtual void confirmChange(bool confirmed);
    virtual void updateValue();
    void populate();
private:
    GameApp * app;
    JGuiListener * listener;
    bool canSelect;
    string preview;
    size_t initialValue;
};

/**
  An option allowing the player to bind a key to a specific interaction.
*/
class OptionKey: public WGuiItem, public KeybGrabber
{
public:
    OptionKey(GameStateOptions* g, LocalKeySym, JButton);
    LocalKeySym from;
    JButton to;
    virtual void Render();
    virtual void Update(float);
    virtual void Overlay();
    virtual bool CheckUserInput(JButton key);
    virtual void KeyPressed(LocalKeySym key);
    virtual bool isModal();
    virtual void ButtonPressed(int controllerId, int controlId);
    virtual bool Visible();
    virtual bool Selectable();
protected:
    bool grabbed;
    GameStateOptions* g;
    SimpleMenu* btnMenu;
};

/**@} This comment used by Doxyyen. */
#endif
