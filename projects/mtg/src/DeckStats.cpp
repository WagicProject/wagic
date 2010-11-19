#include "PrecompiledHeader.h"

#include "DeckStats.h"
#include "Player.h"
#include "GameObserver.h"
#include "MTGDeck.h"
#include "ManaCostHybrid.h"

DeckStats * DeckStats::mInstance = NULL;
    
DeckStat::DeckStat(int _nbgames, int _victories) : nbgames(_nbgames), victories(_victories)
{
}

int DeckStat::percentVictories()
{
    if (nbgames == 0) return 50;
    return (100 * victories / nbgames);
}

DeckStats * DeckStats::GetInstance()
{
    if (!mInstance)
    {
        mInstance = NEW DeckStats();

    }
    return mInstance;
}

void DeckStats::cleanStats()
{
    map<string, DeckStat *>::iterator it;
    for (it = stats.begin(); it != stats.end(); it++)
    {
        SAFE_DELETE(it->second);
    }

    stats.clear();
}

DeckStats::~DeckStats()
{
    cleanStats();
}

int DeckStats::percentVictories(string opponentsFile)
{
    map<string, DeckStat *>::iterator it = stats.find(opponentsFile);
    if (it == stats.end())
    {
        return 50;
    }
    else
    {
        return (it->second->percentVictories());
    }
}

DeckStat* DeckStats::getDeckStat(string opponentsFile)
{
    map<string, DeckStat *>::iterator it = stats.find(opponentsFile);
    if (it == stats.end())
    {
        return NULL;
    }
    else
    {
        return it->second;
    }
}

int DeckStats::nbGames()
{
    int nbgames = 0;
    map<string, DeckStat *>::iterator it;
    for (it = stats.begin(); it != stats.end(); it++)
    {
        DeckStat * d = it->second;
        nbgames += d->nbgames;
    }
    return nbgames;
}

int DeckStats::nbVictories()
{
    int victories = 0;
    int nbgames = 0;
    map<string, DeckStat *>::iterator it;
    for (it = stats.begin(); it != stats.end(); it++)
    {
        DeckStat * d = it->second;
        nbgames += d->nbgames;
        victories += d->victories;
    }

    return victories;
}

int DeckStats::percentVictories()
{
    int victories = 0;
    int nbgames = 0;
    map<string, DeckStat *>::iterator it;
    for (it = stats.begin(); it != stats.end(); it++)
    {
        DeckStat * d = it->second;
        nbgames += d->nbgames;
        victories += d->victories;
    }
    if (nbgames)
    {
        return (victories * 100) / nbgames;
    }
    return 50;
}

void DeckStats::load(Player * player)
{
    char filename[512];
    sprintf(filename, "stats/%s.txt", player->deckFileSmall.c_str());
    load(options.profileFile(filename).c_str());
}

void DeckStats::load(const char * filename)
{
    cleanStats();
    std::ifstream file(filename);
    std::string s;

    if (file)
    {
        while (std::getline(file, s))
        {
            string deckfile = s;
            std::getline(file, s);
            int games = atoi(s.c_str());
            std::getline(file, s);
            int victories = atoi(s.c_str());
            map<string, DeckStat *>::iterator it = stats.find(deckfile);
            if (it == stats.end())
            {
                stats[deckfile] = NEW DeckStat(games, victories);
            }
        }
        file.close();
    }
}

void DeckStats::save(Player * player)
{
    char filename[512];
    sprintf(filename, "stats/%s.txt", player->deckFileSmall.c_str());
    save(options.profileFile(filename).c_str());
}

void DeckStats::save(const char * filename)
{
    std::ofstream file(filename);
    char writer[512];
    if (file)
    {
        map<string, DeckStat *>::iterator it;
        for (it = stats.begin(); it != stats.end(); it++)
        {
            sprintf(writer, "%s\n", it->first.c_str());
            file << writer;
            sprintf(writer, "%i\n", it->second->nbgames);
            file << writer;
            sprintf(writer, "%i\n", it->second->victories);
            file << writer;
        }
        file.close();
    }
}

void DeckStats::saveStats(Player *player, Player *opponent, GameObserver * game)
{
    int victory = 1;
    if (!game->gameOver)
    {
        if (player->life == opponent->life) return;
        if (player->life < opponent->life) victory = 0;
    }
    else if (game->gameOver == player)
    {
        victory = 0;
    }
    load(player);
    map<string, DeckStat *>::iterator it = stats.find(opponent->deckFileSmall);
    if (it == stats.end())
    {
        stats[opponent->deckFileSmall] = NEW DeckStat(1, victory);
    }
    else
    {
        it->second->victories += victory;
        it->second->nbgames += 1;
    }
    save(player);
}
StatsWrapper::StatsWrapper(int deckId)
{
    mDeckId = deckId;
    char buffer[512];
    sprintf(buffer, "stats/player_deck%i.txt", deckId);
    string deckstats = options.profileFile(buffer);
    initStatistics(deckstats);
}

