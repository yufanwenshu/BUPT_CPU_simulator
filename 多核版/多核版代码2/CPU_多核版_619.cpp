#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
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

int main()
{
	HANDLE thread1,thread2;//线程句柄 
 
	struct _cpu cpu_temp; //新建一个结构体 
	cpuPtr cpu = &cpu_temp;
	FILE *fPtr_1=fopen("dict1.txt","r");
	FILE *fPtr_2=fopen("dict2.txt","r");
	if(fPtr_1==NULL||fPtr_2==NULL)
		printf("error!");
	
	init_cpu(cpu); //初始化cpu 
	load(fPtr_1,cpu,START_1); //核心1指令加载 
	load(fPtr_2,cpu,START_2); //核心2指令加载 
	/*创建两个线程*/ 
	thread1 = CreateThread(NULL,0,process1,cpu,0,NULL);
	thread2 = CreateThread(NULL,0,process2,cpu,0,NULL);
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
	int i;
	/*代码段和数据段初始化*/ 
	for(i=0;i<Max;i++)
	{
		cpu->code_data[i] = 0;
	}
	for(i=1;i<=12;i++)
	{
		cpu->core1_ax[i] = 0;
		cpu->core2_ax[i] = 0;
	}
	
	cpu->core2_ax[9] = START_2; //核心2的程序计数器初始时为256 
	
	bin_to_char(cpu,16384,100);  //规定票数100存在数据段头 
	
	cpu->hMutex = CreateMutex (NULL, FALSE, "tickets");//创建互斥对象 
	cpu->hMutex_print = CreateMutex (NULL, FALSE, "print");//创建互斥对象 
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
			cpu->code_data[j++] = temp; //写入代码段 
			temp = 0;
		}
		cpu->code_data[j-1] = 0; //最后一次在代码段写入了EOF(即-1),所以要重新将其改写为默认值0 
}

/*核心1的线程函数*/ 
DWORD WINAPI process1(LPVOID lpParameter)
{
	cpuPtr cpu = (cpuPtr)lpParameter; //参数类型转换 
	short ir[4] = {0};  //拆解指令,分别为指令类型，操作位1，操作位2，立即数 
	/*循环，不断取指令分析执行，直到遇到停机指令*/ 
	do{
		/*代码段中每条指令占四个字节，取出前两个字节并拼接写入ir(指令寄存器)*/
		cpu->core1_ax[10] = cpu->code_data[(cpu->core1_ax[9])++]&0xff; 
		cpu->core1_ax[10] = ((cpu->core1_ax[10]<<8)&0xff00)+(cpu->code_data[(cpu->core1_ax[9])++]&0xff);
		
		/*取出后两个字节并拼接，即为立即数，赋值给 ir[3]*/ 
		ir[3] = cpu->code_data[cpu->core1_ax[9]++]&0xff;
		ir[3] = ((ir[3]<<8)&0xff00)+(cpu->code_data[cpu->core1_ax[9]++]&0xff);
		
		/*指令寄存器储存16位指令，前8位对应操作类型，通过位运算取出赋值给 ir[0]*/ 
		ir[0] = cpu->core1_ax[10]>>8;
		
		/*指令寄存器9-12位、13-16位对应操作位1和操作位2，位运算取出赋值给 ir[1]、ir[2]*/ 
		ir[1] = ((cpu->core1_ax[10]<<8)>>12)&0xf;
		ir[2] = ((cpu->core1_ax[10]<<12)>>12)&0xf;
		
		/*分析指令操作类型，调用对应函数*/ 
		switch(ir[0])
		{
			case 1: transfer(cpu,ir,1);break; //数据传送指令 
			case 2:case 3:case 4:case 5: calculate(cpu,ir,1);break; //算数运算指令 
			case 6:case 7:case 8: logic(cpu,ir,1);break;        //逻辑运算指令 
			case 9: comparation(cpu,ir,1);break; //比较运算指令 
			case 10: skip(cpu,ir,1);break; //跳转指令 
			case 11:case 12: 
				WaitForSingleObject(cpu->hMutex_print,INFINITE);//等待正在执行的线程输出完毕
				ioput(cpu,ir,1);break; //输入输出指令 
			case 13:case 14:case 15: mul(cpu,ir,1);break; //多核版指令 
		}
		
		if(ir[0]!=11&&ir[0]!=12){
			WaitForSingleObject(cpu->hMutex_print,INFINITE);//等待正在执行的线程输出完毕
		} 
		/*每条指令执行完输出各寄存器状态*/ 
		output(cpu,1);
		ReleaseMutex(cpu->hMutex_print); //释放互斥对象 
		
	}while(ir[0]);//ir[0]为0则为停机指令，跳出循环 
}

