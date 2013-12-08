#include "../include/JGE.h"
#include "../include/JTypes.h"
#include "../include/JApp.h"
#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JGameLauncher.h"

#include "corewrapper.h"
#include "TestSuiteAI.h"
#include "GameOptions.h"
#include "MTGDeck.h"
#include "DebugRoutines.h"
#include <QCoreApplication>
#include <QElapsedTimer>

bool JGEToggleFullscreen()
{
    return true;
}

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    int result = 0;
    WagicCore* wagicCore =  new WagicCore();
    MTGCollection()->loadFolder("sets/primitives/");
    MTGCollection()->loadFolder("sets/", "_cards.dat");
    options.reloadProfile();
    TestSuite testSuite("test/_tests.txt");
    result = testSuite.run();
    delete wagicCore;
    DebugTrace("TestSuite done: failed test: " << result << " out of " << testSuite.nbTests + testSuite.nbAITests << " total");
#ifdef CAPTURE_STDERR
    OutputCapturer::debugAndClear();
#endif
    return result;
}
