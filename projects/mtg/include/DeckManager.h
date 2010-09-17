#include <string>
#include <vector>
#include "DeckMetaData.h"

using namespace std;


class DeckManager
{
protected: 
    static DeckManager * mInstance;

public:
    

    vector<DeckMetaData *> playerDeckOrderList;
    vector<DeckMetaData *> aiDeckOrderList;

    void updateMetaDataList(vector<DeckMetaData *>* refList, bool isAI );
    vector<DeckMetaData *> * getPlayerDeckOrderList();
    vector<DeckMetaData *> * getAIDeckOrderList();
        
    static DeckManager * GetInstance();
    static void EndInstance();

    DeckManager();
    ~DeckManager();
};
