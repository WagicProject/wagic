#include "../include/Token.h"

Token::Token(string _name, MTGCardInstance * source, int _power, int _toughness):MTGCardInstance(){
  isToken = true;
  tokenSource = source;
  power = _power;
  toughness = _toughness;
  life=toughness;
  lifeOrig = life;
  name = _name;
  setMTGId(- source->getMTGId());
  setId = source->setId;
  model = this;
  owner = source->owner;
  belongs_to=source->controller()->game;
  attacker = 0;
  defenser = NULL;
  banding = NULL;
  mCache = source->mCache;

}