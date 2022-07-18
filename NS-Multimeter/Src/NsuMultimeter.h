
int __stdcall MulFunctionSet (ViSession handle, int mode);
int __stdcall MulTriggerSet (ViSession handle, int mode);
int __stdcall MulDataMeasure(ViSession handle, int mode, double *measureVal);
int __stdcall MulRangeSet(ViSession handle, int mode, double range);
int __stdcall MulBandSet(ViSession handle, int mode, double freq);
int __stdcall GetMulSeries (ViSession handle, char *mader, char *type, char *code);  


#define MM_Fluke 0     //Fluke����
#define MM_Keysight 1  //Keysight����
#define MM_Keithley 2  //Keithley����
#define MM_Rigol 3     //Rigol����
#define MM_TongHui 4    //ͬ������
#define MM_Tektronix 5	//̩������

#define MM_DCV 0   //ֱ����ѹ
#define MM_DCI 1   //ֱ������
#define MM_ACV 2   //������ѹ
#define MM_ACI 3   //��������
#define MM_RES 4   //���ߵ���
#define MM_FRES 5  //���ߵ���
#define MM_FREQ 6  //Ƶ�ʲ���
#define MM_PER 7   //���ڲ���
#define MM_CONT 8  //�����Բ���
#define MM_DIOD 9  //�����ܲ���
#define MM_CAP 10  //���ݲ���
#define MM_RTD 11  //�����¶�
#define MM_FRTD 12 //�����¶�						

		  
#define MM_IMM 0      //��������
#define MM_BUS 1      //���ߴ���
#define MM_EXT 2      //�ⲿ����
#define MM_TIM 3      //��ʱ����
#define MM_MAN 4      //�ֶ�����
#define MM_INT 5      //�ڲ�����
#define MM_AUTO 6     //�Զ�����
#define MM_SINGLE 7   //���δ���