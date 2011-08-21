#include "PrecompiledHeader.h"

#include "Translate.h"
#include "utils.h"

Translator * Translator::mInstance = NULL;

Translator * Translator::GetInstance()
{
    if (!mInstance) mInstance = NEW Translator();
    return mInstance;
}

void Translator::EndInstance()
{
    SAFE_DELETE(mInstance);
}

int Translator::Add(string from, string to)
{
    values[from] = to;
    return 1;
}

string Translator::translate(string value)
{
    //if (!initDone) init();
    map<string, string>::iterator it = values.find(value);
    if (it != values.end()) return it->second;
#if defined DEBUG_TRANSLATE
    if (checkMisses)
    {
        map<string,int>::iterator it2 = dontCareValues.find(value);
        if (it2 == dontCareValues.end())
        missingValues[value] = 1;
    }
#endif
    return value;
}

Translator::~Translator()
{
#if defined DEBUG_TRANSLATE
    if (!checkMisses) return;
    std::ofstream file;
    if (JFileSystem::GetInstance()->openForWrite(file, "lang/missing.txt"))
    {
        char writer[4096];
        map<string,int>::iterator it;
        for (it = missingValues.begin(); it!=missingValues.end(); it++)
        {
            sprintf(writer,"%s=\n", it->first.c_str());
            file<<writer;
        }
        file.close();
    }
#endif
}
Translator::Translator()
{
    initDone = false;
    neofont = false;
    //init();
}

void Translator::load(string filename, map<string, string> * dictionary)
{

    std::string contents;
    if (JFileSystem::GetInstance()->readIntoString(filename, contents))
    {
        std::stringstream stream(contents);
        string s;

        initDone = true;
#if defined DEBUG_TRANSLATE
        checkMisses = 1;
#endif
        while (std::getline(stream, s))
        {
            if (!s.size()) continue;
            if (s[s.size() - 1] == '\r') s.erase(s.size() - 1); //Handle DOS files
            size_t found = s.find('=');
            if (found == string::npos) continue;
            string s1 = s.substr(0, found);
            string s2 = s.substr(found + 1);
            (*dictionary)[s1] = s2;
        }
    }

#if defined DEBUG_TRANSLATE
    if (!checkMisses) return;

    if (JFileSystem::GetInstance()->readIntoString("lang/dontcare.txt", contents))
    {
        std::stringstream stream(contents);
        string s;
        while(std::getline(stream,s))
        {
            if (!s.size()) continue;
            if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
            size_t found = s.find('=');
            if (found != string::npos)
            s = s.substr(0,found);
            dontCareValues[s] = 1;
        }
    }
#endif
}

void Translator::initCards()
{
    string lang = options[Options::LANG].str;
    if (!lang.size()) return;
    string cards_dict = "lang/" + lang + "_cards.txt";
    load(cards_dict, &tempValues);
}

void Translator::initDecks()
{
    string lang = options[Options::LANG].str;
    if (!lang.size()) return;
    string decks_dict = "lang/" + lang + "_decks.txt";

    // Load file
    std::string contents;
    if (JFileSystem::GetInstance()->readIntoString(decks_dict, contents))
    {
        std::stringstream stream(contents);
        string s;
        initDone = true;
        while (std::getline(stream, s))
        {
            if (!s.size()) continue;
            if (s[s.size() - 1] == '\r') s.erase(s.size() - 1); //Handle DOS files
            // Translate '@' to '\n'
            // Note: general language files don't include any line-break infomation
            char * sp = (char *) s.c_str();
            for (int i = 0; sp[i]; i++)
                if (sp[i] == '@') sp[i] = '\n';
            size_t found = s.find('=');
            if (found == string::npos) continue;
            string s1 = s.substr(0, found);
            string s2 = s.substr(found + 1);
            deckValues[s1] = s2;
        }
    }
}

void Translator::init()
{
#if defined DEBUG_TRANSLATE
    checkMisses = 0;
#endif
    string lang = options[Options::LANG].str;
    if (!lang.size()) return;
    string name = "lang/" + lang + ".txt";

    if (fileExists(name.c_str()))
    {
        // fixup for multibyte support.
        std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);
        if (lang.compare("cn") == 0 || lang.compare("jp") == 0)
            neofont = true;
        else
            neofont = false;
        initDone = true;
        load(name, &values);
    }

    initCards();
    initDecks();
}

string _(string toTranslate)
{
    Translator * t = Translator::GetInstance();
    return t->translate(toTranslate);
}

bool neofont;