StatsWrapper::StatsWrapper(string deckstats)
{
    initStatistics(deckstats);
}

void StatsWrapper::initStatistics(string deckstats)
{
    // Load deck statistics
    DeckStats * stats = DeckStats::GetInstance();
    aiDeckNames.clear();
    aiDeckStats.clear();

    if (fileExists(deckstats.c_str()))
    {
        stats->load(deckstats.c_str());
        percentVictories = stats->percentVictories();
        gamesPlayed = stats->nbGames();

        // Detailed deck statistics against AI
        int found = 1;
        int nbDecks = 0;
        found = 0;
        char buffer[512];
        char smallDeckName[512];
        sprintf(buffer, "%s/deck%i.txt", RESPATH"/ai/baka", nbDecks + 1);
        if (fileExists(buffer))
        {
            MTGDeck * mtgd = NEW MTGDeck(buffer, NULL, 1);
            found = 1;
            nbDecks++;

            sprintf(smallDeckName, "%s_deck%i", "ai_baka", nbDecks);
            DeckStat* deckStat = stats->getDeckStat(string(smallDeckName));

            if ((deckStat != NULL) && (deckStat->nbgames > 0))
            {
                int percentVictories = stats->percentVictories(string(smallDeckName));
                aiDeckNames.push_back(string(mtgd->meta_name));
                aiDeckStats.push_back(deckStat);
            }

            delete mtgd;
        }
    }
    else
    {
        gamesPlayed = 0;
        percentVictories = 0;
    }
}

void StatsWrapper::updateStats(string filename, MTGAllCards *collection)
{
    if (fileExists(filename.c_str()))
    {
        MTGDeck * mtgd = NEW MTGDeck(filename.c_str(), collection);
        DeckDataWrapper *deckDataWrapper = NEW DeckDataWrapper(mtgd);
        updateStats(deckDataWrapper);
        SAFE_DELETE( mtgd );
        SAFE_DELETE( deckDataWrapper );
    }

}

