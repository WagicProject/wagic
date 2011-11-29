#include "PrecompiledHeader.h"

#include "Token.h"

Token::Token(string _name, MTGCardInstance * source, int _power, int _toughness) :
    MTGCardInstance()
{
    isToken = true;
    tokenSource = source;
    power = _power;
    toughness = _toughness;
    life = toughness;
    lifeOrig = life;
    origpower = _power;
    origtoughness = _toughness;
    rarity = Constants::RARITY_T;
    name = _name;
    if (name.size() && name[0] >= 97 && name[0] <= 122) name[0] -= 32; //Poor man's camelcase. We assume strings we get are either Camelcased or lowercase
    setMTGId(-source->getMTGId());
    setId = source->setId;
    model = this;
    data = this;
    owner = source->owner;
    belongs_to = source->controller()->game;
    attacker = 0;
    defenser = NULL;
    banding = NULL;
}

Token::Token(int id) :
    MTGCardInstance()
{
    isToken = true;
    name = "dummyToken";
    setMTGId(id);
}

Token::Token(const Token& source) :
    MTGCardInstance(source.model, source.owner->game)
{
    isToken = source.isToken;
    tokenSource = source.tokenSource;
    power = source.power;
    toughness = source.toughness;
    life = source.life;
    lifeOrig = source.life;
    origpower = source.origpower;
    origtoughness = source.origpower;
    rarity = source.rarity;
    name = source.name;
    setId = source.setId;
    model = source.model;
    data = source.data;
    owner = source.owner;
    belongs_to = source.belongs_to;
    attacker = source.attacker;
    defenser = source.defenser;
    banding = source.banding;
}


MTGCardInstance* Token::clone()
{
    return new Token(*this);
}
