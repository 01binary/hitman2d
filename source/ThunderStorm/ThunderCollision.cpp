/*------------------------------------------------------------------*\
|
| ThunderCollision.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm collision detection class(es) implementation
| Created: 08/26/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"					// precompiled header
#include "ThunderActor.h"			// using Actor
#include "ThunderTile.h"			// using Tile
#include "ThunderCollision.h"		// defining classes

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Collision implementation
\*----------------------------------------------------------*/

CollisionInfo::CollisionInfo(void): m_pTile(NULL),
									m_pActor(NULL)
{
}

bool CollisionInfo::IsTileCollision(void) const
{
	return (m_pTile != NULL);
}

Tile* CollisionInfo::GetTile(void)
{
	return m_pTile;
}

void CollisionInfo::SetTile(Tile* pTile)
{
	m_pTile = pTile;
}

bool CollisionInfo::IsActorCollision(void) const
{
	return (m_pActor != NULL);
}

Actor* CollisionInfo::GetActor(void)
{
	return m_pActor;
}

void CollisionInfo::SetActor(Actor* pActor)
{
	m_pActor = pActor;
}

const Vector2& CollisionInfo::GetSeparation(void) const
{
	return m_vecSeparation;
}

const Vector2& CollisionInfo::GetContactPoint(void) const
{
	return m_vecContact;
}

/*----------------------------------------------------------*\
| VolumeCircle implementation
\*----------------------------------------------------------*/

VolumeCircle::VolumeCircle(void): m_fRadius(0.0f)
{
}

VolumeCircle::VolumeCircle(const Vector2& rvecCenterPoint,
	float fRadius):
	m_vecCenterPoint(rvecCenterPoint),
	m_fRadius(fRadius)
{
}

VolumeCircle::VolumeCircle(const VolumeOBB& rbvOBB)
{
	Set(rbvOBB);
}

VolumeCircle::VolumeCircle(const VolumeHull& rvbHull)
{
	Set(rvbHull);
}

VolumeCircle::VolumeCircle(
	const VolumeCircle& rInit):
	m_vecCenterPoint(rInit.m_vecCenterPoint),
	m_fRadius(rInit.m_fRadius)
{
}

const Vector2& VolumeCircle::GetCenterPoint(void) const
{
	return m_vecCenterPoint;
}

float VolumeCircle::GetRadius(void) const
{
	return m_fRadius;
}

void VolumeCircle::Set(const Vector2& rvecCenterPoint, float fRadius)
{
	m_vecCenterPoint = rvecCenterPoint;
	m_fRadius = fRadius;
}

void VolumeCircle::Set(const VolumeOBB& rbvOBB)
{
	// TODO
}

void VolumeCircle::Set(const VolumeHull& rbvHull)
{
	// TODO
}

bool VolumeCircle::Intersect(const VolumeCircle& rbvOther,
									 const Vector2* pvecVelocity,
									 Vector2* pvecOutContact,
									 Vector2* pvecOutSeparation) const
{
	Vector2 vecDistance = rbvOther.m_vecCenterPoint - m_vecCenterPoint;

	float fRadiiSum = m_fRadius + rbvOther.m_fRadius;

	if (vecDistance.LengthSq() < fRadiiSum * fRadiiSum)
	{
		// TODO: report contact and separation

		return true;
	}

	return false;
}

void VolumeCircle::Empty(void)
{
	m_vecCenterPoint.Empty();
	m_fRadius = 0.0f;
}

VolumeCircle& VolumeCircle::operator=(const VolumeCircle& rAssign)
{
	m_vecCenterPoint = rAssign.m_vecCenterPoint;
	m_fRadius = rAssign.m_fRadius;

	return *this;
}

/*----------------------------------------------------------*\
| VolumeAABB implementation
\*----------------------------------------------------------*/

VolumeAABB::VolumeAABB(void)
{
}

VolumeAABB::VolumeAABB(const Vector2& rvecMin,
					   const Vector2& rvecMax):
					   m_vecMin(rvecMin),
					   m_vecMax(rvecMax)
{
}

