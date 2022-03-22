#pragma once
#include<iostream>
#include<vector>
#include<ctime>
#include<assert.h>
#include<thread>

static const size_t MAX_BYTES = 256 * 1024;//所申请内存的最大容量
static const size_t NFREELIST = 208;//线程缓存中的freeLists数组大小


static void*& NextObj(void*obj) {
	return *((void**)obj);
}

//维护一个头结点
class FreeList {
private:
	void*_head = nullptr;
public:
	//头插
	void Push(void*obj) {
		NextObj(obj) = _head;
		_head = obj;
	}

	//头删并返回
	void*Pop() {
		void*obj = _head;
		_head = NextObj(_head);
		return obj;
	}

	bool Empty() {
		return _head == nullptr;
	}
};

// 计算对象大小的对齐映射规则
class SizeClass{
public:
	// 整体控制在最多10%左右的内碎片浪费
	// [1,128]					8byte对齐	    freelist[0,16)
	// [128+1,1024]				16byte对齐	    freelist[16,72)
	// [1024+1,8*1024]			128byte对齐	    freelist[72,128)
	// [8*1024+1,64*1024]		1024byte对齐     freelist[128,184)
	// [64*1024+1,256*1024]		8*1024byte对齐   freelist[184,208)

	static inline size_t _RoundUp(size_t bytes, size_t alignNum) {
		return (bytes + alignNum - 1)&~(alignNum - 1);
	}

	//计算对齐大小，比如size为7那么对齐大小为8
	static inline size_t RoundUp(size_t size) {
		if (size <= 128) {
			return _RoundUp(size, 8);
		}
		else if (size <= 1024) {
			return _RoundUp(size, 16);
		}
		else if (size <= 8*1024) {
			return _RoundUp(size, 128);
		}
		else if (size <= 64*1024) {
			return _RoundUp(size, 1024);
		}
		else if (size <= 256*1024) {
			return _RoundUp(size, 8*1024);
		}
		else {
			assert(false);
		}
		return -1;
	}

	static inline size_t _Index(size_t bytes, size_t align_shift) {
		return (bytes + (1 << align_shift) - 1) >> align_shift - 1;
	}
	//计算映射的哪一个自由链表桶，比如bytes为7，映射在_freeLists下标为0处的自由链表桶
	static inline size_t Index(size_t bytes) {
		// 每个区间有多少个链
		static int group_array[4] = { 16, 56, 56, 56 };
		if (bytes <= 128) {
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024) {
			return _Index(bytes - 128, 4)+group_array[0];
		}
		else if (bytes <= 8*1024) {
			return _Index(bytes - 1024, 7)+ group_array[0]+ group_array[1];
		}
		else if (bytes <= 64*1024) {
			return _Index(bytes - 8*1024, 10) + group_array[0] + group_array[1]+group_array[2];
		}
		else if (bytes <= 256*1024) {
			return _Index(bytes - 64*1024, 13) + group_array[0] + group_array[1] + group_array[2]+group_array[3];
		}
		else {
			assert(false);
		}
		return -1;
	}
};