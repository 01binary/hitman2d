/*------------------------------------------------------------------*\
|
| ThunderMath.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm math class(es) and function(s)
| Created: 04/08/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_MATH_H
#define THUNDER_MATH_H

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Stream;					// referencing Stream
class InfoElem;					// referencing InfoElem
class Vector2;					// referencing Vector2, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<Vector2> Vector2Array;
typedef std::vector<Vector2>::iterator Vector2ArrayIterator;
typedef std::vector<Vector2>::const_iterator Vector2ArrayConstIterator;


/*----------------------------------------------------------*\
| Vector2 class
\*----------------------------------------------------------*/

class Vector2: public D3DXVECTOR2
{
public:
	Vector2(float fx = 0.0f, float fy = 0.0f): D3DXVECTOR2(fx, fy)
	{
	}

	Vector2(int x, int y): D3DXVECTOR2(float(x), float(y))
	{
	}

	Vector2(const Vector2& rInit)
	{
		x = rInit.x;
		y = rInit.y;
	}

	Vector2(const D3DXVECTOR2& rInit)
	{
		x = rInit.x;
		y = rInit.y;
	}
	
	Vector2(const POINT& rInit)
	{
		x = float(rInit.x);
		y = float(rInit.y);
	}

	Vector2(const SIZE& rInit)
	{
		x = float(rInit.cx);
		y = float(rInit.cy);
	}

public:
	//
	// Operations
	//

	inline float Length(void) const
	{
		return D3DXVec2Length(this);
	}

	inline float LengthSq(void) const
	{
		return D3DXVec2LengthSq(this);
	}

	inline float Dot(const Vector2& rvec)
	{
		return D3DXVec2Dot(this, &rvec);
	}

	inline Vector2 Normal(void) const
	{
		float fLen = D3DXVec2Length(this);

		return Vector2(x / fLen, y / fLen);
	}

	inline float AngleBetween(const Vector2& rvec) const
	{
		return acosf(Normal().Dot(rvec.Normal()));
	}

	inline float Angle(void) const
	{
		// CCW from world x

		return atanf(y / x);
	}

	inline Vector2 Perp(void) const
	{
		return Vector2(-y, x);
	}

	inline Vector2 PerpAdj(void) const
	{
		// Adjusted for windows coordinate system where y+ points down

		return Vector2(-y, -x);
	}

	inline float Cross(const Vector2& rvecWith) const
	{
		return (x * rvecWith.y - y * rvecWith.x);
	}

	inline Vector2 Projection(const Vector2& rvecOn) const
	{
		return rvecOn.operator *(D3DXVec2Dot(this, &rvecOn) / D3DXVec2Dot(&rvecOn, &rvecOn));
	}

	inline float ScalarProjection(const Vector2& rvecOn) const
	{
		return D3DXVec2Dot(this, &rvecOn) / D3DXVec2Length(&rvecOn);
	}

	inline void Set(float ftx, float fty)
	{
		x = ftx;
		y = fty;
	}

	inline void Normalize(void)
	{
		D3DXVec2Normalize(this, this);
	}

	inline void Scale(float fScale)
	{
		x *= fScale;
		y *= fScale;
	}

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	//
	// Deinitialization
	//

	inline void Empty(void)
	{
		x = 0.0f;
		y = 0.0f;
	}

	//
	// Operators
	//

	inline Vector2& operator=(const D3DXVECTOR2& rAssign)
	{
		x = rAssign.x;
		y = rAssign.y;

		return *this;
	}

	inline Vector2& operator=(const POINT& rAssign)
	{
		x = float(rAssign.x);
		y = float(rAssign.y);

		return *this;
	}

	inline Vector2 operator*(float fScale) const
	{
		return Vector2(x * fScale, y * fScale);
	}

	inline Vector2& operator*=(float fScale)
	{
		x *= fScale;
		y *= fScale;

		return *this;
	}

	inline operator POINT(void) const
	{
		POINT pt = { int(x), int(y) };

		return pt;
	}
};

/*----------------------------------------------------------*\
| Vector3 class
\*----------------------------------------------------------*/

class Vector3: public D3DXVECTOR3
{
public:
	Vector3(float fx = 0.0f,
			float fy = 0.0f,
			float fz = 0.0f):
			D3DXVECTOR3(fx, fy, fz)
	{
	}

	Vector3(const Vector3& rInit)
	{
		CopyMemory(this, &rInit, sizeof(D3DXVECTOR3));
	}

	Vector3(const Vector2& rInit)
	{
		x = rInit.x;
		y = rInit.y;
		z = 0.0f;
	}

	Vector3(const D3DXVECTOR3& rInit)
	{
		CopyMemory(this, &rInit, sizeof(D3DXVECTOR3));
	}

public:
	//
	// Operations
	//

	inline float Length(void) const
	{
		return D3DXVec3Length(this);
	}

	inline float LengthSq(void) const
	{
		return D3DXVec3LengthSq(this);
	}

	inline Vector3 Normal(void) const
	{
		float fLength = D3DXVec3Length(this);

		return Vector3(x / fLength, y / fLength, z / fLength);
	}

	inline Vector3 Cross(const Vector3& rvec) const
	{
		return Vector3(y * rvec.z - z * rvec.y,
							   z * rvec.x - x * rvec.z,
							   x * rvec.y - y * rvec.x);
	}

