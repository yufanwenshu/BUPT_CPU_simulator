#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#define Max (16384*2) //����������ݶε��ܳ��� 
#define N 16384 //���ݶε��±�ƫ�� 
#define START_1 0 //����1�Ĵ�����ʼλ�� 
#define START_2 256 //����2�Ĵ�����ʼλ�� 
#define TICKETS 100 //��Ҫ������Ʊ�� 

#define chooce_ax(id) 	short* ax=NULL;if(id==1){ax = cpu->core1_ax;}else if(id==2){ax = cpu->core2_ax;}

/*ģ��cpu�Ľṹ��*/ 
struct _cpu{
	char code_data[Max]; //ģ�����κ����ݶ�  
	short core1_ax[12]; //ģ�����1�ļĴ��� ��0���ã�1-8ͨ�üĴ�����9�����������ip)��10ָ��Ĵ���(ir)��11��־�Ĵ��� (flag)
	short core2_ax[12]; //ģ�����2�ļĴ��� ��0���ã�1-8ͨ�üĴ�����9�����������ip)��10ָ��Ĵ���(ir)��11��־�Ĵ��� (flag)
	HANDLE hMutex;//���������������ִ�ж�˰�ָ�� 
	HANDLE hMutex_print;//��������������ڱ����������� 
};
typedef struct _cpu* cpuPtr;

void init_cpu(cpuPtr cpu); //��ʼ��cpu 
void load(FILE *fPtr,cpuPtr cpu,int start); //ָ����� 
DWORD WINAPI process1(LPVOID lpParameter);  //����1���̺߳��� 
DWORD WINAPI process2(LPVOID lpParameter);  //����2���̺߳��� 
void output(cpuPtr cpu,int id); //�����ǰ�Ĵ�����Ϣ 
void end_output(cpuPtr cpu); //�������������ݶ���Ϣ 
short bin(cpuPtr cpu,int start); //ͨ��λ���㽫����char�ϲ���һ��������short��
void bin_to_char(cpuPtr cpu,int start,short number); //��һ��������short�����Ϊ����char��д�����ݶ�
void transfer(cpuPtr cpu,short ir[],int id); //���ݴ���ָ�� 
void calculate(cpuPtr cpu,short ir[],int id);//��������ָ�� 
void logic(cpuPtr cpu,short ir[],int id); //�߼�����ָ�� 
void comparation(cpuPtr cpu,short ir[],int id); //�Ƚ�ָ�� 
void skip(cpuPtr cpu,short ir[],int id); //��תָ�� 
void ioput(cpuPtr cpu,short ir[],int id); //�������ָ�� 
void mul(cpuPtr cpu,short ir[],int id); //��˰�ָ�� 

int main()
{
	HANDLE thread1,thread2;//�߳̾�� 
 
	struct _cpu cpu_temp; //�½�һ���ṹ�� 
	cpuPtr cpu = &cpu_temp;
	FILE *fPtr_1=fopen("dict1.txt","r");
	FILE *fPtr_2=fopen("dict2.txt","r");
	if(fPtr_1==NULL||fPtr_2==NULL)
		printf("error!");
	
	init_cpu(cpu); //��ʼ��cpu 
	load(fPtr_1,cpu,START_1); //����1ָ����� 
	load(fPtr_2,cpu,START_2); //����2ָ����� 
	/*���������߳�*/ 
	thread1 = CreateThread(NULL,0,process1,cpu,0,NULL);
	thread2 = CreateThread(NULL,0,process2,cpu,0,NULL);
	WaitForSingleObject(thread1, INFINITE);
	WaitForSingleObject(thread2, INFINITE);
	CloseHandle(thread1);
	CloseHandle(thread2);
	
	end_output(cpu);//�������κ����ݶ���Ϣ 
	return 0;
}

