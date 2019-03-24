#include "common/app_cfg.h"
#include "rtos/os_api.h"
#include "rtos/os_cfg.h"
#include "common/error.h"
#include "common/msg.h"
#include "rtos/task_manage.h"
#include "sys_detect.h"
#include "dac/dac_api.h"
#include "play_sel/play_sel.h"
#include "key_drv/key.h"
#include "idle_key.h"
#include "idle.h"
#include "charge.h"

extern void *malloc_fun(void *ptr,u32 len,char *info);
extern void free_fun(void **ptr);
extern void led_close(void);
extern void irq_global_disable(void);
extern void irq_sys_disable(void);
/*----------------------------------------------------------------------------*/
/**@brief  Idle 任务
   @param  p：任务间参数传递指针
   @return 无
   @note   static void idle_task(void *p)
*/
/*----------------------------------------------------------------------------*/
static void idle_task(void *p)
{
	int msg[3];

	idle_puts("\n************************Idle TASK********************\n");
	//led_fre_set(15,0);
    //tone_play(IDLE_TASK_NAME, BPF_IDLE_MP3); //Idle提示音播放
    os_taskq_post_msg(IDLE_TASK_NAME, 1, MSG_IDLE_MODE); //触发idle模块初始化

	while(1)
	{
		memset(msg,0x00,sizeof(msg));
		os_taskq_pend(0, ARRAY_SIZE(msg), msg);
		clear_wdt();
		switch(msg[0])
		{
			case SYS_EVENT_DEL_TASK:
				if (os_task_del_req_name(OS_TASK_SELF) == OS_TASK_DEL_REQ)
				{
				    idle_puts("IDLE_SYS_EVENT_DEL_TASK\n");
				    //play_sel_stop(); //关闭提示音
					os_task_del_res_name(OS_TASK_SELF);
				}
				break;

            case SYS_EVENT_PLAY_SEL_END: //提示音结束
                idle_puts("IDLE_SYS_EVENT_PLAY_SEL_END\n");
                //play_sel_stop(); //关闭提示音
                idle_puts("IDEL_play_tone_end\n");
                os_taskq_post_msg(IDLE_TASK_NAME, 1, MSG_IDLE_MODE);
                break;

            case MSG_IDLE_MODE:
                puts("MSG_IDLE_MODE\n");
				led_close();
				ldo5v_detect_deal(POWER_OFF);
				soft_power_ctl(PWR_OFF);
                break;

            case MSG_IDLE_CTL:
                puts("MSG_IDLE_CTL\n");
                os_taskq_post_msg(MAIN_TASK_NAME, 1, MSG_EXIT_IDLE_TASK);
                break;

            case MSG_HALF_SECOND:
                //idle_puts(" Idle_H ");
                break;

			default:
				break;
		}
	}
}


/*----------------------------------------------------------------------------*/
/**@brief  IDLE 任务创建
   @param  priv：任务间参数传递指针
   @return 无
   @note   static void idle_task_init(void *priv)
*/
/*----------------------------------------------------------------------------*/
static void idle_task_init(void *priv)
{
    u32 err;
	err = os_task_create(idle_task,
			(void*)0,
			TaskIdlePrio,
			10
#if OS_TIME_SLICE_EN > 0
			,1
#endif
			,IDLE_TASK_NAME);

    if(OS_NO_ERR == err)
    {
        key_msg_register(IDLE_TASK_NAME, idle_ad_table, idle_io_table, idle_ir_table, NULL);
    }
}


/*----------------------------------------------------------------------------*/
/**@brief  IDLE 任务删除退出
   @param  无
   @return 无
   @note   static void idle_task_exit(void)
*/
/*----------------------------------------------------------------------------*/
static void idle_task_exit(void)
{
	if (os_task_del_req(IDLE_TASK_NAME) != OS_TASK_NOT_EXIST)
	{
		os_taskq_post_event(IDLE_TASK_NAME, 1, SYS_EVENT_DEL_TASK);
		do{
			OSTimeDly(1);
		} while(os_task_del_req(IDLE_TASK_NAME) != OS_TASK_NOT_EXIST);
		idle_puts("del_idle_task: succ\n");
	}
}


/*----------------------------------------------------------------------------*/
/**@brief  IDLE 任务信息
   @note   const struct task_info idle_task_info
*/
/*----------------------------------------------------------------------------*/
TASK_REGISTER(idle_task_info) = {
	.name = IDLE_TASK_NAME,
	.init = idle_task_init,
	.exit = idle_task_exit,
};
