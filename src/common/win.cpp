#include "win_platform.h"

// Create the application object
QApplication* createApplication(int& argc, char** argv)
{
	return new PlatformWin(argc, argv);
}
