#include<iostream>
#include<unistd.h>

//每个内存块的首部需要记录当前块的大小、当前区块是否已经被分配出去。
//首部对应这样的结构体：
struct mem_control_block {
    int is_available; // 是否可用（如果还没被分配出去，就是 1）
    int size;         // 实际空间的大小
};

//使用首次适应法进行分配：遍历整个链表，找到第一个未被分配、大小合适的内存块；
//如果没有这样的内存块，则向操作系统申请扩展堆内存
class MyMalloc{
private:
    int is_initialized;     // 初始化标志
    void *heap_bottom;  // 指向堆底（内存块起始位置）
    void *heap_top;    // 指向堆顶
public:    
    MyMalloc():is_initialized(0),heap_bottom(nullptr),heap_top(nullptr){

    }
    void MyMallocInit() {
        // 这里不向操作系统申请堆空间，只是为了获取堆的起始地址
        heap_top = sbrk(0);//获取堆顶地址
        heap_bottom = heap_top;//初始堆顶与堆底理应相同
        is_initialized = 1;//1表示已初始化
    }
    
    void *malloc(long numbytes) {
        void *current_location;  // 当前访问的内存位置
        struct mem_control_block *current_location_mcb;
        void *memory_location;  // 这是要返回的内存位置。初始时设为
                                // nullptr，表示没有找到合适的位置
        if (!is_initialized) {
            MyMallocInit();
        }
        // 要查找的内存必须包含内存控制块，所以需要调整 numbytes 的大小
        numbytes = numbytes + sizeof(struct mem_control_block);
        // 初始时设为 nullptr，表示没有找到合适的位置
        memory_location = nullptr;
        /* Begin searching at the start of managed memory */
        // 从被管理内存的起始位置开始搜索
        // heap_bottom 是在 MyMallocInit 中通过 sbrk() 函数设置的
        current_location = heap_bottom;
        while (current_location < heap_top) {
            // current_location 是一个 void 指针，用来计算地址；
            // current_location_mcb 是一个具体的结构体类型
            // 这两个实际上是一个含义
            current_location_mcb = (struct mem_control_block *)current_location;
            if (current_location_mcb->is_available) {
                if (current_location_mcb->size >= numbytes) {
                    // 找到一个可用、大小适合的内存块
                    current_location_mcb->is_available = 0;  // 设为不可用
                    memory_location = current_location;      // 设置内存地址
                    break;
                }
            }
            // 否则，当前内存块不可用或过小，移动到下一个内存块
            current_location =(char*) current_location + current_location_mcb->size;
        }
        // 循环结束，没有找到合适的位置，需要向操作系统申请更多内存
        if (!memory_location) {
            // 扩展堆
            sbrk(numbytes);
            // 新的内存的起始位置就是 heap_top 的旧值
            memory_location = heap_top;
            // 将 heap_top 后移 numbytes，移动到整个内存的最右边界
            heap_top = (char*)heap_top + numbytes;
            // 初始化内存控制块 mem_control_block
            current_location_mcb = (struct mem_control_block *)memory_location;
            current_location_mcb->is_available = 0;
            current_location_mcb->size = numbytes;
      }
      // 最终，memory_location 保存了大小为 numbyte的内存空间，
      // 并且在空间的开始处包含了一个内存控制块，记录了元信息
      // 内存控制块对于用户而言应该是透明的，因此返回指针前，跳过内存分配块
      memory_location = (char*)memory_location + sizeof(struct mem_control_block);
      // 返回内存块的指针
      return memory_location;
    }
    void free(void *ptr) {  // ptr 是要回收的空间
      struct mem_control_block *free;
      free = (struct mem_control_block*)ptr - sizeof(struct mem_control_block); // 找到该内存块的控制信息的地址
      free->is_available = 1;  // 该空间置为可用
      return;
    }
//这种方法的缺点是：
//    已分配和未分配的内存块位于同一个链表中，每次分配都需要从头到尾遍历采用首次适应法，
//    内存块会被整体分配，容易产生较多内部碎片
};

int main(){
    MyMalloc mymalloc;
    int *ptr=(int*)mymalloc.malloc(sizeof(int)*10);
    for(int i=0;i < 10;++i){
        ptr[i]=i*i+(i+1)*(i+2);
    }
    for(int i=0;i < 10;++i){
        std::cout<<ptr[i]<<" ";
    }
    std::cout<<std::endl;
    mymalloc.free(ptr);
    return 0;
}
