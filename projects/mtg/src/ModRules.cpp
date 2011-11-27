#include "PrecompiledHeader.h"

#include "ModRules.h"
#include "utils.h"
#include "GameState.h"
#include "../../../JGE/src/tinyxml/tinyxml.h"


ModRules gModRules;

bool ModRules::load(string filename)
{
    std::string xmlBuffer;
    if (! JFileSystem::GetInstance()->readIntoString(filename, xmlBuffer))
    {
        DebugTrace("FATAL: cannot find modrules.xml");
        return false;
    }

    TiXmlDocument doc;
    doc.Parse(xmlBuffer.c_str());

    for (TiXmlNode* node = doc.FirstChild(); node; node = node->NextSibling())
    {
        TiXmlElement* element = node->ToElement();
        if (element != NULL)
        {
            if (strcmp(element->Value(), "menu") == 0)
            {
                menu.parse(element);
            }
            else if (strcmp(element->Value(), "general") == 0)
            {
                 general.parse(element);
            }
            else if (strcmp(element->Value(), "cards") == 0)
            {
                 cards.parse(element);
            }
            else if (strcmp(element->Value(), "game") == 0)
            {
                 game.parse(element);
            }
            else if (strcmp(element->Value(), "cardgui") == 0)
            {
                 cardgui.parse(element);
            }
        }
    }
    return true;
}

int ModRulesMenuItem::strToAction(string str)
{
    if (str.compare("playMenu") == 0)
         return MENUITEM_PLAY;
     if (str.compare("deckEditor") == 0)
         return MENUITEM_DECKEDITOR;
     if (str.compare("shop") == 0)
         return MENUITEM_SHOP;
     if (str.compare("options") == 0)
         return  MENUITEM_OPTIONS;
     if (str.compare("quit") == 0)
         return MENUITEM_EXIT;
     if (str.compare("trophies") == 0)
         return MENUITEM_TROPHIES;

     return MENUITEM_PLAY;
}

ModRulesMenuItem::ModRulesMenuItem(string actionIdStr, string displayName)
{
    mActionId = strToAction(actionIdStr);
    mDisplayName = displayName;
}


int ModRulesMenuItem::getMatchingGameState()
{
    return getMatchingGameState(mActionId);
}

int  ModRulesMenuItem::getMatchingGameState(int actionId)
{
    switch (actionId)
    {
    case MENUITEM_DECKEDITOR:
        return GAME_STATE_DECK_VIEWER;
    case MENUITEM_SHOP:
        return GAME_STATE_SHOP;
    case MENUITEM_OPTIONS:
        return GAME_STATE_OPTIONS;
    case MENUITEM_TROPHIES:
        return GAME_STATE_AWARDS;
    default:
        return GAME_STATE_NONE;
    }
}

ModRulesMainMenuItem::ModRulesMainMenuItem(string actionIdStr, string displayName, int iconId, string particleFile): 
    ModRulesMenuItem(actionIdStr, displayName), mIconId(iconId), mParticleFile(particleFile)
{
}

JButton ModRulesOtherMenuItem::strToJButton(string str)
{
     if (str.compare("btn_next") == 0)
         return JGE_BTN_NEXT;
     if (str.compare("btn_prev") == 0)
         return JGE_BTN_PREV;
     if (str.compare("btn_ctrl") == 0)
         return JGE_BTN_CTRL;
     if (str.compare("btn_menu") == 0)
         return JGE_BTN_MENU;
     if (str.compare("btn_cancel") == 0)
         return JGE_BTN_CANCEL;
     if (str.compare("btn_pri") == 0)
         return JGE_BTN_PRI;
     if (str.compare("btn_sec") == 0)
         return JGE_BTN_SEC;

     return JGE_BTN_NEXT;
}


ModRulesOtherMenuItem::ModRulesOtherMenuItem(string actionIdStr, string displayName, string keyStr): ModRulesMenuItem(actionIdStr, displayName)
{
    mKey = strToJButton(keyStr);
}
  
void ModRulesMenu::parse(TiXmlElement* element)
{
    TiXmlNode* mainNode = element->FirstChild("main");
    if (mainNode) {
        for (TiXmlNode* node = mainNode->ToElement()->FirstChild("item"); node; node = node->NextSibling("item"))
        {
            TiXmlElement* element = node->ToElement();
            {
                main.push_back(NEW ModRulesMainMenuItem(
                    element->Attribute("action"), 
                    element->Attribute("displayName"), 
                    atoi(element->Attribute("iconId")),
                    element->Attribute("particleFile")));
            }
        }
    }

    TiXmlNode* otherNode = element->FirstChild("other");
    if (otherNode) {
        for (TiXmlNode* node = otherNode->ToElement()->FirstChild("item"); node; node = node->NextSibling("item"))
        {
            TiXmlElement* element = node->ToElement();
            if (element)
            {
                other.push_back(NEW ModRulesOtherMenuItem(
                    element->Attribute("action"), 
                    element->Attribute("displayName"), 
                    element->Attribute("key")));
            }
        }
    }
}

ModRulesMenu::~ModRulesMenu()
{
    for (size_t i = 0; i < main.size(); ++i)
        SAFE_DELETE(main[i]);

    for (size_t i = 0; i < other.size(); ++i)
        SAFE_DELETE(other[i]);

    main.clear();
    other.clear();
}

//inGame rules
ModRulesGame::ModRulesGame()
{
    mCanInterrupt = true;
}

void ModRulesGame::parse(TiXmlElement* element)
{
    int value = ModRules::getValueAsInt(element, "canInterrupt");
    if (value != -1)
        mCanInterrupt = value > 0;
}


