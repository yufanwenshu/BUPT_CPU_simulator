#ifndef _HEADERNAME_H
#define _HEADERNAME_H
#include <stdio.h>
#include <stdlib.h>
#endif
#include "cpu.h"
#include "variable.h"
void process()
{
	short task,front,back,value; 
	//32λָ�1-8λ��Ӧ�������ͣ�9-12λ��Ӧ����λ1,13-16λ��Ӧ����λ2��17-32λ��Ӧ������
	//task����������ͣ�front�������λ1��back�������λ2��value������������ 
	
	/*ѭ��������ȡָ�����ִ�У�ֱ������ͣ��ָ��*/ 
	do
	{
		/*�������ÿ��ָ��ռ�ĸ��ֽڣ�ȡ��ǰ�����ֽڲ�ƴ��д��ir(ָ��Ĵ���)*/ 
		ir = code[(ip)++]&0xff; 
		ir = ((ir<<8)&0xff00)+(code[ip++]&0xff);
		
		/*ȡ���������ֽڲ�ƴ�ӣ���Ϊ����������ֵ��value*/ 
		value = code[ip++]&0xff;
		value = ((value<<8)&0xff00)+(code[ip++]&0xff);
		
		/*ָ��Ĵ�������16λָ�ǰ8λ��Ӧ�������ͣ�ͨ��λ����ȡ����ֵ��task*/ 
		task = ir>>8;
		
		/*ָ��Ĵ���9-12λ��13-16λ��Ӧ����λ1�Ͳ���λ2��λ����ȡ����ֵ��front��back*/ 
		front = ((ir<<8)>>12)&0xf;
		back = ((ir<<12)>>12)&0xf;
		
		/*����ָ��������ͣ����ö�Ӧ����*/ 
		switch(task)
		{
			case 1: task_1(task,front,back,value);break; //���ݴ���ָ�� 
			case 2:case 3:case 4:case 5: task_2(task,front,back,value);break; //��������ָ�� 
			case 6:case 7:case 8: task_6(task,front,back,value);break;        //�߼�����ָ�� 
			case 9: task_9(front,back,value);break; //�Ƚ�����ָ�� 
			case 10: task_10(back,value);break; //��תָ�� 
			case 11:case 12: task_11(task,front);break; //�������ָ�� 
		}
		
		/*ÿ��ָ��ִ����������Ĵ���״̬*/ 
		printf("ip = %d\n",ip);
		printf("flag = %d\n",flag);
		printf("ir = %d\n",ir);
		printf("ax1 = %d ax2 = %d ax3 = %d ax4 = %d\n",ax[1],ax[2],ax[3],ax[4]);
		printf("ax5 = %d ax6 = %d ax7 = %d ax8 = %d\n",ax[5],ax[6],ax[7],ax[8]);
	}while(task); //taskΪ0��Ϊͣ��ָ�����ѭ�� 
	
}

/*���ݴ���ָ���ʵ���Ǹ�ֵ����*/ 
void task_1(short task,short front,short back,short value)
{
	switch(back)//���ݲ���λ2�����ͷ���� 
	{
		case 0:ax[front] = value;break; //����������ֵ�����ݼĴ��� 
		
		case 1:case 2:case 3:case 4:   //�����ݼĴ����е�����ֵ����ַ�Ĵ����е�ַָ���λ�ã���д�����ݶ� 
			bin_to_char(ax[front],ax[back]);break;
		case 5:case 6:case 7:case 8:
			ax[front] = bin(ax[back]);break; //�ѵ�ַ�Ĵ����е�ַָ���λ���д�ŵ�����ֵ�����ݼĴ��� 
	}
}

/*��������ָ��Ӽ��˳�*/ 
void task_2(short task,short front,short back,short value)
{
	switch(back) //���ݲ���λ2�����ͷ���� 
	{
		case 0:  //����λ1��Ӧ�ļĴ�����һ�������ݼĴ������е������������������� 
			switch(task)
			{
				case 2:ax[front] += value;break; //�ӷ� 
				case 3:ax[front] -= value;break; //���� 
				case 4:ax[front] *= value;break; //�˷� 
				case 5:ax[front] /= value;break; //���� 
			}break;
		default: //����λ1 ��һ�������ݼĴ������е��������λ2��һ���ǵ�ַ�Ĵ�����ָ������������� 
			switch(task)
			{
				case 2:ax[front] += bin(ax[back]);break; //�ӷ� 
				case 3:ax[front] -= bin(ax[back]);break; //���� 
				case 4:ax[front] *= bin(ax[back]);break; //�˷� 
				case 5:ax[front] /= bin(ax[back]);break; //����				
			}		
	}
	
}

