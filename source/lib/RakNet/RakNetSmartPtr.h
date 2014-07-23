#ifndef __RAKNET_SMART_PTR_H
#define __RAKNET_SMART_PTR_H

// From http://www.codeproject.com/KB/cpp/SmartPointers.aspx
// with bugs fixed

#include "RakMemoryOverride.h"
#include "Export.h"

//static int allocCount=0;
//static int deallocCount=0;

namespace RakNet
{

class RAK_DLL_EXPORT ReferenceCounter
{
private:
	int refCount;

public:
	ReferenceCounter() {refCount=0;}
	~ReferenceCounter() {}
	void AddRef() {refCount++;}
	int Release() {return --refCount;}
	int GetRefCount(void) const {return refCount;}
};

template < typename T > class RAK_DLL_EXPORT RakNetSmartPtr
{
private:
	T*    ptr;       // pointer
	ReferenceCounter* reference; // Reference refCount

public:
	RakNetSmartPtr() : ptr(0), reference(0)
	{
		// Do not allocate by default, wasteful if we just have a list of preallocated and unassigend smart pointers
	}

	RakNetSmartPtr(T* pValue) : ptr(pValue)
	{
		reference = RakNet::OP_NEW<ReferenceCounter>(_FILE_AND_LINE_);
		reference->AddRef();

//		allocCount+=2;
//		printf("allocCount=%i deallocCount=%i Line=%i\n",allocCount, deallocCount, __LINE__);
	}

	RakNetSmartPtr(const RakNetSmartPtr<T>& sp) : ptr(sp.ptr), reference(sp.reference)
	{
		if (reference)
			reference->AddRef();
	}

	~RakNetSmartPtr()
	{
		if(reference && reference->Release() == 0)
		{
			RakNet::OP_DELETE(ptr, _FILE_AND_LINE_);
			RakNet::OP_DELETE(reference, _FILE_AND_LINE_);

//			deallocCount+=2;
//			printf("allocCount=%i deallocCount=%i Line=%i\n",allocCount, deallocCount, __LINE__);
		}
	}

	bool IsNull(void) const
	{
		return ptr==0;
	}

	void SetNull(void)
	{
		if(reference && reference->Release() == 0)
		{
			RakNet::OP_DELETE(ptr, _FILE_AND_LINE_);
			RakNet::OP_DELETE(reference, _FILE_AND_LINE_);

//			deallocCount+=2;
//			printf("allocCount=%i deallocCount=%i Line=%i\n",allocCount, deallocCount, __LINE__);
		}
		ptr=0;
		reference=0;
	}

	bool IsUnique(void) const
	{
		return reference->GetRefCount()==1;
	}

	// Allow you to change the values of the internal contents of the pointer, without changing what is pointed to by other instances of the smart pointer
	void Clone(bool copyContents)
	{
		if (IsUnique()==false)
		{
			reference->Release();

			reference = RakNet::OP_NEW<ReferenceCounter>(_FILE_AND_LINE_);
			reference->AddRef();
			T* oldPtr=ptr;
			ptr=RakNet::OP_NEW<T>(_FILE_AND_LINE_);
			if (copyContents)
				*ptr=*oldPtr;
		}
	}

	int GetRefCount(void) const
	{
		return reference->GetRefCount();
	}

	T& operator* ()
	{
		return *ptr;
	}

	const T& operator* () const
	{
		return *ptr;
	}

	T* operator-> ()
	{
		return ptr;
	}

	const T* operator-> () const
	{
		return ptr;
	}

	bool operator == (const RakNetSmartPtr<T>& sp)
	{
		return ptr == sp.ptr;
	}
	bool operator<( const RakNetSmartPtr<T> &right ) {return ptr < right.ptr;}
	bool operator>( const RakNetSmartPtr<T> &right ) {return ptr > right.ptr;}

	bool operator != (const RakNetSmartPtr<T>& sp)
	{
		return ptr != sp.ptr;
	}

	RakNetSmartPtr<T>& operator = (const RakNetSmartPtr<T>& sp)
	{
		// Assignment operator

		if (this != &sp) // Avoid self assignment
		{
			if(reference && reference->Release() == 0)
			{
				RakNet::OP_DELETE(ptr, _FILE_AND_LINE_);
				RakNet::OP_DELETE(reference, _FILE_AND_LINE_);

//				deallocCount+=2;
//				printf("allocCount=%i deallocCount=%i Line=%i\n",allocCount, deallocCount, __LINE__);
			}

			ptr = sp.ptr;
			reference = sp.reference;
			if (reference)
				reference->AddRef();
		}
		return *this;
	}


};

} // namespace RakNet

#endif
