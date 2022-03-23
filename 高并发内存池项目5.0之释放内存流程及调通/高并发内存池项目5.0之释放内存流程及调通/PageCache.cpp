#include "PageCache.h"

PageCache PageCache::_sInst;

// 获取一个K页的span
Span* PageCache::NewSpan(size_t k){
	assert(k > 0 && k < NPAGES);

	// 先检查第k个桶里面有没有span
	if (!_spanLists[k].Empty()){
		return _spanLists->PopFront();
	}

	// 检查一下后面的桶里面有没有span，如果有可以把他它进行切分
	for (size_t i = k + 1; i < NPAGES; ++i){
		if (!_spanLists[i].Empty()){
			Span* nSpan = _spanLists[i].PopFront();
			Span* kSpan = new Span;

			// 在nSpan的头部切一个k页下来
			// k页span返回
			// nSpan再挂到对应映射的位置
			kSpan->_pageId = nSpan->_pageId;
			kSpan->_n = k;

			nSpan->_pageId += k;
			nSpan->_n -= k;

			_spanLists[nSpan->_n].PushFront(nSpan);

			// 存储nSpan的首位页号跟nSpan映射，方便page cache回收内存时
			// 进行的合并查找
			_idSpanMap[nSpan->_pageId] = nSpan;
			_idSpanMap[nSpan->_pageId + nSpan->_n - 1] = nSpan;


			// 建立id和span的映射，方便central cache回收小块内存时，查找对应的span
			for (PAGE_ID i = 0; i < kSpan->_n; ++i){
				_idSpanMap[kSpan->_pageId + i] = kSpan;
			}

			return kSpan;
		}
	}

	// 走到这个位置就说明后面没有大页的span了
	// 这时就去找堆要一个128页的span
	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = NPAGES - 1;

	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return NewSpan(k);//回调
}


Span* PageCache::MapObjectToSpan(void* obj){
	PAGE_ID id = ((PAGE_ID)obj >> PAGE_SHIFT);
	auto ret = _idSpanMap.find(id);
	if (ret != _idSpanMap.end()){
		return ret->second;
	}
	else{
		assert(false);
		return nullptr;
	}
}

void PageCache::ReleaseSpanToPageCache(Span* span){
	// 对span前后的页，尝试进行合并，缓解内存碎片问题
	while (1){
		PAGE_ID prevId = span->_pageId - 1;
		auto ret = _idSpanMap.find(prevId);
		// 前面的页号没有，不合并了
		if (ret == _idSpanMap.end()){
			break;
		}

		// 前面相邻页的span在使用，不合并了
		Span* prevSpan = ret->second;
		if (prevSpan->_isUse == true){
			break;
		}

		// 合并出超过128页的span没办法管理，不合并了
		if (prevSpan->_n + span->_n > NPAGES - 1){
			break;
		}

		span->_pageId = prevSpan->_pageId;
		span->_n += prevSpan->_n;

		_spanLists[prevSpan->_n].Erase(prevSpan);
		delete prevSpan;//注意了：span对象是new出来的，span对象不直接存储内存空间，span对象
		//通过页号来与内存空间建立映射关系，因此当prevSpan所指向的span对象无用时应该delete掉
	}

	// 向后合并
	while (1){
		PAGE_ID nextId = span->_pageId + span->_n;
		auto ret = _idSpanMap.find(nextId);
		if (ret == _idSpanMap.end()){
			break;
		}

		Span* nextSpan = ret->second;
		if (nextSpan->_isUse == true){
			break;
		}

		if (nextSpan->_n + span->_n > NPAGES - 1){
			break;
		}

		span->_n += nextSpan->_n;

		_spanLists[nextSpan->_n].Erase(nextSpan);
		delete nextSpan;
	}

	_spanLists[span->_n].PushFront(span);
	span->_isUse = false;
	_idSpanMap[span->_pageId] = span;
	_idSpanMap[span->_pageId + span->_n - 1] = span;
}