#include "../include/config.h"
#include "../include/OptionItem.h"
#include "../include/PlayerData.h"
#include "../include/Translate.h"
#include <algorithm>


//WCFilterFactory
WCFilterFactory* WCFilterFactory::me = NULL;

WCFilterFactory*  WCFilterFactory::GetInstance() {
  if(!me)
    me = NEW WCFilterFactory();
  return me;
}
void WCFilterFactory::Destroy(){
  SAFE_DELETE(me);
}
size_t WCFilterFactory::findNext(string src, size_t start,char open, char close){
  int num = 0;
  for(size_t x=start;x<src.size();x++){
    if(src[x] == open)
      num++;
    if(src[x] == close){
      num--;
      if(num == 0)
        return x;
    }
  }
  return string::npos;
}
WCardFilter * WCFilterFactory::Construct(string src){
  size_t x = 0;
  string whitespaces (" \t\f\v\n\r");
  x = src.find_first_not_of(whitespaces);
  if(x != string::npos) src = src.substr(x);

  if(!src.size())
    return NEW WCFilterNULL(); //Empty string.
  

  for(size_t i=0;i<src.size();i++){
    unsigned char c = src[i];
    if(isspace(c))
      continue;
    if(c == '('){ //Parenthesis 
      size_t endp = findNext(src,i);
      if(endp != string::npos){   
        WCFilterGROUP * g = NEW WCFilterGROUP(Construct(src.substr(i+1,endp-1)));
        if(endp < src.size()){
          if(src[endp+1] == '|')
            return NEW WCFilterOR(g,Construct(src.substr(endp+2)));
          else if(src[endp+1] == '&')
            return NEW WCFilterAND(g,Construct(src.substr(endp+2)));
          else
            return g;
        }
      }
      else
        return NEW WCFilterNULL();
    }else if(c == '{'){ //Negation
      size_t endp = findNext(src,i,'{','}');
      if(endp != string::npos){   
        WCFilterNOT * g = NEW WCFilterNOT(Construct(src.substr(i+1,endp-1)));
        if(endp < src.size()){
          if(src[endp+1] == '|')
            return NEW WCFilterOR(g,Construct(src.substr(endp+2)));
          else if(src[endp+1] == '&')
            return NEW WCFilterAND(g,Construct(src.substr(endp+2)));
          else
            return g;
        }
      }
      else
        return NEW WCFilterNULL();
    }else if(c == '&'){ //And
      return NEW WCFilterAND(Construct(src.substr(0,i)),Construct(src.substr(i+1)));
    }
    else if(c == '|'){ //Or
      return NEW WCFilterOR(Construct(src.substr(0,i)),Construct(src.substr(i+1)));
    }
  }
  return Leaf(src);
}

WCardFilter * WCFilterFactory::Leaf(string src){
  string filter;
  string whitespaces (" \t\f\v\n\r");
  size_t x = src.find_first_not_of(whitespaces);
  if(x != string::npos) src = src.substr(x);

  for(size_t i=0;i<src.size();i++){
    unsigned char c = src[i];
    if(isspace(c))
      continue;
    if(c == '('){ //Scan to ')', call Construct.
      size_t end = src.find(")",i);
      if(end != string::npos){
        string expr = src.substr(i+1,i-end);
        return NEW WCFilterGROUP(Construct(expr));
      }
    }else if(c == '{'){ //Scan to '}', call Construct.
      size_t end = src.find("}",i);
      if(end != string::npos){
        string expr = src.substr(i+1,i-end);
        return NEW WCFilterNOT(Construct(expr));
      }
    }else if(c == ':'){ //Scan ahead to ';', inbetween this is an argument
      size_t end = src.find(";",i);
      if(end != string::npos && filter.size()){
        string arg = src.substr(i+1,end-i-1);
        return Terminal(filter,arg); 
      }
    }
    else
      filter += c;

  }
  return NEW WCFilterNULL();
}

