/*
 * GameStateDeckViewer.cpp
 * Class handling the Deck Editor
 */

#include "PrecompiledHeader.h"

#include <math.h>
#include <iomanip>

#include "GameStateDuel.h"
#include "GameStateDeckViewer.h"
#include "Translate.h"
#include "ManaCostHybrid.h"
#include "MTGCardInstance.h"
#include "WFilter.h"
#include "WDataSrc.h"
#include "DeckManager.h"
#include "DeckMetaData.h"
#include "DeckEditorMenu.h"
#include "SimpleMenu.h"
#include "utils.h"
#include "AIPlayer.h"
#include "GameApp.h"

#include "CarouselDeckView.h"
#include "GridDeckView.h"

#define NO_USER_ACTIVITY_HELP_DELAY 10

static std::string kBgFile = "";

GameStateDeckViewer::GameStateDeckViewer(GameApp* parent) :
    GameState(parent, "deckeditor"), mView(NULL), mCurrentView(CAROUSEL_VIEW)
{
    bgMusic = NULL;
    isAIDeckSave = false;
    mSwitching = false;
    welcome_menu = NULL;
    myCollection = NULL;
    myDeck = NULL;
    mySideboard = NULL;
    myCommandZone = NULL;
    myDungeonZone = NULL;
    filterMenu = NULL;
    source = NULL;
    hudAlpha = 0;
    subMenu = NULL;
    sbMenu = NULL;
    deckMenu = NULL;
    mStatsWrapper = NULL;
    
    statsPrevButton = NEW InteractiveButton(NULL, kPrevStatsButtonId, Fonts::MAIN_FONT, "Stats",  SCREEN_WIDTH_F - 35, SCREEN_HEIGHT_F - 20, JGE_BTN_PREV);
    toggleDeckButton = NEW InteractiveButton(NULL, kToggleDeckActionId, Fonts::MAIN_FONT, "View Deck", 10, SCREEN_HEIGHT_F - 20, JGE_BTN_PRI);
    sellCardButton = NEW InteractiveButton(NULL, kSellCardActionId, Fonts::MAIN_FONT, "Sell Card", (SCREEN_WIDTH_F/ 2) - 125, SCREEN_HEIGHT_F - 20, JGE_BTN_SEC);
    sb_cmd_dng_Button = NEW InteractiveButton(NULL, kSBActionId, Fonts::MAIN_FONT, "View SB", (SCREEN_WIDTH_F/ 2) - 35, SCREEN_HEIGHT_F - 20, JGE_BTN_CTRL);
    filterButton = NEW InteractiveButton(NULL, kFilterButtonId, Fonts::MAIN_FONT, "Filter", (SCREEN_WIDTH_F - 116), SCREEN_HEIGHT_F - 20, JGE_BTN_SOUND);
    //TODO: Check if that button is available:
    toggleViewButton = NEW InteractiveButton(NULL, kSwitchViewButton, Fonts::MAIN_FONT, "Grid", (SCREEN_WIDTH_F/ 2) + 50, SCREEN_HEIGHT_F - 20, JGE_BTN_MAX);
    toggleUpButton = NEW InteractiveButton(NULL, kToggleUpButton, Fonts::MAIN_FONT, "UP", 10, 25, JGE_BTN_DOWN);
    toggleDownButton = NEW InteractiveButton(NULL, kToggleDownButton, Fonts::MAIN_FONT, "DN", SCREEN_WIDTH_F-25, 25, JGE_BTN_UP);
    toggleLeftButton = NEW InteractiveButton(NULL, kToggleLeftButton, Fonts::MAIN_FONT, "<<", 10, SCREEN_HEIGHT_F/2, JGE_BTN_LEFT);
    toggleRightButton = NEW InteractiveButton(NULL, kToggleRightButton, Fonts::MAIN_FONT, ">>", SCREEN_WIDTH_F-20, SCREEN_HEIGHT_F/2, JGE_BTN_RIGHT);
}

GameStateDeckViewer::~GameStateDeckViewer()
{
    SAFE_DELETE(bgMusic);
    SAFE_DELETE(toggleDeckButton);
    SAFE_DELETE(sellCardButton);
    SAFE_DELETE(sb_cmd_dng_Button);
    SAFE_DELETE(statsPrevButton);
    SAFE_DELETE(filterButton);
    SAFE_DELETE(toggleViewButton);
    SAFE_DELETE(mView);
    SAFE_DELETE(toggleUpButton);
    SAFE_DELETE(toggleDownButton);
    SAFE_DELETE(toggleLeftButton);
    SAFE_DELETE(toggleRightButton);
    
    if (myDeck)
    {
        SAFE_DELETE(myDeck->parent);
        SAFE_DELETE(myDeck);
    }
    if (mySideboard)
    {
        SAFE_DELETE(mySideboard->parent);
        SAFE_DELETE(mySideboard);
    }
    if (myCommandZone)
    {
        SAFE_DELETE(myCommandZone->parent);
        SAFE_DELETE(myCommandZone);
    }
    if (myDungeonZone)
    {
        SAFE_DELETE(myDungeonZone->parent);
        SAFE_DELETE(myDungeonZone);
    }
    if (myCollection)
    {
        SAFE_DELETE(myCollection->parent);
        SAFE_DELETE(myCollection);
    }
    SAFE_DELETE(filterMenu);
    kBgFile = ""; //Reset the chosen backgorund.
}

void GameStateDeckViewer::rebuildFilters()
{
    if (!filterMenu) filterMenu = NEW WGuiFilters("Filter by...", NULL);
    if (source)
        SAFE_DELETE(source);
    source = NEW WSrcDeckViewer(myDeck, myCollection);
    filterMenu->setSrc(source);
    if (mView->deck() != myDeck) source->swapSrc();
    filterMenu->Finish(true);

    // no stats need updating if there isn't a deck to update
    if (mStatsWrapper && myDeck)
        mStatsWrapper->updateStats( myDeck );;
}

void GameStateDeckViewer::updateFilters()
{
    if (!mView->deck() || !filterMenu) return;

    filterMenu->recolorFilter(mView->filter() - 1);
    filterMenu->Finish(true);
    mStatsWrapper->updateStats( myDeck );;
    return;
}

void GameStateDeckViewer::toggleCollection()
{
    if(mView->deck() == mySideboard || mView->deck() == myCommandZone || mView->deck() == myDungeonZone)
        return;

    if (mView->deck() == myCollection)
    {
        toggleDeckButton->setText("Collection");
        mView->SetDeck(myDeck);
    }
    else
    {
        toggleDeckButton->setText("View Deck");
        mView->SetDeck(myCollection);
    }
    source->swapSrc();
    updateFilters();
}

void GameStateDeckViewer::toggleSB_CMD_DNG()
{
    if(mView->deck() == myDeck)
        return;

    if (mView->deck() == myCollection)
    {
        mView->SetDeck(mySideboard);
        sb_cmd_dng_Button->setText("View CMD");
    }
    else if (mView->deck() == mySideboard)
    {
        mView->SetDeck(myCommandZone);
        sb_cmd_dng_Button->setText("View DNG");
    }
    else if (mView->deck() == myCommandZone)
    {
        mView->SetDeck(myDungeonZone);
        sb_cmd_dng_Button->setText("View Coll.");
    }
    else
    {
        mView->SetDeck(myCollection);
        sb_cmd_dng_Button->setText("View SB");
    }
    //source->swapSrc();
    //updateFilters();
}

//after renaming and on the first start.
//reloadWelcomeMenu
void GameStateDeckViewer::updateDecks()
{
    SAFE_DELETE(welcome_menu);
    welcome_menu = NEW DeckEditorMenu(MENU_DECK_SELECTION, this, Fonts::OPTION_FONT, "Choose Deck To Edit");
    vector<DeckMetaData *> playerDeckList = fillDeckMenu(welcome_menu, options.profileFile(), "", NULL, 0, GAME_TYPE_CLASSIC, true); // Show all decks in deck editor menu...

    newDeckname = "";
    welcome_menu->Add(MENU_ITEM_NEW_DECK, "--NEW--");
    if (options[Options::CHEATMODE].number && (!myCollection || myCollection->getCount(WSrcDeck::UNFILTERED_MIN_COPIES) < 4))
    {
        welcome_menu->Add(MENU_ITEM_CHEAT_MODE, "--UNLOCK CARDS--");
    }
    welcome_menu->Add(MENU_ITEM_CANCEL, "Cancel");

    // update the deckmanager with the latest information
    DeckManager::GetInstance()->updateMetaDataList(&playerDeckList, false);
}

void GameStateDeckViewer::buildEditorMenu()
{
    SAFE_DELETE(deckMenu);

    deckMenu = NEW DeckEditorMenu(MENU_DECK_BUILDER, this, Fonts::OPTION_FONT, "Deck Editor", myDeck, mStatsWrapper);

    deckMenu->Add(MENU_ITEM_FILTER_BY, _("Filter By..."), _("Narrow down the list of cards. "));
    deckMenu->Add(MENU_ITEM_SWITCH_DECKS_NO_SAVE, _("Switch Decks"), _("No changes. View another deck."));
    deckMenu->Add(MENU_ITEM_SAVE_RENAME, _("Rename Deck"), _("Change the name of the deck"));
    deckMenu->Add(MENU_ITEM_SAVE_RETURN_MAIN_MENU, _("Save & Quit Editor"), _("Save changes. Return to the main menu"));
    deckMenu->Add(MENU_ITEM_SAVE_AS_AI_DECK, _("Save As AI Deck"), _("All changes are final."));
    deckMenu->Add(MENU_ITEM_MAIN_MENU, _("Quit Editor"), _("No changes. Return to the main menu."));
    deckMenu->Add(MENU_ITEM_TOGGLE_VIEW, _("Toggle View"), _("Toggle view grid/carousel."));
    deckMenu->Add(MENU_ITEM_EDITOR_CANCEL, _("Cancel"), _("Close menu."));
}

