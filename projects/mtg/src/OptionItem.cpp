#include "PrecompiledHeader.h"

#include "OptionItem.h"
#include "PlayerData.h"
#include "Translate.h"
#include "Subtypes.h"
#include "TranslateKeys.h"
#include "StyleManager.h"
#include <dirent.h>

//OptionItem
OptionItem::OptionItem(int _id, string _displayValue) :
    WGuiItem(_displayValue)
{
    id = _id;
    mFocus = false;
}

//OptionInteger
void OptionInteger::Render()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
    mFont->SetColor(getColor(WGuiColor::TEXT));
    mFont->DrawString(_(displayValue).c_str(), x + 2, y + 3);
    char buf[512];

    if (maxValue == 1)
    {
        if (value)
            sprintf(buf, "%s", _("Yes").c_str());
        else
            sprintf(buf, "%s", _("No").c_str());
    }
    else
    {
        if (value == defValue && strDefault.size())
            sprintf(buf, "%s", _(strDefault).c_str());
        else
            sprintf(buf, "%i", value);
    }
    mFont->DrawString(buf, width - 5, y + 3, JGETEXT_RIGHT);
}

OptionInteger::OptionInteger(int _id, string _displayValue, int _maxValue, int _increment, int _defV, string _sDef, int _minValue) :
    OptionItem(_id, _displayValue)
{
    defValue = _defV;
    strDefault = _sDef;
    maxValue = _maxValue;
    minValue = _minValue;
    increment = _increment;
    value = ::options[id].number;
    x = 0;
    y = 0;
}

void OptionInteger::setData()
{
    if (id != INVALID_OPTION)
        options[id] = GameOption(value);
}

//Option Select
void OptionSelect::initSelections()
{
    //Find currently active bit in the list.
    for (size_t i = 0; i < selections.size(); ++i)
        if (selections[i] == options[id].str)
            value = i;
}

void OptionSelect::Entering(JButton key)
{
    OptionItem::Entering(key);
    prior_value = value;
}

void OptionSelect::Render()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
    mFont->SetColor(getColor(WGuiColor::TEXT));
    mFont->DrawString(_(displayValue).c_str(), x, y + 2);

    if (value < selections.size())
        mFont->DrawString(_(selections[value]).c_str(), x + width - 10, y + 2, JGETEXT_RIGHT);
    else
        mFont->DrawString(_("Unset").c_str(), x + width - 10, y + 2, JGETEXT_RIGHT);
}

void OptionSelect::setData()
{
    if (id == INVALID_OPTION)
        return;

    if (value < selections.size())
        options[id] = GameOption(selections[value]);
}
bool OptionSelect::Selectable()
{
    return (selections.size() > 1);
}

void OptionSelect::addSelection(string s)
{
    selections.push_back(s);
}

//OptionProfile
const string OptionProfile::DIRTESTER = "collection.dat";
OptionProfile::OptionProfile(GameApp * _app, JGuiListener * jgl) :
    OptionDirectory("profiles/", Options::ACTIVE_PROFILE, "Profile", DIRTESTER)
{
    app = _app;
    listener = jgl;
    height = 60;
    addSelection("Default");
    sort(selections.begin(), selections.end());
    mFocus = false;
    initSelections();
    populate();
}
;

void OptionProfile::initSelections()
{
    OptionSelect::initSelections();
    initialValue = value;
}

void OptionProfile::addSelection(string s)
{
    OptionDirectory::addSelection(s);

    //Check how many options... if 1, we're not selectable.
    if (selections.size() > 1)
        canSelect = true;
    else
        canSelect = false;

}

void OptionProfile::updateValue()
{
    value++;
    if (value > selections.size() - 1)
        value = 0;

    populate();
}

void OptionProfile::Reload()
{
    OptionDirectory::Reload();
    populate();
}

void OptionProfile::populate()
{
    string temp = options[Options::ACTIVE_PROFILE].str;
    if (value >= selections.size())
    { //TODO fail gracefully.
        return;
    }
    options[Options::ACTIVE_PROFILE].str = selections[value];
    PlayerData * pdata = NEW PlayerData(MTGCollection());

    int unlocked = 0, sets = setlist.size();
    std::string contents;
    if (JFileSystem::GetInstance()->readIntoString(options.profileFile(PLAYER_SETTINGS), contents))
    {
        std::stringstream stream(contents);
        std::string s;
        while (std::getline(stream, s))
        {
            if (s.substr(0, 9) == "unlocked_")
                unlocked++;
        }
    }

    options[Options::ACTIVE_PROFILE] = temp;

    char buf[512], format[512];
    sprintf(format, "%s\n%s\n%s\n", _("Credits: %i").c_str(), _("Cards: %i").c_str(), _("Sets: %i (of %i)").c_str());
    sprintf(buf, format, pdata->credits, pdata->collection->totalCards(), unlocked, sets);
    preview = buf;

    SAFE_DELETE(pdata);
}

