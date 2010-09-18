#include "PCTessellate.h"
#include <TootleCore/TLDebug.h>
#include <TootleCore/TString.h>
#include <TootleAsset/TMesh.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TLMaths.h>


//	gr: for now it requires opengl headers for the glut tesselation
#include <TootleOpenglRasteriser/PC/PCOpengl.h>
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
void CALLBACK ftglCombine(GLdouble coords[3], void* vertex_data[4], GLfloat weight[4], void** outData, TLMaths::TGlutTessellator* pTessellator);
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
	TLDebug_Break("Glut Tessellator error");
    pTessellator->AddError(errCode);
}


void CALLBACK ftglVertex(void* data,TLMaths::TGlutTessellator* pTessellator)
{
    float* vertex = static_cast<float*>(data);
	float3 xyz( vertex[0], vertex[1], vertex[2] );
    pTessellator->AddVertex( xyz );
}


void CALLBACK ftglCombine(GLdouble coords[3], void* vertex_data[4], GLfloat weight[4], void** outData,TLMaths::TGlutTessellator* pTessellator)
{
	//	coords is the new position that's been calculated;
	//	it's a combination of the 4 verts passed in (weighted according to weight)
	//	if vertex_data were the colours of 4 verts, you merge them together
	//	we're just dealing with vertexes (no normals, colours, texcoords etc)
	//	so we just take the coord, and the data we pass back is our-vertex-type of data associated with this new vertex (float3)
	float3 xyz( (float)coords[0], (float)coords[1], (float)coords[2] );

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
Bool TLMaths::TGlutTessellator::GenerateTessellations(TLMaths::TLTessellator::TWindingMode WindingMode,float zNormal)
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

	//	count vertexes, we need to pre-alloc the temp verts as it corrupts if the array data is moved during tessellation
	u32 VertexCount = 0;

    for( u32 c=0;	c<m_Contours.GetSize();	c++ )
    {
        TPtr<TLMaths::TContour>& pContour = m_Contours[c];

        gluTessBeginContour(tobj);
		{
            for ( u32 p=0;	p<pContour->GetPoints().GetSize();	p++)
            {
				const float3& Point = pContour->Point(p);

				if ( !TLDebug_CheckFloat(Point ) )
					continue;

				Type3<GLdouble> d( Point.x, Point.y, Point.z );
                gluTessVertex( tobj, d.GetData(), (GLvoid*)Point.GetData() );
				VertexCount++;
            }
		}
        gluTessEndContour(tobj);
    }

	//	pre-alloc temp verts
	m_TempVerts.SetAllocSize( VertexCount );
	
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
			m_pMesh->GenerateTriangle( m_CurrentPolyIndexes[0], m_CurrentPolyIndexes[1], m_CurrentPolyIndexes[2] );
		}
		break;

		case GL_QUADS:
		{
			//	make up new quad
			m_pMesh->GenerateQuad( m_CurrentPolyIndexes );
		}
		break;

		case GL_TRIANGLE_STRIP:
		{
			//	new tri strip
			m_pMesh->GenerateTristrip( m_CurrentPolyIndexes );
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
			//	gr: todo: change to mesh function so we can degenerate small triangle fans
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