void GameStateDeckViewer::Start()
{
    hudAlpha = 0;
    mSwitching = false;
    subMenu = NULL;
    sbMenu = NULL;
    myDeck = NULL;
    mySideboard = NULL;
    myCommandZone = NULL;
    myDungeonZone = NULL;
    mStage = STAGE_WELCOME;

    last_user_activity = NO_USER_ACTIVITY_HELP_DELAY + 1;
    onScreenTransition = 0;

    pricelist = NEW PriceList("settings/prices.dat", MTGCollection());
    playerdata = NEW PlayerData(MTGCollection());
    myCollection = NEW DeckDataWrapper(playerdata->collection);
    myCollection->Sort(WSrcCards::SORT_ALPHA);
    setupView(mCurrentView, myCollection);
    toggleDeckButton->setText("View Deck");

    //Icons
    mIcons = manaIcons;
    for (int i = 0; i < Constants::NB_Colors; i++)
    {
        mIcons[i]->SetHotSpot(16, 16);
    }

    //Grab a texture in VRAM.
    pspIconsTexture = WResourceManager::Instance()->RetrieveTexture("iconspsp.png", RETRIEVE_MANAGE);

    char buf[512];
    for (int i = 0; i < 8; i++)
    {
        sprintf(buf, "iconspsp%d", i);
        pspIcons[i] = WResourceManager::Instance()->RetrieveQuad("iconspsp.png", (float) i * 32, 0, 32, 32, buf);
        pspIcons[i]->SetHotSpot(16, 16);
    }
#ifndef TOUCH_ENABLED
    toggleDeckButton->setImage( pspIcons[6] );
    sellCardButton->setImage( pspIcons[7] );
#endif
    
    //init welcome menu
    updateDecks();

    GameApp::playMusic("Track1.mp3");

    mEngine->ResetInput();
    JRenderer::GetInstance()->EnableVSync(true);
}

void GameStateDeckViewer::End()
{
    JRenderer::GetInstance()->EnableVSync(false);

    SAFE_DELETE(welcome_menu);
    SAFE_DELETE(deckMenu);
    SAFE_DELETE(subMenu);
    SAFE_DELETE(sbMenu);

    WResourceManager::Instance()->Release(pspIconsTexture);
    if (myCollection)
    {
        SAFE_DELETE(myCollection);
    }
    if (myDeck)
    {
        SAFE_DELETE(myDeck->parent);
        SAFE_DELETE(myDeck);
    }
    if (mySideboard)
    {
        SAFE_DELETE(mySideboard->parent);
        SAFE_DELETE(mySideboard);
    }
    if (myCommandZone)
    {
        SAFE_DELETE(myCommandZone->parent);
        SAFE_DELETE(myCommandZone);
    }
    if (myDungeonZone)
    {
        SAFE_DELETE(myDungeonZone->parent);
        SAFE_DELETE(myDungeonZone);
    }
    SAFE_DELETE(pricelist);
    SAFE_DELETE(playerdata);
    SAFE_DELETE(filterMenu);
    SAFE_DELETE(source);
    kBgFile = ""; //Reset the chosen backgorund.
}

