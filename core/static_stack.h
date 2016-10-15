/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Simple fixed-size contiguous memory container as an alternative to use std::vector and save on 
/// dynamic allocations while still using classes instead of C-style arrays
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "core/base.h"
#include "core/utils.h"

//TODO: Why did I even bother adding this class? Remove (probably)
namespace Core
{
	template<class T, size_t Capacity>
	class cStaticStack
	{
	public:
		typedef T tValueType;

		static const size_t CAPACITY = Capacity;

		cStaticStack()
			: mSize() {}

		// The EmplaceBack implementation would be much cleaner with variadic templates, but in VS2012 they are still not supported
		void EmplaceBack()
		{
			CPR_assert(mSize < CAPACITY, "Size exceeds capacity.");
			new (&mArray[mSize++]) T();
		}

		template<class A0>
		void EmplaceBack(A0&& a0)
		{
			CPR_assert(mSize < CAPACITY, "Size exceeds capacity.");
			new (&mArray[mSize++]) T(forward<A0>(a0));
		}

		template<class A0, class A1>
		void EmplaceBack(A0&& a0, A1&& a1)
		{
			CPR_assert(mSize < CAPACITY, "Size exceeds capacity.");
			new (&mArray[mSize++]) T(forward<A0>(a0), forward<A1>(a1));
		}

		template<class A0, class A1, class A2>
		void EmplaceBack(A0&& a0, A1&& a1, A2&& a2)
		{
			CPR_assert(mSize < CAPACITY, "Size exceeds capacity.");
			new (&mArray[mSize++]) T(forward<A0>(a0), forward<A1>(a1), forward<A2>(a2));
		}

		template<class A0, class A1, class A2, class A3>
		void EmplaceBack(A0&& a0, A1&& a1, A2&& a2, A3&& a3)
		{
			CPR_assert(mSize < CAPACITY, "Size exceeds capacity.");
			new (&mArray[mSize++]) T(forward<A0>(a0), forward<A1>(a1), forward<A2>(a2), forward<A3>(a3));
		}

		T& Back()
		{
			return *At(mSize - 1);
		}

		const T& Back() const
		{
			return *At(mSize - 1);
		}

		void PopBack()
		{
			Back().~T();
			--mSize;
		}

		void Clear()
		{
			if (!std::is_trivially_destructible<T>::value)
			{ 
				DestructAllElements();
			}
			mSize = 0;
		}

		T* Begin() { return At(0); }
		T* End() { return At(mSize); }

		const T* ConstBegin() const { return At(0); }
		const T* ConstEnd() const { return At(mSize); }

		T* begin() { return Begin(); }
		T* end() { return End(); }

		const T* cbegin() const { return ConstBegin(); }
		const T* cend() const { return ConstEnd(); }

		size_t Size() const { return mSize; }
		bool Empty() const { return 0 == mSize; }
		bool Full() const { return mSize >= CAPACITY; }

		const T& operator [] (size_t idx) const { CPR_assert(idx < mSize, "Index out of range."); return *At(idx); }
		T& operator [] (size_t idx) { CPR_assert(idx < mSize, "Index out of range."); return *At(idx); }

		const T& operator [] (int idx) const { return operator [] (static_cast<size_t>(idx)); }
		T& operator [] (int idx) { return operator [] (static_cast<size_t>(idx)); }

		void DestructAllElements()
		{
			for (size_t i = 0; i < mSize; ++i)
				At(i)->~T();
		}

		~cStaticStack()
		{
			DestructAllElements();
		}

	private:
		// Non-copyable/movable since it is slow (unless I end up needing it :P)
		cStaticStack(const cStaticStack&);  /* =delete */
		cStaticStack& operator = (const cStaticStack&); /* =delete */

		T* At(size_t idx) { return static_cast<T*>(static_cast<void*>(&mArray[idx])); }
		const T* At(size_t idx) const { return static_cast<const T*>(static_cast<const void*>(&mArray[idx])); }

		typedef typename std::conditional<std::is_trivial<T>::value,
			T,
			typename std::aligned_storage<sizeof(T), __alignof(T)>::type>::type tStorage;

		tStorage mArray[Capacity];
		size_t mSize;
	};

	// mostly for enabling range-based for loops
	template<class T, size_t Capacity>
	T* begin(cStaticStack<T, Capacity>& static_vector) { return static_vector.Begin(); }

	template<class T, size_t Capacity>
	T* end(cStaticStack<T, Capacity>& static_vector) { return static_vector.End(); }

	template<class T, size_t Capacity>
	const T* begin(cStaticStack<T, Capacity>& static_vector) { return static_vector.ConstBegin(); }

	template<class T, size_t Capacity>
	const T* end(cStaticStack<T, Capacity>& static_vector) { return static_vector.ConstEnd(); }
}