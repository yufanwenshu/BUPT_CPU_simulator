#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#define Max 16384 //���ݶκʹ���εĳ��� 
#define N 16384   //���ݶε��±�ƫ�� 
#define START_1 0 //����1�Ĵ�����ʼλ�� 
#define START_2 256 //����2�Ĵ�����ʼλ�� 
#define TICKETS 100 //��Ҫ������Ʊ�� 

/*ģ����ĵĽṹ��*/ 
struct core{
	short ax[9]; //ģ��ͨ�üĴ��� 
	short ip; //ģ���������� 
	short ir; //ģ��ָ��Ĵ��� 
	short flag; //ģ���־�Ĵ��� 
	short task; //1-8λ,��ʾ�������� 
	short front;//9-12λ,��ʾ����λ 1 
	short back; //13-16λ,��ʾ����λ 1 
	short value;//17-32λ,��ʾ������ 
	DWORD *id; //��¼�߳�id 
};
typedef struct core* corePtr;

/*ģ��cpu�Ľṹ��*/ 
struct _cpu{
	char code[Max]; //ģ������ 
	char data[Max]; //ģ�����ݶ� 
	corePtr core_1;
	corePtr core_2;
	HANDLE hMutex;//��������� 
	HANDLE hMutex_state;//�������������
};
typedef struct _cpu* cpuPtr;

void init_cpu(cpuPtr cpu); //��ʼ��cpu 
void init_core(corePtr core); //��ʼ��core 
void load(FILE *fPtr,cpuPtr cpu,int start); //ָ����� 
void end_output(cpuPtr cpu); //�������κ����ݶ���Ϣ 
DWORD WINAPI process(LPVOID lpParameter); //�̺߳��� 
void task_1(cpuPtr cpu,corePtr core); //���ݴ���ָ�� 
void task_2(cpuPtr cpu,corePtr core); //��������ָ�� 
void task_6(cpuPtr cpu,corePtr core); //�߼�����ָ��
void task_9(cpuPtr cpu,corePtr core); //�Ƚ�����ָ�� 
void task_10(cpuPtr cpu,corePtr core);//��תָ�� 
void task_11(cpuPtr cpu,corePtr core);//�������ָ�� 
void task_13(cpuPtr cpu,corePtr core);//��˰�ָ�� 
short bin(cpuPtr cpu,int start);//�����ݶζ�ȡ��Ϣ������char����ת��Ϊ16λ������������short�ͣ�
void bin_to_char(cpuPtr cpu,int start,short number);//��16λ����������(short��)ת��Ϊ����char���������ݶ� 


int main()
{
	HANDLE thread1,thread2;//�߳̾�� 
 
	cpuPtr cpu = (cpuPtr)malloc(sizeof(struct _cpu));
	FILE *fPtr_1=fopen("dict1.txt","r");
	FILE *fPtr_2=fopen("dict2.txt","r");
	if(fPtr_1==NULL||fPtr_2==NULL)
		printf("error!");
	
	init_cpu(cpu); //��ʼ��cpu 
	load(fPtr_1,cpu,START_1); //����1ָ����� 
	load(fPtr_2,cpu,START_2); //����2ָ����� 
	/*���������߳�*/ 
	thread1 = CreateThread(NULL,0,process,cpu,0,cpu->core_1->id);
	thread2 = CreateThread(NULL,0,process,cpu,0,cpu->core_2->id);
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
	corePtr core_1 = (corePtr)malloc(sizeof(struct core));
	corePtr core_2 = (corePtr)malloc(sizeof(struct core));
	init_core(core_1);
	init_core(core_2);
	
	int i;
	/*����κ����ݶγ�ʼ��*/ 
	for(i=0;i<Max;i++)
	{
		cpu->code[i] = 0;
		cpu->data[i] = 0;
	}
	cpu->core_1 = core_1;
	cpu->core_2 = core_2;
	
	bin_to_char(cpu,16384,100);  //�涨Ʊ��100�������ݶ�ͷ 
	cpu->core_2->ip = START_2;//����2 ip��ʼ��Ϊ256 
	
}

