#ifndef _WGUI_H_
#define _WGUI_H_
/**
  @file WFilter.h
  Includes classes and functionality related to card filtering.
*/
#include <set>

class hgeDistortionMesh;
class GameStateOptions;

/**
  @defgroup WGui Basic Gui
  @{
*/

/**
  Color definition groups. Used to group text and background areas of similar purpose, 
  so that their color need only be defined once.
*/
class WGuiColor
{
public:
    enum
    {
        SCROLLBAR, SCROLLBUTTON,
        //Foregrounds only after this
        TEXT,
        TEXT_HEADER,
        TEXT_FAIL,
        TEXT_TAB,
        TEXT_BODY,
        //Backgrounds only after this
        BACK,
        BACK_ALERT,
        BACK_HEADER,
        BACK_FAIL,
        BACK_TAB,
    };
};

/**
  Quad distortion structure. Stores the modified x and y positions for all four corners of a quad.
*/
struct WDistort
{
    WDistort();
    WDistort(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
    float & operator[](int p);
protected:
    float xy[8];
};

/**
  Base class for all GUI item classes. 
*/
class WGuiBase: public JGuiListener
{
public:
    typedef enum
    {
        CONFIRM_NEED, ///< Still needs confirmation
        CONFIRM_OK,   ///< Is okay (no need to confirm, or has been confirmed)
        CONFIRM_CANCEL, ///< Is not okay, must cancel save
    } CONFIRM_TYPE;

    WGuiBase() {};
    virtual ~WGuiBase() {};

    /**
     If false, the option will be skipped over when moving the selection cursor.
    */
    virtual bool Selectable()
    {
        return true;
    }
    ;
    /** 
      If true, the item overrides the button handling of the WGuiMenu classes. See 
      WGuiMenu::CheckUserInput() for an example of modality implementation.
    */
    virtual bool isModal()
    {
        return false;
    }
    ;
    /**
      If false, the item will not render, and any lists will contract as if it weren't there. Meant to be 
      overridden in subclasses so that visibility is conditional on some function.
    */
    virtual bool Visible()
    {
        return true;
    }
    ;
    /**
      Returns true when the underlying data that this item represents has been changed by user interaction.
      This is used to help WDecoConfirm determine if a confirmation dialog is needed or not.
    */
    virtual bool Changed()
    {
        return false;
    }
    ;
    /**
      In cases where a WDecoConfirm dialog is used, it is meant to be overridden with an implementation
      that then applies the change to the underlying data. See the OptionProfile for an example.
      Note: This is ONLY called after the user presses "OK" on a WDecoConfirm dialog. See setData() 
      for the standard method of changing underlying data.
    */
    virtual void confirmChange(bool confirmed) {};
    /**
      Returns whether or not any changes to this item would require confirmation. Can also be used to 
      validate those changes, or to perform necessary cleanup when a change fails.
    */
    virtual CONFIRM_TYPE needsConfirm();
    virtual bool yieldFocus();
    virtual PIXEL_TYPE getColor(int type);
    virtual float getMargin(int type)
    {
        return 4;
    }
    ;

    /**
      What to do when the selection cursor enters the item. 
      @param key The key pressed to enter this item.
    */
    virtual void Entering(JButton key)=0;
    /**
      Request permission to leave the item. If the return value is false, the item remains selected.
      @param key The key pressed to leave this item.
    */
    virtual bool Leaving(JButton key)=0;

    virtual void Update(float dt)=0;
    /**
      Called when the item is selected and the OK button is pressed. Generally used to 
      change the visible notification of the selected item, but not to change the underlying data.
      For example, the OptionTheme class reacts to a button press by changing the selected theme,
      but does not actually apply the theme.
    */
    virtual void updateValue() {};
    virtual void Render()=0;
    /**
      Used to change the underlying data this gui element represents. 
    */
    virtual void setData()=0;

