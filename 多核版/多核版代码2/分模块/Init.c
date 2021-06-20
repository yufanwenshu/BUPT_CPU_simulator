#ifndef HIGH_CLOCK
#define HIGH_CLOCK
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "HighDesign.h" 
#endif

/*cpu��ʼ��*/ 
void init_cpu(cpuPtr cpu)
{
	int i;
	/*����κ����ݶγ�ʼ��*/ 
	for(i=0;i<Max;i++)
	{
		cpu->code_data[i] = 0;
	}
	for(i=1;i<=12;i++)
	{
		cpu->core1_ax[i] = 0;
		cpu->core2_ax[i] = 0;
	}
	
	cpu->core2_ax[9] = START_2; //����2�ĳ����������ʼʱΪ256 
	
	bin_to_char(cpu,16384,100);  //�涨Ʊ��100�������ݶ�ͷ 
	
	cpu->hMutex = CreateMutex (NULL, FALSE, "tickets");//����������� 
	cpu->hMutex_print = CreateMutex (NULL, FALSE, "print");//����������� 
}

/*ָ�����*/ 
void load(FILE *fPtr,cpuPtr cpu,int start)
{
	char ch;
	int i,j;
	int temp;
	for(j=start;!feof(fPtr);)
		{
			for(i=0;i<8;i++)      //��Ϊ�����codeΪchar������
			{                     //����һ��ָ����Ҫռ���ĸ�����ռ�
				ch = fgetc(fPtr); //ÿ�ζ�ȡ8λ������ֵ������ת��Ϊchar��������
				if(ch=='\n')
				{                 //�����ȡ�����з������䶪�����¶�ȡ 
					i--;
					continue;
				}
				temp = temp*2 + ch -'0'; //ÿ�δ��ļ��ж�������ʵ��'1'��'0'��Ӧ��ASCII�룬Ҫ����ת��Ϊ��ֵ���� 
			}
			cpu->code_data[j++] = temp; //д������ 
			temp = 0;
		}
		cpu->code_data[j-1] = 0; //���һ���ڴ����д����EOF(��-1),����Ҫ���½����дΪĬ��ֵ0 
}
