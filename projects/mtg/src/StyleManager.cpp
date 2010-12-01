#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiBackground.h"
#include "GameObserver.h"
#include "Rules.h"
#include "DeckDataWrapper.h"
#include "WFilter.h"
#include "StyleManager.h"
#include "../../../JGE/src/tinyxml/tinyxml.h"

void StyleManager::killRules()
{
    activeStyle = "";
    vector<WStyleRule*>::iterator i;
    for (i = rules.begin(); i != rules.end(); i++)
        SAFE_DELETE(*i);
    rules.clear();

    map<string, WStyle*>::iterator mi;
    for (mi = styles.begin(); mi != styles.end(); mi++)
    {
        SAFE_DELETE(mi->second);
    }
    styles.clear();
}

StyleManager::StyleManager()
{
    loadRules();
}

StyleManager::~StyleManager()
{
    killRules();
}

string WStyle::stylized(string filename)
{
    if (mapping.find(filename) != mapping.end()) return mapping[filename];
    return filename;
}

void StyleManager::loadRules()
{
    killRules();
    //TODO Placeholder until XML format available.
    string filename = JGE_GET_RES(WResourceManager::Instance()->graphicsFile("style.txt"));
    TiXmlDocument xmlfile(filename.c_str());
    if (!xmlfile.LoadFile()) return;
    TiXmlHandle hDoc(&xmlfile);
    TiXmlElement * pRule;
    for (pRule = hDoc.FirstChildElement().Element(); pRule != NULL; pRule = pRule->NextSiblingElement())
    {
        //root should be "pack"
        string tag = pRule->Value();
        std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
        if (tag == "activebg")
        {
            //After validating, handle actual loading.
            TiXmlElement * pSlot;
            const char * holder = NULL;
            holder = pRule->Attribute("source");
            if (holder)
                playerSrc = atoi(holder);
            else
                playerSrc = -1;

            for (pSlot = pRule->FirstChildElement(); pSlot != NULL; pSlot = pSlot->NextSiblingElement())
            {
                //Load slot.
                tag = pSlot->Value();
                std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
                if (tag != "case") continue;

                WStyleRule * r = NEW WStyleRule();
                rules.push_back(r);

                holder = pSlot->Attribute("rule");
                if (holder) r->filter = holder;
                r->style = pSlot->GetText();
            }
        }
        else if (tag == "style")
        {
            TiXmlElement * pSlot;
            const char * holder = NULL;
            holder = pRule->Attribute("name");
            if (!holder) continue;
            string sname = holder;
            WStyle * s = NEW WStyle();

            for (pSlot = pRule->FirstChildElement(); pSlot != NULL; pSlot = pSlot->NextSiblingElement())
            {

                tag = pSlot->Value();
                std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
                if (tag.size() && pSlot->GetText()) s->mapping[tag] = pSlot->GetText();
            }
            if (styles[sname])
            SAFE_DELETE(styles[sname]);
            styles[sname] = s;

        }
    }

    determineActive(NULL, NULL);
    return;
}

WStyle * StyleManager::get()
{
    if (styles.find(activeStyle) != styles.end()) return styles[activeStyle];
    return NULL;
}

void StyleManager::determineActive(MTGDeck * p1, MTGDeck * p2)
{
    string check = options[Options::GUI_STYLE].str;
    if (check.size() && styles.find(check) != styles.end())
    {
        string prior = activeStyle;
        activeStyle = check;
        if (prior != activeStyle) WResourceManager::Instance()->Refresh();
        return;
    }
    topRule = -1;
    topSize = 0;

    MTGDeck * tempDeck = NEW MTGDeck(GameApp::collection);
    if (p1 && playerSrc != 2) tempDeck->add(p1);
    if (p2 && playerSrc != 1) tempDeck->add(p2);
    WCFilterFactory * ff = WCFilterFactory::GetInstance();

    if (tempDeck)
    {
        DeckDataWrapper * ddw = NEW DeckDataWrapper(tempDeck);
        for (int r = 0; r < (int) rules.size(); r++)
        {
            ddw->clearFilters();
            ddw->addFilter(ff->Construct(rules[r]->filter));
            ddw->validate();
            int ct = ddw->getCount(WSrcDeck::FILTERED_COPIES);
            if (ct > topSize)
            {
                topRule = r;
                topSize = ct;
            }
        }
        delete tempDeck;
        delete ddw;
    }

    string prior = activeStyle;
    activeStyle = "";
    if (topRule >= 0)
    {
        map<string, WStyle*>::iterator mi = styles.find(rules[topRule]->style);
        if (mi != styles.end()) activeStyle = mi->first;
    }
    if (prior != activeStyle) WResourceManager::Instance()->Refresh();

}
