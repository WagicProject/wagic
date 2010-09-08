#include <string>
#include <vector>

using namespace std;


class DeckManager
{
protected: 
    static DeckManager * mInstance;

public:
    

    vector<int> playerDeckOrderList;
    vector<int> aiDeckOrderList;


    vector<int> * getPlayerDeckOrderList();
    vector<int> * getAIDeckOrderList();
        
    static DeckManager * GetInstance();
    static void EndInstance();

    DeckManager();
    ~DeckManager();
};
