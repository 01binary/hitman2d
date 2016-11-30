/*------------------------------------------------------------------*\
|
| ScreenButtonEx.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ButtonEx Control implementation
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "Globals.h"			// using global constants
#include "Game.h"				// using Game
#include "Error.h"				// using error constants
#include "ScreenButtonEx.h"		// defining ScreenButtonEx

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenButtonEx::SZ_CAPTION[] =					L"caption";
const WCHAR ScreenButtonEx::SZ_DESCRIPTION[] =				L"description";
const WCHAR ScreenButtonEx::SZ_CAPTIONFONT[] =				L"caption.font";
const WCHAR ScreenButtonEx::SZ_DESCRIPTIONFONT[] =			L"description.font";
const WCHAR ScreenButtonEx::SZ_CAPTIONCOLOR[] =				L"caption.color";
const WCHAR ScreenButtonEx::SZ_DESCRIPTIONCOLOR[] =			L"description.color";
const WCHAR ScreenButtonEx::SZ_DESCRIPTIONCOLORPUSHED[] =	L"description.pushed.color";
const WCHAR ScreenButtonEx::SZ_ICON[] =						L"icon.material";
const WCHAR ScreenButtonEx::SZ_ICON_ALIGN[] =				L"icon.align";
const WCHAR ScreenButtonEx::SZ_LEFTCORNER[] =				L"left.material";
const WCHAR ScreenButtonEx::SZ_RIGHTCORNER[] =				L"right.material";
const WCHAR ScreenButtonEx::SZ_CENTERREPEAT[] =				L"center.material";
const WCHAR ScreenButtonEx::SZ_DISABLESTRETCHING[] =		L"center.disablestretching";
const WCHAR ScreenButtonEx::SZ_CLASS[] =					L"buttonex";

const LPCWSTR ScreenButtonEx::SZ_ICON_ALIGN_TYPES[] =	{
															L"left",
															L"right"
														};


/*----------------------------------------------------------*\
| ScreenButtonEx implementation
\*----------------------------------------------------------*/

ScreenButtonEx::ScreenButtonEx(Engine& rEngine,
							   LPCWSTR pszClass,
							   Screen* pParent):

							   ScreenButton(rEngine, pszClass, pParent),
							   m_pFontDescription(NULL),
							   m_clrDescription(Color::BLEND_ZERO),
							   m_clrDescriptionPushed(Color::BLEND_ZERO),
							   m_nMargin(10),
							   m_nIconAlign(ScreenButtonEx::ICON_ALIGN_LEFT)
{
	SetRectEmpty(&m_rcCaption);
	SetRectEmpty(&m_rcDescription);
}

ScreenButtonEx::~ScreenButtonEx(void)
{
	Empty();
}

Object* ScreenButtonEx::CreateInstance(Engine& rEngine,
									   LPCWSTR pszClass,
									   Object* pParent)
{
	return new ScreenButtonEx(rEngine, pszClass,
		dynamic_cast<Screen*>(pParent));
}

void ScreenButtonEx::SetDescriptionFont(Font* pDescriptionFont)
{
	if (pDescriptionFont != NULL) pDescriptionFont->AddRef();
	if (m_pFontDescription != NULL) m_pFontDescription->Release();

	m_pFontDescription = pDescriptionFont;

	UpdateLayout();
}

