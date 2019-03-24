#include "htk/htk.h"
#include "htk/htk_app.h"
#include "dac/ladc_api.h"
#include "cpu/dac_param.h"
#include "file_operate/file_op.h"
#include "fat/fs_io.h"
#include "fat/tff.h"
#include "file_operate/logic_dev_list.h"
#include "rtos/os_api.h"
#include "common/app_cfg.h"

#if  USE_HTK

#define HTK_DEBUG_ENABLE
#ifdef HTK_DEBUG_ENABLE
#define htk_printf printf
#else
#define htk_printf(...)
#endif

#define SIZEOF_ALIN(var,al)     ((((var)+(al)-1)/(al))*(al))

#define HTK_PCM_BUF_NUMS            (8)
#define HTK_PCM_BUF_ONE_SIZE        (1024*2)
#define HTK_RAM_LEN                 (25*1024)
#define HTK_GET_VOICE_DATA_LENGTH   (256)
#define HTK_FILE_NAME               "jlhtk.bin"

#define SPI_CACHE_READ_ADDR(addr)   (u32)(0x1000000 +(((u32)addr)))

#define HTK_TIMEOUT 30  
typedef enum {
   HTK_CMD_YES = 0x0,                                                                                          
   HTK_CMD_NO = 0x1,                                                                                          
   HTK_CMD_NO_N = 0x2,                                                                                          
   HTK_CMD_MAX,
}HTK_CMD;

typedef struct __MSG_IO {
    void *priv;
    msg_cb_fun  msg_cb;
} MSG_IO;


struct __HTK_PLAYER {
    u8              *pcm_buf;
    sr_config_t     *config;
    cbuffer_t        pcm_cbuf;
    OS_SEM           pcm_sem;
    u32              pcm_lost;
    u8				 msg_break;
    MSG_IO           msg;
};
typedef struct __HTK_PLAYER HTK_PLAYER;
HTK_PLAYER *htk_hdl = NULL;
volatile u8 htk_enable=0;
extern lg_dev_list *lg_dev_find(u32 drv);
extern tbool syd_drive_open(void **p_fs_hdl, void *p_hdev, u32 drive_base);
extern tbool syd_drive_close(void **p_fs_hdl);
extern bool syd_get_file_byindex(void *p_fs_hdl, void **p_f_hdl, u32 file_number, char *lfn);
extern bool syd_file_close(void *p_fs_hdl, void **p_f_hdl);
extern u16 syd_read(void *p_f_hdl, u8 _xdata *buff, u16 len);
extern bool syd_get_file_bypath(void *p_fs_hdl, void **p_f_hdl, u8 *path, char *lfn);
extern tbool syd_get_file_addr(u32 *addr, void **p_f_hdl);
extern void clear_wdt(void);//清看门狗

extern void ladc_reg_init(u8 ch_sel, u16 sr);
#if 1
static tbool open_htk_file(char *filename, int *address)
{
    /* htk_printf("\n open htk file ========="); */
    int htk_source_address;
    lg_dev_list *pnode;
    void *fs_hdl = NULL;  //文件系统句柄
    void *file_hdl = NULL;//文件句柄
    pnode = lg_dev_find('A');

    if (!syd_drive_open(&fs_hdl, pnode->cur_dev.lg_hdl->phydev_item, 0)) { //打开文件系统
        htk_printf("open file system errot \n");
        return false;
    }
    if (!syd_get_file_bypath(fs_hdl, &file_hdl, (u8 *)filename, (char *)filename)) { //打开1号文件
        //以下两部顺序不能变
        syd_file_close(fs_hdl, &file_hdl); //失败的情况下关闭文件
        syd_drive_close(&fs_hdl);//失败的情况下关闭文件系统
        htk_printf("open file errot \n");
        return false;
    }

    if (!syd_get_file_addr((u32 *)&htk_source_address, &file_hdl)) {
        htk_printf("syd_get_file_addr false \n");
        syd_file_close(fs_hdl, &file_hdl); //失败的情况下关闭文件
        syd_drive_close(&fs_hdl);//失败的情况下关闭文件系统
        return false;
    }
    htk_source_address = SPI_CACHE_READ_ADDR(htk_source_address);

    syd_file_close(fs_hdl, &file_hdl); //完成操作后，关闭文件
    syd_drive_close(&fs_hdl);  //完成操作后，关闭文件系统
    /* htk_printf("open htk file ok\r"); */

    if (address == NULL) {
        return false;
    }
    *address = htk_source_address;
    return true;
}
#endif

