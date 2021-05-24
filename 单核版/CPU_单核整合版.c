#include <stdio.h>
#include <stdlib.h>
#define Max (16384*2)
#define N 16384

short ip=0,ir=0,flag=0; //程序计数器，指令寄存器，标志寄存器 
char code[Max] = {0};   //代码段 
char data[Max] = {0};   //数据段 
short ax[9] = {0};      //通用寄存器，ax[0]无用（占位），ax[1]~ax[4]为数据寄存器，ax[5]~ax[8]为地址寄存器 

void read(void); //指令加载，从"dict.dic"中读取指令并写入代码段 
void process(void); //不断取指令并分析执行 
void task_1(short task,short front,short back,short value); //数据传送指令 
void task_2(short task,short front,short back,short value); //算术运算指令 
void task_6(short task,short front,short back,short value); //逻辑运算指令 
void task_9(short front,short back,short value); //比较指令 
void task_10(short back,short value); //跳转指令 
void task_11(short task,short front); //输入输出指令 
void end_output(void); //所有指令执行完后输出代码段和数据段信息 
short bin(int start);  //从数据段读取信息（两个char）并转换为16位二进制整数（short型） 
void bin_to_char(int start,short number); //将16位二进制整数(short型)转化为两个char并存入数据段 

int main()
{
	read(); //指令加载 
	process();//不断取指令并分析执行
	end_output();//所有指令执行完后输出代码段和数据段信息 
	return 0;
}

/*从"dict.dic"文件读取指令并储存在代码段（code数组）*/ 
void read(void)
{
	char ch;
	int i,j;
	int temp;
	FILE *fPtr=NULL;
	fPtr = fopen("dict.dic","r");
	if(!fPtr)
	{
		printf("File open error!\n");
	}
	else //文件打开成功 
	{
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
			code[j++] = temp; //写入代码段 
			temp = 0;
		}
		code[j-1] = 0; //最后一次在代码段写入了EOF(即-1),所以要重新将其改写为默认值0 
	}
}

/*不断从代码段取指令并分析执行，直到遇到停机指令*/ 
void process()
{
	short task,front,back,value; 
	//32位指令，1-8位对应操作类型，9-12位对应操作位1,13-16位对应操作位2，17-32位对应立即数
	//task储存操作类型，front储存操作位1，back储存操作位2，value储存立即数。 
	
	/*循环，不断取指令分析执行，直到遇到停机指令*/ 
	do
	{
		/*代码段中每条指令占四个字节，取出前两个字节并拼接写入ir(指令寄存器)*/ 
		ir = code[(ip)++]&0xff; 
		ir = ((ir<<8)&0xff00)+(code[ip++]&0xff);
		
		/*取出后两个字节并拼接，即为立即数，赋值给value*/ 
		value = code[ip++]&0xff;
		value = ((value<<8)&0xff00)+(code[ip++]&0xff);
		
		/*指令寄存器储存16位指令，前8位对应操作类型，通过位运算取出赋值给task*/ 
		task = ir>>8;
		
		/*指令寄存器9-12位、13-16位对应操作位1和操作位2，位运算取出赋值给front、back*/ 
		front = ((ir<<8)>>12)&0xf;
		back = ((ir<<12)>>12)&0xf;
		
		/*分析指令操作类型，调用对应函数*/ 
		switch(task)
		{
			case 1: task_1(task,front,back,value);break; //数据传送指令 
			case 2:case 3:case 4:case 5: task_2(task,front,back,value);break; //算数运算指令 
			case 6:case 7:case 8: task_6(task,front,back,value);break;        //逻辑运算指令 
			case 9: task_9(front,back,value);break; //比较运算指令 
			case 10: task_10(back,value);break; //跳转指令 
			case 11:case 12: task_11(task,front);break; //输入输出指令 
		}
		
		/*每条指令执行完输出各寄存器状态*/ 
		printf("ip = %d\n",ip);
		printf("flag = %d\n",flag);
		printf("ir = %d\n",ir);
		printf("ax1 = %d ax2 = %d ax3 = %d ax4 = %d\n",ax[1],ax[2],ax[3],ax[4]);
		printf("ax5 = %d ax6 = %d ax7 = %d ax8 = %d\n",ax[5],ax[6],ax[7],ax[8]);
	}while(task); //task为0则为停机指令，跳出循环 
	
}

/*数据传送指令，其实就是赋值运算*/ 
void task_1(short task,short front,short back,short value)
{
	switch(back)//根据操作位2的类型分情况 
	{
		case 0:ax[front] = value;break; //把立即数赋值给数据寄存器 
		
		case 1:case 2:case 3:case 4:   //把数据寄存器中的数赋值给地址寄存器中地址指向的位置，即写入数据段 
			bin_to_char(ax[front],ax[back]);break;
		case 5:case 6:case 7:case 8:
			ax[front] = bin(ax[back]);break; //把地址寄存器中地址指向的位置中存放的数赋值给数据寄存器 
	}
}