	inline float Dot(const Vector3& rvec) const
	{
		return D3DXVec3Dot(this, &rvec);
	}

	inline void Normalize(void)
	{
		float fLength = D3DXVec3Length(this);

		x /= fLength;
		y /= fLength;
		z /= fLength;
	}

	inline void AddScaled(const Vector3& rvecAdd, float fScale)
	{
		x = x + rvecAdd.x * fScale;
		y = y + rvecAdd.y * fScale;
		z = z + rvecAdd.z * fScale;
	}

	inline void Scale(float fScale)
	{
		x *= fScale;
		y *= fScale;
		z *= fScale;
	}

	inline void Set(float ftx = 0.0f, float fty = 0.0f, float ftz = 0.0f)
	{
		x = ftx;
		y = fty;
		z = ftz;
	}

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	//
	// Deinitialization
	//

	inline void Empty(void)
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	
	//
	// Operators
	//

	inline Vector3& operator=(const D3DXVECTOR3& rAssign)
	{
		x = rAssign.x;
		y = rAssign.y;
		z = rAssign.z;

		return *this;
	}

	inline Vector3& operator=(const Vector2& rAssign)
	{
		x = rAssign.x;
		y = rAssign.y;

		return *this;
	}

	inline Vector3 operator*(float fScale) const
	{
		return Vector3(x * fScale, y * fScale, z * fScale);
	}

	inline Vector3& operator*=(float fScale)
	{
		x *= fScale;
		y *= fScale;

		return *this;
	}

	inline operator Vector2(void)
	{
		return Vector2(x, y);
	}
};

/*----------------------------------------------------------*\
| Rect class
\*----------------------------------------------------------*/

class Rect: public RECT
{
public:
	Rect(void)
	{
		ZeroMemory(this, sizeof(Rect));
	}

	Rect(int initleft, int inittop, int initright, int initbottom);

	Rect(const Rect& rc)
	{
		CopyMemory(this, &rc, sizeof(Rect));
	}

	Rect(const RECT& rc)
	{
		CopyMemory(this, &rc, sizeof(Rect));
	}

	Rect(const POINT& rpt, const SIZE& rps)
	{
		left = rpt.x;
		top = rpt.y;
		right = rpt.x + rps.cx;
		bottom = rpt.y + rps.cy;
	}

	Rect(const POINT& rpt1, const POINT& rpt2)
	{
		left = rpt1.x;
		top = rpt1.y;
		right = rpt2.x;
		bottom = rpt2.y;
	}

public:
	//
	// Operations
	//

	inline int GetWidth(void) const
	{
		return (right - left);
	}

	inline int GetHeight(void) const
	{
		return (bottom - top);
	}

	inline int GetArea(void) const
	{
		return (right - left) * (bottom - top);
	}

	POINT GetPosition(void) const
	{
		POINT pt = { left, top };
		return pt;
	}

	SIZE GetSize(void) const
	{
		SIZE size = { right - left, bottom - top };
		return size;
	}

	POINT GetTopLeft(void) const
	{
		POINT pt = { left, top };
		return pt;
	}

	POINT GetBottomRight(void) const
	{
		POINT pt = { right, bottom };
		return pt;
	}

	POINT GetCenter(void) const
	{
		POINT pt = { left + (right - left) / 2,
					 top + (bottom - top) / 2 };

		return pt;
	}

	void Set(int left, int top, int right, int bottom);
	void Set(const POINT& rpt, const SIZE& rps);
	void Set(const POINT& rpt1, const POINT& rpt2);

	void SetPosition(int left, int top);
	void SetSize(int width, int height);

	void Offset(int x, int y);
	void Offset(POINT pt);

	inline void Inflate(int cx, int cy)
	{
		::InflateRect(this, cx, cy);
	}

	inline bool Intersect(const Rect& rrcOther, Rect& rcDest)
	{
		return (::IntersectRect(&rcDest, this, &rrcOther) == TRUE);
	}

	bool Intersect(const Rect& rrcOther)
	{
		Rect rcDest;

		return (::IntersectRect(&rcDest, this, &rrcOther) == TRUE);
	}

	inline bool PtInRect(POINT pt) const
	{
		return (::PtInRect(this, pt) == TRUE);
	}

	bool PtInRect(int x, int y) const
	{
		POINT pt = { x, y };

		return (::PtInRect(this, pt) == TRUE);
	}

	inline void Empty(void)
	{
		ZeroMemory(this, sizeof(Rect));
	}

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	void Serialize(InfoElem& rRoot) const;
	void Deserialize(const InfoElem& rRoot, int nStartElemIndex = 0);

	//
	// Operators
	//

	Rect& operator=(const Rect& rAssign)
	{
		CopyMemory(this, &rAssign, sizeof(Rect));

		return *this;
	}

	Rect& operator=(const RECT& rAssign)
	{
		CopyMemory(this, &rAssign, sizeof(RECT));

		return *this;
	}

	inline bool operator==(const RECT& rCompare) const
	{
		return (memcmp(this, &rCompare, sizeof(RECT)) == 0);
	}

	inline bool operator!=(const RECT& rCompare) const
	{
		return (memcmp(this, &rCompare, sizeof(RECT)) != 0);
	}
};

} // namespace ThunderStorm

#endif // THUNDER_MATH_H