/*核心2的线程函数*/ 
DWORD WINAPI process2(LPVOID lpParameter)
{
	cpuPtr cpu = (cpuPtr)lpParameter; //参数类型转换 
	short ir[4] = {0};  //拆解指令,分别为指令类型，操作位1，操作位2，立即数 
	/*循环，不断取指令分析执行，直到遇到停机指令*/ 
	do{
		/*代码段中每条指令占四个字节，取出前两个字节并拼接写入ir(指令寄存器)*/
		cpu->core2_ax[10] = cpu->code_data[(cpu->core2_ax[9])++]&0xff; 
		cpu->core2_ax[10] = ((cpu->core2_ax[10]<<8)&0xff00)+(cpu->code_data[(cpu->core2_ax[9])++]&0xff);
		
		/*取出后两个字节并拼接，即为立即数，赋值给 ir[3]*/ 
		ir[3] = cpu->code_data[cpu->core2_ax[9]++]&0xff;
		ir[3] = ((ir[3]<<8)&0xff00)+(cpu->code_data[cpu->core2_ax[9]++]&0xff);
		
		/*指令寄存器储存16位指令，前8位对应操作类型，通过位运算取出赋值给 ir[0]*/ 
		ir[0] = cpu->core2_ax[10]>>8;
		
		/*指令寄存器9-12位、13-16位对应操作位1和操作位2，位运算取出赋值给 ir[1]、ir[2]*/ 
		ir[1] = ((cpu->core2_ax[10]<<8)>>12)&0xf;
		ir[2] = ((cpu->core2_ax[10]<<12)>>12)&0xf;
		
		/*分析指令操作类型，调用对应函数*/ 
		switch(ir[0])
		{
			case 1: transfer(cpu,ir,2);break; //数据传送指令 
			case 2:case 3:case 4:case 5: calculate(cpu,ir,2);break; //算数运算指令 
			case 6:case 7:case 8: logic(cpu,ir,2);break;        //逻辑运算指令 
			case 9: comparation(cpu,ir,2);break; //比较运算指令 
			case 10: skip(cpu,ir,2);break; //跳转指令 
			case 11:case 12: 
				WaitForSingleObject(cpu->hMutex_print,INFINITE);//等待正在执行的线程输出完毕
				ioput(cpu,ir,2);break; //输入输出指令 
			case 13:case 14:case 15: mul(cpu,ir,2);break; //多核版指令 
		}
		
		if(ir[0]!=11&&ir[0]!=12){
			WaitForSingleObject(cpu->hMutex_print,INFINITE);//等待正在执行的线程输出完毕
		} 
		
		/*每条指令执行完输出各寄存器状态*/ 
		output(cpu,2);
		ReleaseMutex(cpu->hMutex_print); //释放互斥对象 
		
	}while(ir[0]);//ir[0]为0则为停机指令，跳出循环 
}

/*寄存器状态输出函数*/ 
void output(cpuPtr cpu,int id)
{
	chooce_ax(id)
	printf("id = %d\n",id);
	printf("ip = %d\n",ax[9]);
	printf("flag = %d\n",ax[11]);
	printf("ir = %d\n",ax[10]);
	printf("ax1 = %d ax2 = %d ax3 = %d ax4 = %d\n",ax[1],ax[2],ax[3],ax[4]);
	printf("ax5 = %d ax6 = %d ax7 = %d ax8 = %d\n",ax[5],ax[6],ax[7],ax[8]);
}

