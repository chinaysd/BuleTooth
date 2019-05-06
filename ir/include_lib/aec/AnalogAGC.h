#ifndef _ANALOGAGC_H_
#define _ANALOGAGC_H_

typedef struct _MIC_INIT_SET
{
	unsigned char use_magc;    //agc_sw       //
	char max_gain;    //agc最大增益//
	char min_gain;    //agc最小增益//
	char space;       //占s8       //
	int tms;          //爆掉之后的音量抑制多少ms//
	int target;       //s16_幅度阈值 lg(x)*20480//
	int sup_coeff;    //音量压制系数 Q16        //
	int pro_coeff;    //音量提升系数 Q16        //
}MIC_INIT_SET;

typedef struct
{
	int sup_cnt;    //dec
	int lenv;
	int lgain;      //Q10
	int Target;     //Q10
	int Const_sup;  //Q16
	int Const_pro;  //Q16
	int FIX_dec_cnt;
	int MAX_GainLevel;
	int MIN_GainLevel;
}Analog_AGC;

typedef struct
{
	int sup_cnt;    //dec
	int lenv;
	int lgain;      //Q10
	int Target;     //Q10
	int Const_sup;  //Q16
	int Const_pro;  //Q16
	int FIX_dec_cnt;
	int MAX_GainLevel;
	int MIN_GainLevel;
	int agc_point_cnt;
	int npoint_env;
	int retgain;
}Analog_AGC_n;

void Analog_AGC_Init_nn(Analog_AGC_n *Aagc,MIC_INIT_SET *parm);
unsigned char Analog_AGC_Process_N_n(Analog_AGC_n *Aagc,short *in,int npoint);
unsigned char Analog_AGC_Process_N(Analog_AGC *Aagc,short *in,int npoint);
void Analog_AGC_Init_N(Analog_AGC *Aagc,MIC_INIT_SET *parm);
void Analog_AGC_Updata_D(Analog_AGC_n *Aagc,int max_vol,int last_vol,int risenow);
void Analog_AGC_Set_Hold_Time(Analog_AGC *Aagc,int tms);
unsigned char Analog_AGC_Process(Analog_AGC *Aagc,short *in,int npoint);
void Analog_AGC_Init(Analog_AGC *Aagc,int Target,int TimeConstant,int MAX_GainLevel,int MIN_GainLevel);
#endif
