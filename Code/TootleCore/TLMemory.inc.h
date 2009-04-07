/*------------------------------------------------------
	Expanded/FORCEINLINEd code for TLMemory.h

-------------------------------------------------------*/



//-------------------------------------------------------
//	delete an object
//-------------------------------------------------------
template<typename TYPE>
void TLMemory::Delete(TYPE*& pData)
{
	if ( !pData )
		return;

	//	sometimes we want to NULL the source pointer before we delete it because of destructors updating other lists etc etc
	TYPE* pTmpData = pData;
	pData = NULL;

	delete pTmpData;
}


//-------------------------------------------------------
//	delete array of mem
//-------------------------------------------------------
template<typename TYPE>
void TLMemory::DeleteArray(TYPE*& pData)
{
	if ( !pData )
		return;

	//	sometimes we want to NULL the source pointer before we delete it because of destructors
	TYPE* pTmpData = pData;
	pData = NULL;

	delete[] pTmpData;
}


//--------------------------------------------------------
//	copy a load of raw data
//--------------------------------------------------------
template<typename TYPE>
void TLMemory::CopyData(TYPE* pToData,const TYPE* pFromData,u32 Elements)
{
	//	check params
	if ( !pToData )		{	Debug::Break("ToData expected in CopyData", __FUNCTION__ );	}
	if ( !pFromData )	{	Debug::Break("FromData expected in CopyData", __FUNCTION__ );	}

	//	no work to do
	if ( !Elements || !pToData || !pFromData )
		return;

	Platform::MemCopy( pToData, pFromData, Elements * sizeof(TYPE) );
}


//--------------------------------------------------------
//	move a load of raw data
//--------------------------------------------------------
template<typename TYPE>
void TLMemory::MoveData(TYPE* pToData,const TYPE* pFromData,u32 Elements)
{
	//	check params
	if ( !pToData )		{	Debug::Break("ToData expected in MoveData", __FUNCTION__ );	}
	if ( !pFromData )	{	Debug::Break("FromData expected in MoveData", __FUNCTION__ );	}

	//	no work to do
	if ( !Elements || !pToData || !pFromData )
		return;

	Platform::MemMove( pToData, pFromData, Elements * sizeof(TYPE) );
}


