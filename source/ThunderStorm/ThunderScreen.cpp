/*------------------------------------------------------------------*\
|
| ThunderScreen.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm screen class(es) implementation
| Created: 10/06/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderClient.h"		// using Client, using Engine, defining Screen/List
#include "ThunderStream.h"		// using Stream
#include "ThunderInfoFile.h"	// using InfoFile/Elem

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR Screen::SZ_ROOT[]						= L"screen";
const WCHAR Screen::SZ_CLASS[]						= L"class";
const WCHAR Screen::SZ_STYLE[]						= L"style";
const WCHAR Screen::SZ_ID[]							= L"id";
const WCHAR Screen::SZ_FLAGS[]						= L"flags";
const WCHAR Screen::SZ_POSITION[]					= L"position";
const WCHAR Screen::SZ_BLEND[]						= L"blend.color";
const WCHAR Screen::SZ_BGCOLOR[]					= L"background.color";
const WCHAR Screen::SZ_BG[]							= L"background.material";
const WCHAR Screen::SZ_SIZE[]						= L"size";

const WCHAR Screen::SZ_POSITION_CENTER[]			= L"center";
const WCHAR Screen::SZ_SIZE_FULLSCREEN[]			= L"fullscreen";


const LPCWSTR Screen::SZ_FLAGS_VALUES[] =		{	
													  L"default",
													  L"disabled",
													  L"modal",
													  L"topmost",
													  L"noactivate",
													  L"buffer",
													  L"bufferalpha",
													  L"invisible",
													  L"bgimage",
													  L"bgcolor",
													  L"transparent",
													  L"clip",											  
												};

const DWORD Screen::DW_FLAGS_VALUES[] =			{			
													  Screen::DEFAULT,
													  Screen::DISABLED,
													  Screen::MODAL,
													  Screen::TOPMOST,
													  Screen::NOACTIVATE,
													  Screen::BUFFER,
													  Screen::BUFFERALPHA,
													  Screen::INVISIBLE,
													  Screen::BACKGROUND,
													  Screen::BACKGROUNDCOLOR,
													  Screen::BACKGROUNDTRANSPARENT,
													  Screen::CLIP											  
												};

const Vector2 Screen::CLIENT_POS(0.0f, 0.0f);


/*----------------------------------------------------------*\
| ScreenChildManager implementation
\*----------------------------------------------------------*/

ScreenChildManager::ScreenChildManager(void)
{
}

ScreenChildManager::~ScreenChildManager(void)
{
	Empty();

	m_arScreens.clear();
}

void ScreenChildManager::Add(Screen* pScreen, bool bTopMost)
{
	if (NULL == pScreen)
		throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	if (true == bTopMost)
	{
		// Add to the very top of Z-order (top of the list) without checking flags

		m_arScreens.push_back(pScreen);
	}
	else
	{
		// Add to the bottom of Z-order (bottom of the list) without checking flags

		m_arScreens.insert(m_arScreens.begin(), pScreen);
	}

	// Re-cache position and size after insertion

	pScreen->CachePosition();
	pScreen->OnMove(pScreen->GetPosition());
	pScreen->OnSize(pScreen->GetSize());
}

void ScreenChildManager::Add(Screen* pScreen, ScreenListIterator pos)
{
	if (NULL == pScreen)
		throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	m_arScreens.insert(pos, pScreen);

	// Re-cache position and size after insertion

	pScreen->CachePosition();
	pScreen->OnMove(pScreen->GetPosition());
	pScreen->OnSize(pScreen->GetSize());
}

void ScreenChildManager::Remove(Screen* pScreen, bool bDeallocate)
{
	if (NULL == pScreen)
		throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	ScreenListIterator posFind;

	if (Find(pScreen, posFind, false) == false) return;

	// Null out the screen, will be removed on next Render

	*posFind = NULL;

	if (true == bDeallocate)
		delete pScreen;
}

void ScreenChildManager::Remove(ScreenListIterator pos, bool bDeallocate)
{
	if (true == bDeallocate)
		delete *pos;

	// Null out the screen, will be removed on next Render iteration

	*pos = NULL;
}

