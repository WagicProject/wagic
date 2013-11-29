#ifndef _DECKSTATS_H_
#define _DECKSTATS_H_

#include <map>
#include <string>
#include <vector>
#include "MTGDefinitions.h"
#include <DeckDataWrapper.h>

using namespace std;

class Player;
class GameObserver;

class DeckStat
{
public:
    DeckStat(int _nbgames = 0, int _victories = 0, string manaColorIndex = "");

    int nbgames;
    int victories;
    string manaColorIndex;
    int percentVictories();
};

class DeckStats
{
protected:
    static DeckStats * mInstance;
public:
    string currentDeck;
    map<string, map<string, DeckStat*> > masterDeckStats;
    
    static DeckStats * GetInstance();
    static void EndInstance();
    void saveStats(Player * player, Player * opponent, GameObserver * game);
    void save(const std::string& filename);
    void load(const std::string& filename);

    ~DeckStats();
    int percentVictories(string opponentsDeckFile);
    int percentVictories();
    DeckStat * getDeckStat(string opponentsFile);

    //returns the total number of games played with this deck
    int nbGames();
};

class StatsWrapper
{
private:
    void initValues();

public:
    StatsWrapper(int deckId);
    StatsWrapper(string filename);
    ~StatsWrapper();

    void initStatistics(string deckstats);

    // Stats parameters and status
    int mDeckId;
    int currentPage;
    int pageCount;
    bool needUpdate;

    // Actual stats
    int percentVictories;
    int gamesPlayed;
    int cardCount;
    int countLands;
    int totalPrice;
    int totalManaCost;
    float avgManaCost;
    int totalCreatureCost;
    float avgCreatureCost;
    int totalSpellCost;
    float avgSpellCost;
    int countManaProducers;

    int countCreatures, countSpells, countInstants, countEnchantments, countSorceries, countArtifacts;

    float noLandsProbInTurn[Constants::STATS_FOR_TURNS];
    float noCreaturesProbInTurn[Constants::STATS_FOR_TURNS];

    int countCardsPerCost[Constants::STATS_MAX_MANA_COST + 1];
    int countCardsPerCostAndColor[Constants::STATS_MAX_MANA_COST + 1][Constants::MTG_NB_COLORS + 1];
    int countCreaturesPerCost[Constants::STATS_MAX_MANA_COST + 1];
    int countCreaturesPerCostAndColor[Constants::STATS_MAX_MANA_COST + 1][Constants::MTG_NB_COLORS + 1];
    int countSpellsPerCost[Constants::STATS_MAX_MANA_COST + 1];
    int countSpellsPerCostAndColor[Constants::STATS_MAX_MANA_COST + 1][Constants::MTG_NB_COLORS + 1];
    int countLandsPerColor[Constants::MTG_NB_COLORS + 1];
    int countBasicLandsPerColor[Constants::MTG_NB_COLORS + 1];
    int countNonLandProducersPerColor[Constants::MTG_NB_COLORS + 1];
    int totalCostPerColor[Constants::MTG_NB_COLORS + 1];
    int totalColoredSymbols;
    
    string getManaColorIndex();
    void updateStats(string filename, MTGAllCards * collection);
    void updateStats(DeckDataWrapper *mtgDeck);
    int countCardsByType(const string& _type, DeckDataWrapper * myDeck);
    float noLuck(int n, int a, int x);

    vector<string> aiDeckNames;
    vector<DeckStat*> aiDeckStats;
};

#endif