void write_flash_htk(void)
{
    ;
}

void read_flash_htk(void)
{
    ;
}

void erase_flash_htk(void)
{
    ;
}
void read_change_ptr(void)
{
    ;
}

void set_htk_enable(u8 en)
{
	htk_enable =en;
		
}
u8 get_htk_state()
{
	return 	htk_enable;
}

static HTK_PLAYER *htk_player_creat(void)
{
    HTK_PLAYER *htk;
    u32 buf_len = (SIZEOF_ALIN(sizeof(HTK_PLAYER), 4)
                   + SIZEOF_ALIN(sizeof(sr_config_t), 4)
                   + SIZEOF_ALIN(HTK_PCM_BUF_ONE_SIZE, 4)
                   + SIZEOF_ALIN(HTK_RAM_LEN, 4));

    htk_printf("buf_len:%d\r", buf_len);
    u32 buf_index;
    u8 *ctr_buf = (u8 *)calloc(1, buf_len);
    if (ctr_buf == NULL) {
        return NULL;
    }

    buf_index = 0;
    htk = (HTK_PLAYER *)(ctr_buf + buf_index);

    buf_index += SIZEOF_ALIN(sizeof(HTK_PLAYER), 4);
    htk->config = (sr_config_t *)(ctr_buf + buf_index);


    buf_index += SIZEOF_ALIN(sizeof(sr_config_t), 4);
    htk->pcm_buf = (u8 *)(ctr_buf + buf_index);

    buf_index += SIZEOF_ALIN(HTK_PCM_BUF_ONE_SIZE, 4);
    htk->config->htk_ram = (unsigned char *)(ctr_buf + buf_index);

    os_sem_create(&(htk->pcm_sem), 0);
    cbuf_init(&(htk->pcm_cbuf), htk->pcm_buf, HTK_PCM_BUF_ONE_SIZE);
    htk->pcm_lost = 0;

    return htk;
}


static void htk_player_destroy(HTK_PLAYER **hdl)
{
    if ((hdl == NULL) || (*hdl == NULL)) {
        return ;
    }
	CPU_INT_DIS();
    free(*hdl);
    *hdl = NULL;
	CPU_INT_EN();
}


static u8 my_flag=0;
void htk_isr_callback(void *ladc_buf, u32 buf_flag, u32 buf_len)
{
    s16 *res;
    s16 *ladc_mic;

	if(!get_htk_state())
	{
		return ;	
	}

    //dual buf
    res = (s16 *)ladc_buf;
    res = res + (buf_flag * DAC_SAMPLE_POINT);

    //ladc_buf offset
    /* ladc_l = res; */
    /* ladc_r = res + DAC_DUAL_BUF_LEN; */
    ladc_mic = res + (2 * DAC_DUAL_BUF_LEN);

	if(!htk_hdl)
	{
		return ;	
	}
    cbuffer_t *cbuffer = &(htk_hdl->pcm_cbuf);
    if ((cbuffer->total_len - cbuffer->data_len) < DAC_DUAL_BUF_LEN) {
        htk_hdl->pcm_lost ++;
    } else {
		if(my_flag==0)
		{
           htk_hdl->pcm_lost =0;
            my_flag=1;
		}
        cbuf_write(cbuffer, (void *)ladc_mic, DAC_DUAL_BUF_LEN);
    }

    if (cbuffer->data_len >= HTK_GET_VOICE_DATA_LENGTH) {
        os_sem_set(&(htk_hdl->pcm_sem), 0);
        os_sem_post(&(htk_hdl->pcm_sem));
    }
}
int get_ad_voice(void *inout_spch, int len)
{
    while (1) {
        if (cbuf_get_data_size(&(htk_hdl->pcm_cbuf)) >= HTK_GET_VOICE_DATA_LENGTH) {
            cbuf_read(&(htk_hdl->pcm_cbuf), inout_spch, HTK_GET_VOICE_DATA_LENGTH);
            break;
        }
        os_sem_pend(&(htk_hdl->pcm_sem), 0);
    }

    return 1;
}
int get_ad_key(void)
{
    if (htk_hdl == NULL) {
        return 1;
    }
    int ret = 0;
    if (htk_hdl->msg.msg_cb) {
        ret = htk_hdl->msg.msg_cb(htk_hdl->msg.priv);
    }
    if (ret) {
        htk_hdl->msg_break = 1;
    }

    return ret;
}

