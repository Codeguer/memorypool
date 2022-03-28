#pragma once

#include "Common.h"
#include"ObjectPool.h"
class PageCache{
public:
	static PageCache* GetInstance(){
		return &_sInst;
	}

	// 获取从对象到span的映射
	Span* MapObjectToSpan(void* obj);

	// 释放空闲span回到Pagecache，并合并相邻的span
	void ReleaseSpanToPageCache(Span* span);

	// 获取一个K页的span
	Span* NewSpan(size_t k);

	std::mutex _pageMtx;//全锁
private:
	SpanList _spanLists[NPAGES];
	ObjectPool<Span> _spanPool;//使用定长内存池代替new

	std::unordered_map<PAGE_ID, Span*> _idSpanMap;//这个很重要，页号与span的映射，当thread cache将一部分对象
	//回收给central cache，这一群对象就能通过映射关系找到对应的span
	//此处待优化，tcmalloc使用的是基数树 ，效率更高

	PageCache()
	{}
	PageCache(const PageCache&) = delete;

	static PageCache _sInst;
};

