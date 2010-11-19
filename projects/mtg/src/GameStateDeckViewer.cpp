/*
 * GameStateDeckViewer.cpp
 * Class handling the Deck Editor
 */

#include "PrecompiledHeader.h"

#include <math.h>
#include <iomanip>

#include "DeckManager.h"
#include "GameStateDuel.h"
#include "GameStateDeckViewer.h"
#include "Translate.h"
#include "ManaCostHybrid.h"
#include "MTGCardInstance.h"
#include "WFilter.h"
#include "WDataSrc.h"
#include "DeckEditorMenu.h"
#include "SimpleMenu.h"
#include "utils.h"

//!! helper function; this is probably handled somewhere in the code already.
// If not, should be placed in general library
void StringExplode(string str, string separator, vector<string>* results)
{
    size_t found;
    found = str.find_first_of(separator);
    while (found != string::npos)
    {
        if (found > 0) results->push_back(str.substr(0, found));
        str = str.substr(found + 1);
        found = str.find_first_of(separator);
    }
    if (str.length() > 0) results->push_back(str);
}

GameStateDeckViewer::GameStateDeckViewer(GameApp* parent) :
    GameState(parent)
{
    bgMusic = NULL;
    nbDecks = 0;
    deckNum = 0;
    useFilter = 0;
    isAIDeckSave = false;
    mSwitching = false;
    welcome_menu = NULL;
    myCollection = NULL;
    myDeck = NULL;
    filterMenu = NULL;
    source = NULL;
    hudAlpha = 0;
    subMenu = NULL;
    mRotation = 0;
    mSlide = 0;
    mAlpha = 255;
    menu = NULL;
    stw = NULL;
}

GameStateDeckViewer::~GameStateDeckViewer()
{
    SAFE_DELETE(bgMusic);
    if (myDeck)
    {
        SAFE_DELETE(myDeck->parent);
        SAFE_DELETE(myDeck);
    }
    if (myCollection)
    {
        SAFE_DELETE(myCollection->parent);
        SAFE_DELETE(myCollection);
    }
    SAFE_DELETE(stw);
    SAFE_DELETE(filterMenu);
}

void GameStateDeckViewer::rotateCards(int direction)
{
    int left = direction;
    if (left)
        displayed_deck->next();
    else
        displayed_deck->prev();
    loadIndexes();

    int total = displayed_deck->Size();
    if (total)
    {
        lastPos = getCurrentPos();
        lastTotal = total;
    }
    int i = 0;
}
void GameStateDeckViewer::rebuildFilters()
{
    if (!filterMenu) filterMenu = NEW WGuiFilters("Filter by...", NULL);
    if (source)
    SAFE_DELETE(source);
    source = NEW WSrcDeckViewer(myDeck, myCollection);
    filterMenu->setSrc(source);
    if (displayed_deck != myDeck) source->swapSrc();
    filterMenu->Finish(true);
    stw->updateStats( myDeck );;
}
void GameStateDeckViewer::updateFilters()
{
    if (!displayed_deck) return;

    filterMenu->recolorFilter(useFilter - 1);
    filterMenu->Finish(true);
    int totalAfter = displayed_deck->Size();
    if (totalAfter && lastTotal)
    {

        //This part is a hack. I don't understand why in some cases "displayed_deck's" currentPos is not 0 at this stage
        {
            while (int currentPos = displayed_deck->getOffset())
            {
                if (currentPos > 0)
                    displayed_deck->prev();
                else
                    displayed_deck->next();
            }
        }

        int pos = (totalAfter * lastPos) / lastTotal;
        for (int i = 0; i < pos - 3; ++i)
        { // "-3" because card "0" is displayed at position 3 initially
            displayed_deck->next();
        }
    }
    stw->updateStats( myDeck );;
    return;
}
void GameStateDeckViewer::loadIndexes()
{
    for (int i = 0; i < 7; i++)
    {
        cardIndex[i] = displayed_deck->getCard(i);
    }
}

void GameStateDeckViewer::switchDisplay()
{
    if (displayed_deck == myCollection)
    {
        displayed_deck = myDeck;
    }
    else
    {
        displayed_deck = myCollection;
    }
    source->swapSrc();
    updateFilters();
    loadIndexes();
}

void GameStateDeckViewer::updateDecks()
{
    SAFE_DELETE(welcome_menu);
    welcome_menu = NEW DeckEditorMenu(MENU_DECK_SELECTION, this, Fonts::OPTION_FONT, "Choose Deck To Edit");
    DeckManager * deckManager = DeckManager::GetInstance();
    vector<DeckMetaData *> playerDeckList = fillDeckMenu(welcome_menu, options.profileFile());

    deckNum = 0;
    newDeckname = "";
    nbDecks = playerDeckList.size() + 1;
    welcome_menu->Add(MENU_ITEM_NEW_DECK, "--NEW--");
    if (options[Options::CHEATMODE].number && (!myCollection || myCollection->getCount(WSrcDeck::UNFILTERED_MIN_COPIES) < 4)) welcome_menu->Add(
            MENU_ITEM_CHEAT_MODE, "--UNLOCK CARDS--");
    welcome_menu->Add(MENU_ITEM_CANCEL, "Cancel");

    // update the deckmanager with the latest information
    deckManager->updateMetaDataList(&playerDeckList, false);
    // is this necessary to ensure no memory leaks?
    playerDeckList.clear();
}

void GameStateDeckViewer::buildEditorMenu()
{
    ostringstream deckSummaryInformation;
    deckSummaryInformation << "All changes are final." << endl;

    if (menu)
    SAFE_DELETE( menu );
    //Build menu.
    JRenderer::GetInstance()->FillRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 100, ARGB(0, 0, 0, 0) );
    menu = NEW DeckEditorMenu(MENU_DECK_BUILDER, this, Fonts::OPTION_FONT, "Deck Editor", myDeck, stw);

    menu->Add(MENU_ITEM_FILTER_BY, "Filter By...", "Narrow down the list of cards. ");
    menu->Add(MENU_ITEM_SWITCH_DECKS_NO_SAVE, "Switch Decks", "Do not make any changes.\nView another deck.");
    menu->Add(MENU_ITEM_SAVE_RENAME, "Rename Deck", "Change the name of the deck");
    menu->Add(MENU_ITEM_SAVE_RETURN_MAIN_MENU, "Save & Quit Editor", "Save changes.\nReturn to the main menu");
    menu->Add(MENU_ITEM_SAVE_AS_AI_DECK, "Save As AI Deck", deckSummaryInformation.str());
    menu->Add(MENU_ITEM_MAIN_MENU, "Quit Editor", "Do not make any changes to deck.\nReturn to the main menu.");
    menu->Add(MENU_ITEM_EDITOR_CANCEL, "Cancel", "Close menu.");

}

