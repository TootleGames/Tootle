/*
 *  TSmallObject.hpp
 *  TootleCore
 *
 *  Created by Duane Bradbury on 12/05/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TLMemory.h"


namespace Loki
{
	class NoThreads 
	{
	public:
		class Lock
		{
		};
	};
	class NoMutex	{};
}

namespace TLMemory
{

/** @class SmallObjectBase
 @ingroup SmallObjectGroup
 Base class for small object allocation classes.
 The shared implementation of the new and delete operators are here instead
 of being duplicated in both SmallObject or SmallValueObject, later just 
 called Small-Objects.  This class is not meant to be used directly by clients, 
 or derived from by clients. Class has no data members so compilers can 
 use Empty-Base-Optimization.
 
 @par ThreadingModel
 This class doesn't support ObjectLevelLockable policy for ThreadingModel.
 The allocator is a singleton, so a per-instance mutex is not necessary.
 Nor is using ObjectLevelLockable recommended with SingletonHolder since
 the SingletonHolder::MakeInstance function requires a mutex that exists
 prior to when the object is created - which is not possible if the mutex
 is inside the object, such as required for ObjectLevelLockable.  If you
 attempt to use ObjectLevelLockable, the compiler will emit errors because
 it can't use the default constructor in ObjectLevelLockable.  If you need
 a thread-safe allocator, use the ClassLevelLockable policy.
 
 @par Lifetime Policy
 
 The SmallObjectBase template needs a lifetime policy because it owns
 a singleton of SmallObjAllocator which does all the low level functions. 
 When using a Small-Object in combination with the SingletonHolder template
 you have to choose two lifetimes, that of the Small-Object and that of
 the singleton. The rule is: The Small-Object lifetime must be greater than
 the lifetime of the singleton hosting the Small-Object. Violating this rule
 results in a crash on exit, because the hosting singleton tries to delete
 the Small-Object which is then already destroyed. 
 
 The lifetime policies recommended for use with Small-Objects hosted 
 by a SingletonHolder template are 
 - LongevityLifetime::DieAsSmallObjectParent / LongevityLifetime::DieAsSmallObjectChild
 - SingletonWithLongevity
 - FollowIntoDeath (not supported by MSVC 7.1)
 - NoDestroy
 
 The default lifetime of Small-Objects is 
 LongevityLifetime::DieAsSmallObjectParent to
 insure that memory is not released before a object with the lifetime
 LongevityLifetime::DieAsSmallObjectChild using that
 memory is destroyed. The LongevityLifetime::DieAsSmallObjectParent
 lifetime has the highest possible value of a SetLongevity lifetime, so
 you can use it in combination with your own lifetime not having also
 the highest possible value.
 
 The DefaultLifetime and PhoenixSingleton policies are *not* recommended 
 since they can cause the allocator to be destroyed and release memory 
 for singletons hosting a object which inherit from either SmallObject
 or SmallValueObject.  
 
 @par Lifetime usage
 
 - LongevityLifetime: The Small-Object has 
 LongevityLifetime::DieAsSmallObjectParent policy and the Singleton
 hosting the Small-Object has LongevityLifetime::DieAsSmallObjectChild. 
 The child lifetime has a hard coded SetLongevity lifetime which is 
 shorter than the lifetime of the parent, thus the child dies 
 before the parent.
 
 - Both Small-Object and Singleton use SingletonWithLongevity policy.
 The longevity level for the singleton must be lower than that for the
 Small-Object. This is why the AllocatorSingleton's GetLongevity function 
 returns the highest value.
 
 - FollowIntoDeath lifetime: The Small-Object has 
 FollowIntoDeath::With<LIFETIME>::AsMasterLiftime
 policy and the Singleton has 
 FollowIntoDeath::AfterMaster<MASTERSINGLETON>::IsDestroyed policy,
 where you could choose the LIFETIME. 
 
 - Both Small-Object and Singleton use NoDestroy policy. 
 Since neither is ever destroyed, the destruction order does not matter.
 Note: you will get memory leaks!
 
 - The Small-Object has NoDestroy policy but the Singleton has
 SingletonWithLongevity policy. Note: you will get memory leaks!
 
 
 You should *not* use NoDestroy for the singleton, and then use
 SingletonWithLongevity for the Small-Object. 
 
 @par Examples:
 
 - test/SmallObj/SmallSingleton.cpp
 - test/Singleton/Dependencies.cpp
 */
template
<
std::size_t chunkSize,
std::size_t maxSmallObjectSize,
std::size_t objectAlignSize
>
class SmallObjectBase
{
			
	
#if (LOKI_MAX_SMALL_OBJECT_SIZE != 0) && (LOKI_DEFAULT_CHUNK_SIZE != 0) && (LOKI_DEFAULT_OBJECT_ALIGNMENT != 0)
	
public:
	
	/// Throwing single-object new throws bad_alloc when allocation fails.
#ifdef _MSC_VER
	/// @note MSVC complains about non-empty exception specification lists.
	static void * operator new ( std::size_t size )
#else
	static void * operator new ( std::size_t size ) throw ( std::bad_alloc )
#endif
	{
		// Allocate from the small object allocator
		return g_sMemorySystem.GetSmallObjectAllocator().Allocate(size, TRUE);
	}
	
