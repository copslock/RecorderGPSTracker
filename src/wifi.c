/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        wifi.c
 * Author:           李耀轩       
 * Version:          1.0
 * Date:             2019-09-18
 * Description:      实现WIFI扫描功能
 * Others:      
 * Function List:    
    1. 创建
    2. 销毁 
    3. 定时处理入口
    4. 开启/关闭WIFI
 * History:  
    1. Date:         2019-09-18    
       Author:       李耀轩
       Modification  创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

*/
#include "wifi.h"
#include "json.h"
#include "log_service.h"
#include "utility.h"
#include "gm_type.h"
#include "gm_stdlib.h"


static GM_ERRCODE wifi_transfer_status(u8 new_status);

static void wifi_init_call_back(void *user_data, wlan_init_cnf_struct *cnf);

static void wifi_scan_call_back(void *user_data, wlan_scan_cnf_struct *cnf);

static void wifi_deinit_call_back(void *user_data, wlan_deinit_cnf_struct *cnf);

static void wifi_scan_ap_sequence(wlan_scan_cnf_struct *cnf);


#define WIFI_STATUS_STRING_MAX_LEN 20
#define WIFI_RETRY_INTERVALL    2
#define WIFI_RETRY_MAX_CNT		5



typedef struct 
{
	bool inited;
	WifiStatusEnum status;
	u32 clock;
	u8 fail_count;
	PsFuncPtr call_back_func;
	u8 scan_ap_num;
    scanonly_scan_ap_info_struct  scan_ap[SCANONLY_MAX_SCAN_AP_NUM];
}WifiStruct;

WifiStruct s_wifi;


const char s_wifi_status_string[CURRENT_WIFI_STATUS_MAX][WIFI_STATUS_STRING_MAX_LEN] = 
{
    "CURRENT_WIFI_UNINIT",
    "CURRENT_WIFI_INIT",
    "CURRENT_WIFI_SCAN",
    "CURRENT_WIFI_DEINIT",
    "CURRENT_WIFI_FAIL",
};


const char * wifi_status_string(WifiStatusEnum statu)
{
    return s_wifi_status_string[statu];
}


