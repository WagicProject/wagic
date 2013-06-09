/*
 This is where the player views their awards, etc.
 */
#include "PrecompiledHeader.h"

#include <JRenderer.h>
#include "GameStateAwards.h"
#include "GameApp.h"
#include "MTGDeck.h"
#include "Translate.h"
#include "OptionItem.h"
#include "DeckDataWrapper.h"
#include "Credits.h"

enum ENUM_AWARDS_STATE
{
    STATE_LISTVIEW, 
    STATE_DETAILS,
    EXIT_AWARDS_MENU = -102,
    GUI_AWARD_BUTTON = -103,
    
};

namespace
{
    const int kBackToTrophiesID = 2;
    const int kBackToMainMenuID = 1;
}

GameStateAwards::GameStateAwards(GameApp* parent) :
    GameState(parent, "trophies")
{

}

GameStateAwards::~GameStateAwards()
{

}

void GameStateAwards::End()
{
    SAFE_DELETE(menu);
    SAFE_DELETE(detailview);
    SAFE_DELETE(listview);
    SAFE_DELETE(setSrc);

    if (saveMe)
        options.save();
}
void GameStateAwards::Start()
{
    mParent->DoAnimation(TRANSITION_FADE_IN);
    char buf[256];
    mState = STATE_LISTVIEW;
    options.checkProfile();

    menu = NULL;
    saveMe = options.newAward();

    listview = NEW WGuiList("Listview");
    listview->setX(210);
    listview->setWidth(SCREEN_WIDTH - 220);
    detailview = NULL;
    WGuiAward * aw;
    WGuiButton * btn;

    WGuiHeader * wgh = NEW WGuiHeader("Achievements");
    listview->Add(wgh);

    aw = NEW WGuiAward(Options::DIFFICULTY_MODE_UNLOCKED, "Difficulty Modes", "Achieved a 66% victory ratio.");
    btn = NEW WGuiButton(aw, GUI_AWARD_BUTTON, Options::DIFFICULTY_MODE_UNLOCKED, this);
    listview->Add(btn);

    for (map<string, Unlockable *>::iterator it = Unlockable::unlockables.begin(); it !=  Unlockable::unlockables.end(); ++it) {
        Unlockable * award = it->second;
        aw = NEW WGuiAward(award->getValue("id"), award->getValue("name"), award->getValue("trophyroom_text"));
        btn = NEW WGuiButton(aw, GUI_AWARD_BUTTON, 0, this);
        listview->Add(btn);
    }

    aw = NEW WGuiAward(Options::EVILTWIN_MODE_UNLOCKED, "Evil Twin Mode", "Won with same army size.");
    btn = NEW WGuiButton(aw, GUI_AWARD_BUTTON, Options::EVILTWIN_MODE_UNLOCKED, this);
    listview->Add(btn);

    aw = NEW WGuiAward(Options::RANDOMDECK_MODE_UNLOCKED, "Random Deck Mode", "Won against a higher difficulty.");
    btn = NEW WGuiButton(aw, GUI_AWARD_BUTTON, Options::RANDOMDECK_MODE_UNLOCKED, this);
    listview->Add(btn);

    aw = NEW WGuiAward(Options::AWARD_COLLECTOR, "Valuable Collection", "Collection valued over 10,000c.", "Collection Info");
    btn = NEW WGuiButton(aw, GUI_AWARD_BUTTON, Options::AWARD_COLLECTOR, this);
    listview->Add(btn);

    wgh = NEW WGuiHeader("");
    listview->Add(wgh);

    int locked = 0;
    for (int i = 0; i < setlist.size(); i++)
    {
        MTGSetInfo * si = setlist.getInfo(i);
        if (!si)
            continue;
        if (!options[Options::optionSet(i)].number)
        {
            locked++;
            continue;
        }

        if (!si->author.size())
            sprintf(buf, _("%i cards.").c_str(), si->totalCards());
        else if (si->year > 0)
            sprintf(buf, _("%s (%i): %i cards").c_str(), si->author.c_str(), si->year, si->totalCards());
        else
            sprintf(buf, _("%s: %i cards.").c_str(), si->author.c_str(), si->totalCards());

        aw = NEW WGuiAward(Options::optionSet(i), si->getName(), buf, "Card Spoiler");
        aw->mFlags = WGuiItem::NO_TRANSLATE;
        btn = NEW WGuiButton(aw, GUI_AWARD_BUTTON, Options::optionSet(i), this);
        listview->Add(btn);
    }
    if (locked)
        sprintf(buf, _("%i locked sets remain.").c_str(), locked);
    else
        sprintf(buf, _("Unlocked all %i sets.").c_str(), setlist.size());

    wgh->setDisplay(buf);
    wgh->mFlags = WGuiItem::NO_TRANSLATE;

    listview->Entering(JGE_BTN_NONE);
    detailview = NULL;
    setSrc = NULL;
    showMenu = false;
}

