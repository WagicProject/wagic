#include "../include/config.h"
#include "../include/PriceList.h"

int PriceList::randomKey = 0;

PriceList::PriceList(const char * _filename, MTGAllCards * _collection):collection(_collection){
  filename = _filename;
  std::ifstream file(_filename);
  std::string cardid;
  std::string price;
  if(file){
    while(std::getline(file,cardid)){
      std::getline(file,price);
      prices[atoi(cardid.c_str())]= atoi(price.c_str());
    }
    file.close();
  }
  if(randomKey == 0)
    randomKey = rand();
}


PriceList::~PriceList(){
}

int PriceList::save(){
  std::ofstream file(filename.c_str());
  char writer[20];
  if (file){
      map<int,int>::iterator it=prices.begin();
      while(it != prices.end()){
        sprintf(writer,"%i\n%i\n", (*it).first, (*it).second);
        it++;
        file<<writer;
    }
    file.close();
  }

  return 1;
}
int PriceList::getPrice(int cardId){
  map<int,int>::iterator it = prices.find(cardId);
  if (it != prices.end()) return (*it).second;

  char rarity = collection->getCardById(cardId)->getRarity();
  switch(rarity){
  case Constants::RARITY_M:
    return Constants::PRICE_1M;
    break;
  case Constants::RARITY_R:
    return Constants::PRICE_1R;
    break;
  case Constants::RARITY_U:
    return Constants::PRICE_1U;
    break;
  case Constants::RARITY_C:
    return Constants::PRICE_1C;
    break;
  case Constants::RARITY_L:
    return Constants::PRICE_1L;
    break;
  default:
    return Constants::PRICE_1C;
    break;
  }

}

int PriceList::setPrice(int cardId, int price){
  prices[cardId] = price;
  return price;
}
int PriceList::getSellPrice(int cardid){
  return getPurchasePrice(cardid) / 2;
}
int PriceList::getPurchasePrice(int cardid){
  float price = (float) getPrice(cardid);
  int rnd = abs(cardid + randomKey) % (1+Constants::PRICE_FLUX_RANGE);
  price += price * (float)(rnd - Constants::PRICE_FLUX_MINUS)/100.0f;
  return price;
}
