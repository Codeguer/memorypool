#include"ThreadCache.h"

void* ThreadCache::Allocate(size_t size) {
	assert(size <= MAX_BYTES);
	size_t alignSize = SizeClass::RoundUp(size);//计算出要获取的单个obj的大小
	size_t index = SizeClass::Index(size);

	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();
	}
	else//向central cache要
	{
		return FetchFromCentralCache(index, alignSize);
	}
}

void ThreadCache::Deallocate(void*ptr, size_t size) {
	assert(ptr);
	assert(size <= MAX_BYTES);

	// 找对映射的自由链表桶，对象插入进入
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);
}

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size) {
	//..............
	return nullptr;
}