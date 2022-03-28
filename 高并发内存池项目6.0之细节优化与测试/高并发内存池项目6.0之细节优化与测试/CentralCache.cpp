#include "CentralCache.h"
#include"PageCache.h"
CentralCache CentralCache::_sInst;

// 获取一个非空的span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size){
	// 查看当前的spanlist中是否有还有未分配对象的span
	Span* it = list.Begin();
	while (it != list.End()){
		if (it->_freeList != nullptr){
			return it;
		}
		else{
			it = it->_next;
		}
	}

	// 先把central cache的桶锁解掉，这样如果其他线程释放内存对象回来，不会阻塞
	list._mtx.unlock();

	// 走到这里说明没有空闲span了，只能找page cache要
	PageCache::GetInstance()->_pageMtx.lock();//全锁
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	span->_isUse = true;
	span->_objSize = size;
	PageCache::GetInstance()->_pageMtx.unlock();

	// 对获取span进行切分，不需要加锁，因为这会其他线程访问不到这个span（span是局部变量）

	// 计算span的大块内存的起始地址和大块内存的大小(字节数)
	char* start = (char*)(span->_pageId << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;

	// 把大块内存切成自由链表链接起来
	// 1、先切一块下来去做头，方便尾插
	span->_freeList = start;
	start += size;
	void* tail = span->_freeList;
	int i = 1;
	while (start < end)
	{
		++i;
		NextObj(tail) = start;
		tail = NextObj(tail); // tail = start;
		start += size;
	}
	NextObj(tail) = nullptr;
	// 切好span以后，需要把span挂到桶里面去的时候，再加锁
	list._mtx.lock();
	list.PushFront(span);

	return span;
}

// 从中心缓存获取一定数量的对象给thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size){
	size_t index = SizeClass::Index(size);//先根据对象的大小找到_spanLists对应的下标
	_spanLists[index]._mtx.lock();//寻找合适的span

	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);

	// 从span中获取batchNum个对象
	// 如果不够batchNum个，有多少拿多少
	start = span->_freeList;
	end = start;
	size_t i = 0;
	size_t actualNum = 1;
	while (i < batchNum - 1 && NextObj(end) != nullptr){
		end = NextObj(end);
		++i;
		++actualNum;
	}
	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;
	span->_useCount += actualNum;

	_spanLists[index]._mtx.unlock();

	return actualNum;
}

void CentralCache::ReleaseListToSpans(void* start, size_t size){
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();//桶锁
	while (start){
		void* next = NextObj(start);

		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
		NextObj(start) = span->_freeList;
		span->_freeList = start;
		span->_useCount--;

		// 说明span的切分出去的所有小块内存都回来了
		// 这个span就可以再回收给page cache，pagecache可以再尝试去做前后页的合并
		if (span->_useCount == 0){
			_spanLists[index].Erase(span);
			span->_freeList = nullptr;
			span->_next = nullptr;
			span->_prev = nullptr;

			// 释放span给page cache时，使用page cache的锁就可以了
			// 这时把桶锁解掉
			_spanLists[index]._mtx.unlock();

			PageCache::GetInstance()->_pageMtx.lock();
			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
			PageCache::GetInstance()->_pageMtx.unlock();

			_spanLists[index]._mtx.lock();
		}

		start = next;
	}

	_spanLists[index]._mtx.unlock();
}