void ScreenButtonEx::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Calculate background color

	float fBlend = GetFrontBufferBlend().GetAFloat();

	Color clrBack(m_clrBackColor);

	if (IsFlagSet(DISABLED) == true)
		clrBack.SetAlpha(clrBack.GetAFloat() * fBlend * 0.5f);
	else
		clrBack.SetAlpha(clrBack.GetAFloat() * fBlend);

	if (false == m_bTextOnly)
	{
		// Render left corner

		if (m_Left.IsEmpty() == false)
		{
			rGraphics.RenderQuad(m_Left, m_vecCachedPos, clrBack);
		}

		// Render center repeat

		if (m_CenterRepeat.IsEmpty() == false)
		{
			int nSpaceToFill = m_psSize.cx -
					m_Left.GetTextureCoords().GetWidth() -
					m_Right.GetTextureCoords().GetWidth();

			if (m_bDisableStretching)
			{
				// If default mode of repetition (stretching) is disabled, repeat by re-rendering the background image

				int nBgWidth = m_CenterRepeat.GetTextureCoords().GetWidth();
				int nBgRepeat = nSpaceToFill / nBgWidth;
				int nBgRemainder = nSpaceToFill - nBgRepeat * nBgWidth;

				Vector2 vecDrawPos = m_vecCenterPos;
				float fDrawIncrement = float(nBgWidth);

				for (int n = 0; n < nBgRepeat; n++, vecDrawPos.x += fDrawIncrement)
				{
					rGraphics.RenderQuad(m_CenterRepeat, vecDrawPos, clrBack);
				}

				if (nBgRemainder > 0)
				{
					// If after tiling background there is space left over, fill remainder with a piece of background just wide enough

					Rect rcCoords = m_CenterRepeat.GetTextureCoords();
					rcCoords.right = rcCoords.left + nBgRemainder;
					m_CenterRepeat.SetTextureCoords(rcCoords);

					rGraphics.RenderQuad(m_CenterRepeat, vecDrawPos, clrBack);

					rcCoords.right = rcCoords.left + nBgWidth;
					m_CenterRepeat.SetTextureCoords(rcCoords);
				}
			}
			else
			{
				rGraphics.RenderQuad(m_CenterRepeat, m_vecCenterPos,
					Vector2(nSpaceToFill, m_CenterRepeat.GetTextureCoords().GetHeight()),
					clrBack);
			}
		}

		// Render right corner

		if (m_Right.IsEmpty() == false)
		{
			rGraphics.RenderQuad(m_Right, m_vecRightPos,
				clrBack);
		}
	}

	// Render icon (optional)

	if (m_Icon.IsEmpty() == false)
	{
		Vector2 vecIconPos = m_vecIconPos;

		if (ScreenButton::STATE_PUSHED == m_nState ||
		   ScreenButton::STATE_PUSHEDTOGGLED == m_nState)
		{
			vecIconPos.x += float(m_ptPushedTextOffset.x);
			vecIconPos.y += float(m_ptPushedTextOffset.y);
		}

		rGraphics.RenderQuad(m_Icon, vecIconPos,
			GetFrontBufferBlend());
	}
	
	Color clrText;
	Rect rcText;

	// Render text (optional) - centered if no description

	if (m_strText.IsEmpty() == false && m_pFont != NULL)
	{
		rcText = m_rcCaption;

		if (ScreenButton::STATE_PUSHED == m_nState ||
		   ScreenButton::STATE_PUSHEDTOGGLED == m_nState)
		{
			clrText = m_clrTextPushed;

			rcText.left += m_ptPushedTextOffset.x;
			rcText.top += m_ptPushedTextOffset.y;
		}
		else if (ScreenButton::STATE_HOVER == m_nState)
		{
			clrText = m_clrTextHover;
		}
		else if (ScreenButton::STATE_NORMALTOGGLED == m_nState)
		{
			clrText = m_clrTextToggled;
		}
		else if (ScreenButton::STATE_HOVERTOGGLED == m_nState)
		{
			clrText = m_clrTextHoverToggled;
		}
		
		// If disabled, less alpha
		if (IsFlagSet(DISABLED) == true)
			clrText.SetAlpha(clrText.GetAFloat() * clrBack.GetAFloat());
		else
			clrText.SetAlpha(clrText.GetAFloat() * fBlend);

		DWORD dwFontFlags = Font::USE_MNEMONICS | Font::ALIGN_VCENTER;
	
		if (false == m_bTextAlignLeft && m_strDescription.IsEmpty() == true)
			dwFontFlags |= Font::ALIGN_CENTER;

		m_pFont->RenderText(rcText, m_strText,
			-1, clrText, dwFontFlags);
	}

	// Render description (optional)

	if (m_strDescription.IsEmpty() == false &&
	   m_pFontDescription != NULL)
	{
		rcText = m_rcDescription;

		if (ScreenButton::STATE_PUSHED == m_nState ||
		   ScreenButton::STATE_PUSHEDTOGGLED == m_nState)
		{
			clrText = m_clrDescriptionPushed;

			rcText.left += m_ptPushedTextOffset.x;
			rcText.top += m_ptPushedTextOffset.y;
		}
		else
		{
			if (ScreenButton::STATE_NORMALTOGGLED == m_nState ||
			   ScreenButton::STATE_HOVERTOGGLED == m_nState)
				clrText = m_clrDescriptionPushed;
			else
				clrText = m_clrDescription;
		}

		if (IsFlagSet(DISABLED) == true)
			clrText.SetAlpha(clrText.GetAFloat() * clrBack.GetAFloat());
		else
			clrText.SetAlpha(clrText.GetAFloat() * fBlend);

		m_pFontDescription->RenderText(rcText, m_strDescription,
			-1, clrText);
	}
}

