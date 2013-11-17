#include <qplatformdefs.h>
#include <QtOpenGL>
#include "qtcorewrapper.h"
#include <QElapsedTimer>

#ifdef TESTSUITE
#include "TestSuiteAI.h"
#include "GameOptions.h"
#include "MTGDeck.h"
#endif
#include "DebugRoutines.h"

QElapsedTimer QtWagicCore::g_startTimer;

QtWagicCore::QtWagicCore(super *parent) :
    super(parent), m_active(false)
#ifdef Q_WS_MAEMO_5
  , dBusConnection(QDBusConnection::systemBus()), dBusInterface(0)
#endif //Q_WS_MAEMO_5
{
#ifdef QT_WIDGET
#if (defined Q_WS_MAEMO_5)
    setAttribute(Qt::WA_Maemo5AutoOrientation);
    setAttribute(Qt::WA_Maemo5NonComposited);
#endif //Q_WS_MAEMO_5
    setAttribute(Qt::WA_AcceptTouchEvents);
  //  setAttribute(Qt::WA_InputMethodEnabled);
    setMouseTracking(true);
    grabGesture(Qt::TapAndHoldGesture);
    resize(SCREEN_WIDTH, SCREEN_HEIGHT);
#else
    setWidth(ACTUAL_SCREEN_WIDTH);
    setHeight(SCREEN_HEIGHT);
    setFlag(QGraphicsItem::ItemHasNoContents, false);
#endif //QT_WIDGET
    g_startTimer.restart();
    m_lastTickCount = g_startTimer.elapsed();

#ifdef Q_WS_MAEMO_5
    dBusInterface = new QDBusInterface(MCE_SERVICE, MCE_REQUEST_PATH,
                                        MCE_REQUEST_IF, dBusConnection);

    // Handle screen state on / off
    dBusConnection.connect(MCE_SERVICE, MCE_SIGNAL_PATH, MCE_SIGNAL_IF, MCE_DISPLAY_SIG, this, SLOT(displayStateChanged(const QDBusMessage &)));
#endif
}

QtWagicCore::~QtWagicCore()
{
#ifdef Q_WS_MAEMO_5
    if(dBusInterface)
        delete dBusInterface;
#endif //Q_WS_MAEMO_5
}

void QtWagicCore::pixelInput(int x, int y)
{
    m_Wagic.onPointerPressed(WagicCore::LEFT, x, y);
}

void QtWagicCore::timerEvent( QTimerEvent* )
{
    bool result = m_Wagic.onUpdate();
    if(!result)
        QApplication::instance()->quit();

    update();
}

void QtWagicCore::setActive(bool active)
{
    if(!m_active && active)
    {
        m_Wagic.setActive(true);
    #if (defined Q_WS_MAEMO_5) || defined(MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
        // 30 fps max on mobile
        m_timerId = startTimer(33);
    #else
        // 200 fps max on desktop
        m_timerId = startTimer(5);
    #endif //Q_WS_MAEMO_5
        m_active = active;
        emit activeChanged();
    }
    else if(m_active && !active)
    {
        m_Wagic.setActive(false);
    #if (defined Q_WS_MAEMO_5) || defined(MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
        killTimer(m_timerId);
    #endif
        m_active = active;
        emit activeChanged();
    }
}

void QtWagicCore::initializeGL()
{
    m_Wagic.initApp();
}

#ifndef QT_WIDGET
void QtWagicCore::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->beginNativePainting();

    initApp();

    resizeGL ( boundingRect().size().width(), boundingRect().size().height());

    initializeGL();

    paintGL();

    painter->endNativePainting();
}
#endif //QT_WIDGET

void QtWagicCore::paintGL()
{
    m_Wagic.onRender();
}

void QtWagicCore::resizeGL(int width, int height)
{
    m_Wagic.onWindowResize(this, width, height);
}

void QtWagicCore::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
  {
#if (defined Q_WS_MAEMO_5)
  case Qt::Key_F7:
    /* interrupt please */
    m_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
    break;
  case Qt::Key_F8:
    /* next phase please */
    m_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
    break;
#endif // Q_WS_MAEMO_5
  case Qt::Key_F:
    JGEToggleFullscreen();
    break;
  default:
    m_Wagic.onKeyDown((LocalKeySym)event->key());
  }
  event->accept();
  super::keyPressEvent(event);
}