static GM_ERRCODE wifi_transfer_status(WifiStatusEnum new_status)
{
    WifiStatusEnum old_status = s_wifi.status;
    GM_ERRCODE ret = GM_PARAM_ERROR;
    switch(s_wifi.status)
    {
    	case CURRENT_WIFI_UNINIT:
            switch(new_status)
            {
            	case CURRENT_WIFI_UNINIT:
                    break;
                case CURRENT_WIFI_INIT:
					ret = GM_SUCCESS;
                    break;
                case CURRENT_WIFI_SCAN:
                    break;
                case CURRENT_WIFI_DEINIT:
                    break;
				case CURRENT_WIFI_FAIL:
                    break;
                default:
                    break;
            }
            break;
        case CURRENT_WIFI_INIT:
            switch(new_status)
            {
            	case CURRENT_WIFI_UNINIT:
					ret = GM_SUCCESS;
                    break;
                case CURRENT_WIFI_INIT:
                    break;
                case CURRENT_WIFI_SCAN:
					ret = GM_SUCCESS;
                    break;
                case CURRENT_WIFI_DEINIT:
                    break;
				case CURRENT_WIFI_FAIL:
					ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case CURRENT_WIFI_SCAN:
            switch(new_status)
            {
            	case CURRENT_WIFI_UNINIT:
					ret = GM_SUCCESS;
                    break;
                case CURRENT_WIFI_INIT:
                    break;
                case CURRENT_WIFI_SCAN:
                    break;
                case CURRENT_WIFI_DEINIT:
					ret = GM_SUCCESS;
                    break;
				case CURRENT_WIFI_FAIL:
					ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case CURRENT_WIFI_DEINIT:
            switch(new_status)
            {
            	case CURRENT_WIFI_UNINIT:
					ret = GM_SUCCESS;
                    break;
                case CURRENT_WIFI_INIT:
                    break;
                case CURRENT_WIFI_SCAN:
                    break;
                case CURRENT_WIFI_DEINIT:
                    break;
				case CURRENT_WIFI_FAIL:
                    break;
                default:
                    break;
            }
            break;
		case CURRENT_WIFI_FAIL:
            switch(new_status)
            {
            	case CURRENT_WIFI_UNINIT:
					ret = GM_SUCCESS;
                    break;
                case CURRENT_WIFI_INIT:
                    break;
                case CURRENT_WIFI_SCAN:
                    break;
                case CURRENT_WIFI_DEINIT:
					ret = GM_SUCCESS;
                    break;
				case CURRENT_WIFI_FAIL:
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
        s_wifi.status = new_status;
        LOG(INFO,"clock(%d) wifi_transfer_status from %s to %s success", util_clock(), wifi_status_string(old_status),wifi_status_string(new_status));
    }
    else
    {
        LOG(WARN,"clock(%d) wifi_transfer_status assert(from %s to %s) failed", util_clock(), wifi_status_string(old_status),wifi_status_string(new_status));
    }

    return ret;

}


WifiStatusEnum get_wifi_status(void)
{
	return s_wifi.status;
}


u8 get_wifi_scan_ap_num(void)
{
	return s_wifi.scan_ap_num;
}


u8 *get_wifi_scan_ap_info(void)
{
	return (u8 *)&s_wifi.scan_ap;
}


GM_ERRCODE clear_wifi_scan_ap(void)
{
	s_wifi.scan_ap_num = 0;
	GM_memset(&s_wifi.scan_ap, 0, sizeof(scanonly_scan_ap_info_struct)*SCANONLY_MAX_SCAN_AP_NUM);
	return GM_SUCCESS;
}


static void wifi_scan_ap_sequence(wlan_scan_cnf_struct *cnf)
{
	u8 idx;
	u8 index;
	s8 rssi;
	
	for (idx=0; idx<cnf->scan_ap_num-1; ++idx)
	{
		for (index=0; index<cnf->scan_ap_num-1-idx; ++index)
		{
			if (cnf->scan_ap[index].rssi < cnf->scan_ap[index+1].rssi)
			{
				rssi = cnf->scan_ap[index].rssi;
				cnf->scan_ap[index].rssi = cnf->scan_ap[index+1].rssi;
				cnf->scan_ap[index+1].rssi = rssi;
			}
		}
	}
}


void wifi_scan_result(bool result)
{
	if (s_wifi.call_back_func)
	{
		s_wifi.call_back_func((void *)&result);
	}
	else
	{
		LOG(INFO, "clocl(%d) wifi_scan_result call_back_func NULL", util_clock());
	}
}


static void wifi_init_call_back(void *user_data, wlan_init_cnf_struct *cnf)
{
	if (!s_wifi.inited)
	{
		return;
	}
	
	if (SCANONLY_SUCCESS == cnf->status)
	{
		wlan_scan_req_struct scan_req;
		JsonObject* p_log_root = json_create();
		json_add_string(p_log_root, "event", "wifi init");
		json_add_string(p_log_root, "result", "success");
		log_service_upload(INFO,p_log_root);
        LOG(INFO,"clock(%d) wifi_init_call_back success.", util_clock());
		s_wifi.fail_count = 0;
		scan_req.scan_type = 1;
		wlan_scan(&scan_req,wifi_scan_call_back,NULL);
		wifi_transfer_status(CURRENT_WIFI_SCAN);
	}
	else
	{
		s_wifi.fail_count++;
		LOG(WARN,"clock(%d) wifi_init_call_back status(%d) fail.", util_clock(), cnf->status);
	}
}


static void wifi_init_proc(void)
{
	wlan_result_enum result;

	if (!s_wifi.inited)
	{
		return;
	}
	
	if (util_clock()- s_wifi.clock >= WIFI_RETRY_INTERVALL)
	{
		if (s_wifi.fail_count < WIFI_RETRY_MAX_CNT)
		{
			s_wifi.clock = util_clock();
			result = wlan_init(NULL,wifi_init_call_back,NULL);
			if (result != WLAN_RESULT_SUCCESS)
			{
				s_wifi.fail_count++;
			}
			LOG(INFO,"clock(%d) wifi_init_proc result(%d).", util_clock(), result);
		}
		else
		{
			JsonObject* p_log_root = json_create();
			json_add_string(p_log_root, "event", "wifi init");
			json_add_string(p_log_root, "result", "fail");
			log_service_upload(INFO,p_log_root);
			LOG(WARN,"clock(%d) wifi_init_proc fail.", util_clock());
			s_wifi.fail_count = 0;
			wifi_transfer_status(CURRENT_WIFI_FAIL);
			wifi_scan_result(false);
		}
	}
}


static void wifi_scan_call_back(void *user_data, wlan_scan_cnf_struct *cnf)
{
	if (!s_wifi.inited)
	{
		return;
	}
	
	if (SCANONLY_SUCCESS == cnf->status)
	{
        LOG(INFO,"clock(%d) wifi_scan_call_back success.", util_clock());
		s_wifi.fail_count = 0;
		clear_wifi_scan_ap();
		if(cnf->scan_ap_num > 0)
		{
			wifi_scan_ap_sequence(cnf);
			s_wifi.scan_ap_num = cnf->scan_ap_num;
			GM_memcpy(&s_wifi.scan_ap, cnf->scan_ap, sizeof(scanonly_scan_ap_info_struct)*SCANONLY_MAX_SCAN_AP_NUM);
			wifi_scan_result(true);
			//wlan_deinit(NULL,wifi_deinit_call_back,NULL);
			//wifi_transfer_status(CURRENT_WIFI_DEINIT);
		}
	}
	else
	{
		s_wifi.fail_count++;
		LOG(WARN,"clock(%d) wifi_scan_call_back status(%d) fail.", util_clock(), cnf->status);
	}
}



static void wifi_scan_proc(void)
{
	wlan_result_enum result;
	wlan_scan_req_struct scan_req;

	if (!s_wifi.inited)
	{
		return;
	}
	
	if (util_clock()- s_wifi.clock >= WIFI_RETRY_INTERVALL)
	{
		if (s_wifi.fail_count < WIFI_RETRY_MAX_CNT)
		{
			s_wifi.clock = util_clock();
			scan_req.scan_type = 1;
			result = wlan_scan(&scan_req,wifi_scan_call_back,NULL);
			LOG(INFO,"clock(%d) wifi_scan_proc result(%d).", util_clock(), result);
		}
		else
		{
			JsonObject* p_log_root = json_create();
			json_add_string(p_log_root, "event", "wifi scan");
			json_add_string(p_log_root, "result", "fail");
			log_service_upload(INFO,p_log_root);
			LOG(WARN,"clock(%d) wifi_scan_proc fail.", util_clock());
			s_wifi.fail_count = 0;
			wifi_transfer_status(CURRENT_WIFI_DEINIT);
			wifi_scan_result(false);
		}
	}
}


static void wifi_deinit_call_back(void *user_data, wlan_deinit_cnf_struct *cnf)
{
	if (!s_wifi.inited)
	{
		return;
	}
	
	if (SCANONLY_SUCCESS == cnf->status)
	{
		JsonObject* p_log_root = json_create();
		json_add_string(p_log_root, "event", "wifi deinit");
		json_add_string(p_log_root, "result", "success");
		log_service_upload(INFO,p_log_root);
        LOG(INFO,"clock(%d) wifi_deinit_call_back success.", util_clock());
		s_wifi.fail_count = 0;
		s_wifi.inited = false;
		s_wifi.scan_ap_num = 0;
		GM_memset(&s_wifi.scan_ap, 0x00, sizeof(scanonly_scan_ap_info_struct)*SCANONLY_MAX_SCAN_AP_NUM);
		wifi_transfer_status(CURRENT_WIFI_UNINIT);
	}
	else
	{
		s_wifi.fail_count++;
		LOG(WARN,"clock(%d) wifi_deinit_call_back status(%d) fail.", util_clock(), cnf->status);
	}
}



static void wifi_deinit_proc(void)
{
	wlan_result_enum result;

	if (!s_wifi.inited)
	{
		return;
	}
	
	if (util_clock()- s_wifi.clock >= WIFI_RETRY_INTERVALL)
	{
		if (s_wifi.fail_count < WIFI_RETRY_MAX_CNT)
		{
			s_wifi.clock = util_clock();
			result = wlan_deinit(NULL,wifi_deinit_call_back,NULL);
			LOG(INFO,"clock(%d) wifi_deinit_proc result(%d).", util_clock(), result);
		}
		else
		{
			JsonObject* p_log_root = json_create();
			json_add_string(p_log_root, "event", "wifi deinit");
			json_add_string(p_log_root, "result", "fail");
			log_service_upload(INFO,p_log_root);
			s_wifi.fail_count = 0;
			s_wifi.inited = false;
			wifi_transfer_status(CURRENT_WIFI_UNINIT);
			LOG(WARN,"clock(%d) wifi_deinit_proc fail.", util_clock());
		}
	}
}



GM_ERRCODE wifi_create(PsFuncPtr callback)
{
	GM_memset(&s_wifi, 0, sizeof(WifiStruct));
	s_wifi.clock = util_clock();
	s_wifi.inited = true;
	s_wifi.call_back_func = callback;
	wifi_transfer_status(CURRENT_WIFI_INIT);
	wlan_init(NULL,wifi_init_call_back,NULL);
	LOG(INFO,"clock(%d) wifi_create.", util_clock());
	return GM_SUCCESS;
}


GM_ERRCODE wifi_destroy(void)
{
	s_wifi.clock = util_clock();
	s_wifi.fail_count = 0;
	s_wifi.call_back_func = NULL;
	if (CURRENT_WIFI_SCAN == get_wifi_status())
	{
		wifi_transfer_status(CURRENT_WIFI_DEINIT);
	}
	else
	{
		s_wifi.inited = false;
		wifi_transfer_status(CURRENT_WIFI_UNINIT);
	}
	LOG(INFO,"clock(%d) wifi_destroy.", util_clock());
	return GM_SUCCESS;
}

GM_ERRCODE wifi_timer_proc(void)
{
	switch(s_wifi.status)
    {
    	case CURRENT_WIFI_UNINIT:
			break;
        case CURRENT_WIFI_INIT:
            wifi_init_proc();
            break;
		 case CURRENT_WIFI_SCAN:
		 	wifi_scan_proc();
            break;
        case CURRENT_WIFI_DEINIT:
			wifi_deinit_proc();
            break;
		case CURRENT_WIFI_FAIL:
			wifi_destroy();
            break;
    }
	
    return GM_SUCCESS;
}



GM_ERRCODE wifi_register(PsFuncPtr callback)
{
	s_wifi.call_back_func = callback;
	return GM_SUCCESS;
}