void GameStateDeckViewer::Start()
{
    hudAlpha = 0;
    mSwitching = false;
    subMenu = NULL;
    myDeck = NULL;
    mStage = STAGE_WELCOME;
    mRotation = 0;
    mSlide = 0;
    mAlpha = 255;
    last_user_activity = NO_USER_ACTIVITY_HELP_DELAY + 1;
    onScreenTransition = 0;
    useFilter = 0;
    lastPos = 0;
    lastTotal = 0;

    pricelist = NEW PriceList(JGE_GET_RES("settings/prices.dat").c_str(), mParent->collection);
    playerdata = NEW PlayerData(mParent->collection);
    myCollection = NEW DeckDataWrapper(playerdata->collection);
    myCollection->Sort(WSrcCards::SORT_ALPHA);
    displayed_deck = myCollection;

    //Icons
    mIcons[Constants::MTG_COLOR_ARTIFACT] = resources.GetQuad("c_artifact");
    mIcons[Constants::MTG_COLOR_LAND] = resources.GetQuad("c_land");
    mIcons[Constants::MTG_COLOR_WHITE] = resources.GetQuad("c_white");
    mIcons[Constants::MTG_COLOR_RED] = resources.GetQuad("c_red");
    mIcons[Constants::MTG_COLOR_BLACK] = resources.GetQuad("c_black");
    mIcons[Constants::MTG_COLOR_BLUE] = resources.GetQuad("c_blue");
    mIcons[Constants::MTG_COLOR_GREEN] = resources.GetQuad("c_green");
    for (int i = 0; i < 7; i++)
    {
        mIcons[i]->SetHotSpot(16, 16);
    }

    //Grab a texture in VRAM.
    pspIconsTexture = resources.RetrieveTexture("iconspsp.png", RETRIEVE_LOCK);

    char buf[512];
    for (int i = 0; i < 8; i++)
    {
        sprintf(buf, "iconspsp%d", i);
        pspIcons[i] = resources.RetrieveQuad("iconspsp.png", (float) i * 32, 0, 32, 32, buf);
        pspIcons[i]->SetHotSpot(16, 16);
    }

    backQuad = resources.GetQuad("back");

    //init welcome menu
    updateDecks();

    GameApp::playMusic("track1.mp3");

    loadIndexes();
    mEngine->ResetInput();
    JRenderer::GetInstance()->EnableVSync(true);
}

void GameStateDeckViewer::End()
{
    JRenderer::GetInstance()->EnableVSync(false);

    SAFE_DELETE(welcome_menu);
    SAFE_DELETE(menu);
    SAFE_DELETE(subMenu);

    resources.Release(pspIconsTexture);
    if (myCollection)
    {
        SAFE_DELETE(myCollection);
    }
    if (myDeck)
    {
        SAFE_DELETE(myDeck->parent);
        SAFE_DELETE(myDeck);
    }
    SAFE_DELETE(pricelist);
    SAFE_DELETE(playerdata);
    SAFE_DELETE(filterMenu);
    SAFE_DELETE(source);
}

void GameStateDeckViewer::addRemove(MTGCard * card)
{
    if (!card) return;
    if (displayed_deck->Remove(card, 1, (displayed_deck == myDeck)))
    {
        if (displayed_deck == myCollection)
        {
            myDeck->Add(card);
            myDeck->Sort(WSrcCards::SORT_ALPHA);
        }
        else
        {
            myCollection->Add(card);
        }
    }
    myCollection->validate();
    myDeck->validate();
    stw->needUpdate = true;
    loadIndexes();
}

void GameStateDeckViewer::saveDeck()
{
    myDeck->save();
    playerdata->save();
    pricelist->save();
}

/**
 save the deck in a readable format to allow people to edit the file offline
 */
void GameStateDeckViewer::saveAsAIDeck(string deckName)
{
    DeckManager * deckManager = DeckManager::GetInstance();
    vector<DeckMetaData *> aiDecks = GameState::getValidDeckMetaData(JGE_GET_RES("ai/baka"), "ai_baka", NULL);
    int nbAiDecks = aiDecks.size() + 1;
    aiDecks.clear();

    string defaultAiDeckName = "deck";
    std::ostringstream oss;
    oss << "deck" << nbAiDecks;
    defaultAiDeckName = oss.str();
    oss.str("");
    if (myDeck->parent->meta_desc == "")
        oss << endl << "Can you beat your own creations?" << endl << "User created AI Deck # " << nbAiDecks;
    else
        oss << myDeck->parent->meta_desc;
    string deckDesc = oss.str();
    string filepath = JGE_GET_RES("ai/baka/");
    filepath.append(defaultAiDeckName).append(".txt");
    DebugTrace("saving AI deck " << filepath);
    myDeck->save(filepath, true, deckName, deckDesc);
}

void GameStateDeckViewer::Update(float dt)
{

    int myD = (displayed_deck == myDeck);

    if (options.keypadActive())
    {
        options.keypadUpdate(dt);

        if (newDeckname != "")
        {
            newDeckname = options.keypadFinish();

            if (newDeckname != "")
            {
                if (isAIDeckSave)
                {
                    saveAsAIDeck(newDeckname);
                    isAIDeckSave = false;
                }
                else if (myDeck && myDeck->parent)
                {
                    myDeck->parent->meta_name = newDeckname;
                    saveDeck();
                }
                mStage = STAGE_WAITING;
            }
            newDeckname = "";
        }
        //Prevent screen from updating.
        return;
    }
    hudAlpha = 255 - ((int) last_user_activity * 500);
    if (hudAlpha < 0) hudAlpha = 0;
    if (subMenu)
    {
        subMenu->Update(dt);
        if (subMenu->closed)
        {
            SAFE_DELETE(subMenu);
        }
        return;
    }
    if (mStage == STAGE_WAITING || mStage == STAGE_ONSCREEN_MENU)
    {
        switch (mEngine->ReadButton())
        {
        case JGE_BTN_LEFT:
            last_user_activity = 0;
            mStage = STAGE_TRANSITION_LEFT;
            break;
        case JGE_BTN_RIGHT:
            last_user_activity = 0;
            mStage = STAGE_TRANSITION_RIGHT;
            break;
        case JGE_BTN_UP:
            last_user_activity = 0;
            mStage = STAGE_TRANSITION_UP;
            useFilter++;
            if (useFilter >= MAX_SAVED_FILTERS) useFilter = 0;
            break;
        case JGE_BTN_DOWN:
            last_user_activity = 0;
            mStage = STAGE_TRANSITION_DOWN;
            useFilter--;
            if (useFilter < 0) useFilter = MAX_SAVED_FILTERS - 1;
            break;
        case JGE_BTN_CANCEL:
            options[Options::DISABLECARDS].number = !options[Options::DISABLECARDS].number;
            break;
        case JGE_BTN_PRI:
            if (last_user_activity > 0.2)
            {
                last_user_activity = 0;
                switchDisplay();
            }
            break;
        case JGE_BTN_OK:
            last_user_activity = 0;
            addRemove(cardIndex[2]);
            break;
        case JGE_BTN_SEC:
            last_user_activity = 0;
            SAFE_DELETE(subMenu);
            char buffer[4096];
            {
                MTGCard * card = cardIndex[2];
                if (card && displayed_deck->count(card))
                {
                    price = pricelist->getSellPrice(card->getMTGId());
                    sprintf(buffer, "%s : %i %s", _(card->data->getName()).c_str(), price, _("credits").c_str());
                    const float menuXOffset = SCREEN_WIDTH_F - 300;
                    const float menuYOffset = SCREEN_HEIGHT_F / 2;
                    subMenu = NEW SimpleMenu(MENU_CARD_PURCHASE, this, Fonts::MAIN_FONT, menuXOffset, menuYOffset, buffer);
                    subMenu->Add(MENU_ITEM_YES, "Yes");
                    subMenu->Add(MENU_ITEM_NO, "No", "", true);
                }
            }
            stw->needUpdate = true;
            break;

        case JGE_BTN_MENU:
            mStage = STAGE_MENU;
            buildEditorMenu();
            break;
        case JGE_BTN_CTRL:
            mStage = STAGE_FILTERS;
            if (!filterMenu)
            {
                filterMenu = NEW WGuiFilters("Filter by...", NULL);
                if (source)
                SAFE_DELETE(source);
                source = NEW WSrcDeckViewer(myDeck, myCollection);
                filterMenu->setSrc(source);
                if (displayed_deck != myDeck) source->swapSrc();
            }
            filterMenu->Entering(JGE_BTN_NONE);
            break;
        case JGE_BTN_PREV:
            if (last_user_activity < NO_USER_ACTIVITY_HELP_DELAY)
                last_user_activity = NO_USER_ACTIVITY_HELP_DELAY + 1;
            else if ((mStage == STAGE_ONSCREEN_MENU) && (--stw->currentPage < 0)) stw->currentPage = stw->pageCount;
            break;
        case JGE_BTN_NEXT:
            if (last_user_activity < NO_USER_ACTIVITY_HELP_DELAY)
                last_user_activity = NO_USER_ACTIVITY_HELP_DELAY + 1;
            else if ((mStage == STAGE_ONSCREEN_MENU) && (++stw->currentPage > stw->pageCount)) stw->currentPage = 0;
            break;
        default: // no keypress
            if (last_user_activity > NO_USER_ACTIVITY_HELP_DELAY)
            {
                if (mStage != STAGE_ONSCREEN_MENU)
                {
                    mStage = STAGE_ONSCREEN_MENU;
                    onScreenTransition = 1;
                }
                else
                {
                    if (onScreenTransition > 0)
                        onScreenTransition -= 0.05f;
                    else
                        onScreenTransition = 0;
                }
            }
            else
                last_user_activity += dt;
        }

    }
    if (mStage == STAGE_TRANSITION_RIGHT || mStage == STAGE_TRANSITION_LEFT)
    {
        if (mStage == STAGE_TRANSITION_RIGHT)
        {
            mRotation -= dt * MED_SPEED;
            if (mRotation < -1.0f)
            {
                do
                {
                    rotateCards(mStage);
                    mRotation += 1;
                } while (mRotation < -1.0f);
                mStage = STAGE_WAITING;
                mRotation = 0;
            }
        }
        else if (mStage == STAGE_TRANSITION_LEFT)
        {
            mRotation += dt * MED_SPEED;
            if (mRotation > 1.0f)
            {
                do
                {
                    rotateCards(mStage);
                    mRotation -= 1;
                } while (mRotation > 1.0f);
                mStage = STAGE_WAITING;
                mRotation = 0;
            }
        }
    }
    if (mStage == STAGE_TRANSITION_DOWN || mStage == STAGE_TRANSITION_UP)
    {
        if (mStage == STAGE_TRANSITION_DOWN)
        {
            mSlide -= 0.05f;
            if (mSlide < -1.0f)
            {
                updateFilters();
                loadIndexes();
                mSlide = 1;
            }
            else if (mSlide > 0 && mSlide < 0.05)
            {
                mStage = STAGE_WAITING;
                mSlide = 0;
            }
        }
        if (mStage == STAGE_TRANSITION_UP)
        {
            mSlide += 0.05f;
            if (mSlide > 1.0f)
            {
                updateFilters();
                loadIndexes();
                mSlide = -1;
            }
            else if (mSlide < 0 && mSlide > -0.05)
            {
                mStage = STAGE_WAITING;
                mSlide = 0;
            }
        }

    }
    else if (mStage == STAGE_WELCOME)
        welcome_menu->Update(dt);
    else if (mStage == STAGE_MENU)
        menu->Update(dt);
    else if (mStage == STAGE_FILTERS)
    {
        JButton key = mEngine->ReadButton();
        if (filterMenu)
        {
            if (key == JGE_BTN_CTRL)
            {
                //useFilter = 0;
                filterMenu->Finish(true);
                filterMenu->Update(dt);
                loadIndexes();
                return;
            }
            if (!filterMenu->isFinished())
            {
                filterMenu->CheckUserInput(key);
                filterMenu->Update(dt);
            }
            else
            {
                mStage = STAGE_WAITING;
                updateFilters();
                loadIndexes();
            }
        }
    }
}

