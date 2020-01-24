#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <queue>
#include <process.h>
using namespace std;

DWORD WINAPI Consumer(void*);//���������ߺ���
DWORD WINAPI Producer(void*);//���������ߺ���

#define N 32	//���建��������

/*���ݽṹ�Ķ���*/
struct MyData{
	HANDLE m_S_Empty;//������Semaphore
	HANDLE m_S_Full;//������Semaphore
	HANDLE m_M_Mutex;//�����ź���
	queue<int> food;//������������	
	bool producerfinished;//��־���������Ƿ��������
};

int j=0;//�߳�ִ�д���


/*������*/
DWORD WINAPI Producer(void* lp){   	
	MyData * md = (MyData*)lp;
	
	for(int i =0 ; i < 100; i++){
		
		//���Ի������Ƿ��пռ�
		WaitForSingleObject(md->m_S_Empty, INFINITE);
		
		/*������ʻ�����*/
		WaitForSingleObject(md->m_M_Mutex, INFINITE);		
		md->food.push(1);	//���뻺����
		printf("%d\t����������1����Ʒ������������%d����Ʒ\t�߳�ID = %d\n",j++,md->food.size(),GetCurrentThreadId());	
		ReleaseMutex(md->m_M_Mutex);	//�ͷŻ����ź���	
		
		//��ָ���źŶ���(�в�Ʒ�Ļ�����)�ļ�����һ
		ReleaseSemaphore(md->m_S_Full, 1, NULL);	
	}
	md->producerfinished=true;	//�����߽��������������������߳̽���
	return 0;
}


/*������*/
DWORD WINAPI Consumer(void* lp){   
	MyData * md = (MyData*)lp;

	//��������û�н�������
	while(!md->producerfinished){
		
		//���Ի������Ƿ��в�Ʒ
		WaitForSingleObject(md->m_S_Full,INFINITE);	

		/*������ʻ�����*/
		WaitForSingleObject(md->m_M_Mutex,INFINITE);	
		md->food.pop();	//����һ����Ʒ
		printf("%d\t����������1����Ʒ����������ʣ%d����Ʒ\t�߳�ID = %d\n",j++,md->food.size(),GetCurrentThreadId());
		ReleaseMutex(md->m_M_Mutex); //�ͷŻ����ź���

		//��ָ���źŶ���(�յĻ�����)�ļ�����һ
		ReleaseSemaphore(md->m_S_Empty,1,NULL);	
	}
	return 0;
}


int main(){

	/*�Ը����ź�����ֵ*/
	MyData mydata;	//����һ��MyData�������͵�ʵ��mydata
	mydata.m_M_Mutex = CreateMutex(NULL,false, NULL);	//���������Ļ������
	
	/*����һ�������źŶ��󣬳�ʼ����ΪN��
	����ɹ��ʹ���һ��handle�����򴫻�NULL*/
	mydata.m_S_Empty = CreateSemaphore(NULL, N, N, NULL);
	mydata.m_S_Full = CreateSemaphore(NULL, 0, N, NULL);	//��ʼ����Ϊ0
	mydata.producerfinished=false;//�����߽�����־�տ�ʼ����Ϊfalse,��ʾû�н���
	
	//���������ߺ��������߳�	
	HANDLE procedure_handles[100];
	HANDLE customer_handles[80];
	for(int i=0;i<100;i++)
		procedure_handles[i] = CreateThread(NULL,0,&Producer,(void*)&mydata,0,0);
	for(int j=0;j<80;j++)
		customer_handles[j] = CreateThread(NULL,0,&Consumer,(void*)&mydata,0,0);	

	//�ȴ������̶߳�����������ִ��
	WaitForMultipleObjects(2, procedure_handles, true, INFINITE);
	WaitForMultipleObjects(2, customer_handles, true, INFINITE);	
	CloseHandle(mydata.m_M_Mutex);	
	CloseHandle(mydata.m_S_Full);	
	CloseHandle(mydata.m_S_Empty);

	return 0;
}
