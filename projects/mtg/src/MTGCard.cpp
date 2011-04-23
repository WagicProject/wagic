//------------------------------------------------------
//MTGCard Class
//-------------------------------------------------
//TODO Fill BasicAbilities

#include "PrecompiledHeader.h"

#include "MTGDeck.h"
#include "MTGCard.h"
#include "CardPrimitive.h"
#include "Subtypes.h"
#include "Translate.h"

using std::string;

SUPPORT_OBJECT_ANALYTICS(MTGCard)

MTGCard::MTGCard()
{
    init();
}

MTGCard::MTGCard(int set_id)
{
    init();
    setId = set_id;
}

MTGCard::MTGCard(MTGCard * source)
{
    strcpy(image_name, source->image_name);
    rarity = source->rarity;
    mtgid = source->mtgid;
    setId = source->setId;
    data = source->data;
}

MTGCard::~MTGCard()
{
}

int MTGCard::init()
{
    setId = 0;
    mtgid = 0;
    data = NULL;
    rarity = Constants::RARITY_C;
    return 1;
}

void MTGCard::setMTGId(int id)
{
    mtgid = id;
    if (id < 0)
    {
        sprintf(image_name, "%dt.jpg", -mtgid);
    }
    else
    {
        sprintf(image_name, "%d.jpg", mtgid);
    }
}

int MTGCard::getMTGId() const
{
    return mtgid;
}
int MTGCard::getId() const
{
    return mtgid;
}
char MTGCard::getRarity() const
{
    return rarity;
}

void MTGCard::setRarity(char _rarity)
{
    rarity = _rarity;
}

char * MTGCard::getImageName()
{
    return image_name;
}

void MTGCard::setPrimitive(CardPrimitive * cp)
{
    data = cp;
}

const vector<string>& MTGCard::GetFormattedText()
{
    if (mFormattedText.empty())
    {
        if (data != NULL)
        {
            std::string s = data->text;
            std::string::size_type found = s.find_first_of("{}");
            while (found != string::npos)
            {
                s[found] = '/';
                found = s.find_first_of("{}", found + 1);
            }
            WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAGIC_FONT);
            mFont->FormatText(s, mFormattedText);
        }
    }
    return mFormattedText;
}