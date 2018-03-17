#include <cstdio>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include "Console.h"

int Console::KeyboardHit()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}

void Console::SetNonBlock(Console::NonBlock state)
{
	struct termios ttystate;
	tcgetattr(STDIN_FILENO, &ttystate);
	if (state == NB_ENABLE) {
		ttystate.c_lflag &= ~(ICANON | ECHO); // turn off 'canonical' and 'echo' mode
		ttystate.c_cc[VMIN] = 1; // minimum of number input read.
	} else if (state == NB_DISABLE) {
		ttystate.c_lflag |= ICANON | ECHO; // turn on 'canonical' and 'echo' mode
	}
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int Console::GetInputChar()
{
	SetNonBlock(NB_ENABLE);
	for (;;) {
		if (KeyboardHit()) {
			int c = fgetc(stdin);
			SetNonBlock(NB_DISABLE);
			return c;
		}
		usleep(0);
	}
}