void StatsWrapper::updateStats(DeckDataWrapper *myDeck)
{
    if (!this->needUpdate || !myDeck) return;
    this->needUpdate = false;
    this->cardCount = myDeck->getCount(WSrcDeck::UNFILTERED_COPIES);
    this->countLands = myDeck->getCount(Constants::MTG_COLOR_LAND);
    this->totalPrice = myDeck->totalPrice();

    this->countManaProducers = 0;
    // Mana cost
    int currentCount, convertedCost;
    ManaCost * currentCost;
    this->totalManaCost = 0;
    this->totalCreatureCost = 0;
    this->totalSpellCost = 0;
    MTGCard * current = myDeck->getCard();

    // Clearing arrays
    for (int i = 0; i <= Constants::STATS_MAX_MANA_COST; i++)
    {
        this->countCardsPerCost[i] = 0;
        this->countCreaturesPerCost[i] = 0;
        this->countSpellsPerCost[i] = 0;
    }

    for (int i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        this->totalCostPerColor[i] = 0;
        this->countLandsPerColor[i] = 0;
        this->countBasicLandsPerColor[i] = 0;
        this->countNonLandProducersPerColor[i] = 0;
    }

    for (int i = 0; i <= Constants::STATS_MAX_MANA_COST; i++)
    {
        for (int k = 0; k <= Constants::MTG_NB_COLORS; k++)
        {
            this->countCardsPerCostAndColor[i][k] = 0;
            this->countCreaturesPerCostAndColor[i][k] = 0;
            this->countSpellsPerCostAndColor[i][k] = 0;
        }
    }

    for (int ic = 0; ic < myDeck->Size(true); ic++)
    {
        current = myDeck->getCard(ic, true);
        currentCost = current->data->getManaCost();
        convertedCost = currentCost->getConvertedCost();
        currentCount = myDeck->count(current);

        // Add to the cards per cost counters
        this->totalManaCost += convertedCost * currentCount;
        if (convertedCost > Constants::STATS_MAX_MANA_COST)
        {
            convertedCost = Constants::STATS_MAX_MANA_COST;
        }
        this->countCardsPerCost[convertedCost] += currentCount;
        if (current->data->isCreature())
        {
            this->countCreaturesPerCost[convertedCost] += currentCount;
            this->totalCreatureCost += convertedCost * currentCount;
        }
        else if (current->data->isSpell())
        {
            this->countSpellsPerCost[convertedCost] += currentCount;
            this->totalSpellCost += convertedCost * currentCount;
        }

        // Lets look for mana producing abilities

        vector<string> abilitiesVector;
        string thisstring = current->data->magicText;
        abilitiesVector = split(thisstring, '\n');

        for (int v = 0; v < (int) abilitiesVector.size(); v++)
        {
            string s = abilitiesVector[v];
            size_t t = s.find("add");
            if (t != string::npos)
            {
                s = s.substr(t + 3);
                ManaCost * mc = ManaCost::parseManaCost(s);
                for (int j = 0; j < Constants::MTG_NB_COLORS; j++)
                {
                    if (mc->hasColor(j))
                    {
                        if (current->data->isLand())
                        {
                            if (current->data->hasType("Basic"))
                            {
                                this->countBasicLandsPerColor[j] += currentCount;
                            }
                            else
                            {
                                this->countLandsPerColor[j] += currentCount;
                            }
                        }
                        else
                        {
                            this->countNonLandProducersPerColor[j] += currentCount;
                        }
                    }
                }
                SAFE_DELETE(mc);
            }
        }

        // Add to the per color counters
        //  a. regular costs
        for (int j = 0; j < Constants::MTG_NB_COLORS; j++)
        {
            this->totalCostPerColor[j] += currentCost->getCost(j) * currentCount;
            if (current->data->hasColor(j))
            {
                // Add to the per cost and color counter
                this->countCardsPerCostAndColor[convertedCost][j] += currentCount;
                if (current->data->isCreature())
                {
                    this->countCreaturesPerCostAndColor[convertedCost][j] += currentCount;
                }
                else if (current->data->isSpell())
                {
                    this->countSpellsPerCostAndColor[convertedCost][j] += currentCount;
                }
            }
        }

        //  b. Hybrid costs
        ManaCostHybrid * hybridCost;
        int i;
        i = 0;

        while ((hybridCost = currentCost->getHybridCost(i++)) != NULL)
        {
            this->totalCostPerColor[hybridCost->color1] += hybridCost->value1 * currentCount;
            this->totalCostPerColor[hybridCost->color2] += hybridCost->value2 * currentCount;
        }
    }

    this->totalColoredSymbols = 0;
    for (int j = 1; j < Constants::MTG_NB_COLORS; j++)
    {
        this->totalColoredSymbols += this->totalCostPerColor[j];
    }

    this->countCardsPerCost[0] -= this->countLands;

    // Counts by type
    this->countCreatures = countCardsByType("Creature", myDeck);
    this->countInstants = countCardsByType("Instant", myDeck);
    this->countEnchantments = countCardsByType("Enchantment", myDeck);
    this->countSorceries = countCardsByType("Sorcery", myDeck);
    this->countSpells = this->countInstants + this->countEnchantments + this->countSorceries;
    //this->countArtifacts = countCardsByType("Artifact", myDeck);

    // Average mana costs
    this->avgManaCost = ((this->cardCount - this->countLands) <= 0) ? 0 : (float) this->totalManaCost / (this->cardCount
            - this->countLands);
    this->avgCreatureCost = (this->countCreatures <= 0) ? 0 : (float) this->totalCreatureCost / this->countCreatures;
    this->avgSpellCost = (this->countSpells <= 0) ? 0 : (float) this->totalSpellCost / this->countSpells;

    // Probabilities
    // TODO: this could be optimized by reusing results
    for (int i = 0; i < Constants::STATS_FOR_TURNS; i++)
    {
        this->noLandsProbInTurn[i] = noLuck(this->cardCount, this->countLands, 7 + i) * 100;
        this->noCreaturesProbInTurn[i] = noLuck(this->cardCount, this->countCreatures, 7 + i) * 100;
    }
}

// This should probably be cached in DeckDataWrapper
// or at least be calculated for all common types in one go
int StatsWrapper::countCardsByType(const char * _type, DeckDataWrapper * myDeck)
{
    int result = 0;
    for (int i = 0; i < myDeck->Size(true); i++)
    {
        MTGCard * current = myDeck->getCard(i, true);
        if (current->data->hasType(_type))
        {
            result += myDeck->count(current);
        }
    }
    return result;
}

// n cards total, a of them are of desired type (A), x drawn
// returns probability of no A's
float StatsWrapper::noLuck(int n, int a, int x)
{
    if ((a >= n) || (a == 0)) return 1;
    if ((n == 0) || (x == 0) || (x > n) || (n - a < x)) return 0;

    a = n - a;
    float result = 1;

    for (int i = 0; i < x; i++)
        result *= (float) (a - i) / (n - i);
    return result;
}

StatsWrapper::~StatsWrapper()
{
    aiDeckNames.clear();
    aiDeckStats.clear();
}

