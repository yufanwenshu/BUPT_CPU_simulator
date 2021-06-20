#ifndef HIGH_CLOCK
#define HIGH_CLOCK
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "HighDesign.h" 
#endif
int main()
{
	HANDLE thread1,thread2;//�߳̾�� 
 
	struct _cpu cpu_temp; //�½�һ���ṹ�� 
	cpuPtr cpu = &cpu_temp;
	FILE *fPtr_1=fopen("dict1.txt","r");
	FILE *fPtr_2=fopen("dict2.txt","r");
	if(fPtr_1==NULL||fPtr_2==NULL)
		printf("error!");
	
	init_cpu(cpu); //��ʼ��cpu 
	load(fPtr_1,cpu,START_1); //����1ָ����� 
	load(fPtr_2,cpu,START_2); //����2ָ����� 
	/*���������߳�*/ 
	thread1 = CreateThread(NULL,0,process1,cpu,0,NULL);
	thread2 = CreateThread(NULL,0,process2,cpu,0,NULL);
	WaitForSingleObject(thread1, INFINITE);
	WaitForSingleObject(thread2, INFINITE);
	CloseHandle(thread1);
	CloseHandle(thread2);
	
	end_output(cpu);//�������κ����ݶ���Ϣ 
	return 0;
}
