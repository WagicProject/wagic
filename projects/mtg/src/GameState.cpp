#include "PrecompiledHeader.h"

#include "Translate.h"
#include "GameState.h"
#include "Player.h"
#include "SimpleMenu.h"
#include "DeckStats.h"
#include "DeckMetaData.h"
#include "Player.h"

// The purpose of this method is to create a listing of decks to be used for the input menu
// by default, the list will be sorted by name
// TODO: revise sorting strategy to allow other types of sorting.  Currently, it is hardwired to use
//    sortByName to do the sorting.  This was done since the menu item display is done in insertion order.

vector<DeckMetaData *> GameState::fillDeckMenu(SimpleMenu * _menu, const string& path, const string& smallDeckPrefix,
                Player * statsPlayer)
{
    bool translate = _menu->autoTranslate;
    _menu->autoTranslate = false;
    vector<DeckMetaData *> deckMetaDataVector = getValidDeckMetaData(path, smallDeckPrefix, statsPlayer);
    renderDeckMenu(_menu, deckMetaDataVector);
    _menu->autoTranslate = translate;

    return deckMetaDataVector;
}

vector<DeckMetaData *> GameState::fillDeckMenu(DeckMenu * _menu, const string& path, const string& smallDeckPrefix,
                Player * statsPlayer)
{
    bool translate = _menu->mAutoTranslate;
    _menu->mAutoTranslate = false;
    vector<DeckMetaData *> deckMetaDataVector = getValidDeckMetaData(path, smallDeckPrefix, statsPlayer);
    renderDeckMenu(_menu, deckMetaDataVector);
    _menu->mAutoTranslate = translate;

    return deckMetaDataVector;
}

vector<DeckMetaData *> GameState::getValidDeckMetaData(const string& path, const string& smallDeckPrefix, Player * statsPlayer)
{
    vector<DeckMetaData*> retList;

    DeckMetaDataList * metas = DeckMetaDataList::decksMetaData;
    int found = 1;
    int nbDecks = 1;
    while (found)
    {
        found = 0;
        std::ostringstream filename;
        filename << path << "/deck" << nbDecks << ".txt";
        DeckMetaData * meta = metas->get(filename.str(), statsPlayer);
        if (meta)
        {
            found = 1;
            if (statsPlayer)
            {
                std::ostringstream smallDeckName;
                smallDeckName << smallDeckPrefix << "_deck" << nbDecks;
                meta->loadStatsForPlayer(statsPlayer, smallDeckName.str());
            }
            else
            {
                std::ostringstream playerStatsDeckName;
                playerStatsDeckName << "stats/player_deck" << nbDecks << ".txt";
                string deckstats = options.profileFile(playerStatsDeckName.str());
                meta->loadStatsForPlayer(NULL, deckstats);
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
        //int deckId = deckMetaData -> getDeckId(); //do we need this?
        _menu->Add(deckNumber++, deckName.c_str(), deckDescription.c_str(), false, deckMetaData);
    }
}

// deck sorting routines
bool sortByName(DeckMetaData * d1, DeckMetaData * d2)
{
    return strcmp(d1->getName().c_str(), d2->getName().c_str()) < 0;

}

//end deck sorting routine
