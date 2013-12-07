#include "PrecompiledHeader.h"

#include "PriceList.h"
#include "utils.h"
#include "MTGCard.h"

int PriceList::randomKey = 0;

PriceList::PriceList(const char * _filename, MTGAllCards * _collection) :
    collection(_collection)
{
    filename = _filename;
    std::string contents;
    if (JFileSystem::GetInstance()->readIntoString(filename, contents))
    {
        std::stringstream stream(contents);
        std::string cardid;
        std::string price;
        while (std::getline(stream, cardid))
        {
            std::getline(stream, price);
            prices[atoi(cardid.c_str())] = atoi(price.c_str());
        }
    }
    if (randomKey == 0) randomKey = rand();
}

PriceList::~PriceList()
{
}

int PriceList::save()
{
    std::ofstream file;
    if (JFileSystem::GetInstance()->openForWrite(file, filename))
    {
        char writer[20];
        map<int, int>::iterator it = prices.begin();
        while (it != prices.end())
        {
            sprintf(writer, "%i\n%i\n", (*it).first, (*it).second);
            it++;
            file << writer;
        }
        file.close();
    }

    return 1;
}
int PriceList::getPrice(MTGCard * card)
{
    map<int, int>::iterator it = prices.find(card->getId());
    if (it != prices.end()) return (*it).second;

    char rarity = card->getRarity();
    switch (rarity)
    {
    case Constants::RARITY_M:
        return Constants::PRICE_1M;
        break;
    case Constants::RARITY_S:
        return Constants::PRICE_1S;
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

int PriceList::getPrice(int cardId)
{
    return getPrice(collection->getCardById(cardId));
}

int PriceList::setPrice(int cardId, int price)
{
    prices[cardId] = price;
    return price;
}

int PriceList::setPrice(MTGCard * card, int price)
{
    prices[card->getId()] = price;
    return price;
}

int PriceList::getSellPrice(int cardid)
{
    return getPrice(collection->getCardById(cardid));
}

int PriceList::getSellPrice(MTGCard *card)
{
    return getPrice(card);
}

float PriceList::difficultyScalar(float price, int cardid)
{
    float badluck = (float) (abs(cardid + randomKey) % 201) / 100; //Float between 0 and 2.

    switch (options[Options::ECON_DIFFICULTY].number)
    {
    case Constants::ECON_EASY:
        badluck -= 1.5;
        price /= 2;
        break; //Price from .25x to .75x, .25x more likely.
    case Constants::ECON_NORMAL:
    default:
        badluck /= 2;
        break; //price from 1x to 2x, even probability.
    case Constants::ECON_HARD:
        price *= 1.5;
        break; //price from 1.5x to 3x, 3x being twice as likely.
    case Constants::ECON_LUCK:
        badluck += .25;
        return price * badluck;
        break; //Price from .25x to 2.25x, randomly.
    }
    if (badluck > 1)
        badluck = 1;
    else if (badluck < -1) badluck = -1;
    return (price + price * badluck);
}
int PriceList::getPurchasePrice(int cardid)
{
    float p = difficultyScalar((float) getPrice(cardid), cardid);
    if (p < 2) p = 2; //Prevents "Sell for 0 credits"
    return (int) p;
}
int PriceList::getOtherPrice(int amt)
{
    float p = difficultyScalar((float) amt, 0);
    if (p < 2) p = 2;
    return (int) p;
}
