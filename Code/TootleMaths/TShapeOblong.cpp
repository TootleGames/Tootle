#include "TShapeOblong.h"
#include <TootleCore/TBinaryTree.h>



//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeOblong2D::ImportData(TBinaryTree& Data)
{
	if ( !Data.ImportArrays("Corners", m_Oblong.GetBoxCorners() ) )		return FALSE;
	if ( !Data.ImportData("Valid", m_Oblong.IsValid() ) )					return FALSE;

	return TRUE;
}

//---------------------------------------
//	
//---------------------------------------
Bool TLMaths::TShapeOblong2D::ExportData(TBinaryTree& Data) const
{
	Data.ExportArray("Corners", m_Oblong.GetBoxCorners() );
	Data.ExportData("Valid", m_Oblong.IsValid() );

	return TRUE;
}


Bool TLMaths::TShapeOblong2D::HasIntersection(TShapeBox2D& OtherShape)
{
	//	just to silence the currently seemingly uncessary assert...
	TLDebug_Print("gr: todo? TShapeOblong2D::HasIntersection");
	return FALSE;
}
