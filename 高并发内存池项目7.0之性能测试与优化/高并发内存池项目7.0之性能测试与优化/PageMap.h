#pragma once
#include"Common.h"

// Single-level array
template <int BITS>
class TCMalloc_PageMap1 {
private:
	static const int LENGTH = 1 << BITS;
	void** array_;//指针数组

public:
	typedef uintptr_t Number;

	explicit TCMalloc_PageMap1() {
		size_t size = sizeof(void*) << BITS;
		size_t alignSize = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);
		array_ = (void**)SystemAlloc(alignSize >> PAGE_SHIFT);//开辟内存给array
		memset(array_, 0, sizeof(void*) << BITS);
	}

	void* get(Number k) const {
		if ((k >> BITS) > 0) {
			return NULL;
		}
		return array_[k];
	}

	void set(Number k, void* v) {
		array_[k] = v;
	}
};