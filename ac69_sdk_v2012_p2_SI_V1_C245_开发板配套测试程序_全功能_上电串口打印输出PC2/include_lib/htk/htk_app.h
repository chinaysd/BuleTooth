#ifndef HTK_APP_H_INCLUDED
#define HTK_APP_H_INCLUDED

#define HTK_ENABLE

#ifdef HTK_ENABLE

#define CONT_AD_DEFAULT_NOISE	((17<<8)|(10)) 


#define CONT_REJ_RATE	       96//数字越大识别率越高，误识别会降低

struct sr_config_s {
    char  words_cnt;                        //识别词数
    char  words_tab[10];                    //识别列表
    short  init_noise;                       //初始化噪声水平  
 	char train_flag;                        //bit 0 训练模式与否   bit 1 识别取名与否
    // int ad_const_init;                      //默认是10，值变大，识别收尾时间越短
	int rej_rate;
    unsigned char *htk_ram;                 //初始化语音识别RAM 需要20k
    int   const_addr;                       //升级数据更新起始地址
    int   cont_def_long;                    //10*1000/16  // 10ms
    int   htk_ad_flag;                      //说话状态变量
    unsigned char   *spi_flash_buf[3];                 //3个spi flash block地址
};


typedef struct sr_config_s sr_config_t;


extern int jl_htk_init(sr_config_t *htk_init,int size);
extern int htk_main(sr_config_t *htk_init);

#endif

#endif // HTK_APP_H_INCLUDED

