#ifndef COREWRAPPER_H
#define COREWRAPPER_H

#include "../include/JGE.h"
#include "../include/JTypes.h"
#include "../include/JApp.h"
#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JGameLauncher.h"
#ifdef QT_CONFIG
#include <QTime>
#endif

class WagicCore
{
private:

public:
    explicit WagicCore();
    virtual ~WagicCore();
    void initApp();

    void doOK() {
        doAndEnqueue(JGE_BTN_OK);
    };
    void doNext() {
        doAndEnqueue(JGE_BTN_PREV);
    };
    void doCancel() {
        doAndEnqueue(JGE_BTN_SEC);
    };
    void doMenu() {
        doAndEnqueue(JGE_BTN_MENU);
    };
    void done() {
        while(m_buttonQueue.size())
        {
            m_engine->ReleaseKey(m_buttonQueue.front());
            m_buttonQueue.pop();
        }
        m_engine->ResetInput();
    };
    void doScroll(int x, int y) {
        m_engine->Scroll(x, y);
    };
    int getNominalHeight(){ return SCREEN_HEIGHT;};
    int getNominalWidth(){ return SCREEN_WIDTH;};
    float getNominalRatio() { return ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);};
    bool getActive() { return m_active; };
    void setActive(bool active);
    void onKeyDown(LocalKeySym);
    void onKeyUp(LocalKeySym);
    void onWindowResize(void* window, float width, float height);
    void onWheelChanged(int deltaX, int deltaY);
    static char* getApplicationName() {
        return JGameLauncher::GetName();
    };

	typedef enum {
		LEFT,
		MIDLE,
		RIGHT
	} PointerId;

    bool onPointerPressed(PointerId, int, int);
    bool onPointerReleased(PointerId, int, int);
    bool onPointerMoved(PointerId, int, int);
    bool onUpdate();
    bool onRender();
	void registerDefaultBindings();
	static WagicCore* getInstance() { return s_instance; };
	char *GetName(){ return m_launcher->GetName();};
#ifdef QT_CONFIG
    QTime startTime;
#endif

private:
    void doAndEnqueue(JButton action) {
        m_engine->HoldKey_NoRepeat(action);
        m_buttonQueue.push(action);
    }

private:
    friend int JGEGetTime();
	static WagicCore* s_instance;
    JGE* m_engine;
    JApp* m_app;
    JGameLauncher* m_launcher;
    int64_t m_lastTickCount;
    std::queue<JButton> m_buttonQueue;
    bool m_active;
	float mMouseDownX;
	float mMouseDownY;
	int mLastFingerDownTime;
};


#endif // COREWRAPPER_H