void ScreenChildManager::RemoveAll(void)
{
	for(ScreenListIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{
		// Null out the screen, will be removed on next render

		Screen* pRemove = *pos;

		*pos = NULL;

		delete pRemove;
	}
}

bool ScreenChildManager::Find(Screen* pScreenFind,
							  ScreenListIterator& posFind,
							  bool bSearchChildren)
{
	for(ScreenListIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{		
		if (*pos == pScreenFind)
		{
			posFind = pos;
			return true;
		}
		else
		{
			if (true == bSearchChildren &&
			   (*pos)->GetChildren().GetCount())
			{
				ScreenListIterator posChild;
				
				if ((*pos)->GetChildren().Find(pScreenFind, posChild, true))
				{
					posFind = posChild;
					return true;
				}				
			}
		}
	}

	return false;
}

Screen* ScreenChildManager::FindByID(int nID, bool bSearchChildren)
{
	for(ScreenListIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{		
		if ((*pos)->GetID() == nID)
		{
			return *pos;
		}
		else
		{
			if (true == bSearchChildren && (*pos)->GetChildren().GetCount())
			{
				Screen* pChild =
					(*pos)->GetChildren().FindByID(nID, true);

				if (pChild) return pChild;
			}
		}
	}

	return NULL;
}

Screen* ScreenChildManager::FindByName(LPCWSTR pszName, bool bSearchChildren)
{
	for(ScreenListIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{		
		if ((*pos)->GetName() == pszName)
		{
			return *pos;
		}
		else
		{
			if (true == bSearchChildren && (*pos)->GetChildren().GetCount())
			{
				Screen* pChild =
					(*pos)->GetChildren().FindByName(pszName, true);

				if (pChild != NULL) return pChild;
			}
		}
	}

	return NULL;
}

Screen* ScreenChildManager::FindByClass(LPCWSTR pszClass, bool bSearchChildren)
{
	for(ScreenListIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{		
		if ((*pos)->GetClass() == pszClass)
		{
			return *pos;
		}
		else
		{
			if (true == bSearchChildren && (*pos)->GetChildren().GetCount())
			{
				Screen* pChild =
					(*pos)->GetChildren().FindByClass(pszClass, true);

				if (pChild != NULL)
					return pChild;
			}
		}
	}

	return NULL;
}

Screen* ScreenChildManager::ScreenFromPoint(POINT ptPos,
											bool bIgnoreDisabled,
											bool bIgnoreInvisible)
{
	// Return a screen under this point (absolute coordinates)
	// This function is recursive, and we want to keep stack size down

	static RECT rcAbs;

	for(ScreenListReverseIterator pos = m_arScreens.rbegin();
		pos != m_arScreens.rend();
		pos++)
	{
		Screen* pScreen = *pos;

		// If this screen has been removed, skip it

		if (NULL == pScreen) continue;

		// If this screen is disabled and ignoring disabled, skip it

		if (true == bIgnoreDisabled && pScreen->IsFlagSet(Screen::DISABLED) == true)
			continue;

		// If this screen is invisible and ignoring invisibles, skip it

		if (true == bIgnoreInvisible && pScreen->IsFlagSet(Screen::INVISIBLE) == true)
			continue;

		// Get this screen's absolute rectangle

		pScreen->GetAbsRect(rcAbs);

		// If this point is within that screen (or if it's modal),
		// check it's children

		if (pScreen->IsFlagSet(Screen::MODAL) == true ||
			PtInRect(&rcAbs, ptPos) == TRUE)
		{
			Screen* pChild =
				pScreen->GetChildren().ScreenFromPoint(ptPos,
					bIgnoreDisabled, bIgnoreInvisible);

			if (pChild != NULL)
			{
				// If this point is within one of the children, return that child

				return pChild;
			}
			else
			{
				// If this point is not within one of the children, return this screen

				return pScreen;
			}
		}
	}

	// We did not find any screens that match the criteria

	return NULL;
}

void ScreenChildManager::Render(void)
{
	for(ScreenListIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{
		// Remove any deallocated screens from the list

		while(NULL == *pos)
		{
			pos = m_arScreens.erase(pos);
			if (pos == m_arScreens.end()) return;
		}

		// Render if visible

		if ((*pos)->IsFlagSet(Screen::INVISIBLE) == false)
		   (*pos)->Render();
	}
}

void ScreenChildManager::Render(const RECT& rrc)
{
	RECT rcChild;

	for(ScreenListIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{
		// Remove any deallocated screens from the list

		while(NULL == *pos)
		{
			pos = m_arScreens.erase(pos);
			if (pos == m_arScreens.end()) return;
		}

		// Render if visible and intersects rectangle

		if ((*pos)->IsFlagSet(Screen::INVISIBLE) == false)
		{
			if (IntersectRect(&rcChild, &rrc,
				&(*pos)->GetBufferRect()) == TRUE)
			{
				OffsetRect(&rcChild, -(*pos)->GetPosition().x,
									 -(*pos)->GetPosition().y);
				
				(*pos)->Render(rcChild);
			}
		}
	}
}

void ScreenChildManager::OnLostDevice(bool bRecreate)
{
	for(ScreenListConstIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{
		(*pos)->OnLostDevice(bRecreate);
	}
}

void ScreenChildManager::OnResetDevice(bool bRecreate)
{
	for(ScreenListConstIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{
		(*pos)->OnResetDevice(bRecreate);
	}
}

DWORD ScreenChildManager::GetMemoryFootprint(void) const
{
	DWORD dwSize = 0;

	for(ScreenListConstIterator pos = m_arScreens.begin();
		pos != m_arScreens.end();
		pos++)
	{
		dwSize += (*pos)->GetMemoryFootprint();
	}

	return dwSize;
}

/*----------------------------------------------------------*\
| ScreenManager class
\*----------------------------------------------------------*/

ScreenManager::ScreenManager(Engine& rEngine):
							 m_rEngine(rEngine),
							 m_pActiveScreen(NULL),
							 m_pFocusScreen(NULL),
							 m_pCaptureScreen(NULL),
							 m_pHoverScreen(NULL),
							 m_Theme(rEngine)
{
	ZeroMemory(m_szBasePath, sizeof(m_szBasePath));
	ZeroMemory(m_szBaseExt, sizeof(m_szBaseExt));
}

ScreenManager::~ScreenManager(void)
{
}

Screen* ScreenManager::Create(LPCWSTR pszClass, Screen* pParent)
{
	Screen* pNew = NULL;

	if (String::IsEmpty(pszClass) == false)
	{
		pNew = dynamic_cast<Screen*>(
			m_rEngine.GetClasses().Create(pszClass, pParent));

		if (NULL == pNew)
			throw m_rEngine.GetErrors().Push(Error::CLASS_CREATE,
				__FUNCTIONW__, pszClass);
	}
	else
	{
		try
		{
			pNew = new Screen(m_rEngine, pszClass);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC,
				__FUNCTIONW__, sizeof(Screen));
		}
	}

	return pNew;
}

Screen* ScreenManager::Load(LPCWSTR pszPath)
{
	// Validate path

	if (String::IsEmpty(pszPath) == true)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	// Make sure we are using full path

	WCHAR szFullPath[MAX_PATH] = {0};

	if (*(PathFindExtension(pszPath)) != L'\0')
	{
		GetAbsolutePath(pszPath, szFullPath);
	}
	else
	{
		m_rEngine.GetBaseFilePath(pszPath,
			m_szBasePath, m_szBaseExt, szFullPath);
	}

	// Open file

	Stream stream(&m_rEngine.GetErrors(),
		m_rEngine.GetOption(Engine::OPTION_ENABLE_STREAM_CACHE) ?
		&m_rEngine.GetStreamCache() : NULL);

	try
	{
		stream.Open(szFullPath, GENERIC_READ,
			OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN,
			__FUNCTIONW__, pszPath);
	}

	InfoFile screenfile(&m_rEngine.GetErrors());

	try
	{
		screenfile.Deserialize(stream);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, pszPath);
	}

	if (screenfile.GetRoot() == NULL ||
		screenfile.GetRoot()->GetName() != Screen::SZ_ROOT)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, pszPath, Screen::SZ_ROOT);
	}

	// Try to find the class name. If can't find, then screen
	// doesn't have a class

	const InfoElem* pClassElem =
		screenfile.GetRoot()->FindChildConst(Screen::SZ_CLASS);

	if (pClassElem != NULL &&
	   pClassElem->GetVarType() != Variable::TYPE_STRING)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, pszPath, Screen::SZ_CLASS);

	// Set current directory to screen directory

	PUSH_CURRENT_DIRECTORY(szFullPath);

	// Load screen

	Screen* pScreen =
		Create(pClassElem ? pClassElem->GetStringValue() : NULL);

	try
	{		
		pScreen->Deserialize(*screenfile.GetRoot());
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		delete pScreen;

		SetCurrentDirectory(szCurDir);
		
		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, pszPath);
	}

	// Restore current directory

	POP_CURRENT_DIRECTORY();

	// Add to top level list

	pScreen->ZOrder(Screen::ZORDER_ADD);

	// Fire OnMove and OnSize for the first time

	pScreen->OnMove(pScreen->GetPosition());
	pScreen->OnSize(pScreen->GetSize());

	return pScreen;
}

Screen* ScreenManager::Show(LPCWSTR pszPath)
{
	Screen* pScreen = Load(pszPath);

	if (pScreen->IsFlagSet(Screen::NOACTIVATE) == true)
	{
		pScreen->ZOrder(Screen::ZORDER_TOP);
	}
	else
	{
		SetActiveScreen(pScreen);
		SetFocusScreen(pScreen);
	}

	return pScreen;
}

Screen* ScreenManager::GetForegroundScreen(void)
{
	if (m_arScreens.empty() == true)
		return NULL;

	return *GetBeginPos();
}

Screen* ScreenManager::SetForegroundScreen(Screen* pForegroundScreen)
{
	Screen* pPrevForegroundScreen = GetForegroundScreen();

	pForegroundScreen->ZOrder(Screen::ZORDER_TOP);

	return pPrevForegroundScreen;
}

Screen* ScreenManager::SetActiveScreen(Screen* pActiveScreen)
{
	if (pActiveScreen != NULL &&
		pActiveScreen->IsFlagSet(Screen::NOACTIVATE) == true)
	{
		return m_pActiveScreen;
	}

	if (m_pActiveScreen == pActiveScreen) return pActiveScreen;

	Screen* pOld = m_pActiveScreen;
	m_pActiveScreen = pActiveScreen;

	if (m_rEngine.GetOption(Engine::OPTION_SCREEN_EVENTS) == TRUE)
	{
		if (pOld != NULL)
			pOld->OnDeactivate(pActiveScreen);

		if (pActiveScreen != NULL)
			pActiveScreen->OnActivate(pOld);
	}

	return pOld;
}

Screen* ScreenManager::GetActiveScreen(void) const
{
	return m_pActiveScreen;
}

Screen* ScreenManager::SetFocusScreen(Screen* pFocusScreen)
{
	if (m_pFocusScreen == pFocusScreen ||
	   (pFocusScreen != NULL &&
		pFocusScreen->IsFlagSet(Screen::DISABLED) == true))
		return pFocusScreen;

	Screen* pOld = m_pFocusScreen;
	m_pFocusScreen = pFocusScreen;

	if (m_rEngine.GetOption(Engine::OPTION_SCREEN_EVENTS) == TRUE)
	{
		if (pOld != NULL) pOld->OnDefocus(pFocusScreen);
		if (pFocusScreen != NULL) pFocusScreen->OnFocus(pOld);
	}

	return pOld;
}

Screen* ScreenManager::GetFocusScreen(void) const
{
	return m_pFocusScreen;
}

Screen* ScreenManager::SetCaptureScreen(Screen* pCaptureScreen)
{
	if (m_pCaptureScreen == pCaptureScreen) return pCaptureScreen;

	Screen* pOld = m_pCaptureScreen;
	m_pCaptureScreen = pCaptureScreen;

	if (m_rEngine.GetOption(Engine::OPTION_SCREEN_EVENTS) == TRUE)
	{
		if (pOld != NULL)
			pOld->OnDecapture(pCaptureScreen);

		if (pCaptureScreen != NULL)
			pCaptureScreen->OnCapture(pOld);
	}

	return pOld;
}

Screen* ScreenManager::GetCaptureScreen(void) const
{
	return m_pCaptureScreen;
}

Screen* ScreenManager::GetHoverScreen(void) const
{
	return m_pHoverScreen;
}

Screen* ScreenManager::SetHoverScreen(Screen* pHoverScreen)
{
	if (m_pHoverScreen == pHoverScreen) return pHoverScreen;

	Screen* pOld = m_pHoverScreen;
	m_pHoverScreen = pHoverScreen;

	if (m_rEngine.GetOption(Engine::OPTION_SCREEN_EVENTS) == TRUE)
	{
		if (pOld != NULL)
			pOld->OnMouseLeave();

		if (pHoverScreen != NULL)
			pHoverScreen->OnMouseEnter();
	}

	return pOld;
}

void ScreenManager::SetTheme(LPCWSTR pszPath)
{
	if (NULL == pszPath)
		return;

	m_Theme.Deserialize(pszPath);

	// Notify game of theme change

	if (m_rEngine.GetClientInstance())
		m_rEngine.GetClientInstance()->OnThemeChange(m_Theme);

	// Notify top level screens of theme change

	for(ScreenListIterator pos = GetBeginPos();
		pos != GetEndPos();
		pos++)
	{
		(*pos)->OnThemeChange(m_Theme);
	}
}

void ScreenManager::SetBasePath(LPCWSTR pszBasePath)
{
	wcscpy_s(m_szBasePath, sizeof(m_szBasePath) / sizeof(WCHAR),
		pszBasePath);
}

void ScreenManager::SetBaseExtension(LPCWSTR pszBaseExtension)
{
	wcscpy_s(m_szBaseExt, sizeof(m_szBaseExt) / sizeof(WCHAR),
		pszBaseExtension);
}

void ScreenManager::Empty(void)
{
	// Deallocate all screens

	ScreenChildManager::Empty();

	// Deallocate theme styles and elements

	m_Theme.Empty();

	// Reset pointers

	m_pActiveScreen = NULL;
	m_pFocusScreen = NULL;
	m_pCaptureScreen = NULL;
	m_pHoverScreen = NULL;
}

/*----------------------------------------------------------*\
| Screen implementation
\*----------------------------------------------------------*/

Screen::Screen(Engine& rEngine, LPCWSTR pszClass, Screen* pParent):
			   Object(rEngine),
			   m_pParent(pParent),
			   m_nID(0),
			   m_bCenter(false),
			   m_Buffer(rEngine),
			   m_clrBlend(Color::BLEND_ONE),
			   m_clrBackColor(Color::BLEND_ONE),
			   m_pBufferScreen(NULL),
			   m_strClass(pszClass),
			   m_pStyle(NULL)
{
	m_ptPos.x = 0;
	m_ptPos.y = 0;

	m_psSize.cx = 0;
	m_psSize.cy = 0;

	SetRectEmpty(&m_rcCachedRect);
}

Screen::~Screen(void)
{
	Empty();
}

void Screen::SetStyle(LPCWSTR pszStyle)
{
	bool bNew = (m_strStyle != pszStyle);

	m_strStyle = pszStyle;

	// Cache style pointer

	if (m_strStyle.IsEmpty() == true)
	{
		m_pStyle = NULL;
	}
	else
	{
		m_pStyle =
			m_rEngine.GetScreens().GetTheme().GetStyle(m_strStyle);

		if (NULL == m_pStyle)
			throw m_rEngine.GetErrors().Push(Error::THEME_INVALIDSTYLE,
				__FUNCTIONW__, m_strStyle);
	}

	if (true == bNew)
		OnThemeStyleChange();
}

bool Screen::LoadMaterialInstance(
	MaterialInstance& rOutInstance,
	LPCWSTR pszElementName,
	const InfoElem* pSource1,
	ThemeStyle* pSource2,
	ThemeStyle* pSource3)
{
	if (pSource1 != NULL)
	{
		const InfoElem* pElem =
			pSource1->FindChildConst(pszElementName);

		if (pElem != NULL)
		{
			rOutInstance.Deserialize(m_rEngine, *pElem);
			return true;
		}
	}

	const MaterialInstance* pInst = NULL;

	if (pSource2 != NULL)
	{
		pInst =
			pSource2->GetMaterialInstanceConst(pszElementName);

		if (pInst != NULL)
		{
			rOutInstance = *pInst;
			return true;
		}
	}

	if (pSource3 != NULL)
	{
		pInst =
			pSource3->GetMaterialInstanceConst(pszElementName);

		if (pInst != NULL)
		{
			rOutInstance = *pInst;
			return true;
		}
	}

	return false;
}

bool Screen::LoadFont(Font** ppOutFont,
	LPCWSTR pszElementName,
	const InfoElem* pSource1,
	ThemeStyle* pSource2,
	ThemeStyle* pSource3)
{
	Font* pFont = NULL;

	if (pSource1 != NULL)
	{
		const InfoElem* pElem =
			pSource1->FindChildConst(pszElementName);

		if (pElem != NULL)
		{
			pFont = m_rEngine.GetFonts().LoadInstance(*pElem);

			if (pFont != NULL)
			{
				pFont->AddRef();

				SAFERELEASE((*ppOutFont));

				*ppOutFont = pFont;

				return true;
			}
		}
	}	

	if (pSource2 != NULL)
	{
		pFont = pSource2->GetFont(pszElementName);

		if (pFont != NULL)
		{
			pFont->AddRef();

			SAFERELEASE((*ppOutFont));

			*ppOutFont = pFont;

			return true;
		}
	}

	if (pSource3 != NULL)
	{
		pFont = pSource3->GetFont(pszElementName);

		if (pFont != NULL)
		{
			pFont->AddRef();

			SAFERELEASE((*ppOutFont));

			*ppOutFont = pFont;

			return true;
		}
	}

	return false;
}

bool Screen::LoadColor(Color& rOutColor,
	LPCWSTR pszElementName,
	const InfoElem* pSource1,
	ThemeStyle* pSource2,
	ThemeStyle* pSource3)
{
	if (pSource1 != NULL)
	{
		const InfoElem* pElem = pSource1->FindChildConst(pszElementName);

		if (pElem != NULL)
		{
			rOutColor.Deserialize(*pElem);

			return true;
		}
	}

	const Color* pColor = NULL;

	if (pSource2 != NULL)
	{
		pColor = pSource2->GetColor(pszElementName);

		if (pColor != NULL)
		{
			rOutColor = *pColor;
			return true;
		}
	}

	if (pSource3 != NULL)
	{
		pColor = pSource3->GetColor(pszElementName);

		if (pColor != NULL)
		{
			rOutColor = *pColor;
			return true;
		}
	}

	return false;
}

bool Screen::LoadVariable(const Variable** ppOutVar,
	LPCWSTR pszElementName,	
	Variable::Types nType1,
	Variable::Types nType2,
	const InfoElem* pSource1,
	ThemeStyle* pSource2,
	ThemeStyle* pSource3)
{
	const Variable* pFind = NULL;

	if (pSource1 != NULL)
	{
		pFind = pSource1->FindChildConst(pszElementName,
			InfoElem::TYPE_ANY, nType1);

		if (NULL == pFind && nType2 != Variable::TYPE_UNDEFINED)
			pFind = pSource1->FindChildConst(pszElementName,
				InfoElem::TYPE_ANY, nType2);
	}

	if (NULL == pFind && pSource2 != NULL)
	{
		pFind = pSource2->GetVariable(pszElementName);
	}

	if (NULL == pFind && pSource3 != NULL)
	{
		pFind = pSource3->GetVariable(pszElementName);
	}

	if (pFind != NULL)
	{
		*ppOutVar = pFind;
		return true;
	}

	return false;
}

void Screen::SetFlags(DWORD dwFlags)
{
	if ((~m_dwFlags & CLIP) && (dwFlags & CLIP))
		dwFlags &= ~CLIP;

	Object::SetFlags(dwFlags);
}

bool Screen::IsVisible(void) const
{
	// Return true if tracing to topmost level reveals no hidden parents
	if (IsFlagSet(INVISIBLE))
	{
		return false;
	}

	Screen* pParent = m_pParent;

	while (pParent != NULL)
	{
		if (pParent->IsFlagSet(INVISIBLE))
		{
			return false;
		}

		pParent = pParent->m_pParent;
	}

	return true;
}

bool Screen::IsInteractive(void) const
{
	// Return true if tracing to topmost level revails no hidden or disabled parents
	if (IsFlagSet(INVISIBLE) || IsFlagSet(DISABLED))
	{
		return false;
	}

	Screen* pParent = m_pParent;

	while (pParent != NULL)
	{
		if (pParent->IsFlagSet(INVISIBLE) || pParent->IsFlagSet(DISABLED))
		{
			return false;
		}

		pParent = pParent->m_pParent;
	}

	return true;
}

Color Screen::GetFrontBufferBlend(void) const
{
	if (NULL == m_pParent || m_pParent == m_pBufferScreen)
		return m_clrBlend;

	float fAlpha = m_clrBlend.GetAFloat();

	if (m_pParent != NULL)
		fAlpha *= m_pParent->GetFrontBufferBlend().GetAFloat();

	return Color(m_clrBlend.GetRFloat(), m_clrBlend.GetGFloat(),
		m_clrBlend.GetBFloat(), fAlpha);
}

void Screen::CreateBuffer(void)
{
	if (IsFlagSet(BUFFER) == true)
	{
		// If already had buffer, release it

		m_Buffer.Empty();
	}
	else
	{
		// Set the buffer flag to indicate this screen now has a back buffer

		SetFlag(BUFFER);
	}

	// Create buffer

	m_Buffer.Allocate(m_psSize.cx, m_psSize.cy, IsFlagSet(BUFFERALPHA));
	m_Buffer.SetName(m_strName + L"-buffer.png");

	// Copy material from background, and set texture & texture coords

	if (m_Background.IsEmpty() == true)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	m_BufferInst.SetMaterial(m_Background.GetMaterial());
	m_BufferInst.SetBaseTexture(&m_Buffer);
	m_BufferInst.SetTextureCoords(0, 0, m_psSize.cx, m_psSize.cy);

	// Update buffer for the first time

	Invalidate();
}

void Screen::SetPosition(int x, int y)
{
	POINT ptOldPos = { m_ptPos.x, m_ptPos.y };

	m_ptPos.x = x;
	m_ptPos.y = y;

	// Cache positions

	CachePosition();

	// Fire move events

	OnMove(ptOldPos);
}

void Screen::SetSize(int cx, int cy)
{
	SIZE psOldSize = { m_psSize.cx, m_psSize.cy };

	m_psSize.cx = cx;
	m_psSize.cy = cy;

	// Update client rectangle

	m_BufferInst.SetTextureCoords(
		m_BufferInst.GetTextureCoords().left,
		m_BufferInst.GetTextureCoords().top,
		cx,
		cy);

	// Update cached rectangle

	m_rcCachedRect.right = m_rcCachedRect.left + cx;
	m_rcCachedRect.bottom = m_rcCachedRect.top + cy;

	// If buffered, re-create buffer

	if (IsFlagSet(BUFFER) == true)
		CreateBuffer();

	OnSize(psOldSize);
}

void Screen::GetAbsRect(RECT& rrc) const
{
	rrc.left = m_ptPos.x;
	rrc.top = m_ptPos.y;

	for(Screen* pAbove = m_pParent;
		pAbove != NULL;
		pAbove = pAbove->m_pParent)
	{
		rrc.left += pAbove->m_ptPos.x;
		rrc.top += pAbove->m_ptPos.y;
	}

	rrc.right = rrc.left + m_psSize.cx;
	rrc.bottom = rrc.top + m_psSize.cy;
}

void Screen::GetAbsPos(POINT& rpt) const
{
	rpt.x = m_ptPos.x;
	rpt.y = m_ptPos.y;

	for(Screen* pAbove = m_pParent;
		pAbove != NULL;
		pAbove = pAbove->m_pParent)
	{
		rpt.x += pAbove->m_ptPos.x;
		rpt.y += pAbove->m_ptPos.y;
	}
}

void Screen::CachePosition(void)
{
	// If non top-level, use coordinates on parent's buffer
	// (could become absolute rect if no buffered parents found)

	m_rcCachedRect.left = m_ptPos.x;
	m_rcCachedRect.top = m_ptPos.y;

	Screen* pAbove = m_pParent;

	for(; pAbove != NULL && pAbove->IsFlagSet(BUFFER) == false;
		pAbove = pAbove->m_pParent)
	{
		m_rcCachedRect.left += pAbove->m_ptPos.x;
		m_rcCachedRect.top += pAbove->m_ptPos.y;
	}

	m_rcCachedRect.right = m_rcCachedRect.left + m_psSize.cx;
	m_rcCachedRect.bottom = m_rcCachedRect.top + m_psSize.cy;

	if (pAbove != NULL && pAbove->IsFlagSet(BUFFER) == true)
		m_pBufferScreen = pAbove;

	m_vecCachedPos.x = float(m_rcCachedRect.left);
	m_vecCachedPos.y = float(m_rcCachedRect.top);

	// Cache positions for children

	for(ScreenListIterator pos = m_lstChildren.GetBeginPos();
		pos != m_lstChildren.GetEndPos();
		pos++)
	{
		if ((*pos) != NULL)
			(*pos)->CachePosition();
	}
}

void Screen::CacheStyle(void)
{
	String strClassPath = m_strClass;

	for(;;)
	{
		if (m_strStyle.IsEmpty() == false)
		{
			// Get stand-alone or class style

			if (m_strClass.IsEmpty() == true)
				m_pStyle = m_rEngine.GetScreens().GetTheme().GetStyle(m_strStyle);
			else
				m_pStyle = m_rEngine.GetScreens().GetTheme().GetStyle(m_strStyle,
					strClassPath);
		}
		else
		{
			// If no style specified, try to get default class style

			m_pStyle =
				m_rEngine.GetScreens().GetTheme().GetDefaultClassStyle(strClassPath);			
		}

		if (NULL == m_pStyle)
		{
			// If can't find style, go one level up the chain to parent class
			// This is done so we don't have to duplicate a style
			// for each child of a parent - waste of resources and clutter

			if (strClassPath.IsEmpty() == true)
				return;

			int nPos = strClassPath.ReverseFind(L':');

			if (INVALID_INDEX == nPos || strClassPath[nPos - 1] != L':')
				break;

			_wcsset(strClassPath.GetBuffer() + nPos - 1, L'\0');
		}
		else
		{
			break;
		}
	}

	if (NULL == m_pStyle)
	{
		if (m_strStyle.IsEmpty() == false)
			throw m_rEngine.GetErrors().Push(Error::THEME_INVALIDSTYLE,
				__FUNCTIONW__, m_strStyle);
	}
	else
	{
		// Notify theme style change

		OnThemeStyleChange();
	}
}

void Screen::Activate(void)
{
	// If child, have to activate parent

	if (m_pParent != NULL)
		m_pParent->Activate();
	else
		m_rEngine.GetScreens().SetActiveScreen(this);
}

void Screen::Deactivate(void)
{
	if (m_rEngine.GetScreens().GetActiveScreen() == this)
	{
		ScreenListIterator pos;

		if (m_rEngine.GetScreens().Find(this, pos) &&
			pos != m_rEngine.GetScreens().GetBeginPos() &&
			m_rEngine.GetScreens().GetCount() > 1)
		{
			pos--;

			while((*pos)->IsFlagSet(NOACTIVATE) == true ||
				  (*pos)->IsFlagSet(DISABLED) == true ||
				  (*pos)->IsFlagSet(INVISIBLE) == true)
			{
				if (pos == m_rEngine.GetScreens().GetBeginPos())
				{
					if (m_rEngine.GetScreens().GetFocusScreen() == this)
						m_rEngine.GetScreens().SetFocusScreen(NULL);

					m_rEngine.GetScreens().SetActiveScreen(NULL);

					return;
				}

				pos--;
			}

			m_rEngine.GetScreens().SetFocusScreen(*pos);
			m_rEngine.GetScreens().SetActiveScreen(*pos);

			return;
		}

		m_rEngine.GetScreens().SetFocusScreen(NULL);
		m_rEngine.GetScreens().SetActiveScreen(NULL);		
	}
	else
	{
		if (m_rEngine.GetScreens().GetFocusScreen() == this)
			m_rEngine.GetScreens().SetFocusScreen(NULL);

		OnDeactivate(NULL);		
	}
}

void Screen::ZOrder(ZOrderOps nZOrder)
{
	// If don't have a parent, use engine's top-level screens

	ScreenChildManager& rList = m_pParent != NULL ?
		m_pParent->GetChildren() : m_rEngine.GetScreens();

	// Find this screen in the chosen list, exit if not found (and not adding)

	ScreenListIterator pos;

	if (nZOrder != ZORDER_ADD && !rList.Find(this, pos))
	   return;

	switch(nZOrder)
	{
	case ZORDER_TOP:
		{
			// Add as high as possible, below existing top-most screens

			ScreenListIterator posNew = rList.GetEndPos();

			do
			{
				posNew--;

				if (NULL == *posNew)
				{
					if (posNew == rList.GetBeginPos())
						break;
					else
						continue;
				}

				if (posNew == pos) return;

			} while(*posNew != NULL &&
					(*posNew)->IsFlagSet(TOPMOST) &&
					posNew != rList.GetBeginPos());

			// Add at this position

			posNew++;

			rList.Add(this, posNew);

			// Remove

			rList.Remove(pos, false);
		}
		break;
	case ZORDER_BOTTOM:
		{
			// Add to very bottom

			rList.Add(this, false);

			// Remove

			rList.Remove(pos, false);
		}
		break;
	case ZORDER_UP:
		{
			// Move up in z-order

			if (rList.GetTopMost() != this)
			{
				ScreenListIterator posOld = pos;

				for(;;)
				{
					pos++;

					if (pos == rList.GetEndPos())
					{
						rList.Add(this, true);
						return;
					}

					if ((*pos)->IsFlagSet(TOPMOST))
					{
						pos--;
						break;
					}
				}

				if (pos != posOld)
				{
					rList.Add(this, pos);				
					rList.Remove(posOld, false);
				}
			}
		}
		break;
	case ZORDER_DOWN:
		{
			// Move down in z-order (1 up screen list)

			if (rList.GetBottomMost() != this)
			{
				ScreenListIterator posOld = pos;

				for(;;)
				{
					pos--;

					if (rList.GetBeginPos() == pos)
					{
						rList.Add(this, false);
						return;
					}
				}				

				/*THIS IS SUPPOSED TO REMOVE FROM OLD POS if (pos != posOld)
				{
					_ASSERT(FALSE);

					rList.Add(this, pos);
					rList.Remove(posOld, false);
				}*/
			}
		}
		break;
	case ZORDER_REMOVE:
		{
			// Remove this screen from the Z-order list

			rList.Remove(this, false);
		}
		break;
	case ZORDER_ADD:
		{
			if (rList.GetCount() == 0)
			{
				rList.Add(this);
				return;
			}

			if (IsFlagSet(TOPMOST) == true)
			{
				// Add above all top-most screens

				rList.Add(this, true);
			}
			else
			{
				// Add as high as possible, below existing top-most screens

				pos = rList.GetEndPos();

				do
				{
					pos--;
			
					if (NULL == *pos) continue;

				} while(pos != rList.GetBeginPos() && (*pos)->IsFlagSet(TOPMOST));

				pos++;

				rList.Add(this, pos);
			}
		}
		break;
	}
}

void Screen::OnRenderBackground(Graphics& rGraphics)
{
	// Clear render target with background color

	if (IsFlagSet(BACKGROUNDCOLOR) == true)
	{
		if (IsFlagSet(BUFFER) == false)
			rGraphics.FlushBatch();

		Rect rc = m_BufferInst.GetTextureCoords();

		rGraphics.Clear(m_clrBackColor,
			IsFlagSet(BUFFER) ? &rc : &m_rcCachedRect);
	}

	// Render background texture

	if (IsFlagSet(BACKGROUND) == true && m_Background.IsEmpty() == false)
	{
		if (IsFlagSet(BUFFERALPHA) == true)
		{
			rGraphics.FlushBatch();

			rGraphics.GetStates()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		}

		rGraphics.BeginBatch();

		// Render the whole background at top left corner

		rGraphics.RenderQuad(m_Background,
			IsFlagSet(BUFFER) ? Vector2() : m_vecCachedPos,
			GetFrontBufferBlend());

		rGraphics.EndBatch();

		if (IsFlagSet(BUFFERALPHA) == true)
			rGraphics.GetStates()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	}
}

void Screen::OnRenderBackground(Graphics& rGraphics, const RECT& rrc)
{
	// Render background color

	if (IsFlagSet(BACKGROUNDCOLOR) == true)
	{
		if (IsFlagSet(BUFFER) == true)
		{
			rGraphics.Clear(m_clrBackColor, &rrc);
		}
		else
		{
			rGraphics.FlushBatch();

			RECT rc = { m_rcCachedRect.left + rrc.left,
				m_rcCachedRect.top + rrc.top,
				m_rcCachedRect.left + rrc.right,
				m_rcCachedRect.top + rrc.bottom };

			rGraphics.Clear(m_clrBackColor, &rc);
		}
	}

	// Render background texture

	if (IsFlagSet(BACKGROUND) == true)
	{
		if (IsFlagSet(BUFFERALPHA) == true)
		{
			rGraphics.FlushBatch();

			rGraphics.GetStates()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		rGraphics.BeginBatch();

		// Render a subset of the background at a specific place

		RECT rcSrc;
		CopyRect(&rcSrc, &rrc);

		OffsetRect(&rcSrc, m_Background.GetTextureCoords().left,
			m_Background.GetTextureCoords().top);

		Vector3 vecPos(0.0f, 0.0f, 0.0f);

		if (IsFlagSet(BUFFER))
		{
			vecPos.x = float(rrc.left);
			vecPos.y = float(rrc.top);
		}
		else
		{
			vecPos.x = m_vecCachedPos.x + float(rrc.left);
			vecPos.y = m_vecCachedPos.y + float(rrc.top);
		}

		rGraphics.RenderQuad(m_Background, vecPos,
			IsFlagSet(BUFFER) ? Color::BLEND_ONE : GetFrontBufferBlend());

		rGraphics.EndBatch();

		if (IsFlagSet(BUFFERALPHA) == true)
			rGraphics.GetStates()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	}
}

void Screen::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Default handler

	if (prc != NULL)
	{
		// Render background

		OnRenderBackground(rGraphics, *prc);

		// Render children

		m_lstChildren.Render(*prc);
	}
	else
	{
		// Render background

		OnRenderBackground(rGraphics);

		// Render children

		m_lstChildren.Render();
	}
}

void Screen::Render(void)
{
	Graphics& rGraphics = m_rEngine.GetGraphics();

	if (IsFlagSet(BUFFER) == true)
	{
		// Render buffer

		rGraphics.BeginBatch();

		if (m_pParent != NULL)
		{
			// If has parent and drawn directly on front buffer,
			// blend alpha values with parent

			rGraphics.RenderQuad(m_BufferInst,
				m_vecCachedPos, GetFrontBufferBlend());
		}
		else
		{
			// Use own alpha value

			rGraphics.RenderQuad(m_BufferInst,
				m_vecCachedPos, m_clrBlend);
		}

		rGraphics.EndBatch();
	}
	else
	{
		// If this screen has no buffer, call OnRender to render it
		// Do clipping if specified

		if (IsFlagSet(CLIP) == true)
			rGraphics.BeginClipping(m_rcCachedRect);

		OnRender(rGraphics);

		if (IsFlagSet(CLIP) == true)
			rGraphics.EndClipping();
	}
}

void Screen::Render(const RECT& rrc)
{
	Graphics& rGraphics = m_rEngine.GetGraphics();

	if (IsFlagSet(BUFFER) == true)
	{
		rGraphics.BeginBatch();

		Rect rcOldClient = m_BufferInst.GetTextureCoords();

		m_BufferInst.SetTextureCoords(rrc);

		// Render a portion of the buffer

		if (m_pParent != NULL)
		{
			// If has parent and drawn directly on front buffer,
			// blend alpha values with parent

			rGraphics.RenderQuad(m_BufferInst,
				m_vecCachedPos, GetFrontBufferBlend());
		}
		else
		{
			// Use own alpha value

			rGraphics.RenderQuad(m_BufferInst,
				m_vecCachedPos, m_clrBlend);
		}

		m_BufferInst.SetTextureCoords(rcOldClient);

		rGraphics.EndBatch();
	}
	else
	{
		// If this screen has no buffer, call OnRender to render it
		// Assuming that clipping is done by caller

		OnRender(rGraphics, &rrc);
	}
}

void Screen::Invalidate(void)
{
	Graphics& rGraphics = m_rEngine.GetGraphics();

	if (m_Buffer.GetD3DTexture() != NULL)
	{
		// Re-render self buffer

		m_Buffer.BeginScene();

		rGraphics.BeginBatch();

		OnRender(rGraphics);

		rGraphics.EndBatch();

		m_Buffer.EndScene();
	}

	if (m_pBufferScreen != NULL)
	{
		if (IsFlagSet(BACKGROUNDTRANSPARENT) == true ||
		   IsFlagSet(BUFFERALPHA) == true)
		{
			// Tell parent to re-render it's buffer in place of this screen

			m_pBufferScreen->Invalidate(m_rcCachedRect);
		}
		else
		{
			// If opaque, only re-render self on parent

			if (m_Buffer.GetD3DTexture() != NULL)
			{
				m_pBufferScreen->m_Buffer.BeginScene();

				rGraphics.BeginBatch();

				Render();

				rGraphics.EndBatch();

				m_pBufferScreen->m_Buffer.EndScene();
			}
		}
	}
}

void Screen::Invalidate(const RECT& rc)
{
	Graphics& rGraphics = m_rEngine.GetGraphics();

	if (m_Buffer.GetD3DTexture() != NULL)
	{
		// Re-render buffer

		m_Buffer.BeginScene();

		if (rGraphics.GetDeviceCaps().RasterCaps & D3DPRASTERCAPS_SCISSORTEST)
		{
			// Render a portion with clipping enabled	

			rGraphics.BeginClipping(rc);

			rGraphics.BeginBatch();

			OnRender(rGraphics, &rc);

			rGraphics.EndBatch();

			rGraphics.EndClipping();
		}
		else
		{
			// Render the whole buffer if clipping is unsupported

			rGraphics.BeginBatch();

			OnRender(rGraphics);

			rGraphics.EndBatch();
		}

		m_Buffer.EndScene();
	}

	if (m_pBufferScreen != NULL)
	{
		if (IsFlagSet(BACKGROUNDTRANSPARENT) == true ||
		   IsFlagSet(BUFFERALPHA) == true)
		{
			// Re-render parent buffer

			RECT rcClip = { rc.left, rc.top, rc.right, rc.bottom };

			OffsetRect(&rcClip, m_rcCachedRect.left, m_rcCachedRect.top);

			m_pBufferScreen->Invalidate(rcClip);
		}
		else
		{
			// Re-render only self on the parent buffer

			m_pBufferScreen->m_Buffer.BeginScene();

			rGraphics.BeginBatch();

			Render(rc);

			rGraphics.EndBatch();

			m_pBufferScreen->m_Buffer.EndScene();
		}
	}
}

void Screen::OnLostDevice(bool bRecreate)
{
	// Notify buffer

	if (IsFlagSet(BUFFER) == true)
		m_Buffer.OnLostDevice(bRecreate);

	// Notify all children

	for(ScreenListIterator pos = m_lstChildren.GetBeginPos();
		pos != m_lstChildren.GetEndPos();
		pos++)
	{
		(*pos)->OnLostDevice(bRecreate);
	}
}

void Screen::OnResetDevice(bool bRecreate)
{
	// Notify all children

	for(ScreenListIterator pos = m_lstChildren.GetBeginPos();
		pos != m_lstChildren.GetEndPos();
		pos++)
	{
		(*pos)->OnResetDevice(bRecreate);
	}

	// Notify buffer

	if (IsFlagSet(BUFFER) == true)
	{
		m_Buffer.OnResetDevice(bRecreate);

		Invalidate();
	}

	// Re-position after rendering change

	if (m_bCenter)
	{
		int x = (int(m_rEngine.GetGraphics().
			GetDeviceParams().BackBufferWidth) - m_psSize.cx) / 2;

		int y = (int(m_rEngine.GetGraphics().
			GetDeviceParams().BackBufferHeight) - m_psSize.cy) / 2;

		this->SetPosition(x, y);
	}
}

void Screen::SetParent(Screen* pNewParent)
{
	if (m_pParent == pNewParent)
		return;

	m_pParent = pNewParent;

	// Recache on-buffer position

	CachePosition();
}

bool Screen::IsDescendantOf(Screen *pScreen)
{
	if (this == pScreen)
		return true;

	Screen* pParent = m_pParent;

	while(pParent != NULL)
	{
		if (pParent == pScreen)
			return true;

		pParent = pParent->GetParent();
	}

	return false;
}

Screen* Screen::Duplicate(LPCWSTR pszClass)
{
	String strOldClass = m_strClass;

	m_strClass = pszClass;

	Screen* pNew = Duplicate();

	m_strClass = strOldClass;

	return pNew;
}

Screen* Screen::Duplicate(void) const
{
	// Create an instance of same class

	Screen* pNew = m_rEngine.GetScreens().Create(m_strClass, m_pParent);

	// Set base properties

	pNew->m_strStyle = m_strStyle;

	pNew->m_pStyle = m_pStyle;

	pNew->m_nID = m_nID;

	pNew->m_pBufferScreen = m_pBufferScreen;

	pNew->m_clrBackColor = m_clrBackColor;

	pNew->m_clrBlend = m_clrBlend;

	pNew->m_Background = m_Background;

	pNew->SetPosition(m_ptPos);

	pNew->SetSize(m_psSize);

	// Duplicate all children

	for(ScreenListConstIterator pos = m_lstChildren.GetBeginPosConst();
		pos != m_lstChildren.GetEndPosConst();
		pos++)
	{
		Screen* pNewChild = (*pos)->Duplicate();
		pNewChild->SetParent(pNew);

		pNew->GetChildren().Add(pNewChild);
	}

	return pNew;
}

void Screen::Deserialize(const InfoElem& rRoot)
{
	// Clear

	m_Background.Empty();

	// Read name

	if (rRoot.GetElemType() == InfoElem::TYPE_VALUEBLOCK &&
	   rRoot.GetVarType() == Variable::TYPE_STRING)
	{
		// If root element has a string value, use that as name

		m_strName = rRoot.GetStringValue();
	}
	else if (NULL == m_pParent)
	{
		// If name is not explicitly specified, take from file title

		WCHAR szName[MAX_PATH] = {0};

		wcscpy_s(szName, MAX_PATH,
			PathFindFileName(rRoot.GetDocumentConst().GetPath()));
	
		PathRemoveExtension(szName);

		m_strName = szName;
	}

	// Read ID (0 by default)

	const InfoElem* pElem =
		rRoot.FindChildConst(SZ_ID,
		InfoElem::TYPE_VALUE, Variable::TYPE_INT);

	if (pElem != NULL) m_nID = pElem->GetIntValue();

	// Read style

	pElem = rRoot.FindChildConst(SZ_STYLE,
		InfoElem::TYPE_VALUE, Variable::TYPE_STRING);

	if (pElem != NULL)
		m_strStyle = pElem->GetStringValue();

	CacheStyle();

	// Read flags (none by default)

	pElem = rRoot.FindChildConst(SZ_FLAGS);

	if (pElem != NULL)
	{
		SetFlag(pElem->ToFlags(SZ_FLAGS_VALUES,
			DW_FLAGS_VALUES,
			sizeof(DW_FLAGS_VALUES) / sizeof(DWORD)));
	}

	// Read size (0 by default)

	pElem = rRoot.FindChildConst(SZ_SIZE);

	if (pElem != NULL)
	{
		if (pElem->GetElemType() == InfoElem::TYPE_VALUE)
		{
			if (pElem->GetVarType() == Variable::TYPE_ENUM &&
			   wcscmp(pElem->GetEnumValue(), SZ_SIZE_FULLSCREEN) == 0)
			{
				m_psSize.cx =
					int(m_rEngine.GetGraphics().GetDeviceParams().BackBufferWidth);

				m_psSize.cy =
					int(m_rEngine.GetGraphics().GetDeviceParams().BackBufferHeight);
			}
		}
		else if (pElem->GetElemType() == InfoElem::TYPE_VALUELIST)
		{
			pElem->ToIntArray(reinterpret_cast<int*>(&m_psSize), 2);
		}
	}

	// Update client rectangle

	m_BufferInst.SetTextureCoords(0, 0, m_psSize.cx, m_psSize.cy);

	// Read position (top-left corner by default)

	pElem = rRoot.FindChildConst(SZ_POSITION);

	if (pElem != NULL)
	{
		if (pElem->GetElemType() == InfoElem::TYPE_VALUE)
		{
			if (pElem->GetVarType() == Variable::TYPE_ENUM &&
			   wcscmp(pElem->GetEnumValue(), SZ_POSITION_CENTER) == 0)
			{
				// Center

				m_ptPos.x = (int(m_rEngine.GetGraphics().
					GetDeviceParams().BackBufferWidth) - m_psSize.cx) / 2;

				m_ptPos.y = (int(m_rEngine.GetGraphics().
					GetDeviceParams().BackBufferHeight) - m_psSize.cy) / 2;

				m_bCenter = true;
			}
		}
		else if (pElem->GetElemType() == InfoElem::TYPE_VALUELIST)
		{
			pElem->ToIntArray(reinterpret_cast<int*>(&m_ptPos), 2);
		}
	}

	// Read background (optional)

	if (m_Background.IsEmpty() == true)
	{
		LoadMaterialInstance(m_Background, SZ_BG,
			&rRoot, m_pStyle);
	}

	if (m_Background.IsAnimated() == true &&
	   m_Background.GetAnimation() != NULL)
	{
		// Start the animation timer

		m_rEngine.GetTimers().Add(this, 0.001f, BACKGROUND_ANIM_TIMER);

		// Start playing animation if playing flag was set

		if (m_Background.IsPlaying() == true)
			m_Background.Play(m_rEngine.GetRunTime(),
				m_Background.IsLooping(), 0, m_Background.IsReverse());
	}

	// If no size specified, take size from background

	if (0 == m_psSize.cx)
		m_psSize.cx = m_Background.GetTextureCoords().GetWidth();

	if (0 == m_psSize.cy)
		m_psSize.cy = m_Background.GetTextureCoords().GetHeight();

	// Read background color (optional)

	LoadColor(m_clrBackColor, SZ_BGCOLOR, &rRoot, m_pStyle);

	// Read blend (optional)

	LoadColor(m_clrBlend, SZ_BLEND, &rRoot, m_pStyle);

	// Cache position and size

	CachePosition();

	// Read children (optional)

	InfoElemConstRange range = rRoot.FindChildrenConst(SZ_ROOT);

	for(InfoElemConstRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		// Find child's class

		pElem = pos->second->FindChildConst(SZ_CLASS);

		if (pElem != NULL && pElem->GetVarType() != Variable::TYPE_STRING)
			throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
				__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(), SZ_CLASS,
				Variable::GetVarTypeString(Variable::TYPE_STRING));

		// Create child

		Screen* pChild = NULL;
		
		if (pElem != NULL)
		{
			pChild = dynamic_cast<Screen*>(m_rEngine.GetClasses().Create(
				pElem->GetStringValue(), this));

			if (NULL == pChild)
				throw m_rEngine.GetErrors().Push(Error::CLASS_CREATE,
				__FUNCTIONW__, pElem->GetStringValue()); 
		}
		else
		{
			try
			{
				pChild = new Screen(m_rEngine, NULL, this);
			}

			catch(std::bad_alloc)
			{
				throw Error(Error::MEM_ALLOC,
					__FUNCTIONW__, sizeof(Screen));
			}
		}

		// Add it to the list of children

		pChild->ZOrder(ZORDER_ADD);

		// Load it

		pChild->Deserialize(*pos->second);

		// Fire OnMove and OnSize for the first time

		pChild->OnSize(pChild->GetSize());
		pChild->OnMove(pChild->GetPosition());
	}

	// Create back buffer if not created (if buffered)

	if (IsFlagSet(BUFFER) == true)
		CreateBuffer();
}

void Screen::RegisterEventListener(Screen* pListener)
{
	m_lstListeners.push_back(pListener);
}

void Screen::UnregisterEventListener(Screen* pListener)
{
	if (m_lstListeners.empty() == true)
		return;

	m_lstListeners.remove(pListener);
}

DWORD Screen::GetMemoryFootprint(void) const
{
	DWORD dwSize = Object::GetMemoryFootprint() -
		sizeof(Object) +
		sizeof(Screen) + 
		(DWORD)m_strClass.GetLengthBytes();

	if (IsFlagSet(BUFFER))
		dwSize += m_Buffer.GetMemoryFootprint();

	dwSize += m_lstChildren.GetMemoryFootprint();

	return dwSize;
}

void Screen::Empty(void)
{
	// Make sure engine has no references to this screen

	if (m_rEngine.GetScreens().GetFocusScreen() == this)
		m_rEngine.GetScreens().SetFocusScreen(NULL);

	if (m_rEngine.GetScreens().GetActiveScreen() == this)
		m_rEngine.GetScreens().SetActiveScreen(NULL);

	if (m_rEngine.GetScreens().GetCaptureScreen() == this)
		m_rEngine.GetScreens().SetCaptureScreen(NULL);

	if (m_rEngine.GetScreens().GetHoverScreen() == this)
		m_rEngine.GetScreens().SetHoverScreen(NULL);

	m_rEngine.GetTimers().Remove(this, BACKGROUND_ANIM_TIMER);

	// Unload all children

	m_lstChildren.RemoveAll();

	// Destroy buffer if any

	m_Buffer.Empty();
}

int Screen::Release(void)
{
	// Remove from parent's child list

	ZOrder(ZORDER_REMOVE);

	// Deallocate

	delete this;

	return 0;
}

void Screen::OnMove(const POINT& rptOldPos)
{
	// Send OnMove event to every top-level child if not buffered

	if (IsFlagSet(BUFFER) == false)
	{
		for(ScreenListIterator pos = m_lstChildren.GetBeginPos();
			pos != m_lstChildren.GetEndPos();
			pos++)
		{
			if ((*pos) != NULL)
				(*pos)->OnMove((*pos)->GetPosition());
		}
	}
}

void Screen::OnSize(const SIZE& rpsOldSize)
{
	// Send OnSize event to every top-level child

	for(ScreenListIterator pos = m_lstChildren.GetBeginPos();
		pos != m_lstChildren.GetEndPos();
		pos++)
	{
		if (*pos != NULL)
			(*pos)->OnSize((*pos)->GetSize());
	}
}

void Screen::OnActivate(Screen* pOldActive)
{
	// Activate parent, if any

	UNREFERENCED_PARAMETER(pOldActive);

	if (m_pParent != NULL)
		m_pParent->Activate();
}

void Screen::OnDeactivate(Screen* pNewActive)
{
	// Default handler

	UNREFERENCED_PARAMETER(pNewActive);
}

void Screen::OnFocus(Screen* pOldFocus)
{
	// Default handler

	UNREFERENCED_PARAMETER(pOldFocus);
}

void Screen::OnDefocus(Screen* pNewFocus)
{
	// Default handler

	UNREFERENCED_PARAMETER(pNewFocus);
}

void Screen::OnCapture(Screen* pOldCapture)
{
	// Default handler

	UNREFERENCED_PARAMETER(pOldCapture);
}

void Screen::OnDecapture(Screen* pNewCapture)
{
	// Default handler
	
	UNREFERENCED_PARAMETER(pNewCapture);
}

void Screen::OnThemeChange(Theme& rNewTheme)
{
	// Cache style pointer

	CacheStyle();

	// Notify children

	for(ScreenListIterator pos = m_lstChildren.GetBeginPos();
		pos != m_lstChildren.GetEndPos();
		pos++)
	{
		if (*pos != NULL)
			(*pos)->OnThemeChange(rNewTheme);
	}
}

void Screen::OnThemeStyleChange(void)
{
	// Load background

	LoadMaterialInstance(m_Background, SZ_BG, NULL, m_pStyle);

	// Read blend (optional)

	LoadColor(m_clrBlend, SZ_BLEND, NULL, m_pStyle);

	// Read background color (optional)

	LoadColor(m_clrBackColor, SZ_BGCOLOR, NULL, m_pStyle);
}

void Screen::OnSessionPause(bool bPause)
{
	// Notify children

	for(ScreenListIterator pos = m_lstChildren.GetBeginPos();
		pos != m_lstChildren.GetEndPos();
		pos++)
	{
		if ((*pos) != NULL)
			(*pos)->OnSessionPause(bPause);
	}
}

void Screen::OnTimer(Timer& rTimer)
{
	// Handle background animation

	if (rTimer.GetID() == BACKGROUND_ANIM_TIMER)
		m_Background.Update(m_rEngine.GetRunTime());
}

void Screen::OnCommand(int nCommandID, Screen* pSender, int nParam)
{
	// Default handler

	UNREFERENCED_PARAMETER(nCommandID);
	UNREFERENCED_PARAMETER(nParam);
	UNREFERENCED_PARAMETER(pSender);
}

int Screen::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	UNREFERENCED_PARAMETER(nNotifyID);
	UNREFERENCED_PARAMETER(nParam);
	UNREFERENCED_PARAMETER(pSender);

	return 0;
}

void Screen::OnKeyDown(int nKeyCode)
{
	// Default handler

	UNREFERENCED_PARAMETER(nKeyCode);
}

void Screen::OnKeyUp(int nKeyCode)
{
	// Default handler

	UNREFERENCED_PARAMETER(nKeyCode);
}

void Screen::OnKeyPress(int nAsciiCode, bool extended, bool alt)
{
	// Default handler

	UNREFERENCED_PARAMETER(nAsciiCode);
	UNREFERENCED_PARAMETER(extended);
	UNREFERENCED_PARAMETER(alt);
}

void Screen::OnMouseMove(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);
}

void Screen::OnMouseLDown(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);

	Activate();
}

void Screen::OnMouseLUp(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);
}

void Screen::OnMouseLDbl(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);
}

void Screen::OnMouseRDown(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);
}

void Screen::OnMouseRUp(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);
}

void Screen::OnMouseRDbl(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);
}

void Screen::OnMouseMDown(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);
}

void Screen::OnMouseMUp(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);
}

void Screen::OnMouseMDbl(POINT pt)
{
	// Default handler

	UNREFERENCED_PARAMETER(pt);
}

void Screen::OnMouseWheel(int nZDelta)
{
	// Default handler

	UNREFERENCED_PARAMETER(nZDelta);
}

void Screen::OnMouseEnter(void)
{
	// Default handler
}

void Screen::OnMouseLeave(void)
{
	// Default handler
}