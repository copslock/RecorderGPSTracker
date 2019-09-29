/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        wifi.h
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

#ifndef __WIFI_H_
#define __WIFI_H_

#include "gm_type.h"
#include "error_code.h"

typedef enum 
{
	CURRENT_WIFI_UNINIT = 0,
	CURRENT_WIFI_INIT = 1,
	CURRENT_WIFI_SCAN = 2,
	CURRENT_WIFI_DEINIT = 3,
	CURRENT_WIFI_FAIL = 4,
	
	CURRENT_WIFI_STATUS_MAX
}WifiStatusEnum;
	


typedef enum
{  
   SCANONLY_SUCCESS = 0,
   
   SCANONLY_INIT_BUSY_IS_INITING= 1,
   SCANONLY_INIT_FAIL__ALREAD_INITED,
   SCANONLY_INIT_FAIL__DRIVER_REASON,
   SCANONLY_INIT_FAIL__UNKOWN,

   SCANONLY_DEINIT_BUSY__IS_DEINITING = 10,
   SCANONLY_DEINIT_FAIL__ALREAD_DEINITED,
   SCANONLY_DEINIT_FAIL__DRIVER_REASON,
   SCANONLY_DEINIT_FAIL__UNKOWN,

   SCANONLY_SCAN_BUSY__IS_SCANNING = 20,
   SCANONLY_SCAN_FAIL__NOT_INITED,
   SCANONLY_SCAN_FAIL__DRIVER_REASON,
   SCANONLY_SCAN_FAIL__UNKOWN,
   
   SCANONLY_STATUS_END
} SCANONLY_STATUS_ENUM;


#define SCANONLY_MAX_SCAN_AP_NUM			30
#define SCANONLY_MAC_ADDRESS_LEN	    	6
#define SCANONLY_SSID_MAX_LEN				32

typedef struct scanonly_scan_ap_info_struct
{    
  kal_uint8                      		bssid[ SCANONLY_MAC_ADDRESS_LEN ];
  kal_uint8                       		ssid_len;
  kal_uint8                       		ssid [ SCANONLY_SSID_MAX_LEN ];    
  kal_int8                        		rssi;                           
  kal_uint8                       		channel_number;    
} scanonly_scan_ap_info_struct;


/* Result enum */
typedef enum
{
    WLAN_RESULT_SUCCESS,
    WLAN_RESULT_WOULDBLOCK,
    WLAN_RESULT_FAIL,
    
    WLAN_RESULT_TOTAL
} wlan_result_enum;

/* Struct */
typedef struct
{
    // Reserved struct
    void *para;
} wlan_init_req_struct;

typedef struct
{
    SCANONLY_STATUS_ENUM status;
} wlan_init_cnf_struct;

typedef struct
{
    // Reserved struct
    void *para;
} wlan_deinit_req_struct;

typedef struct
{
    SCANONLY_STATUS_ENUM status;
} wlan_deinit_cnf_struct;

typedef struct
{
    // Reserved struct
    void *para;
    kal_uint8 scan_type;
} wlan_scan_req_struct;

typedef struct
{
    SCANONLY_STATUS_ENUM                 status;
    kal_uint8                            scan_ap_num;
    scanonly_scan_ap_info_struct         scan_ap[SCANONLY_MAX_SCAN_AP_NUM];
} wlan_scan_cnf_struct;

/* Callback */
typedef void (*wlan_init_cb_func_ptr) (void *user_data, wlan_init_cnf_struct *cnf);
typedef void (*wlan_deinit_cb_func_ptr) (void *user_data, wlan_deinit_cnf_struct *cnf);
typedef void (*wlan_scan_cb_func_ptr) (void *user_data, wlan_scan_cnf_struct *cnf);


/**
 * Function:   获取WIFI模块工作状态
 * Description:获取WIFI模块工作状态
 * Input:	   无
 * Output:	   无
 * Return:	   WIFI模块当前工作状态
 * Others:	   
 */
WifiStatusEnum get_wifi_status(void);


/**
 * Function:   获取WIFI已扫描AP数目
 * Description:获取WIFI已扫描AP数目
 * Input:	   无
 * Output:	   无
 * Return:	   已扫描AP数目
 * Others:	   
 */
u8 get_wifi_scan_ap_num(void);


/**
 * Function:   获取WIFI已扫描AP存储地址
 * Description:获取WIFI已扫描AP存储地址，返回数据结构 scanonly_scan_ap_info_struct
 * Input:	   无
 * Output:	   无
 * Return:	   AP存储地址
 * Others:	   
 */
u8 *get_wifi_scan_ap_info(void);


/**
 * Function:   清除WIFI已扫描AP
 * Description:清除WIFI已扫描AP
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   更新WIFI前必须调用
 */
GM_ERRCODE clear_wifi_scan_ap(void);


/**
 * Function:   创建wifi模块
 * Description:创建wifi模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   使用前必须调用
 */
GM_ERRCODE wifi_create(PsFuncPtr callback);


/**
 * Function:   销毁wifi模块
 * Description:销毁wifi模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE wifi_destroy(void);


/**
 * Function:   wifi模块定时处理入口
 * Description:wifi模块定时处理入口
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE wifi_timer_proc(void);


/**
 * Function:   wifi扫描结果通知
 * Description:wifi扫描结果通知
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE wifi_register(PsFuncPtr callback);


/*****************************************************************************
 * FUNCTION
 *  wlan_init
 * DESCRIPTION
 *  The function is used to init the WLAN.
 * PARAMETERS
 *  callback    :   [IN]    callback function to notify applications the init operation is done
 *  user_data   :   [IN]    application piggyback data 
 * RETURN VALUES
 *  WLAN_RESULT_WOULDBLOCK    : waiting callback
 *****************************************************************************/

wlan_result_enum wlan_init(wlan_init_req_struct *req, wlan_init_cb_func_ptr callback, void *user_data);


/*****************************************************************************
 * FUNCTION
 *  wlan_deinit
 * DESCRIPTION
 *  The function is used to deinit the WLAN.
 * PARAMETERS
 *  callback    :   [IN]    callback function to notify applications the init operation is done
 *  user_data   :   [IN]    application piggyback data 
 * RETURN VALUES
 *  WLAN_RESULT_WOULDBLOCK    : waiting callback
 *****************************************************************************/
wlan_result_enum wlan_deinit(wlan_deinit_req_struct *req, wlan_deinit_cb_func_ptr callback, void *user_data);


/*****************************************************************************
 * FUNCTION
 *  wlan_scan
 * DESCRIPTION
 *  The function is used to scan.
 * PARAMETERS
 *  callback    :   [IN]    callback function to notify applications the init operation is done
 *  user_data   :   [IN]    application piggyback data 
 * RETURN VALUES
 *  WLAN_RESULT_WOULDBLOCK    : waiting callback
 *****************************************************************************/
wlan_result_enum wlan_scan(wlan_scan_req_struct *req, wlan_scan_cb_func_ptr callback, void *user_data);


#endif /*__WIFI_H_*/