    virtual void ButtonPressed(int controllerId, int controlId) {};
    /**
      Used when it is necessary to update some information. Often called from confirmChange(), but also called
      in other places, such as to reload the list of possible profiles after a new one is created. See OptionProfile
      for details.
    */
    virtual void Reload() {};

    /**
      Render something after (and thus on top of) the regular Render() call.
    */
    virtual void Overlay() {};
    /**
      Render something before (and thus under) the regular Render() call.
    */
    virtual void Underlay() {};

    /**
      Returns true if the object currently has the focus.
    */
    virtual bool hasFocus()=0;
    /**
      Sets whether or not the object has the focus.
    */
    virtual void setFocus(bool bFocus)=0;
    virtual float getX()=0;
    virtual float getY()=0;
    virtual float getWidth()=0;
    virtual float getHeight()=0;
    virtual int getId()
    {
        return INVALID_ID;
    }
    ;
    virtual string getDisplay() const
    {
        return "";
    }
    ;
    virtual float minWidth()
    {
        return getWidth();
    }
    ;
    virtual float minHeight()
    {
        return getHeight();
    }
    ;

    /** Sets the modality of the item, if applicable. */
    virtual void setModal(bool val) {};
    virtual void setDisplay(string s) {};
    virtual void setX(float _x) {};
    virtual void setY(float _y) {};
    virtual void setWidth(float _w) {};
    virtual void setHeight(float _h) {};
    virtual void setId(int _id) {};
    virtual void setHidden(bool bHidden){};
    virtual void setVisible(bool bVisisble) {};
    virtual void renderBack(WGuiBase * it);
    virtual void subBack(WGuiBase * item) {};

    virtual bool CheckUserInput(JButton key)
    {
        return false;
    }
    ;
protected:
    vector<WGuiBase*> items;
};

/**
  Base class for all GUI concrete implementation classes. 
*/
class WGuiItem: public WGuiBase
{
public:
    virtual void Entering(JButton key);
    virtual bool Leaving(JButton key);
    virtual bool CheckUserInput(JButton key);
    virtual void Update(float dt) {};
    virtual void Render();

    WGuiItem(string _display, u8 _mF = 0);
    virtual ~WGuiItem() {};

    string _(string input); //Override global with our flag checker.

    virtual void setData() {};

    virtual bool hasFocus()
    {
        return mFocus;
    }
    ;
    virtual void setFocus(bool bFocus)
    {
        mFocus = bFocus;
    }
    ;

    virtual string getDisplay() const
    {
        return displayValue;
    }
    ;
    virtual void setDisplay(string s)
    {
        displayValue = s;
    }
    ;

    virtual int getId()
    {
        return INVALID_ID;
    }
    ;
    virtual float getX()
    {
        return x;
    }
    ;
    virtual float getY()
    {
        return y;
    }
    ;
    virtual float getWidth()
    {
        return width;
    }
    ;
    virtual float getHeight()
    {
        return height;
    }
    ;
    virtual float minWidth();
    virtual float minHeight();
    virtual void setId(int _id) {};
    virtual void setX(float _x)
    {
        x = _x;
    }
    ;
    virtual void setY(float _y)
    {
        y = _y;
    }
    ;
    virtual void setWidth(float _w)
    {
        width = _w;
    }
    ;
    virtual void setHeight(float _h)
    {
        height = _h;
    }
    ;
    enum
    {
        NO_TRANSLATE = (1 << 1),
    };

