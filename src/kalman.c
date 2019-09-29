/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        kalman.c
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

#include <gm_memory.h>
#include "kalman.h"
#include "log_service.h"

KalmanFilter* kalman_create(int state_dimension,int observation_dimension) 
{
	KalmanFilter* p_filter = GM_MemoryAlloc(sizeof(KalmanFilter));
	p_filter->timestep = 0;
	p_filter->state_dimension = state_dimension;
	p_filter->observation_dimension = observation_dimension;

	p_filter->p_state_transition = matrix_create(state_dimension,state_dimension);
	p_filter->p_observation_model = matrix_create(observation_dimension,state_dimension);
	p_filter->p_process_noise_covariance = matrix_create(state_dimension,state_dimension);
	p_filter->p_observation_noise_covariance = matrix_create(observation_dimension,observation_dimension);

	p_filter->p_observation = matrix_create(observation_dimension, 1);

	p_filter->p_predicted_state = matrix_create(state_dimension, 1);
	p_filter->p_predicted_estimate_covariance = matrix_create(state_dimension,state_dimension);
	p_filter->p_innovation = matrix_create(observation_dimension, 1);
	p_filter->p_innovation_covariance = matrix_create(observation_dimension,observation_dimension);
	p_filter->p_inverse_innovation_covariance = matrix_create(observation_dimension,observation_dimension);
	p_filter->p_optimal_gain = matrix_create(state_dimension,observation_dimension);
	p_filter->p_state_estimate = matrix_create(state_dimension, 1);
	p_filter->p_estimate_covariance = matrix_create(state_dimension,state_dimension);

	p_filter->p_vertical_scratch = matrix_create(state_dimension,observation_dimension);
	p_filter->p_small_square_scratch = matrix_create(observation_dimension,observation_dimension);
	p_filter->p_big_square_scratch = matrix_create(state_dimension,state_dimension);

	return p_filter;
}

void kalman_destroy(KalmanFilter* p_filter) 
{ 
	matrix_destroy(p_filter->p_state_transition);
	matrix_destroy(p_filter->p_observation_model);
	matrix_destroy(p_filter->p_process_noise_covariance);
	matrix_destroy(p_filter->p_observation_noise_covariance);

	matrix_destroy(p_filter->p_observation);

	matrix_destroy(p_filter->p_predicted_state);
	matrix_destroy(p_filter->p_predicted_estimate_covariance);
	matrix_destroy(p_filter->p_innovation);
	matrix_destroy(p_filter->p_innovation_covariance);
	matrix_destroy(p_filter->p_inverse_innovation_covariance);
	matrix_destroy(p_filter->p_optimal_gain);
	matrix_destroy(p_filter->p_state_estimate);
	matrix_destroy(p_filter->p_estimate_covariance);

	matrix_destroy(p_filter->p_vertical_scratch);
	matrix_destroy(p_filter->p_small_square_scratch);
	matrix_destroy(p_filter->p_big_square_scratch);
}

bool kalman_update(KalmanFilter* p_filter)
{
	kalman_predict(p_filter);
	return kalman_estimate(p_filter);
}

void kalman_predict(KalmanFilter* p_filter) 
{
	p_filter->timestep++;

	//预测状态
	matrix_multiply(*p_filter->p_state_transition, *p_filter->p_state_estimate,p_filter->p_predicted_state);

	//预测状态估计协方差
	matrix_multiply(*p_filter->p_state_transition, *p_filter->p_estimate_covariance,p_filter->p_big_square_scratch);
	matrix_multiply_by_transpose(*p_filter->p_big_square_scratch, *p_filter->p_state_transition,p_filter->p_predicted_estimate_covariance);
	matrix_add(*p_filter->p_predicted_estimate_covariance, *p_filter->p_process_noise_covariance,p_filter->p_predicted_estimate_covariance);
}

bool kalman_estimate(KalmanFilter* p_filter) 
{
	//计算新息（innovation）
	matrix_multiply(*p_filter->p_observation_model, *p_filter->p_predicted_state, p_filter->p_innovation);
	matrix_subtract(*p_filter->p_observation, *p_filter->p_innovation,p_filter->p_innovation);

	//计算新息（innovation）协方差
	matrix_multiply_by_transpose(*p_filter->p_predicted_estimate_covariance, *p_filter->p_observation_model, p_filter->p_vertical_scratch);
	matrix_multiply(*p_filter->p_observation_model, *p_filter->p_vertical_scratch,p_filter->p_innovation_covariance);
	matrix_add(*p_filter->p_innovation_covariance, *p_filter->p_observation_noise_covariance,p_filter->p_innovation_covariance);

	//求新息（innovation）协方差的逆矩阵，可能不可逆
	if(false == matrix_destructive_invert(*p_filter->p_innovation_covariance,p_filter->p_inverse_innovation_covariance))
	{
		LOG(DEBUG,"Failed to kalman_estimate!");
		matrix_print(*p_filter->p_innovation_covariance);
		return false;
	}

	//计算最优卡尔曼增益
	matrix_multiply(*p_filter->p_vertical_scratch, *p_filter->p_inverse_innovation_covariance,p_filter->p_optimal_gain);

	//估计状态
	matrix_multiply(*p_filter->p_optimal_gain, *p_filter->p_innovation,p_filter->p_state_estimate);
	matrix_add(*p_filter->p_state_estimate, *p_filter->p_predicted_state,p_filter->p_state_estimate);

	//估计状态协方差
	matrix_multiply(*p_filter->p_optimal_gain, *p_filter->p_observation_model,p_filter->p_big_square_scratch);
	matrix_subtract_from_identity(p_filter->p_big_square_scratch);
	matrix_multiply(*p_filter->p_big_square_scratch, *p_filter->p_predicted_estimate_covariance,p_filter->p_estimate_covariance);

	return true;
}

