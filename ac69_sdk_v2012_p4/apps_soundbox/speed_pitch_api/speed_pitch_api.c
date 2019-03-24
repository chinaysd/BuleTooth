#include "speed_pitch_api.h"
#include "speed_pitch/ps_for69_api.h"
#include "rtos/os_api.h"
#include "common/printf.h"
#include "string.h"
#include "mem/malloc.h"
#include "uart.h"

#define PS_DEBUG_ENABLE
#ifdef PS_DEBUG_ENABLE
#define ps_printf	printf
#else
#define ps_printf(...)
#endif//PS_DEBUG_ENABLE


#define SIZEOF_ALIN(var, al)	((((var) + (al) - 1)/(al))*(al))

typedef struct __PS69_OPS {
    void 				*hdl;
    PS69_API_CONTEXT 	*_io;
} PS69_OPS;

struct __PS_OBJ {
    PS69_OPS 			 ops;
    PS69_CONTEXT_CONF	 config;
    PS69_audio_IO		 output;
    DEC_SET_INFO_CB		 set_info;
    OS_SEM		 	 	 mutex;
    volatile u8			 set_flag;
};




/*----------------------------------------------------------------------------*/
/**@brief 变速变调资源创建
   @param void
   @return 变速变调控制句柄
   @note
*/
/*----------------------------------------------------------------------------*/
PS_OBJ	*speed_pitch_creat(void)
{
    PS_OBJ *obj;
    u8 *need_buf;
    u32 need_buf_len;

    PS69_API_CONTEXT *tmp_io = get_ps69_api();

    need_buf_len = SIZEOF_ALIN(sizeof(PS_OBJ), 4)
                   + SIZEOF_ALIN(tmp_io->need_size(), 4);

    ps_printf("need_buf_len = %d!!\n", need_buf_len);

    need_buf = (u8 *)calloc(1, need_buf_len);
    if (need_buf == NULL) {
        return NULL;
    }

    obj = (PS_OBJ *)need_buf;
    need_buf += SIZEOF_ALIN(sizeof(PS_OBJ), 4);

    obj->ops.hdl = (void *)need_buf;
    obj->ops._io = tmp_io;

    if (os_mutex_create(&obj->mutex) != OS_NO_ERR) {
        speed_pitch_destroy(&obj);
        return NULL;
    }

    return obj;
}


/*----------------------------------------------------------------------------*/
/**@brief 变速变调资源释放
   @param 控制句柄，注意是双重指针
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void speed_pitch_destroy(PS_OBJ **obj)
{
    if (obj == NULL || (*obj) == NULL) {
        return ;
    }

    free(*obj);
    *obj = NULL;
}


/*----------------------------------------------------------------------------*/
/**@brief 设置变速变调数据输出回调注册
   @param obj:控制句柄，cbk：要注册的回调函数，priv：回调函数私有参数
   @return void
   @note
*/
/*----------------------------------------------------------------------------*/
void speed_pitch_set_output(PS_OBJ *obj, void *cbk, void *priv)
{
    if (obj == NULL) {
        return ;
    }
    obj->output.outpriv = priv;
    obj->output.output = (void *)cbk;
    obj->ops._io->open(obj->ops.hdl, &(obj->output));
}

/*----------------------------------------------------------------------------*/
/**@brief 设置变速变调采样率
   @param obj:控制句柄，sr:采样率
   @return void
   @note
*/
/*----------------------------------------------------------------------------*/
void speed_pitch_set_samplerate(PS_OBJ *obj, u16 sr)
{
    if (obj == NULL) {
        return ;
    }

    os_mutex_pend(&obj->mutex, 0);
    obj->config.sr = sr;
    obj->set_flag = 1;
    os_mutex_post(&obj->mutex);
}

/*----------------------------------------------------------------------------*/
/**@brief 设置变速变调采样率回调函数注册
   @param obj:控制句柄，cbk:需要注册的回调函数，priv：回调函数私有参数
   @return void
   @note
*/
/*----------------------------------------------------------------------------*/
void speed_pitch_set_set_info_cbk(PS_OBJ *obj, u32(*cbk)(void *priv, dec_inf_t *inf, tbool wait), void *priv)
{
    if (obj == NULL) {
        return ;
    }
    obj->set_info.priv = priv;
    obj->set_info._cb = (void *)cbk;
}

/*----------------------------------------------------------------------------*/
/**@brief 设置变速变调采声道数
   @param obj:控制句柄，track:声道个数
   @return void
   @note
*/
/*----------------------------------------------------------------------------*/
void speed_pitch_set_track_nums(PS_OBJ *obj, u16 track)
{
    if (obj == NULL) {
        return ;
    }

    os_mutex_pend(&obj->mutex, 0);
    obj->config.chn = track;
    obj->set_flag = 1;
    os_mutex_post(&obj->mutex);
}

/*----------------------------------------------------------------------------*/
/**@brief 设置变速变调的变速参数
   @param obj:控制句柄，val:变速参数值
   @return void
   @note:变数参数说明，>80变快，<80变慢,建议范围：40_160, 但是20-200也有效
*/
/*----------------------------------------------------------------------------*/
void speed_pitch_set_speed(PS_OBJ *obj, u16 val)
{
    if (obj == NULL) {
        return ;
    }

    os_mutex_pend(&obj->mutex, 0);
    obj->config.speedV = val;
    obj->set_flag = 1;
    os_mutex_post(&obj->mutex);
}

/*----------------------------------------------------------------------------*/
/**@brief 设置变速变调的变调参数
   @param obj:控制句柄，val:变调参数值
   @return void
   @note:变调参数说明：>32768音调高，<32768音调低，变调比例pitchV/32768
*/
/*----------------------------------------------------------------------------*/
void speed_pitch_set_pitch(PS_OBJ *obj, u16 val)
{
    if (obj == NULL) {
        return ;
    }

    os_mutex_pend(&obj->mutex, 0);
    obj->config.pitchV = val;
    obj->set_flag = 1;
    os_mutex_post(&obj->mutex);
}


/*----------------------------------------------------------------------------*/
/**@brief 获取当前变速参数值
   @param obj:控制句柄
   @return 当前值
   @note
*/
/*----------------------------------------------------------------------------*/
u16 speed_pitch_get_cur_speed(PS_OBJ *obj)
{
    if (obj == NULL) {
        return 0;
    }
    return obj->config.speedV;
}

/*----------------------------------------------------------------------------*/
/**@brief 获取当前变调参数值
   @param obj:控制句柄
   @return 当前值
   @note
*/
/*----------------------------------------------------------------------------*/
u16 speed_pitch_get_cur_pitch(PS_OBJ *obj)
{
    if (obj == NULL) {
        return 0;
    }
    return obj->config.pitchV;
}

/*----------------------------------------------------------------------------*/
/**@brief 变数变调执行函数，需要被解码输出调用
   @param obj:控制句柄
   @return 当前值
   @note
*/
/*----------------------------------------------------------------------------*/
void speed_pitch_main(PS_OBJ *obj, void *buf, u32 len)
{
    if (obj == NULL) {
        return ;
    }

    os_mutex_pend(&obj->mutex, 0);
    if (obj->set_flag) {
        obj->ops._io->dconfig(obj->ops.hdl, &(obj->config));
        obj->set_flag = 0;
    }
    os_mutex_post(&obj->mutex);

    obj->ops._io->run(obj->ops.hdl, buf, len);
}


