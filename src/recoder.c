/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        recoder.c
 * Author:           李耀轩       
 * Version:          1.0
 * Date:             2019-09-20
 * Description:      
 * Others:      
 * Function List:    

 * History: 
    1. Date:         2019-09-20
       Author:       李耀轩
       Modification: 创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

 */

#include "recoder.h"
#include "gm_type.h"
#include "utility.h"
#include "gm_memory.h"
#include "log_service.h"
#include "config_service.h"
#include "gps_service.h"
#include "gprs.h"
#include "gm_fs.h"
#include "gm_stdlib.h"
#include "gm_record.h"
#include "gm_timer.h"
#include "system_state.h"
#include "json.h"
#include "auto_test.h"

#define RECODER_STATUS_STRING_MAX_LEN 30
#define MAX_VOICE_FILE 3
#define ONE_VOICE_PACKT_MAXLEN  900
#define VOICE_FILE_NAME_MAX_LEN 30
#define VOICE_FILE_NAME   "Z:\\goome\\voice%d.amr\0"
#define OFFSETOF(TYPE,num)  (u32)&(((TYPE *)0)->num)
#define ASSERTADR(_addr) if(!_addr)break;
#define RECODER_FILE_SEND_INTERVAL 10
#define RECODER_FILE_SEND_COUNT 5
typedef void (*result_cb_fun)(u8 state);




typedef enum
{
    AUD_VOLUME_CTN = 0,                     /* 0: tone */
    AUD_VOLUME_KEY,                         /* 1: keytone */
    AUD_VOLUME_MIC,                         /* 2: microphone */
    AUD_VOLUME_FMR,                         /* 3: FM Radio */
    AUD_VOLUME_SPH,                         /* 4: Speech */
    AUD_VOLUME_SID,                         /* 5: Side-tone */
    AUD_VOLUME_MEDIA,                       /* 6: Multi-Media */
    AUD_VOLUME_TVO,                         /* 7: TV-OUT */
    AUD_VOLUME_ATV,                         /* 8: ATV */
    
    AUD_MAX_VOLUME_TYPE
}aud_volume_enum;


typedef enum 
{
	CURRENT_RECODER_INIT = 0, //初始化状态
	CURRENT_RECODER_WORK = 1, //已初始化，等待录音指令
	CURRENT_RECODER_FAIL = 2, //失败
	
	CURRENT_RECODER_STATUS_MAX
}RecoderStatusEnum;

typedef enum
{
	RECD_IDLE, //空闲
	RECD_INIT, //收到录音指令，准备录音
	RECD_GOING, //正在录音
	RECD_BROK,  //录音完成

	RECD_END
}RECD_STATE_ENUM;

typedef enum
{
	VOICE_QUALITY_LOW,      /* Low quality */
	VOICE_QUALITY_MED,      /* Medium quality */
	VOICE_QUALITY_HIGH,     /* High quality */
	VOICE_QUALITY_AUTO      /* Auto profile */
}VOICE_FILE_QUALITY_ENUM;

typedef enum
{
	VOICE_UP_NONE,
	VOICE_UP_COMPLETE,
	VOICE_UPLOADING,
	VOICE_UP_END
}VOICE_FILE_UPLOAD_ENUM;

typedef enum
{
	RES_ILLGLE,
	RES_RECD_SUCESS,
	RES_RECD_TERMINATED,
	RES_RECD_DISC_FULL,
	RES_RECD_END_OF_FILE,
	RES_RECD_NO_SPACE
}RecodeResultEnum;


typedef struct
{
	u8 ret;
	void *user;
}sound_recoder_cb_struct;


typedef struct
{
	u8 in;
	u8 out;
	u8 max;
}voice_file_queue_struct;

typedef struct
{
	u16 filename[VOICE_FILE_NAME_MAX_LEN];
	int f_handle;
	u8 time[6];
	u32 file_size; //录音文件总长度
	u64 ack_map;
	u64 send_map;
	u16 send_timeout;
	u16 total_pack;
	u8 lockflg;
	bool complete;
}voice_file_struct;


