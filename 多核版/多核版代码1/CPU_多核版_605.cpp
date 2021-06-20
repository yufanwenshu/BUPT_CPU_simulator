#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
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


int main()
{
	HANDLE thread1,thread2;//线程句柄 
 
	cpuPtr cpu = (cpuPtr)malloc(sizeof(struct _cpu));
	FILE *fPtr_1=fopen("dict1.txt","r");
	FILE *fPtr_2=fopen("dict2.txt","r");
	if(fPtr_1==NULL||fPtr_2==NULL)
		printf("error!");
	
	init_cpu(cpu); //初始化cpu 
	load(fPtr_1,cpu,START_1); //核心1指令加载 
	load(fPtr_2,cpu,START_2); //核心2指令加载 
	/*创建两个线程*/ 
	thread1 = CreateThread(NULL,0,process,cpu,0,cpu->core_1->id);
	thread2 = CreateThread(NULL,0,process,cpu,0,cpu->core_2->id);
	WaitForSingleObject(thread1, INFINITE);
	WaitForSingleObject(thread2, INFINITE);
	CloseHandle(thread1);
	CloseHandle(thread2);
	
	end_output(cpu);//输出代码段和数据段信息 
	return 0;
}

/*cpu初始化*/ 
void init_cpu(cpuPtr cpu)
{
	corePtr core_1 = (corePtr)malloc(sizeof(struct core));
	corePtr core_2 = (corePtr)malloc(sizeof(struct core));
	init_core(core_1);
	init_core(core_2);
	
	int i;
	/*代码段和数据段初始化*/ 
	for(i=0;i<Max;i++)
	{
		cpu->code[i] = 0;
		cpu->data[i] = 0;
	}
	cpu->core_1 = core_1;
	cpu->core_2 = core_2;
	
	bin_to_char(cpu,16384,100);  //规定票数100存在数据段头 
	cpu->core_2->ip = START_2;//核心2 ip初始化为256 
	
}

/*core初始化*/ 
void init_core(corePtr core)
{
	int i;
	DWORD *id = (DWORD*)malloc(sizeof(DWORD)); //标识线程id 
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

/*指令加载*/ 
void load(FILE *fPtr,cpuPtr cpu,int start)
{
	char ch;
	int i,j;
	int temp;
	for(j=start;!feof(fPtr);)
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
			cpu->code[j++] = temp; //写入代码段 
			temp = 0;
		}
		cpu->code[j-1] = 0; //最后一次在代码段写入了EOF(即-1),所以要重新将其改写为默认值0 
}

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