void QtWagicCore::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
#if (defined Q_WS_MAEMO_5)
    case Qt::Key_F7:
      /* interrupt please */
      m_engine->ReleaseKey(JGE_BTN_SEC);
      break;
    case Qt::Key_F8:
      /* next phase please */
      m_engine->ReleaseKey(JGE_BTN_PREV);
      break;
  #endif // Q_WS_MAEMO_5
    default:
        m_Wagic.onKeyUp((LocalKeySym)event->key());
    }

    event->accept();
    super::keyReleaseEvent(event);
}

#ifdef QT_WIDGET
void QtWagicCore::wheelEvent(QWheelEvent *event)
#else
void QtWagicCore::wheelEvent ( QGraphicsSceneWheelEvent * event)
#endif
{
    if(event->orientation() == Qt::Vertical)
        m_Wagic.onWheelChanged(0, 3*event->delta());
    else
        m_Wagic.onWheelChanged(3*event->delta(), 0);

    event->accept();
}

#ifdef QT_WIDGET
void QtWagicCore::tapAndHoldTriggered(QTapAndHoldGesture* gesture)
{
    if (gesture->state() == Qt::GestureFinished) {
        m_Wagic.doMenu();
    }
}

void QtWagicCore::mousePressEvent(QMouseEvent *event)
{
    WagicCore::PointerId pointer;
    if(event->button() == Qt::LeftButton) {
        pointer = WagicCore::LEFT;
        m_Wagic.onPointerPressed(pointer, event->pos().x(), event->pos().y());
        event->accept();
    } else if (event->button() == Qt::RightButton) {
        pointer = WagicCore::RIGHT;
        m_Wagic.onPointerPressed(pointer, event->pos().x(), event->pos().y());
        event->accept();
    } else if (event->button() == Qt::MidButton) {
        pointer = WagicCore::MIDLE;
        m_Wagic.onPointerPressed(pointer, event->pos().x(), event->pos().y());
        event->accept();
    } else {
        super::mousePressEvent(event);
    }
}

void QtWagicCore::mouseReleaseEvent(QMouseEvent *event)
{
    WagicCore::PointerId pointer;
    if(event->button() == Qt::LeftButton) {
        pointer = WagicCore::LEFT;
        m_Wagic.onPointerReleased(pointer, event->pos().x(), event->pos().y());
        event->accept();
    } else if (event->button() == Qt::RightButton) {
        pointer = WagicCore::RIGHT;
        m_Wagic.onPointerReleased(pointer, event->pos().x(), event->pos().y());
        event->accept();
    } else if (event->button() == Qt::MidButton) {
        pointer = WagicCore::MIDLE;
        m_Wagic.onPointerReleased(pointer, event->pos().x(), event->pos().y());
        event->accept();
    } else {
        super::mouseReleaseEvent(event);
    }
}

void QtWagicCore::mouseMoveEvent(QMouseEvent *event)
{
    bool result =
            m_Wagic.onPointerMoved(WagicCore::LEFT, event->pos().x(), event->pos().y());

  if (result) {
    event->accept();
  } else {
    super::mouseMoveEvent(event);
  }
}

void QtWagicCore::showEvent ( QShowEvent * )
{
  setActive(true);
}

void QtWagicCore::hideEvent ( QHideEvent * )
{
  setActive(false);
}

bool QtWagicCore::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent*>(event));
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
    else if (event->type() == QEvent::WindowActivate)
    {
        showEvent(NULL);
    }
    else if (event->type() == QEvent::WindowDeactivate)
    {
        hideEvent(NULL);
    }
#endif

    return QGLWidget::event(event);
}

bool QtWagicCore::gestureEvent(QGestureEvent* event)
{
    if (QGesture *tapAndHold = event->gesture(Qt::TapAndHoldGesture))
        tapAndHoldTriggered(static_cast<QTapAndHoldGesture *>(tapAndHold));

    return true;
}

void QtWagicCore::start(int)
{
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
    showFullScreen();
#else
    show();
#endif
    m_Wagic.initApp();
}

#endif //QT_WIDGET

#ifdef Q_WS_MAEMO_5
void QtWagicCore::displayStateChanged(const QDBusMessage &message)
{
  QString state = message.arguments().at(0).toString();
  if (!state.isEmpty()) {
    if (state == MCE_DISPLAY_ON_STRING && isActiveWindow()) {
       setActive(true);
    }
    else if (state == MCE_DISPLAY_OFF_STRING) {
        setActive(false);
    }
  }
}
#endif
