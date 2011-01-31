#include "PrecompiledHeader.h"

#include "Translate.h"
#include "GameState.h"
#include "Player.h"
#include "SimpleMenu.h"
#include "DeckStats.h"
#include "DeckMetaData.h"
#include "DeckManager.h"
#include "Player.h"

// The purpose of this method is to create a listing of decks to be used for the input menu
// by default, the list will be sorted by name
// TODO: revise sorting strategy to allow other types of sorting.  Currently, it is hardwired to use
//    sortByName to do the sorting.  This was done since the menu item display is done in insertion order.

vector<DeckMetaData *> GameState::fillDeckMenu(SimpleMenu * _menu, const string& path, const string& smallDeckPrefix,
                Player * statsPlayer)
{

    vector<DeckMetaData *> deckMetaDataVector = BuildDeckList(path, smallDeckPrefix, statsPlayer);
    renderDeckMenu(_menu, deckMetaDataVector);

    return deckMetaDataVector;
}

vector<DeckMetaData *> GameState::fillDeckMenu(DeckMenu * _menu, const string& path, const string& smallDeckPrefix,
                Player * statsPlayer, int maxDecks)
{

    vector<DeckMetaData *> deckMetaDataVector = BuildDeckList(path, smallDeckPrefix, statsPlayer, maxDecks);
    renderDeckMenu(_menu, deckMetaDataVector);

    return deckMetaDataVector;
}

vector<DeckMetaData *> GameState::BuildDeckList(const string& path, const string& smallDeckPrefix, Player * statsPlayer, int maxDecks)
{
    vector<DeckMetaData*> retList;

    int found = 1;
    int nbDecks = 1;
    DeckManager *deckManager = DeckManager::GetInstance();
    bool isAI = path.find("baka") != string::npos;
    while (found && (!maxDecks || nbDecks <= maxDecks))
    {
        found = 0;
        std::ostringstream filename;
        filename << path << "/deck" << nbDecks << ".txt";
        DeckMetaData * meta = deckManager->getDeckMetaDataByFilename(filename.str(), isAI);

        if (meta)
        {
            found = 1;
            if (statsPlayer)
            {
                std::ostringstream aiStatsDeckName;
                aiStatsDeckName << smallDeckPrefix << "_deck" << nbDecks;
				meta->mStatsFilename = aiStatsDeckName.str();
                meta->mIsAI = true;
                if (meta->mPlayerDeck != statsPlayer->GetCurrentDeckStatsFile())
                {
                    meta->mPlayerDeck = statsPlayer->GetCurrentDeckStatsFile();
                    meta->Invalidate();
                }
            }
            else
            {
                std::ostringstream playerStatsDeckName;
                playerStatsDeckName << "stats/player_deck" << nbDecks << ".txt";
                meta->mStatsFilename = options.profileFile(playerStatsDeckName.str());
                meta->mIsAI = false;
            }

            retList.push_back(meta);
            nbDecks++;
        }
        meta = NULL;
    }

	std::sort(retList.begin(), retList.end(), sortByName);
    return retList;

}

// build a menu with the given deck list and return a vector of the deck ids created.
void GameState::renderDeckMenu(SimpleMenu * _menu, const vector<DeckMetaData *>& deckMetaDataList)
{
    int deckNumber = 1;
    for (vector<DeckMetaData *>::const_iterator i = deckMetaDataList.begin(); i != deckMetaDataList.end(); i++)
    {
        DeckMetaData * deckMetaData = *i;
        string deckName = deckMetaData -> getName();
        string deckDescription = deckMetaData -> getDescription();
        _menu->Add(deckNumber++, deckName.c_str(), deckDescription.c_str());
    }
}

// build a menu with the given deck list and return a vector of the deck ids created.
void GameState::renderDeckMenu(DeckMenu * _menu, const vector<DeckMetaData *>& deckMetaDataList)
{
    int deckNumber = 1;
    for (vector<DeckMetaData *>::const_iterator i = deckMetaDataList.begin(); i != deckMetaDataList.end(); i++)
    {
        DeckMetaData * deckMetaData = *i;
        string deckName = deckMetaData -> getName();
        string deckDescription = deckMetaData -> getDescription();
        _menu->Add(deckNumber++, deckName.c_str(), deckDescription.c_str(), false, deckMetaData);
    }
}

// deck sorting routines
bool sortByName(DeckMetaData * d1, DeckMetaData * d2)
{
    return strcmp(d1->getName().c_str(), d2->getName().c_str()) < 0;

}

//end deck sorting routine
