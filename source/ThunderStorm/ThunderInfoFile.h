/*------------------------------------------------------------------*\
|
| ThunderInfoFile.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm Markup Language format reader/writer class
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_INFO_FILE_H
#define THUNDER_INFO_FILE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderVariable.h"	// using Variable
#include "ThunderError.h"		// using ErrorManager

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class InfoElem;		// referencing InfoElem, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<InfoElem*>::const_iterator InfoElemConstIterator;
typedef std::vector<InfoElem*>::iterator InfoElemIterator;
typedef std::multimap<String, InfoElem*>::iterator InfoElemRangeIterator;
typedef std::pair<InfoElemRangeIterator, InfoElemRangeIterator> InfoElemRange;
typedef std::multimap<String, InfoElem*>::const_iterator InfoElemConstRangeIterator;
typedef std::pair<InfoElemConstRangeIterator, InfoElemConstRangeIterator> InfoElemConstRange;


/*----------------------------------------------------------*\
| InfoFile class - ThunderStorm Markup Language File
\*----------------------------------------------------------*/

class InfoFile
{
private:
	// InfoElem only accesses m_pErrorContext and m_pszTempData
	friend class InfoElem;			

	// Path originally loaded from or last saved to
	String m_strPath;

	// Title based on path
	String m_strTitle;

	// One-and-only root element
	InfoElem* m_pRoot;

	// Optional error context for pushing errors
	ErrorManager* m_pErrorContext;

	// Temporary data pointer, valid only while data is being parsed
	LPCWSTR m_pszTempData;

public:
	InfoFile(ErrorManager* pErrorContext = NULL);
	~InfoFile(void);

public:
	//
	// Error context
	//

	inline ErrorManager* GetErrorContext(void) const
	{
		return m_pErrorContext;
	}

	//
	// Path
	//

	inline const String& GetPath(void) const
	{
		return m_strPath;
	}

	//
	// Title
	//

	inline const String& GetTitle(void) const
	{
		return m_strTitle;
	}

	//
	// Root
	//

	inline InfoElem* GetRoot(void) const
	{
		return m_pRoot;
	}

	InfoElem* CreateSetRoot(LPCWSTR pszName, bool bHasValue = false);

	//
	// Serialization
	//

	void Serialize(LPCWSTR pszPath) const;
	void Serialize(Stream& rStream) const;

	void Deserialize(LPCWSTR pszPath);
	void Deserialize(Stream& rStream);

	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| InfoElem class
\*----------------------------------------------------------*/

class InfoElem: public Variable
{
public:
	//
	// Constants
	//

	// InfoElem types

	enum InfoElemTypes					
	{
		// Value. Example: "testvalue: 32"
		TYPE_VALUE,

		// Value list. Example: "testvalue: 32, 3, 4, 5, 'hello'"
		TYPE_VALUELIST,	

		// Value block. Example: "testvalue: testing { }"
		TYPE_VALUEBLOCK,

		// Block. Example: "testblock {}"
		TYPE_BLOCK,

		// Not yet determined
		TYPE_ANY = -1
	};

private:
	// Keep reference to the owner file
	InfoFile& m_rDocument;
	
	// Keep pointer to parent
	InfoElem* m_pParent;

	// Element type
	InfoElemTypes m_nElemType;

	// Element name
	String m_strName;

	// Child elements
	std::vector<InfoElem*> m_arChildren;

	// Child elements mapped by name for faster lookup
	std::multimap<String, InfoElem*> m_mapChildren;

public:
	InfoElem(InfoFile& rDocument, InfoElem* pParent = NULL, LPCWSTR pszName = NULL, InfoElemTypes nElemType = InfoElem::TYPE_VALUE, Variable::Types nVarType = Variable::TYPE_UNDEFINED);
	InfoElem(const InfoElem& rInit);

	inline ~InfoElem(void)
	{
		Empty();
	}

public:
	//
	// Info File
	//

	inline InfoFile& GetDocument(void)
	{
		return m_rDocument;
	}

	inline const InfoFile& GetDocumentConst(void) const
	{
		return m_rDocument;
	}

	//
	// Parent
	//

	inline InfoElem* GetParent(void)
	{
		return m_pParent;
	}

	inline const InfoElem* GetParentConst(void) const
	{
		return m_pParent;
	}