void GameStateDeckViewer::addRemove(MTGCard * card)
{
    if (!card) return;
    if ((card->getRarity() == Constants::RARITY_T) || (card->getId() < 1)) return;
    if (mView->deck()->Remove(card, 1, (mView->deck() == myDeck)))
    {
        if (mView->deck() == myCollection)
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
    mStatsWrapper->needUpdate = true;
    mView->reloadIndexes();
}

void GameStateDeckViewer::SBaddRemove(MTGCard * card)
{
    if (!card) return;
    if ((card->getRarity() == Constants::RARITY_T) || (card->getId() < 1)) return;
    if (mView->deck()->Remove(card, 1, (mView->deck() == mySideboard)))
    {
        if (mView->deck() == myCollection)
        {
            mySideboard->Add(card);
            mySideboard->Sort(WSrcCards::SORT_ALPHA);
        }
        else
        {
            myCollection->Add(card);
        }
    }
    myCollection->validate();
    mySideboard->validate();
    mStatsWrapper->needUpdate = true;
    mView->reloadIndexes();
}

void GameStateDeckViewer::CMDaddRemove(MTGCard * card)
{
    if (!card) return;
    if ((card->getRarity() == Constants::RARITY_T) || (card->getId() < 1)) return;
    if (mView->deck()->Remove(card, 1, (mView->deck() == myCommandZone)))
    {
        if (mView->deck() == myCollection)
        {
            myCommandZone->Add(card);
            myCommandZone->Sort(WSrcCards::SORT_ALPHA);
        }
        else
        {
            myCollection->Add(card);
        }
    }
    myCollection->validate();
    myCommandZone->validate();
    mStatsWrapper->needUpdate = true;
    mView->reloadIndexes();
}

void GameStateDeckViewer::DNGaddRemove(MTGCard * card)
{
    if (!card) return;
    if ((card->getRarity() != Constants::RARITY_T) || (card->getId() < 1)) return;
    if (mView->deck()->Remove(card, 1, (mView->deck() == myDungeonZone)))
    {
        if (mView->deck() == myCollection)
        {
            myDungeonZone->Add(card);
            myDungeonZone->Sort(WSrcCards::SORT_ALPHA);
        }
        else
        {
            myCollection->Add(card);
        }
    }
    myCollection->validate();
    myDungeonZone->validate();
    mStatsWrapper->needUpdate = true;
    mView->reloadIndexes();
}

void GameStateDeckViewer::choiceAddRemove(MTGCard * card)
{
    if (!card) return;
    if ((card->getRarity() == Constants::RARITY_T) || (card->getId() < 1)) return;
    last_user_activity = 0;
    const float menuXOffset = SCREEN_WIDTH_F - 300;
    const float menuYOffset = SCREEN_HEIGHT_F / 2;
    SAFE_DELETE(sbMenu);
    {
        if (mView->deck() == myCollection)
        {
            sbMenu = NEW SimpleMenu(JGE::GetInstance(), WResourceManager::Instance(), SBMENU_CHOICE, this, Fonts::MAIN_FONT, menuXOffset, menuYOffset, "Add/Remove Cards");
            if(!card->data->hasType("Dungeon")){
                sbMenu->Add(SBMENU_ADD_NORMAL, "Add to Deck");
                sbMenu->Add(SBMENU_ADD_SB, "Add to Sideboard");
                if(card->data->hasType("Legendary") && (card->data->hasType("Creature") || card->data->basicAbilities[Constants::CANBECOMMANDER] || card->data->hasType("Background")))
                    sbMenu->Add(SBMENU_ADD_CMD, "Choose as Commander");
            } else
                sbMenu->Add(SBMENU_ADD_DNG, "Add to Dungeons");
            sbMenu->Add(SBMENU_ADD_CANCEL, "Cancel");
        }
        else if (mView->deck() == myCommandZone)
        {
            sbMenu = NEW SimpleMenu(JGE::GetInstance(), WResourceManager::Instance(), SBMENU_CHOICE, this, Fonts::MAIN_FONT, menuXOffset, menuYOffset, "Add/Remove Cards");
            sbMenu->Add(SBMENU_ADD_CMD, "Remove as Commander");
            sbMenu->Add(SBMENU_ADD_CANCEL, "Cancel");
        }
        else if (mView->deck() == mySideboard)
        {
            sbMenu = NEW SimpleMenu(JGE::GetInstance(), WResourceManager::Instance(), SBMENU_CHOICE, this, Fonts::MAIN_FONT, menuXOffset, menuYOffset, "Add/Remove Cards");
            sbMenu->Add(SBMENU_ADD_SB, "Remove from Sideboard");
            sbMenu->Add(SBMENU_ADD_CANCEL, "Cancel");
        }
        else if (mView->deck() == myDungeonZone)
        {
            sbMenu = NEW SimpleMenu(JGE::GetInstance(), WResourceManager::Instance(), SBMENU_CHOICE, this, Fonts::MAIN_FONT, menuXOffset, menuYOffset, "Add/Remove Cards");
            sbMenu->Add(SBMENU_ADD_DNG, "Remove from Dungeons");
            sbMenu->Add(SBMENU_ADD_CANCEL, "Cancel");
        }
        else if (mView->deck() == myDeck)
        {
            sbMenu = NEW SimpleMenu(JGE::GetInstance(), WResourceManager::Instance(), SBMENU_CHOICE, this, Fonts::MAIN_FONT, menuXOffset, menuYOffset, "Add/Remove Cards");
            sbMenu->Add(SBMENU_ADD_NORMAL, "Remove Card");
            sbMenu->Add(SBMENU_ADD_CANCEL, "Cancel");
        }
    }
    mStatsWrapper->needUpdate = true;
}

void GameStateDeckViewer::insertSideBoard()
{
    if(mySideboard->getCount())
    {
        vector<string> newSB;
        for (int i = 0; i < mySideboard->Size(true); i++)
        {
            MTGCard * current = mySideboard->getCard(i, true);
            int howmanyinDeck = mySideboard->count(current);
            for (int i = 0; i < howmanyinDeck; i++)
            {
                stringstream cid;
                cid << current->getMTGId();
                newSB.push_back(cid.str());
            }
        }
        myDeck->parent->replaceSB(newSB);
    }
}

void GameStateDeckViewer::insertCommandZone()
{
    if(myCommandZone->getCount())
    {
        vector<string> newCMD;
        for (int i = 0; i < myCommandZone->Size(true); i++)
        {
            MTGCard * current = myCommandZone->getCard(i, true);
            int howmanyinDeck = myCommandZone->count(current);
            for (int i = 0; i < howmanyinDeck; i++)
            {
                stringstream cid;
                cid << current->getMTGId();
                newCMD.push_back(cid.str());
            }
        }
        myDeck->parent->replaceCMD(newCMD);
    }
}

void GameStateDeckViewer::insertDungeonZone()
{
    if(myDungeonZone->getCount())
    {
        vector<string> newDNG;
        for (int i = 0; i < myDungeonZone->Size(true); i++)
        {
            MTGCard * current = myDungeonZone->getCard(i, true);
            int howmanyinDeck = myDungeonZone->count(current);
            for (int i = 0; i < howmanyinDeck; i++)
            {
                stringstream cid;
                cid << current->getMTGId();
                newDNG.push_back(cid.str());
            }
        }
        myDeck->parent->replaceDNG(newDNG);
    }
}

void GameStateDeckViewer::saveDeck()
{
    //update the corresponding meta data object
    DeckMetaData *metaData = DeckManager::GetInstance()->getDeckMetaDataById( myDeck->parent->meta_id, false );
    if ( newDeckname.length() > 0 )
        metaData->setDeckName( newDeckname );
    mSwitching = true;
    //insert sideboards to mydeck parents
    insertSideBoard();
    //insert commanders to mydeck parents
    insertCommandZone();
    //insert dungeons to mydeck parents
    insertDungeonZone();
    //save deck
    myDeck->save();
    playerdata->save();
    pricelist->save();
}

/**
 save the deck in a readable format to allow people to edit the file offline
 */
void GameStateDeckViewer::saveAsAIDeck(string deckName)
{

    int deckId = AIPlayer::getTotalAIDecks() + 1;

    std::ostringstream oss;
    oss << "deck" <<deckId;
    string aiDeckName = oss.str();
    oss.str("");
    if (myDeck->parent->meta_desc == "")
        oss << endl << "Can you beat your own creations?" << endl << "User created AI Deck # " << deckId;
    else
        oss << myDeck->parent->meta_desc;
    string deckDesc = oss.str();
    string filepath = "ai/baka/";
    filepath.append(aiDeckName).append(".txt");
    DebugTrace("saving AI deck " << filepath);
    myDeck->save(filepath, true, deckName, deckDesc);
    AIPlayer::invalidateTotalAIDecks(); //We added one AI deck, so we need to invalidate the count cache
}

void GameStateDeckViewer::sellCard()
{
    last_user_activity = 0;
    SAFE_DELETE(subMenu);
    char buffer[4096];
    {
        MTGCard * card = mView->getActiveCard();
        if (card && mView->deck()->count(card))
        {
            int price = pricelist->getSellPrice(card);
            sprintf(buffer, "%s : %i %s", _(card->data->getName()).c_str(), price, _("credits").c_str());
            const float menuXOffset = SCREEN_WIDTH_F - 300;
            const float menuYOffset = SCREEN_HEIGHT_F / 2;
            subMenu = NEW SimpleMenu(JGE::GetInstance(), WResourceManager::Instance(), MENU_CARD_PURCHASE, this, Fonts::MAIN_FONT, menuXOffset, menuYOffset, buffer);
            subMenu->Add(MENU_ITEM_YES, "Yes");
            subMenu->Add(MENU_ITEM_NO, "No", "", true);
        }
    }
    mStatsWrapper->needUpdate = true;
}

bool GameStateDeckViewer::userPressedButton()
{
    return ((toggleDeckButton->ButtonPressed())
            || (sellCardButton->ButtonPressed())
            || (sb_cmd_dng_Button->ButtonPressed())
            || (statsPrevButton->ButtonPressed())
            || (filterButton->ButtonPressed())
            || (toggleViewButton->ButtonPressed())
            || (toggleUpButton->ButtonPressed())
            || (toggleDownButton->ButtonPressed())
            || (toggleLeftButton->ButtonPressed())
            || (toggleRightButton->ButtonPressed())
            );
}

void GameStateDeckViewer::setButtonState(bool state)
{
    toggleDeckButton->setIsSelectionValid(state);
    sellCardButton->setIsSelectionValid(state);
    sb_cmd_dng_Button->setIsSelectionValid(state);
    statsPrevButton->setIsSelectionValid(state);
    filterButton->setIsSelectionValid(state);
    toggleViewButton->setIsSelectionValid(state);
    toggleUpButton->setIsSelectionValid(state);
    toggleDownButton->setIsSelectionValid(state);
    toggleLeftButton->setIsSelectionValid(state);
    toggleRightButton->setIsSelectionValid(state);
}

void GameStateDeckViewer::RenderButtons()
{
    if(mView->deck() != mySideboard && mView->deck() != myCommandZone && mView->deck() != myDungeonZone)
        toggleDeckButton->Render();
    sellCardButton->Render();
    if(mView->deck() != myDeck)
        sb_cmd_dng_Button->Render();
    if(mView->deck() != mySideboard && mView->deck() != myCommandZone && mView->deck() != myDungeonZone)
        filterButton->Render();
    statsPrevButton->Render();
    toggleViewButton->Render();
    toggleUpButton->Render();
    toggleDownButton->Render();
    toggleLeftButton->Render();
    toggleRightButton->Render();
}

void GameStateDeckViewer::setupView(GameStateDeckViewer::AvailableView view, DeckDataWrapper *deck)
{
    SAFE_DELETE(mView);

    if(view == CAROUSEL_VIEW) mView = NEW CarouselDeckView();
    else if(view == GRID_VIEW) mView = NEW GridDeckView();

    mView->SetDeck(deck);
    updateFilters();
}

void GameStateDeckViewer::toggleView()
{
    if(mCurrentView == CAROUSEL_VIEW)
    {
        mCurrentView = GRID_VIEW;
        toggleViewButton->setText("Carousel");
    }
    else
    {
        mCurrentView = CAROUSEL_VIEW;
        toggleViewButton->setText("Grid");
    }
    setupView(mCurrentView, mView->deck());
}

void GameStateDeckViewer::Update(float dt)
{   
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
                    updateDecks();
                }
                mStage = STAGE_WAITING;
            }
            newDeckname = "";
        }
        //Prevent screen from updating.
        return;
    }
    hudAlpha = 255 - (int)(MAX(last_user_activity-2.0f, 0) * 500);
    if (hudAlpha < 0) hudAlpha = 0;
    if (subMenu)
    {
        subMenu->Update(dt);
        if (subMenu->isClosed())
        {
            SAFE_DELETE(subMenu);
        }
        return;
    }
    if (sbMenu)
    {
        sbMenu->Update(dt);
        if (sbMenu->isClosed())
        {
            SAFE_DELETE(sbMenu);
        }
        return;
    }
    if (mStage == STAGE_WAITING || mStage == STAGE_ONSCREEN_MENU)
    {
        JButton button = mEngine->ReadButton();
        switch (button)
        {
        case JGE_BTN_LEFT:
        case JGE_BTN_RIGHT:
        case JGE_BTN_UP:
        case JGE_BTN_DOWN:
            if(mView->ButtonPressed(button))
            {
                last_user_activity = 0;
                mStage = STAGE_WAITING;
            }
            break;
        case JGE_BTN_CANCEL:
            options[Options::DISABLECARDS].number = !options[Options::DISABLECARDS].number;
            break;
        case JGE_BTN_CTRL:
            if (last_user_activity > 0.2)
            {
                last_user_activity = 0;
                toggleSB_CMD_DNG();
            }
            break;
        case JGE_BTN_PRI:
            if (last_user_activity > 0.2)
            {
                last_user_activity = 0;
                toggleCollection();
            }
            break;
        case JGE_BTN_MAX:
            if (last_user_activity > 0.2)
            {
                last_user_activity = 0;
                toggleView();
            }
            break;
        case JGE_BTN_OK:
        {
            // verify that none of the buttons fired
            if (userPressedButton())
            {
                Update(dt);
                break;
            }

            int x, y;
            if (mEngine->GetLeftClickCoordinates(x, y))
            {
                mEngine->LeftClickedProcessed();
                if(mView->Click(x, y) != NULL)
                {
                    //addRemove(mView->getActiveCard());
                    choiceAddRemove(mView->getActiveCard());
                }
            }
            else
            {
                if(mView->Click() != NULL)
                {
                    //addRemove(mView->getActiveCard());
                    choiceAddRemove(mView->getActiveCard());
                }
            }

            last_user_activity = 0;
            mStage = STAGE_WAITING;
            break;
        }
        case JGE_BTN_SEC:
            sellCard();
            break;

        case JGE_BTN_MENU:
            if(mView->deck() == mySideboard || mView->deck() == myCommandZone || mView->deck() == myDungeonZone)
            {
                mView->SetDeck(myCollection);
                sb_cmd_dng_Button->setText("View SB");
            }
            else
            {
                mStage = STAGE_MENU;
                buildEditorMenu();
            }
            break;
        case JGE_BTN_SOUND:
            if (mView->deck() == mySideboard || mView->deck() == myCommandZone || mView->deck() == myDungeonZone)
                break;//SB is for viewing add or remove only
            else if(!mView->ButtonPressed(JGE_BTN_CTRL))
            {
                mStage = STAGE_FILTERS;
                if (!filterMenu)
                {
                    filterMenu = NEW WGuiFilters("Filter by...", NULL);
                    if (source)
                        SAFE_DELETE(source);
                    source = NEW WSrcDeckViewer(myDeck, myCollection);
                    filterMenu->setSrc(source);
                    if (mView->deck() != myDeck) source->swapSrc();
                }
                filterMenu->Entering(JGE_BTN_NONE);
            }
            break;
        case JGE_BTN_PREV:
            if (last_user_activity < NO_USER_ACTIVITY_HELP_DELAY)
                last_user_activity = NO_USER_ACTIVITY_HELP_DELAY + 1;
            else if ((mStage == STAGE_ONSCREEN_MENU) && (--mStatsWrapper->currentPage < 0)) mStatsWrapper->currentPage = mStatsWrapper->pageCount;
            break;
        case JGE_BTN_NEXT:
            if (last_user_activity < NO_USER_ACTIVITY_HELP_DELAY)
                last_user_activity = NO_USER_ACTIVITY_HELP_DELAY + 1;
            else if ((mStage == STAGE_ONSCREEN_MENU) && (++mStatsWrapper->currentPage > mStatsWrapper->pageCount)) mStatsWrapper->currentPage = 0;
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

            break;
        }

    }

    mView->Update(dt);
    if(mView->dirtyFilters)
    {
        updateFilters();
        mView->reloadIndexes();
        mView->dirtyFilters = false;
    }

    if (mStage == STAGE_WELCOME)
        welcome_menu->Update(dt);
    else if (mStage == STAGE_MENU)
        deckMenu->Update(dt);
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
                mView->reloadIndexes();
                return;
            }
            if (!filterMenu->isFinished())
            {
                filterMenu->CheckUserInput(key);
                filterMenu->Update(dt);
                mView->reloadIndexes();
            }
            else
            {
                mStage = STAGE_WAITING;
                updateFilters();
                mView->reloadIndexes();
            }
        }
    }
}

