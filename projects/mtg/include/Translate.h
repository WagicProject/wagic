#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

#include <string>
#include <map>

using namespace std;

class Translator{
 protected:
  static Translator * mInstance;
public:
  map<string,string> values;
  string translate(string toTranslate);
  Translator();
  int Add(string from, string to);
  static Translator * GetInstance();
  static void EndInstance();
};

string _(string toTranslate);

#endif