void GameStateDeckViewer::renderOnScreenBasicInfo()
{
    JRenderer *renderer = JRenderer::GetInstance();
    WFont * mFont = resources.GetWFont(Fonts::MAIN_FONT);
    char buffer[256];
    int myD = (displayed_deck == myDeck);

    float y = 0;
    int allCopies, nowCopies;
    nowCopies = displayed_deck->getCount(WSrcDeck::FILTERED_COPIES);
    allCopies = displayed_deck->getCount(WSrcDeck::UNFILTERED_COPIES);
    WCardFilter * wc = displayed_deck->getFiltersRoot();

    if (wc)
        sprintf(buffer, "%s %i of %i cards (%i unique)", (displayed_deck == myDeck) ? "DECK: " : " ", nowCopies, allCopies,
                displayed_deck->getCount(WSrcDeck::FILTERED_UNIQUE));
    else
        sprintf(buffer, "%s%i cards (%i unique)", (displayed_deck == myDeck) ? "DECK: " : " ", allCopies, displayed_deck->getCount(
                WSrcDeck::UNFILTERED_UNIQUE));

    float w = mFont->GetStringWidth(buffer);
    renderer->FillRoundRect(SCREEN_WIDTH - (w + 27), y + 5, w + 10, 15, 5, ARGB(128,0,0,0));

    mFont->DrawString(buffer, SCREEN_WIDTH - 22, y + 15, JGETEXT_RIGHT);
    if (useFilter != 0) renderer->RenderQuad(mIcons[useFilter - 1], SCREEN_WIDTH - 10, y + 15, 0.0f, 0.5, 0.5);
}

//returns position of the current card (cusor) in the currently viewed color/filter
int GameStateDeckViewer::getCurrentPos()
{
    int total = displayed_deck->Size();

    int currentPos = displayed_deck->getOffset();
    currentPos += 2; //we start by displaying card number 3
    currentPos = currentPos % total + 1;
    if (currentPos < 0) currentPos = (total + currentPos);
    if (!currentPos) currentPos = total;
    return currentPos;
}

void GameStateDeckViewer::renderSlideBar()
{
    WFont * mFont = resources.GetWFont(Fonts::MAIN_FONT);

    int total = displayed_deck->Size();
    if (total == 0) return;

    float filler = 15;
    float y = SCREEN_HEIGHT_F - 25;
    float bar_size = SCREEN_WIDTH_F - 2 * filler;
    JRenderer * r = JRenderer::GetInstance();
    int currentPos = getCurrentPos();

    float cursor_pos = bar_size * currentPos / total;

    r->FillRoundRect(filler + 5, y + 5, bar_size, 0, 3, ARGB(hudAlpha/2,0,0,0));
    r->DrawLine(filler + cursor_pos + 5, y + 5, filler + cursor_pos + 5, y + 10, ARGB(hudAlpha/2,0,0,0));

    r->FillRoundRect(filler, y, bar_size, 0, 3, ARGB(hudAlpha/2,128,128,128));
    r->DrawLine(filler + cursor_pos, y, filler + cursor_pos, y + 5, ARGB(hudAlpha,255,255,255));
    char buffer[256];
    string deckname = _("Collection");
    if (displayed_deck == myDeck)
    {
        deckname = _("Deck");
    }
    sprintf(buffer, "%s - %i/%i", deckname.c_str(), currentPos, total);
    mFont->SetColor(ARGB(hudAlpha,255,255,255));
    mFont->DrawString(buffer, SCREEN_WIDTH / 2, y, JGETEXT_CENTER);

    mFont->SetColor(ARGB(255,255,255,255));
}

void GameStateDeckViewer::renderDeckBackground()
{
    int max1 = 0;
    int maxC1 = 4;
    int max2 = 0;
    int maxC2 = 4;

    for (int i = 0; i < Constants::MTG_NB_COLORS - 1; i++)
    {
        int value = myDeck->getCount(i);
        if (value > max1)
        {
            max2 = max1;
            maxC2 = maxC1;
            max1 = value;
            maxC1 = i;
        }
        else if (value > max2)
        {
            max2 = value;
            maxC2 = i;
        }
    }
    if (max2 < max1 / 2)
    {
        maxC2 = maxC1;
    }
    PIXEL_TYPE colors[] = { ARGB(255, Constants::_r[maxC1], Constants::_g[maxC1], Constants::_b[maxC1]),
            ARGB(255, Constants::_r[maxC1], Constants::_g[maxC1], Constants::_b[maxC1]),
            ARGB(255, Constants::_r[maxC2], Constants::_g[maxC2], Constants::_b[maxC2]),
            ARGB(255, Constants::_r[maxC2], Constants::_g[maxC2], Constants::_b[maxC2]), };

    JRenderer::GetInstance()->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, colors);

}

