/*------------------------------------------------------
	Expanded/FORCEINLINEd code for TLMemory.h

-------------------------------------------------------*/



template<typename TYPE>
TYPE* TLMemory::SOAAllocate(std::size_t size, Bool bThrow)
{
#ifdef ENABLE_SOA
	return ( static_cast<TYPE*>(TLMemory::TMemorySystem::Instance().GetSmallObjectAllocator().Allocate(size, bThrow)) );
#else
	return ( static_cast<TYPE*>(TLMemory::TMemorySystem::Instance().Allocate(size)) ) ;
#endif
}
	

//-------------------------------------------------------
//	delete an object
//-------------------------------------------------------
template<typename TYPE>
void TLMemory::SOADelete(TYPE*& pData)
{
	if ( !pData )
		return;
	
#ifdef ENABLE_SOA	
	TLMemory::TMemorySystem::Instance().GetSmallObjectAllocator().Deallocate( pData );
#else
	TLMemory::TMemorySystem::Instance().Deallocate(pData);
#endif
	pData = NULL;

}






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

	//	gr: put size into 32 bit to get rid of loads of warnings on 64bit mac build
	u32 Size = Elements * (u32)sizeof(TYPE);
	Platform::MemCopy( pToData, pFromData, Size );
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