static int htk_msg_callback(void *priv)
{
    u8 ret;
    int msg[6];
    while (1) {
        memset((u8 *)msg, 0x0, (sizeof(msg) / sizeof(msg[0])));
        ret = os_taskq_accept_no_clean(ARRAY_SIZE(msg), msg);
        clear_wdt();
        if (ret == OS_Q_EMPTY) {
            break;
        } else {
            if (msg[0] == SYS_EVENT_DEL_TASK) {
                return 1;
            } else {
                os_taskq_msg_clean(msg[0], 1);
            }
        }
    }
    return 0;
}
static tbool htk_action_callback(void *priv, char result[], char nums, u8 isTimeout)
{
    HTK_CMD action = (HTK_CMD)result[0];
    /* NOTICE_PlAYER_ERR n_err; */
    tbool ret = false;
    void *htk_op = priv;
	/*
    if (htk_op == NULL) {
        return false;
    }
	*/

    if ((isTimeout == 1) || (nums == 0)) {
        htk_printf("isTimeout or nums == 0\n");
        return ret;
    } else {
        if (action < HTK_CMD_MAX) {
			htk_printf("htk ack=0x%x\n",action );
            switch (action) {
            case HTK_CMD_YES:
				os_taskq_post_msg("btmsg", 1, MSG_BT_ANSWER_CALL);
                ret = true;
                break;
			case HTK_CMD_NO:
			case HTK_CMD_NO_N:
				os_taskq_post_msg("btmsg", 1, MSG_BT_CALL_REJECT);
                ret = true;
                break;
            default:
                break;

            }
            return ret;
        } else {
            htk_printf("----%d---- htk ack is unknowni!!!\n");
            return ret;
        }
    }
    return ret;
}


