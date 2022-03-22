#include<iostream>
#include<thread>

using namespace std;
__thread int iVar = 100;//每个线程独有的一个全局变量

void Thread1(){
    iVar += 200;
    cout<<"Thead1 Val : "<<iVar<<endl;
}

void Thread2(){
    iVar += 400;
    cout<<"Thead2 Val : "<<iVar<<endl;
}

int main(){
    thread t1(Thread1);
    thread t2(Thread2);

    t1.join();
    t2.join();
    return 0;
}