void GameStateAwards::Create()
{
}
void GameStateAwards::Destroy()
{
}

void GameStateAwards::Render()
{
    JRenderer * r = JRenderer::GetInstance();
    r->ClearScreen(ARGB(0,0,0,0));

    JQuadPtr background = WResourceManager::Instance()->RetrieveTempQuad("awardback.jpg", TEXTURE_SUB_5551);
    if (background.get())
        r->RenderQuad(background.get(), 0, 0);

    switch (mState)
    {
    case STATE_LISTVIEW:
        if (listview)
            listview->Render();
        break;
    case STATE_DETAILS:
        if (detailview)
            detailview->Render();
        break;
    }

    if (showMenu && menu)
        menu->Render();
}

void GameStateAwards::Update(float dt)
{
    if (mEngine->GetButtonClick(JGE_BTN_CANCEL))
        options[Options::DISABLECARDS].number = !options[Options::DISABLECARDS].number;

    if (showMenu)
    {
        menu->Update(dt);
    }
    else
    {
        JButton key = JGE_BTN_NONE;

        while ((key = JGE::GetInstance()->ReadButton()))
        {
            switch (key)
            {
            case JGE_BTN_MENU:
                showMenu = true;
                SAFE_DELETE(menu);
                menu = NEW SimpleMenu(JGE::GetInstance(), EXIT_AWARDS_MENU, this, Fonts::MENU_FONT, 50, 170);
                if (mState == STATE_DETAILS)
                    menu->Add(kBackToTrophiesID, "Back to Trophies");
                menu->Add(kBackToMainMenuID, "Back to Main Menu");
                menu->Add(kCancelMenuID, "Cancel");
                break;
            case JGE_BTN_PREV:
                mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
                break;
            case JGE_BTN_SEC:
                if (mState == STATE_LISTVIEW)
                    mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
                else
                {
                    mState = STATE_LISTVIEW;
                    SAFE_DELETE(detailview);
                }
                break;
            default:
                if (mState == STATE_LISTVIEW && listview)
                {
                    listview->CheckUserInput(key);
                    listview->Update(dt);
                }
                else if (mState == STATE_DETAILS && detailview)
                {
                    detailview->CheckUserInput(key);
                    detailview->Update(dt);
                }
                break;
            }
        }
    }
    if (setSrc)
        setSrc->Update(dt);
}

