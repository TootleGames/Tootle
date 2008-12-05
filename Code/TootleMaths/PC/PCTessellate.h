/*------------------------------------------------------

	Tesselation routines to turn an outline(points) into 
	polygons in a mesh - platform specific atm, a generic
	tesselation will be done at some point

-------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TPtrArray.h>
#include <TootleCore/TLMaths.h>
#include <TootleCore/TColour.h>

#include "../TTessellate.h"


namespace TLMaths
{
	class TGlutTessellator;

	namespace Platform
	{
		TTessellator*	CreateTessellator(TPtr<TLAsset::TMesh>& pMesh);	//	create platform specific tessellator
	}
}



class TLMaths::TGlutTessellator : public TLMaths::TTessellator
{
public:
	TGlutTessellator(TPtr<TLAsset::TMesh>& pMesh);

	virtual Bool	GenerateTessellations(TLMaths::TLTessellator::TWindingMode WindingMode,TLMaths::TLTessellator::TOutsetType OutsetType, float zNormal=-1.f,float outsetSize=0.f);			//	generate new polygons into this mesh

	void			AddError(u32 Error)							{	m_Errors.Add( Error );	}

	void			StartPoly(u32 PolyType)						{	m_CurrentPolyType = PolyType;	m_CurrentPolyIndexes.Empty();	}
	void			AddVertex(const float3& Vertex);
	float3*			AddTempVertex(const float3& Vertex)			{	s32 Index = m_TempVerts.Add( Vertex );	return (Index == -1) ? NULL : &m_TempVerts[Index];	}
	void			FinishPoly();

public:
	TArray<u32>		m_Errors;
	u32				m_CurrentPolyType;
	TArray<u16>		m_CurrentPolyIndexes;
	TArray<float3>	m_TempVerts;			//	Holds extra points created by gluTesselator. See ftglCombine.
};