void OptionProfile::Render()
{
    JRenderer * renderer = JRenderer::GetInstance();
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
    mFont->SetScale(1);
    int spacing = 2 + (int) mFont->GetHeight();

    float pX, pY;
    pX = x;
    pY = y;
    char buf[512];
    if (selections[value] == "Default")
        sprintf(buf, "player/avatar.jpg");
    else
        sprintf(buf, "profiles/%s/avatar.jpg", selections[value].c_str());
    string filename = buf;
    JQuadPtr avatar = WResourceManager::Instance()->RetrieveTempQuad(filename, TEXTURE_SUB_EXACT);

    if (avatar)
    {
        renderer->RenderQuad(avatar.get(), x, pY);
        pX += 40;
    }

    mFont->SetColor(getColor(WGuiColor::TEXT_HEADER));
    mFont->DrawString(selections[value].c_str(), pX, pY + 2, JGETEXT_LEFT);
    mFont->SetScale(0.8f);
    mFont->SetColor(getColor(WGuiColor::TEXT_BODY));
    mFont->DrawString(preview.c_str(), pX, pY + spacing + 2, JGETEXT_LEFT);
    mFont->SetScale(1.0f);

}

void OptionProfile::Entering(JButton key)
{
    mFocus = true;
    initialValue = value;
}

void OptionProfile::confirmChange(bool confirmed)
{
    if (initialValue >= selections.size())
        return;

    int result;

    if (confirmed)
        result = value;
    else
        result = initialValue;

    options[Options::ACTIVE_PROFILE] = selections[result];
    value = result;

    populate();
    if (listener && confirmed)
    {
        listener->ButtonPressed(-102, 5);
        initialValue = value;
    }
    return;
}

//OptionThemeStyle
OptionThemeStyle::OptionThemeStyle(string _displayValue) :
    OptionSelect(Options::GUI_STYLE, _displayValue)
{
    Reload();
    initSelections();
}

bool OptionThemeStyle::Visible()
{
    return (selections.size() > 1);
}

void OptionThemeStyle::confirmChange(bool confirmed)
{
    options.getStyleMan()->determineActive(NULL, NULL);
}

void OptionThemeStyle::Reload()
{
    selections.clear();
    addSelection("Dynamic");
    map<string, WStyle*>::iterator it;

    StyleManager * sm = options.getStyleMan();
    for (it = sm->styles.begin(); it != sm->styles.end(); it++)
        addSelection(it->first);
}
//OptionLanguage
OptionLanguage::OptionLanguage(string _displayValue) :
    OptionSelect(Options::LANG, _displayValue)
{
    Reload();
    initSelections();
}
;

void OptionLanguage::setData()
{
    if (id == INVALID_OPTION)
        return;

    if (value < selections.size())
    {
        options[id] = GameOption(actual_data[value]);
        Translator::EndInstance();
        Translator::GetInstance()->init();
        Translator::GetInstance()->tempValues.clear();
    }
}

void OptionLanguage::confirmChange(bool confirmed)
{
    if (!confirmed)
        value = prior_value;
    else
    {
        setData();
        if (Changed())
        {
            options[id] = GameOption(actual_data[value]);
            Translator::EndInstance();
            Translator::GetInstance()->init();
            Translator::GetInstance()->tempValues.clear();
        }
        prior_value = value;
    }
}

void OptionLanguage::Reload()
{

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
            if (!s.size())
            {
                lang = "";
            }
            else
            {
                if (s[s.size() - 1] == '\r')
                    s.erase(s.size() - 1); //Handle DOS files
                size_t found = s.find("#LANG:");
                if (found != 0)
                    lang = "";
                else
                    lang = s.substr(6);
            }
        }
        file.close();

        if (lang.size())
        {
            string filen = langFiles[i];
            addSelection(filen.substr(0, filen.size() - 4), lang);
        }
    }
    initSelections();
}

void OptionLanguage::addSelection(string s, string show)
{
    selections.push_back(show);
    actual_data.push_back(s);
}

