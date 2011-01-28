#include <string>
#include <vector>

#include "DeckMetaData.h"

using namespace std;

class DeckManager
{
private:
    static bool instanceFlag;
    static DeckManager *mInstance;
    DeckManager()
    {
        //private constructor
    }

public:

    vector<DeckMetaData *> playerDeckOrderList;
    vector<DeckMetaData *> aiDeckOrderList;
    
    map<string, StatsWrapper *> playerDeckStatsMap;
    map<string, StatsWrapper *> aiDeckStatsMap;
    
    void updateMetaDataList(vector<DeckMetaData *>* refList, bool isAI);
    vector<DeckMetaData *> * getPlayerDeckOrderList();
    vector<DeckMetaData *> * getAIDeckOrderList();

    DeckMetaData * getDeckMetaDataById( int deckId, bool isAI );
    StatsWrapper * getExtendedStatsForDeckId( int deckId, MTGAllCards *collection, bool isAI );
    StatsWrapper * getExtendedDeckStats( DeckMetaData *selectedDeck, MTGAllCards *collection, bool isAI );
    static DeckManager * GetInstance();
    static void EndInstance();

    //convenience method to get the difficulty rating between two decks.  This should be refined a little more
    //since the eventual move of all deck meta data should be managed by this class

    static int getDifficultyRating(Player *statsPlayer, Player *player);
    
    ~DeckManager();

};