//General Rules
ModRulesGeneral::ModRulesGeneral()
{
    mHasDeckEditor = true;
    mHasShop = true;
}

void ModRulesGeneral::parse(TiXmlElement* element)
{
    int value = ModRules::getValueAsInt(element, "hasDeckEditor");
    if (value != -1)
        mHasDeckEditor = value > 0;

    value = ModRules::getValueAsInt(element, "hasShop");
    if (value != -1)
        mHasShop = value > 0;

}

int ModRules::getValueAsInt(TiXmlElement* element, string childName){
    TiXmlNode* node = element->FirstChild(childName.c_str());
    if (node) {
        const char* value = node->ToElement()->GetText();
        return atoi(value);
    }
    return -1;
}

ModRulesCards::ModRulesCards()
{
    activateEffect = NEW SimpleCardEffectRotate(M_PI/2); //Default activation effect
}

SimpleCardEffect * ModRulesCards::parseEffect(string s)
{
    size_t limiter = s.find("(");
    string function, params;
    if (limiter != string::npos)
    {
        function = s.substr(0, limiter);
        params = s.substr(limiter+1, s.size() - 2 - limiter);
    }
    else
    {
        function = s;
    }

    if (function.compare("rotate") == 0)
    {
        return NEW SimpleCardEffectRotate(M_PI*atoi(params.c_str())/180);
    }

    if (function.compare("mask") == 0)
    {
        vector<string> argb = split( params, ',');
        if (argb.size() < 4)
        {
            DebugTrace("not enough params in mask");
            return NULL;
        }
        PIXEL_TYPE mask = ARGB(
            atoi(argb[0].c_str()),
            atoi(argb[1].c_str()),
            atoi(argb[2].c_str()),
            atoi(argb[3].c_str())
            );
        return NEW SimpleCardEffectMask(mask);
    }
    return NULL;
}

void  ModRulesCards::parse(TiXmlElement* element)
{
    TiXmlNode* node = element->FirstChild("general");
    if (node) {
        TiXmlElement* generalElement = node->ToElement();
        TiXmlNode* nodeActivation = generalElement->FirstChild("activate");
        if (nodeActivation) {
            TiXmlElement* activateElement = nodeActivation->ToElement();
            TiXmlNode* nodeUIEvent = activateElement->FirstChild("uiEvent");
            if (nodeUIEvent) {
                const char* event = nodeUIEvent->ToElement()->GetText();
                SAFE_DELETE(activateEffect);
                activateEffect = parseEffect(event);
            }
        }
    }

}

ModRulesCards::~ModRulesCards()
{
    SAFE_DELETE(activateEffect);
}

ModRulesBackGroundCardGuiItem::ModRulesBackGroundCardGuiItem(string ColorId,string ColorName, string DisplayImg, string DisplayThumb,string MenuIcon)
{
    mColorId = atoi(ColorId.c_str());
    MColorName = ColorName;
    mDisplayImg = DisplayImg;
    mDisplayThumb = DisplayThumb;
    mMenuIcon = atoi(MenuIcon.c_str());
}

ModRulesRenderCardGuiItem::ModRulesRenderCardGuiItem(string Name, string PosX, string PosY, string FormattedData, string Type)
{
    mName = Name;
    mPosX = atoi(PosX.c_str());
    mPosY = atoi(PosY.c_str());
    mFormattedData = FormattedData;
    mType = Type;
}

void ModRulesCardGui::parse(TiXmlElement* element)
{
    TiXmlNode* mainNode = element->FirstChild("background");
    if (mainNode) {
        for (TiXmlNode* node = mainNode->ToElement()->FirstChild("card"); node; node = node->NextSibling("card"))
        {
            TiXmlElement* element = node->ToElement();
            {
                background.push_back(NEW ModRulesBackGroundCardGuiItem(
                    element->Attribute("id"), 
                    element->Attribute("color"),
                    element->Attribute("img"), 
                    element->Attribute("thumb"),
                    element->Attribute("menuicon")));
            }
        }
    }
    mainNode = element->FirstChild("renderbig");
    if (mainNode) {
        for (TiXmlNode* node = mainNode->ToElement()->FirstChild("item"); node; node = node->NextSibling("item"))
        {
            TiXmlElement* element = node->ToElement();
            {
                renderbig.push_back(NEW ModRulesRenderCardGuiItem(
                    element->Attribute("name"), 
                    element->Attribute("posx"), 
                    element->Attribute("posy"),
                    element->Attribute("formattedtext"), 
                    element->Attribute("type")));
            }
        }
    }
    mainNode = element->FirstChild("rendertinycrop");
    if (mainNode) {
        for (TiXmlNode* node = mainNode->ToElement()->FirstChild("item"); node; node = node->NextSibling("item"))
        {
            TiXmlElement* element = node->ToElement();
            {
                rendertinycrop.push_back(NEW ModRulesRenderCardGuiItem(
                    element->Attribute("name"), 
                    element->Attribute("posx"), 
                    element->Attribute("posy"),
                    element->Attribute("formattedtext"), 
                    element->Attribute("type")));
            }
        }
    }
}

ModRulesCardGui::~ModRulesCardGui()
{
    for (size_t i = 0; i < background.size(); ++i)
        SAFE_DELETE(background[i]);
    for (size_t i = 0; i < renderbig.size(); ++i)
        SAFE_DELETE(renderbig[i]);
    for (size_t i = 0; i < rendertinycrop.size(); ++i)
        SAFE_DELETE(rendertinycrop[i]);

    background.clear();
    renderbig.clear(); 
    rendertinycrop.clear();
    
}