void OptionLanguage::initSelections()
{
    //Find currently active bit in the list.
    for (size_t i = 0; i < actual_data.size(); i++)
    {
        if (actual_data[i] == options[id].str)
            value = i;
    }
}

bool OptionLanguage::Visible()
{
    if (selections.size() > 1)
        return true;
    return false;
}

bool OptionLanguage::Selectable()
{
    if (selections.size() > 1)
        return true;
    return false;
}

//OptionDirectory
void OptionDirectory::Reload()
{
    vector<string> subfolders = JFileSystem::GetInstance()->scanfolder(root);
    for (size_t i = 0; i < subfolders.size(); ++i)
    {
        string filename = root + "/" + subfolders[i] + "/" + type;
        if (!JFileSystem::GetInstance()->FileExists(filename))
            continue;
        if (find(selections.begin(), selections.end(), subfolders[i]) == selections.end())
            addSelection(subfolders[i]);
    }
    initSelections();
}

OptionDirectory::OptionDirectory(string root, int id, string displayValue, string type) :
    OptionSelect(id, displayValue), root(root), type(type)
{
    vector<string> subfolders = JFileSystem::GetInstance()->scanfolder(root);
    
    for (size_t i = 0; i < subfolders.size(); ++i)
    {
        string subfolder = subfolders[i].substr(0, subfolders[i].length());
        if(subfolders[i].find("/") == subfolders[i].length())
        subfolder = subfolders[i].substr(0, subfolders[i].length() - 1); //remove trailing "/" 
        vector<string> path;
        path.push_back(root);
        path.push_back(subfolder);
        string filePath = buildFilePath(path, type);
        if (JFileSystem::GetInstance()->FileExists(filePath))
            addSelection(subfolder);
    }
    initSelections();
}

const string OptionTheme::DIRTESTER = "preview.png";
OptionTheme::OptionTheme(OptionThemeStyle * style) :
    OptionDirectory("themes", Options::ACTIVE_THEME, "Current Theme", DIRTESTER)
{
    addSelection("Default");
    sort(selections.begin(), selections.end());
    initSelections();
    mFocus = false;
    bChecked = false;
    ts = style;
}

JQuadPtr OptionTheme::getImage()
{
    char buf[512];
    string val = selections[value];
    if (val == "Default")
        sprintf(buf, "graphics/preview.png");
    else
        sprintf(buf, "themes/%s/preview.png", val.c_str());
    string filename = buf;
    return WResourceManager::Instance()->RetrieveTempQuad(filename, TEXTURE_SUB_EXACT);
}

float OptionTheme::getHeight()
{
    return 130;
}

void OptionTheme::updateValue()
{
    OptionDirectory::updateValue();
    bChecked = false;
}

void OptionTheme::Render()
{
    JRenderer * renderer = JRenderer::GetInstance();
    char buf[512];
    if (!bChecked)
    {
        author = "";
        bChecked = true;
        if (selections[value] == "Default")
            sprintf(buf, "%s", "graphics/themeinfo.txt");
        else
            sprintf(buf, "themes/%s/themeinfo.txt", selections[value].c_str());
        string contents;
        if (JFileSystem::GetInstance()->readIntoString(buf, contents))
        {
            std::stringstream stream(contents);
            string temp;
            std::getline(stream, temp);
            for (unsigned int x = 0; x < 17 && x < temp.size(); x++)
            {
                if (isprint(temp[x])) //Clear stuff that breaks mFont->DrawString, cuts to 16 chars.
                    author += temp[x];
            }
        }
    }
    sprintf(buf, _("Theme: %s").c_str(), selections[value].c_str());

    JQuadPtr q = getImage();
    if (q)
    {
        float scale = 128 / q->mHeight;
        renderer->RenderQuad(q.get(), x, y, 0, scale, scale);
    }

    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
    mFont->SetColor(getColor(WGuiColor::TEXT_HEADER));
    mFont->DrawString(buf, x + 2, y + 2);
    if (bChecked && author.size())
    {
        mFont->SetColor(getColor(WGuiColor::TEXT_BODY));
        mFont->SetScale(0.8f);
        float hi = mFont->GetHeight();
        sprintf(buf, _("Artist: %s").c_str(), author.c_str());
        mFont->DrawString(buf, x + 2, y + getHeight() - hi);
        mFont->SetScale(1);
    }
}

bool OptionTheme::Visible()
{
    if (selections.size() <= 1)
        return false;

    return true;
}

