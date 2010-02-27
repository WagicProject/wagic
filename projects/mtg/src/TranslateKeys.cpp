#include <map>
#include "../include/Translate.h"
#include "../include/WResourceManager.h"
#include "../include/TranslateKeys.h"

using std::string;
using std::map;

static map<const LocalKeySym, KeyRep> fattable;
static map<const JButton, KeyRep> slimtable;

#ifdef LINUX
const KeyRep& translateKey(LocalKeySym key) {
  {
    map<const LocalKeySym, KeyRep>::iterator res;
    if ((res = fattable.find(key)) != fattable.end())
      return res->second;
  }

 char* str = XKeysymToString(key);
  if (!str)
    {
      str = new char[11];
      sprintf(str, "%lu", key);
    }
  const KeyRep k = make_pair(str, static_cast<JQuad*>(NULL));
  fattable[key] = k;
  return fattable[key];
}
#else
#ifdef WIN32
const KeyRep& translateKey(LocalKeySym key) {
  {
    map<const LocalKeySym, KeyRep>::iterator res;
    if ((res = fattable.find(key)) != fattable.end())
      return *(res->second);
  }

  /* I think the code should look like this ?
     Documentation from : http://msdn.microsoft.com/en-us/library/ms171538.aspx

  Keys keyCode = (Keys)((int)key) & Keys::KeyCode;
  string s = keyCode.ToString();
  KeyRep k;
  if (0 == s.length()) {
    char*str = new char[11];
    sprintf(str, "%d", key);
    k = make_pair(str, static_cast<JQuad*>(NULL));
  }
  else k = make_pair(s, static_cast<JQuad*>(NULL));
  fattable[key] = k;
  return fattable[key];

  ... Instead of the following :
  */


  char* str = new char[11];
  sprintf(str, "%d", key);
  const KeyRep k = make_pair(str, static_cast<JQuad*>(NULL));
  fattable[key] = k;
  return fattable[key];
}
#else // PSP