    u8 mFlags;

protected:
    bool mFocus;
    float x, y;
    float width, height;
    string displayValue;
};

/**
  An image drawn from the current position in a WDataSource.
*/
class WGuiImage: public WGuiItem
{
public:
    WGuiImage(WDataSource * wds, float _w = 0, float _h = 0, int _margin = 0);
    virtual bool Selectable()
    {
        return false;
    }
    ;
    virtual void Render();
    virtual float getHeight();
    virtual void imageScale(float _w, float _h);
    virtual void setSource(WDataSource *s)
    {
        source = s;
    }
    ;
protected:
    int margin;
    float imgW, imgH;
    WDataSource * source;
};

/**
  A card image drawn from the current position in a WDataSource.
*/
class WGuiCardImage: public WGuiImage
{
public:
    WGuiCardImage(WDataSource * wds, bool _thumb = false);
    virtual void Render();
    WSyncable mOffset;
protected:
    bool bThumb;
};

/**
  A variation of the WGuiCardImage that is distorted.
*/
class WGuiCardDistort: public WGuiCardImage
{
public:
    WGuiCardDistort(WDataSource * wds, bool _thumb = false, WDataSource * _distort = NULL);
    ~WGuiCardDistort();
    virtual void Render();
    WDistort xy;
    /* we assume first xy is the top left of the distorted card */
    virtual float getX()
    {
        return xy[0];
    }
    ;
    virtual float getY()
    {
        return xy[1];
    }
    ;
protected:
    hgeDistortionMesh* mesh;
    WDataSource * distortSrc;
};

/**
  Base decorator class, wraps all WGuiBase functionality and forwards it to the decorated item.
*/
class WGuiDeco: public WGuiBase
{
public:
    WGuiDeco(WGuiBase* _it)
    {
        it = _it;
    }
    ;
    virtual ~WGuiDeco()
    {
        SAFE_DELETE(it);
    }
    ;

    virtual bool Selectable()
    {
        return it->Selectable();
    }
    ;
    virtual bool Visible()
    {
        return it->Visible();
    }
    ;
    virtual bool Changed()
    {
        return it->Changed();
    }
    ;
    virtual void confirmChange(bool confirmed)
    {
        it->confirmChange(confirmed);
    }
    ;
    virtual CONFIRM_TYPE needsConfirm()
    {
        return it->needsConfirm();
    }
    ;
    virtual bool yieldFocus()
    {
        return it->yieldFocus();
    }
    ;

    virtual void Entering(JButton key)
    {
        it->Entering(key);
    }
    ;
    virtual bool Leaving(JButton key)
    {
        return it->Leaving(key);
    }
    ;
    virtual void Update(float dt)
    {
        it->Update(dt);
    }
    ;
    virtual void updateValue()
    {
        it->updateValue();
    }
    ;
    virtual void Reload()
    {
        it->Reload();
    }
    ;
    virtual void Overlay()
    {
        it->Overlay();
    }
    ;
    virtual void Underlay()
    {
        it->Underlay();
    }
    ;
    virtual void Render()
    {
        it->Render();
    }
    ;
    virtual void setData()
    {
        it->setData();
    }
    ;

    virtual void ButtonPressed(int controllerId, int controlId)
    {
        it->ButtonPressed(controllerId, controlId);
    }
    ;

    virtual bool hasFocus()
    {
        return it->hasFocus();
    }
    ;
    virtual string getDisplay() const
    {
        return it->getDisplay();
    }
    ;
    virtual int getId()
    {
        return it->getId();
    }
    ;
    virtual float getX()
    {
        return it->getX();
    }
    ;
    virtual float getY()
    {
        return it->getY();
    }
    ;
    virtual float getWidth()
    {
        return it->getWidth();
    }
    ;
    virtual float getHeight()
    {
        return it->getHeight();
    }
    ;
    virtual PIXEL_TYPE getColor(int type)
    {
        return it->getColor(type);
    }
    ;
    WGuiBase * getDecorated()
    {
        return it;
    }
    ;