/*cpu��ʼ��*/ 
void init_cpu(cpuPtr cpu)
{
	int i;
	/*����κ����ݶγ�ʼ��*/ 
	for(i=0;i<Max;i++)
	{
		cpu->code_data[i] = 0;
	}
	for(i=1;i<=12;i++)
	{
		cpu->core1_ax[i] = 0;
		cpu->core2_ax[i] = 0;
	}
	
	cpu->core2_ax[9] = START_2; //����2�ĳ����������ʼʱΪ256 
	
	bin_to_char(cpu,16384,100);  //�涨Ʊ��100�������ݶ�ͷ 
	
	cpu->hMutex = CreateMutex (NULL, FALSE, "tickets");//����������� 
	cpu->hMutex_print = CreateMutex (NULL, FALSE, "print");//����������� 
}

/*ָ�����*/ 
void load(FILE *fPtr,cpuPtr cpu,int start)
{
	char ch;
	int i,j;
	int temp;
	for(j=start;!feof(fPtr);)
		{
			for(i=0;i<8;i++)      //��Ϊ�����codeΪchar������
			{                     //����һ��ָ����Ҫռ���ĸ�����ռ�
				ch = fgetc(fPtr); //ÿ�ζ�ȡ8λ������ֵ������ת��Ϊchar��������
				if(ch=='\n')
				{                 //�����ȡ�����з������䶪�����¶�ȡ 
					i--;
					continue;
				}
				temp = temp*2 + ch -'0'; //ÿ�δ��ļ��ж�������ʵ��'1'��'0'��Ӧ��ASCII�룬Ҫ����ת��Ϊ��ֵ���� 
			}
			cpu->code_data[j++] = temp; //д������ 
			temp = 0;
		}
		cpu->code_data[j-1] = 0; //���һ���ڴ����д����EOF(��-1),����Ҫ���½����дΪĬ��ֵ0 
}

/*����1���̺߳���*/ 
DWORD WINAPI process1(LPVOID lpParameter)
{
	cpuPtr cpu = (cpuPtr)lpParameter; //��������ת�� 
	short ir[4] = {0};  //���ָ��,�ֱ�Ϊָ�����ͣ�����λ1������λ2�������� 
	/*ѭ��������ȡָ�����ִ�У�ֱ������ͣ��ָ��*/ 
	do{
		/*�������ÿ��ָ��ռ�ĸ��ֽڣ�ȡ��ǰ�����ֽڲ�ƴ��д��ir(ָ��Ĵ���)*/
		cpu->core1_ax[10] = cpu->code_data[(cpu->core1_ax[9])++]&0xff; 
		cpu->core1_ax[10] = ((cpu->core1_ax[10]<<8)&0xff00)+(cpu->code_data[(cpu->core1_ax[9])++]&0xff);
		
		/*ȡ���������ֽڲ�ƴ�ӣ���Ϊ����������ֵ�� ir[3]*/ 
		ir[3] = cpu->code_data[cpu->core1_ax[9]++]&0xff;
		ir[3] = ((ir[3]<<8)&0xff00)+(cpu->code_data[cpu->core1_ax[9]++]&0xff);
		
		/*ָ��Ĵ�������16λָ�ǰ8λ��Ӧ�������ͣ�ͨ��λ����ȡ����ֵ�� ir[0]*/ 
		ir[0] = cpu->core1_ax[10]>>8;
		
		/*ָ��Ĵ���9-12λ��13-16λ��Ӧ����λ1�Ͳ���λ2��λ����ȡ����ֵ�� ir[1]��ir[2]*/ 
		ir[1] = ((cpu->core1_ax[10]<<8)>>12)&0xf;
		ir[2] = ((cpu->core1_ax[10]<<12)>>12)&0xf;
		
		/*����ָ��������ͣ����ö�Ӧ����*/ 
		switch(ir[0])
		{
			case 1: transfer(cpu,ir,1);break; //���ݴ���ָ�� 
			case 2:case 3:case 4:case 5: calculate(cpu,ir,1);break; //��������ָ�� 
			case 6:case 7:case 8: logic(cpu,ir,1);break;        //�߼�����ָ�� 
			case 9: comparation(cpu,ir,1);break; //�Ƚ�����ָ�� 
			case 10: skip(cpu,ir,1);break; //��תָ�� 
			case 11:case 12: 
				WaitForSingleObject(cpu->hMutex_print,INFINITE);//�ȴ�����ִ�е��߳�������
				ioput(cpu,ir,1);break; //�������ָ�� 
			case 13:case 14:case 15: mul(cpu,ir,1);break; //��˰�ָ�� 
		}
		
		if(ir[0]!=11&&ir[0]!=12){
			WaitForSingleObject(cpu->hMutex_print,INFINITE);//�ȴ�����ִ�е��߳�������
		} 
		/*ÿ��ָ��ִ����������Ĵ���״̬*/ 
		output(cpu,1);
		ReleaseMutex(cpu->hMutex_print); //�ͷŻ������ 
		
	}while(ir[0]);//ir[0]Ϊ0��Ϊͣ��ָ�����ѭ�� 
}