const KeyRep& translateKey(LocalKeySym key) {
  map<const LocalKeySym, KeyRep>::iterator res;
  if ((res = fattable.find(key)) == fattable.end())
    {
      if (fattable.end() == fattable.find(PSP_CTRL_SELECT))
        {
          fattable[PSP_CTRL_SELECT]   = make_pair(_("Select"),        static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_START]    = make_pair(_("Start"),         static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_UP]       = make_pair(_("Up"),            static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_RIGHT]    = make_pair(_("Right"),         static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_DOWN]     = make_pair(_("Down"),          static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_LEFT]     = make_pair(_("Left"),          static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_LTRIGGER] = make_pair(_("Left trigger"),  static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_RTRIGGER] = make_pair(_("Right trigger"), static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_TRIANGLE] = make_pair(_("Triangle"),      static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_CIRCLE]   = make_pair(_("Circle"),        static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_CROSS]    = make_pair(_("Cross"),         static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_SQUARE]   = make_pair(_("Square"),        static_cast<JQuad*>(NULL));
          fattable[PSP_CTRL_HOLD]     = make_pair(_("Hold"),          static_cast<JQuad*>(NULL));
        }
      else
        {
          char* str = new char[11];
          sprintf(str, "%d", key);
          fattable[key] = make_pair(str, static_cast<JQuad*>(static_cast<JQuad*>(NULL)));
        }
      res = fattable.find(key);
    }
  KeyRep& k = res->second;
  switch(key)
    {
    case PSP_CTRL_SELECT :   k.second = resources.RetrieveQuad("iconspsp.png", (float)2*32, 32, 64, 32, "PSP_CTRL_SELECT",   RETRIEVE_NORMAL); break;
    case PSP_CTRL_START :    k.second = resources.RetrieveQuad("iconspsp.png", (float)0*32, 32, 64, 32, "PSP_CTRL_START",    RETRIEVE_NORMAL); break;
    case PSP_CTRL_UP :       k.second = resources.RetrieveQuad("iconspsp.png", (float)0*32,  0, 32, 32, "PSP_CTRL_UP",       RETRIEVE_NORMAL); break;
    case PSP_CTRL_RIGHT :    k.second = resources.RetrieveQuad("iconspsp.png", (float)3*32,  0, 32, 32, "PSP_CTRL_RIGHT",    RETRIEVE_NORMAL); break;
    case PSP_CTRL_DOWN :     k.second = resources.RetrieveQuad("iconspsp.png", (float)1*32,  0, 32, 32, "PSP_CTRL_DOWN",     RETRIEVE_NORMAL); break;
    case PSP_CTRL_LEFT :     k.second = resources.RetrieveQuad("iconspsp.png", (float)2*32,  0, 32, 32, "PSP_CTRL_LEFT",     RETRIEVE_NORMAL); break;
    case PSP_CTRL_LTRIGGER : k.second = resources.RetrieveQuad("iconspsp.png", (float)6*32, 32, 64, 32, "PSP_CTRL_LTRIGGER", RETRIEVE_NORMAL); break;
    case PSP_CTRL_RTRIGGER : k.second = resources.RetrieveQuad("iconspsp.png", (float)8*32, 32, 64, 32, "PSP_CTRL_RTRIGGER", RETRIEVE_NORMAL); break;
    case PSP_CTRL_TRIANGLE : k.second = resources.RetrieveQuad("iconspsp.png", (float)5*32,  0, 32, 32, "PSP_CTRL_TRIANGLE", RETRIEVE_NORMAL); break;
    case PSP_CTRL_CIRCLE :   k.second = resources.RetrieveQuad("iconspsp.png", (float)4*32,  0, 32, 32, "PSP_CTRL_CIRCLE",   RETRIEVE_NORMAL); break;
    case PSP_CTRL_CROSS :    k.second = resources.RetrieveQuad("iconspsp.png", (float)7*32,  0, 32, 32, "PSP_CTRL_CROSS",    RETRIEVE_NORMAL); break;
    case PSP_CTRL_SQUARE :   k.second = resources.RetrieveQuad("iconspsp.png", (float)6*32,  0, 32, 32, "PSP_CTRL_SQUARE",   RETRIEVE_NORMAL); break;
    case PSP_CTRL_HOLD :     k.second = resources.RetrieveQuad("iconspsp.png", (float)4*32,  0, 32, 32, "PSP_CTRL_HOLD",     RETRIEVE_NORMAL); break;
    default: /* Unknown key : no icon */ ;
    }
  return k;
}
#endif
#endif

const KeyRep& translateKey(JButton key) {
  {
    map<const JButton, KeyRep>::iterator res;
    if ((res = slimtable.find(key)) != slimtable.end())
      return res->second;
  }

  slimtable[JGE_BTN_NONE] =   make_pair(_("None"),                     static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_QUIT] =   make_pair(_("Quit"),                     static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_MENU] =   make_pair(_("Menu"),                     static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_CTRL] =   make_pair(_("Control"),                  static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_POWER] =  make_pair(_("Power"),                    static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_SOUND] =  make_pair(_("Sound"),                    static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_RIGHT] =  make_pair(_("Right"),                    static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_LEFT] =   make_pair(_("Left"),                     static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_UP] =     make_pair(_("Up"),                       static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_DOWN] =   make_pair(_("Down"),                     static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_OK] =     make_pair(_("Ok"),                       static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_CANCEL] = make_pair(_("Cancel"),                   static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_PRI] =    make_pair(_("Primary"),                  static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_SEC] =    make_pair(_("Secondary"),                static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_PREV] =   make_pair(_("Next phase/Previous item"), static_cast<JQuad*>(NULL));
  slimtable[JGE_BTN_NEXT] =   make_pair(_("Open hand/Next item"),      static_cast<JQuad*>(NULL));

  return slimtable[key];
}
