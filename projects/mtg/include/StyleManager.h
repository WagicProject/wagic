class WStyle{
public:
    friend class StyleManager;
    string stylized(string filename);
protected:
    map<string,string> mapping;
};

class WStyleRule{
public:
    string filter;  //The condition
    string style;   //The style to use.
};
class MTGDeck;
class StyleManager{
public:
  friend class OptionThemeStyle;
  friend class OptionTheme;
  StyleManager();
  ~StyleManager();
  void determineActive(MTGDeck * p1, MTGDeck * p2);
  WStyle * get();
protected:
  int topRule; int topSize;
  int playerSrc;

  void loadRules();
  void killRules();
  vector<WStyleRule*> rules;
  string activeStyle;
  map<string,WStyle*> styles;
};