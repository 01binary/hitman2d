/*------------------------------------------------------------------*\
|
| ThunderMath.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm mathematics class(es) and function(s)
| Created: 05/03/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderMath.h"		// defining TileVector/3, TileRect, TileRange
#include "ThunderStream.h"		// using Stream
#include "ThunderInfoFile.h"	// using InfoElem

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Vector2 implementation
\*----------------------------------------------------------*/

void Vector2::Serialize(Stream& rStream) const
{
	rStream.Write(this, sizeof(Vector2));
}

void Vector2::Deserialize(Stream& rStream)
{
	rStream.Read(this, sizeof(Vector2));
}

/*----------------------------------------------------------*\
| Vector3 implementation
\*----------------------------------------------------------*/

void Vector3::Serialize(Stream& rStream) const
{
	rStream.Write(this, sizeof(Vector3));
}

void Vector3::Deserialize(Stream& rStream)
{
	rStream.Read(this, sizeof(Vector3));
}

/*----------------------------------------------------------*\
| Rect implementation
\*----------------------------------------------------------*/

Rect::Rect(int initleft,
		   int inittop,
		   int initright,
		   int initbottom)
{
	left = initleft;
	top = inittop;
	right = initright;
	bottom = initbottom;
}

void Rect::Set(int left, int top, int right, int bottom)
{
	this->left = left;
	this->top = top;
	this->right = right;
	this->bottom = bottom;
}

void Rect::Set(const POINT& rpt, const SIZE& rps)
{
	left = rpt.x;
	top = rpt.y;
	right = rpt.x + rps.cx;
	bottom = rpt.y + rps.cy;
}

void Rect::Set(const POINT& rpt1, const POINT& rpt2)
{
	left = rpt1.x;
	top = rpt1.y;
	right = rpt2.x;
	bottom = rpt2.y;
}

void Rect::SetPosition(int left, int top)
{
	int cx = this->right - this->left;
	int cy = this->bottom - this->top;

	this->left = left;
	this->top = top;
	this->right = left + cx;
	this->bottom = top + cy;
}

void Rect::SetSize(int width, int height)
{
	right = left + width;
	bottom = top + height;
}

void Rect::Offset(int x, int y)
{
	left += x;
	right += x;

	top += y;
	bottom += y;
}

void Rect::Offset(POINT pt)
{
	left += pt.x;
	right += pt.x;

	top += pt.y;
	bottom += pt.y;
}

void Rect::Serialize(Stream& rStream) const
{
	rStream.Write(this, sizeof(Rect));
}

void Rect::Deserialize(Stream& rStream)
{
	rStream.Read(this, sizeof(Rect));
}

void Rect::Serialize(InfoElem& rRoot) const
{
	rRoot.FromIntArray(reinterpret_cast<const int*>(&left), 4);
}

void Rect::Deserialize(const InfoElem& rRoot, int nStartElemIndex)
{
	if (rRoot.GetElemType() != InfoElem::TYPE_VALUELIST)
		throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
			rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

	if (rRoot.GetChildCount() < (nStartElemIndex + 4))
		throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
			rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

	rRoot.ToIntArray(reinterpret_cast<int*>(&left), 4, nStartElemIndex);

	// Always read rects and position and size instead of 4 points
	// Makes it easier to modify for humans

	right += left;
	bottom += top;
}