    virtual void setFocus(bool bFocus)
    {
        it->setFocus(bFocus);
    }
    ;
    virtual void setDisplay(string s)
    {
        it->setDisplay(s);
    }
    ;
    virtual void setId(int _id)
    {
        it->setId(_id);
    }
    ;
    virtual void setX(float _x)
    {
        it->setX(_x);
    }
    ;
    virtual void setY(float _y)
    {
        it->setY(_y);
    }
    ;
    virtual void setWidth(float _w)
    {
        it->setWidth(_w);
    }
    ;
    virtual void setHeight(float _h)
    {
        it->setHeight(_h);
    }
    ;
    virtual void setHidden(bool bHidden)
    {
        it->setHidden(bHidden);
    }
    ;
    virtual void setVisible(bool bVisisble)
    {
        it->setVisible(bVisisble);
    }
    ;
    virtual bool CheckUserInput(JButton key)
    {
        return it->CheckUserInput(key);
    }
    ;
protected:
    WGuiBase * it;
};

/**
  An item used to represent an award, can also function as a button for more details. 
  Visibility and selectability are dependent on whether or not the award has been unlocked.
*/
class WGuiAward: public WGuiItem
{
public:
    WGuiAward(int _id, string name, string _text, string _details = "");
    virtual ~WGuiAward();
    virtual void Render();
    virtual bool Selectable()
    {
        return Visible();
    }
    ;
    virtual bool Visible();
    virtual int getId()
    {
        return id;
    }
    ;
    virtual void Underlay();
    virtual void Overlay();

protected:
    string details;
    int id;
    string text;
};

/**
  When the decorated items are members of a WGuiList, causes them to render in two columns.
*/
class WGuiSplit: public WGuiItem
{
public:
    WGuiSplit(WGuiBase* _left, WGuiBase* _right);
    virtual ~WGuiSplit();

    virtual bool yieldFocus();
    virtual void Reload();
    virtual void Overlay();
    virtual void Underlay();
    virtual void setData();
    virtual bool isModal();
    virtual void setModal(bool val);
    virtual void Render();
    virtual void Update(float dt);
    virtual void setX(float _x);
    virtual void setY(float _y);
    virtual void setWidth(float _w);
    virtual void setHeight(float _h);
    virtual float getHeight();
    virtual void ButtonPressed(int controllerId, int controlId);
    virtual void confirmChange(bool confirmed);

    virtual void Entering(JButton key);
    virtual bool Leaving(JButton key);
    virtual bool CheckUserInput(JButton key);

    bool bRight;
    float percentRight;
    WGuiBase* right;
    WGuiBase* left;
};


/**
  Causes a confirmation dialog to pop up when the decorated item is changed. 
*/
class WDecoConfirm: public WGuiDeco
{
public:
    WDecoConfirm(JGuiListener * _listener, WGuiBase * it);
    virtual ~WDecoConfirm();

    virtual bool isModal();
    virtual void setData();
    virtual void setModal(bool val);
    virtual void Entering(JButton key);
    virtual bool Leaving(JButton key);
    virtual void Update(float dt);
    virtual void Overlay();
    virtual void ButtonPressed(int controllerId, int controlId);
    virtual bool CheckUserInput(JButton key);

    string confirm;
    string cancel;
protected:
    enum
    {
        OP_UNCONFIRMED,
        OP_CONFIRMING,
        OP_CONFIRMED,
    } mState;

