#include "launcher.h"

#include "debug.h"

#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <sstream>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Bind and activate the framebuffer console on selected platforms.
#define BIND_CONSOLE \
		defined(PLATFORM_A320) || defined(PLATFORM_GCW0)|| defined(PLATFORM_RG350)

#if BIND_CONSOLE
#include <linux/vt.h>
#endif

using namespace std;


Launcher::Launcher(vector<string> const& commandLine, bool consoleApp)
	: commandLine(commandLine)
	, consoleApp(consoleApp)
{
}

Launcher::Launcher(vector<string> && commandLine, bool consoleApp)
	: commandLine(commandLine)
	, consoleApp(consoleApp)
{
}

void Launcher::exec()
{
	DEBUG("Launcher::exec - enter");
	
	if (consoleApp) {
		DEBUG("Launcher::exec - console app");
#if BIND_CONSOLE
		/* Enable the framebuffer console */
		DEBUG("Launcher::exec - enabling framebuffer");
		char c = '1';
		int fd = open("/sys/devices/virtual/vtconsole/vtcon1/bind", O_WRONLY);
		if (fd < 0) {
			WARNING("Unable to open fbcon handle\n");
		} else {
			write(fd, &c, 1);
			close(fd);
		}
		DEBUG("Launcher::exec - opening tty handle");
		fd = open("/dev/tty1", O_RDWR);
		if (fd < 0) {
			WARNING("Unable to open tty1 handle\n");
		} else {
			if (ioctl(fd, VT_ACTIVATE, 1) < 0)
				WARNING("Unable to activate tty1\n");
			close(fd);
		}
#endif
		DEBUG("Launcher::exec - end of console specific work");
	}

	DEBUG("Launcher::exec - sorting args out");
	vector<const char *> args;
	args.reserve(commandLine.size() + 1);
	for (auto arg : commandLine) {
		args.push_back(arg.c_str());
	}
	args.push_back(nullptr);
	
	DEBUG("Launcher::exec - exec-ing now");
	execvp(commandLine[0].c_str(), (char* const*)&args[0]);
	WARNING("Failed to exec '%s': %s\n",
			commandLine[0].c_str(), strerror(errno));
	
	DEBUG("Launcher::exec - exit");
}
