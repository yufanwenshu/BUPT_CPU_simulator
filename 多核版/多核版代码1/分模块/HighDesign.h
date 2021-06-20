#ifndef HIGH_CLOCK
#define HIGH_CLOCK
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "HighDesign.h" 
#endif
#define Max 16384 //���ݶκʹ���εĳ��� 
#define N 16384   //���ݶε��±�ƫ�� 
#define START_1 0 //����1�Ĵ�����ʼλ�� 
#define START_2 256 //����2�Ĵ�����ʼλ�� 
#define TICKETS 100 //��Ҫ������Ʊ�� 

/*ģ����ĵĽṹ��*/ 
struct core{
	short ax[9]; //ģ��ͨ�üĴ��� 
	short ip; //ģ���������� 
	short ir; //ģ��ָ��Ĵ��� 
	short flag; //ģ���־�Ĵ��� 
	short task; //1-8λ,��ʾ�������� 
	short front;//9-12λ,��ʾ����λ 1 
	short back; //13-16λ,��ʾ����λ 1 
	short value;//17-32λ,��ʾ������ 
	DWORD *id; //��¼�߳�id 
};
typedef struct core* corePtr;

/*ģ��cpu�Ľṹ��*/ 
struct _cpu{
	char code[Max]; //ģ������ 
	char data[Max]; //ģ�����ݶ� 
	corePtr core_1;
	corePtr core_2;
	HANDLE hMutex;//��������� 
	HANDLE hMutex_state;//�������������
};
typedef struct _cpu* cpuPtr;

void init_cpu(cpuPtr cpu); //��ʼ��cpu 
void init_core(corePtr core); //��ʼ��core 
void load(FILE *fPtr,cpuPtr cpu,int start); //ָ����� 
void end_output(cpuPtr cpu); //�������κ����ݶ���Ϣ 
DWORD WINAPI process(LPVOID lpParameter); //�̺߳��� 
void task_1(cpuPtr cpu,corePtr core); //���ݴ���ָ�� 
void task_2(cpuPtr cpu,corePtr core); //��������ָ�� 
void task_6(cpuPtr cpu,corePtr core); //�߼�����ָ��
void task_9(cpuPtr cpu,corePtr core); //�Ƚ�����ָ�� 
void task_10(cpuPtr cpu,corePtr core);//��תָ�� 
void task_11(cpuPtr cpu,corePtr core);//�������ָ�� 
void task_13(cpuPtr cpu,corePtr core);//��˰�ָ�� 
short bin(cpuPtr cpu,int start);//�����ݶζ�ȡ��Ϣ������char����ת��Ϊ16λ������������short�ͣ�
void bin_to_char(cpuPtr cpu,int start,short number);//��16λ����������(short��)ת��Ϊ����char���������ݶ� 