void GameStateDeckViewer::renderOnScreenBasicInfo()
{
    JRenderer *renderer = JRenderer::GetInstance();
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    char buffer[256];

    float y = 0;
    int allCopies, nowCopies;
    nowCopies = mView->deck()->getCount(WSrcDeck::FILTERED_COPIES);
    allCopies = mView->deck()->getCount(WSrcDeck::UNFILTERED_COPIES);
    WCardFilter * wc = mView->deck()->getFiltersRoot();

    if (wc)
        sprintf(buffer, "%s %i of %i cards (%i unique)", (mView->deck() == myDeck) ? "DECK: " : " ", nowCopies, allCopies,
                mView->deck()->getCount(WSrcDeck::FILTERED_UNIQUE));
    else
        sprintf(buffer, "%s%i cards (%i unique)", (mView->deck() == myDeck) ? "DECK: " : " ", allCopies,
                mView->deck()->getCount(WSrcDeck::UNFILTERED_UNIQUE));

    float w = mFont->GetStringWidth(buffer);
    PIXEL_TYPE backupColor = mFont->GetColor();

    renderer->FillRoundRect(SCREEN_WIDTH - (w + 27), y, w + 5, 4, 5, ARGB(hudAlpha/2,0,0,0));
    mFont->SetColor(ARGB(hudAlpha,255,255,255));
    mFont->DrawString(buffer, SCREEN_WIDTH - 20, y + 1, JGETEXT_RIGHT);
    mFont->SetColor(backupColor);
    
    if (mView->filter() != 0) renderer->RenderQuad(mIcons[mView->filter() - 1].get(), SCREEN_WIDTH - 10, y + 15, 0.0f, 0.5, 0.5);
}

void GameStateDeckViewer::renderSlideBar()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);

    int total = mView->deck()->Size();
    if (total == 0) return;

    float filler = 25;
    float y = SCREEN_HEIGHT_F - 30;
    float bar_size = SCREEN_WIDTH_F - 2 * filler;
    JRenderer * r = JRenderer::GetInstance();
    int currentPos = mView->getPosition();

    float cursor_pos = bar_size * currentPos / total;

    //r->FillRoundRect(filler + 5, y + 5, bar_size, 0, 4, ARGB(hudAlpha/2,0,0,0));
    //r->DrawLine(filler + cursor_pos + 5, y + 5, filler + cursor_pos + 5, y + 10, ARGB(hudAlpha/2,0,0,0));

    r->FillRoundRect(filler, y, bar_size, 0, 4, ARGB(hudAlpha/2,128,128,128));
    r->DrawRoundRect(filler, y, bar_size, 0, 4, ARGB(hudAlpha/2,0,0,0));
    r->DrawLine(filler + cursor_pos, y, filler + cursor_pos, y + 8, ARGB(hudAlpha,0,255,0));
    char buffer[256];
    string deckname = _("Collection");
    if (mView->deck() == myDeck)
    {
        deckname = _("Deck");
    }
    sprintf(buffer, "%s - %i/%i", deckname.c_str(), currentPos, total);
    mFont->SetColor(ARGB(hudAlpha,255,255,255));
    mFont->DrawString(buffer, SCREEN_WIDTH / 2, y-2, JGETEXT_CENTER);

    mFont->SetColor(ARGB(255,255,255,255));
}

