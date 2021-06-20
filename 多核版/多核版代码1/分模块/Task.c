#ifndef HIGH_CLOCK
#define HIGH_CLOCK
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "HighDesign.h" 
#endif

void end_output(cpuPtr cpu)
{	
	int i,j,k=0;
	/*最后输出代码段和数据段信息*/ 
	printf("\ncodeSegment :\n");
	for(i=0;i<16;i++) //共输出16行 
	{
		for(j=0;j<7;j++) //每行前7个数后要跟一个空格 
		{
			/*代码段中每条指令存为4个char，通过位运算将其转换为32位整数并输出*/ 
			printf("%d ",((cpu->code[k]<<24)&0xff000000)+((cpu->code[k+1]<<16)&0xff0000)+((cpu->code[k+2]<<8)&0xff00)+(cpu->code[k+3]&0xff));
			k += 4;
		}
		//每行最后一个数后要跟一个换行符 
		printf("%d\n",((cpu->code[k]<<24)&0xff000000)+((cpu->code[k+1]<<16)&0xff0000)+((cpu->code[k+2]<<8)&0xff00)+(cpu->code[k+3]&0xff));
		k += 4;
	}
	
	k=N;
	printf("\ndataSegment :\n");
	for(i=0;i<16;i++) //共输出16行 
	{
		for(j=0;j<15;j++) //每行前15个数后要跟一个空格 
		{
			printf("%d ",bin(cpu,k)); //调用bin()将数据段中相邻两个char转换为一个整数（short）输出 
			k += 2;
		}
		//每行最后一个数后要跟一个换行符 
		printf("%d\n",bin(cpu,k));
		k += 2;
	}
}

DWORD WINAPI process(LPVOID lpParameter)
{
	cpuPtr cpu = (cpuPtr)lpParameter; //参数类型转换 
	int core_id; //记录当前运行核心
	corePtr core = NULL; //当前核心指针 
	
	cpu->hMutex=CreateMutex (NULL, FALSE, "tickets");//创建互斥类型 
	cpu->hMutex_state=CreateMutex (NULL, FALSE, "states");//创建互斥类型 
	
	if(GetCurrentThreadId()==*(cpu->core_1->id)){ //如果当前线程id与核心1的id一致，说明当前函数由线程1执行 
		core_id = 1;
		core = cpu->core_1;//当前核心为核心1 
	}else if(GetCurrentThreadId()==*(cpu->core_2->id)){//如果当前线程id与核心2的id一致，说明当前函数由线程2执行
		core_id = 2;
		core = cpu->core_2;//当前核心为核心2 
	}else{
		printf("线程识别错误\n"); 
	}
	
	/*循环，不断取指令分析执行，直到遇到停机指令*/ 
	do{
		/*代码段中每条指令占四个字节，取出前两个字节并拼接写入ir(指令寄存器)*/
		core->ir = cpu->code[(core->ip)++]&0xff; 
		core->ir = ((core->ir<<8)&0xff00)+(cpu->code[core->ip++]&0xff);
		
		/*取出后两个字节并拼接，即为立即数，赋值给value*/ 
		core->value = cpu->code[core->ip++]&0xff;
		core->value = ((core->value<<8)&0xff00)+(cpu->code[core->ip++]&0xff);
		
		/*指令寄存器储存16位指令，前8位对应操作类型，通过位运算取出赋值给task*/ 
		core->task = core->ir>>8;
		
		/*指令寄存器9-12位、13-16位对应操作位1和操作位2，位运算取出赋值给front、back*/ 
		core->front = ((core->ir<<8)>>12)&0xf;
		core->back = ((core->ir<<12)>>12)&0xf;
		
		/*分析指令操作类型，调用对应函数*/ 
		switch(core->task)
		{
			case 1: task_1(cpu,core);break; //数据传送指令 
			case 2:case 3:case 4:case 5: task_2(cpu,core);break; //算数运算指令 
			case 6:case 7:case 8: task_6(cpu,core);break;        //逻辑运算指令 
			case 9: task_9(cpu,core);break; //比较运算指令 
			case 10: task_10(cpu,core);break; //跳转指令 
			case 11:case 12: 
				WaitForSingleObject(cpu->hMutex_state,INFINITE);//等待正在执行的线程输出完毕
				task_11(cpu,core); //输入输出指令 
				break;
			case 13:case 14:case 15:task_13(cpu,core);break; //多线程操作指令 
		}
		
		if(core->task!=11&&core->task!=12){
			WaitForSingleObject(cpu->hMutex_state,INFINITE);//等待正在执行的线程输出完毕
		} 
		/*每条指令执行完输出各寄存器状态*/ 
		printf("id = %d\n",core_id);
		printf("ip = %d\n",core->ip);
		printf("flag = %d\n",core->flag);
		printf("ir = %d\n",core->ir);
		printf("ax1 = %d ax2 = %d ax3 = %d ax4 = %d\n",core->ax[1],core->ax[2],core->ax[3],core->ax[4]);
		printf("ax5 = %d ax6 = %d ax7 = %d ax8 = %d\n",core->ax[5],core->ax[6],core->ax[7],core->ax[8]);
		ReleaseMutex(cpu->hMutex_state); //释放互斥对象 
	}while(core->task);//task为0则为停机指令，跳出循环 
}

/*数据传送指令，其实就是赋值运算*/ 
void task_1(cpuPtr cpu,corePtr core)
{
	if(core->back==0)
		core->ax[core->front] = core->value; //把立即数赋值给数据寄存器 
	else if(core->back<=4)
		//把数据寄存器中的数赋值给地址寄存器中地址指向的位置，即写入数据段 
		bin_to_char(cpu,core->ax[core->front],core->ax[core->back]); 
	else
		//把地址寄存器中地址指向的位置中存放的数赋值给数据寄存器 
		core->ax[core->front] = bin(cpu,core->ax[core->back]);
}

