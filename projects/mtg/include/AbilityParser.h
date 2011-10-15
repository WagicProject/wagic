#ifndef _ABILITY_PARSER_H_
#define _ABILITY_PARSER_H_

#include <string>
#include <vector>
using std::string;
using std::vector;
using std::map;

class AutoLineMacro {
private:
    string mName;
    string mResult;
    vector<string> mParams;
    void parse(string& s);
    string process(string& s);
    static vector<AutoLineMacro *> gAutoLineMacros;
    static map<string, bool> gAutoLineMacrosIndex;
public:
    AutoLineMacro(string& s);
    static void Destroy();
    static bool AddMacro(string& s);
    static string Process(string& s);
};

#endif