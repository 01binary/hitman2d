/*------------------------------------------------------------------*\
|
| ThunderControls.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine control binding class
| Created: 11/07/2010
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_CONTROLS_H
#define THUNDER_CONTROLS_H

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<int, int> ControlBindMap;
typedef std::map<int, int>::iterator ControlBindMapIterator;
typedef std::map<int, int>::const_iterator ControlBindMapConstIterator;

typedef std::vector<int> IntArray;
typedef std::vector<int>::iterator IntArrayIterator;
typedef std::vector<int>::const_iterator IntArrayConstIterator;


/*----------------------------------------------------------*\
| ControlManager class
\*----------------------------------------------------------*/

class ControlManager
{
public:
	//
	// Constants
	//

	enum
	{
		KEY_UNASSIGNED = 0,
		VK_NUMRETURN = 0x0E
	};

private:
	//
	// Members
	//

	// Binds key to control ID
	ControlBindMap m_mapBinds;

	// Reverse binding lookup
	IntArray m_arRevBinds;

	// Control names (indexed by control ID)
	StringArray m_arNames;

public:
	//
	// Binding
	//

	inline void Bind(int nControlID, int nKey)
	{
		m_mapBinds[nKey] = nControlID;
		m_arRevBinds[nControlID] = nKey;
	}

	inline bool IsBound(int nKey)
	{
		return (m_mapBinds.find(nKey) != m_mapBinds.end());
	}

	inline int GetBoundControl(int nKey) const
	{
		ControlBindMapConstIterator pos = m_mapBinds.find(nKey);

		if (m_mapBinds.end() != pos)
			return pos->second;

		return INVALID_INDEX;
	}

	inline void Unbind(int nKey)
	{
		ControlBindMapIterator pos = m_mapBinds.find(nKey);

		if (m_mapBinds.end() != pos)
		{
			m_arRevBinds[pos->second] = KEY_UNASSIGNED;
			m_mapBinds.erase(pos);
		}
	}

	//
	// Controls
	//

	inline int RegisterControl(LPCWSTR pszName)
	{
		m_arNames.push_back(pszName);
		m_arRevBinds.push_back(KEY_UNASSIGNED);
		
		return int(m_arNames.size());
	}

	inline const String& GetControlName(int nControlID)
	{
		return m_arNames[nControlID];
	}

	inline int GetControlCount(void) const
	{
		return int(m_arNames.size());
	}

	inline bool IsBoundControl(int nControlID) const
	{
		if (nControlID < 0 || nControlID >= int(m_arRevBinds.size()))
			return false;

		return (m_arRevBinds[nControlID] != KEY_UNASSIGNED);
	}

	inline int GetControlBoundKey(int nControlID) const
	{
		if (nControlID < 0 || nControlID >= int(m_arRevBinds.size()))
			return KEY_UNASSIGNED;

		return m_arRevBinds[nControlID];
	}

	//
	// Processing
	//

	inline int Translate(int nKey)
	{
		ControlBindMapConstIterator pos = m_mapBinds.find(nKey);

		return (m_mapBinds.end() == pos ? INVALID_INDEX : pos->second);
	}

	//
	// Key State
	//

	inline static bool IsKeyPressed(int nKeyCode)
	{
		return ((::GetKeyState(nKeyCode) & 0x8000) != 0);
	}

	inline static bool IsKeyToggled(int nKeyCode)
	{
		return ((::GetKeyState(nKeyCode) & 0x0001) != 0);
	}

	//
	// Key Info
	//

	static LPCWSTR GetKeyDescription(int nKey);
	static int GetKeyCode(LPCWSTR pszDescription);

	//
	// Serialization
	//

	void Serialize(LPCWSTR pszPath);
	void Deserialize(LPCWSTR pszPath);

	//
	// Deinitialization
	//

	inline bool IsEmpty(void)
	{
		return m_mapBinds.empty();
	}

	void Empty(void);
};

} // namespace ThunderStorm

#endif // THUNDER_CONTROLS_H