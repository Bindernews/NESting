#pragma once

#include <initializer_list>
#include <algorithm>

#define SLICE_ARRAY(Ty, arr) bn::slice<Ty>((arr), sizeof(arr)/sizeof(Ty))

namespace bn {

template<typename T>
class slice
{
public:
	typedef T Ty;

	slice() : this(nullptr, 0) {};
	slice(T* ptr, size_t size) : d(ptr), _len(size) {}

	inline T& operator[] (size_t n) { return d[n]; }
	inline const T& operator[] (size_t n) const { return d[n]; }

	inline size_t len() const { return _len; }

	/// Pointer to the data refernced by this slice.
	T* d;

private:
	/// The length of this slice
	size_t _len;
};

}