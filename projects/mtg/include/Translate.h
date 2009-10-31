#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

#include <string>
#include <map>


#if defined WIN32
#define DEBUG_TRANSLATE
#endif

using namespace std;

class Translator{
 protected:
  static Translator * mInstance;
public:
  map<string,string> values;
#if defined DEBUG_TRANSLATE
  map<string,int> missingValues;
  map<string,int> dontCareValues;
  int checkMisses;
#endif
  string translate(string toTranslate);
  Translator();
  ~Translator();
  int Add(string from, string to);
  static Translator * GetInstance();
  static void EndInstance();
};

string _(string toTranslate);

#endif