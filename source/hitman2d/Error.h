/*------------------------------------------------------------------*\
|
| Error.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D error class
| Created: 06/30/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef ERROR_H
#define ERROR_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| Error class
\*----------------------------------------------------------*/

class ErrorGame: public Error
{
public:
	//
	// Constants
	//

	enum
	{
		ENGINEVERSION = Error::USER,
		CONFIGUREEXIT,
		CUSTOM
	};

public:
	ErrorGame(int nCode, LPCWSTR pszFunctionName, ...);

public:
	inline operator Error&(void)
	{
		return *static_cast<Error*>(this);
	}
};

} // namespace Hitman2D

#endif // ERROR_H