/*�߼����㣬����*/ 
void task_6(short task,short front,short back,short value)
{
	if(!back) //�������λ2��0�������λ1��һ�������ݼĴ����������������� 
	{
		switch(task) 
		{
			case 6: //�߼��� 
				ax[front] = ax[front]&&value;break;
			case 7: //�߼��� 
				ax[front] = ax[front]||value;break;
			case 8: //�߼��� 
				ax[front] = !ax[front];break;
		}
	}
	else  //����λ2��Ϊ0�������λ1(һ�������ݼĴ���)�е��������λ2(һ���ǵ�ַ�Ĵ���)ָ��������� 
	{
		switch(task)
		{
			case 6: //�߼��� 
				ax[front] = ax[front]&&bin(ax[back]);break;
			case 7: //�߼��� 
				ax[front] = ax[front]||bin(ax[back]);break;
			case 8: //�߼��� 
				bin_to_char(ax[back],!bin(ax[back]));break;
		}
	}
}

/*�Ƚ�����*/ 
void task_9(short front,short back,short value)
{
	switch(back)
	{
		case 0: //����λ2Ϊ0�������λ1��һ�������ݼĴ������е������������Ƚ� 
			if(ax[front]>value)
				flag = 1;
			else if(ax[front]==value)
				flag = 0;
			else
				flag = -1;
			break;
		default: //����λ2��Ϊ0�������λ1��һ�������ݼĴ������е��������λ2��һ���ǵ�ַ�Ĵ�����ָ������Ƚ� 
			if(ax[front]>bin(ax[back]))
				flag = 1;
			else if(ax[front]==bin(ax[back]))
				flag = 0;
			else
				flag = -1;
	}
}

/*��תָ��*/ 
void task_10(short back,short value)
{
	short temp = ip-4;  
	switch(back)
	{
		case 0:ip = (value + temp)&0xffff ;break; //����λ2Ϊ0��ǿ����ת�����³��������ip 
		case 1: //����λ2Ϊ1�������־�Ĵ���flagΪ0����ת 
			if(flag==0){
				ip = (value + temp)&0xffff;
			}break;
		case 2: //����λ2Ϊ2�������־�Ĵ���flagΪ1����ת 
			if(flag==1){
				ip = (value + temp)&0xffff;
			}break;
		case 3:	//����λ2Ϊ3�������־�Ĵ���flagΪ-1����ת 
			if(flag==-1){
				ip = (value + temp)&0xffff;
			}break;
	}
}

/*�������ָ��*/ 
void task_11(short task,short front)
{
	if(task==11) //taskΪ11����Ӧ����ָ�� 
	{
		printf("in:\n");
		scanf("%hd",&ax[front]);
	}
	if(task==12) //taskΪ12����Ӧ���ָ�� 
	{
		printf("out: ");
		printf("%d\n",ax[front]);
	}
}

/*����ָ��ִ�����������������ݶ���Ϣ*/ 
void end_output(void)
{
	int i,j,k=0;
	printf("\ncodeSegment :\n");
	for(i=0;i<16;i++) //�����16�� 
	{
		for(j=0;j<7;j++) //ÿ��ǰ7������Ҫ��һ���ո� 
		{
			/*�������ÿ��ָ���Ϊ4��char��ͨ��λ���㽫��ת��Ϊ32λ���������*/ 
			printf("%d ",((code[k]<<24)&0xff000000)+((code[k+1]<<16)&0xff0000)+((code[k+2]<<8)&0xff00)+(code[k+3]&0xff));
			k += 4;
		}
		//ÿ�����һ������Ҫ��һ�����з� 
		printf("%d\n",((code[k]<<24)&0xff000000)+((code[k+1]<<16)&0xff0000)+((code[k+2]<<8)&0xff00)+(code[k+3]&0xff));
		k += 4;
	}
	
	k=N;
	printf("\ndataSegment :\n");
	for(i=0;i<16;i++) //�����16�� 
	{
		for(j=0;j<15;j++) //ÿ��ǰ15������Ҫ��һ���ո� 
		{
			printf("%d ",bin(k)); //����bin()�����ݶ�����������charת��Ϊһ��������short����� 
			k += 2;
		}
		//ÿ�����һ������Ҫ��һ�����з� 
		printf("%d\n",bin(k));
		k += 2;
	}
}

/*�����ݶ�����������char�ϲ�Ϊһ������*/ 
short bin(int start)
{
	start -= N; //startΪN��16384��������ݶε�һ��Ԫ��data[0]����startת��Ϊ�����±� 
	return ((data[start]<<8)&0xff00)+(data[start+1]&0xff); //ͨ��λ���㽫����char�ϲ���һ��������short�� 
}

/*��һ��������short�����Ϊ����char��д�����ݶ�*/ 
void bin_to_char(int start,short number)
{
	start -= N; //startΪN��16384��������ݶε�һ��Ԫ��data[0]����startת��Ϊ�����±� 
	data[start++] = (char)(number>>8); //ǿ������ת����shortǿ��ת��Ϊchar�ǵ�λ�ض� 
	data[start] = (char)(number);
}
