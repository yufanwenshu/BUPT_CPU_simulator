#include <stdio.h>
#include <stdlib.h>
#define Max (16384*2)
#define N 16384

/*模拟cpu的结构体*/ 
struct _cpu{
	char code[Max]; //模拟代码段 
	char data[Max]; //模拟数据段 
	short ax[9]; //模拟通用寄存器 
	short ip; //模拟程序计数器 
	short ir; //模拟指令寄存器 
	short flag; //模拟标志寄存器 
};
typedef struct _cpu* cpuPtr;

/*储存某条指令（32位）信息的结构体*/ 
struct _code{
	short task; //1-8位,表示操作类型 
	short front;//9-12位,表示操作位 1 
	short back; //13-16位,表示操作位 1 
	short value;//17-32位,表示立即数 
};
typedef struct _code* codePtr;

void init(cpuPtr cpu); //初始化cpu 
void load(FILE *fPtr,cpuPtr cpu); //加载指令 
void process(cpuPtr cpu,codePtr now_code); //取指令分析执行 
short bin(cpuPtr cpu,int start); //将cpu数据段指定位置两字节的char拼接转换为short 
void bin_to_char(cpuPtr cpu,int start,short number); //将short截断为两字节char写入cpu数据段指定位置 
void task_1(cpuPtr cpu,codePtr now_code); //数据传送
void task_2(cpuPtr cpu,codePtr now_code); //算数运算 
void task_6(cpuPtr cpu,codePtr now_code); //逻辑运算 
void task_9(cpuPtr cpu,codePtr now_code); //比较运算 
void task_10(cpuPtr cpu,codePtr now_code);//跳转 
void task_11(cpuPtr cpu,codePtr now_code);//输入输出 
int main()
{
	struct _cpu cpu_1;
	struct _code code_1;
	cpuPtr cpu = &cpu_1;
	codePtr now_code = &code_1;
	FILE *fPtr=fopen("dict.txt","r");
	if(fPtr==NULL)
		printf("error!");
	
	init(cpu); //初始化cpu 
	load(fPtr,cpu); //指令加载 
	process(cpu,now_code); //不断取指令分析执行 
	return 0;
}

/*cpu初始化*/ 
void init(cpuPtr cpu)
{
	int i;
	/*代码段和数据段初始化*/ 
	for(i=0;i<Max;i++)
	{
		cpu->code[i] = 0;
		cpu->data[i] = 0;
	}
	for(i=0;i<9;i++)
	{
		cpu->ax[i] = 0;
	}
	/*初始化程序计数器、指令寄存器、标志寄存器*/ 
	cpu->ip = cpu->ir = cpu->flag = 0;	
}

