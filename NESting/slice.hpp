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
	slice(T* ptr, size_t size) : _data(ptr), _len(size) {}
	slice(const T* ptr, size_t size) : _data(ptr), _len(size) {}

	inline const T& operator[] (size_t n) const { return _data[n]; }
	inline const T& operator[] (int n) const { return _data[n]; }
	inline const T* data() const { return _data; }
	inline size_t len() const { return _len; }

private:
	/// The data pointer
	const T* _data;
	/// The length of this slice
	size_t _len;
};

}