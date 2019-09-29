/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        gps_kalman_filter.h
 * Author:           王志华       
 * Version:          1.0
 * Date:             2019-09-07
 * Description:      卡尔曼滤波算法
 * Others:           不支持控制输入
 * Function List:    
    
 * History: 
    1. Date:         2019-09-07
       Author:       王志华
       Modification: 创建初始版本
    2. Date:          
       Author:         
       Modification: 

 */

#ifndef __GPS_KALMAN_FILTER_H__
#define __GPS_KALMAN_FILTER_H__

#include <stdio.h>
#include "kalman.h"

void gps_kalman_filter_create(double noise);
void gps_kalman_filter_destroy(void);
bool gps_kalman_filter_update(double lat, double lon, double seconds_since_last_timestep);
void gps_kalman_filter_read(double* lat, double* lon);

#endif