    SimpleMenu * confirmMenu;
    JGuiListener * listener;
    bool bModal;
};

/**
  Decorator for numeric values, transforms them into meaningful strings. Used by options 
  that have an enum or similar as their underlying representation. Requires an EnumDefinition 
  so it knows what value maps to what string.
*/
class WDecoEnum: public WGuiDeco
{
public:
    WDecoEnum(WGuiBase * _it, EnumDefinition *_edef = NULL);
    virtual void Render();
    string lookupVal(int value);
protected:
    EnumDefinition * edef;
};

/**
  Makes the decorated item's visibility contingent on the player cheating. 
*/
class WDecoCheat: public WGuiDeco
{
public:
    WDecoCheat(WGuiBase * _it);
    virtual bool Visible();
    bool Selectable();
    virtual void Reload();
protected:
    bool bVisible;
};

/**
  Allows the decorated item to send a button notification to a JGuiListener.
*/
class WGuiButton: public WGuiDeco
{
public:
    WGuiButton(WGuiBase* _it, int _controller, int _control, JGuiListener * jgl);
    virtual void updateValue();
    virtual bool CheckUserInput(JButton key);
    virtual bool Selectable()
    {
        return Visible();
    }
    ;
    virtual PIXEL_TYPE getColor(int type);
    virtual int getControlID()
    {
        return control;
    }
    ;
    virtual int getControllerID()
    {
        return controller;
    }
    ;
protected:
    int control, controller;
    JGuiListener * mListener;
};

/**
  Similar to an HTML heading, this displays text without any other functionality.
*/
class WGuiHeader: public WGuiItem
{
public:
    WGuiHeader(string _displayValue) : WGuiItem(_displayValue) {};
    virtual bool Selectable()
    {
        return false;
    }
    ;
    virtual void Render();
};

/**
  Allows the application of a certain color style in the decorated class.
*/
class WDecoStyled: public WGuiDeco
{
public:
    WDecoStyled(WGuiItem * _it) :
        WGuiDeco(_it)
    {
        mStyle = DS_DEFAULT;
    }
    ;
    PIXEL_TYPE getColor(int type);
    void subBack(WGuiBase * item);
    enum
    {
        DS_DEFAULT = (1 << 0), DS_COLOR_BRIGHT = (1 << 1), DS_COLOR_DARK = (1 << 2), DS_STYLE_ALERT = (1 << 3),
                    DS_STYLE_EDGED = (1 << 4), DS_STYLE_BACKLESS = (1 << 5),
    };

    u8 mStyle;
};

/**
  Base class for menu GUIs. Provides useful functionality: basic 1-dimensional item selection 
  and the ability to sync the current index of a WSyncable with the menu's selected index 
  (to allow the menu to easily iterate through arrays of data). Items are rendered at the 
  X/Y position of the individual WGuiItem itself.
*/
class WGuiMenu: public WGuiItem
{
public:
    friend class WGuiFilters;
    virtual ~WGuiMenu();
    WGuiMenu(JButton next, JButton prev, bool mDPad = false, WSyncable * syncme = NULL);

    virtual bool yieldFocus();
    virtual void Render();
    virtual void Reload();
    virtual void Update(float dt);
    virtual void ButtonPressed(int controllerId, int controlId);
    virtual void Add(WGuiBase* item); //Remember, does not set X & Y of items automatically.
    virtual void confirmChange(bool confirmed);
    virtual bool Leaving(JButton key);
    virtual void Entering(JButton key);
    virtual void subBack(WGuiBase * item);
    virtual bool CheckUserInput(JButton key);
    WGuiBase * Current();
    virtual int getSelected()
    {
        return currentItem;
    }
    ;
    virtual void setSelected(vector<WGuiBase*>::iterator& it)
    {
        int c = it - items.begin();
        setSelected(c);
    }
    ;
    virtual void setSelected(int newItem);
    virtual bool nextItem();
    virtual bool prevItem();
    virtual bool isModal();
    virtual void setModal(bool val);

    void setData();

protected:
    virtual void syncMove();
    virtual bool isButtonDir(JButton key, int dir); //For the DPad override.
    JButton buttonNext, buttonPrev;
    bool mDPad;
    int currentItem;
    JButton held;
    WSyncable * sync;
    float duration;
};

/**
  Creates a vertically scrolling menu of items. Items are rendered based on their 
  position within the items vector, and disregard the X/Y position of the individual
  WGuiItem itself.
*/
class WGuiList: public WGuiMenu
{
public:
    WGuiList(string name, WSyncable * syncme = NULL);

    string failMsg;

