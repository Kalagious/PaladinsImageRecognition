#include "Header.h"


void mouseMove(int x, int y)
{
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE;
	Input.mi.dx = x;
	Input.mi.dy = y;
	::SendInput(1, &Input, sizeof(INPUT));
}

