class WParsedInt
{
private:
    void init(string s, Spell * spell, MTGCardInstance * card);

public:
    int intValue;

    int getValue();
    string getStringValue();

    int computeX(Spell * spell, MTGCardInstance * card);
    int countDevotionTo(MTGCardInstance * card, MTGGameZone * zone, int color1, int color2);
    int countCardNameinZone(string name, MTGGameZone * zone);
    int countCardsInPlaybyColor(int color, GameObserver * observer);
    int mostCommonColor(int color, MTGCardInstance * card);
    int countCardTypeinZone(string type, MTGGameZone * zone);
    int cardHasTypeinZone(const char * type, MTGGameZone * zone);
    int countCanTargetby(string type, MTGCardInstance * card, Player * player);
    int countManaProducedby(int color, MTGCardInstance * target, Player * player);

    WParsedInt(int value = 0);
    WParsedInt(string s, Spell * spell, MTGCardInstance * card);
    WParsedInt(string s, MTGCardInstance * card);
};

class WParsedPT
{
public:
    bool ok;
    WParsedInt power, toughness;

    WParsedPT(int p, int t);
    WParsedPT(string s, Spell * spell, MTGCardInstance * card);
};