typedef struct
{
    u32 recd_clock;
    u32 send_clock;
    u8 send_count;
    u8 send_index;
    RecoderResStateEnum res_state;
	RECD_STATE_ENUM   state;
	VOICE_FILE_QUALITY_ENUM quality;
	VOICE_FILE_UPLOAD_ENUM up;
	RecoderSendRegStruct op; 
	u32 lim_time;
	u32 lim_size;
	voice_file_queue_struct queue;
	voice_file_struct voice[MAX_VOICE_FILE];
}recoder_ctrl_struct;

recoder_ctrl_struct s_recoder_ctrl;


typedef struct 
{
	bool inited;
	RecoderStatusEnum status;
	u32 clock;
	u8 fail_count;
	result_cb_fun call_back_func;
}RecoderStruct;

RecoderStruct s_recoder;


const char s_recoder_status_string[CURRENT_RECODER_STATUS_MAX][RECODER_STATUS_STRING_MAX_LEN] = 
{
    "CURRENT_RECODER_INIT",
    "CURRENT_RECODER_WORK",
    "CURRENT_RECODER_FAIL",
};


GM_ERRCODE recoder_register(result_cb_fun call_back_func);



const char * recoder_status_string(RecoderStatusEnum statu)
{
    return s_recoder_status_string[statu];
}

