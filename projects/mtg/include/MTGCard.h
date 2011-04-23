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

#include "ObjectAnalytics.h"

class CardPrimitive;

using namespace std;

class MTGCard
#ifdef TRACK_OBJECT_USAGE
    : public InstanceCounter<MTGCard>
#endif
{
protected:
    friend class MTGSetInfo;
    int mtgid;
    char rarity;
    char image_name[MTGCARD_NAME_SIZE];
    vector<string> mFormattedText;
    int init();

public:
    int setId;
    CardPrimitive * data;

    MTGCard();
    MTGCard(int set_id);
    MTGCard(MTGCard * source);
    virtual ~MTGCard();

    void setMTGId(int id);
    void setRarity(char _rarity);
    //void setImageName( char * value);
    void setPrimitive(CardPrimitive * cp);

    const vector<string>& GetFormattedText();

    int getMTGId() const;
    int getId() const;
    char getRarity() const;
    char * getImageName();
};

#endif
