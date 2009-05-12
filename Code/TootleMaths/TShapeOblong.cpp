#include "TShapeOblong.h"
#include <TootleCore/TBinaryTree.h>



//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeOblong2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportArrays("Corners", m_Shape.GetBoxCorners() ) )		return FALSE;
	if ( !Data.ImportData("Valid", m_Shape.IsValid() ) )					return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeOblong2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportArray("Corners", m_Shape.GetBoxCorners() );
	Data.ExportData("Valid", m_Shape.IsValid() );

	return TRUE;
}


Bool TLMaths::TShapeOblong2D::HasIntersection(TShapeBox2D& OtherShape)
{
	//	just to silence the currently seemingly uncessary assert...
	TLDebug_Print("gr: todo? TShapeOblong2D::HasIntersection");
	return FALSE;
}

