/*
FILE: main.cpp
PRIMARY AUTHOR: Kieran

Entry point into the program.
Currently just a place to play around in.

Copyright (c) 2017 DigiPen (USA) Corporation.
*/

#include "Engine/Engine.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <iostream>

int main()
{

	Engine engine;

	engine.MainLoop();
	// We shouldn't get past MainLoop at this point.

	return 0;
}