/*����2���̺߳���*/ 
DWORD WINAPI process2(LPVOID lpParameter)
{
	cpuPtr cpu = (cpuPtr)lpParameter; //��������ת�� 
	short ir[4] = {0};  //���ָ��,�ֱ�Ϊָ�����ͣ�����λ1������λ2�������� 
	/*ѭ��������ȡָ�����ִ�У�ֱ������ͣ��ָ��*/ 
	do{
		/*�������ÿ��ָ��ռ�ĸ��ֽڣ�ȡ��ǰ�����ֽڲ�ƴ��д��ir(ָ��Ĵ���)*/
		cpu->core2_ax[10] = cpu->code_data[(cpu->core2_ax[9])++]&0xff; 
		cpu->core2_ax[10] = ((cpu->core2_ax[10]<<8)&0xff00)+(cpu->code_data[(cpu->core2_ax[9])++]&0xff);
		
		/*ȡ���������ֽڲ�ƴ�ӣ���Ϊ����������ֵ�� ir[3]*/ 
		ir[3] = cpu->code_data[cpu->core2_ax[9]++]&0xff;
		ir[3] = ((ir[3]<<8)&0xff00)+(cpu->code_data[cpu->core2_ax[9]++]&0xff);
		
		/*ָ��Ĵ�������16λָ�ǰ8λ��Ӧ�������ͣ�ͨ��λ����ȡ����ֵ�� ir[0]*/ 
		ir[0] = cpu->core2_ax[10]>>8;
		
		/*ָ��Ĵ���9-12λ��13-16λ��Ӧ����λ1�Ͳ���λ2��λ����ȡ����ֵ�� ir[1]��ir[2]*/ 
		ir[1] = ((cpu->core2_ax[10]<<8)>>12)&0xf;
		ir[2] = ((cpu->core2_ax[10]<<12)>>12)&0xf;
		
		/*����ָ��������ͣ����ö�Ӧ����*/ 
		switch(ir[0])
		{
			case 1: transfer(cpu,ir,2);break; //���ݴ���ָ�� 
			case 2:case 3:case 4:case 5: calculate(cpu,ir,2);break; //��������ָ�� 
			case 6:case 7:case 8: logic(cpu,ir,2);break;        //�߼�����ָ�� 
			case 9: comparation(cpu,ir,2);break; //�Ƚ�����ָ�� 
			case 10: skip(cpu,ir,2);break; //��תָ�� 
			case 11:case 12: 
				WaitForSingleObject(cpu->hMutex_print,INFINITE);//�ȴ�����ִ�е��߳�������
				ioput(cpu,ir,2);break; //�������ָ�� 
			case 13:case 14:case 15: mul(cpu,ir,2);break; //��˰�ָ�� 
		}
		
		if(ir[0]!=11&&ir[0]!=12){
			WaitForSingleObject(cpu->hMutex_print,INFINITE);//�ȴ�����ִ�е��߳�������
		} 
		
		/*ÿ��ָ��ִ����������Ĵ���״̬*/ 
		output(cpu,2);
		ReleaseMutex(cpu->hMutex_print); //�ͷŻ������ 
		
	}while(ir[0]);//ir[0]Ϊ0��Ϊͣ��ָ�����ѭ�� 
}

