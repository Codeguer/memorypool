CSDN首页

消息

高并发内存池详解
8 / 100

封面&摘要：
*

待填

3/256
文章标签：
*
分类专栏：
C++精华
新建分类专栏
最多选择3个分类专栏#为二级分类
文章类型：
申请原创将启用 Creative Commons 版权模板，如果不是原创文章，请选择转载或翻译。 原创文章默认开启打赏， 打赏管理
发布形式：
所有用户将均可访问和阅读，优质文章将会获得更高的推荐曝光

内容等级：
*
共 1801 字
发布文章
topSpInfo
发文助手
1





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
