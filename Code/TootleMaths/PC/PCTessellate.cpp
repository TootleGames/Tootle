#include "PCTessellate.h"
#include <TootleCore/TLDebug.h>
#include <TootleCore/TString.h>
#include <TootleAsset/TMesh.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TLMaths.h>


//	gr: for now it requires opengl headers for the glut tesselation
#include <TootleRender/PC/PCRender.h>
#include <gl/glu.h>

#if defined __APPLE_CC__ && __APPLE_CC__ < 5465
    typedef GLvoid (*GLUTesselatorFunction) (...);
#elif defined WIN32 && !defined __CYGWIN__
    typedef GLvoid (CALLBACK *GLUTesselatorFunction) ();
#else
    typedef GLvoid (*GLUTesselatorFunction) ();
#endif



void CALLBACK ftglError(GLenum errCode, TLMaths::TGlutTessellator* pTessellator);
void CALLBACK ftglVertex(void* data, TLMaths::TGlutTessellator* pTessellator);
void CALLBACK ftglCombine(float coords[3], void* vertex_data[4], GLfloat weight[4], void** outData, TLMaths::TGlutTessellator* pTessellator);
void CALLBACK ftglBegin(GLenum type, TLMaths::TGlutTessellator* pTessellator);
void CALLBACK ftglEnd(TLMaths::TGlutTessellator* pTessellator);
void CALLBACK ftglEdgeFlag(GLboolean EdgeFlag,TLMaths::TGlutTessellator* pTessellator);



//------------------------------------------------
//	create platform specific tessellator
//------------------------------------------------
TLMaths::TTessellator* TLMaths::Platform::CreateTessellator(TPtr<TLAsset::TMesh>& pMesh)
{
	return new TLMaths::TGlutTessellator( pMesh );
}



void CALLBACK ftglError(GLenum errCode,TLMaths::TGlutTessellator* pTessellator)
{
    pTessellator->AddError(errCode);
}


void CALLBACK ftglVertex(void* data,TLMaths::TGlutTessellator* pTessellator)
{
    float* vertex = static_cast<float*>(data);
	float3 xyz( vertex[0], vertex[1], vertex[2] );
    pTessellator->AddVertex( xyz );
}


void CALLBACK ftglCombine(float coords[3], void* vertex_data[4], GLfloat weight[4], void** outData,TLMaths::TGlutTessellator* pTessellator)
{
    const float* vertex = static_cast<const float*>(coords);
	float3 xyz( vertex[0], vertex[1], vertex[2] );

	float3* pTempPoint = pTessellator->AddTempVertex( xyz );
	*outData = pTempPoint->GetData();
}

void CALLBACK ftglBegin(GLenum type,TLMaths::TGlutTessellator* pTessellator)
{
    pTessellator->StartPoly( type );
}


void CALLBACK ftglEnd(TLMaths::TGlutTessellator* pTessellator)
{
    pTessellator->FinishPoly();
}



void CALLBACK ftglEdgeFlag(GLboolean EdgeFlag,TLMaths::TGlutTessellator* pTessellator)
{
    
}




TLMaths::TGlutTessellator::TGlutTessellator(TPtr<TLAsset::TMesh>& pMesh) : 
	TLMaths::TTessellator	( pMesh ),
	m_CurrentPolyType		( 0 )
{
}


//-------------------------------------------------------------
//	generate new polygons into this mesh
//	gr: if you find polygons aren't appearing (maybe a CW/CCW issue) 
//	then try flipping the znormal as this is dependant on the [counter]clockwise order
//-------------------------------------------------------------
Bool TLMaths::TGlutTessellator::GenerateTessellations(TLMaths::TLTessellator::TWindingMode WindingMode,TLMaths::TLTessellator::TOutsetType OutsetType,float zNormal,float outsetSize)
{
	if ( !m_pMesh )
	{
		TLDebug_Break("Mesh expected");
		return FALSE;
	}

	GLUtesselator* tobj = gluNewTess();

	gluTessCallback(tobj, GLU_TESS_BEGIN_DATA,		(GLUTesselatorFunction)ftglBegin);
    gluTessCallback(tobj, GLU_TESS_VERTEX_DATA,		(GLUTesselatorFunction)ftglVertex);
    gluTessCallback(tobj, GLU_TESS_COMBINE_DATA,	(GLUTesselatorFunction)ftglCombine);
    gluTessCallback(tobj, GLU_TESS_END_DATA,		(GLUTesselatorFunction)ftglEnd );
    gluTessCallback(tobj, GLU_TESS_ERROR_DATA,		(GLUTesselatorFunction)ftglError );
    //gluTessCallback(tobj, GLU_TESS_EDGE_FLAG_DATA,	(GLUTesselatorFunction)ftglEdgeFlag );
	

	switch ( WindingMode )
	{
	case TLMaths::TLTessellator::WindingMode_NonZero:
		gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
		break;

	case TLMaths::TLTessellator::WindingMode_Positive:
		gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
		break;

	default:
	case TLMaths::TLTessellator::WindingMode_Odd:
		gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
		break;
	};

    gluTessProperty(tobj, GLU_TESS_TOLERANCE, 0);
    gluTessNormal(tobj, 0.0f, 0.0f, zNormal);
    gluTessBeginPolygon(tobj, this );

    for( u32 c=0;	c<m_Contours.GetSize();	c++ )
    {
        TPtr<TLMaths::TContour>& pContour = m_Contours[c];

		// Build the
        switch(OutsetType)
        {
			case TLMaths::TLTessellator::OutsetType_Front:	pContour->buildFrontOutset(outsetSize); break;
            case TLMaths::TLTessellator::OutsetType_Back:	pContour->buildBackOutset(outsetSize); break;
        }

        gluTessBeginContour(tobj);
		{
            for ( u32 p=0;	p<pContour->PointCount();	p++)
            {
				const float3* pPoint = NULL;
                switch(OutsetType)
                {
                    case TLMaths::TLTessellator::OutsetType_Front:	pPoint = &pContour->FrontPoint(p); break;
                    case TLMaths::TLTessellator::OutsetType_Back:	pPoint = &pContour->BackPoint(p); break;
                    case TLMaths::TLTessellator::OutsetType_None:	pPoint = &pContour->Point(p); break;
					default:										pPoint = &pContour->Point(p); break;
                }

				if ( !pPoint )
					continue;

				if ( !TLDebug_CheckFloat( *pPoint ) )
					continue;

				Type3<GLdouble> d( pPoint->x, pPoint->y, pPoint->z );
                gluTessVertex(tobj, d.GetData(), (GLvoid*)pPoint->GetData() );
            }
		}
        gluTessEndContour(tobj);
    }
	
    gluTessEndPolygon(tobj);
    gluDeleteTess(tobj);

	return TRUE;
}


