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

    void updateMetaDataList(vector<DeckMetaData *>* refList, bool isAI );
    vector<DeckMetaData *> * getPlayerDeckOrderList();
    vector<DeckMetaData *> * getAIDeckOrderList();
        
    static DeckManager * GetInstance();
    static void EndInstance();

   
    ~DeckManager()
    {
        instanceFlag = false;
    }

};
