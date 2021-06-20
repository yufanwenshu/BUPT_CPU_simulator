#ifndef HIGH_CLOCK
#define HIGH_CLOCK
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#endif

#define Max (16384*2) //代码段与数据段的总长度 
#define N 16384 //数据段的下标偏移 
#define START_1 0 //核心1的代码起始位置 
#define START_2 256 //核心2的代码起始位置 
#define TICKETS 100 //需要卖出的票数 

#define chooce_ax(id) 	short* ax=NULL;if(id==1){ax = cpu->core1_ax;}else if(id==2){ax = cpu->core2_ax;}

/*模拟cpu的结构体*/ 
struct _cpu{
	char code_data[Max]; //模拟代码段和数据段  
	short core1_ax[12]; //模拟核心1的寄存器 ，0闲置，1-8通用寄存器，9程序计数器（ip)，10指令寄存器(ir)，11标志寄存器 (flag)
	short core2_ax[12]; //模拟核心2的寄存器 ，0闲置，1-8通用寄存器，9程序计数器（ip)，10指令寄存器(ir)，11标志寄存器 (flag)
	HANDLE hMutex;//互斥对象句柄，用于执行多核版指令 
	HANDLE hMutex_print;//互斥对象句柄，用于避免输出被打断 
};
typedef struct _cpu* cpuPtr;

/*函数声明*/ 
void init_cpu(cpuPtr cpu); //初始化cpu 
void load(FILE *fPtr,cpuPtr cpu,int start); //指令加载 
DWORD WINAPI process1(LPVOID lpParameter);  //核心1的线程函数 
DWORD WINAPI process2(LPVOID lpParameter);  //核心2的线程函数 
void output(cpuPtr cpu,int id); //输出当前寄存器信息 
void end_output(cpuPtr cpu); //输出代码段与数据段信息 
short bin(cpuPtr cpu,int start); //通过位运算将两个char合并成一个整数（short）
void bin_to_char(cpuPtr cpu,int start,short number); //将一个整数（short）拆分为两个char并写入数据段
void transfer(cpuPtr cpu,short ir[],int id); //数据传送指令 
void calculate(cpuPtr cpu,short ir[],int id);//算术运算指令 
void logic(cpuPtr cpu,short ir[],int id); //逻辑运算指令 
void comparation(cpuPtr cpu,short ir[],int id); //比较指令 
void skip(cpuPtr cpu,short ir[],int id); //跳转指令 
void ioput(cpuPtr cpu,short ir[],int id); //输入输出指令 
void mul(cpuPtr cpu,short ir[],int id); //多核版指令 