/*指令加载*/ 
void load(FILE *fPtr,cpuPtr cpu)
{
	char ch;
	int i,j;
	int temp;
	for(j=0;!feof(fPtr);)
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

/*不断取指令分析执行，最后输出代码段与数据段信息*/ 
void process(cpuPtr cpu,codePtr now_code)
{
	int i,j,k=0;
	/*循环，不断取指令分析执行，直到遇到停机指令*/ 
	do{
		/*代码段中每条指令占四个字节，取出前两个字节并拼接写入ir(指令寄存器)*/
		cpu->ir = cpu->code[(cpu->ip)++]&0xff; 
		cpu->ir = ((cpu->ir<<8)&0xff00)+(cpu->code[cpu->ip++]&0xff);
		
		/*取出后两个字节并拼接，即为立即数，赋值给value*/ 
		now_code->value = cpu->code[cpu->ip++]&0xff;
		now_code->value = ((now_code->value<<8)&0xff00)+(cpu->code[cpu->ip++]&0xff);
		
		/*指令寄存器储存16位指令，前8位对应操作类型，通过位运算取出赋值给task*/ 
		now_code->task = cpu->ir>>8;
		
		/*指令寄存器9-12位、13-16位对应操作位1和操作位2，位运算取出赋值给front、back*/ 
		now_code->front = ((cpu->ir<<8)>>12)&0xf;
		now_code->back = ((cpu->ir<<12)>>12)&0xf;
		
		/*分析指令操作类型，调用对应函数*/ 
		switch(now_code->task)
		{
			case 1: task_1(cpu,now_code);break; //数据传送指令 
			case 2:case 3:case 4:case 5: task_2(cpu,now_code);break; //算数运算指令 
			case 6:case 7:case 8: task_6(cpu,now_code);break;        //逻辑运算指令 
			case 9: task_9(cpu,now_code);break; //比较运算指令 
			case 10: task_10(cpu,now_code);break; //跳转指令 
			case 11:case 12: task_11(cpu,now_code);break; //输入输出指令 
		}
		
		/*每条指令执行完输出各寄存器状态*/ 
		printf("ip = %d\n",cpu->ip);
		printf("flag = %d\n",cpu->flag);
		printf("ir = %d\n",cpu->ir);
		printf("ax1 = %d ax2 = %d ax3 = %d ax4 = %d\n",cpu->ax[1],cpu->ax[2],cpu->ax[3],cpu->ax[4]);
		printf("ax5 = %d ax6 = %d ax7 = %d ax8 = %d\n",cpu->ax[5],cpu->ax[6],cpu->ax[7],cpu->ax[8]);
	}while(now_code->task);//task为0则为停机指令，跳出循环 
	
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

/*数据传送指令，其实就是赋值运算*/ 
void task_1(cpuPtr cpu,codePtr now_code)
{
	if(now_code->back==0)
		cpu->ax[now_code->front] = now_code->value; //把立即数赋值给数据寄存器 
	else if(now_code->back<=4)
		//把数据寄存器中的数赋值给地址寄存器中地址指向的位置，即写入数据段 
		bin_to_char(cpu,cpu->ax[now_code->front],cpu->ax[now_code->back]); 
	else
		//把地址寄存器中地址指向的位置中存放的数赋值给数据寄存器 
		cpu->ax[now_code->front] = bin(cpu,cpu->ax[now_code->back]);
}

/*代数运算指令，加减乘除*/ 
void task_2(cpuPtr cpu,codePtr now_code)
{
	if(now_code->back){
		//操作位1 （一定是数据寄存器）中的数与操作位2（一定是地址寄存器）指向的数进行运算 
		switch(now_code->task)
			{
				case 2:cpu->ax[now_code->front] += bin(cpu,cpu->ax[now_code->back]);break; //加法 
				case 3:cpu->ax[now_code->front] -= bin(cpu,cpu->ax[now_code->back]);break; //减法 
				case 4:cpu->ax[now_code->front] *= bin(cpu,cpu->ax[now_code->back]);break; //乘法 
				case 5:cpu->ax[now_code->front] /= bin(cpu,cpu->ax[now_code->back]);break; //除法				
			}
	}else{
		//操作位1对应的寄存器（一定是数据寄存器）中的数与立即数进行运算
		switch(now_code->task)
			{
				case 2:cpu->ax[now_code->front] += now_code->value;break; //加法 
				case 3:cpu->ax[now_code->front] -= now_code->value;break; //减法 
				case 4:cpu->ax[now_code->front] *= now_code->value;break; //乘法 
				case 5:cpu->ax[now_code->front] /= now_code->value;break; //除法 
			}
	}	
}

/*逻辑运算，与或非*/ 
void task_6(cpuPtr cpu,codePtr now_code)
{
	if(now_code->back==0) //如果操作位2是0，则操作位1（一定是数据寄存器）与立即数运算 
	{
		switch(now_code->task) 
		{
			case 6: //逻辑与 
				cpu->ax[now_code->front] = cpu->ax[now_code->front]&&now_code->value;break;
			case 7: //逻辑或 
				cpu->ax[now_code->front] = cpu->ax[now_code->front]||now_code->value;break;
			case 8: //逻辑非 
				cpu->ax[now_code->front] = !cpu->ax[now_code->front];break;
		}
	}
	else  //操作位2不为0，则操作位1(一定是数据寄存器)中的数与操作位2(一定是地址寄存器)指向的数运算 
	{
		switch(now_code->task)
		{
			case 6: //逻辑与 
				cpu->ax[now_code->front] = cpu->ax[now_code->front]&&bin(cpu,cpu->ax[now_code->back]);break;
			case 7: //逻辑或 
				cpu->ax[now_code->front] = cpu->ax[now_code->front]||bin(cpu,cpu->ax[now_code->back]);break;
			case 8: //逻辑非 
				bin_to_char(cpu,cpu->ax[now_code->back],!bin(cpu,cpu->ax[now_code->back]));break;
		}
	}
}

/*比较运算*/ 
void task_9(cpuPtr cpu,codePtr now_code)
{
	if(now_code->back==0){ 
		//操作位2为0，则操作位1（一定是数据寄存器）中的数与立即数比较 
		if(cpu->ax[now_code->front]>now_code->value)
			cpu->flag = 1;
		else if(cpu->ax[now_code->front]==now_code->value)
			cpu->flag = 0;
		else
			cpu->flag = -1;	
	}else{ 
		//操作位2不为0，则操作位1（一定是数据寄存器）中的数与操作位2（一定是地址寄存器）指向的数比较 
		if(cpu->ax[now_code->front]>bin(cpu,cpu->ax[now_code->back]))
			cpu->flag = 1;
		else if(cpu->ax[now_code->front]==bin(cpu,cpu->ax[now_code->back]))
			cpu->flag = 0;
		else
			cpu->flag = -1;
	}
}

/*跳转指令*/ 
void task_10(cpuPtr cpu,codePtr now_code)
{
	short temp = cpu->ip-4;  
	switch(now_code->back)
	{
		case 0:cpu->ip = (now_code->value + temp)&0xffff ;break; //操作位2为0，强制跳转，更新程序计数器ip 
		case 1: //操作位2为1，如果标志寄存器flag为0才跳转 
			if(cpu->flag==0){
				cpu->ip = (now_code->value + temp)&0xffff;
			}break;
		case 2: //操作位2为2，如果标志寄存器flag为1才跳转 
			if(cpu->flag==1){
				cpu->ip = (now_code->value + temp)&0xffff;
			}break;
		case 3:	//操作位2为3，如果标志寄存器flag为-1才跳转 
			if(cpu->flag==-1){
				cpu->ip = (now_code->value + temp)&0xffff;
			}break;
	}
}

/*输入输出指令*/ 
void task_11(cpuPtr cpu,codePtr now_code)
{
	if(now_code->task==11) //task为11，对应输入指令 
	{
		printf("in:\n");
		scanf("%hd",&cpu->ax[now_code->front]);
	}
	if(now_code->task==12) //task为12，对应输出指令 
	{
		printf("out: ");
		printf("%d\n",cpu->ax[now_code->front]);
	}
}