void ScreenButtonEx::Deserialize(const InfoElem& rRoot)
{
	ScreenButton::Deserialize(rRoot);

	// Make sure font was read

	if (NULL == m_pFont)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(),
			SZ_LEFTCORNER);

	// Read description

	const InfoElem* pElem = rRoot.FindChildConst(SZ_DESCRIPTION,
		InfoElem::TYPE_VALUE,
		Variable::TYPE_STRING);

	if (pElem != NULL)
		m_strDescription = pElem->GetStringValue();

	// Read description font

	LoadFont(&m_pFontDescription, SZ_DESCRIPTIONFONT,
		&rRoot, m_pStyle,
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE));

	// Read description color

	LoadColor(m_clrDescription, SZ_DESCRIPTIONCOLOR,
		&rRoot, m_pStyle);

	// Read description pushed color

	if (LoadColor(m_clrDescriptionPushed, SZ_DESCRIPTIONCOLORPUSHED,
		&rRoot, m_pStyle) == false)
			m_clrDescriptionPushed = m_clrDescription;

	// Read margin

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_SCREEN_MARGIN,
		Variable::TYPE_INT, Variable::TYPE_DWORD, &rRoot, m_pStyle) == true)
			m_nMargin = pVar->GetIntValue();

	// Read icon align

	if (LoadVariable(&pVar, SZ_ICON_ALIGN,
		Variable::TYPE_ENUM, Variable::TYPE_INT, &rRoot, m_pStyle) == true)
	{
		if (pVar->GetVarType() == Variable::TYPE_ENUM)
			m_nIconAlign = IconAlign(pVar->GetEnumValue(SZ_ICON_ALIGN_TYPES,
				ICON_ALIGN_COUNT, ICON_ALIGN_LEFT));
		else
			m_nIconAlign = IconAlign(pVar->GetIntValue());
	}

	// Read disable stretching

	if (LoadVariable(&pVar, SZ_DISABLESTRETCHING,
	   Variable::TYPE_BOOL, Variable::TYPE_INT, &rRoot, m_pStyle) == true)
		m_bDisableStretching = pVar->GetBoolValue();

	// Read icon texture

	LoadMaterialInstance(m_Icon, SZ_ICON,
		&rRoot, m_pStyle);

	// Read left corner texture

	if (LoadMaterialInstance(m_Left, SZ_LEFTCORNER,
		&rRoot, m_pStyle) == false)	
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(),
			SZ_LEFTCORNER);
	}

	// Read right corner texture

	if (LoadMaterialInstance(m_Right, SZ_RIGHTCORNER,
		&rRoot, m_pStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(),
			SZ_RIGHTCORNER);
	}

	// Read center repeat texture

	if (LoadMaterialInstance(m_CenterRepeat, SZ_CENTERREPEAT,
		&rRoot, m_pStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(),
			SZ_CENTERREPEAT);
	}
}

void ScreenButtonEx::OnThemeStyleChange(void)
{
	ScreenButton::OnThemeStyleChange();

	// Read description font

	LoadFont(&m_pFontDescription, SZ_DESCRIPTIONFONT,
		NULL, m_pStyle,
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE));

	// Read description color

	LoadColor(m_clrDescription, SZ_DESCRIPTIONCOLOR,
		NULL, m_pStyle);

	// Read description pushed color

	if (LoadColor(m_clrDescriptionPushed, SZ_DESCRIPTIONCOLORPUSHED,
		NULL, m_pStyle) == false)
			m_clrDescriptionPushed = m_clrDescription;

	// Read margin

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_SCREEN_MARGIN,
		Variable::TYPE_INT, Variable::TYPE_DWORD, NULL, m_pStyle) == true)
			m_nMargin = pVar->GetIntValue();

	// Read icon align

	if (LoadVariable(&pVar, SZ_ICON_ALIGN,
		Variable::TYPE_ENUM, Variable::TYPE_INT, NULL, m_pStyle) == true)
	{
		if (pVar->GetVarType() == Variable::TYPE_ENUM)
			m_nIconAlign = IconAlign(pVar->GetEnumValue(SZ_ICON_ALIGN_TYPES,
				ICON_ALIGN_COUNT, ICON_ALIGN_LEFT));
		else
			m_nIconAlign = IconAlign(pVar->GetIntValue());
	}

	// Read disable stretching

	if (LoadVariable(&pVar, SZ_DISABLESTRETCHING,
	   Variable::TYPE_BOOL, Variable::TYPE_INT, NULL, m_pStyle) == true)
		m_bDisableStretching = pVar->GetBoolValue();

	// Read icon texture

	LoadMaterialInstance(m_Icon, SZ_ICON,
		NULL, m_pStyle);

	// Read left corner texture

	LoadMaterialInstance(m_Left, SZ_LEFTCORNER,
		NULL, m_pStyle);

	// Read right corner texture

	LoadMaterialInstance(m_Right, SZ_RIGHTCORNER,
		NULL, m_pStyle);

	// Read center repeat texture

	LoadMaterialInstance(m_CenterRepeat, SZ_CENTERREPEAT,
		NULL, m_pStyle);

	// If height not equal to left corner height, set

	if (m_psSize.cy != m_Left.GetTextureCoords().GetHeight())
		m_psSize.cy = m_Left.GetTextureCoords().GetHeight();
}

