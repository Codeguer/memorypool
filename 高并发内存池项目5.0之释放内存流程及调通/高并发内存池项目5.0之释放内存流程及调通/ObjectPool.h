#pragma once
#include"Common.h"

template<class T>
class ObjectPool{
private:
	char* _memory = nullptr;//管理大块内存
	size_t remainBytes = 0;//记录_memory管理的大块内存的数量
	void* _freeList = nullptr;//管理释放回来的内存

public:
	T*New(){
		T*obj = nullptr;//要返回的内存块对象
		if(_freeList){//优先从_freeList获取内存
			obj = (T*)_freeList;
			_freeList = NextObj(_freeList);
		}
		else {
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			if (remainBytes < objSize) {//如果所剩内存大小不足一个objSize那么就要申请一块大的
				remainBytes = 128 * 1024;
				//按页为单位获取内存
				_memory =(char*) SystemAlloc(remainBytes >> 13);//一页为8kb
			}
			obj = (T*)_memory;
			remainBytes -= objSize;
			_memory += objSize;
		}

		new(obj)T;//定位new表达式，显示调用T类的构造函数进行初始化

		return obj;
	}

	void Delete(T *obj) {//将释放的obj交给_freeList
		obj->~T();//显示调用T类的析构函数完成obj对象的清理
		//头插
		NextObj((void*)obj) = _freeList;//这就必须要保证obj大小至少要sizeof(void*)的大小
		_freeList = obj;
	}
};

struct TreeNode//自定义类型
{
	int _val;
	TreeNode* _left;
	TreeNode* _right;

	TreeNode()
		:_val(0)
		, _left(nullptr)
		, _right(nullptr)
	{}
};

void TestObjectPool()
{
	// 申请释放的轮次
	const size_t Rounds = 5;

	// 每轮申请释放多少次
	const size_t N = 100000;

	std::vector<TreeNode*> v1;
	v1.reserve(N);

	size_t begin1 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v1.push_back(new TreeNode);
		}
		for (int i = 0; i < N; ++i)
		{
			delete v1[i];
		}
		v1.clear();
	}

	size_t end1 = clock();

	std::vector<TreeNode*> v2;
	v2.reserve(N);

	ObjectPool<TreeNode> TNPool;
	size_t begin2 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v2.push_back(TNPool.New());
		}
		for (int i = 0; i < N; ++i)
		{
			TNPool.Delete(v2[i]);
		}
		v2.clear();
	}
	size_t end2 = clock();

	std::cout << "new cost time:" << end1 - begin1 << std::endl;
	std::cout << "object pool cost time:" << end2 - begin2 << std::endl;
}