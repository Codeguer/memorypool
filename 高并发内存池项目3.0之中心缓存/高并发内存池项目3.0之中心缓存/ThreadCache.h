#include"Common.h"

//不要用STL的数据结构，因为可能我们的内存池就是用来服务STL的
//以及malloc，因为我们的内存池将来就是要替代malloc的

//_freeLists是一个映射自由链表的哈希桶，即_freeLists[16]，只是桶里存的不是数据，而是obj内存块的地址
//_freeLists数组存放的是头结点_head,由_head来链接obj内存块
//这样申请某个大小的内存直接找到自由链表_freeLists取内存，释放则将内存挂在_freeLists对应下标的地方
class ThreadCache {
private:
	FreeList _freeLists[NFREELIST];
public:
	// 申请和释放内存对象
	//需要通过size的大小进行映射找到对应大小的内存桶，获取一个obj内存块

	//size表示一个obj内存块的大小
	void* Allocate(size_t size);
	void Deallocate(void*ptr, size_t size);//回收一个内存块将其挂在 _freeLists下
	//这里为什么不起名叫New与Delete呢？因为在C++中new与delete除了开辟空间释放空间外
	//还好调用构造函数、析构函数，进行初始化、清理工作，而这里只是单纯的只是进行内存空间的分配与释放
	//即这里是内存池而不是对象池

	// 从中心缓存获取对象
	void* FetchFromCentralCache(size_t index, size_t size);//index表示freeLists的数组下标
};

//第一种方法：建立一个全局的数据结构比如说一个数组或者一个链式结构
//或者用一个map，每个线程都有自己的线程id，然后与thread cache建立映射
//一个线程来了通过某个函数就可知道其线程id，然后通过map找，找不到说明是一个新线程
//申请一个thread cache出来再与其id进行绑定，多个线程访问map就需要加锁了
// map<int, ThreadCache> idCache;//
//第二种方法
// TLS  全称thread local storage//线程本地存储，不是网络的加密的那个库
//线程中的栈是独享的
/*Text  Segment（代码段）、Data  Segment（数据段）、堆都是共享的
如果定义一个函数,在各线程中都可以调用,
如果定义一个全局变量,在各线程中都可以访问到*/
//线程本地存储要做的事情就是给每个线程开辟一个属于自己独有的全局变量！！！

//windows下加__declspec(thread)这个tls_threadcache就是每个线程独有的全局的指针了，Linux有自己的用法
#ifdef _WIN32
	static __declspec(thread) ThreadCache* pTLSThreadCache = nullptr;
#else
	static __thread ThreadCache* pTLSThreadCache = nullptr;
#endif

//TLS分为动态与静态的，静态的比如创建全局变量
//动态的像malloc那样

//子线程的生命周期并不是伴随着整个进程，如果子线程提前结束，那么其挂载的obj就需要释放
//因此就类似回调函数一样当子线程快结束的时候调用回调函数进行内存释放
//使用动态的TLS好处就是可以注册一个这样的函数帮助我们释放obj