/*�Ĵ���״̬�������*/ 
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

/*����������ݶ��������*/ 
void end_output(cpuPtr cpu)
{
	int temp;
	int i,j,k=0;
	printf("\ncodeSegment :\n");
	for(i=0;i<16;i++) //�����16�� 
	{
		for(j=0;j<7;j++) //ÿ��ǰ7������Ҫ��һ���ո� 
		{
			/*�������ÿ��ָ-���Ϊ4��char��ͨ��λ���㽫��ת��Ϊ32λ���������*/ 
			temp = (cpu->code_data[k++]<<24)&0xff000000;
			temp += (cpu->code_data[k++]<<16)&0xff0000;
			temp += (cpu->code_data[k++]<<8)&0xff00;
			temp += cpu->code_data[k++]&0xff;
			printf("%d ",temp);
		}
		//ÿ�����һ������Ҫ��һ�����з� 
		temp = (cpu->code_data[k++]<<24)&0xff000000;
		temp += (cpu->code_data[k++]<<16)&0xff0000;
		temp += (cpu->code_data[k++]<<8)&0xff00;
		temp += cpu->code_data[k++]&0xff;
		printf("%d\n",temp);
	}
	
	k=N;
	printf("\ndataSegment :\n");
	for(i=0;i<16;i++) //�����16�� 
	{
		for(j=0;j<15;j++) //ÿ��ǰ15������Ҫ��һ���ո� 
		{
			printf("%d ",bin(cpu,k)); //����bin()�����ݶ�����������charת��Ϊһ��������short����� 
			k += 2;
		}
		//ÿ�����һ������Ҫ��һ�����з� 
		printf("%d\n",bin(cpu,k));
		k += 2;
	}
}

short bin(cpuPtr cpu,int start)
{
	return (cpu->code_data[start]&0xff)*256+(cpu->code_data[start+1]&0xff); //ͨ��λ���㽫����char�ϲ���һ��������short�� 
}

/*��һ��������short�����Ϊ����char��д�����ݶ�*/ 
void bin_to_char(cpuPtr cpu,int start,short number)
{
	cpu->code_data[start++] = (char)(number>>8); //ǿ������ת����shortǿ��ת��Ϊchar�ǵ�λ�ض� 
	cpu->code_data[start] = (char)(number);
}

/*���ݴ���ָ���ʵ���Ǹ�ֵ����*/ 
void transfer(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(!ir[2])
		ax[ir[1]] = ir[3]; //����������ֵ�����ݼĴ��� 
	else if(ir[2]<=4)
		//�����ݼĴ����е�����ֵ����ַ�Ĵ����е�ַָ���λ�ã���д�����ݶ� 
		bin_to_char(cpu,ax[ir[1]],ax[ir[2]]); 
	else
		//�ѵ�ַ�Ĵ����е�ַָ���λ���д�ŵ�����ֵ�����ݼĴ��� 
		ax[ir[1]] = bin(cpu,ax[ir[2]]);
}

/*��������ָ��Ӽ��˳�*/ 
void calculate(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(ir[2]){
		//����λ1 ��һ�������ݼĴ������е��������λ2��һ���ǵ�ַ�Ĵ�����ָ������������� 
		switch(ir[0])
			{
				case 2:ax[ir[1]] += bin(cpu,ax[ir[2]]);break; //�ӷ� 
				case 3:ax[ir[1]] -= bin(cpu,ax[ir[2]]);break; //���� 
				case 4:ax[ir[1]] *= bin(cpu,ax[ir[2]]);break; //�˷� 
				case 5:ax[ir[1]] /= bin(cpu,ax[ir[2]]);break; //����				
			}
	}else{
		//����λ1��Ӧ�ļĴ�����һ�������ݼĴ������е�������������������
		switch(ir[0])
			{
				case 2:ax[ir[1]] += ir[3];break; //�ӷ� 
				case 3:ax[ir[1]] -= ir[3];break; //���� 
				case 4:ax[ir[1]] *= ir[3];break; //�˷� 
				case 5:ax[ir[1]] /= ir[3];break; //���� 
			}
	}	
}

