/*------------------------------------------------------------------*\
|
| ThunderVariable.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm Variable classes
| Created: 03/06/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_VARIABLE_H
#define THUNDER_VARIABLE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderString.h"	// using String

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Variable;				// referencing Variable, declared below
class InfoElem;				// referencing InfoElem
class ErrorManager;			// referencing ErrorManager

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<String, Variable*> VariableMap;
typedef std::map<String, Variable*>::iterator VariableMapIterator;
typedef std::map<String, Variable*>::const_iterator VariableMapConstIterator;


/*----------------------------------------------------------*\
| Variable class
\*----------------------------------------------------------*/

class Variable
{
public:
	//
	// Constants
	//

	enum Types
	{
		// Undefined
		TYPE_UNDEFINED,

		// Signed 32 bit
		TYPE_INT,

		// Unsigned 32 bit
		TYPE_DWORD,

		// Signed 32 bit, only 1 or 0 (or true/false)
		TYPE_BOOL,

		// Signed 32 bit float
		TYPE_FLOAT,

		// Unsigned 32 bit pointer to unicode chars on heap
		TYPE_STRING,

		// Same as string, but stands for something
		// (typically used as a symbolic name for a number, hence name 'enum')
		TYPE_ENUM,

		// Number of var types defined
		TYPE_COUNT
	};

	static const LPCWSTR SZ_TYPES[];

protected:
	//
	// Members
	//

	// Type
	Types m_nType;

	// Value
	union
	{
		int m_nValue;
		DWORD m_dwValue;
		float m_fValue;
		LPWSTR m_pszValue;
		bool m_bValue;
	};

public:
	Variable(Types nVarType = TYPE_UNDEFINED);
	Variable(const Variable& rInit);

	Variable(int nValue);
	Variable(DWORD dwValue);
	Variable(float fValue);
	Variable(LPCWSTR pszValue);
	Variable(bool bValue);

	~Variable(void);

public:
	//
	// Type
	//

	inline Types GetVarType(void) const
	{
		return m_nType;
	}

	void SetVarType(Types nNewType);

	LPCWSTR GetVarTypeString(void) const;

	static LPCWSTR GetVarTypeString(Types nType);

	//
	// Value
	//

	inline int GetIntValue(void) const
	{
		return m_nValue;
	}

	inline bool GetBoolValue(void) const
	{
		return m_bValue;
	}

	inline DWORD GetDwordValue(void) const
	{
		return m_dwValue;
	}

	inline float GetFloatValue(void) const
	{
		return m_fValue;
	}	

	inline LPCWSTR GetStringValue(void) const
	{
		return m_pszValue;
	}

	inline String GetString(void) const
	{
		return String(m_pszValue);
	}

	int GetEnumValue(const LPCWSTR* ppszEnumValueNames, int nEnumCount, int nDefault = -1) const;
	int GetEnumValue(const LPCWSTR* ppszEnumValueNames, const int* pnEnumValues, int nEnumCount, int nDefault = -1) const;
	DWORD GetEnumValue(const LPCWSTR* ppszEnumValueNames, const DWORD* pdwEnumValues, int nEnumCount, DWORD dwDefault = 0) const;
	LPCWSTR GetEnumValue(void) const;

	void SetIntValue(int nValue);
	void SetDwordValue(DWORD dwValue);
	void SetFlagsValue(DWORD dwFlags);
	void SetFloatValue(float fValue);
	void SetStringValue(LPCWSTR pszValue);
	void SetBoolValue(bool bValue);
	void SetEnumValue(int nEnumValueIndex, const LPCWSTR* ppszEnumValueNames, int nEnumCount, LPCWSTR pszDefault = NULL);
	void SetEnumValue(int nEnumValue, const LPCWSTR* ppszEnumValueNames, const int* pnEnumValues, int nEnumCount, LPCWSTR pszDefault = NULL);
	void SetEnumValue(DWORD dwEnumValue, const LPCWSTR* ppszEnumValueNames, const DWORD* pdwEnumValues, int nEnumCount, LPCWSTR pszDefault = NULL);
	void SetEnumValue(LPCWSTR pszEnumValueName);

	//
	// Transition
	//

	// Returns number of characters read
	int FromString(LPCWSTR psz);

	String ToString(void) const;

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	void Serialize(InfoElem& rRoot) const;
	void Deserialize(const InfoElem& rRoot);

	//
	// Operators
	//

	Variable& operator=(const Variable& rCopy);
	Variable& operator=(int nValue);
	Variable& operator=(DWORD dwValue);
	Variable& operator=(float fValue);
	Variable& operator=(LPCWSTR pszValue);

	bool operator==(const Variable& rCompare) const;
	bool operator!=(const Variable& rCompare) const;

	//
	// Diagnostics
	//

	DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| VariableManager class
\*----------------------------------------------------------*/

class VariableManager
{
protected:
	//
	// Members
	//

	// Optional error context for outputting errors
	ErrorManager* m_pErrorContext;

	// Variable names to variable pointers
	VariableMap m_mapVars;

public:
	VariableManager(ErrorManager* pErrorContext = NULL);
	~VariableManager(void);

public:
	//
	// Variables
	//

	Variable* Add(LPCWSTR pszName, Variable::Types nType = Variable::TYPE_UNDEFINED);

	Variable* Find(LPCWSTR pszName);
	const Variable* FindConst(LPCWSTR pszName) const;

	int GetCount(void) const;

	void Remove(LPCWSTR pszName);
	void RemoveAll(void);
	
	VariableMapIterator GetBeginPos(void);
	VariableMapIterator GetEndPos(void);

	VariableMapConstIterator GetBeginPosConst(void) const;
	VariableMapConstIterator GetEndPosConst(void) const;

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	//
	// Diagnostics
	//

	DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	void Empty(void);
};

} // namespace ThunderStorm

#endif