void GameStateDeckViewer::renderOnScreenMenu()
{

    WFont * font = resources.GetWFont(Fonts::MAIN_FONT);
    font->SetColor(ARGB(255,255,255,255));
    JRenderer * r = JRenderer::GetInstance();
    float pspIconsSize = 0.5;
    float fH = font->GetHeight() + 1;

    float leftTransition = onScreenTransition * 84;
    float rightTransition = onScreenTransition * 204;
    float leftPspX = 40 - leftTransition;
    float leftPspY = SCREEN_HEIGHT / 2 - 20;
    float rightPspX = SCREEN_WIDTH - 100 + rightTransition;
    float rightPspY = SCREEN_HEIGHT / 2 - 20;

    if (stw->currentPage == 0)
    {
        //FillRects
        r->FillRect(0 - (onScreenTransition * 84), 0, 84, SCREEN_HEIGHT, ARGB(128,0,0,0));
        r->FillRect(SCREEN_WIDTH - 204 + (onScreenTransition * 204), 0, 200, SCREEN_HEIGHT, ARGB(128,0,0,0));

        //LEFT PSP CIRCLE render
        r->FillCircle(leftPspX, leftPspY, 40, ARGB(128,50,50,50));

        r->RenderQuad(pspIcons[0], leftPspX, leftPspY - 20, 0, pspIconsSize, pspIconsSize);
        r->RenderQuad(pspIcons[1], leftPspX, leftPspY + 20, 0, pspIconsSize, pspIconsSize);
        r->RenderQuad(pspIcons[2], leftPspX - 20, leftPspY, 0, pspIconsSize, pspIconsSize);
        r->RenderQuad(pspIcons[3], leftPspX + 20, leftPspY, 0, pspIconsSize, pspIconsSize);

        font->DrawString(_("Prev."), leftPspX - 35, leftPspY - 15);
        font->DrawString(_("Next"), leftPspX + 15, leftPspY - 15);
        font->DrawString(_("card"), leftPspX - 35, leftPspY);
        font->DrawString(_("card"), leftPspX + 15, leftPspY);
        font->DrawString(_("Next edition"), leftPspX - 33, leftPspY - 35);
        font->DrawString(_("Prev. edition"), leftPspX - 33, leftPspY + 25);

        //RIGHT PSP CIRCLE render
        r->FillCircle(rightPspX + (onScreenTransition * 204), rightPspY, 40, ARGB(128,50,50,50));
        r->RenderQuad(pspIcons[4], rightPspX + 20, rightPspY, 0, pspIconsSize, pspIconsSize);
        r->RenderQuad(pspIcons[5], rightPspX, rightPspY - 20, 0, pspIconsSize, pspIconsSize);
        r->RenderQuad(pspIcons[6], rightPspX - 20, rightPspY, 0, pspIconsSize, pspIconsSize);
        r->RenderQuad(pspIcons[7], rightPspX, rightPspY + 20, 0, pspIconsSize, pspIconsSize);

        font->DrawString(_("Toggle Images"), rightPspX - 35, rightPspY - 40);

        if (displayed_deck == myCollection)
        {
            font->DrawString(_("Add card"), rightPspX + 20, rightPspY - 15);
            font->DrawString(_("View Deck"), rightPspX - 20, rightPspY - 15, JGETEXT_RIGHT);
        }
        else
        {
            font->DrawString(_("Remove card"), rightPspX + 20, rightPspY - 15);
            font->DrawString(_("View Collection"), rightPspX - 20, rightPspY - 15, JGETEXT_RIGHT);
        }
        font->DrawString(_("Sell card"), rightPspX - 30, rightPspY + 20);
        //Bottom menus
        font->DrawString(_("menu"), SCREEN_WIDTH - 35 + rightTransition, SCREEN_HEIGHT - 15);
        font->DrawString(_("filter"), SCREEN_WIDTH - 95 + rightTransition, SCREEN_HEIGHT - 15);

        //Your Deck Information
        char buffer[300];
        int nb_letters = 0;
        for (int j = 0; j < Constants::MTG_NB_COLORS; j++)
        {
            int value = myDeck->getCount(j);
            if (value > 0)
            {
                sprintf(buffer, "%i", value);
                font->DrawString(buffer, SCREEN_WIDTH - 190 + rightTransition + nb_letters * 13, SCREEN_HEIGHT / 2 + 40);
                r->RenderQuad(mIcons[j], SCREEN_WIDTH - 197 + rightTransition + nb_letters * 13, SCREEN_HEIGHT / 2 + 46, 0, 0.5,
                        0.5);
                if (value > 9)
                {
                    nb_letters += 3;
                }
                else
                {
                    nb_letters += 2;
                }
            }
        }
        int value = myDeck->getCount(WSrcDeck::UNFILTERED_COPIES);
        sprintf(buffer, _("Your Deck: %i cards").c_str(), value);
        font->DrawString(buffer, SCREEN_WIDTH - 200 + rightTransition, SCREEN_HEIGHT / 2 + 25);

        if (displayed_deck == myCollection)
        {
            font->DrawString(_("in: collection"), 5 - leftTransition, 5);
            font->DrawString(_("Use SQUARE to view your deck,"), SCREEN_WIDTH - 200 + rightTransition, 5);
        }
        else
        {
            font->DrawString(_("in: deck"), 5 - leftTransition, 5);
            font->DrawString(_("Use SQUARE to view collection,"), SCREEN_WIDTH - 200 + rightTransition, 5);
        }
        font->DrawString(_("Press L/R to cycle through"), SCREEN_WIDTH - 200 + rightTransition, 5 + fH);
        font->DrawString(_("deck statistics."), SCREEN_WIDTH - 200 + rightTransition, 5 + fH * 2);

    }
    else
    {
        stw->updateStats( myDeck );;

        char buffer[300];

        leftTransition = -(onScreenTransition / 2) * SCREEN_WIDTH;
        rightTransition = -leftTransition;

        r->FillRect(0 + leftTransition, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, ARGB(128,0,0,0));
        r->FillRect(SCREEN_WIDTH / 2 + rightTransition, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, ARGB(128,0,0,0));
        r->FillRect(10 + leftTransition, 10, SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT - 20, ARGB(128,0,0,0));
        r->FillRect(SCREEN_WIDTH / 2 + rightTransition, 10, SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT - 20, ARGB(128,0,0,0));
        font->DrawString(_("menu"), SCREEN_WIDTH - 35 + rightTransition, SCREEN_HEIGHT - 15);
        font->DrawString(_("filter"), SCREEN_WIDTH - 95 + rightTransition, SCREEN_HEIGHT - 15);

        int nb_letters = 0;
        float posX, posY;
        DWORD graphColor;

        graphColor = ARGB(200, 155, 155, 155);
        string STATS_TITLE_FORMAT = _("%i: %s");

        switch (stw->currentPage)
        {
        case 1: // Counts, price
            // Title
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), stw->currentPage, _("Statistics Summary").c_str());
            font->DrawString(buffer, 10 + leftTransition, 10);

            posY = 30;
            posX = 180;
            sprintf(buffer, _("Your Deck: %i cards").c_str(), stw->cardCount);
            font->DrawString(buffer, 20 + leftTransition, posY);
            posY += 10;

            // Counts by color
            for (int j = 0; j < Constants::MTG_NB_COLORS; j++)
            {
                int value = myDeck->getCount(j);
                if (value > 0)
                {
                    sprintf(buffer, "%i", value);
                    font->DrawString(buffer, 38 + nb_letters * 13 + leftTransition, posY + 5);
                    r->RenderQuad(mIcons[j], 30 + nb_letters * 13 + leftTransition, posY + 11, 0, 0.5, 0.5);
                    if (value > 9)
                    {
                        nb_letters += 3;
                    }
                    else
                    {
                        nb_letters += 2;
                    }
                }
            }
            posY += 25;

            r->DrawLine(posX - 4 + leftTransition, posY - 1, posX - 4 + leftTransition, posY + 177, ARGB(128, 255, 255, 255));
            r->DrawLine(19 + leftTransition, posY - 1, 19 + leftTransition, posY + 177, ARGB(128, 255, 255, 255));
            r->DrawLine(posX + 40 + leftTransition, posY - 1, posX + 40 + leftTransition, posY + 177, ARGB(128, 255, 255, 255));

            r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));

            font->DrawString(_("Lands"), 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), stw->countLands);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 14;
            r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Creatures"), 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), stw->countCreatures);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 14;
            r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Spells"), 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), stw->countSpells);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Instants"), 30 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), stw->countInstants);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Enchantments"), 30 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), stw->countEnchantments);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Sorceries"), 30 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), stw->countSorceries);
            font->DrawString(buffer, posX + leftTransition, posY);
            //sprintf(buffer, "Artifacts: %i", stw->countArtifacts);
            //mFont->DrawString(buffer, 20, 123);

            posY += 14;
            r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));

            font->DrawString(_("Average converted mana cost"), 20 + leftTransition, posY);
            sprintf(buffer, _("%2.2f").c_str(), stw->avgManaCost);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 14;
            r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Probabilities"), 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("No land in 1st hand"), 30 + leftTransition, posY);
            sprintf(buffer, _("%2.2f%%").c_str(), stw->noLandsProbInTurn[0]);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 10;
            font->DrawString(_("No land in 9 cards"), 30 + leftTransition, posY);
            sprintf(buffer, _("%2.2f%%").c_str(), stw->noLandsProbInTurn[2]);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 10;
            font->DrawString(_("No creatures in 1st hand"), 30 + leftTransition, posY);
            sprintf(buffer, _("%2.2f%%").c_str(), stw->noCreaturesProbInTurn[0]);
            font->DrawString(buffer, posX + leftTransition, posY);

            // Playgame Statistics
            posY += 14;
            r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Playgame statistics"), 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Games played"), 30 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), stw->gamesPlayed);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Victory ratio"), 30 + leftTransition, posY);
            sprintf(buffer, _("%i%%").c_str(), stw->percentVictories);
            font->DrawString(buffer, posX + leftTransition, posY);

            posY += 15;
            r->DrawLine(20 + leftTransition, posY - 1, posX + 40 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Total price (credits)"), 20 + leftTransition, posY);
            sprintf(buffer, _("%i ").c_str(), stw->totalPrice);
            font->DrawString(buffer, posX + leftTransition, posY);
            r->DrawLine(20 + leftTransition, posY + 13, posX + 40 + leftTransition, posY + 13, ARGB(128, 255, 255, 255));

            break;

        case 5: // Land statistics
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), stw->currentPage, _("Mana production").c_str());
            font->DrawString(buffer, 10 + leftTransition, 10);

            font->DrawString(_("Counts of manasources per type and color:"), 20 + leftTransition, 30);

            posY = 70;

            // Column titles
            for (int j = 0; j < Constants::MTG_NB_COLORS - 1; j++)
            {
                r->RenderQuad(mIcons[j], 52 + j * 15 + leftTransition, posY - 10, 0, 0.5, 0.5);
            }

            //font->DrawString(_("C"), 30 + leftTransition, posY-16);
            //font->DrawString(_("Ty"), 27 + leftTransition, posY-16);

            // Horizontal table lines
            r->DrawLine(27 + leftTransition, posY - 20, 60 + (Constants::MTG_NB_COLORS - 2) * 15 + leftTransition, posY - 20,
                    ARGB(128, 255, 255, 255));
            r->DrawLine(27 + leftTransition, posY - 1, 60 + (Constants::MTG_NB_COLORS - 2) * 15 + leftTransition, posY - 1,
                    ARGB(128, 255, 255, 255));
            r->DrawLine(27 + leftTransition, 2 * 10 + posY + 12, 60 + (Constants::MTG_NB_COLORS - 2) * 15 + leftTransition, 2 * 10
                    + posY + 12, ARGB(128, 255, 255, 255));
            r->DrawLine(27 + leftTransition, 3 * 10 + posY + 14, 60 + (Constants::MTG_NB_COLORS - 2) * 15 + leftTransition, 3 * 10
                    + posY + 14, ARGB(128, 255, 255, 255));

            // Vertical table lines
            r->DrawLine(26 + leftTransition, posY - 20, 26 + leftTransition, 3 * 10 + posY + 14, ARGB(128, 255, 255, 255));
            r->DrawLine(43 + leftTransition, posY - 20, 43 + leftTransition, 3 * 10 + posY + 14, ARGB(128, 255, 255, 255));
            r->DrawLine(60 + leftTransition + (Constants::MTG_NB_COLORS - 2) * 15, posY - 20, 60 + leftTransition
                    + (Constants::MTG_NB_COLORS - 2) * 15, 3 * 10 + posY + 14, ARGB(128, 255, 255, 255));

            font->DrawString(_("BL"), 27 + leftTransition, posY);
            font->DrawString(_("NB"), 27 + leftTransition, posY + 10);
            font->DrawString(_("O"), 30 + leftTransition, posY + 20);
            font->DrawString(_("T"), 30 + leftTransition, posY + 33);

            int curCount;

            for (int j = 0; j < Constants::MTG_NB_COLORS - 1; j++)
            {
                curCount = stw->countBasicLandsPerColor[j];
                sprintf(buffer, (curCount == 0 ? "." : "%i"), curCount);
                font->DrawString(buffer, 49 + leftTransition + j * 15, posY);

                curCount = stw->countLandsPerColor[j];
                sprintf(buffer, (curCount == 0 ? "." : "%i"), curCount);
                font->DrawString(buffer, 49 + leftTransition + j * 15, posY + 10);

                curCount = stw->countNonLandProducersPerColor[j];
                sprintf(buffer, (curCount == 0 ? "." : "%i"), curCount);
                font->DrawString(buffer, 49 + leftTransition + j * 15, posY + 20);

                curCount = stw->countLandsPerColor[j] + stw->countBasicLandsPerColor[j] + stw->countNonLandProducersPerColor[j];
                sprintf(buffer, (curCount == 0 ? "." : "%i"), curCount);
                font->DrawString(buffer, 49 + leftTransition + j * 15, posY + 33);
            }

            posY += 55;
            font->DrawString(_("BL - Basic lands"), 20 + leftTransition, posY);
            posY += 10;
            font->DrawString(_("NB - Non-basic lands"), 20 + leftTransition, posY);
            posY += 10;
            font->DrawString(_("O - Other (non-land) manasources"), 26 + leftTransition, posY);
            posY += 10;
            font->DrawString(_("T - Totals"), 26 + leftTransition, posY);

            break;

        case 6: // Land statistics - in symbols
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), stw->currentPage, _("Mana production - in mana symbols").c_str());
            font->DrawString(buffer, 10 + leftTransition, 10);
            font->DrawString(_("Total colored manasymbols in lands' production:"), 20 + leftTransition, 30);

            int totalProducedSymbols;
            totalProducedSymbols = 0;
            for (int i = 1; i < Constants::MTG_NB_COLORS - 1; i++)
            {
                totalProducedSymbols += stw->countLandsPerColor[i] + stw->countBasicLandsPerColor[i]; //!! Move to updatestats!
            }

            posY = 50;
            for (int i = 1; i < Constants::MTG_NB_COLORS - 1; i++)
            {
                if (stw->countLandsPerColor[i] + stw->countBasicLandsPerColor[i] > 0)
                {
                    sprintf(buffer, _("%i").c_str(), stw->countLandsPerColor[i] + stw->countBasicLandsPerColor[i]);
                    font->DrawString(buffer, 20 + leftTransition, posY);
                    sprintf(buffer, _("(%i%%)").c_str(), (int) (100 * (float) (stw->countLandsPerColor[i]
                            + stw->countBasicLandsPerColor[i]) / totalProducedSymbols));
                    font->DrawString(buffer, 33 + leftTransition, posY);
                    posX = 72;
                    for (int j = 0; j < stw->countLandsPerColor[i] + stw->countBasicLandsPerColor[i]; j++)
                    {
                        r->RenderQuad(mIcons[i], posX + leftTransition, posY + 6, 0, 0.5, 0.5);
                        posX += ((j + 1) % 10 == 0) ? 17 : 13;
                        if ((((j + 1) % 30) == 0) && (j < stw->countLandsPerColor[i] + stw->countBasicLandsPerColor[i] - 1))
                        {
                            posX = 72;
                            posY += 15;
                        }
                    }
                    posY += 17;
                }
            }

            break;

        case 2: // Mana cost detail
        case 3:
        case 4:
            int (*countPerCost)[Constants::STATS_MAX_MANA_COST + 1];
            int (*countPerCostAndColor)[Constants::STATS_MAX_MANA_COST + 1][Constants::MTG_NB_COLORS + 1];
            float avgCost;

            switch (stw->currentPage)
            { // Nested switch on the same variable. Oh yes.
            case 2: // Total counts
                // Title
                sprintf(buffer, STATS_TITLE_FORMAT.c_str(), stw->currentPage, _("Mana cost detail").c_str());
                font->DrawString(buffer, 10 + leftTransition, 10);
                font->DrawString(_("Card counts per mana cost:"), 20 + leftTransition, 30);
                avgCost = stw->avgManaCost;
                countPerCost = &stw->countCardsPerCost;
                countPerCostAndColor = &stw->countCardsPerCostAndColor;
                break;
            case 3: // Creature counts
                // Title
                sprintf(buffer, STATS_TITLE_FORMAT.c_str(), stw->currentPage, _("Mana cost detail - Creatures").c_str());
                font->DrawString(buffer, 10 + leftTransition, 10);
                font->DrawString(_("Creature counts per mana cost:"), 20 + leftTransition, 30);
                avgCost = stw->avgCreatureCost;
                countPerCost = &stw->countCreaturesPerCost;
                countPerCostAndColor = &stw->countCreaturesPerCostAndColor;
                break;
            case 4: // Spell counts
                // Title
                sprintf(buffer, STATS_TITLE_FORMAT.c_str(), stw->currentPage, _("Mana cost detail - Spells").c_str());
                font->DrawString(buffer, 10 + leftTransition, 10);
                font->DrawString(_("Non-creature spell counts per mana cost:"), 20 + leftTransition, 30);
                avgCost = stw->avgSpellCost;
                countPerCost = &stw->countSpellsPerCost;
                countPerCostAndColor = &stw->countSpellsPerCostAndColor;
                break;
            default:
                countPerCost = NULL;
                countPerCostAndColor = NULL;
                avgCost = 0;
                break;
            }

            posY = 70;

            // Column titles
            for (int j = 0; j < Constants::MTG_NB_COLORS - 1; j++)
            {
                r->RenderQuad(mIcons[j], 67 + j * 15 + leftTransition, posY - 10, 0, 0.5, 0.5);
            }

            font->DrawString(_("C"), 30 + leftTransition, posY - 16);
            font->DrawString(_("#"), 45 + leftTransition, posY - 16);

            // Horizontal table lines
            r->DrawLine(27 + leftTransition, posY - 20, 75 + (Constants::MTG_NB_COLORS - 2) * 15 + leftTransition, posY - 20,
                    ARGB(128, 255, 255, 255));
            r->DrawLine(27 + leftTransition, posY - 1, 75 + (Constants::MTG_NB_COLORS - 2) * 15 + leftTransition, posY - 1,
                    ARGB(128, 255, 255, 255));
            r->DrawLine(27 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12, 75 + (Constants::MTG_NB_COLORS - 2)
                    * 15 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12, ARGB(128, 255, 255, 255));

            // Vertical table lines
            r->DrawLine(26 + leftTransition, posY - 20, 26 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12,
                    ARGB(128, 255, 255, 255));
            r->DrawLine(41 + leftTransition, posY - 20, 41 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12,
                    ARGB(128, 255, 255, 255));
            r->DrawLine(58 + leftTransition, posY - 20, 58 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12,
                    ARGB(128, 255, 255, 255));
            r->DrawLine(75 + leftTransition + (Constants::MTG_NB_COLORS - 2) * 15, posY - 20, 75 + leftTransition
                    + (Constants::MTG_NB_COLORS - 2) * 15, Constants::STATS_MAX_MANA_COST * 10 + posY + 12,
                    ARGB(128, 255, 255, 255));

            for (int i = 0; i <= Constants::STATS_MAX_MANA_COST; i++)
            {
                sprintf(buffer, _("%i").c_str(), i);
                font->DrawString(buffer, 30 + leftTransition, posY);
                sprintf(buffer, ((*countPerCost)[i] > 0) ? _("%i").c_str() : ".", (*countPerCost)[i]);
                font->DrawString(buffer, 45 + leftTransition, posY);
                for (int j = 0; j < Constants::MTG_NB_COLORS - 1; j++)
                {
                    sprintf(buffer, ((*countPerCostAndColor)[i][j] > 0) ? _("%i").c_str() : ".", (*countPerCostAndColor)[i][j]);
                    font->DrawString(buffer, 64 + leftTransition + j * 15, posY);
                }
                r->FillRect(77.f + leftTransition + (Constants::MTG_NB_COLORS - 2) * 15.0f, posY + 2.0f, (*countPerCost)[i] * 5.0f,
                        8.0f, graphColor);
                posY += 10;
            }

            posY += 10;
            sprintf(buffer, _("Average converted mana cost: %2.2f").c_str(), avgCost);
            font->DrawString(buffer, 20 + leftTransition, posY);
            posY += 15;
            sprintf(buffer, _("C - Converted mana cost. Cards with cost>%i are included in the last row.").c_str(),
                    Constants::STATS_MAX_MANA_COST);
            font->DrawString(buffer, 20 + leftTransition, posY);
            posY += 10;
            font->DrawString(_("# - Total number of cards with given cost"), 20 + leftTransition, posY);

            break;

        case 8:
            // Title
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), stw->currentPage, _("Probabilities").c_str());
            font->DrawString(buffer, 10 + leftTransition, 10);

            // No lands detail
            float graphScale, graphWidth;
            graphWidth = 100;
            graphScale = (stw->noLandsProbInTurn[0] == 0) ? 0 : (graphWidth / stw->noLandsProbInTurn[0]);
            font->DrawString(_("No lands in first n cards:"), 20 + leftTransition, 30);

            posY = 50;
            for (int i = 0; i < Constants::STATS_FOR_TURNS; i++)
            {
                sprintf(buffer, _("%i:").c_str(), i + 7);
                font->DrawString(buffer, 30 + leftTransition, posY);
                sprintf(buffer, _("%2.2f%%").c_str(), stw->noLandsProbInTurn[i]);
                font->DrawString(buffer, 45 + leftTransition, posY);
                r->FillRect(84 + leftTransition, posY + 2, graphScale * stw->noLandsProbInTurn[i], 8, graphColor);
                posY += 10;
            }

            // No creatures probability detail
            posY += 10;
            font->DrawString(_("No creatures in first n cards:"), 20 + leftTransition, posY);
            posY += 20;
            graphScale = (stw->noCreaturesProbInTurn[0] == 0) ? 0 : (graphWidth / stw->noCreaturesProbInTurn[0]);

            for (int i = 0; i < Constants::STATS_FOR_TURNS; i++)
            {
                sprintf(buffer, _("%i:").c_str(), i + 7);
                font->DrawString(buffer, 30 + leftTransition, posY);
                sprintf(buffer, _("%2.2f%%").c_str(), stw->noCreaturesProbInTurn[i]);
                font->DrawString(buffer, 45 + leftTransition, posY);
                r->FillRect(84 + leftTransition, posY + 2, graphScale * stw->noCreaturesProbInTurn[i], 8, graphColor);
                posY += 10;
            }

            break;

        case 7: // Total mana cost per color
            // Title
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), stw->currentPage, _("Mana cost per color").c_str());
            font->DrawString(buffer, 10 + leftTransition, 10);

            font->DrawString(_("Total colored manasymbols in cards' casting costs:"), 20 + leftTransition, 30);

            posY = 50;
            for (int i = 1; i < Constants::MTG_NB_COLORS - 1; i++)
            {
                if (stw->totalCostPerColor[i] > 0)
                {
                    sprintf(buffer, _("%i").c_str(), stw->totalCostPerColor[i]);
                    font->DrawString(buffer, 20 + leftTransition, posY);
                    sprintf(buffer, _("(%i%%)").c_str(), (int) (100 * (float) stw->totalCostPerColor[i] / stw->totalColoredSymbols));
                    font->DrawString(buffer, 33 + leftTransition, posY);
                    posX = 72;
                    for (int j = 0; j < stw->totalCostPerColor[i]; j++)
                    {
                        r->RenderQuad(mIcons[i], posX + leftTransition, posY + 6, 0, 0.5, 0.5);
                        posX += ((j + 1) % 10 == 0) ? 17 : 13;
                        if ((((j + 1) % 30) == 0) && (j < stw->totalCostPerColor[i] - 1))
                        {
                            posX = 72;
                            posY += 15;
                        }
                    }
                    posY += 17;
                }
            }
            break;

        case 9: // Victory statistics
            // Title
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), stw->currentPage, _("Victory statistics").c_str());
            font->DrawString(buffer, 10 + leftTransition, 10);

            font->DrawString(_("Victories against AI:"), 20 + leftTransition, 30);

            sprintf(buffer, _("Games played: %i").c_str(), stw->gamesPlayed);
            font->DrawString(buffer, 20 + leftTransition, 45);
            sprintf(buffer, _("Victory ratio: %i%%").c_str(), stw->percentVictories);
            font->DrawString(buffer, 20 + leftTransition, 55);

            int AIsPerColumn = 19;
            posY = 70;
            posX = 20;

            // ToDo: Multiple pages when too many AI decks are present
            for (int i = 0; i < (int) stw->aiDeckStats.size(); i++)
            {
                sprintf(buffer, _("%.14s").c_str(), stw->aiDeckNames.at(i).c_str());
                font->DrawString(buffer, posX + (i < 2 * AIsPerColumn ? leftTransition : rightTransition), posY);
                sprintf(buffer, _("%i/%i").c_str(), stw->aiDeckStats.at(i)->victories, stw->aiDeckStats.at(i)->nbgames);
                font->DrawString(buffer, posX + (i < AIsPerColumn ? leftTransition : rightTransition) + 80, posY);
                sprintf(buffer, _("%i%%").c_str(), stw->aiDeckStats.at(i)->percentVictories());
                font->DrawString(buffer, posX + (i < AIsPerColumn ? leftTransition : rightTransition) + 110, posY);
                posY += 10;
                if (((i + 1) % AIsPerColumn) == 0)
                {
                    posY = 70;
                    posX += 155;
                }
            }
            break;
        }
    }
}


