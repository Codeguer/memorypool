#include <stdio.h>
#include <unistd.h>

int main(){
    void* p = sbrk(0);//0取当前位置
    void* p1 = sbrk(4);//p1指向初始位置，新位置在4字节
    void* p2 = sbrk(4);//映射一个内存页
    void* p3 = sbrk(4);// 正数代表分配内存
    void* p4 = sbrk(4);//位置在16
    *(int*)p4 = 100;
    printf("堆顶指针开始位置:%p,p1=%p,p2=%p,p3=%p,p4=%p\n",p,p1,p2,p3,p4);
    p = sbrk(0);//0取当前位置
    printf("堆顶指针:%p\n",p);
    sbrk(-12);//负数代表 释放内存，返回值没有意义
    printf("%d\n",*(int*)p4);
    p = sbrk(0);//0取当前位置
    printf("堆顶指针:%p\n",p);
    p = sbrk(4093);//4093+4 = 4097 超过一个字节,多一页
    printf("p=%p\n",p);
    printf("释放1字节内存\n");
    sbrk(-1);//少了一页
    p = sbrk(0);//0取当前位置
    printf("堆顶指针:%p\n",p);
    printf("全部释放\n");//解除映射
    sbrk(-4096);
    p = sbrk(0);//0取当前位置
    printf("堆顶指针:%p\n",p);
    return 0;
)