void GameStateDeckViewer::renderDeckBackground()
{
    int max1 = 0;
    int maxC1 = 4;
    int max2 = 0;
    int maxC2 = 4;

    for (int i = 0; i < Constants::NB_Colors - 1; i++)
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
    WFont * font = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
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

#ifdef TOUCH_ENABLED
    bool renderPSPIcons = false;
#else
    bool renderPSPIcons = true;
#endif
    
    if (mStatsWrapper->currentPage == 0)
    {
        //FillRects
        r->FillRect(0 - (onScreenTransition * 84), 0, 84, SCREEN_HEIGHT, ARGB(128,0,0,0));
        r->FillRect(SCREEN_WIDTH - 204 + (onScreenTransition * 204), 0, 204, SCREEN_HEIGHT, ARGB(128,0,0,0));
        if (renderPSPIcons)
        {
            //LEFT PSP CIRCLE render
            r->FillCircle(leftPspX, leftPspY, 40, ARGB(128,50,50,50));
            
            r->RenderQuad(pspIcons[0].get(), leftPspX, leftPspY - 20, 0, pspIconsSize, pspIconsSize);
            r->RenderQuad(pspIcons[1].get(), leftPspX, leftPspY + 20, 0, pspIconsSize, pspIconsSize);
            r->RenderQuad(pspIcons[2].get(), leftPspX - 20, leftPspY, 0, pspIconsSize, pspIconsSize);
            r->RenderQuad(pspIcons[3].get(), leftPspX + 20, leftPspY, 0, pspIconsSize, pspIconsSize);
            
            font->DrawString(_("Prev."), leftPspX - 35, leftPspY - 15);
            font->DrawString(_("Next"), leftPspX + 15, leftPspY - 15);
            font->DrawString(_("card"), leftPspX - 35, leftPspY);
            font->DrawString(_("card"), leftPspX + 15, leftPspY);
            font->DrawString(_("Next edition"), leftPspX - 33, leftPspY - 35);
            font->DrawString(_("Prev. edition"), leftPspX - 33, leftPspY + 25);
            
            //RIGHT PSP CIRCLE render
            r->FillCircle(rightPspX + (onScreenTransition * 204), rightPspY, 40, ARGB(128,50,50,50));
            r->RenderQuad(pspIcons[4].get(), rightPspX + 20, rightPspY, 0, pspIconsSize, pspIconsSize);
            r->RenderQuad(pspIcons[5].get(), rightPspX, rightPspY - 20, 0, pspIconsSize, pspIconsSize);
            r->RenderQuad(pspIcons[6].get(), rightPspX - 20, rightPspY, 0, pspIconsSize, pspIconsSize);
            r->RenderQuad(pspIcons[7].get(), rightPspX, rightPspY + 20, 0, pspIconsSize, pspIconsSize);
            
            font->DrawString(_("Toggle Images"), rightPspX - 35, rightPspY - 40);
            
            if (mView->deck() == myCollection)
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

            if (mView->deck() == myCollection)
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
            // print stuff here about the editor commands
            float textYOffset = SCREEN_HEIGHT_F/2;
            font->DrawString(_("Click on the card image"), SCREEN_WIDTH - 200 + rightTransition, textYOffset - (2 * fH));
            if (mView->deck() == myCollection)
                font->DrawString(_("to add card to deck."), SCREEN_WIDTH - 200 + rightTransition, textYOffset - fH);
            else
                font->DrawString(_("to remove card from deck."), SCREEN_WIDTH - 200 + rightTransition, textYOffset - fH);
        }
        //Your Deck Information
        char buffer[300];
        int nb_letters = 0;
        int value = myDeck->getCount(WSrcDeck::UNFILTERED_COPIES);
        int sb_value = mySideboard->getCount(WSrcDeck::UNFILTERED_COPIES);
        int cmd_value = myCommandZone->getCount(WSrcDeck::UNFILTERED_COPIES);
        int dng_value = myDungeonZone->getCount(WSrcDeck::UNFILTERED_COPIES);
        sprintf(buffer, _("Your Deck: %i cards.\nSideboard: %i cards.\nCommanders: %i cards.\nDungeons: %i cards").c_str(), value, sb_value,cmd_value,dng_value);
        font->DrawString(buffer, SCREEN_WIDTH - 200 + rightTransition, SCREEN_HEIGHT / 2 + 15);

        for (int j = 0; j < Constants::NB_Colors; j++)
        {
            int value = myDeck->getCount(j);
            if(value > 0)
            {
                int modx = value < 9?2:0;
                r->RenderQuad(mIcons[j].get(), SCREEN_WIDTH - 190 + rightTransition + nb_letters * 8, SCREEN_HEIGHT / 2 + 49, 0, 0.45f,0.45f);
                sprintf(buffer, "%i", value);
                font->DrawString(buffer, SCREEN_WIDTH - 195 + rightTransition + modx + nb_letters * 8, SCREEN_HEIGHT / 2 + 55);
                nb_letters += 3;
            }
        }

    }
    else
    {
        mStatsWrapper->updateStats( myDeck );;

        char buffer[300];

        leftTransition = -(onScreenTransition / 2) * SCREEN_WIDTH;
        rightTransition = -leftTransition;

        r->FillRect(0 + leftTransition, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, ARGB(128,0,0,0));
        r->FillRect(SCREEN_WIDTH / 2 + rightTransition, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, ARGB(128,0,0,0));
        r->FillRect(10 + leftTransition, 10, SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT - 20, ARGB(128,0,0,0));
        r->FillRect(SCREEN_WIDTH / 2 + rightTransition, 10, SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT - 20, ARGB(128,0,0,0));
#ifndef TOUCH_ENABLED
        font->DrawString(_("menu"), SCREEN_WIDTH - 35 + rightTransition, SCREEN_HEIGHT - 15);
        font->DrawString(_("filter"), SCREEN_WIDTH - 95 + rightTransition, SCREEN_HEIGHT - 15);
#endif
        int nb_letters = 0;
        float posX, posY;
        DWORD graphColor;

        graphColor = ARGB(200, 155, 155, 155);
        string STATS_TITLE_FORMAT = _("%i: %s");

        switch (mStatsWrapper->currentPage)
        {
        case 1: // Counts, price
            // Title
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), mStatsWrapper->currentPage, _("Statistics Summary").c_str());
            font->DrawString(buffer, 10 + 20 + leftTransition, 10);

            posY = 30;
            posX = 180;
            sprintf(buffer, _("Your Deck: %i cards").c_str(), mStatsWrapper->cardCount);
            font->DrawString(buffer, 20 + 20 + leftTransition, posY);
            posY += 10;

            // Counts by color
            for (int j = 0; j < Constants::NB_Colors; j++)
            {
                int value = myDeck->getCount(j);
                if (value > 0)
                {
                    sprintf(buffer, "%i", value);
                    font->DrawString(buffer, 38 + nb_letters * 13 + 20 + leftTransition, posY + 5);
                    r->RenderQuad(mIcons[j].get(), 30 + nb_letters * 13 + 20 + leftTransition, posY + 11, 0, 0.5, 0.5);
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

            r->DrawLine(posX - 4 + 20 + leftTransition, posY - 1, posX - 4 + 20 + leftTransition, posY + 177, ARGB(128, 255, 255, 255));
            r->DrawLine(19 + 20 + leftTransition, posY - 1, 19 + 20 + leftTransition, posY + 177, ARGB(128, 255, 255, 255));
            r->DrawLine(posX + 40 + 20 + leftTransition, posY - 1, posX + 40 + 20 + leftTransition, posY + 177, ARGB(128, 255, 255, 255));

            r->DrawLine(20 + 20 + leftTransition, posY - 1, posX + 40 + 20 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));

            font->DrawString(_("Lands"), 20 + 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), mStatsWrapper->countLands);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 14;
            r->DrawLine(20 + 20 + leftTransition, posY - 1, posX + 40 + 20 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Creatures"), 20 + 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), mStatsWrapper->countCreatures);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 14;
            r->DrawLine(20 + 20 + leftTransition, posY - 1, posX + 40 + 20 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Spells"), 20 + 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), mStatsWrapper->countSpells);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Instants"), 30 + 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), mStatsWrapper->countInstants);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Enchantments"), 30 + 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), mStatsWrapper->countEnchantments);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Sorceries"), 30 + 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), mStatsWrapper->countSorceries);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);
            //sprintf(buffer, "Artifacts: %i", stw->countArtifacts);
            //mFont->DrawString(buffer, 20, 123);

            posY += 14;
            r->DrawLine(20 + 20 + leftTransition, posY - 1, posX + 40 + 20 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));

            font->DrawString(_("Average converted mana cost"), 20 + 20 + leftTransition, posY);
            sprintf(buffer, _("%2.2f").c_str(), mStatsWrapper->avgManaCost);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 14;
            r->DrawLine(20 + 20 + leftTransition, posY - 1, posX + 40 + 20 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Probabilities"), 20 + 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("No land in 1st hand"), 30 + 20 + leftTransition, posY);
            sprintf(buffer, _("%2.2f%%").c_str(), mStatsWrapper->noLandsProbInTurn[0]);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("No land in 9 cards"), 30 + 20 + leftTransition, posY);
            sprintf(buffer, _("%2.2f%%").c_str(), mStatsWrapper->noLandsProbInTurn[2]);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("No creatures in 1st hand"), 30 + 20 + leftTransition, posY);
            sprintf(buffer, _("%2.2f%%").c_str(), mStatsWrapper->noCreaturesProbInTurn[0]);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            // Playgame Statistics
            posY += 14;
            r->DrawLine(20 + 20 + leftTransition, posY - 1, posX + 40 + 20 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Playgame statistics"), 20 + 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Games played"), 30 + 20 + leftTransition, posY);
            sprintf(buffer, _("%i").c_str(), mStatsWrapper->gamesPlayed);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 10;
            font->DrawString(_("Victory ratio"), 30 + 20 + leftTransition, posY);
            sprintf(buffer, _("%i%%").c_str(), mStatsWrapper->percentVictories);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);

            posY += 15;
            r->DrawLine(20 + 20 + leftTransition, posY - 1, posX + 40 + 20 + leftTransition, posY - 1, ARGB(128, 255, 255, 255));
            font->DrawString(_("Total price (credits)"), 20 + 20 + leftTransition, posY);
            sprintf(buffer, _("%i ").c_str(), mStatsWrapper->totalPrice);
            font->DrawString(buffer, posX + 20 + leftTransition, posY);
            r->DrawLine(20 + 20 + leftTransition, posY + 13, posX + 40 + 20 + leftTransition, posY + 13, ARGB(128, 255, 255, 255));

            break;

        case 5: // Land statistics
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), mStatsWrapper->currentPage, _("Mana production").c_str());
            font->DrawString(buffer, 10 + 20 + leftTransition, 10);

            font->DrawString(_("Counts of manasources per type and color:"), 20 + 20 + leftTransition, 30);

            posY = 70;

            // Column titles
            for (int j = 0; j < Constants::NB_Colors - 1; j++)
            {
                r->RenderQuad(mIcons[j].get(), 52 + j * 15 + 20 + leftTransition, posY - 10, 0, 0.5, 0.5);
            }

            //font->DrawString(_("C"), 30 + leftTransition, posY-16);
            //font->DrawString(_("Ty"), 27 + leftTransition, posY-16);

            // Horizontal table lines
            r->DrawLine(27 + 20 + leftTransition, posY - 20, 60 + (Constants::NB_Colors - 2) * 15 + 20 + leftTransition, posY - 20,
                        ARGB(128, 255, 255, 255));
            r->DrawLine(27 + 20 + leftTransition, posY - 1, 60 + (Constants::NB_Colors - 2) * 15 + 20 + leftTransition, posY - 1,
                        ARGB(128, 255, 255, 255));
            r->DrawLine(27 + 20 + leftTransition, 2 * 10 + posY + 12, 60 + (Constants::NB_Colors - 2) * 15 + 20 + leftTransition, 2 * 10
                        + posY + 12, ARGB(128, 255, 255, 255));
            r->DrawLine(27 + 20 + leftTransition, 3 * 10 + posY + 14, 60 + (Constants::NB_Colors - 2) * 15 + 20 + leftTransition, 3 * 10
                        + posY + 14, ARGB(128, 255, 255, 255));

            // Vertical table lines
            r->DrawLine(26 + 20 + leftTransition, posY - 20, 26 + 20 + leftTransition, 3 * 10 + posY + 14, ARGB(128, 255, 255, 255));
            r->DrawLine(43 + 20 + leftTransition, posY - 20, 43 + 20 + leftTransition, 3 * 10 + posY + 14, ARGB(128, 255, 255, 255));
            r->DrawLine(60 + 20 + leftTransition + (Constants::NB_Colors - 2) * 15, posY - 20, 60 + 20 + leftTransition
                        + (Constants::NB_Colors - 2) * 15, 3 * 10 + posY + 14, ARGB(128, 255, 255, 255));

            font->DrawString(_("BL"), 27 + 20 + leftTransition, posY);
            font->DrawString(_("NB"), 27 + 20 + leftTransition, posY + 10);
            font->DrawString(_("O"), 30 + 20 + leftTransition, posY + 20);
            font->DrawString(_("T"), 30 + 20 + leftTransition, posY + 33);

            int curCount;

            for (int j = 0; j < Constants::NB_Colors - 1; j++)
            {
                curCount = mStatsWrapper->countBasicLandsPerColor[j];
                if(curCount == 0) {
                    sprintf(buffer, ".");
                } else {
                    sprintf(buffer, "%i", curCount);
                }
                font->DrawString(buffer, 49 + 20 + leftTransition + j * 15, posY);

                curCount = mStatsWrapper->countLandsPerColor[j];
                if(curCount == 0) {
                    sprintf(buffer, ".");
                } else {
                    sprintf(buffer, "%i", curCount);
                }
                font->DrawString(buffer, 49 + 20 + leftTransition + j * 15, posY + 10);

                curCount = mStatsWrapper->countNonLandProducersPerColor[j];
                if(curCount == 0) {
                    sprintf(buffer, ".");
                } else {
                    sprintf(buffer, "%i", curCount);
                }
                font->DrawString(buffer, 49 + 20 + leftTransition + j * 15, posY + 20);

                curCount = mStatsWrapper->countLandsPerColor[j] + mStatsWrapper->countBasicLandsPerColor[j] + mStatsWrapper->countNonLandProducersPerColor[j];
                if(curCount == 0) {
                    sprintf(buffer, ".");
                } else {
                    sprintf(buffer, "%i", curCount);
                }
                font->DrawString(buffer, 49 + 20 + leftTransition + j * 15, posY + 33);
            }

            posY += 55;
            font->DrawString(_("BL - Basic lands"), 20 + 20 + leftTransition, posY);
            posY += 10;
            font->DrawString(_("NB - Non-basic lands"), 20 + 20 + leftTransition, posY);
            posY += 10;
            font->DrawString(_("O - Other (non-land) manasources"), 26 + 20 + leftTransition, posY);
            posY += 10;
            font->DrawString(_("T - Totals"), 26 + 20 + leftTransition, posY);

            break;

        case 6: // Land statistics - in symbols
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), mStatsWrapper->currentPage, _("Mana production - in mana symbols").c_str());
            font->DrawString(buffer, 10 + 20 + leftTransition, 10);
            font->DrawString(_("Total colored mana symbols in lands' production:"), 20 + 20 + leftTransition, 30);

            int totalProducedSymbols;
            totalProducedSymbols = 0;
            for (int i = 1; i < Constants::NB_Colors - 1; i++)
            {
                totalProducedSymbols += mStatsWrapper->countLandsPerColor[i] + mStatsWrapper->countBasicLandsPerColor[i]; //!! Move to updatestats!
            }

            posY = 50;
            for (int i = 1; i < Constants::NB_Colors - 1; i++)
            {
                if (mStatsWrapper->countLandsPerColor[i] + mStatsWrapper->countBasicLandsPerColor[i] > 0)
                {
                    sprintf(buffer, _("%i").c_str(), mStatsWrapper->countLandsPerColor[i] + mStatsWrapper->countBasicLandsPerColor[i]);
                    font->DrawString(buffer, 20 + leftTransition, posY);
                    sprintf(buffer, _("(%i%%)").c_str(), (int) (100 * (float) (mStatsWrapper->countLandsPerColor[i]
                                                                               + mStatsWrapper->countBasicLandsPerColor[i]) / totalProducedSymbols));
                    font->DrawString(buffer, 33 + 20 + leftTransition, posY);
                    posX = 72;
                    for (int j = 0; j < mStatsWrapper->countLandsPerColor[i] + mStatsWrapper->countBasicLandsPerColor[i]; j++)
                    {
                        r->RenderQuad(mIcons[i].get(), posX + 20 + leftTransition, posY + 6, 0, 0.5, 0.5);
                        posX += ((j + 1) % 10 == 0) ? 17 : 13;
                        if ((((j + 1) % 30) == 0) && (j < mStatsWrapper->countLandsPerColor[i] + mStatsWrapper->countBasicLandsPerColor[i] - 1))
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

            switch (mStatsWrapper->currentPage)
            { // Nested switch on the same variable. Oh yes.
            case 2: // Total counts
                // Title
                sprintf(buffer, STATS_TITLE_FORMAT.c_str(), mStatsWrapper->currentPage, _("Mana cost detail").c_str());
                font->DrawString(buffer, 10 + 20 + leftTransition, 10);
                font->DrawString(_("Card counts per mana cost:"), 20 + 20 + leftTransition, 30);
                avgCost = mStatsWrapper->avgManaCost;
                countPerCost = &mStatsWrapper->countCardsPerCost;
                countPerCostAndColor = &mStatsWrapper->countCardsPerCostAndColor;
                break;
            case 3: // Creature counts
                // Title
                sprintf(buffer, STATS_TITLE_FORMAT.c_str(), mStatsWrapper->currentPage, _("Mana cost detail - Creatures").c_str());
                font->DrawString(buffer, 10 + 20 + leftTransition, 10);
                font->DrawString(_("Creature counts per mana cost:"), 20 + 20 + leftTransition, 30);
                avgCost = mStatsWrapper->avgCreatureCost;
                countPerCost = &mStatsWrapper->countCreaturesPerCost;
                countPerCostAndColor = &mStatsWrapper->countCreaturesPerCostAndColor;
                break;
            case 4: // Spell counts
                // Title
                sprintf(buffer, STATS_TITLE_FORMAT.c_str(), mStatsWrapper->currentPage, _("Mana cost detail - Spells").c_str());
                font->DrawString(buffer, 10 + 20 + leftTransition, 10);
                font->DrawString(_("Non-creature spell counts per mana cost:"), 20 + 20 + leftTransition, 30);
                avgCost = mStatsWrapper->avgSpellCost;
                countPerCost = &mStatsWrapper->countSpellsPerCost;
                countPerCostAndColor = &mStatsWrapper->countSpellsPerCostAndColor;
                break;
            default:
                countPerCost = NULL;
                countPerCostAndColor = NULL;
                avgCost = 0;
                break;
            }

            posY = 70;

            // Column titles
            for (int j = 0; j < Constants::NB_Colors - 1; j++)
            {
                r->RenderQuad(mIcons[j].get(), 67 + j * 15 + 20 + leftTransition, posY - 10, 0, 0.5, 0.5);
            }

            font->DrawString(_("C"), 30 + 20 + leftTransition, posY - 16);
            font->DrawString(_("#"), 45 + 20 + leftTransition, posY - 16);

            // Horizontal table lines
            r->DrawLine(27 + 20 + leftTransition, posY - 20, 75 + (Constants::NB_Colors - 2) * 15 + 20 + leftTransition, posY - 20,
                        ARGB(128, 255, 255, 255));
            r->DrawLine(27 + 20 + leftTransition, posY - 1, 75 + (Constants::NB_Colors - 2) * 15 + 20 + leftTransition, posY - 1,
                        ARGB(128, 255, 255, 255));
            r->DrawLine(27 + 20 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12, 75 + (Constants::NB_Colors - 2)
                        * 15 + 20 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12, ARGB(128, 255, 255, 255));

            // Vertical table lines
            r->DrawLine(26 + 20 + leftTransition, posY - 20, 26 + 20 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12,
                        ARGB(128, 255, 255, 255));
            r->DrawLine(41 + 20 + leftTransition, posY - 20, 41 + 20 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12,
                        ARGB(128, 255, 255, 255));
            r->DrawLine(58 + 20 + leftTransition, posY - 20, 58 + 20 + leftTransition, Constants::STATS_MAX_MANA_COST * 10 + posY + 12,
                        ARGB(128, 255, 255, 255));
            r->DrawLine(75 + 20 + leftTransition + (Constants::NB_Colors - 2) * 15, posY - 20, 75 + 20 + leftTransition
                        + (Constants::NB_Colors - 2) * 15, Constants::STATS_MAX_MANA_COST * 10 + posY + 12,
                        ARGB(128, 255, 255, 255));

            for (int i = 0; i <= Constants::STATS_MAX_MANA_COST; i++)
            {
                sprintf(buffer, _("%i").c_str(), i);
                font->DrawString(buffer, 30 + 20 + leftTransition, posY);
                sprintf(buffer, ((*countPerCost)[i] > 0) ? _("%i").c_str() : ".", (*countPerCost)[i]);
                font->DrawString(buffer, 45 + 20 + leftTransition, posY);
                for (int j = 0; j < Constants::NB_Colors - 1; j++)
                {
                    sprintf(buffer, ((*countPerCostAndColor)[i][j] > 0) ? _("%i").c_str() : ".", (*countPerCostAndColor)[i][j]);
                    font->DrawString(buffer, 64 + 20 + leftTransition + j * 15, posY);
                }
                r->FillRect(77.f + 20 + leftTransition + (Constants::NB_Colors - 2) * 15.0f, posY + 2.0f, (*countPerCost)[i] * 5.0f,
                            8.0f, graphColor);
                posY += 10;
            }

            posY += 10;
            sprintf(buffer, _("Average converted mana cost: %2.2f").c_str(), avgCost);
            font->DrawString(buffer, 20 + 20 + leftTransition, posY);
            posY += 15;
            sprintf(buffer, _("C - Converted mana cost. Cards with cost>%i are included in the last row.").c_str(),
                    Constants::STATS_MAX_MANA_COST);
            font->DrawString(buffer, 20 + 20 + leftTransition, posY);
            posY += 10;
            font->DrawString(_("# - Total number of cards with given cost"), 20 + 20 + leftTransition, posY);

            break;

        case 8:
            // Title
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), mStatsWrapper->currentPage, _("Probabilities").c_str());
            font->DrawString(buffer, 10 + 20 + leftTransition, 10);

            // No lands detail
            float graphScale, graphWidth;
            graphWidth = 100;
            graphScale = (mStatsWrapper->noLandsProbInTurn[0] == 0) ? 0 : (graphWidth / mStatsWrapper->noLandsProbInTurn[0]);
            font->DrawString(_("No lands in first n cards:"), 20 + 20 + leftTransition, 30);

            posY = 50;
            for (int i = 0; i < Constants::STATS_FOR_TURNS; i++)
            {
                sprintf(buffer, _("%i:").c_str(), i + 7);
                font->DrawString(buffer, 30 + 20 + leftTransition, posY);
                sprintf(buffer, _("%2.2f%%").c_str(), mStatsWrapper->noLandsProbInTurn[i]);
                font->DrawString(buffer, 45 + 20 + leftTransition, posY);
                r->FillRect(84 + 20 + leftTransition, posY + 2, graphScale * mStatsWrapper->noLandsProbInTurn[i], 8, graphColor);
                posY += 10;
            }

            // No creatures probability detail
            posY += 10;
            font->DrawString(_("No creatures in first n cards:"), 20 + 20 + leftTransition, posY);
            posY += 20;
            graphScale = (mStatsWrapper->noCreaturesProbInTurn[0] == 0) ? 0 : (graphWidth / mStatsWrapper->noCreaturesProbInTurn[0]);

            for (int i = 0; i < Constants::STATS_FOR_TURNS; i++)
            {
                sprintf(buffer, _("%i:").c_str(), i + 7);
                font->DrawString(buffer, 30 + 20 + leftTransition, posY);
                sprintf(buffer, _("%2.2f%%").c_str(), mStatsWrapper->noCreaturesProbInTurn[i]);
                font->DrawString(buffer, 45 + 20 + leftTransition, posY);
                r->FillRect(84 + 20 + leftTransition, posY + 2, graphScale * mStatsWrapper->noCreaturesProbInTurn[i], 8, graphColor);
                posY += 10;
            }

            break;

        case 7: // Total mana cost per color
            // Title
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), mStatsWrapper->currentPage, _("Mana cost per color").c_str());
            font->DrawString(buffer, 10 + 20 + leftTransition, 10);

            font->DrawString(_("Total colored mana symbols in cards' casting costs:"), 20 + 20 + leftTransition, 30);

            posY = 50;
            for (int i = 1; i < Constants::NB_Colors - 1; i++)
            {
                if (mStatsWrapper->totalCostPerColor[i] > 0)
                {
                    sprintf(buffer, _("%i").c_str(), mStatsWrapper->totalCostPerColor[i]);
                    font->DrawString(buffer, 20 + leftTransition, posY);
                    sprintf(buffer, _("(%i%%)").c_str(), (int) (100 * (float) mStatsWrapper->totalCostPerColor[i] / mStatsWrapper->totalColoredSymbols));
                    font->DrawString(buffer, 33 + leftTransition, posY);
                    posX = 72;
                    for (int j = 0; j < mStatsWrapper->totalCostPerColor[i]; j++)
                    {
                        r->RenderQuad(mIcons[i].get(), posX + 20 + leftTransition, posY + 6, 0, 0.5, 0.5);
                        posX += ((j + 1) % 10 == 0) ? 17 : 13;
                        if ((((j + 1) % 30) == 0) && (j < mStatsWrapper->totalCostPerColor[i] - 1))
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
            sprintf(buffer, STATS_TITLE_FORMAT.c_str(), mStatsWrapper->currentPage, _("Victory statistics").c_str());
            font->DrawString(buffer, 10 + 20 + leftTransition, 10);

            font->DrawString(_("Victories against AI:"), 20 + 20 + leftTransition, 30);

            sprintf(buffer, _("Games played: %i").c_str(), mStatsWrapper->gamesPlayed);
            font->DrawString(buffer, 20 + 20 + leftTransition, 45);
            sprintf(buffer, _("Victory ratio: %i%%").c_str(), mStatsWrapper->percentVictories);
            font->DrawString(buffer, 20 + 20 + leftTransition, 55);

            int AIsPerColumn = 19;
            posY = 70;
            posX = 20;

            // ToDo: Multiple pages when too many AI decks are present
            for (int i = 0; i < (int) mStatsWrapper->aiDeckStats.size(); i++)
            {
                sprintf(buffer, _("%.14s").c_str(), mStatsWrapper->aiDeckNames.at(i).c_str());
                font->DrawString(buffer, posX + (i < 2 * AIsPerColumn ? leftTransition : rightTransition), posY);
                sprintf(buffer, _("%i/%i").c_str(), mStatsWrapper->aiDeckStats.at(i)->victories, mStatsWrapper->aiDeckStats.at(i)->nbgames);
                font->DrawString(buffer, posX + (i < AIsPerColumn ? leftTransition : rightTransition) + 80, posY);
                sprintf(buffer, _("%i%%").c_str(), mStatsWrapper->aiDeckStats.at(i)->percentVictories());
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

void GameStateDeckViewer::Render()
{
    setButtonState(false);
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    JRenderer::GetInstance()->ClearScreen(ARGB(0,0,0,0));
#if !defined (PSP)
    //Now it's possibile to randomly use up to 10 background images for deck editor background (if random index is 0, it will be rendered the default "bgdeckeditor.jpg" image).
    JTexture * wpTex = NULL;
    if(kBgFile == ""){
        char temp[4096];
        sprintf(temp, "bgdeckeditor%i.jpg", std::rand() % 10);
        kBgFile.assign(temp);
        wpTex = WResourceManager::Instance()->RetrieveTexture(kBgFile);
        if (wpTex) {
            JQuadPtr wpQuad = WResourceManager::Instance()->RetrieveTempQuad(kBgFile);
            if (wpQuad.get())
                JRenderer::GetInstance()->RenderQuad(wpQuad.get(), 0, 0, 0, SCREEN_WIDTH_F / wpQuad->mWidth, SCREEN_HEIGHT_F / wpQuad->mHeight);
            else {
               kBgFile = "bgdeckeditor.jpg"; //Fallback to default background image for deck editor background.
               wpTex = NULL;
            }
        } else
            kBgFile = "bgdeckeditor.jpg"; //Fallback to default background image for deck editor background.
    }
    if(!wpTex)
        wpTex = WResourceManager::Instance()->RetrieveTexture(kBgFile);
    if (wpTex)
    {
        JQuadPtr wpQuad = WResourceManager::Instance()->RetrieveTempQuad(kBgFile);
        JRenderer::GetInstance()->RenderQuad(wpQuad.get(), 0, 0, 0, SCREEN_WIDTH_F / wpQuad->mWidth, SCREEN_HEIGHT_F / wpQuad->mHeight);
    }
    /*if (mView->deck() == myDeck && mStage != STAGE_MENU)
        renderDeckBackground();*/
#else
    JTexture * wpTex = WResourceManager::Instance()->RetrieveTexture("pspbgdeckeditor.jpg");
    if (wpTex)
    {
        JQuadPtr wpQuad = WResourceManager::Instance()->RetrieveTempQuad("pspbgdeckeditor.jpg");
        JRenderer::GetInstance()->RenderQuad(wpQuad.get(), 0, 0, 0, SCREEN_WIDTH_F / wpQuad->mWidth, SCREEN_HEIGHT_F / wpQuad->mHeight);
    }
    /*if (mView->deck() == myDeck && mStage != STAGE_MENU)
        renderDeckBackground();*/
#endif
    mView->Render();

    if (mView->deck()->Size() > 0)
    {
        setButtonState(true);
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
        setButtonState(false);
        welcome_menu->Render();
    }
    else
    {
        setButtonState(true);
        renderOnScreenBasicInfo();
    }
    
    if (mStage == STAGE_MENU)
    {
        setButtonState(false);
        deckMenu->Render();
    }
    
    if (subMenu) subMenu->Render();

    if (sbMenu) sbMenu->Render();

    if (filterMenu && !filterMenu->isFinished())
    {
        setButtonState(false);
        filterMenu->Render();
    }
    
    if (options.keypadActive()) options.keypadRender();

    RenderButtons();
}

int GameStateDeckViewer::loadDeck(int deckid)
{

    if (!mStatsWrapper)
    {
        DeckManager *deckManager = DeckManager::GetInstance();
        mStatsWrapper = deckManager->getExtendedStatsForDeckId( deckid, MTGCollection(), false );
    }
    
    mStatsWrapper->currentPage = 0;
    mStatsWrapper->pageCount = 9;
    mStatsWrapper->needUpdate = true;

    if (!playerdata) playerdata = NEW PlayerData(MTGCollection());
    SAFE_DELETE(myCollection);
    myCollection = NEW DeckDataWrapper(playerdata->collection);
    myCollection->Sort(WSrcCards::SORT_ALPHA);
    mView->SetDeck(myCollection);

    char deckname[256];
    sprintf(deckname, "deck%i.txt", deckid);
    if (myDeck)
    {
        SAFE_DELETE(myDeck->parent);
        SAFE_DELETE(myDeck);
    }
    myDeck = NEW DeckDataWrapper(NEW MTGDeck(options.profileFile(deckname, "", false).c_str(), MTGCollection()));

    // Check whether the cards in the deck are actually available in the player's collection:
    int cheatmode = options[Options::CHEATMODE].number;
    bool bPure = true;
    bool cPure = true;
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
                myDeck->Remove(current,howmanyinDeck-i); //Nope. Remove it from deck.
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
    //sb
    if (mySideboard)
    {
        SAFE_DELETE(mySideboard->parent);
        SAFE_DELETE(mySideboard);
    }
    //cmd
    if (myCommandZone)
    {
        SAFE_DELETE(myCommandZone->parent);
        SAFE_DELETE(myCommandZone);
    }
    //dng
    if (myDungeonZone)
    {
        SAFE_DELETE(myDungeonZone->parent);
        SAFE_DELETE(myDungeonZone);
    }
    //temp deck for sb?
    MTGDeck * sbtempDeck = NEW MTGDeck(MTGCollection());
    if(myDeck->parent)
    {//add cards from sdeboard lists
        if(myDeck->parent->Sideboard.size())
        {
            for(unsigned int j = 0; j < myDeck->parent->Sideboard.size(); j++)
            {
                string cardID = myDeck->parent->Sideboard[j];
                sbtempDeck->add(atoi(cardID.c_str()));
            }
        }
    }
    mySideboard = NEW DeckDataWrapper(sbtempDeck);
    for (int i = 0; i < mySideboard->Size(true); i++)
    {
        MTGCard * current = mySideboard->getCard(i, true);
        int howmanyinDeck = mySideboard->count(current);
        for (int i = myCollection->count(current); i < howmanyinDeck; i++)
        {
            cPure = false;
            if (cheatmode)
            { //Are we cheating?
                playerdata->collection->add(current); //Yup, add it to collection permanently.
                myCollection->Add(current);
            }
            else
            {
                mySideboard->Remove(current,howmanyinDeck-i); //Nope. Remove it from sb.
                break;
            }
        }

        myCollection->Remove(current, mySideboard->count(current));
    }
    if (!cPure)
    {
        mySideboard->validate();
        myCollection->validate();
    }
    //endsb
    //temp deck for cmd?
    MTGDeck * cmdtempDeck = NEW MTGDeck(MTGCollection());
    if(myDeck->parent)
    {//add cards from commanders lists
        if(myDeck->parent->CommandZone.size())
        {
            for(unsigned int j = 0; j < myDeck->parent->CommandZone.size(); j++)
            {
                string cardID = myDeck->parent->CommandZone[j];
                cmdtempDeck->add(atoi(cardID.c_str()));
            }
        }
    }
    myCommandZone = NEW DeckDataWrapper(cmdtempDeck);
    for (int i = 0; i < myCommandZone->Size(true); i++)
    {
        MTGCard * current = myCommandZone->getCard(i, true);
        int howmanyinDeck = myCommandZone->count(current);
        for (int i = myCollection->count(current); i < howmanyinDeck; i++)
        {
            cPure = false;
            if (cheatmode)
            { //Are we cheating?
                playerdata->collection->add(current); //Yup, add it to collection permanently.
                myCollection->Add(current);
            }
            else
            {
                myCommandZone->Remove(current,howmanyinDeck-i); //Nope. Remove it from sb.
                break;
            }
        }

        myCollection->Remove(current, myCommandZone->count(current));
    }
    if (!cPure)
    {
        myCommandZone->validate();
        myCollection->validate();
    }
    //endcmd
    //temp deck for dng?
    MTGDeck * dngtempDeck = NEW MTGDeck(MTGCollection());
    if(myDeck->parent)
    {//add cards from commanders lists
        if(myDeck->parent->DungeonZone.size())
        {
            for(unsigned int j = 0; j < myDeck->parent->DungeonZone.size(); j++)
            {
                string cardID = myDeck->parent->DungeonZone[j];
                dngtempDeck->add(atoi(cardID.c_str()));
            }
        }
    }
    myDungeonZone = NEW DeckDataWrapper(dngtempDeck);
    for (int i = 0; i < myDungeonZone->Size(true); i++)
    {
        MTGCard * current = myDungeonZone->getCard(i, true);
        int howmanyinDeck = myDungeonZone->count(current);
        for (int i = myCollection->count(current); i < howmanyinDeck; i++)
        {
            cPure = false;
            if (cheatmode)
            { //Are we cheating?
                playerdata->collection->add(current); //Yup, add it to collection permanently.
                myCollection->Add(current);
            }
            else
            {
                myDungeonZone->Remove(current,howmanyinDeck-i); //Nope. Remove it from sb.
                break;
            }
        }

        myCollection->Remove(current, myDungeonZone->count(current));
    }
    if (!cPure)
    {
        myDungeonZone->validate();
        myCollection->validate();
    }
    //enddng

    myDeck->Sort(WSrcCards::SORT_ALPHA);
    mySideboard->Sort(WSrcCards::SORT_ALPHA);
    myCommandZone->Sort(WSrcCards::SORT_ALPHA);
    myDungeonZone->Sort(WSrcCards::SORT_ALPHA);
    SAFE_DELETE(filterMenu);
    rebuildFilters();
    mView->reloadIndexes();
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
            mView->SetDeck(myCollection);
            rebuildFilters();
            mView->reloadIndexes();
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
        case MENU_ITEM_TOGGLE_VIEW:
            mStage = STAGE_WAITING;
            last_user_activity = 0;
            toggleView();
            break;
        }
        break;

    case MENU_CARD_PURCHASE: // Yes/ No sub menu.
        switch (controlId)
        {
        case MENU_ITEM_YES:
        {
            MTGCard * card = mView->getActiveCard();
            if (card)
            {
                int rnd = (rand() % 25);
                int price = pricelist->getSellPrice(card);
                playerdata->credits += price;
                GameApp::mycredits = playerdata->credits;
                price = price - (rnd * price) / 100;
                pricelist->setPrice(card->getMTGId(), price);
                playerdata->collection->remove(card->getMTGId());
                mView->deck()->Remove(card, 1);
                mView->deck()->validate();
                mStatsWrapper->needUpdate = true;
                mView->reloadIndexes();
            }
        }
        case MENU_ITEM_NO:
            subMenu->Close();
            break;
        }

        case SBMENU_CHOICE: // sideboard
        switch (controlId)
        {
        case SBMENU_ADD_NORMAL:
        {
            MTGCard * card = mView->getActiveCard();
            if (card)
                addRemove(card);
            sbMenu->Close();
            break;
        }
        case SBMENU_ADD_SB:
        {
            MTGCard * card = mView->getActiveCard();
            if (card)
                SBaddRemove(card);
            sbMenu->Close();
            break;
        }
        case SBMENU_ADD_CMD:
        {
            MTGCard * card = mView->getActiveCard();
            if (card)
                CMDaddRemove(card);
            sbMenu->Close();
            break;
        }
        case SBMENU_ADD_DNG:
        {
            MTGCard * card = mView->getActiveCard();
            if (card)
                DNGaddRemove(card);
            sbMenu->Close();
            break;
        }
        case SBMENU_ADD_CANCEL:
            sbMenu->Close();
            break;
        }
    }
}

/*
  to keep actions consistent across the different platforms, we need to branch the way swipes are interpreted.  iOS5 changes
 the way a swipe moves a document on the page.  swipe down is to simulate dragging the page down instead of moving down
 on a page.
 */
void GameStateDeckViewer::OnScroll(int inXVelocity, int inYVelocity)
{
    int magnitude = static_cast<int>( sqrtf( (float )( (inXVelocity * inXVelocity) + (inYVelocity * inYVelocity))));
    
    bool flickHorizontal = (abs(inXVelocity) > abs(inYVelocity));
    bool flickUp = !flickHorizontal && (inYVelocity > 0) ? true : false;
    bool flickRight = flickHorizontal && (inXVelocity < 0) ? true : false;
    
    if (flickHorizontal)
    {
        if(abs(inXVelocity) > 300)
        {
            //FIXME: this 500 is a bit arbitrary
            int numCards = (magnitude / 500) % 8;
            mView->changePositionAnimated(flickRight ? numCards : - numCards);
        }
    }
    else
    {
        if(abs(inYVelocity) > 300)
        {
            //FIXME: this 500 is a bit arbitrary
            int numFilters = (magnitude / 500);
            mView->changeFilterAnimated(flickUp ? numFilters : - numFilters);
        }
    }

    last_user_activity = 0;
}