/*代码段与数据段输出函数*/ 
void end_output(cpuPtr cpu)
{
	int temp;
	int i,j,k=0;
	printf("\ncodeSegment :\n");
	for(i=0;i<16;i++) //共输出16行 
	{
		for(j=0;j<7;j++) //每行前7个数后要跟一个空格 
		{
			/*代码段中每条指-令存为4个char，通过位运算将其转换为32位整数并输出*/ 
			temp = (cpu->code_data[k++]<<24)&0xff000000;
			temp += (cpu->code_data[k++]<<16)&0xff0000;
			temp += (cpu->code_data[k++]<<8)&0xff00;
			temp += cpu->code_data[k++]&0xff;
			printf("%d ",temp);
		}
		//每行最后一个数后要跟一个换行符 
		temp = (cpu->code_data[k++]<<24)&0xff000000;
		temp += (cpu->code_data[k++]<<16)&0xff0000;
		temp += (cpu->code_data[k++]<<8)&0xff00;
		temp += cpu->code_data[k++]&0xff;
		printf("%d\n",temp);
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

short bin(cpuPtr cpu,int start)
{
	return (cpu->code_data[start]&0xff)*256+(cpu->code_data[start+1]&0xff); //通过位运算将两个char合并成一个整数（short） 
}

/*将一个整数（short）拆分为两个char并写入数据段*/ 
void bin_to_char(cpuPtr cpu,int start,short number)
{
	cpu->code_data[start++] = (char)(number>>8); //强制类型转换，short强制转换为char是低位截断 
	cpu->code_data[start] = (char)(number);
}

/*数据传送指令，其实就是赋值运算*/ 
void transfer(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(!ir[2])
		ax[ir[1]] = ir[3]; //把立即数赋值给数据寄存器 
	else if(ir[2]<=4)
		//把数据寄存器中的数赋值给地址寄存器中地址指向的位置，即写入数据段 
		bin_to_char(cpu,ax[ir[1]],ax[ir[2]]); 
	else
		//把地址寄存器中地址指向的位置中存放的数赋值给数据寄存器 
		ax[ir[1]] = bin(cpu,ax[ir[2]]);
}

/*代数运算指令，加减乘除*/ 
void calculate(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(ir[2]){
		//操作位1 （一定是数据寄存器）中的数与操作位2（一定是地址寄存器）指向的数进行运算 
		switch(ir[0])
			{
				case 2:ax[ir[1]] += bin(cpu,ax[ir[2]]);break; //加法 
				case 3:ax[ir[1]] -= bin(cpu,ax[ir[2]]);break; //减法 
				case 4:ax[ir[1]] *= bin(cpu,ax[ir[2]]);break; //乘法 
				case 5:ax[ir[1]] /= bin(cpu,ax[ir[2]]);break; //除法				
			}
	}else{
		//操作位1对应的寄存器（一定是数据寄存器）中的数与立即数进行运算
		switch(ir[0])
			{
				case 2:ax[ir[1]] += ir[3];break; //加法 
				case 3:ax[ir[1]] -= ir[3];break; //减法 
				case 4:ax[ir[1]] *= ir[3];break; //乘法 
				case 5:ax[ir[1]] /= ir[3];break; //除法 
			}
	}	
}

/*逻辑运算，与或非*/ 
void logic(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(!ir[2]) //如果操作位2是0，则操作位1（一定是数据寄存器）与立即数运算 
	{
		switch(ir[0]) 
		{
			case 6: //逻辑与 
				ax[ir[1]] = ax[ir[1]]&&ir[3];break;
			case 7: //逻辑或 
				ax[ir[1]] = ax[ir[1]]||ir[3];break;
			case 8: //逻辑非 
				ax[ir[1]] = !ax[ir[1]];break;
		}
	}
	else  //操作位2不为0，则操作位1(一定是数据寄存器)中的数与操作位2(一定是地址寄存器)指向的数运算 
	{
		switch(ir[0])
		{
			case 6: //逻辑与 
				ax[ir[1]] = ax[ir[1]]&&bin(cpu,ax[ir[2]]);break;
			case 7: //逻辑或 
				ax[ir[1]] = ax[ir[1]]||bin(cpu,ax[ir[2]]);break;
			case 8: //逻辑非 
				bin_to_char(cpu,ax[ir[2]],!bin(cpu,ax[ir[2]]));break;
		}
	}
}

/*比较运算*/ 
void comparation(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(!ir[2]){ 
		//操作位2为0，则操作位1（一定是数据寄存器）中的数与立即数比较 
		if(ax[ir[1]]>ir[3])
			ax[11] = 1;
		else if(ax[ir[1]]==ir[3])
			ax[11] = 0;
		else
			ax[11] = -1;	
	}else{ 
		//操作位2不为0，则操作位1（一定是数据寄存器）中的数与操作位2（一定是地址寄存器）指向的数比较 
		if(ax[ir[1]]>bin(cpu,ax[ir[2]]))
			ax[11] = 1;
		else if(ax[ir[1]]==bin(cpu,ax[ir[2]]))
			ax[11] = 0;
		else
			ax[11] = -1;
	}
}

/*跳转指令*/ 
void skip(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	switch(ir[2])
	{
		case 0:ax[9] += ir[3]-4;break; //操作位2为0，强制跳转，更新程序计数器ip 
		case 1: //操作位2为1，如果标志寄存器flag为0才跳转 
			if(ax[11]==0){
				ax[9] += ir[3]-4;
			}break;
		case 2: //操作位2为2，如果标志寄存器flag为1才跳转 
			if(ax[11]==1){
				ax[9] += ir[3]-4;
			}break;
		case 3:	//操作位2为3，如果标志寄存器flag为-1才跳转 
			if(ax[11]==-1){
				ax[9] += ir[3]-4;
			}break;
	}
}

/*输入输出指令*/ 
void ioput(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(ir[0]==11) //task为11，对应输入指令 
	{
		printf("in:\n");
		scanf("%hd",&ax[ir[1]]);
	}
	if(ir[0]==12) //task为12，对应输出指令 
	{
		printf("id = %d    out: ",id);
		printf("%d\n",ax[ir[1]]);
	}
}

/*多线程操作*/
void mul(cpuPtr cpu,short ir[],int id)
{
	switch(ir[0])
	{
		case 13: WaitForSingleObject(cpu->hMutex,INFINITE);break; //锁内存 
		case 14: ReleaseMutex(cpu->hMutex);break; //释放内存（解锁） 
		case 15: Sleep(ir[3]);break; //睡眠 
			
	}
}
