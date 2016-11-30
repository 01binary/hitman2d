/*------------------------------------------------------------------*\
|
| ScreenTabControl.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Tab control class
| Created: 04/04/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_TABCONTROL_H
#define SCREEN_TABCONTROL_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenButtonEx.h"		// using ScreenButtonEx

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenTabControl class
\*----------------------------------------------------------*/

class ScreenTabControl: public Screen
{
public:
	//
	// Constants
	//

	enum FrameElements
	{
		FRAME_TOP_ACTIVE,
		FRAME_TOP_INACTIVE,
		FRAME_TOPLEFT_ACTIVE,
		FRAME_TOPLEFT_INACTIVE,
		FRAME_TOPRIGHT_ACTIVE,
		FRAME_TOPRIGHT_INACTIVE,
		FRAME_LEFT,
		FRAME_RIGHT,
		FRAME_BOTTOMLEFT,
		FRAME_BOTTOMRIGHT,
		FRAME_BOTTOM,
		FRAME_CENTER,
		FRAME_ELEMENT_COUNT
	};

	static const int MINTABSIZE;
	static const LPCWSTR SZ_ELEMENTS[];
	static const WCHAR SZ_TAB[];
	static const WCHAR SZ_TABSTYLEACTIVE[];
	static const WCHAR SZ_TABSTYLEINACTIVE[];
	static const WCHAR SZ_CLASS[];

protected:
	int m_nActiveTab;

	String m_strTabStyleActive;
	String m_strTabStyleInactive;

	MaterialInstance
		m_Elements[FRAME_ELEMENT_COUNT];

	ScreenList m_lstTabs;

	std::map<int, int> m_mapTabContainers;	// Containers mapped to tabs

public:
	ScreenTabControl(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenTabControl(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Tabs
	//

	int AddTab(LPCWSTR pszName, int nContainerID, bool bUpdateLayout = true);
	void RemoveTab(int nID);
	void RemoveAllTabs(void);

	inline ScreenButtonEx* GetTab(int nID)
	{
		return dynamic_cast<ScreenButtonEx*>(m_lstChildren.FindByID(nID));
	}

	inline int GetTabContainerID(int nID)
	{
		return m_mapTabContainers[nID];
	}

	inline int GetTabCount(void) const
	{
		return int(m_lstTabs.size());
	}

	inline int GetActiveTab(void) const
	{
		return m_nActiveTab;
	}

	void SetActiveTab(int nID);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Events
	//
	
	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);
	virtual void OnCommand(int nCommandID, Screen* pSender = NULL, int nParam = 0);

	virtual int OnNotify(int nNotifyID, Screen* pSender, int nParam = 0);

	virtual void OnKeyDown(int nKeyCode);
	virtual void OnKeyPress(int nAsciiCode, bool extended, bool alt);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);
	virtual void OnMouseMove(POINT pt);

	virtual void OnSize(const SIZE& rpsOldSize);

	virtual void OnThemeStyleChange(void);
};

} // namespace Hitman2D

#endif // SCREEN_TABCONTROL_H