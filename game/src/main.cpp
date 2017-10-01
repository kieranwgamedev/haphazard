/*
FILE: main.cpp
PRIMARY AUTHOR: Kieran

Entry point into the program.
Currently just a place to play around in.

Copyright � 2017 DigiPen (USA) Corporation.
*/

#include "Engine/Engine.h"

Engine engine;


int main()
{
	engine.LoggingInit();
	engine.MainLoop();
	// We shouldn't get past MainLoop at this point.

	return 0;
}
