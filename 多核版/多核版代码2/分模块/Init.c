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
	int i;
	/*代码段和数据段初始化*/ 
	for(i=0;i<Max;i++)
	{
		cpu->code_data[i] = 0;
	}
	for(i=1;i<=12;i++)
	{
		cpu->core1_ax[i] = 0;
		cpu->core2_ax[i] = 0;
	}
	
	cpu->core2_ax[9] = START_2; //核心2的程序计数器初始时为256 
	
	bin_to_char(cpu,16384,100);  //规定票数100存在数据段头 
	
	cpu->hMutex = CreateMutex (NULL, FALSE, "tickets");//创建互斥对象 
	cpu->hMutex_print = CreateMutex (NULL, FALSE, "print");//创建互斥对象 
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
			cpu->code_data[j++] = temp; //写入代码段 
			temp = 0;
		}
		cpu->code_data[j-1] = 0; //最后一次在代码段写入了EOF(即-1),所以要重新将其改写为默认值0 
}
