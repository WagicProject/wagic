#include "../include/Translate.h"
#include "../include/config.h"
#include <JGE.h>
#include <fstream>
#include <iostream>

Translator *  Translator::mInstance = NULL;

Translator * Translator::GetInstance(){
  if (!mInstance) mInstance = NEW Translator();
  return mInstance;
}

void Translator::EndInstance(){
  SAFE_DELETE(mInstance);
}

int Translator::Add(string from, string to){
  values[from] = to;
  return 1;
}

string Translator::translate(string value){
  map<string,string>::iterator it = values.find(value);
  if (it != values.end()) return it->second;
  return value;
}

Translator::Translator(){
  std::ifstream file("Res/lang/_lang.txt");
  std::string s;

  if(file){
    while(std::getline(file,s)){
      if (!s.size()) continue;
      if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
      size_t found = s.find('=');
      if (found == string::npos) continue;
      string s1 = s.substr(0,found);
      string s2 = s.substr(found+1);
      Add(s1,s2);
    }
    file.close();
  }
}

string _(string toTranslate){
  Translator * t = Translator::GetInstance();
  return t->translate(toTranslate);
}