	//
	// Name
	//

	inline const String& GetName(void) const
	{
		return m_strName;
	}

	inline void SetName(LPCWSTR pszName)
	{
		m_strName = pszName;
	}

	//
	// Type
	//

	inline InfoElemTypes GetElemType(void) const
	{
		return m_nElemType;
	}

	void SetElemType(InfoElemTypes nElemType);

	//
	// Value
	//

	Variable& GetValue(void)
	{
		return *static_cast<Variable*>(this);
	}

	const Variable& GetValueConst(void) const
	{
		return *static_cast<const Variable*>(this);
	}

	//
	// Children
	//

	InfoElem* CreateChild(LPCWSTR pszName = NULL, InfoElemTypes nElemType = InfoElem::TYPE_VALUE, Variable::Types nVarType = Variable::TYPE_UNDEFINED);
	void AddChild(InfoElem* pChild);
	InfoElem* AddChild(LPCWSTR pszName = NULL, InfoElemTypes nElemType = InfoElem::TYPE_VALUE, Variable::Types nVarType = Variable::TYPE_UNDEFINED);

	void RemoveChild(InfoElem* pChild, bool bDeallocate = true);
	void RemoveChild(InfoElemIterator posChild, bool bDeallocate = true);
	void RemoveAllChildren(void);

	inline int GetChildCount(void) const
	{
		return int(m_arChildren.size());
	}

	inline InfoElem* GetChild(int nIndex)
	{
		return m_arChildren[nIndex];
	}

	inline const InfoElem* GetChildConst(int nIndex) const
	{
		return m_arChildren[nIndex];
	}

	InfoElem* FindChild(LPCWSTR pszName, InfoElemTypes nElemType = InfoElem::TYPE_ANY,
		Variable::Types nVarType = Variable::TYPE_UNDEFINED, bool bRecursive = false);

	const InfoElem* FindChildConst(LPCWSTR pszName, InfoElemTypes nElemType = InfoElem::TYPE_ANY,
		Variable::Types nVarType = Variable::TYPE_UNDEFINED, bool bRecursive = false) const;

	InfoElemRange FindChildren(LPCWSTR pszName, bool bRecursive = false);
	InfoElemConstRange FindChildrenConst(LPCWSTR pszName, bool bRecursive = false) const;

	inline InfoElemIterator GetBeginChildPos(void)
	{
		return m_arChildren.begin();
	}

	inline InfoElemIterator GetEndChildPos(void)
	{
		return m_arChildren.end();
	}

	inline InfoElemConstIterator GetBeginChildPosConst(void) const
	{
		return m_arChildren.begin();
	}

	inline InfoElemConstIterator GetEndChildPosConst(void) const
	{
		return m_arChildren.end();
	}

	//
	// Serialization
	//

	void Deserialize(LPCWSTR pszData, int* pnLengthRead = NULL);
	void Serialize(Stream& rStream, int nLevel = 0) const;

	//
	// Conversion
	//

	int ToIntArray(int* pnArray, int nElements, int nStart = 0) const;
	int ToFloatArray(float* pfArray, int nElements, int nStart = 0) const;
	int ToDwordArray(DWORD* pdwArray, int nElements, int nStart = 0) const;
	DWORD ToFlags(const LPCWSTR* ppszFlagNames, const DWORD* pdwFlagValues, int nFlagCount, DWORD dwDefault = 0, int nStart = 0) const;

	void FromIntArray(const int* pnArray, int nElements);
	void FromFloatArray(const float* pfArray, int nElements);
	void FromDwordArray(const DWORD* pdwArray, int nElements);
	void FromFlags(DWORD dwFlags, const LPCWSTR* ppszFlagNames, const DWORD* pdwFlagValues, int nFlagCount);

	inline void FromVariable(const Variable& rVariable)
	{
		*static_cast<Variable*>(this) = rVariable;
	}

	//
	// Deinitialization
	//

	void Empty(void);
	void Release(void);

	//
	// Operators
	//

	InfoElem& operator=(const InfoElem& rAssign);

	bool operator==(const InfoElem& rCompare) const;
	
	inline bool operator!=(const InfoElem& rCompare) const
	{
		return !operator==(rCompare);
	}
};

} // namespace ThunderStorm

#endif