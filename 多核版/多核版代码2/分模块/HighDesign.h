#ifndef HIGH_CLOCK
#define HIGH_CLOCK
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#endif

#define Max (16384*2) //����������ݶε��ܳ��� 
#define N 16384 //���ݶε��±�ƫ�� 
#define START_1 0 //����1�Ĵ�����ʼλ�� 
#define START_2 256 //����2�Ĵ�����ʼλ�� 
#define TICKETS 100 //��Ҫ������Ʊ�� 

#define chooce_ax(id) 	short* ax=NULL;if(id==1){ax = cpu->core1_ax;}else if(id==2){ax = cpu->core2_ax;}

/*ģ��cpu�Ľṹ��*/ 
struct _cpu{
	char code_data[Max]; //ģ�����κ����ݶ�  
	short core1_ax[12]; //ģ�����1�ļĴ��� ��0���ã�1-8ͨ�üĴ�����9�����������ip)��10ָ��Ĵ���(ir)��11��־�Ĵ��� (flag)
	short core2_ax[12]; //ģ�����2�ļĴ��� ��0���ã�1-8ͨ�üĴ�����9�����������ip)��10ָ��Ĵ���(ir)��11��־�Ĵ��� (flag)
	HANDLE hMutex;//���������������ִ�ж�˰�ָ�� 
	HANDLE hMutex_print;//��������������ڱ����������� 
};
typedef struct _cpu* cpuPtr;

/*��������*/ 
void init_cpu(cpuPtr cpu); //��ʼ��cpu 
void load(FILE *fPtr,cpuPtr cpu,int start); //ָ����� 
DWORD WINAPI process1(LPVOID lpParameter);  //����1���̺߳��� 
DWORD WINAPI process2(LPVOID lpParameter);  //����2���̺߳��� 
void output(cpuPtr cpu,int id); //�����ǰ�Ĵ�����Ϣ 
void end_output(cpuPtr cpu); //�������������ݶ���Ϣ 
short bin(cpuPtr cpu,int start); //ͨ��λ���㽫����char�ϲ���һ��������short��
void bin_to_char(cpuPtr cpu,int start,short number); //��һ��������short�����Ϊ����char��д�����ݶ�
void transfer(cpuPtr cpu,short ir[],int id); //���ݴ���ָ�� 
void calculate(cpuPtr cpu,short ir[],int id);//��������ָ�� 
void logic(cpuPtr cpu,short ir[],int id); //�߼�����ָ�� 
void comparation(cpuPtr cpu,short ir[],int id); //�Ƚ�ָ�� 
void skip(cpuPtr cpu,short ir[],int id); //��תָ�� 
void ioput(cpuPtr cpu,short ir[],int id); //�������ָ�� 
void mul(cpuPtr cpu,short ir[],int id); //��˰�ָ�� 
