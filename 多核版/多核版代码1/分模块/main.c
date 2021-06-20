#ifndef HIGH_CLOCK
#define HIGH_CLOCK
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "HighDesign.h" 
#endif
int main()
{
	HANDLE thread1,thread2;//线程句柄 
 
	cpuPtr cpu = (cpuPtr)malloc(sizeof(struct _cpu));
	FILE *fPtr_1=fopen("dict1.txt","r");
	FILE *fPtr_2=fopen("dict2.txt","r");
	if(fPtr_1==NULL||fPtr_2==NULL)
		printf("error!");
	
	init_cpu(cpu); //初始化cpu 
	load(fPtr_1,cpu,START_1); //核心1指令加载 
	load(fPtr_2,cpu,START_2); //核心2指令加载 
	/*创建两个线程*/ 
	thread1 = CreateThread(NULL,0,process,cpu,0,cpu->core_1->id);
	thread2 = CreateThread(NULL,0,process,cpu,0,cpu->core_2->id);
	WaitForSingleObject(thread1, INFINITE);
	WaitForSingleObject(thread2, INFINITE);
	CloseHandle(thread1);
	CloseHandle(thread2);
	
	end_output(cpu);//输出代码段和数据段信息 
	return 0;
}