static GM_ERRCODE recoder_transfer_status(RecoderStatusEnum new_status)
{
    RecoderStatusEnum old_status = s_recoder.status;
    GM_ERRCODE ret = GM_PARAM_ERROR;
    switch(s_recoder.status)
    {
        case CURRENT_RECODER_INIT:
            switch(new_status)
            {
                case CURRENT_RECODER_INIT:
                    break;
                case CURRENT_RECODER_WORK:
					ret = GM_SUCCESS;
                    break;
				case CURRENT_RECODER_FAIL:
					ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case CURRENT_RECODER_WORK:
            switch(new_status)
            {
                case CURRENT_RECODER_INIT:
                	ret = GM_SUCCESS;
                    break;
                case CURRENT_RECODER_WORK:
                    break;
				case CURRENT_RECODER_FAIL:
					ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
		case CURRENT_RECODER_FAIL:
            switch(new_status)
            {
                case CURRENT_RECODER_INIT:
                	ret = GM_SUCCESS;
                    break;
                case CURRENT_RECODER_WORK:
                    break;
				case CURRENT_RECODER_FAIL:
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }


    if(GM_SUCCESS == ret)
    {
        s_recoder.status = new_status;
        LOG(INFO,"clock(%d) recoder_transfer_status from %s to %s success", util_clock(), recoder_status_string(old_status),recoder_status_string(new_status));
    }
    else
    {
        LOG(WARN,"clock(%d) recoder_transfer_status assert(from %s to %s) failed", util_clock(), recoder_status_string(old_status),recoder_status_string(new_status));
    }

    return ret;

}



GM_ERRCODE recoder_control_init(void)
{
	if(s_recoder_ctrl.op.data) 
	{
		GM_MemoryFree(s_recoder_ctrl.op.data);
		s_recoder_ctrl.op.data =NULL;
	}
	GM_memset((u8 *)&s_recoder_ctrl,0,sizeof(recoder_ctrl_struct));
	s_recoder_ctrl.state = RECD_INIT;
	s_recoder_ctrl.lim_time = (get_auto_test_state()) ? 10 :30; //最大录音30秒
	s_recoder_ctrl.quality  = VOICE_QUALITY_LOW; //录音质量采用低质量
	s_recoder_ctrl.lim_size = 0; //不限制文件大小
	return GM_SUCCESS;
}



RecoderResStateEnum get_recoder_response_state(void)
{
	return s_recoder_ctrl.res_state;
}


static void delete_voice_file(voice_file_struct *voc)
{
	GM_FS_Close(voc->f_handle);
	GM_FS_Delete(voc->filename);
	GM_memset((u8 *)voc->time, 0, (sizeof(voice_file_struct)-OFFSETOF(voice_file_struct,time[0])));
	voc->f_handle = -1;
}


static GM_ERRCODE open_voice_file(voice_file_struct *voc)
{
	u16 idx = 0;
	UINT f_size;

	if (voc->f_handle >= 0)
	{
		return GM_SUCCESS;
	}
	
	voc->f_handle = GM_FS_Open(voc->filename, GM_FS_READ_ONLY | GM_FS_ATTR_ARCHIVE);
	voc->file_size = 0;
	if(voc->f_handle >= 0)
	{
		if(GM_FS_GetFileSize(voc->f_handle, &f_size) >= 0)
		{
			voc->file_size = f_size;
			voc->total_pack = f_size/ONE_VOICE_PACKT_MAXLEN;
			if(f_size%ONE_VOICE_PACKT_MAXLEN)
			{
				voc->total_pack += 1;
			}
			voc->ack_map = 0;
			for(idx = 0 ; idx < voc->total_pack; idx++)
			{
				voc->ack_map    |= ((u64)1<<idx);
				voc->send_map |= ((u64)1<<idx);
			}
			LOG(INFO, "clock(%d) open_voice_file file_size(%d) total_pack(%d)", util_clock(), voc->file_size, voc->total_pack);
		}
		else
		{
			LOG(WARN, "clock(%d) recoder_voice_file_upload_proc get recd file size fail.", util_clock());
			GM_FS_Close(voc->f_handle);
			return GM_SYSTEM_ERROR;
		}
	}
	else
	{
		LOG(WARN, "clock(%d) recoder_voice_file_upload_proc open recd file fail.", util_clock());
		return GM_SYSTEM_ERROR;
	}

	return GM_SUCCESS;
}


u8 find_voice_file_index_by_time(u8 *time)
{
	u8 index = 0;
	u8 idx = 0;
	voice_file_struct *voc = s_recoder_ctrl.voice;

	for (index=0; index<3;index++)
	{
		for (idx=0; idx<6;idx++)
		{
			if (voc[index].time[idx] != time[idx])
			{
				continue;
			}
			else if (idx >= 5)
			{
				return index;
			}
		}
	}

	return index;
}


void recoder_file_send_ack(u8 *ack_msg)
{
	voice_file_struct *voc = s_recoder_ctrl.voice;

	if (s_recoder_ctrl.send_index == find_voice_file_index_by_time(&ack_msg[1]))
	{
		LOG(DEBUG , "clock(%d) recoder_file_send_ack ackmap(%d)", util_clock(), ack_msg[0]);
		voc[s_recoder_ctrl.send_index].ack_map &= (~((u64)1<<ack_msg[0]));
		s_recoder_ctrl.send_count = 0;
	}
	else
	{
		LOG(WARN , "clock(%d) recoder_file_send_ack ackmap(%d) time:%02d%02d%02d%02d%02d%02d", util_clock(), ack_msg[0], ack_msg[1],ack_msg[2],ack_msg[3],ack_msg[4],ack_msg[5],ack_msg[6]);
	}
}



void recoder_voice_file_upload_proc(void)
{
	voice_file_queue_struct *que = &s_recoder_ctrl.queue;
	voice_file_struct *voc = s_recoder_ctrl.voice;
	RecoderSendRegStruct *op= &s_recoder_ctrl.op;
	u16 i,j;
	UINT f_size,r_size;

	if (s_recoder_ctrl.up < VOICE_UP_COMPLETE)
	{
		return;
	}
	
	if (false == gprs_is_ok())
	{
		return;
	}

	if((que->max > 0) && (que->in != que->out))
	{
		i = que->out % que->max;
		s_recoder_ctrl.send_index = i;
		if(voc[i].complete)
		{
			voc[i].lockflg = true;
			if (GM_SUCCESS != open_voice_file(&voc[i]))
			{
				delete_voice_file(&voc[i]);
				que->out++;
				s_recoder_ctrl.up = VOICE_UP_END;
				s_recoder_ctrl.send_count = 0;
				return;
			}

			if (voc[i].send_timeout)voc[i].send_timeout--;
			if (voc[i].send_map && (voc[i].send_map == voc[i].ack_map || voc[i].send_timeout == 0))  
			{
				s_recoder_ctrl.up = VOICE_UPLOADING;
				op->data = (u8 *)GM_MemoryAlloc(ONE_VOICE_PACKT_MAXLEN+1);
				if (!op->data)
				{
					return;
				}
				GM_memset(op->data, 0x00, ONE_VOICE_PACKT_MAXLEN+1);
				for(j = 0 ; j < 64 ; j++)
				{
					if(voc[i].send_map & ((u64)1 << j))
					{
						GM_FS_Seek(voc[i].f_handle, j * ONE_VOICE_PACKT_MAXLEN, 0);
						if((voc[i].file_size - j*ONE_VOICE_PACKT_MAXLEN) < ONE_VOICE_PACKT_MAXLEN)
						{
							f_size = voc[i].file_size - j*ONE_VOICE_PACKT_MAXLEN;
						}
						else
						{
							f_size = ONE_VOICE_PACKT_MAXLEN;
						}

						if(GM_FS_Read(voc[i].f_handle, (void *)op->data, f_size, &r_size) >= 0)
						{
							if(r_size == f_size)
							{									
								op->pack_len = f_size; 
								op->cur_pack = j;
								op->total_pack = voc[i].total_pack;
								GM_memcpy((u8 *)op->time, (u8 *)voc[i].time, 6);
								LOG(DEBUG, "clock(%d) recoder_voice_file_upload_proc file(%d) total_pack(%d) cur_pack(%d) pack_len(%d)", util_clock(), i,op->total_pack, op->cur_pack, op->pack_len);
								s_recoder_ctrl.send_clock = util_clock();
								if (GM_SUCCESS == gps_service_send_one_recoder_file_pack(op))
								{
									voc[i].send_timeout = (GM_SYSTEM_STATE_WORK == system_state_get_work_state()) ? 20 : 0;
									voc[i].send_map &= (~((u64)1<<j));
								}
								else
								{
									voc[i].send_timeout = (GM_SYSTEM_STATE_WORK == system_state_get_work_state()) ? 100 : 0;
								}
							}
						}
						break;
					}
				}
				GM_MemoryFree(op->data);
				op->data = NULL;
			}
			else if (voc[i].ack_map)
			{
				if (util_clock() - s_recoder_ctrl.send_clock > RECODER_FILE_SEND_INTERVAL && voc[i].send_timeout == 0)
				{
					s_recoder_ctrl.send_count++;
					if (s_recoder_ctrl.send_count < RECODER_FILE_SEND_COUNT)
					{
						voc[i].send_map = voc[i].ack_map;
					}
					else
					{
						JsonObject* p_log_root = NULL;
						delete_voice_file(&voc[i]);
						que->out++;
						s_recoder_ctrl.up = VOICE_UP_END;
						s_recoder_ctrl.send_count = 0;
						p_log_root = json_create();
						json_add_string(p_log_root, "event", "recode fail");
						json_add_string(p_log_root, "cause", "send to service fail");
						log_service_upload(INFO,p_log_root);
					}
				}
			}
			else
			{
				delete_voice_file(&voc[i]);
				que->out++;
				s_recoder_ctrl.up = VOICE_UP_END;
				s_recoder_ctrl.send_count = 0;
			}
		}
		else if(!voc[i].lockflg)
		{
			voc[i].f_handle = -1;
			que->out++;
		}
	}
}



static void recoder_send_to_service_call_back(u8 state)
{
	if (state == RECD_INIT || state == RECD_GOING)
	{
		gps_service_push_recoder_response_state(true);
	}
	else if (state == RECD_IDLE)
	{
		gps_service_push_recoder_response_state(false);
	}
	else// if (state == RECD_BROK)
	{
		if (VOICE_UP_NONE == s_recoder_ctrl.up)
		{
			s_recoder_ctrl.up = VOICE_UP_COMPLETE;
		}
	}
}



void recoder_recode_result_call_back(void *arg)
{
	sound_recoder_cb_struct *p = arg;
	voice_file_struct *v = NULL;

	if (!p)
	{
		return;
	}

	if (s_recoder_ctrl.state <= RECD_INIT)
	{	//说明已关闭录音
		return;
	}
	
	s_recoder_ctrl.recd_clock = util_clock();
	if(p->user) v = p->user;
	switch(p->ret)
	{
		case RES_RECD_TERMINATED:
		case RES_RECD_SUCESS:
		case RES_RECD_DISC_FULL:
			s_recoder_ctrl.queue.in++;
			ASSERTADR(v);
			v->lockflg = 0;
			v->complete = true;
			if (s_recoder.call_back_func)
			{
				s_recoder.call_back_func(RECD_BROK);
			}
			else
			{
				auto_test_recoder_file_count();
				GM_FS_Delete(v->filename);
			}
			LOG(INFO, "clock(%d) recoder_recode_result_call_back ret(%d).", util_clock(), p->ret);
			break;
			
		case RES_RECD_END_OF_FILE:
		case RES_RECD_NO_SPACE:
			LOG(WARN, "clock(%d) recoder_recode_result_call_back RES_RECD_NO_SPACE or RES_RECD_END_OF_FILE.", util_clock());
			ASSERTADR(v);
			s_recoder_ctrl.queue.max = 0;
			s_recoder_ctrl.state = RECD_BROK;
			GM_FS_Delete(v->filename);
			GM_memset((u8 *)v, 0 , sizeof(voice_file_struct));
			break;
			
		default:
			s_recoder_ctrl.state = RECD_BROK;
			ASSERTADR(v);
			break;
	}
}



void recoder_file_create(void)
{
	u8 idx = 0, i = 0;
	char temp[VOICE_FILE_NAME_MAX_LEN];
	voice_file_struct *ptr = s_recoder_ctrl.voice;
	voice_file_queue_struct *qtr = &s_recoder_ctrl.queue;

	LOG(INFO, "clock(%d) recoder_file_create.", util_clock());
	//先创建文件名，最多存在3个录音文件
	for(i = 0; i < MAX_VOICE_FILE ; i++)
	{
		GM_memset((u8 *)temp, 0, VOICE_FILE_NAME_MAX_LEN);
		
		GM_snprintf(temp,VOICE_FILE_NAME_MAX_LEN,VOICE_FILE_NAME, i);
		LOG(DEBUG, "clock(%d) recoder_file_create file_name:%s", util_clock(), temp);
		
		//录音文件名需要宽字符
		for(idx = 0; idx < GM_strlen(temp); idx++)
		{
			ptr->filename[idx] = (u16)temp[idx];
		}

		//录音文件存在，先删除
		if(GM_FS_CheckFile((U16 *)ptr->filename) >= 0)
		{
			GM_FS_Delete((U16 *)ptr->filename);
		}

		ptr->f_handle = -1;
		ptr++;
	}

	qtr->in  = 0;
	qtr->out = 0;
	qtr->max = i;
	
	s_recoder_ctrl.recd_clock = util_clock();
	s_recoder_ctrl.res_state = RES_SUCCESS;
	//是否往平台返回开启录音
	if (s_recoder.call_back_func)
	{
		s_recoder.call_back_func(RECD_INIT);
	}
	s_recoder_ctrl.state = RECD_GOING;
}



void recoder_work_recoding_proc(void)
{
	voice_file_struct *p = s_recoder_ctrl.voice;
	voice_file_queue_struct *que = &s_recoder_ctrl.queue;
	u8 i = 0;
	s32 state;
	
	if(que->max == 0)
	{
		return;
	}
	
	i = ((que->in)%(que->max));
	if(p[i].lockflg != true)
	{		
		state = GM_StartRecord(p[i].filename , s_recoder_ctrl.quality, s_recoder_ctrl.lim_size, s_recoder_ctrl.lim_time,(void *)(&p[i]),(kal_bool)0,0);
		if(state >= 0)
		{	
			LOG(INFO, "clock(%d) recoder_work_recoding_proc id(%d) GM_StartRecord(%d) success.", util_clock(), i, state);
			p[i].lockflg = true;
			util_get_current_local_time(p[i].time, NULL, 0x00);
			s_recoder_ctrl.recd_clock = util_clock();
			if (s_recoder_ctrl.res_state == RES_SUCCESS)
			{
				s_recoder_ctrl.res_state = RES_START_RECD;
			}
		}
		else
		{
			JsonObject* p_log_root = NULL;
			LOG(WARN, "clock(%d) recoder_work_recoding_proc id(%d) GM_StartRecord(%d) fail.", util_clock(), i, state);
			if (s_recoder_ctrl.res_state == RES_SUCCESS)
			{
				s_recoder_ctrl.res_state = RES_START_FAIL;
			}
			recoder_transfer_status(CURRENT_RECODER_FAIL);
			s_recoder_ctrl.recd_clock = util_clock();
			GM_StopRecord();
			p_log_root = json_create();
			json_add_string(p_log_root, "event", "recode fail");
			json_add_int(p_log_root, "cause", state);
			log_service_upload(INFO,p_log_root);
		}
		if (s_recoder.call_back_func)
		{
			s_recoder.call_back_func(RECD_GOING);
		}
	}
	else if (p[i].complete != true)
	{
		if (util_clock() - s_recoder_ctrl.recd_clock > s_recoder_ctrl.lim_time)
		{
			LOG(DEBUG, "clock(%d) recoder_work_recoding_proc GM_StopRecord because lim_time(%d).", util_clock(), s_recoder_ctrl.lim_time);
			s_recoder_ctrl.recd_clock = util_clock();
			GM_StopRecord();
		}
	}

	recoder_voice_file_upload_proc();
}



GM_ERRCODE recoder_work_proc(void)
{
	if (false == s_recoder.inited)
	{
		return GM_SUCCESS;
	}

	switch(s_recoder_ctrl.state)
	{
		case RECD_IDLE:
		break;
		case RECD_INIT:
		recoder_file_create();
		break;
		case RECD_GOING:
		recoder_work_recoding_proc();
		break;
		case RECD_BROK:
		break;
		default:
		break;
	}

	return GM_SUCCESS;
}


GM_ERRCODE stop_recode(bool upload_service)
{
	if (upload_service)
	{
		recoder_register(recoder_send_to_service_call_back);
	}
	if (s_recoder.inited == false)
	{
		s_recoder_ctrl.res_state = RES_IDLE;
	}
	else
	{
		s_recoder_ctrl.res_state = RES_SUCCESS;
	}
	recoder_destroy();
	return GM_SUCCESS;
}



GM_ERRCODE start_recode(bool upload_service)
{
	if (s_recoder.inited == false)
	{
		recoder_create(upload_service);
	}
	
	return GM_SUCCESS;
}



GM_ERRCODE recoder_create(bool upload_service)
{
	JsonObject* p_log_root = NULL;

	if (s_recoder.inited == true)
	{
		return GM_SUCCESS;
	}
	
	s_recoder.inited = true;
	GM_Set_Mic_Volume(AUD_VOLUME_MIC,6);
	recoder_control_init();
	GM_RecordRegister(recoder_recode_result_call_back);
	recoder_transfer_status(CURRENT_RECODER_WORK);
	if (upload_service == true)
	{
		recoder_register(recoder_send_to_service_call_back);
	}
	
	p_log_root = json_create();
	json_add_string(p_log_root, "event", "create recode");
	json_add_int(p_log_root, "upload_service", upload_service);
	log_service_upload(INFO,p_log_root);
	return GM_SUCCESS;
}



GM_ERRCODE recoder_destroy(void)
{
	JsonObject* p_log_root = NULL;

	if (s_recoder.inited == false)
	{
		return GM_SUCCESS;
	}
	
	s_recoder.inited = false;
	if (s_recoder_ctrl.state == RECD_GOING)
	{
		s_recoder_ctrl.state = RECD_IDLE;
		GM_StopRecord();
	}
	
	if (s_recoder.call_back_func)
	{
		s_recoder.call_back_func(RECD_IDLE);
		s_recoder.call_back_func = NULL;
	}
	recoder_control_init();
	recoder_transfer_status(CURRENT_RECODER_INIT);
	p_log_root = json_create();
	json_add_string(p_log_root, "event", "destroy recode");
	log_service_upload(INFO,p_log_root);
	return GM_SUCCESS;
}


GM_ERRCODE recoder_timer_proc(void)
{
	switch(s_recoder.status)
	{
		case CURRENT_RECODER_INIT:
		break;
		case CURRENT_RECODER_WORK:
		recoder_work_proc();
		break;
		case CURRENT_RECODER_FAIL:
		recoder_destroy();
		break;
		default:
		break;
	}
	
	return GM_SUCCESS;
}

GM_ERRCODE recoder_register(result_cb_fun call_back_func)
{
	s_recoder.call_back_func = call_back_func;
	return GM_SUCCESS;
}




