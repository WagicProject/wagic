#ifndef _MTGCARD_H_
#define _MTGCARD_H_

#define MTGCARD_NAME_SIZE 16

#define MTG_IMAGE_WIDTH 200
#define MTG_IMAGE_HEIGHT 285

#define MTG_MINIIMAGE_WIDTH 45
#define MTG_MINIIMAGE_HEIGHT 64

#include <string>
#include <vector>
#include <map>

class CardPrimitive;

using namespace std;

class MTGCard {
 protected:
   friend class MTGSetInfo;
  int mtgid;
  char rarity;
  char image_name[MTGCARD_NAME_SIZE];
  int  init();

 public:

  int setId;
  CardPrimitive * data;
 
  MTGCard();
  MTGCard(int set_id);
  MTGCard(MTGCard * source);

  void setMTGId(int id);
  int getMTGId();
  int getId();


  char getRarity();
  void setRarity(char _rarity);

  //void setImageName( char * value);
  char * getImageName ();

  void setPrimitive(CardPrimitive * cp);

};




#endif