void GameStateDeckViewer::renderCard(int id, float rotation)
{
    WFont * mFont = resources.GetWFont(Fonts::MAIN_FONT);
    MTGCard * card = cardIndex[id];

    float max_scale = 0.96f;
    float x_center_0 = 180;
    float right_border = SCREEN_WIDTH - 20;

    float x_center = x_center_0 + cos((rotation + 8 - id) * M_PI / 12) * (right_border - x_center_0);
    float scale = max_scale / 1.12f * cos((x_center - x_center_0) * 1.5f / (right_border - x_center_0)) + 0.2f * max_scale * cos(
            cos((x_center - x_center_0) * 0.15f / (right_border - x_center_0)));
    float x = x_center; // ;

    float y = (SCREEN_HEIGHT_F) / 2.0f + SCREEN_HEIGHT_F * mSlide * (scale + 0.2f);

    int alpha = (int) (255 * (scale + 1.0 - max_scale));

    if (!card) return;
    JQuad * quad = NULL;

    int cacheError = CACHE_ERROR_NONE;

    if (!options[Options::DISABLECARDS].number)
    {
        quad = resources.RetrieveCard(card, RETRIEVE_EXISTING);
        cacheError = resources.RetrieveError();
        if (!quad && cacheError != CACHE_ERROR_404)
        {
            if (last_user_activity > (abs(2 - id) + 1) * NO_USER_ACTIVITY_SHOWCARD_DELAY)
                quad = resources.RetrieveCard(card);
            else
            {
                quad = backQuad;
            }
        }
    }

    int quadAlpha = alpha;
    if (!displayed_deck->count(card)) quadAlpha /= 2;
    if (quad)
    {
        if (quad == backQuad)
        {
            quad->SetColor(ARGB(255,255,255,255));
            float _scale = scale * (285 / quad->mHeight);
            JRenderer::GetInstance()->RenderQuad(quad, x, y, 0.0f, _scale, _scale);
        }
        else
        {
            Pos pos = Pos(x, y, scale * 285 / 250, 0.0, 255);
            CardGui::DrawCard(card, pos);
        }
    }
    else
    {
        Pos pos = Pos(x, y, scale * 285 / 250, 0.0, 255);
        CardGui::DrawCard(card, pos, DrawMode::kText);
        if (!options[Options::DISABLECARDS].number) quad = resources.RetrieveCard(card, CACHE_THUMB);
        if (quad)
        {
            float _scale = 285 * scale / quad->mHeight;
            quad->SetColor(ARGB(40,255,255,255));
            JRenderer::GetInstance()->RenderQuad(quad, x, y, 0, _scale, _scale);
        }
    }
    quadAlpha = 255 - quadAlpha;
    if (quadAlpha > 0)
    {
        JRenderer::GetInstance()->FillRect(x - scale * 100.0f, y - scale * 142.5f, scale * 200.0f, scale * 285.0f,
                ARGB(quadAlpha,0,0,0));
    }
    if (last_user_activity < 3)
    {
        int fontAlpha = alpha;
        float qtY = y - 135 * scale;
        float qtX = x + 40 * scale;
        char buffer[4096];
        sprintf(buffer, "x%i", displayed_deck->count(card));
        WFont * font = mFont;
        font->SetColor(ARGB(fontAlpha/2,0,0,0));
        JRenderer::GetInstance()->FillRect(qtX, qtY, font->GetStringWidth(buffer) + 6, 16, ARGB(fontAlpha/2,0,0,0));
        font->DrawString(buffer, qtX + 4, qtY + 4);
        font->SetColor(ARGB(fontAlpha,255,255,255));
        font->DrawString(buffer, qtX + 2, qtY + 2);
        font->SetColor(ARGB(255,255,255,255));
    }
}

