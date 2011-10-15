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
    void parse(const string& s);
    string process(const string& s);

    static vector<AutoLineMacro *> gAutoLineMacros;
    static map<string, bool> gAutoLineMacrosIndex;
public:
    AutoLineMacro(const string& s);
    static void Destroy();
    static bool AddMacro(const string& s);
    static string Process(const string& s);
};

#endif