/*----------------------------------------------------------------------------*/
/**@brief   语音识别及处理流程接口
   @param   priv:用户调用可以传递的私有参数, 将会被传递给act_cb和msg_cb
   @param   act_cb:识别完成后的处理过程
   @param   msg_cb:识别过程中响应消息处理， 可用于打断语音识别过程
   @param   timout:识别超时配置
   @return  void
   @note
*/
/*----------------------------------------------------------------------------*/
tbool htk_player_process(void *priv, act_cb_fun act_cb, msg_cb_fun msg_cb, u16 timeout)
{
    HTK_PLAYER *tem_htk_hdl = NULL;
    tbool res = true;
    u32 cpu_sr;
    int htk_file_addr;
    char  words_tab[10] = {0};
    char  words_cnt;
    int   htk_res;
    short htk_init_noise = CONT_AD_DEFAULT_NOISE;
	tem_htk_hdl= htk_player_creat();

    if ((tem_htk_hdl == NULL) || (tem_htk_hdl->config == NULL))
   	{
        res = false;
        goto ERR;
    }
    res = open_htk_file(HTK_FILE_NAME, &htk_file_addr);
    if (res == false) 
    {	
        htk_printf("htk file open fail !!\n");
        goto ERR;
   	}
    tem_htk_hdl->msg_break = 0;
    tem_htk_hdl->msg.priv = priv;
    tem_htk_hdl->msg.msg_cb = msg_cb;
    sr_config_t *config = tem_htk_hdl->config;
    config->cont_def_long = timeout * 1000 / 16;
    config->const_addr = htk_file_addr;
    config->init_noise = htk_init_noise;
  
    config->rej_rate = CONT_REJ_RATE;
    config->words_cnt = 0;
    CPU_INT_DIS();
    htk_hdl = tem_htk_hdl;
    CPU_INT_EN();
  
    jl_htk_init(htk_hdl->config,25*1024);	
	while (1)
   	{
        /* htk_printf("----------------------htk start----------------------------------\n"); */
		htk_printf("\n--htk start>>>>>>>>\n");
        htk_res = htk_main(htk_hdl->config);
        if (htk_hdl->msg_break == 0)
	   	{
            words_cnt = config->words_cnt;
            memcpy((u8 *)words_tab, (u8 *)config->words_tab, sizeof(words_tab));

            htk_printf("config->words_tab[0] :%d, cnt = %d, htk_res = %d, pcm lost = %d\r", \
                       config->words_tab[0], words_cnt, htk_res, htk_hdl->pcm_lost);

            ///deal result
          if (act_cb) 
		  {
                tbool act_res = act_cb(priv, words_tab, words_cnt, htk_res);
                if (act_res == true) {
                    res = true;
					htk_player_destroy(&htk_hdl);
                    break;
                } else {
                    res = false;
                    continue;
                }
            } 
		    else 
			{
                htk_printf("HTK nothing to do without cb\r");
                res = false;
                break;
            }
        }
	   	else 
		{
            htk_printf("msg break htk !!\n");
            res = false;
            break;
        }

    }
ERR:

    htk_player_destroy(&htk_hdl);
    return res;
}
static void htk_task(void *parg)
{
    int msg[6] = {0};
    /* void *htk_op = parg; */
    tbool ret;
    malloc_stats();
    set_htk_enable(1);
	
	puts("htk_task start\n");
    while(1)
    {
        memset(msg,0x00,sizeof(msg));
        os_taskq_pend(0, ARRAY_SIZE(msg), msg);
        clear_wdt();
		switch(msg[0])
		{
			case MSG_HTK_LOOP_CMD:
                ladc_reg_init(ENC_MIC_CHANNEL,LADC_SR8000);
		        ladc_mic_gain(22, 0);//根据不同的mic调下mic增益
				ret = htk_player_process(NULL, htk_action_callback, htk_msg_callback, HTK_TIMEOUT);
                ladc_disable(ENC_MIC_CHANNEL);
				/* printf("MSG_HTK_LOOP_CMD=0x%x\n", ret); */
				break;
			case SYS_EVENT_DEL_TASK:
                if (os_task_del_req_name(OS_TASK_SELF) == OS_TASK_DEL_REQ) {
                    printf("htk_del \r");
                    os_task_del_res_name(OS_TASK_SELF);
                }
				break;
            break;
        default:
            break;
				
		}
	}
}
extern tbool mutex_resource_apply(char *resource, int prio , void (*apply_response)(), void (*release_request)());
extern tbool mutex_resource_release(char *resource);
void htk_mutex_start()
{
	puts("-------htk_mutex_start\n");
		
}
void htk_mutex_stop()
{
    puts("------htk_mutex_stop\n");		
}


void htk_start(u8 type)//iphone连接来电话时,内存资源不够，如果来电话播完提示音再启动,type参数为1
{
	if(get_htk_state())
	{
	    puts("----htk_start\n");
		if(type)
		{
		   puts("htk mutex_resource_apply\n");
	       mutex_resource_apply("htk",5,htk_mutex_start,htk_mutex_stop);
		}
	    os_taskq_post("HTKTask",1,MSG_HTK_LOOP_CMD);
			
	}
}
void htk_task_init(void)
{
    u32 err;
	if(get_htk_state())
	{
		return ;	
	}
	puts("htk_task_init\n");
    err = os_task_create_ext(htk_task,
                             (void*)0,
                             TaskBtAecSDPrio,
                             32,
#if OS_TIME_SLICE_EN > 0
                             1,
#endif
                             "HTKTask",
                             TONE_PHY_TASK_STACK_SIZE
                            );
    if(OS_NO_ERR != err)
    {
        printf("htk_start err =%x \n",err);
    }

}
void htk_task_exit(void)
{
	puts("htk_task_exit\n");
	if (os_task_del_req("HTKTask") != OS_TASK_NOT_EXIST)
    {	
	     puts("del_HTK_task");
		 os_taskq_post_event("HTKTask", 1, SYS_EVENT_DEL_TASK);
		 do
		 {	
		      OSTimeDly(1);
		 }
		 while(os_task_del_req("HTKTask") != OS_TASK_NOT_EXIST);
         mutex_resource_release("htk");
         set_htk_enable(0);
		 puts("del_HTK_task: succ\n");
	}
}	
#endif
