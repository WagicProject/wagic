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
#if defined DEBUG_TRANSLATE
  if (checkMisses){
    map<string,int>::iterator it2 = dontCareValues.find(value);
    if (it2 == dontCareValues.end())
      missingValues[value] = 1;
  }
#endif
  return value;
}


Translator::~Translator(){
#if defined DEBUG_TRANSLATE
  if (!checkMisses) return;
  std::ofstream file("Res/lang/missing.txt");
  char writer[4096];
  if (file){
    map<string,int>::iterator it;
    for (it = missingValues.begin(); it!=missingValues.end(); it++){
      sprintf(writer,"%s=\n", it->first.c_str());
      file<<writer;
    }
    file.close();
  }
#endif
}
Translator::Translator(){
#if defined DEBUG_TRANSLATE
  checkMisses = 0;
#endif
  std::ifstream file("Res/lang/_lang.txt");
  std::string s;

  if(file){
#if defined DEBUG_TRANSLATE
  checkMisses = 1;
#endif
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

#if defined DEBUG_TRANSLATE
  if (!checkMisses) return;
  std::ifstream file2("Res/lang/dontcare.txt");

  if(file2){
    while(std::getline(file2,s)){
      if (!s.size()) continue;
      if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
      size_t found = s.find('=');
      if (found != string::npos)
        s = s.substr(0,found);
      dontCareValues[s] = 1;
    }
    file2.close();
  }
#endif

}

string _(string toTranslate){
  Translator * t = Translator::GetInstance();
  return t->translate(toTranslate);
}

