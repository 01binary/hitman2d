/*------------------------------------------------------------------*\
|
| ThunderStorm.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine master include file
| Created: 08/28/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_STORM_H
#define THUNDER_STORM_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

//
// Libraries
//

// Imports and Headers: STL, Win32, DirectX, DirectSound, DirectShow
#include "stdafx.h"

//
// Utilities
//

// Global definitions, constants, and functions
#include "ThunderGlobals.h"

// Vector classes, math constants and definitions
#include "ThunderMath.h"

// Lightweight string class used throughout the engine
#include "ThunderString.h"

// File reader/writer with buffering support
#include "ThunderStream.h"

// Info file reader/writer
#include "ThunderInfoFile.h"

// Profile reader/writer
#include "ThunderIniFile.h"

// Log file writer (supports text and HTML)
#include "ThunderLogFile.h"

// Error and std::exception classes
#include "ThunderError.h"

//
// Core
//

// Engine class that provides top level interface to the engine
#include "ThunderEngine.h"

// Client class is a link between engine and game. Derive your game 'application' from this.
#include "ThunderClient.h"

// Base class for dynamically createable objects (map, actors, screens and resources)
#include "ThunderObject.h"

// Tile Map and Actor classes that define game world
#include "ThunderTileMap.h"

// Screen classes for in-game user interface
#include "ThunderScreen.h"

//
// Resources
//

// Resource manager class and a base class for all resources
#include "ThunderResource.h"

// Texture classes - a texture is a graphic bitmap loaded into memory through D3DX9
#include "ThunderTexture.h"

// Animation classes - an animation is a collection of frames that reference a material and have texture coordinates
#include "ThunderAnimation.h"

// Sprite classes - a sprite is a collection of animations
#include "ThunderSprite.h"

// Region class - a collision map used for per-pixel collision detection
#include "ThunderRegion.h"

// TextureFont class - used for drawing custom texture fonts. TextureFont class is used for storing D3DXFont's
#include "ThunderFont.h"

// Sound classes - supports playing WAV sounds through DirectSound8 (MP3 can be played if you give file a wav header)
#include "ThunderSound.h"

// Music classes - supports playing WAV and MP3 through DirectShow
#include "ThunderMusic.h"

// Video classes - supports playing MPEG through DirectShow
#include "ThunderVideo.h"

// String table class - used for storing collections of named unicode strings in external files
#include "ThunderStringTable.h"

//
// Scripting
//

// Command classes - used to map client code to command names that can be called in a script
#include "ThunderCommand.h"

// Variable classes - used to store client variables in the engine and in maps, and by utility classes to store data.
#include "ThunderVariable.h"

#endif // THUNDER_STORM_H