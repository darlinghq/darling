#ifndef OLD_PTR_H
#define OLD_PTR_H


template<typename T>
static T* advance_ptr(T* ptr, size_t bytes)
{
	uintptr_t v = reinterpret_cast<uintptr_t>(ptr);
	v += bytes;
	return reinterpret_cast<T*>(v);
}


#endif
