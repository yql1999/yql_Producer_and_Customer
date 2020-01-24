#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <queue>
#include <process.h>
using namespace std;

DWORD WINAPI Consumer(void*);//声明消费者函数
DWORD WINAPI Producer(void*);//声明生产者函数

#define N 32	//定义缓冲区数量

/*数据结构的定义*/
struct MyData{
	HANDLE m_S_Empty;//生产者Semaphore
	HANDLE m_S_Full;//消费者Semaphore
	HANDLE m_M_Mutex;//互斥信号量
	queue<int> food;//共享缓冲区队列	
	bool producerfinished;//标志着生产者是否结束生产
};

int j=0;//线程执行次数


/*生产者*/
DWORD WINAPI Producer(void* lp){   	
	MyData * md = (MyData*)lp;
	
	for(int i =0 ; i < 100; i++){
		
		//测试缓冲区是否有空间
		WaitForSingleObject(md->m_S_Empty, INFINITE);
		
		/*互斥访问缓冲区*/
		WaitForSingleObject(md->m_M_Mutex, INFINITE);		
		md->food.push(1);	//放入缓冲区
		printf("%d\t生产者生产1个产品，缓冲区共有%d个产品\t线程ID = %d\n",j++,md->food.size(),GetCurrentThreadId());	
		ReleaseMutex(md->m_M_Mutex);	//释放互斥信号量	
		
		//将指定信号对象(有产品的缓冲区)的计数加一
		ReleaseSemaphore(md->m_S_Full, 1, NULL);	
	}
	md->producerfinished=true;	//生产者结束生产，控制消费者线程结束
	return 0;
}


/*消费者*/
DWORD WINAPI Consumer(void* lp){   
	MyData * md = (MyData*)lp;

	//若生产者没有结束生产
	while(!md->producerfinished){
		
		//测试缓冲区是否有产品
		WaitForSingleObject(md->m_S_Full,INFINITE);	

		/*互斥访问缓冲区*/
		WaitForSingleObject(md->m_M_Mutex,INFINITE);	
		md->food.pop();	//消费一个产品
		printf("%d\t消费者消费1个产品，缓冲区还剩%d个产品\t线程ID = %d\n",j++,md->food.size(),GetCurrentThreadId());
		ReleaseMutex(md->m_M_Mutex); //释放互斥信号量

		//将指定信号对象(空的缓冲区)的计数加一
		ReleaseSemaphore(md->m_S_Empty,1,NULL);	
	}
	return 0;
}


int main(){

	/*对各个信号量赋值*/
	MyData mydata;	//创建一个MyData数据类型的实体mydata
	mydata.m_M_Mutex = CreateMutex(NULL,false, NULL);	//创建无名的互斥对象
	
	/*创建一个无名信号对象，初始计数为N，
	如果成功就传回一个handle，否则传回NULL*/
	mydata.m_S_Empty = CreateSemaphore(NULL, N, N, NULL);
	mydata.m_S_Full = CreateSemaphore(NULL, 0, N, NULL);	//初始计数为0
	mydata.producerfinished=false;//生产者结束标志刚开始设置为false,表示没有结束
	
	//创建生产者和消费者线程	
	HANDLE procedure_handles[100];
	HANDLE customer_handles[80];
	for(int i=0;i<100;i++)
		procedure_handles[i] = CreateThread(NULL,0,&Producer,(void*)&mydata,0,0);
	for(int j=0;j<80;j++)
		customer_handles[j] = CreateThread(NULL,0,&Consumer,(void*)&mydata,0,0);	

	//等待所有线程都结束才往下执行
	WaitForMultipleObjects(2, procedure_handles, true, INFINITE);
	WaitForMultipleObjects(2, customer_handles, true, INFINITE);	
	CloseHandle(mydata.m_M_Mutex);	
	CloseHandle(mydata.m_S_Full);	
	CloseHandle(mydata.m_S_Empty);

	return 0;
}
