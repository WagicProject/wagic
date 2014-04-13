#ifndef _PRICELIST_H_
#define _PRICELIST_H_

#include<string>
#include "MTGDefinitions.h"
#include "MTGDeck.h"
#include <stdio.h>

class MTGAllCards;

class PriceList
{
private:
    MTGAllCards * collection;
    string filename;
    map<int, int> prices;
    static int randomKey;
public:
    PriceList(const char * file, MTGAllCards * _collection);
    ~PriceList();
    int save();
    int getSellPrice(int cardid);
    int getSellPrice(MTGCard* card);
    int getPurchasePrice(int cardid);
    int getPrice(MTGCard *card);
    int getPrice(int cardId);
    int setPrice(int cardId, int price);
    int setPrice(MTGCard *card, int price);
    int getOtherPrice(int amt);
    static float difficultyScalar(float price, int cardid = 0);
    static void updateKey()
    {
        randomKey = rand();
    }
};

#endif
