/*------------------------------------------------------------------*\
|
| Application.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D application entry point implementation
| Created: 04/14/2010
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"		// precompiled header
#include "Game.h"		// using Game


int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nShowCmd)
{
	Hitman2D::Game game;

	game.Run(GetCommandLine(), nShowCmd);

	return 0;
}