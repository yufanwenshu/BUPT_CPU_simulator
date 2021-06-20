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
	corePtr core_1 = (corePtr)malloc(sizeof(struct core));  
	corePtr core_2 = (corePtr)malloc(sizeof(struct core));
	init_core(core_1); //��ʼ������1 
	init_core(core_2); //��ʼ������2 
	
	int i;
	/*����κ����ݶγ�ʼ��*/ 
	for(i=0;i<Max;i++)
	{
		cpu->code[i] = 0;
		cpu->data[i] = 0;
	}
	cpu->core_1 = core_1;
	cpu->core_2 = core_2;
	
	bin_to_char(cpu,16384,100); //�涨Ʊ��100�������ݶ�ͷ 
	cpu->core_2->ip = START_2;//����2 ip��ʼ��Ϊ256 
	
}

/*core��ʼ��*/ 
void init_core(corePtr core)
{
	int i;
	DWORD *id = (DWORD*)malloc(sizeof(DWORD)); //��ʶ�߳�id 
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
			cpu->code[j++] = temp; //д������ 
			temp = 0;
		}
		cpu->code[j-1] = 0; //���һ���ڴ����д����EOF(��-1),����Ҫ���½����дΪĬ��ֵ0 
}