/*�߼����㣬����*/ 
void logic(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(!ir[2]) //�������λ2��0�������λ1��һ�������ݼĴ����������������� 
	{
		switch(ir[0]) 
		{
			case 6: //�߼��� 
				ax[ir[1]] = ax[ir[1]]&&ir[3];break;
			case 7: //�߼��� 
				ax[ir[1]] = ax[ir[1]]||ir[3];break;
			case 8: //�߼��� 
				ax[ir[1]] = !ax[ir[1]];break;
		}
	}
	else  //����λ2��Ϊ0�������λ1(һ�������ݼĴ���)�е��������λ2(һ���ǵ�ַ�Ĵ���)ָ��������� 
	{
		switch(ir[0])
		{
			case 6: //�߼��� 
				ax[ir[1]] = ax[ir[1]]&&bin(cpu,ax[ir[2]]);break;
			case 7: //�߼��� 
				ax[ir[1]] = ax[ir[1]]||bin(cpu,ax[ir[2]]);break;
			case 8: //�߼��� 
				bin_to_char(cpu,ax[ir[2]],!bin(cpu,ax[ir[2]]));break;
		}
	}
}

/*�Ƚ�����*/ 
void comparation(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(!ir[2]){ 
		//����λ2Ϊ0�������λ1��һ�������ݼĴ������е������������Ƚ� 
		if(ax[ir[1]]>ir[3])
			ax[11] = 1;
		else if(ax[ir[1]]==ir[3])
			ax[11] = 0;
		else
			ax[11] = -1;	
	}else{ 
		//����λ2��Ϊ0�������λ1��һ�������ݼĴ������е��������λ2��һ���ǵ�ַ�Ĵ�����ָ������Ƚ� 
		if(ax[ir[1]]>bin(cpu,ax[ir[2]]))
			ax[11] = 1;
		else if(ax[ir[1]]==bin(cpu,ax[ir[2]]))
			ax[11] = 0;
		else
			ax[11] = -1;
	}
}

/*��תָ��*/ 
void skip(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	switch(ir[2])
	{
		case 0:ax[9] += ir[3]-4;break; //����λ2Ϊ0��ǿ����ת�����³��������ip 
		case 1: //����λ2Ϊ1�������־�Ĵ���flagΪ0����ת 
			if(ax[11]==0){
				ax[9] += ir[3]-4;
			}break;
		case 2: //����λ2Ϊ2�������־�Ĵ���flagΪ1����ת 
			if(ax[11]==1){
				ax[9] += ir[3]-4;
			}break;
		case 3:	//����λ2Ϊ3�������־�Ĵ���flagΪ-1����ת 
			if(ax[11]==-1){
				ax[9] += ir[3]-4;
			}break;
	}
}

/*�������ָ��*/ 
void ioput(cpuPtr cpu,short ir[],int id)
{
	chooce_ax(id);
	if(ir[0]==11) //taskΪ11����Ӧ����ָ�� 
	{
		printf("in:\n");
		scanf("%hd",&ax[ir[1]]);
	}
	if(ir[0]==12) //taskΪ12����Ӧ���ָ�� 
	{
		printf("id = %d    out: ",id);
		printf("%d\n",ax[ir[1]]);
	}
}

/*���̲߳���*/
void mul(cpuPtr cpu,short ir[],int id)
{
	switch(ir[0])
	{
		case 13: WaitForSingleObject(cpu->hMutex,INFINITE);break; //���ڴ� 
		case 14: ReleaseMutex(cpu->hMutex);break; //�ͷ��ڴ棨������ 
		case 15: Sleep(ir[3]);break; //˯�� 
			
	}
}
