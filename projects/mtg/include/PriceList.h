#ifndef _PRICELIST_H_
#define _PRICELIST_H_

#include<string>
#include "../include/MTGDefinitions.h"
#include "../include/MTGDeck.h"
#include <stdio.h>

class MTGAllCards;

class Price{
 public:
  int cardid;
  int price;
  Price(int _cardid, int _price);
};

class PriceList{
 private:
  MTGAllCards * collection;
  string filename;
  Price * prices[Constants::TOTAL_NUMBER_OF_CARDS];
  int nbprices;
 public:
  PriceList(const char * file, MTGAllCards * _collection);
  ~PriceList();
  int save();
  int getPrice(int cardId);
  int setPrice(int cardId, int price);

};

#endif
