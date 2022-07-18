
int __stdcall MulFunctionSet (ViSession handle, int mode);
int __stdcall MulTriggerSet (ViSession handle, int mode);
int __stdcall MulDataMeasure(ViSession handle, int mode, double *measureVal);
int __stdcall MulRangeSet(ViSession handle, int mode, double range);
int __stdcall MulBandSet(ViSession handle, int mode, double freq);
int __stdcall GetMulSeries (ViSession handle, char *mader, char *type, char *code);  


#define MM_Fluke 0     //Fluke数表
#define MM_Keysight 1  //Keysight数表
#define MM_Keithley 2  //Keithley数表
#define MM_Rigol 3     //Rigol数表
#define MM_TongHui 4    //同惠数表
#define MM_Tektronix 5	//泰克数表

#define MM_DCV 0   //直流电压
#define MM_DCI 1   //直流电流
#define MM_ACV 2   //交流电压
#define MM_ACI 3   //交流电流
#define MM_RES 4   //两线电阻
#define MM_FRES 5  //四线电阻
#define MM_FREQ 6  //频率测量
#define MM_PER 7   //周期测量
#define MM_CONT 8  //连续性测试
#define MM_DIOD 9  //二极管测试
#define MM_CAP 10  //电容测试
#define MM_RTD 11  //两线温度
#define MM_FRTD 12 //四线温度						

		  
#define MM_IMM 0      //立即触发
#define MM_BUS 1      //总线触发
#define MM_EXT 2      //外部触发
#define MM_TIM 3      //计时触发
#define MM_MAN 4      //手动触发
#define MM_INT 5      //内部触发
#define MM_AUTO 6     //自动触发
#define MM_SINGLE 7   //单次触发
