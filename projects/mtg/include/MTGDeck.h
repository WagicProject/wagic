#ifndef _MTGDECK_H_
#define _MTGDECK_H_

#define MTG_ERROR -1

#include "MTGDefinitions.h"
#include "WResourceManager.h"
#include <dirent.h>
#include <Threading.h>
#include <Subtypes.h>
#include <string>

using std::string;
class GameApp;
class MTGCard;
class CardPrimitive;
class MTGPack;
class MTGSetInfo
{
public:
    MTGSetInfo(const string& _id);
    ~MTGSetInfo();
    string id; //Short name: 10E, RAV, etc. Automatic from folder.
    string author; //Author of set, for crediting mod makers, etc.
    string name; //Long name: Tenth Edition
    int block; //For future use by tournament mode, etc.
    int year; //The year the set was released.
    //TODO Way to group cards by name, rather than mtgid.

    void count(MTGCard * c);

    int totalCards();
    string getName();
    string getBlock();
    void processConfLine(string line);

    enum
    {
        //For memoized counts
        LAND = 0,
        COMMON = 1,
        UNCOMMON = 2,
        RARE = 3,
        MAX_RARITY = 4, //For boosters, mythic is part of rare... always.
        MYTHIC = 4,
        TOTAL_CARDS = 5,
        MAX_COUNT = 6
    };

    MTGPack * mPack; //Does it use a specialized booster pack?
    bool bZipped; //Is this set's images present as a zip file?
    bool bThemeZipped; //[...] in the theme?
    int counts[MTGSetInfo::MAX_COUNT];
};

class MTGSets
{
public:

    //These values have to be < 0
    // A setID with a value >=0 will be looked into the sets table,
    // Negative values will be compared to these enums throughout the code (shop, filters...)
    enum
    {
        INTERNAL_SET = -1, ALL_SETS = -2,
    };

    friend class MTGSetInfo;
    MTGSets();
    ~MTGSets();

    int Add(const string& subtype);
    int findSet(string value);
    int findBlock(string s);
    int size();

    int getSetNum(MTGSetInfo*i);

    int operator[](string id); //Returns set id index, -1 for failure.
    string operator[](int id); //Returns set id name, "" for failure.

    MTGSetInfo* getInfo(int setID);
    MTGSetInfo* randomSet(int blockId = -1, int atleast = -1); //Tries to match, otherwise 100% random unlocked set

protected:
    vector<string> blocks;
    vector<MTGSetInfo*> setinfo;
};

extern MTGSets setlist;

class MTGAllCards
{
private:
    MTGCard * tempCard; //used by parser
    CardPrimitive * tempPrimitive; //used by parser
    int currentGrade; //used by Parser (we don't want an additional attribute for the primitives for that as it is only used at load time)
    static MTGAllCards* instance;

protected:
    int conf_read_mode;
    vector <int> colorsCount;
    int total_cards;
    void init();
    void initCounters();
    MTGAllCards();
    ~MTGAllCards();

public:
    enum
    {
        READ_ANYTHING = 0,
        READ_CARD = 1,
        READ_METADATA = 2,
    };
    vector<int> ids;
    map<int, MTGCard *> collection;
    map<string, CardPrimitive *> primitives;
    MTGCard * _(int id);
    MTGCard * getCardById(int id);

#ifdef TESTSUITE
    void prefetchCardNameCache();
#endif

    MTGCard * getCardByName(string name);
    void loadFolder(const string& folder, const string& filename="" );

    int load(const string& config_file);
    int load(const string& config_file, const string& setName);
    int load(const string& config_file, int set_id);
    int countByType(const string& _type);
    int countByColor(int color);
    int countBySet(int setId);
    int totalCards();
    int randomCardId();

    static int findType(string subtype, bool forceAdd = true) {
        boost::mutex::scoped_lock lock(instance->mMutex);
        int result = instance->subtypesList.find(subtype, forceAdd);
        return result;
    };
    static int add(string value, unsigned int parentType) {
        boost::mutex::scoped_lock lock(instance->mMutex);
        int result = instance->subtypesList.add(value, parentType);
        return result;
    };
    static string findType(unsigned int id) {
        boost::mutex::scoped_lock lock(instance->mMutex);
        return instance->subtypesList.find(id);
    };
    static const vector<string>& getValuesById() {
        boost::mutex::scoped_lock lock(instance->mMutex);
        return instance->subtypesList.getValuesById();
    };
    static const vector<string>& getCreatureValuesById() {
        boost::mutex::scoped_lock lock(instance->mMutex);
        return instance->subtypesList.getCreatureValuesById();
    };
    static bool isSubtypeOfType(unsigned int subtype, unsigned int type) {
        boost::mutex::scoped_lock lock(instance->mMutex);
        return instance->subtypesList.isSubtypeOfType(subtype, type);
    };
    static bool isSuperType(unsigned int type) {
        boost::mutex::scoped_lock lock(instance->mMutex);
        return instance->subtypesList.isSuperType(type);
    };
    static bool isType(unsigned int type) {
        boost::mutex::scoped_lock lock(instance->mMutex);
        return instance->subtypesList.isType(type);
    };
    static bool isSubType(unsigned int type) {
        boost::mutex::scoped_lock lock(instance->mMutex);
        return instance->subtypesList.isSubType(type);
    };

    static void sortSubtypeList()
    {
        boost::mutex::scoped_lock lock(instance->mMutex);
        instance->subtypesList.sortSubTypes();
    }

    static int findSubtypeId(string value){
        return instance->subtypesList.find(value,false);
    }

    static void unloadAll();
    static MTGAllCards* getInstance();

private:
    boost::mutex mMutex;
    Subtypes subtypesList;
    map<string, MTGCard *> mtgCardByNameCache;
    int processConfLine(string &s, MTGCard* card, CardPrimitive * primitive);
    bool addCardToCollection(MTGCard * card, int setId);
    CardPrimitive * addPrimitive(CardPrimitive * primitive, MTGCard * card = NULL);
};

#define MTGCollection() MTGAllCards::getInstance()

class MTGDeck
{
private:
    string getCardBlockText( const string& title, const string& textBlock );
    void printDetailedDeckText(std::ofstream& file );
protected:
    string filename;
    int total_cards;

public:
    MTGAllCards * database;
    map<int, int> cards;
    string meta_desc;
    string meta_name;
    vector<string> meta_AIHints;
    string meta_unlockRequirements;

    int meta_id;
    int totalCards();
    int totalPrice();
    MTGDeck(MTGAllCards * _allcards);
    MTGDeck(const string& config_file, MTGAllCards * _allcards, int meta_only = 0,int difficultySetting = 0);
    int addRandomCards(int howmany, int * setIds = NULL, int nbSets = 0, int rarity = -1, const string& subtype = "",
            int * colors = NULL, int nbcolors = 0);
    int add(int cardid);
    int add(MTGDeck * deck); // adds the contents of "deck" into myself
    int complete();
    int remove(int cardid);
    int removeAll();
    int add(MTGCard * card);
    int remove(MTGCard * card);
    string getFilename();
    int save();
    int save(const string& destFileName, bool useExpandedDescriptions, const string& deckTitle, const string& deckDesc);
    MTGCard * getCardById(int id);

};

#endif