void GameStateDeckViewer::renderCard(int id)
{
    renderCard(id, 0);
}

void GameStateDeckViewer::Render()
{

    WFont * mFont = resources.GetWFont(Fonts::MAIN_FONT);

    JRenderer * r = JRenderer::GetInstance();
    r->ClearScreen(ARGB(0,0,0,0));
    if (displayed_deck == myDeck && mStage != STAGE_MENU) renderDeckBackground();
    int order[3] = { 1, 2, 3 };
    if (mRotation < 0.5 && mRotation > -0.5)
    {
        order[1] = 3;
        order[2] = 2;
    }
    else if (mRotation < -0.5)
    {
        order[0] = 3;
        order[2] = 1;
    }

    renderCard(6, mRotation);
    renderCard(5, mRotation);
    renderCard(4, mRotation);
    renderCard(0, mRotation);

    for (int i = 0; i < 3; i++)
    {
        renderCard(order[i], mRotation);
    }

    if (displayed_deck->Size() > 0)
    {
        renderSlideBar();
    }
    else
    {
        mFont->DrawString(_("No Card"), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, JGETEXT_CENTER);
    }
    if (mStage == STAGE_ONSCREEN_MENU)
    {
        renderOnScreenMenu();
    }
    else if (mStage == STAGE_WELCOME)
    {
        welcome_menu->Render();
    }
    else
    {
        renderOnScreenBasicInfo();

    }
    if (mStage == STAGE_MENU)
    {
        menu->Render();
    }
    if (subMenu) subMenu->Render();

    if (filterMenu && !filterMenu->isFinished()) filterMenu->Render();

    if (options.keypadActive()) options.keypadRender();

}

