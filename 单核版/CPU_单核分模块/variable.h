#ifndef VARIABLE_CLOCK 
#define VARIABLE_CLOCK
#define Max (16384*2)
#define N 16384
short ip=0,ir=0,flag=0; //程序计数器，指令寄存器，标志寄存器 
char code[Max] = {0};   //代码段 
char data[Max] = {0};   //数据段 
short ax[9] = {0};      //通用寄存器，ax[0]无用（占位），ax[1]~ax[4]为数据寄存器，ax[5]~ax[8]为地址寄存器  
#endif