	/// Non-throwing single-object new returns NULL if allocation fails.
	static void * operator new ( std::size_t size, const std::nothrow_t & ) throw ()
	{

		// Allocate from the small object allocator
		return g_sMemorySystem.GetSmallObjectAllocator().Allocate(size, FALSE);
	}
	
	/// Placement single-object new merely calls global placement new.
	inline static void * operator new ( std::size_t size, void * place )
	{
		return ::operator new( size, place );
	}
	
	/// Single-object delete.
	static void operator delete ( void * pData, std::size_t size ) throw ()
	{
		g_sMemorySystem.GetSmallObjectAllocator().Deallocate(pData, size);
	}
	
	/** Non-throwing single-object delete is only called when nothrow
	 new operator is used, and the constructor throws an exception.
	 */
	static void operator delete ( void * pData, const std::nothrow_t & ) throw()
	{
		g_sMemorySystem.GetSmallObjectAllocator().Deallocate(pData);
	}
	
	/// Placement single-object delete merely calls global placement delete.
	inline static void operator delete ( void * pData, void * place )
	{
		::operator delete ( pData, place );
	}
	
#ifdef LOKI_SMALL_OBJECT_USE_NEW_ARRAY
	
	/// Throwing array-object new throws bad_alloc when allocation fails.
#ifdef _MSC_VER
	/// @note MSVC complains about non-empty exception specification lists.
	static void * operator new [] ( std::size_t size )
#else
	static void * operator new [] ( std::size_t size )
	throw ( std::bad_alloc )
#endif
	{
		return g_sMemorySystem.GetSmallObjectAllocator().Allocate(size, TRUE);
	}
	
	/// Non-throwing array-object new returns NULL if allocation fails.
	static void * operator new [] ( std::size_t size,
								   const std::nothrow_t & ) throw ()
	{
		return g_sMemorySystem.GetSmallObjectAllocator().Allocate(size, FALSE);
	}
	
	/// Placement array-object new merely calls global placement new.
	inline static void * operator new [] ( std::size_t size, void * place )
	{
		return ::operator new( size, place );
	}
	
	/// Array-object delete.
	static void operator delete [] ( void * p, std::size_t size ) throw ()
	{
		g_sMemorySystem.GetSmallObjectAllocator().Deallocate(pData, size);
	}
	
	/** Non-throwing array-object delete is only called when nothrow
	 new operator is used, and the constructor throws an exception.
	 */
	static void operator delete [] ( void * p,
									const std::nothrow_t & ) throw()
	{
		g_sMemorySystem.GetSmallObjectAllocator().Deallocate(pData);
	}
	
	/// Placement array-object delete merely calls global placement delete.
	inline static void operator delete [] ( void * p, void * place )
	{
		::operator delete ( p, place );
	}
#endif  // #if use new array functions.
	
#endif  // #if default template parameters are not zero
	
protected:
	inline SmallObjectBase( void ) {}
	inline SmallObjectBase( const SmallObjectBase & ) {}
	inline SmallObjectBase & operator = ( const SmallObjectBase & )
	{ return *this; }
	inline ~SmallObjectBase() {}
}; // end class SmallObjectBase


/** @class SmallObject
 @ingroup SmallObjectGroup
 SmallObject Base class for polymorphic small objects, offers fast
 allocations & deallocations.  Destructor is virtual and public.  Default
 constructor is trivial.   Copy-constructor and copy-assignment operator are
 not implemented since polymorphic classes almost always disable those
 operations.  Class has no data members so compilers can use
 Empty-Base-Optimization.
 */

template 
<
std::size_t chunkSize = LOKI_DEFAULT_CHUNK_SIZE,
std::size_t maxSmallObjectSize = LOKI_MAX_SMALL_OBJECT_SIZE,
std::size_t objectAlignSize = LOKI_DEFAULT_OBJECT_ALIGNMENT
>
class SmallObject : public SmallObjectBase< chunkSize, maxSmallObjectSize, objectAlignSize >
{

public:
	virtual ~SmallObject() {}
protected:
	inline SmallObject( void ) {}

private:
	/// Copy-constructor is not implemented.
	SmallObject( const SmallObject & );
	/// Copy-assignment operator is not implemented.
	SmallObject & operator = ( const SmallObject & );
}; // end class SmallObject


/** @class SmallValueObject
 @ingroup SmallObjectGroup
 SmallValueObject Base class for small objects with value-type
 semantics - offers fast allocations & deallocations.  Destructor is
 non-virtual, inline, and protected to prevent unintentional destruction
 through base class.  Default constructor is trivial.   Copy-constructor
 and copy-assignment operator are trivial since value-types almost always
 need those operations.  Class has no data members so compilers can use
 Empty-Base-Optimization.
 */
template
<
std::size_t chunkSize = LOKI_DEFAULT_CHUNK_SIZE,
std::size_t maxSmallObjectSize = LOKI_MAX_SMALL_OBJECT_SIZE,
std::size_t objectAlignSize = LOKI_DEFAULT_OBJECT_ALIGNMENT
>
class SmallValueObject : public SmallObjectBase< chunkSize, maxSmallObjectSize, objectAlignSize>
{
protected:
	inline SmallValueObject( void ) {}
	inline SmallValueObject( const SmallValueObject & ) {}
	inline SmallValueObject & operator = ( const SmallValueObject & )
	{ return *this; }
	inline ~SmallValueObject() {}
}; // end class SmallValueObject

};// end namespace



