/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.6 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Platform_SharedPtr
#define TC_HEADER_Platform_SharedPtr

#include <stdexcept>
#include "SharedVal.h"

namespace TrueCrypt
{
	template <class T>
	class SharedPtr 
	{
	public:
		explicit SharedPtr ()
			: Pointer (nullptr), UseCount (nullptr) { }

		explicit SharedPtr (T *pointer)
			: Pointer (pointer), UseCount (new SharedVal <uint64> (1)) { }

		SharedPtr (const SharedPtr &source)
		{
			CopyFrom (source);
		}

		~SharedPtr ()
		{
			Release();
		}

		SharedPtr &operator= (const SharedPtr &source)
		{
			if (&source == this)
				return *this;

			Release();
			CopyFrom (source);
			return *this;
		}

		bool operator == (const SharedPtr &other)
		{
			return get() == other.get();
		}

		bool operator != (const SharedPtr &other)
		{
			return get() != other.get();
		}

		T &operator* () const
		{
#ifdef DEBUG
			if (Pointer == nullptr)
				throw std::runtime_error (SRC_POS);
#endif
			return *Pointer;
		}

		T *operator-> () const
		{
#ifdef DEBUG
			if (Pointer == nullptr)
				throw std::runtime_error (SRC_POS);
#endif
			return Pointer;
		}

		operator bool () const
		{
			return Pointer != nullptr;
		}

		T *get () const
		{
			return Pointer;
		}

		void reset ()
		{
			Release();
		}

		void reset (T *pointer)
		{
			*this = SharedPtr (pointer);
		}

		uint64 use_count () const
		{
			if (!UseCount)
				return 0;

			return *UseCount;
		}

	protected:
		void CopyFrom (const SharedPtr &source)
		{
			Pointer = source.Pointer;
			UseCount = source.UseCount;
			
			if (UseCount)
				UseCount->Increment();
		}

		void Release ()
		{
			if (UseCount != nullptr)
			{
				if (UseCount->Decrement() == 0)
				{
					if (Pointer != nullptr)
						delete Pointer;
					delete UseCount;
				}

				Pointer = nullptr;
				UseCount = nullptr;
			}
		}

		T *Pointer;
		SharedVal <uint64> *UseCount;
	};

#ifdef shared_ptr
#undef shared_ptr
#endif
#define shared_ptr TrueCrypt::SharedPtr

#define make_shared_auto(typeName,instanceName) shared_ptr <typeName> instanceName (new typeName ())

	template <class T> shared_ptr <T> make_shared ()
	{
		return shared_ptr <T> (new T ());
	}

	template <class T, class A> shared_ptr <T> make_shared (const A &arg)
	{
		return shared_ptr <T> (new T (arg));
	}
}

#endif