VolumeAABB::VolumeAABB(float x1, float y1, float x2, float y2):
					   m_vecMin(x1, y1),
					   m_vecMax(x2, y2)
{
}

VolumeAABB::VolumeAABB(const VolumeAABB& rInit):
					   m_vecMin(rInit.m_vecMin),
					   m_vecMax(rInit.m_vecMax)
{
}

VolumeAABB::VolumeAABB(const VolumeOBB& rbvOBB)
{
	Set(rbvOBB);
}

VolumeAABB::VolumeAABB(const VolumeHull& rbvHull)
{
	Set(rbvHull);
}

const Vector2& VolumeAABB::GetMin(void) const
{
	return m_vecMin;
}

const Vector2& VolumeAABB::GetMax(void) const
{
	return m_vecMax;
}

void VolumeAABB::Set(const Vector2& rvecMin, const Vector2& rvecMax)
{
	m_vecMin = rvecMin;
	m_vecMax = rvecMax;
}

void VolumeAABB::Set(const VolumeOBB& rbvOBB)
{
	// TODO
}

void VolumeAABB::Set(const VolumeHull& rbvHull)
{
	// TODO
}

bool VolumeAABB::Intersect(const VolumeAABB& rbvOther,
						   const Vector2* pvecVelocity,
						   Vector2* pvecOutContact,
						   Vector2* pvecOutSeparation) const
{
	if (m_vecMin.x > rbvOther.m_vecMax.x)
		return false;

	if (m_vecMax.x <= rbvOther.m_vecMin.x)
		return false;

	if (m_vecMin.y > rbvOther.m_vecMax.y)
		return false;

	if (m_vecMax.y <= rbvOther.m_vecMin.y)
		return false;

	if (pvecOutSeparation != NULL)
	{
		// Calculate penetration
		// TODO: this does not account for either AABB being inside of another - revise axes!

		if (rbvOther.m_vecMin.x >= m_vecMin.x && rbvOther.m_vecMin.x <= m_vecMax.x)
			pvecOutSeparation->x = rbvOther.m_vecMin.x - m_vecMax.x;
		else if (m_vecMin.x >= rbvOther.m_vecMin.x && m_vecMin.x <= rbvOther.m_vecMax.x)
			pvecOutSeparation->x = rbvOther.m_vecMax.x - m_vecMin.x;

		if (rbvOther.m_vecMin.y >= m_vecMin.y && rbvOther.m_vecMin.y <= m_vecMax.y)
			pvecOutSeparation->y = rbvOther.m_vecMin.y - m_vecMax.y;
		else if (m_vecMin.y >= rbvOther.m_vecMin.y && m_vecMin.y <= rbvOther.m_vecMax.y)
			pvecOutSeparation->y = rbvOther.m_vecMax.y - m_vecMin.y;

		if (pvecVelocity != NULL)
		{
			// If given movement vector, weigh penetration in movement direction

			float fVelocity = pvecVelocity->Length();

			pvecOutSeparation->x *= fabsf(pvecVelocity->x / fVelocity);
			pvecOutSeparation->y *= fabsf(pvecVelocity->y / fVelocity);
		}
		else
		{
			// Choose the smallest absolute penetration axis

			if (fabsf(pvecOutSeparation->x) > fabsf(pvecOutSeparation->y)) 
				pvecOutSeparation->x = 0.0f;
			else
				pvecOutSeparation->y = 0.0f;
		}
	}

	return true;
}

void VolumeAABB::Empty(void)
{
	m_vecMin.Empty();
	m_vecMax.Empty();
}

VolumeAABB& VolumeAABB::operator=(const VolumeAABB& rAssign)
{
	m_vecMin = rAssign.m_vecMin;
	m_vecMax = rAssign.m_vecMax;
	
	return *this;
}

/*----------------------------------------------------------*\
| VolumeOBB implementation
\*----------------------------------------------------------*/