    virtual void Render();
    virtual void confirmChange(bool confirmed);
    virtual void ButtonPressed(int controllerId, int controlId);
    virtual void setData();
    WGuiBase * operator[](int);
protected:
    bool mFocus;
};

/**
  Creates a horizontal menu of items, each of which is usually a WGuiMenu derivative 
  themselves. Items are rendered based on their position within the items vector, and 
  disregard the X/Y position of the individual WGuiItem itself.
*/
class WGuiTabMenu: public WGuiMenu
{
public:
    WGuiTabMenu() : WGuiMenu(JGE_BTN_NEXT, JGE_BTN_PREV) {};
    virtual void Render();
    virtual void Add(WGuiBase * it);
    void save();
    virtual bool CheckUserInput(JButton key);
};
/**
  A variant of WGuiList that renders as a horizontal row, rather than vertically.
*/
class WGuiListRow: public WGuiList
{
public:
    WGuiListRow(string n, WSyncable * s = NULL);
    virtual void Render();
};

/**
  The filter building interface. 
*/
class WGuiFilters: public WGuiItem
{
public:
    friend class WGuiFilterItem;
    WGuiFilters(string header, WSrcCards * src);
    ~WGuiFilters();
    bool CheckUserInput(JButton key);
    string getCode(); //For use in filter factory.
    void Update(float dt);
    void Render();
    void Entering(JButton key);
    void addColumn();
    void recolorFilter(int color);
    bool isAvailable(int type);
    bool isAvailableCode(string code);
    bool Finish(bool emptyset = false); //Returns true if card set reasonably expected to be changed.
    bool isFinished()
    {
        return bFinished;
    }
    ;
    void ButtonPressed(int controllerId, int controlId);
    void buildList();
    void setSrc(WSrcCards * wsc);
protected:
    void clearArgs();
    void addArg(string display, string code);
    vector<pair<string, string> > tempArgs; //TODO FIXME this is inefficient
    bool bFinished;
    int recolorTo;
    WSrcCards* source;
    SimpleMenu* subMenu;
    WGuiList * list;
};

/**
  An interface item representing a single component of a filter.
*/
class WGuiFilterItem: public WGuiItem
{
public:
    friend class WGuiFilters;
    WGuiFilterItem(WGuiFilters * parent);
    void updateValue();
    void ButtonPressed(int controllerId, int controlId);
    string getCode();
    bool isModal();
    enum
    {
        STATE_UNSET,
        STATE_CHOOSE_TYPE,
        STATE_CHOOSE_VAL,
        STATE_FINISHED,
        STATE_REMOVE,
        STATE_CANCEL,
        BEGIN_FILTERS = 0,
        FILTER_SET = BEGIN_FILTERS,
        FILTER_ALPHA,
        FILTER_RARITY,
        FILTER_COLOR,
        FILTER_PRODUCE,
        FILTER_TYPE,
        FILTER_SUBTYPE,
        FILTER_BASIC,
        FILTER_CMC,
        FILTER_POWER,
        FILTER_TOUGH,
        END_FILTERS
    };
protected:
    string mCode;
    int filterType;
    int filterVal;
    int mState;
    bool mNew;
    WGuiFilters * mParent;
};

/**
  Used by the options menu for keybindings. This WGuiList derivative automatically populates itself 
  with OptionKey items representing all the potential interaction bindings.
*/
class WGuiKeyBinder: public WGuiList
{
public:
    WGuiKeyBinder(string name, GameStateOptions* parent);
    virtual bool isModal();
    virtual bool CheckUserInput(JButton);
    virtual void setData();
    virtual void Update(float);
    virtual void Render();
    virtual CONFIRM_TYPE needsConfirm();
    virtual void ButtonPressed(int controllerId, int controlId);
    virtual bool yieldFocus();
protected:
    GameStateOptions* parent;
    SimpleMenu* confirmMenu;
    bool modal;
    CONFIRM_TYPE confirmed;
    LocalKeySym confirmingKey;
    JButton confirmingButton;
    set<LocalKeySym> confirmedKeys;
    set<JButton> confirmedButtons;
    string confirmationString;
};

/**@} This comment used by Doxyyen. */
#endif
