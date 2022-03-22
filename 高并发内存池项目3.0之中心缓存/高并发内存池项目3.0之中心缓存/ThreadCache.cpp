#include"ThreadCache.h"
#include"CentralCache.h"
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
	// 获取一批对象，数量使用慢启动方式
	//batch批量的意思
	//取两个值较少的那个，目的：当单个对象比较小那么一次就多给点内存块；
				//				当单个对象比较大那么一次就少给点内存块；
	//这样做的目的就是在防止空间浪费与减少访问central cache次数之间进行平衡
	//_freeLists[i].MaxSize()这个一开始设置为了1，那么第一次返回1
	//如果size为8字节，因为有_freeLists[i].MaxSize()的控制不会一开始就返回512个
	//而是慢慢增加第一次返回1个，第二次返回2个，第三次返回3个...（_freeLists[i].MaxSize()不断加1）
	//当_freeLists[i].MaxSize()>512就不再起作用了
	//如果size为8kb,那么当_freeLists[i].MaxSize()>(64kb/8kb)就失去作用了
	size_t batchNum = std::min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
	//这里要强调的是batchNum仅仅是为了提高效率，实际分配的对象数量可能不足batchNum(比如一个span里没有这么多的数量)

	// 去中心缓存获取batch_num个对象
	void* start = nullptr;
	void* end = nullptr;
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	//实际我只想要一个对象，batchNum仅仅是为了提高效率，比如batchNum为10个，可是Span里没有10个空闲的空间
	//只有4个了，那也没有关系，通过返回值就可以知道实际返回了多少个空间

	assert(actualNum > 0);

	if (_freeLists[index].MaxSize() == batchNum) {//即当_freeLists[i].MaxSize()<=SizeClass::NumMoveSize(size)时
		//SizeClass::NumMoveSize(size)区间为[2,512]，如果_freeLists[i].MaxSize()大于这个区间那么
		//_freeLists[i].MaxSize()就失去了作用，因此当_freeLists[i].MaxSize() != batchNum时
		//就不用再关心_freeLists[i].MaxSize()了
		_freeLists[index].MaxSize()+=1;
	}
	
	if (actualNum == 1){
		assert(start == end);
		return start;
	}
	else{
		// >1，返回一个，剩下挂到自由链表
		_freeLists[index].PushRange(NextObj(start), end);
		return start;
	}	
	
}