VolumeOBB::VolumeOBB(void)
{
}

VolumeOBB::VolumeOBB(const Vector2& rvecCenterPoint,
									 const Vector2& rvecPivotPoint,
									 const Vector2& rvecSize,
									 float fRotation,
									 float fScale)
{
	Set(rvecCenterPoint, rvecPivotPoint, rvecSize, fRotation, fScale);
}

VolumeOBB::VolumeOBB(const VolumeOBB& rInit)
{
	m_vecCenterPoint = rInit.m_vecCenterPoint;
	m_vecAxes[0] = rInit.m_vecAxes[0];
	m_vecAxes[1] = rInit.m_vecAxes[1];
	m_vecExtents = rInit.m_vecExtents;
}

VolumeOBB::VolumeOBB(const VolumeHull& rbvHull)
{
	Set(rbvHull);
}

const Vector2& VolumeOBB::GetCenterPoint(void) const
{
	return m_vecCenterPoint;
}

const Vector2* VolumeOBB::GetAxes(void) const
{
	return m_vecAxes;
}

const Vector2& VolumeOBB::GetExtents(void) const
{
	return m_vecExtents;
}

void VolumeOBB::Set(const Vector2& rvecCenterPoint,
							const Vector2& rvecPivotPoint,
							const Vector2& rvecSize,
							float fRotation,
							float fScale)
{
	// TODO
}

void VolumeOBB::Set(const VolumeHull& rbvHull)
{
	// TODO
}

bool VolumeOBB::Intersect(const VolumeOBB& rbvOther,
								  const Vector2* pvecVelocity,
								  Vector2* pvecOutContact,
								  Vector2* pvecOutSeparation) const
{
	// TODO: OBB vs OBB

	return false;
}

void VolumeOBB::Empty(void)
{
	m_vecCenterPoint.Empty();
	m_vecAxes[0].Empty();
	m_vecAxes[1].Empty();
	m_vecExtents.Empty();
}

VolumeOBB& VolumeOBB::operator=(const VolumeOBB& rAssign)
{
	m_vecCenterPoint = rAssign.m_vecCenterPoint;
	m_vecAxes[0] = rAssign.m_vecAxes[0];
	m_vecAxes[1] = rAssign.m_vecAxes[1];
	m_vecExtents = rAssign.m_vecExtents;

	return *this;
}

/*----------------------------------------------------------*\
| VolumeHull implementation
\*----------------------------------------------------------*/

VolumeHull::VolumeHull(void)
{
}

VolumeHull::VolumeHull(const Vector2& rvecCenterPoint,
									   const Vector2& rvecPivotPoint,
									   const Vector2Array& rarLocalSpacePoints,
									   float fRotation,
									   float fScale)
{
	Set(rvecCenterPoint, rvecPivotPoint, rarLocalSpacePoints, fRotation, fScale);
}

const Vector2& VolumeHull::GetCenterPoint(void) const
{
	return m_vecCenterPoint;
}

const Vector2Array& VolumeHull::GetEdges(void) const
{
	return m_arEdges;
}

void VolumeHull::Set(const Vector2& rvecCenterPoint,
							 const Vector2& rvecPivotPoint,
							 const Vector2Array& rarLocalSpacePoints,
							 float fRotation,
							 float fScale)
{
	// TODO
}

bool VolumeHull::Intersect(const VolumeHull& rbvOther,
								   const Vector2* pvecVelocity,
								   Vector2* pvecOutContact,
								   Vector2* pvecOutSeparation) const
{
	// TODO: convex hull vs convex hull

	return false;
}

void VolumeHull::Empty(void)
{
	m_vecCenterPoint.Empty();
	m_arEdges.clear();
}

VolumeHull& VolumeHull::operator=(const VolumeHull& rAssign)
{
	m_vecCenterPoint = rAssign.m_vecCenterPoint;

	m_arEdges.clear();

	m_arEdges = rAssign.m_arEdges;

	return *this;
}