/*core��ʼ��*/ 
void init_core(corePtr core)
{
	int i;
	DWORD *id = (DWORD*)malloc(sizeof(DWORD)); //��ʶ�߳�id 
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
			cpu->code[j++] = temp; //д������ 
			temp = 0;
		}
		cpu->code[j-1] = 0; //���һ���ڴ����д����EOF(��-1),����Ҫ���½����дΪĬ��ֵ0 
}

void end_output(cpuPtr cpu)
{	
	int i,j,k=0;
	/*����������κ����ݶ���Ϣ*/ 
	printf("\ncodeSegment :\n");
	for(i=0;i<16;i++) //�����16�� 
	{
		for(j=0;j<7;j++) //ÿ��ǰ7������Ҫ��һ���ո� 
		{
			/*�������ÿ��ָ���Ϊ4��char��ͨ��λ���㽫��ת��Ϊ32λ���������*/ 
			printf("%d ",((cpu->code[k]<<24)&0xff000000)+((cpu->code[k+1]<<16)&0xff0000)+((cpu->code[k+2]<<8)&0xff00)+(cpu->code[k+3]&0xff));
			k += 4;
		}
		//ÿ�����һ������Ҫ��һ�����з� 
		printf("%d\n",((cpu->code[k]<<24)&0xff000000)+((cpu->code[k+1]<<16)&0xff0000)+((cpu->code[k+2]<<8)&0xff00)+(cpu->code[k+3]&0xff));
		k += 4;
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

DWORD WINAPI process(LPVOID lpParameter)
{
	cpuPtr cpu = (cpuPtr)lpParameter; //��������ת�� 
	int core_id; //��¼��ǰ���к���
	corePtr core = NULL; //��ǰ����ָ�� 
	
	cpu->hMutex=CreateMutex (NULL, FALSE, "tickets");//������������ 
	cpu->hMutex_state=CreateMutex (NULL, FALSE, "states");//������������ 
	
	if(GetCurrentThreadId()==*(cpu->core_1->id)){ //�����ǰ�߳�id�����1��idһ�£�˵����ǰ�������߳�1ִ�� 
		core_id = 1;
		core = cpu->core_1;//��ǰ����Ϊ����1 
	}else if(GetCurrentThreadId()==*(cpu->core_2->id)){//�����ǰ�߳�id�����2��idһ�£�˵����ǰ�������߳�2ִ��
		core_id = 2;
		core = cpu->core_2;//��ǰ����Ϊ����2 
	}else{
		printf("�߳�ʶ�����\n"); 
	}
	
	/*ѭ��������ȡָ�����ִ�У�ֱ������ͣ��ָ��*/ 
	do{
		/*�������ÿ��ָ��ռ�ĸ��ֽڣ�ȡ��ǰ�����ֽڲ�ƴ��д��ir(ָ��Ĵ���)*/
		core->ir = cpu->code[(core->ip)++]&0xff; 
		core->ir = ((core->ir<<8)&0xff00)+(cpu->code[core->ip++]&0xff);
		
		/*ȡ���������ֽڲ�ƴ�ӣ���Ϊ����������ֵ��value*/ 
		core->value = cpu->code[core->ip++]&0xff;
		core->value = ((core->value<<8)&0xff00)+(cpu->code[core->ip++]&0xff);
		
		/*ָ��Ĵ�������16λָ�ǰ8λ��Ӧ�������ͣ�ͨ��λ����ȡ����ֵ��task*/ 
		core->task = core->ir>>8;
		
		/*ָ��Ĵ���9-12λ��13-16λ��Ӧ����λ1�Ͳ���λ2��λ����ȡ����ֵ��front��back*/ 
		core->front = ((core->ir<<8)>>12)&0xf;
		core->back = ((core->ir<<12)>>12)&0xf;
		
		/*����ָ��������ͣ����ö�Ӧ����*/ 
		switch(core->task)
		{
			case 1: task_1(cpu,core);break; //���ݴ���ָ�� 
			case 2:case 3:case 4:case 5: task_2(cpu,core);break; //��������ָ�� 
			case 6:case 7:case 8: task_6(cpu,core);break;        //�߼�����ָ�� 
			case 9: task_9(cpu,core);break; //�Ƚ�����ָ�� 
			case 10: task_10(cpu,core);break; //��תָ�� 
			case 11:case 12: 
				WaitForSingleObject(cpu->hMutex_state,INFINITE);//�ȴ�����ִ�е��߳�������
				task_11(cpu,core); //�������ָ�� 
				break;
			case 13:case 14:case 15:task_13(cpu,core);break; //���̲߳���ָ�� 
		}
		
		if(core->task!=11&&core->task!=12){
			WaitForSingleObject(cpu->hMutex_state,INFINITE);//�ȴ�����ִ�е��߳�������
		} 
		/*ÿ��ָ��ִ����������Ĵ���״̬*/ 
		printf("id = %d\n",core_id);
		printf("ip = %d\n",core->ip);
		printf("flag = %d\n",core->flag);
		printf("ir = %d\n",core->ir);
		printf("ax1 = %d ax2 = %d ax3 = %d ax4 = %d\n",core->ax[1],core->ax[2],core->ax[3],core->ax[4]);
		printf("ax5 = %d ax6 = %d ax7 = %d ax8 = %d\n",core->ax[5],core->ax[6],core->ax[7],core->ax[8]);
		ReleaseMutex(cpu->hMutex_state); //�ͷŻ������ 
	}while(core->task);//taskΪ0��Ϊͣ��ָ�����ѭ�� 
}

/*���ݴ���ָ���ʵ���Ǹ�ֵ����*/ 
void task_1(cpuPtr cpu,corePtr core)
{
	if(core->back==0)
		core->ax[core->front] = core->value; //����������ֵ�����ݼĴ��� 
	else if(core->back<=4)
		//�����ݼĴ����е�����ֵ����ַ�Ĵ����е�ַָ���λ�ã���д�����ݶ� 
		bin_to_char(cpu,core->ax[core->front],core->ax[core->back]); 
	else
		//�ѵ�ַ�Ĵ����е�ַָ���λ���д�ŵ�����ֵ�����ݼĴ��� 
		core->ax[core->front] = bin(cpu,core->ax[core->back]);
}

/*��������ָ��Ӽ��˳�*/ 
void task_2(cpuPtr cpu,corePtr core)
{
	if(core->back){
		//����λ1 ��һ�������ݼĴ������е��������λ2��һ���ǵ�ַ�Ĵ�����ָ������������� 
		switch(core->task)
			{
				case 2:core->ax[core->front] += bin(cpu,core->ax[core->back]);break; //�ӷ� 
				case 3:core->ax[core->front] -= bin(cpu,core->ax[core->back]);break; //���� 
				case 4:core->ax[core->front] *= bin(cpu,core->ax[core->back]);break; //�˷� 
				case 5:core->ax[core->front] /= bin(cpu,core->ax[core->back]);break; //����				
			}
	}else{
		//����λ1��Ӧ�ļĴ�����һ�������ݼĴ������е�������������������
		switch(core->task)
			{
				case 2:core->ax[core->front] += core->value;break; //�ӷ� 
				case 3:core->ax[core->front] -= core->value;break; //���� 
				case 4:core->ax[core->front] *= core->value;break; //�˷� 
				case 5:core->ax[core->front] /= core->value;break; //���� 
			}
	}	
}

/*�߼����㣬����*/ 
void task_6(cpuPtr cpu,corePtr core)
{
	if(core->back==0) //�������λ2��0�������λ1��һ�������ݼĴ����������������� 
	{
		switch(core->task) 
		{
			case 6: //�߼��� 
				core->ax[core->front] = core->ax[core->front]&&core->value;break;
			case 7: //�߼��� 
				core->ax[core->front] = core->ax[core->front]||core->value;break;
			case 8: //�߼��� 
				core->ax[core->front] = !core->ax[core->front];break;
		}
	}
	else  //����λ2��Ϊ0�������λ1(һ�������ݼĴ���)�е��������λ2(һ���ǵ�ַ�Ĵ���)ָ��������� 
	{
		switch(core->task)
		{
			case 6: //�߼��� 
				core->ax[core->front] = core->ax[core->front]&&bin(cpu,core->ax[core->back]);break;
			case 7: //�߼��� 
				core->ax[core->front] = core->ax[core->front]||bin(cpu,core->ax[core->back]);break;
			case 8: //�߼��� 
				bin_to_char(cpu,core->ax[core->back],!bin(cpu,core->ax[core->back]));break;
		}
	}
}

/*�Ƚ�����*/ 
void task_9(cpuPtr cpu,corePtr core)
{
	if(core->back==0){ 
		//����λ2Ϊ0�������λ1��һ�������ݼĴ������е������������Ƚ� 
		if(core->ax[core->front]>core->value)
			core->flag = 1;
		else if(core->ax[core->front]==core->value)
			core->flag = 0;
		else
			core->flag = -1;	
	}else{ 
		//����λ2��Ϊ0�������λ1��һ�������ݼĴ������е��������λ2��һ���ǵ�ַ�Ĵ�����ָ������Ƚ� 
		if(core->ax[core->front]>bin(cpu,core->ax[core->back]))
			core->flag = 1;
		else if(core->ax[core->front]==bin(cpu,core->ax[core->back]))
			core->flag = 0;
		else
			core->flag = -1;
	}
}

/*��תָ��*/ 
void task_10(cpuPtr cpu,corePtr core)
{
	short temp = core->ip-4;  
	switch(core->back)
	{
		case 0:core->ip = (core->value + temp)&0xffff ;break; //����λ2Ϊ0��ǿ����ת�����³��������ip 
		case 1: //����λ2Ϊ1�������־�Ĵ���flagΪ0����ת 
			if(core->flag==0){
				core->ip = (core->value + temp)&0xffff;
			}break;
		case 2: //����λ2Ϊ2�������־�Ĵ���flagΪ1����ת 
			if(core->flag==1){
				core->ip = (core->value + temp)&0xffff;
			}break;
		case 3:	//����λ2Ϊ3�������־�Ĵ���flagΪ-1����ת 
			if(core->flag==-1){
				core->ip = (core->value + temp)&0xffff;
			}break;
	}
}

/*�������ָ��*/ 
void task_11(cpuPtr cpu,corePtr core)
{
	if(core->task==11) //taskΪ11����Ӧ����ָ�� 
	{
		printf("in:\n");
		scanf("%hd",&core->ax[core->front]);
	}
	if(core->task==12) //taskΪ12����Ӧ���ָ�� 
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

/*���̲߳���*/
void task_13(cpuPtr cpu,corePtr core)
{
	switch(core->task)
	{
		case 13: WaitForSingleObject(cpu->hMutex,INFINITE);break; //���ڴ� 
		case 14: ReleaseMutex(cpu->hMutex);break; //�ͷ��ڴ棨������ 
		case 15: Sleep(core->value);break; //˯�� 
			
	}
}

short bin(cpuPtr cpu,int start)
{
	start -= N; //startΪN��16384��������ݶε�һ��Ԫ��data[0]����startת��Ϊ�����±� 
	return ((cpu->data[start]<<8)&0xff00)+(cpu->data[start+1]&0xff); //ͨ��λ���㽫����char�ϲ���һ��������short�� 
}

/*��һ��������short�����Ϊ����char��д�����ݶ�*/ 
void bin_to_char(cpuPtr cpu,int start,short number)
{
	start -= N; //startΪN��16384��������ݶε�һ��Ԫ��data[0]����startת��Ϊ�����±� 
	cpu->data[start++] = (char)(number>>8); //ǿ������ת����shortǿ��ת��Ϊchar�ǵ�λ�ض� 
	cpu->data[start] = (char)(number);
}
