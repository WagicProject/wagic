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

    void updateMetaDataList(vector<DeckMetaData *>* refList, bool isAI);
    vector<DeckMetaData *> * getPlayerDeckOrderList();
    vector<DeckMetaData *> * getAIDeckOrderList();

    static DeckManager * GetInstance();
    static void EndInstance();

    //convenience method to get the difficulty rating between two decks.  This should be refined a little more
    //since the eventual move of all deck meta data should be managed by this class

    static int getDifficultyRating(Player *statsPlayer, Player *player);

    ~DeckManager()
    {
        instanceFlag = false;
    }

};
