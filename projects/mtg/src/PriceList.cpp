#include "../include/config.h"
#include "../include/PriceList.h"


Price::Price(int _cardid, int _price): cardid(_cardid),price(_price){
}



PriceList::PriceList(const char * _filename, MTGAllCards * _collection):collection(_collection){
  nbprices = 0;
  filename = _filename;
  std::ifstream file(_filename);
  std::string cardid;
  std::string price;
  if(file){
    while(std::getline(file,cardid)){
      std::getline(file,price);
      prices[nbprices]= NEW Price(atoi(cardid.c_str()), atoi(price.c_str()));
      nbprices++;
    }
    file.close();
  }
}


PriceList::~PriceList(){
  for (int i = 0; i < nbprices; i++){
    delete (prices[i]);
  }
  nbprices = 0;
}

int PriceList::save(){
  std::ofstream file(filename.c_str());
  char writer[20];
  if (file){
    for (int i = 0; i<nbprices; i++){
      sprintf(writer,"%i\n%i\n", prices[i]->cardid, prices[i]->price);
      file<<writer;
    }
    file.close();
  }

  return 1;
}
int PriceList::getPrice(int cardId){
  for(int i = 0; i < nbprices; i++){
    if (prices[i]->cardid == cardId){
      return prices[i]->price;
    }
  }
  char rarity = collection->getCardById(cardId)->getRarity();
  switch(rarity){
  case RARITY_M:
    return 3000;
    break;
  case RARITY_R:
    return 500;
    break;
  case RARITY_U:
    return 100;
    break;
  case RARITY_C:
    return 20;
    break;
  case RARITY_L:
    return 5;
    break;
  default:
    return 20;
    break;

  }

}

int PriceList::setPrice(int cardId, int price){
  for(int i = 0; i < nbprices; i++){
    if (prices[i]->cardid == cardId){
      prices[i]->price = price;
      return prices[i]->price;
    }
  }
  prices[nbprices] = NEW Price(cardId, price);
  nbprices++;
  return prices[nbprices-1]->price;
}