DWORD ScreenButtonEx::GetMemoryFootprint(void) const
{
	return sizeof(ScreenButtonEx) -
		   sizeof(ScreenButton) * 2 +
		   ScreenButton::GetMemoryFootprint() +
		   m_strDescription.GetLengthBytes();
}

void ScreenButtonEx::OnMove(const POINT& rptOldPos)
{
	Screen::OnMove(rptOldPos);

	UpdateLayout();
}

void ScreenButtonEx::OnSize(const SIZE& rpsOldSize)
{
	Screen::OnSize(rpsOldSize);

	UpdateLayout();
}

void ScreenButtonEx::UpdateLayout(void)
{
	// Cache center texture position

	m_vecCenterPos.x = m_vecCachedPos.x + 
		float(m_Left.GetTextureCoords().GetWidth());

	m_vecCenterPos.y = m_vecCachedPos.y;

	// Cache right texture position

	m_vecRightPos.x = m_vecCachedPos.x +
		float(m_psSize.cx - m_Right.GetTextureCoords().GetWidth());

	m_vecRightPos.y = m_vecCachedPos.y;

	// Cache icon position

	if (m_strText.IsEmpty() && m_strDescription.IsEmpty())
	{
		m_vecIconPos.x = m_vecCachedPos.x + floor(
			float(m_psSize.cx -
			m_Icon.GetTextureCoords().GetWidth()) / 2.0f);
	}
	else
	{
		if (ICON_ALIGN_LEFT == m_nIconAlign)
		{
			m_vecIconPos.x = m_vecCachedPos.x + float(m_nMargin);
		}
		else
		{		
			m_vecIconPos.x = m_vecCachedPos.x +
				float(m_psSize.cx - m_nMargin - m_Icon.GetTextureCoords().GetWidth());
		}
	}

	m_vecIconPos.y = m_vecCachedPos.y + floor(
		float(m_psSize.cy -
		m_Icon.GetTextureCoords().GetHeight()) / 2.0f);

	// Cache caption position

	if (m_Icon.GetTextureCoords().GetWidth() > 0)
	{
		if (ICON_ALIGN_LEFT == m_nIconAlign)
			m_rcCaption.left = m_rcCachedRect.left +
				m_Icon.GetTextureCoords().GetWidth() + m_nMargin * 2;
		else
			m_rcCaption.left = m_rcCachedRect.left + m_nMargin;
	}
	else
	{
		m_rcCaption.left = m_rcCachedRect.left + m_nMargin;
	}

	m_rcCaption.top = m_rcCachedRect.top + m_nMargin;
	m_rcCaption.right = m_rcCachedRect.left + m_psSize.cx - m_nMargin;

	if (m_strDescription.IsEmpty() == true)
		m_rcCaption.bottom = m_rcCachedRect.top + m_psSize.cy - m_nMargin;
	else
		m_rcCaption.bottom = m_rcCaption.top +
			m_pFont->GetLineSpacing();

	// Cache description position

	m_rcDescription.left = m_rcCaption.left;
	m_rcDescription.top = m_rcCaption.bottom + m_nMargin / 2;
	m_rcDescription.right = m_rcCaption.right;
	m_rcDescription.bottom = m_rcCachedRect.top + m_psSize.cy - m_nMargin;
}