int GameStateDeckViewer::loadDeck(int deckid)
{

    if (!stw) stw = new StatsWrapper(deckid);

    stw->currentPage = 0;
    stw->pageCount = 9;
    stw->needUpdate = true;

    if (!playerdata) playerdata = NEW PlayerData(mParent->collection);
    SAFE_DELETE(myCollection);
    myCollection = NEW DeckDataWrapper(playerdata->collection);
    myCollection->Sort(WSrcCards::SORT_ALPHA);
    displayed_deck = myCollection;

    char deckname[256];
    sprintf(deckname, "deck%i.txt", deckid);
    if (myDeck)
    {
        SAFE_DELETE(myDeck->parent);
        SAFE_DELETE(myDeck);
    }
    myDeck = NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(deckname, "", false, false).c_str(), mParent->collection));

    // Check whether the cards in the deck are actually available in the player's collection:
    int cheatmode = options[Options::CHEATMODE].number;
    bool bPure = true;
    for (int i = 0; i < myDeck->Size(true); i++)
    {
        MTGCard * current = myDeck->getCard(i, true);
        int howmanyinDeck = myDeck->count(current);
        for (int i = myCollection->count(current); i < howmanyinDeck; i++)
        {
            bPure = false;
            if (cheatmode)
            { //Are we cheating?
                playerdata->collection->add(current); //Yup, add it to collection permanently.
                myCollection->Add(current);
            }
            else
            {
                myDeck->Remove(current); //Nope. Remove it from deck.
                break;
            }
        }

        myCollection->Remove(current, myDeck->count(current));
    }
    if (!bPure)
    {
        myDeck->validate();
        myCollection->validate();
    }

    myDeck->Sort(WSrcCards::SORT_ALPHA);
    SAFE_DELETE(filterMenu);
    rebuildFilters();
    loadIndexes();
    return 1;
}