/*代数运算指令，加减乘除*/ 
void task_2(cpuPtr cpu,corePtr core)
{
	if(core->back){
		//操作位1 （一定是数据寄存器）中的数与操作位2（一定是地址寄存器）指向的数进行运算 
		switch(core->task)
			{
				case 2:core->ax[core->front] += bin(cpu,core->ax[core->back]);break; //加法 
				case 3:core->ax[core->front] -= bin(cpu,core->ax[core->back]);break; //减法 
				case 4:core->ax[core->front] *= bin(cpu,core->ax[core->back]);break; //乘法 
				case 5:core->ax[core->front] /= bin(cpu,core->ax[core->back]);break; //除法				
			}
	}else{
		//操作位1对应的寄存器（一定是数据寄存器）中的数与立即数进行运算
		switch(core->task)
			{
				case 2:core->ax[core->front] += core->value;break; //加法 
				case 3:core->ax[core->front] -= core->value;break; //减法 
				case 4:core->ax[core->front] *= core->value;break; //乘法 
				case 5:core->ax[core->front] /= core->value;break; //除法 
			}
	}	
}

/*逻辑运算，与或非*/ 
void task_6(cpuPtr cpu,corePtr core)
{
	if(core->back==0) //如果操作位2是0，则操作位1（一定是数据寄存器）与立即数运算 
	{
		switch(core->task) 
		{
			case 6: //逻辑与 
				core->ax[core->front] = core->ax[core->front]&&core->value;break;
			case 7: //逻辑或 
				core->ax[core->front] = core->ax[core->front]||core->value;break;
			case 8: //逻辑非 
				core->ax[core->front] = !core->ax[core->front];break;
		}
	}
	else  //操作位2不为0，则操作位1(一定是数据寄存器)中的数与操作位2(一定是地址寄存器)指向的数运算 
	{
		switch(core->task)
		{
			case 6: //逻辑与 
				core->ax[core->front] = core->ax[core->front]&&bin(cpu,core->ax[core->back]);break;
			case 7: //逻辑或 
				core->ax[core->front] = core->ax[core->front]||bin(cpu,core->ax[core->back]);break;
			case 8: //逻辑非 
				bin_to_char(cpu,core->ax[core->back],!bin(cpu,core->ax[core->back]));break;
		}
	}
}

/*比较运算*/ 
void task_9(cpuPtr cpu,corePtr core)
{
	if(core->back==0){ 
		//操作位2为0，则操作位1（一定是数据寄存器）中的数与立即数比较 
		if(core->ax[core->front]>core->value)
			core->flag = 1;
		else if(core->ax[core->front]==core->value)
			core->flag = 0;
		else
			core->flag = -1;	
	}else{ 
		//操作位2不为0，则操作位1（一定是数据寄存器）中的数与操作位2（一定是地址寄存器）指向的数比较 
		if(core->ax[core->front]>bin(cpu,core->ax[core->back]))
			core->flag = 1;
		else if(core->ax[core->front]==bin(cpu,core->ax[core->back]))
			core->flag = 0;
		else
			core->flag = -1;
	}
}

/*跳转指令*/ 
void task_10(cpuPtr cpu,corePtr core)
{
	short temp = core->ip-4;  
	switch(core->back)
	{
		case 0:core->ip = (core->value + temp)&0xffff ;break; //操作位2为0，强制跳转，更新程序计数器ip 
		case 1: //操作位2为1，如果标志寄存器flag为0才跳转 
			if(core->flag==0){
				core->ip = (core->value + temp)&0xffff;
			}break;
		case 2: //操作位2为2，如果标志寄存器flag为1才跳转 
			if(core->flag==1){
				core->ip = (core->value + temp)&0xffff;
			}break;
		case 3:	//操作位2为3，如果标志寄存器flag为-1才跳转 
			if(core->flag==-1){
				core->ip = (core->value + temp)&0xffff;
			}break;
	}
}

/*输入输出指令*/ 
void task_11(cpuPtr cpu,corePtr core)
{
	if(core->task==11) //task为11，对应输入指令 
	{
		printf("in:\n");
		scanf("%hd",&core->ax[core->front]);
	}
	if(core->task==12) //task为12，对应输出指令 
	{
		int core_id;
		if(*(core->id)==*(cpu->core_1->id)){
			core_id = 1;
		}else{
			core_id = 2;
		}
		printf("id = %d    out: ",core_id);
		printf("%d\n",core->ax[core->front]);
	}
}

/*多线程操作*/
void task_13(cpuPtr cpu,corePtr core)
{
	switch(core->task)
	{
		case 13: WaitForSingleObject(cpu->hMutex,INFINITE);break; //锁内存 
		case 14: ReleaseMutex(cpu->hMutex);break; //释放内存（解锁） 
		case 15: Sleep(core->value);break; //睡眠 
			
	}
}

short bin(cpuPtr cpu,int start)
{
	start -= N; //start为N（16384）则对数据段第一个元素data[0]，将start转换为数组下标 
	return ((cpu->data[start]<<8)&0xff00)+(cpu->data[start+1]&0xff); //通过位运算将两个char合并成一个整数（short） 
}

/*将一个整数（short）拆分为两个char并写入数据段*/ 
void bin_to_char(cpuPtr cpu,int start,short number)
{
	start -= N; //start为N（16384）则对数据段第一个元素data[0]，将start转换为数组下标 
	cpu->data[start++] = (char)(number>>8); //强制类型转换，short强制转换为char是低位截断 
	cpu->data[start] = (char)(number);
}