/*代数运算指令，加减乘除*/ 
void task_2(short task,short front,short back,short value)
{
	switch(back) //根据操作位2的类型分情况 
	{
		case 0:  //操作位1对应的寄存器（一定是数据寄存器）中的数与立即数进行运算 
			switch(task)
			{
				case 2:ax[front] += value;break; //加法 
				case 3:ax[front] -= value;break; //减法 
				case 4:ax[front] *= value;break; //乘法 
				case 5:ax[front] /= value;break; //除法 
			}break;
		default: //操作位1 （一定是数据寄存器）中的数与操作位2（一定是地址寄存器）指向的数进行运算 
			switch(task)
			{
				case 2:ax[front] += bin(ax[back]);break; //加法 
				case 3:ax[front] -= bin(ax[back]);break; //减法 
				case 4:ax[front] *= bin(ax[back]);break; //乘法 
				case 5:ax[front] /= bin(ax[back]);break; //除法				
			}		
	}
	
}

/*逻辑运算，与或非*/ 
void task_6(short task,short front,short back,short value)
{
	if(!back) //如果操作位2是0，则操作位1（一定是数据寄存器）与立即数运算 
	{
		switch(task) 
		{
			case 6: //逻辑与 
				ax[front] = ax[front]&&value;break;
			case 7: //逻辑或 
				ax[front] = ax[front]||value;break;
			case 8: //逻辑非 
				ax[front] = !ax[front];break;
		}
	}
	else  //操作位2不为0，则操作位1(一定是数据寄存器)中的数与操作位2(一定是地址寄存器)指向的数运算 
	{
		switch(task)
		{
			case 6: //逻辑与 
				ax[front] = ax[front]&&bin(ax[back]);break;
			case 7: //逻辑或 
				ax[front] = ax[front]||bin(ax[back]);break;
			case 8: //逻辑非 
				bin_to_char(ax[back],!bin(ax[back]));break;
		}
	}
}

/*比较运算*/ 
void task_9(short front,short back,short value)
{
	switch(back)
	{
		case 0: //操作位2为0，则操作位1（一定是数据寄存器）中的数与立即数比较 
			if(ax[front]>value)
				flag = 1;
			else if(ax[front]==value)
				flag = 0;
			else
				flag = -1;
			break;
		default: //操作位2不为0，则操作位1（一定是数据寄存器）中的数与操作位2（一定是地址寄存器）指向的数比较 
			if(ax[front]>bin(ax[back]))
				flag = 1;
			else if(ax[front]==bin(ax[back]))
				flag = 0;
			else
				flag = -1;
	}
}

/*跳转指令*/ 
void task_10(short back,short value)
{
	short temp = ip-4;  
	switch(back)
	{
		case 0:ip = (value + temp)&0xffff ;break; //操作位2为0，强制跳转，更新程序计数器ip 
		case 1: //操作位2为1，如果标志寄存器flag为0才跳转 
			if(flag==0){
				ip = (value + temp)&0xffff;
			}break;
		case 2: //操作位2为2，如果标志寄存器flag为1才跳转 
			if(flag==1){
				ip = (value + temp)&0xffff;
			}break;
		case 3:	//操作位2为3，如果标志寄存器flag为-1才跳转 
			if(flag==-1){
				ip = (value + temp)&0xffff;
			}break;
	}
}

/*输入输出指令*/ 
void task_11(short task,short front)
{
	if(task==11) //task为11，对应输入指令 
	{
		printf("in:\n");
		scanf("%hd",&ax[front]);
	}
	if(task==12) //task为12，对应输出指令 
	{
		printf("out: ");
		printf("%d\n",ax[front]);
	}
}

/*所有指令执行完输出代码段与数据段信息*/ 
void end_output(void)
{
	int i,j,k=0;
	printf("\ncodeSegment :\n");
	for(i=0;i<16;i++) //共输出16行 
	{
		for(j=0;j<7;j++) //每行前7个数后要跟一个空格 
		{
			/*代码段中每条指令存为4个char，通过位运算将其转换为32位整数并输出*/ 
			printf("%d ",((code[k]<<24)&0xff000000)+((code[k+1]<<16)&0xff0000)+((code[k+2]<<8)&0xff00)+(code[k+3]&0xff));
			k += 4;
		}
		//每行最后一个数后要跟一个换行符 
		printf("%d\n",((code[k]<<24)&0xff000000)+((code[k+1]<<16)&0xff0000)+((code[k+2]<<8)&0xff00)+(code[k+3]&0xff));
		k += 4;
	}
	
	k=N;
	printf("\ndataSegment :\n");
	for(i=0;i<16;i++) //共输出16行 
	{
		for(j=0;j<15;j++) //每行前15个数后要跟一个空格 
		{
			printf("%d ",bin(k)); //调用bin()将数据段中相邻两个char转换为一个整数（short）输出 
			k += 2;
		}
		//每行最后一个数后要跟一个换行符 
		printf("%d\n",bin(k));
		k += 2;
	}
}

/*将数据段中相邻两个char合并为一个整数*/ 
short bin(int start)
{
	start -= N; //start为N（16384）则对数据段第一个元素data[0]，将start转换为数组下标 
	return ((data[start]<<8)&0xff00)+(data[start+1]&0xff); //通过位运算将两个char合并成一个整数（short） 
}

/*将一个整数（short）拆分为两个char并写入数据段*/ 
void bin_to_char(int start,short number)
{
	start -= N; //start为N（16384）则对数据段第一个元素data[0]，将start转换为数组下标 
	data[start++] = (char)(number>>8); //强制类型转换，short强制转换为char是低位截断 
	data[start] = (char)(number);
}
