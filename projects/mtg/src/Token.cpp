#include "PrecompiledHeader.h"

#include "Token.h"

Token::Token(string _name, MTGCardInstance * source, int _power, int _toughness):MTGCardInstance(){
  isToken = true;
  tokenSource = source;
  power = _power;
  toughness = _toughness;
  life=toughness;
  lifeOrig = life;
  rarity = Constants::RARITY_T;
  name = _name;
  if (name.size() && name[0]>=97 && name[0]<=122) name[0]-=32; //Poor man's camelcase. We assume strings we get are either Camelcased or lowercase
  setMTGId(- source->getMTGId());
  setId = source->setId;
  model = this;
  data=this;
  owner = source->owner;
  belongs_to=source->controller()->game;
  attacker = 0;
  defenser = NULL;
  banding = NULL;
}
