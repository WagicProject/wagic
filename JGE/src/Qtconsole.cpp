#include "../include/JGE.h"
#include "../include/JTypes.h"
#include "../include/JApp.h"
#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JGameLauncher.h"

#include "TestSuiteAI.h"
#include "GameOptions.h"
#include "MTGDeck.h"
#include "DebugRoutines.h"

#include <sys/time.h>
#include <QCoreApplication>

class WagicWrapper
{
public:
    WagicWrapper();
    virtual ~WagicWrapper();

private:
    JGE* m_engine;
    JApp* m_app;
    JGameLauncher* m_launcher;
};

int JGEGetTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

bool JGEToggleFullscreen()
{
    return true;
}

void JGECreateDefaultBindings()
{
}

WagicWrapper::WagicWrapper()
{
    m_launcher = new JGameLauncher();
    u32 flags = m_launcher->GetInitFlags();
    if ((flags&JINIT_FLAG_ENABLE3D)!=0)
    {
        JRenderer::Set3DFlag(true);
    }

    JGECreateDefaultBindings();

    m_engine = JGE::GetInstance();
    m_app = m_launcher->GetGameApp();
    m_app->Create();
    m_engine->SetApp(m_app);
    JRenderer::GetInstance()->Enable2D();
}

WagicWrapper::~WagicWrapper()
{
    if(m_launcher)
    {
        delete m_launcher;
        m_launcher = NULL;
    }

    if(m_engine)
        m_engine->SetApp(NULL);

    if (m_app)
    {
      m_app->Destroy();
      delete m_app;
      m_app = NULL;
    }

    JGE::Destroy();
    m_engine = NULL;
}

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    int result = 0;
    WagicWrapper* wagicCore =  new WagicWrapper();
    MTGCollection()->loadFolder("sets/primitives/");
    MTGCollection()->loadFolder("sets/", "_cards.dat");
    options.reloadProfile();
    TestSuite testSuite("test/_tests.txt");
    result = testSuite.run();
    delete wagicCore;
    DebugTrace("TestSuite done: failed test: " << result);
    return result;
}
