#ifndef COREWRAPPER_H
#define COREWRAPPER_H

#include <QObject>
#include <QtDeclarative>
#include <QGraphicsItem>
#include "../include/JGE.h"
#include "../include/JTypes.h"
#include "../include/JApp.h"
#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JGameLauncher.h"

 
class WagicCore : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(int nominalWidth READ getNominalWidth CONSTANT)
    Q_PROPERTY(int nominalHeight READ getNominalHeight CONSTANT)
    Q_PROPERTY(float nominalRatio READ getNominalRatio CONSTANT)
    Q_PROPERTY(bool active READ getActive WRITE setActive NOTIFY activeChanged)

public:
    explicit WagicCore(QDeclarativeItem *parent = 0);
    virtual ~WagicCore();
    void initApp();
    void render(){
        if(m_engine)
            m_engine->Render();
    };

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
    };
    Q_INVOKABLE void pixelInput(int x, int y);
    Q_INVOKABLE qint64 getTick() {
        return g_startTimer.elapsed();
    };
    Q_INVOKABLE void doScroll(int x, int y) {
        m_engine->Scroll(x, y);
    };
    int getNominalHeight(){ return SCREEN_HEIGHT;};
    int getNominalWidth(){ return SCREEN_WIDTH;};
    float getNominalRatio() { return ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);};
    bool getActive() { return m_active; };
    void setActive(bool active);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void resize ( const QRectF &rect);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void wheelEvent ( QGraphicsSceneWheelEvent * event);

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
};
QML_DECLARE_TYPE(WagicCore)

#endif // COREWRAPPER_H
