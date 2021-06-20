#ifndef HIGH_CLOCK
#define HIGH_CLOCK
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "HighDesign.h" 
#endif

/*cpu初始化*/ 
void init_cpu(cpuPtr cpu)
{
	corePtr core_1 = (corePtr)malloc(sizeof(struct core));  
	corePtr core_2 = (corePtr)malloc(sizeof(struct core));
	init_core(core_1); //初始化核心1 
	init_core(core_2); //初始化核心2 
	
	int i;
	/*代码段和数据段初始化*/ 
	for(i=0;i<Max;i++)
	{
		cpu->code[i] = 0;
		cpu->data[i] = 0;
	}
	cpu->core_1 = core_1;
	cpu->core_2 = core_2;
	
	bin_to_char(cpu,16384,100); //规定票数100存在数据段头 
	cpu->core_2->ip = START_2;//核心2 ip初始化为256 
	
}

/*core初始化*/ 
void init_core(corePtr core)
{
	int i;
	DWORD *id = (DWORD*)malloc(sizeof(DWORD)); //标识线程id 
	*id = 0;  
	for(i=0;i<9;i++)
	{
		core->ax[i] = 0;
	}
	core->ip = 0;
	core->ir = 0;
	core->flag = 0;
	core->task = 0;
	core->front = 0;
	core->back = 0;
	core->value = 0;
	core->id = id; 
}

/*指令加载*/ 
void load(FILE *fPtr,cpuPtr cpu,int start)
{
	char ch;
	int i,j;
	int temp;
	for(j=start;!feof(fPtr);)
		{
			for(i=0;i<8;i++)      //因为代码段code为char型数组
			{                     //所以一条指令需要占用四个数组空间
				ch = fgetc(fPtr); //每次读取8位二进制值并将其转换为char存入数组
				if(ch=='\n')
				{                 //如果读取到换行符，则将其丢弃重新读取 
					i--;
					continue;
				}
				temp = temp*2 + ch -'0'; //每次从文件中读到的其实是'1'或'0'对应的ASCII码，要将其转换为数值类型 
			}
			cpu->code[j++] = temp; //写入代码段 
			temp = 0;
		}
		cpu->code[j-1] = 0; //最后一次在代码段写入了EOF(即-1),所以要重新将其改写为默认值0 
}