WCardFilter * WCFilterFactory::Terminal(string src, string arg){
  string type;
  for(size_t x=0;x<src.size();x++){
    if(isspace(src[x])) continue;
    type += src[x];
  }
  std::transform(type.begin(),type.end(),type.begin(),::tolower);

  if(type == "r" || type == "rarity")
    return NEW WCFilterRarity(arg);
  else if(type == "c" || type == "color")
    return NEW WCFilterColor(arg); 
  else if(type == "xc" || type == "xcolor")
    return NEW WCFilterOnlyColor(arg);
  else if(type == "s" || type == "set")
    return NEW WCFilterSet(arg);
  else if(type == "alpha")
    return NEW WCFilterLetter(arg);
  else if(type == "t" || type == "type")
    return NEW WCFilterType(arg);
  else if(type == "a" || type == "ability")
    return NEW WCFilterAbility(arg);
  else if(type == "cmc")
    return NEW WCFilterCMC(arg);
  else if(type == "produces" || type == "ma")
    return NEW WCFilterProducesColor(arg);
  else if(type == "pow" || type == "power")
    return NEW WCFilterPower(arg);
  else if(type == "tgh" || type == "tough" || type == "toughness")
    return NEW WCFilterToughness(arg);

  return NEW WCFilterNULL();
}
//WCFilterLetter
WCFilterLetter::WCFilterLetter(string arg){
  if(!arg.size())
    alpha = 'a';
  else
    alpha = tolower(arg[0]);
}
bool WCFilterLetter::isMatch(MTGCard * c){
  if(!c || !c->data)
    return false;
  string s = c->data->getLCName();
  if(!s.size())
    return false;
  if(s[0] == alpha || (alpha == '#' && (isdigit(s[0]) || ispunct(s[0]))))
    return true;
  return false;
}
string WCFilterLetter::getCode(){
  char buf[24];
  sprintf(buf,"alpha:%c;",alpha);
  return buf;
}
//WCFilterSet
WCFilterSet::WCFilterSet(string arg){
  setid = setlist.findSet(arg);
}

string WCFilterSet::getCode(){
  char buf[256]; 
  sprintf(buf,"set:%s;",setlist[setid].c_str());
  return buf;
};

//WCFilterColor
bool WCFilterColor::isMatch(MTGCard * c){
  if(!c || !c->data)
    return false;
  return (c->data->hasColor(color) > 0);
}
string WCFilterColor::getCode(){
  char buf[12]; 
  char c = '?';
  if(color < 0 || color >= Constants::MTG_NB_COLORS)
    c = Constants::MTGColorChars[color];
  sprintf(buf,"color:%c;",c); 
  return buf;
};
WCFilterColor::WCFilterColor(string arg){
  color = -1;
  char c = tolower(arg[0]);
  for(int i=0;i<Constants::MTG_NB_COLORS;i++){
    if(Constants::MTGColorChars[i] == c){
      color = i;
      break;
    }
  }
}
//WCFilterOnlyColor
bool WCFilterOnlyColor::isMatch(MTGCard * c){
 if(!c || !c->data)
    return false;
 for(int i=0;i<Constants::MTG_NB_COLORS;i++){
   if(i == color) continue;
   if(c->data->hasColor(i) > 0)
       return false;
 }
  return (c->data->hasColor(color) > 0);
}
string WCFilterOnlyColor::getCode(){
  char buf[12]; 
  char c = '?';
  if(color < 0 || color >= Constants::MTG_NB_COLORS)
    c = Constants::MTGColorChars[color];
  sprintf(buf,"xcolor:%c;",c); 
  return buf;
}
//WCFilterProducesColor
bool WCFilterProducesColor::isMatch(MTGCard * c){
  bool bMatch = false;
 if(!c || !c->data)
    return false;
  string s = c->data->magicText;
  size_t t = s.find("add");
  while(t != string::npos){
    s = s.substr(t+3);
    ManaCost * mc = ManaCost::parseManaCost(s);
    if(mc->hasColor(color) > 0) {bMatch = true; SAFE_DELETE(mc); break;}
    SAFE_DELETE(mc);
    t = s.find("add");    
  }
  return bMatch;
}
string WCFilterProducesColor::getCode(){
  char buf[12]; 
  char c = '?';
  if(color < 0 || color >= Constants::MTG_NB_COLORS)
    c = Constants::MTGColorChars[color];
  sprintf(buf,"produces:%c;",c); 
  return buf;
}
//WCFilterNumeric
WCFilterNumeric::WCFilterNumeric(string arg){
  number = atoi(arg.c_str());
}
//WCFilterCMC
bool WCFilterCMC::isMatch(MTGCard * c){
  if(!c || !c->data)
    return false;
  ManaCost * mc = c->data->getManaCost();
  return (mc->getConvertedCost() == number);
}

