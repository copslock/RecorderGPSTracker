/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        gps_kalman_filter.c
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
 
#include <math.h>
#include "gps_kalman_filter.h"
	
	
static KalmanFilter* s_p_filter = NULL;

static KalmanFilter* alloc_filter_velocity2d(double noise);

static void set_seconds_per_timestep(KalmanFilter* p_filter, double seconds_per_timestep);

static bool update_velocity2d(KalmanFilter* p_filter, double lat, double lon,double seconds_since_last_update);

static void get_lat_long(KalmanFilter* p_filter, double* lat, double* lon);

void gps_kalman_filter_create(double noise)
{
	s_p_filter = alloc_filter_velocity2d(noise);
}

void gps_kalman_filter_destroy(void)
{
	kalman_destroy(s_p_filter);
	s_p_filter = NULL;
}


bool gps_kalman_filter_update(double lat, double lon, double seconds_since_last_timestep)
{
	if (NULL == s_p_filter)
	{
		return false;
	}
	return update_velocity2d(s_p_filter, lat, lon, seconds_since_last_timestep);
}

void gps_kalman_filter_read(double* lat, double* lon)
{
	if (NULL == s_p_filter)
	{
		return;
	}

	get_lat_long(s_p_filter, lat, lon);
}


KalmanFilter* alloc_filter_velocity2d(double noise) 
{
	KalmanFilter* p_filter = kalman_create(4, 2);
  	double pos = 0.000001;
	const double trillion = 1000.0 * 1000.0 * 1000.0 * 1000.0;
	
  	matrix_set_identity(p_filter->p_state_transition);
  	set_seconds_per_timestep(p_filter, 1.0);
		 
	matrix_set(p_filter->p_observation_model,1.0, 0.0, 0.0, 0.0,0.0, 1.0, 0.0, 0.0);

  	matrix_set(p_filter->p_process_noise_covariance,pos, 0.0, 0.0, 0.0, 0.0, pos, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

	matrix_set(p_filter->p_observation_noise_covariance,pos * noise, 0.0, 0.0, pos * noise);

	matrix_set(p_filter->p_state_estimate, 0.0, 0.0, 0.0, 0.0);
	matrix_set_identity(p_filter->p_estimate_covariance);
	matrix_scale(p_filter->p_estimate_covariance, trillion);

	return p_filter;
}

void set_seconds_per_timestep(KalmanFilter* p_filter,double seconds_per_timestep) 
{
	double unit_scaler = 0.001;
	p_filter->p_state_transition->data[0][2] = unit_scaler * seconds_per_timestep;
	p_filter->p_state_transition->data[1][3] = unit_scaler * seconds_per_timestep;
}

bool update_velocity2d(KalmanFilter* p_filter, double lat, double lon,double seconds_since_last_timestep)
{
	set_seconds_per_timestep(p_filter, seconds_since_last_timestep);
	matrix_set(p_filter->p_observation, lat * 1000.0, lon * 1000.0);
	return kalman_update(p_filter);
}

void get_lat_long(KalmanFilter* p_filter, double* lat, double* lon)
{
	*lat = p_filter->p_state_estimate->data[0][0] / 1000.0;
	*lon = p_filter->p_state_estimate->data[1][0] / 1000.0;
}

