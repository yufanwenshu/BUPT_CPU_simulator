#ifndef _HEADERNAME_H
#define _HEADERNAME_H
#include <stdio.h>
#include <stdlib.h>
#endif

extern char code[]; 
void read(void)
{
	char ch;
	int i,j;
	int temp;
	FILE *fPtr=NULL;
	fPtr = fopen("dict.txt","r");
	if(!fPtr)
	{
		printf("File open error!\n");
	}
	else //文件打开成功 
	{
		for(j=0;!feof(fPtr);)
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
			code[j++] = temp; //写入代码段 
			temp = 0;
		}
		code[j-1] = 0; //最后一次在代码段写入了EOF(即-1),所以要重新将其改写为默认值0 
	}
}
