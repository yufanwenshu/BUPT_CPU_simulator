#ifndef CPU_CLOCK
#define CPU_CLOCK
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
#endif
