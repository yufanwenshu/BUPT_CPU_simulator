#ifndef CPU_CLOCK
#define CPU_CLOCK
void read(void); //ָ����أ���"dict.dic"�ж�ȡָ�д������ 
void process(void); //����ȡָ�����ִ�� 
void task_1(short task,short front,short back,short value); //���ݴ���ָ�� 
void task_2(short task,short front,short back,short value); //��������ָ�� 
void task_6(short task,short front,short back,short value); //�߼�����ָ�� 
void task_9(short front,short back,short value); //�Ƚ�ָ�� 
void task_10(short back,short value); //��תָ�� 
void task_11(short task,short front); //�������ָ�� 
void end_output(void); //����ָ��ִ������������κ����ݶ���Ϣ 
short bin(int start);  //�����ݶζ�ȡ��Ϣ������char����ת��Ϊ16λ������������short�ͣ� 
void bin_to_char(int start,short number); //��16λ����������(short��)ת��Ϊ����char���������ݶ� 
#endif