//-------------------------------------------------------------
//	add vertex to the mesh and record it's index for the final poly
//-------------------------------------------------------------
void TLMaths::TGlutTessellator::AddVertex(const float3& Vertex)
{
	if ( !m_pMesh )
	{
		TLDebug_Break("Mesh expected");
		return;
	}

	s32 VertIndex = m_VertexColourValid ? m_pMesh->AddVertex( Vertex, m_VertexColour ) : m_pMesh->AddVertex( Vertex );
	if ( VertIndex == -1 )
		return;

	m_CurrentPolyIndexes.Add( (u16)VertIndex );
}

//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLMaths::TGlutTessellator::FinishPoly()
{
	if ( !m_pMesh )
	{
		TLDebug_Break("Mesh expected");
		return;
	}

	if ( m_CurrentPolyIndexes.GetSize() < 3 )
	{
		if ( !TLDebug_Break("not enough points for new polygon...") )
			return;
	}

	//	make up the new polygon from our points
	switch ( m_CurrentPolyType )
	{
		case GL_TRIANGLES:
		{
			//	new triangle
			TLAsset::TMesh::Triangle* pTriangle = m_pMesh->GetTriangles().AddNew();
			pTriangle->Set( m_CurrentPolyIndexes[0], m_CurrentPolyIndexes[1], m_CurrentPolyIndexes[2] );
		}
		break;

		case GL_QUADS:
		{
			//	new quad as a tri strip
			TLAsset::TMesh::Tristrip* pTristrip = m_pMesh->GetTristrips().AddNew();
			pTristrip->Add( m_CurrentPolyIndexes[0] );
			pTristrip->Add( m_CurrentPolyIndexes[1] );
			pTristrip->Add( m_CurrentPolyIndexes[3] );
			pTristrip->Add( m_CurrentPolyIndexes[2] );
		}
		break;

		case GL_TRIANGLE_STRIP:
		{
			//	new tri strip
			TLAsset::TMesh::Tristrip* pTristrip = m_pMesh->GetTristrips().AddNew();
			for( u32 p=0;	p<m_CurrentPolyIndexes.GetSize();	p++ )
				pTristrip->Add( m_CurrentPolyIndexes[p] );
		}
		break;

		case GL_POLYGON:
		{
			//	new polygon
			//	gr: todo: need to tristrip this polygon as polys aren't supported any more
			TLDebug_Break("gr: todo: tristrip polygon");
			/*
			TLAsset::TMesh::Polygon* pPolygon = m_pMesh->GetPolygons().AddNew();
			for( u32 p=0;	p<m_CurrentPolyIndexes.GetSize();	p++ )
				pPolygon->Add( m_CurrentPolyIndexes[p] );
			*/
		}
		break;

		case GL_TRIANGLE_FAN:
		{
			//	new polygon
			TLAsset::TMesh::Trifan* pTrifan = m_pMesh->GetTrifans().AddNew();
			for( u32 p=0;	p<m_CurrentPolyIndexes.GetSize();	p++ )
				pTrifan->Add( m_CurrentPolyIndexes[p] );
		}
		break;

		default:
		{
			TTempString DebugString;
			DebugString.Appendf("Unknown polygon type (0x%x) in tesselation of character", m_CurrentPolyType );
			TLDebug_Print(DebugString);
		}
		break;
	}


}


