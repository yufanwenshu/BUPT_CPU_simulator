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
	else //�ļ��򿪳ɹ� 
	{
		for(j=0;!feof(fPtr);)
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
			code[j++] = temp; //д������ 
			temp = 0;
		}
		code[j-1] = 0; //���һ���ڴ����д����EOF(��-1),����Ҫ���½����дΪĬ��ֵ0 
	}
}
