#ifndef _HEADERNAME_H
#define _HEADERNAME_H
#include <stdio.h>
#include <stdlib.h>
#endif
#include "cpu.h"
int main()
{
	read(); //指令加载 
	process();//不断取指令并分析执行
	end_output();//所有指令执行完后输出代码段和数据段信息 
	return 0;
}