void OptionTheme::confirmChange(bool confirmed)
{
    bChecked = false;
    if (!confirmed)
        value = prior_value;
    else
    {
        setData();
        options.getStyleMan()->loadRules();
        options.getStyleMan()->determineActive(NULL, NULL);
        if (ts)
            ts->Reload();

        WResourceManager::Instance()->Refresh(); //Update images
        prior_value = value;
    }
}

OptionKey::OptionKey(GameStateOptions* g, LocalKeySym from, JButton to) :
    WGuiItem(""), from(from), to(to), grabbed(false), g(g), btnMenu(NULL)
{
}

void OptionKey::Update(float dt)
{
    if (btnMenu)
        btnMenu->Update(dt);
}
void OptionKey::Render()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
    mFont->SetColor(getColor(WGuiColor::TEXT));
    JRenderer * renderer = JRenderer::GetInstance();

    if (LOCAL_KEY_NONE == from)
    {
        string msg = _("New binding...");
        mFont->DrawString(msg, (SCREEN_WIDTH - mFont->GetStringWidth(msg.c_str())) / 2, y + 2);
    }
    else
    {
        const KeyRep& rep = translateKey(from);
        if (rep.second)
            renderer->RenderQuad(rep.second, x + 4, y + 3, 0, 16.0f / rep.second->mHeight, 16.0f / rep.second->mHeight);
        else
            mFont->DrawString(rep.first, x + 4, y + 3, JGETEXT_LEFT);
        const KeyRep& rep2 = translateKey(to);
        if (rep2.second)
        {
            float ratio = 16.0f / rep2.second->mHeight;
            renderer->RenderQuad(rep2.second, x + width - (ratio * rep2.second->mWidth) - 2, y + 3, 0, ratio, ratio);
        }
        else
            mFont->DrawString(rep2.first, width - 4, y + 3, JGETEXT_RIGHT);
    }
}

bool OptionKey::CheckUserInput(JButton key)
{
    if (btnMenu)
        return btnMenu->CheckUserInput(key);
    if (JGE_BTN_OK == key)
    {
        grabbed = true;
        g->GrabKeyboard(this);
        return true;
    }
    return false;
}

static const JButton btnList[] =
{
                JGE_BTN_MENU,
                JGE_BTN_CTRL,
                JGE_BTN_RIGHT,
                JGE_BTN_LEFT,
                JGE_BTN_UP,
                JGE_BTN_DOWN,
                JGE_BTN_OK,
                JGE_BTN_CANCEL,
                JGE_BTN_PRI,
                JGE_BTN_SEC,
                JGE_BTN_PREV,
                JGE_BTN_NEXT,
#ifdef LINUX
                JGE_BTN_FULLSCREEN,
#endif
                JGE_BTN_NONE
};

void OptionKey::KeyPressed(LocalKeySym key)
{
    from = key;
    g->UngrabKeyboard(this);
    grabbed = false;

    btnMenu = NEW SimpleMenu(0, this, Fonts::MENU_FONT, 80, 10);
    for (int i = sizeof(btnList) / sizeof(btnList[0]) - 1; i >= 0; --i)
    {
        const KeyRep& rep = translateKey(btnList[i]);
        btnMenu->Add(i, rep.first.c_str());
    }
}
bool OptionKey::isModal()
{
    return grabbed || btnMenu;
}

void OptionKey::Overlay()
{
    JRenderer * renderer = JRenderer::GetInstance();
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
    mFont->SetColor(ARGB(255, 0, 0, 0));
    if (grabbed)
    {
        static const float x = 30, y = 45;
        renderer->FillRoundRect(x, y, SCREEN_WIDTH - 2 * x, 50, 2, ARGB(200, 200, 200, 255));
        string msg = _("Press a key to associate.");
        mFont->DrawString(msg, (SCREEN_WIDTH - mFont->GetStringWidth(msg.c_str())) / 2, y + 20);
    }
    else if (btnMenu)
        btnMenu->Render();
}

void OptionKey::ButtonPressed(int controllerId, int controlId)
{
    to = btnList[controlId];
    SAFE_DELETE(btnMenu);
    btnMenu = NULL;
}

bool OptionKey::Visible()
{
    return JGE_BTN_NONE != to || LOCAL_KEY_NONE == from || btnMenu != NULL;
}

bool OptionKey::Selectable()
{
    return JGE_BTN_NONE != to || LOCAL_KEY_NONE == from || btnMenu != NULL;
}
