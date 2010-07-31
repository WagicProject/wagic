#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

#include <string>
#include <map>


#if defined _DEBUG
#define DEBUG_TRANSLATE
#endif

using namespace std;

class Translator{
 protected:
  static Translator * mInstance;
  bool initDone;
  
  void load(string filename, map<string,string> * dictionary);
public:
  map<string,string> values;
  map<string,string> tempValues;
  map<string,string> deckValues;
#if defined DEBUG_TRANSLATE
  map<string,int> missingValues;
  map<string,int> dontCareValues;
  int checkMisses;
#endif
  string translate(string toTranslate);
  Translator();
  ~Translator();
  int Add(string from, string to);
  void initCards();
  void initDecks();
  void init();
  static Translator * GetInstance();
  static void EndInstance();
};

string _(string toTranslate);

extern bool neofont;
#endif