string WCFilterCMC::getCode(){
  char buf[64];
  sprintf(buf,"cmc:%i;",number);
  return buf;
}
//WCFilterPower
bool WCFilterPower::isMatch(MTGCard * c){
  if(!c || !c->data)
    return false;
  return (c->data->getPower() == number);
}
string WCFilterPower::getCode(){
  char buf[64];
  sprintf(buf,"power:%i;",number);
  return buf;
}
//WCFilterPower
bool WCFilterToughness::isMatch(MTGCard * c){
  if(!c || !c->data)
    return false;
  return (c->data->getToughness() == number);
}
string WCFilterToughness::getCode(){
  char buf[64];
  sprintf(buf,"toughness:%i;",number);
  return buf;
}
//WCFilterRarity
float WCFilterRarity::filterFee(){
  switch(rarity){
    case 'M': return 2.0f;
    case 'R': return 1.0f;
    case 'U': return 0.5f;
    case 'C': return 0.2f;
  }
  return 0.0f;
}
bool WCFilterRarity::isMatch(MTGCard * c){
  if(!c || !c->data)
    return false;
  if(rarity == 'A') return true; //A for "Any" or "All"
  return (c->getRarity() == rarity);
}
string WCFilterRarity::getCode(){
  char buf[64]; 
  const char* rarities[8] = {"any","token","land","common","uncommon","rare","mythic","special"};
  int x = 0;
  switch(rarity){
    case 'S': x=7; break;
    case 'M': x=6; break;
    case 'R': x=5; break;
    case 'U': x=4; break;
    case 'C': x=3; break;
    case 'L': x=2; break;
    case 'T': x=1; break;
  }
  sprintf(buf,"rarity:%s;",rarities[x]); 
  return buf;
};
WCFilterRarity::WCFilterRarity(string arg){
  rarity = -1;
  char c = toupper(arg[0]);
  switch(c){
    case 'S':
    case 'M':
    case 'R':
    case 'U':
    case 'C':
    case 'L':
    case 'T':
      rarity = c;
      return;
  }
  rarity = 'A';
}
//WCFilterAbility
bool WCFilterAbility::isMatch(MTGCard * c){
  if(ability < 0)
    return false;
  map<int,int>::iterator it = c->data->basicAbilities.find(ability);

  if(it != c->data->basicAbilities.end()){
    if(it->second > 0)
      return true;
  }
  return false;
}
WCFilterAbility::WCFilterAbility(string arg){
  std::transform(arg.begin(),arg.end(),arg.begin(),::tolower);
  for(int i = 0;i<Constants::NB_BASIC_ABILITIES;i++){
    if(arg == Constants::MTGBasicAbilities[i]){
      ability = i;
      return;
    }
  }
  ability = -1;
}
string WCFilterAbility::getCode(){
  char buf[64]; 
  if(ability < 0 || ability >= Constants::NB_BASIC_ABILITIES)
    return "";
  sprintf(buf,"ability:%s;",Constants::MTGBasicAbilities[ability]); 
  return buf;
};
float WCFilterAbility::filterFee(){
  switch(ability){
    case Constants::SHROUD:
    case Constants::DEATHTOUCH:
    case Constants::UNBLOCKABLE:
    case Constants::WITHER:
    case Constants::PERSIST:
      return 0.8f;
    case Constants::PROTECTIONBLACK:
    case Constants::PROTECTIONWHITE:
    case Constants::PROTECTIONBLUE:
    case Constants::PROTECTIONRED:
    case Constants::PROTECTIONGREEN:
    case Constants::DOUBLESTRIKE:
    case Constants::LIFELINK:
      return 0.7f;
    case Constants::TRAMPLE:
    case Constants::FLYING:
    case Constants::FEAR:
    case Constants::VIGILANCE:
    case Constants::FIRSTSTRIKE:
      return 0.6f;
    case Constants::PLAINSHOME:
    case Constants::SWAMPHOME:
    case Constants::ISLANDHOME:    
    case Constants::MOUNTAINHOME:
    case Constants::FORESTHOME:
      return -0.2f;
    case Constants::DEFENDER:
    case Constants::CLOUD:
      return 0.2f;
    default:
      return 0.4f;
  }
  return 0.0f;
}
//WCFilterType
bool WCFilterType::isMatch(MTGCard * c){
  return c->data->hasType(type.c_str());
}
string WCFilterType::getCode(){
  char buf[4068];
  sprintf(buf,"type:%s;",type.c_str());
  return buf;
}
//Misc. filter code
float WCFilterAND::filterFee(){
  return lhs->filterFee() + rhs->filterFee();
}
float WCFilterOR::filterFee(){
  if(lhs->filterFee() > rhs->filterFee())
    return lhs->filterFee();
  return rhs->filterFee();
}
string WCFilterNOT::getCode(){
    char buf[4068];
    sprintf(buf,"{%s}",kid->getCode().c_str());
    return buf;
}
string WCFilterGROUP::getCode(){
  char buf[4068];
  sprintf(buf,"(%s)",kid->getCode().c_str());
  return buf;
}

string WCFilterAND::getCode(){
  char buf[4068];
  sprintf(buf,"%s&%s",lhs->getCode().c_str(),rhs->getCode().c_str());
  return buf;
}
string WCFilterOR::getCode(){
  char buf[4068];
  sprintf(buf,"%s|%s",lhs->getCode().c_str(),rhs->getCode().c_str());
  return buf;
}


bool WCFilterOR::isMatch(MTGCard *c){
  if(lhs->isMatch(c))
    return true;
  if(rhs->isMatch(c))
    return true;
  return false;
};