bool GameStateAwards::enterSet(int setid)
{
    MTGSetInfo * si = setlist.getInfo(setid);
    map<int, MTGCard *>::iterator it;

    if (!si)
        return false;

    SAFE_DELETE(detailview);
    SAFE_DELETE(setSrc);

    setSrc = NEW WSrcCards();
    setSrc->addFilter(NEW WCFilterSet(setid));
    setSrc->loadMatches(MTGCollection());
    setSrc->bakeFilters();
    setSrc->Sort(WSrcCards::SORT_COLLECTOR);

    detailview = NEW WGuiMenu(JGE_BTN_DOWN, JGE_BTN_UP);

    WGuiList * spoiler = NEW WGuiList("Spoiler", setSrc);
    spoiler->setX(210);
    spoiler->setWidth(SCREEN_WIDTH - 220);
    for (int t = 0; t < setSrc->Size(); t++)
    {
        MTGCard * c = setSrc->getCard(t);
        if (c)
            spoiler->Add(NEW WGuiItem(c->data->name));
    }
    setSrc->setOffset(0);
    spoiler->Entering(JGE_BTN_NONE);
    WGuiCardImage * wi = NEW WGuiCardImage(setSrc);
    wi->setX(105);
    wi->setY(137);
    detailview->Add(wi);
    detailview->Add(spoiler);
    detailview->Entering(JGE_BTN_NONE);
    return true;
}
bool GameStateAwards::enterStats(int option)
{
    if (option != Options::AWARD_COLLECTOR)
        return false;
    DeckDataWrapper* ddw = NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(PLAYER_COLLECTION).c_str(), MTGCollection()));
    if (!ddw)
        return false;

    SAFE_DELETE(detailview);
    detailview = NEW WGuiList("Details");

    detailview->Add(NEW WGuiHeader("Collection Stats"));
    detailview->Entering(JGE_BTN_NONE);

    //Discover favorite set
    if (setlist.size() > 0)
    {
        int * counts = (int*) calloc(setlist.size(), sizeof(int));
        int setid = -1;
        int dupes = 0;
        MTGCard * many = NULL;
        MTGCard * costly = NULL;
        MTGCard * strong = NULL;
        MTGCard * tough = NULL;

        for (int t = 0; t < ddw->Size(); t++)
        {
            MTGCard * c = ddw->getCard(t);
            if (!c)
                continue;
            int count = ddw->count(c);
            if (!c->data->isLand() && (many == NULL || count > dupes))
            {
                many = c;
                dupes = count;
            }
            counts[c->setId] += count;
            if (costly == NULL || c->data->getManaCost()->getConvertedCost() > costly->data->getManaCost()->getConvertedCost())
                costly = c;

            if (c->data->isCreature() && (strong == NULL || c->data->getPower() > strong->data->getPower()))
                strong = c;

            if (c->data->isCreature() && (tough == NULL || c->data->getToughness() > tough->data->getToughness()))
                tough = c;
        }
        for (int i = 0; i < setlist.size(); i++)
        {
            if (setid < 0 || counts[i] > counts[setid])
                setid = i;
        }
        free(counts);

        char buf[1024];
        sprintf(buf, _("Total Value: %ic").c_str(), ddw->totalPrice());
        detailview->Add(NEW WGuiItem(buf, WGuiItem::NO_TRANSLATE));//ddw->colors

        sprintf(buf, _("Total Cards (including duplicates): %i").c_str(), ddw->getCount(WSrcDeck::UNFILTERED_COPIES));
        detailview->Add(NEW WGuiItem(buf, WGuiItem::NO_TRANSLATE));//ddw->colors

        sprintf(buf, _("Unique Cards: %i").c_str(), ddw->getCount(WSrcDeck::UNFILTERED_UNIQUE));
        detailview->Add(NEW WGuiItem(buf, WGuiItem::NO_TRANSLATE));

        if (many)
        {
            sprintf(buf, _("Most Duplicates: %i (%s)").c_str(), dupes, many->data->getName().c_str());
            detailview->Add(NEW WGuiItem(buf, WGuiItem::NO_TRANSLATE));
        }
        if (setid >= 0)
        {
            sprintf(buf, _("Favorite Set: %s").c_str(), setlist[setid].c_str());
            detailview->Add(NEW WGuiItem(buf, WGuiItem::NO_TRANSLATE));
        }
        if (costly)
        {
            sprintf(buf, _("Highest Mana Cost: %i (%s)").c_str(), costly->data->getManaCost()->getConvertedCost(),
                            costly->data->getName().c_str());
            detailview->Add(NEW WGuiItem(buf, WGuiItem::NO_TRANSLATE));
        }
        if (strong)
        {
            sprintf(buf, _("Most Powerful: %i (%s)").c_str(), strong->data->getPower(), strong->data->getName().c_str());
            detailview->Add(NEW WGuiItem(buf, WGuiItem::NO_TRANSLATE));
        }
        if (tough)
        {
            sprintf(buf, _("Toughest: %i (%s)").c_str(), tough->data->getToughness(), strong->data->getName().c_str());
            detailview->Add(NEW WGuiItem(buf, WGuiItem::NO_TRANSLATE));
        }
    }

    SAFE_DELETE(ddw->parent);
    SAFE_DELETE(ddw);
    return true;
}
void GameStateAwards::ButtonPressed(int controllerId, int controlId)
{
    if (controllerId == EXIT_AWARDS_MENU)
        switch (controlId)
        {
        case kBackToMainMenuID:
            mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
            showMenu = false;
            break;
        case kBackToTrophiesID:
            mState = STATE_LISTVIEW;
            SAFE_DELETE(detailview);
            showMenu = false;
            break;
        case kCancelMenuID:
            showMenu = false;
            break;
        }
    else if (controllerId == GUI_AWARD_BUTTON)
    {
        int setid = controlId - Options::SET_UNLOCKS;

        if (controlId >= Options::SET_UNLOCKS && enterSet(setid))
        {
            mState = STATE_DETAILS;
            mDetailItem = controlId;

        }
        else if (controlId == Options::AWARD_COLLECTOR && enterStats(controlId))
        {
            mState = STATE_DETAILS;
        }
    }
}

void GameStateAwards::OnScroll(int, int inYVelocity)
{
    if (abs(inYVelocity) > 300)
    {
        bool flickUpwards = (inYVelocity < 0);
        int velocity = (inYVelocity < 0) ? (-1 * inYVelocity) : inYVelocity;
        while(velocity > 0)
        {
            mEngine->HoldKey_NoRepeat(flickUpwards ? JGE_BTN_DOWN : JGE_BTN_UP);
            velocity -= 100;
        }
    }
}
