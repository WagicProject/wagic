/*
A class for very simple menus structure
*/
#ifndef _SIMPLEMENU_H_
#define _SIMPLEMENU_H_

#include <string>
#include <JGui.h>
#include "WFont.h"
#include "hge/hgeparticle.h"

#include "WResource_Fwd.h"

class SimpleMenu: public JGuiController
{
private:
    float mHeight, mWidth, mX, mY;
    int fontId;
    std::string title;
    int displaytitle;
    int maxItems, startId;
    float selectionT, selectionY;
    float timeOpen;
    bool mClosed;

    bool mCenterHorizontal;
    bool mCenterVertical;

    static JQuadPtr spadeR, spadeL, jewel, side;
    static JTexture *spadeRTex, *spadeLTex, *jewelTex, *sideTex;
    hgeParticleSystem* stars;

    inline void MogrifyJewel();
    void drawHorzPole(float x, float y, float width);
    void drawVertPole(float x, float y, float height);

public:
    bool autoTranslate;
    bool isMultipleChoice;
    SimpleMenu(JGE*, int id, JGuiListener* listener, int fontId, float x, float y, const char * _title = "", int _maxItems = 7, bool centerHorizontal = true, bool centerVertical = true);
    virtual ~SimpleMenu();
    void Render();
    void Update(float dt);
    void Add(int id, const char * Text, string desc = "", bool forceFocus = false);
    int getmCurr(){return mCurr;}
    float getWidth(){return mWidth; }
    void Close();

    void RecenterMenu();

    float selectionTargetY;
    bool isClosed()
    {
        return mClosed;
    }
    static void destroy();
};

#endif
