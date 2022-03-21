#include <stdio.h>
#include <unistd.h>
#include <string.h>
 
int main(){ 
  char* p = (char*)sbrk(0);//先取初始位置，以后才能brk
  int r = brk(p+4);//分配了4个字节
  if(r == -1){ perror("brk"); return -1; }
  brk(p+8);//重新分配了4个字节
  brk(p+4);//回收了4个字节
  int* pi = (int*)p;//从初始位置开始
  *pi = 100;
  //重新分配10个字节，放字符串 "abcde"
  char* s = (char*)sbrk(0);
  brk(s+10);//分配10字节
  //s = "abcde";//有问题，指向了只读区，堆区内存泄漏
  strcpy(s,"abcde");
  double* pd = (double*)sbrk(0);//取首地址
  brk(pd+1);//pd+1 移动了8个字节
  *pd = 1.0;
  printf("%d,%s,%lf\n",*pi,s,*pd);
  brk(pd);//释放double的空间
  brk(s);
  brk(pi);//全部释放
}
