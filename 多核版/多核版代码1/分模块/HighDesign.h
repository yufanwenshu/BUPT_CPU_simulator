#ifndef HIGH_CLOCK
#define HIGH_CLOCK
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "HighDesign.h" 
#endif
#define Max 16384 //数据段和代码段的长度 
#define N 16384   //数据段的下标偏移 
#define START_1 0 //核心1的代码起始位置 
#define START_2 256 //核心2的代码起始位置 
#define TICKETS 100 //需要卖出的票数 

/*模拟核心的结构体*/ 
struct core{
	short ax[9]; //模拟通用寄存器 
	short ip; //模拟程序计数器 
	short ir; //模拟指令寄存器 
	short flag; //模拟标志寄存器 
	short task; //1-8位,表示操作类型 
	short front;//9-12位,表示操作位 1 
	short back; //13-16位,表示操作位 1 
	short value;//17-32位,表示立即数 
	DWORD *id; //记录线程id 
};
typedef struct core* corePtr;

/*模拟cpu的结构体*/ 
struct _cpu{
	char code[Max]; //模拟代码段 
	char data[Max]; //模拟数据段 
	corePtr core_1;
	corePtr core_2;
	HANDLE hMutex;//互斥对象句柄 
	HANDLE hMutex_state;//避免输出不完整
};
typedef struct _cpu* cpuPtr;

void init_cpu(cpuPtr cpu); //初始化cpu 
void init_core(corePtr core); //初始化core 
void load(FILE *fPtr,cpuPtr cpu,int start); //指令加载 
void end_output(cpuPtr cpu); //输出代码段和数据段信息 
DWORD WINAPI process(LPVOID lpParameter); //线程函数 
void task_1(cpuPtr cpu,corePtr core); //数据传送指令 
void task_2(cpuPtr cpu,corePtr core); //算术运算指令 
void task_6(cpuPtr cpu,corePtr core); //逻辑运算指令
void task_9(cpuPtr cpu,corePtr core); //比较运算指令 
void task_10(cpuPtr cpu,corePtr core);//跳转指令 
void task_11(cpuPtr cpu,corePtr core);//输入输出指令 
void task_13(cpuPtr cpu,corePtr core);//多核版指令 
short bin(cpuPtr cpu,int start);//从数据段读取信息（两个char）并转换为16位二进制整数（short型）
void bin_to_char(cpuPtr cpu,int start,short number);//将16位二进制整数(short型)转化为两个char并存入数据段 
