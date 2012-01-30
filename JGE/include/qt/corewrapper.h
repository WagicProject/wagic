#ifndef COREWRAPPER_H
#define COREWRAPPER_H

#include <QObject>
#include <QElapsedTimer>
#ifndef QT_WIDGET
#include <QtDeclarative>
#include <QGraphicsItem>
#endif //QT_WIDGET
#include "../include/JGE.h"
#include "../include/JTypes.h"
#include "../include/JApp.h"
#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JGameLauncher.h"

#if (defined Q_WS_MAEMO_5)
// For screen on/off events support
#include <mce/dbus-names.h>
#include <mce/mode-names.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#endif //Q_WS_MAEMO_5

#ifdef QT_WIDGET
class WagicCore : public QGLWidget
#else
class WagicCore : public QDeclarativeItem
#endif
{
private:
#ifdef QT_WIDGET
    typedef QGLWidget super;
#else
  typedef QDeclarativeItem super;
#endif //QT_WIDGET

public:
    Q_OBJECT
    Q_PROPERTY(int nominalWidth READ getNominalWidth CONSTANT)
    Q_PROPERTY(int nominalHeight READ getNominalHeight CONSTANT)
    Q_PROPERTY(float nominalRatio READ getNominalRatio CONSTANT)
    Q_PROPERTY(bool active READ getActive WRITE setActive NOTIFY activeChanged)


public:
    explicit WagicCore(super *parent = 0);
    virtual ~WagicCore();
    void initApp();

    Q_INVOKABLE void doOK() {
        doAndEnqueue(JGE_BTN_OK);
    };
    Q_INVOKABLE void doNext() {
        doAndEnqueue(JGE_BTN_PREV);
    };
    Q_INVOKABLE void doCancel() {
        doAndEnqueue(JGE_BTN_SEC);
    };
    Q_INVOKABLE void doMenu() {
        doAndEnqueue(JGE_BTN_MENU);
    };
    Q_INVOKABLE void done() {
        while(m_buttonQueue.size())
        {
            m_engine->ReleaseKey(m_buttonQueue.front());
            m_buttonQueue.pop();
        }
        m_engine->ResetInput();
    };
    Q_INVOKABLE void pixelInput(int x, int y);
    Q_INVOKABLE qint64 getTick() {
        return g_startTimer.elapsed();
    };
    Q_INVOKABLE void doScroll(int x, int y, int magnitude) {
        m_engine->Scroll(x, y);
    };
    int getNominalHeight(){ return SCREEN_HEIGHT;};
    int getNominalWidth(){ return SCREEN_WIDTH;};
    float getNominalRatio() { return ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);};
    bool getActive() { return m_active; };
    void setActive(bool active);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    static char* getApplicationName() {
        return JGameLauncher::GetName();
    };

#ifdef QT_WIDGET
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void tapAndHoldTriggered(QTapAndHoldGesture* gesture);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    bool gestureEvent(QGestureEvent* event);
    bool event(QEvent *event);
    void wheelEvent(QWheelEvent *event);
#else
    void wheelEvent ( QGraphicsSceneWheelEvent * event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
#endif

#ifdef QT_WIDGET
public slots:
    void start(int);
#endif

#ifdef Q_WS_MAEMO_5
public slots:
    void displayStateChanged(const QDBusMessage &message);
#endif //Q_WS_MAEMO_5


signals:
    void activeChanged();

private slots:

private:
    void timerEvent( QTimerEvent* );
    void doAndEnqueue(JButton action) {
        m_engine->HoldKey_NoRepeat(action);
        m_buttonQueue.push(action);
    }

public:
    // used mainly to mesure the delta between 2 updates
    static QElapsedTimer g_startTimer;
private:
    JGE* m_engine;
    JApp* m_app;
    JGameLauncher* m_launcher;
    qint64 m_lastTickCount;
    std::queue<JButton> m_buttonQueue;
    int m_timerId;
    bool m_active;
    QRect m_viewPort;
#ifdef QT_WIDGET
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
  int mMouseDownX;
  int mMouseDownY;
  qint64 mLastFingerDownTime;
#endif //Q_WS_MAEMO_5
#endif //QT_WIDGET

#ifdef Q_WS_MAEMO_5
    QDBusConnection dBusConnection;
    QDBusInterface* dBusInterface;
 #endif //Q_WS_MAEMO_5
};
#ifndef QT_WIDGET
QML_DECLARE_TYPE(WagicCore)
#endif //QT_WIDGET

#endif // COREWRAPPER_H
