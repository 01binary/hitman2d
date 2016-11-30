/*------------------------------------------------------------------*\
|
| ScreenImage.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Image control class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_IMAGE_H
#define SCREEN_IMAGE_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenImage class
\*----------------------------------------------------------*/

class ScreenImage: public Screen
{
public:
	//
	// Constants
	//

	// Class Name

	static const WCHAR SZ_CLASS[];

public:
	ScreenImage(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenImage(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Events
	//

	virtual void OnCommand(int nCommandID, Screen* pSender = NULL, int nParam = 0);
	virtual int OnNotify(int nNotifyID, Screen* pSender = NULL, int nParam = 0);

	virtual void OnFocus(Screen* pOldFocus);
	virtual void OnKeyDown(int nKeyCode);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);
	virtual void OnMouseMove(POINT pt);

	virtual void OnThemeStyleChange(void);
};

} // namespace Hitman2D

#endif // SCREEN_IMAGE_H