void GameStateDeckViewer::ButtonPressed(int controllerId, int controlId)
{
    int deckIdNumber = controlId;
    int deckListSize = 0;
    string defaultAiName;
    DeckManager *deckManager = DeckManager::GetInstance();
    vector<DeckMetaData *> * deckList;
    switch (controllerId)
    {
    case MENU_DECK_SELECTION: //Deck menu
        if (controlId == MENU_ITEM_CANCEL)
        {
            if (!mSwitching)
                mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
            else
                mStage = STAGE_WAITING;

            mSwitching = false;
            break;
        }
        else if (controlId == MENUITEM_MORE_INFO)
        {
            break;
        }
        else if (controlId == MENU_ITEM_CHEAT_MODE)
        { // (PSY) Cheatmode: Complete the collection
            playerdata->collection->complete(); // Add the cards
            playerdata->collection->save(); // Save the new collection
            for (int i = 0; i < setlist.size(); i++)
            { // Update unlocked sets
                GameOptionAward * goa = dynamic_cast<GameOptionAward*> (&options[Options::optionSet(i)]);
                if (goa) goa->giveAward();
            }
            options.save();
            SAFE_DELETE(myCollection);
            myCollection = NEW DeckDataWrapper(playerdata->collection);
            myCollection->Sort(WSrcCards::SORT_ALPHA);
            displayed_deck = myCollection;
            rebuildFilters();
            loadIndexes();
            mStage = STAGE_WELCOME;
            break;
        }
        mStage = STAGE_WAITING;
        deckList = deckManager->getPlayerDeckOrderList();
        deckListSize = deckList->size();

        if (controlId == MENU_ITEM_NEW_DECK) // new deck option selected
            deckIdNumber = deckList->size() + 1;
        else if (deckListSize > 0 && controlId <= deckListSize)
            deckIdNumber = deckList->at(controlId - 1)-> getDeckId();
        else
            deckIdNumber = controlId;

        loadDeck(deckIdNumber);
        mStage = STAGE_WAITING;
        deckNum = controlId;
        break;

    case MENU_DECK_BUILDER: //Save / exit menu
        switch (controlId)
        {

        case MENU_ITEM_SAVE_RETURN_MAIN_MENU:
            saveDeck();
            mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
            break;

        case MENU_ITEM_SAVE_RENAME:
            if (myDeck && myDeck->parent)
            {
                options.keypadStart(myDeck->parent->meta_name, &newDeckname);
                options.keypadTitle("Rename deck");
            }
            break;

        case MENU_ITEM_SAVE_AS_AI_DECK:
            // find the next unused ai deck number
            // warn user that once saved, no edits can be made
            // save entire collection to ai as spelled out card with count
            // bring user to main deck editor menu.
            isAIDeckSave = true;
            defaultAiName = myDeck && myDeck->parent ? myDeck->parent->meta_name : "Custom AI Deck";
            options.keypadStart(defaultAiName, &newDeckname);
            options.keypadTitle("Name Custom AI Deck");
            updateDecks();
            mStage = STAGE_WELCOME;
            mSwitching = true;
            break;

        case MENU_ITEM_SWITCH_DECKS_NO_SAVE:
            updateDecks();
            mStage = STAGE_WELCOME;
            mSwitching = true;
            break;
        case MENU_ITEM_MAIN_MENU:
            mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
            break;
        case MENU_ITEM_EDITOR_CANCEL:
            mStage = STAGE_WAITING;
            break;
        case MENU_ITEM_FILTER_BY:
            mStage = STAGE_FILTERS;
            if (!filterMenu) rebuildFilters();
            filterMenu->Entering(JGE_BTN_NONE);
            break;
        }
        break;

    case MENU_CARD_PURCHASE: // Yes/ No sub menu.
        switch (controlId)
        {
        case MENU_ITEM_YES:
        {
            MTGCard * card = cardIndex[2];
            if (card)
            {
                int rnd = (rand() % 25);
                playerdata->credits += price;
                price = price - (rnd * price) / 100;
                pricelist->setPrice(card->getMTGId(), price);
                playerdata->collection->remove(card->getMTGId());
                displayed_deck->Remove(card, 1);
                displayed_deck->validate();
                stw->needUpdate = true;
                loadIndexes();
            }
        }
        case MENU_ITEM_NO:
            subMenu->Close();
            break;
        }
    }
}

